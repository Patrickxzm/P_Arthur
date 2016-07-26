#include "mosquito.h"
#include "url/url.h"
#include "util/xmlfile.h"
#include "util/arg.h"
#include "util/util.h"
#include <iostream>
using namespace std;

ostream&
help(ostream& os)
{
	os<<"Crawl a host. \n"
	"Usage: Cmd (--host= [--port= ] --lpath-file=)|--root= (--save-link=)* \n"
	"\t--visited-prefix= [--visited-capacity=] [--cookie-file=] [--debug]\n"
	"\t--npages=  [--interval=] [--retry-interval=] [--silence|--cout= ] \n"
	"\t[--config=] [-h|--help]\n"
	"\t\t--host= : name of the host you want to crawl \n"
	"\t\t--port= : port of the web service on this host. [default:80] \n"
	"\t\t--lpath-file = : local paths in URL strings on this host to crawl. \n"
	"\t\t\t{ lpath [towait:-1] [waited:0] [new_url_sum:0] }* \n"
	"\t\t--root= : a root url.\n"
	"\t\t--save-link= : save urls of this host, if found.\n"
	"\t\t--visited-prefix= : name prefix of the \"visited\" bitmap files.\n"
	"\t\t--visited-capacity= : if no shadows exist, create with the capacity.[default:1000]\n"
	"\t\t--cookie-file= : name of file to load/save cookies.\n"
	"\t\t--debug : Prompt and output more for debug. \n"
	"\t\t--npages= : Number of pages to download, or 10*npages to try.\n"
	"\t\t--interval= : Number of seconds waited between two HTTP requests. [default: 1]\n"
	"\t\t\t\t\t * max(robots.crawl_delay, interval) is used at last.\n"
	"\t\t--retry-interval= : Number of seconds waited after a http failure before next retry. [default: 600]\n"
	"\t\t--silence : no output.\n"
	"\t\t--cout= : redirect the cout to a file.\n"
	"\t\t--config= : name of xml config file, /etc/pa.cnf will be used before it.\n"
	"\t\t\tcontents in config file can be overwritten by command line argument.\n"
	"\t\t-h|--help : print this message.\n"
	  <<endl;
	return os;
}

int
readFromArg(const CArg& arg, CMosquito::CEnvCrawler &env)
{
	string strURL, fn;
	if (arg.findLast("--root=", strURL))
	{
		CURL urlroot(strURL);
		env.host = urlroot.host();
		env.port = urlroot.port();
		env.tasks.push_back(CTask(urlroot.localstr(), 0, 0, CTask::Unknown));
	}
	else if (arg.findLast("--host=", env.host) && arg.findLast("--lpath-file=", fn))
	{
		ifstream ifs(fn.c_str());
		CTask task;
		while (ifs>>task)
			env.tasks.push_back(task);
		string strNum;
		if (arg.findLast("--port=", strNum))
			env.port = stoi(strNum);
		else
			env.port = 80;
	}
	else
	{
		cerr<<"\"(--host= [--port= ] --lpath-file=)|--root=\" is required."
		   <<endl;
		return -1;
	}
	vector<string> target = arg.find("--save-link=");
	for (unsigned i=0; i<target.size(); i++)
	{
		string hostport = target[i];
		string host;
		int port;
		CURL::split("http", hostport, host, port);
		if (port == 80)
			hostport = host;
		env.save_link_of.insert(hostport);
	}
	if (!arg.findLast("--visited-prefix=", env.shadow_prefix)
                || env.shadow_prefix.empty())
	{
		cerr<<"a non-empty prefix is required. By \"--visited-prefix=\"."
		   <<endl;
		return -3;
	}
        int num;
	if (arg.findLastInt("--visited-capacity=", num) && num>0)
		env.shadow_capacity = num;
	else
		env.shadow_capacity = 1000; //default value
	arg.findLast("--cookie-file=", env.cookie_file);
	if (!arg.findLastInt("--npages=", env.pageNum))
	{
		cerr<<"How many pages wanted? Specify it by \"--npages=\"."<<endl;
		return -6;
	}
	if (!arg.findLastInt("--interval=", env.interval))
		env.interval = 1; //default value
	if (!arg.findLastInt("--retry-interval=", env.retry_interval))
		env.retry_interval = 600;
	return 0;
}

int
main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	if (arg.found("--help") || arg.found("-h"))
	{
		help(cout);
		return 1;
	}
	if (arg.found("--debug"))
	{
		cout<<"Please debug me!(pid="<<getpid()<<")"<<endl;
                cout<<"input 'y' to continue..."<<endl;
                string c;
                while (cin>>c && c!="y")
                    ;
	}

#if 0 // pseudo code
                  mosquito.readConfig("/etc/pa.cnf");
		  mosquito.readConfig(arg.find1("--config="));
		  readFromArg(arg, env);
		  mosquito.loadEnv(env);
#endif //0
	CMosquito::CEnvCrawler env;
	string fn;
	if (arg.found("--silence"))
		env.my_log.reset();
	else if (arg.findLast("--cout=", fn))
		env.my_log = auto_ptr<ostream>(new ofstream(fn.c_str()));
	else
		env.my_log = auto_ptr<ostream>(new ostream(cout.rdbuf()));
	env.my_error = auto_ptr<ostream>(new ostream(cerr.rdbuf()));
	if (env.my_log.get())
	{
		for (int i=0; i<argc; i++)
			(*env.my_log)<<argv[i]<<' ';
		(*env.my_log)<<endl;
	}

	CMosquito mosquito;
	if (arg.found("--debug"))
		mosquito.setDebug(true);
	CXMLFile xfile;
	if (xfile.open("/etc/pa.cnf")==0)
	{
		mosquito.readConfig(xfile);
		if (env.my_log.get())
			(*env.my_log)<<"Read config file \"/etc/pa.cnf\"."<<endl;
	}
	if (arg.findLast("--config=", fn) && xfile.open(fn.c_str())==0)
	{
		mosquito.readConfig(xfile);
		if (env.my_log.get())
			(*env.my_log)<<"Read config file \""<<fn<<"\"."<<endl;
	}
	if (readFromArg(arg, env) != 0)
		return -1;
	mosquito.loadEnv(env);
	mosquito.run();
	mosquito.report();
	return 0;
}
catch (std::exception &e)
{
	cerr<<"Catch std::exception: "<<e.what()<<endl;
	return -6;
}
