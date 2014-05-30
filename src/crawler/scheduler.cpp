#include "scheduler.h"
#include "url/url.h"
#include "util/util.h"
#include "util/xmlGet.h"
#include "util/rmdirtree.h"
#include "common.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <fstream>
using std::pair;
using std::ostringstream;
using std::endl;
using std::flush;
using std::cout;
using std::cerr;
using std::runtime_error;
using std::ios;
using std::ofstream;
using std::ifstream;
using namespace mysqlpp;

const char* flag_keep_env = "keep_the_env";

CScheduler::CScheduler(auto_ptr<CHostTable> hostTable, Connection *c, const string& pathExport)
{
	this->hostTable = hostTable;
	this->conn = c;
	this->pathExport = pathExport;
	char* h = getenv("HOSTNAME");
	if (!h) throw runtime_error("Can not get env \"HOSTNAME\".");
	crawler = h;

	const int bufsize=256;
	char buf[bufsize];
	time_t now;
	time(&now);
	strftime(buf, bufsize, "%y%m%d", gmtime(&now));
	string daystr = buf;
	daystr_len =daystr.length();
}

int
CScheduler::getProcessNum()
{
	Query select = conn->query();
	select << "SELECT ProcessQuota FROM crawler WHERE name="
	  <<quote<<crawler;
	StoreQueryResult res = select.store();
	if (!res || res.size() == 0)
		return -1;
	return res[0]["ProcessQuota"];
}

CScheduler::~CScheduler()
{
	if (!fileExport.empty())
	{
		expose(fileExport);
		fileExport.clear();
	}
}

/*
 * CScheduler::startCrawler()
 *  Return value -2, -1, 0, 1
 *      -2: Please quit.
 *      -1: failed to start a crawler process
 *      0:  no task host or no process quota
 *      1: start a crawler process
 */
int
CScheduler::startCrawler()
{
    int max_crawler_num = getProcessNum(); 
    if (max_crawler_num < 0)
        return -2;	
    if ((int)pool.size() >= max_crawler_num)
        return 0;
    if (hosts.empty()) 
    {
        hostTable->selectStartEnv(hosts, "./");
        if (hosts.empty())
            return 0;
    }
	CEnv env = hosts.back();
	assert(!env.host.empty());
	pid_t pid=run_crawler(env, true);
	if (pid <= 0) 
		return -1;
	hosts.pop_back();
	pool.insert(pair<pid_t, string>(pid
            , CURL::hostport("http", env.host, env.port)));
	cout<<"start crawler: "<<this->pid<<"==>"<<pid<<endl;
	return 1;
}

int 
CScheduler::collectRaw(const string &page_fn, const string &path, time_t now)
{
	const int bufsize=256;
	char buf[bufsize];
	strftime(buf, bufsize, "%y%m%d%H%M.twr", gmtime(&now));
	string nowstr = buf;

	ifstream page(page_fn.c_str());
	ofstream ofsExport;
	if (fileExport.size()>0)
	{ // check fileExport size.
		struct stat info;
		if (0 != stat((pathExport+"/."+fileExport).c_str(), &info))
		{
			ostringstream oss;
			oss<<"cleanCrawler():stat() failed, "<<pathExport<<"/."
			  <<fileExport<<": "<<strerror(errno);
			throw runtime_error(oss.str());
		}
		if (info.st_size > 1000000000  // over maximum file size 
		   || nowstr.compare(0, daystr_len, fileExport, 0
			, daystr_len)>0   // output at least one file one day 
		   )
		{
			expose(fileExport);
			fileExport.clear();
		}
	}
	if (fileExport.empty())
	{  //create new file
		fileExport = nowstr;
		ofsExport.open((pathExport+"/."+fileExport).c_str());
		if (ofsExport)
		{
			ofsExport.close(); // flush file for following chmod()
			chmod((pathExport+"/."+fileExport).c_str(), 0666);
			ofsExport.open((pathExport+"/."+fileExport).c_str());
		}
		else
		{
			ostringstream oss;
			oss<<"cleanCrawler():Can't open file: "<<pathExport<<"/."
			  <<fileExport<<": "<<strerror(errno);
			throw runtime_error(oss.str());
		}
	}
	else { // resume old file
		ofsExport.open((pathExport+"/."+fileExport).c_str(), ios::app|ios::out);
	}
	if (!(ofsExport<<page.rdbuf()))
	{
		ostringstream oss;
		oss<<"cleanCrawler():write file failed: "<<pathExport<<"/."<<fileExport;
		throw runtime_error(oss.str());
	}
	return 0;
}

int
CScheduler::cleanCrawler(const string &path)
{
	// Get current time to create export file name.
	time_t now;
	time(&now);

	const int bufsize=256;
	char buf[bufsize];
	// copy Web_Raw
	string page_fn = path+"/"+raw_file;
	if (file_size(page_fn) > 0)
		collectRaw(page_fn, path, now);
	strftime(buf, bufsize, "%y%m%d", gmtime(&now));
	// copy out neighbor file
	if (this->collect_outlinks)
		append(path+"/"+out_neighbor_file, string(out_neighbor_file)+"."+buf
			, path+"\n", "\n");
	// copy overflow file
	if (this->collect_overflow)
		append(path+"/"+overflow_file, string(overflow_file)+"."+buf
			, path+"\n", "\n");
	// copy discard file
	if (this->collect_discard)
		append(path+"/"+discard_file, string(discard_file)+"."+buf
			, path+"\n", "\n"); 
	// copy report file
	if (0 != append(path+"/"+report_file, string(report_file)+"."+buf, "", ""))
	{
		ostringstream oss;
		oss<<"cleanCrawler():Can't read file:"<<path<<"/"<<report_file;
		throw runtime_error(oss.str());
	}

	// copy saved link file
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	doc.reset(xmlParseFile((path+"/"+report_file).c_str()));
	if (!doc.get())
		throw runtime_error("Can not open xml file: "+path+"/"+report_file);
	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> ctx;
	ctx.reset(xmlXPathNewContext(doc.get()));
	if (!ctx.get())
		throw runtime_error("xmlXPathNewContext() failed.");
	auto_ptr<xmlChar_ptr_vector> linkFiles;
	linkFiles.reset(xmlGetMultiStr(ctx.get(), BAD_CAST"//SavedLinkFile", 0));
	for (unsigned i=0; i<linkFiles->size(); i++)
	{
		string fn = (const char*)linkFiles->operator[](i);
		if (0 != append(path+"/"+fn, fn+".links", "", ""))
		{
			ostringstream oss;
			oss<<"cleanCrawler():Can't read file:"<<path<<"/"<<fn;
			throw runtime_error(oss.str());
		}
	}
	// remove the env directory
	ifstream keep_env((path+"/"+flag_keep_env).c_str());
	if (!keep_env)
	{
		switch(rmdirtree(path))
		{
		case 0: 
			break;
		case -1:
			cerr<<"Can not list a directory."<<endl;
			return -1;
		case -2:
			cerr<<"Can not unlink a file."<<endl;
			return -2;
		case -3:
			cerr<<"Can not remove a dir."<<endl;
			return -3;
		default: 
			assert(false);
		}
	}
	return 0;
}

int
CScheduler::cleanCrawler()
{
	int stat;
	pid_t pid = waitpid((pid_t)-1, &stat, WNOHANG);
	if (pid == 0)
		return 0;
	else if (pid < 0)
		return -1;
	crawler_pool_t::iterator it = pool.find(pid);
	assert(it != pool.end());
	string path = "./"+it->second;
	// Test "report.xml" file
	int fd = open((path+'/'+report_file).c_str(), O_RDWR);
	if (fd >= 0)
	{
		close(fd);
	}
	else if (fd == -1)
	{
		if (errno == ENOENT)
		{
			cout<<"crawler crashed! pid="<<pid<<", host="
			  <<it->second<<endl;
			hostTable->reportCrash(it->second);
			pool.erase(it);
			return pid;
		}
		else
		{
			ostringstream oss;
			oss<<"cleanCrawler():Can't open file:"<<path<<"/"
			  <<report_file<<": "<<strerror(errno);
			throw runtime_error(oss.str());
		}
	}
	if (0==hostTable->saveResult(path) && 0==cleanCrawler(path))
		cout<<"clean crawler OK! pid="<<pid<<", host="<<it->second<<endl;
	else
		cerr<<"clean crawler failed, pid="<<pid<<", host="<<it->second<<endl;
	pool.erase(it);
	return pid;
}

/*	
 * Run the crawler command like this:
 *  ../host_crawler --host=www.pku.edu.cn --port=80 --lpath-file=root \
 *      --visited-prefix=visited --npages=100000 --interval=3 --cout=myout
 */
int 
CScheduler::exec_crawler(const CEnv &env, bool debug)
{
	const char* prog = "host_crawler";
	vector<string> v;
	v.push_back(prog);
	v.push_back("--host="+env.host);
	v.push_back("--port="+tostring(env.port));
	v.push_back(string("--lpath-file=")+root_file);
	v.push_back(string("--visited-prefix=")+visited_prefix);
	v.push_back(string("--cookie-file=")+cookie_fn);
	if (debug)
		v.push_back("--debug");
	v.push_back("--npages="+tostring(env.pageNum));
	v.push_back("--interval="+tostring(env.interval));
	for (set<string>::const_iterator cit=env.save_link_of.begin()
	  ; cit != env.save_link_of.end(); cit ++) 
		v.push_back("--save-link="+*cit);
	v.push_back("--cout=myout");
	char* argv[v.size()+1];
	for (int i=0; i<(int)v.size(); i++)
		argv[i] = (char*)v[i].c_str();
	argv[v.size()] = 0;

	cout<<prog<<' ';
	for (int i=0; argv[i] != 0; i++)
		cout<<argv[i]<<' ';
	cout<<endl;
#if 0	// We prefer seeing error infomation in the console
	// Here: redirect error output to a local file "myerr"
	int fd_err;
	if ((fd_err=open("myerr", O_WRONLY|O_APPEND|O_CREAT, 0666)) == -1)
	{
		ostringstream oss;
		oss<<"open(\"myerr\"):"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	if (-1 == dup2(fd_err, 2))
	{
		ostringstream oss;
		oss<<"dup2():"<<strerror(errno);
		throw runtime_error(oss.str());
	}
#endif //0
	if (execvp(prog, argv) == -1)
	{
		ostringstream oss;
		oss<<"Can not run execvp():"<<strerror(errno)<<"\n";
		oss<<prog<<' ';
		for (int i=0; argv[i] != 0; i++)
			oss<<argv[i]<<' ';
		throw runtime_error(oss.str());
	}
	return 0;
}


pid_t 
CScheduler::run_crawler(const CEnv &env, bool call_fork, bool debug)
{
	string hostport = CURL::hostport("http", env.host, env.port);
	string path = "./" + hostport;
	ifstream ifs(("./"+hostport+".links").c_str());
	if (ifs)
	{
		ofstream ofs((path+"/"+root_file).c_str(), ios::out|ios::app);
		ofs<<ifs.rdbuf();
		ifs.close();
		unlink(("./"+hostport+".links").c_str());
	}
	pid_t pid = 0;
	if (call_fork)
		pid = fork();
	if (pid == 0) // child process or not call_fork;
	{
		if (0 != chdir(path.c_str()))
		{
			ostringstream oss;
			oss<<"Can not chdir to dir:"<<path;
			throw runtime_error(oss.str());
		}
		if (call_fork && setpriority(PRIO_PROCESS, 0, 10) != 0)
		{
			ostringstream oss;
			oss<<"run_crawler():setpriority() error,"<<strerror(errno);
			throw runtime_error(oss.str());
		}
		exec_crawler(env, debug);
		assert(false);
		exit(-1);
	}
	else if (pid < 0)
	{
		cout<<"fork failed!"<<endl;
		return -4;
	}
	else if (pid > 0)
		return pid;
	return -1;
}

int 
CScheduler::expose(const string& fileExport)
{
	if ( 0 != rename((pathExport+"/."+fileExport).c_str()
	   , (pathExport+"/"+fileExport).c_str()) )
	{
		ostringstream oss;
		oss<<"CScheduler::expose():Can't rename file: "
		  <<pathExport<<"/."<<fileExport<<": "
		  <<strerror(errno);
		throw runtime_error(oss.str());
	}
	string command = "gzip "+pathExport+"/"+fileExport;
	int res = system(command.c_str());
	if (res == -1 || res == 127)
	{
		ostringstream oss;
		oss<<"CScheduler::expose:system(\""<<command<<"\") failed.";
		throw runtime_error(oss.str());
	}
	return 0;
}

int
CScheduler::run()
{
	this->pid = getpid();
	while (true)
	{
		bool busy = false;
		int res_start;
		while ((res_start=startCrawler())>0)
			busy = true;
		while (cleanCrawler()>0)
			busy = true;
		if (pool.empty() && res_start==-2)
			break;
		if (!busy)
		{
			for (int i=0; i<6; i++)
			{
				cout<<".Z"<<flush;
				sleep(15);
			}
		}
	}
	cout<<"Scheduler END."<<endl;
	return 0;
}

int
CScheduler::append(const string &from, const string &to, const string &head, const string &tail)
{
	if (file_size(from) > 0)
	{
		ifstream ifs(from.c_str());
		ofstream ofs(to.c_str(), ios::out|ios::app);
		if (!(ofs<<head<<ifs.rdbuf()<<tail))
		{
			ostringstream oss;
			oss<<"CScheduler::append():failed:"<<from<<"==>"<<to;
			throw runtime_error(oss.str());
		}
		return 0;
	}
	return -1;
}
