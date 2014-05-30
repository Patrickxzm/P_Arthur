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
	CArg::ArgVal val, val1;
	if (val = arg.find1("--root="))
	{
		CURL urlroot(val.get());
		env.host = urlroot.host();
		env.port = urlroot.port();
		env.tasks.push_back(CTask(urlroot.localstr(), 0, 0, CTask::Unknown));
	}
	else if ((val = arg.find1("--host=")) && (val1 = arg.find1("--lpath-file=")))
	{
		env.host = val;
		ifstream ifs(val1);
		CTask task;
		while (ifs>>task)
			env.tasks.push_back(task);
		if (val = arg.find1("--port="))
			env.port = val.INT();
		else
			env.port = 80;
	}
	else 
	{
		cerr<<"\"(--host= [--port= ] --lpath-file=)|--root=\" is required."
		   <<endl;
		return -1;
	}
	vector<CArg::ArgVal> target = arg.find("--save-link=");
	for (unsigned i=0; i<target.size(); i++)
	{
		string hostport = target[i].get();
		string host;
		int port;
		CURL::split("http", hostport, host, port);
		if (port == 80)
			hostport = host;
		env.save_link_of.insert(hostport);
	}
	if ((val = arg.find1("--visited-prefix=")) && *val.get() != '\0')
		env.shadow_prefix = val;
	else {
		cerr<<"a non-empty prefix is required. By \"--visited-prefix=\"."
		   <<endl;
		return -3;
	}
	if (val = arg.find1("--visited-capacity="))
		env.shadow_capacity = val.INT();
	else
		env.shadow_capacity = 1000; //default value
	if (val = arg.find1("--cookie-file="))
		env.cookie_file = val;
	if (val = arg.find1("--npages="))
		env.pageNum = val.INT();
	else {
		cerr<<"How many pages wanted? Specify it by \"--npages=\"."<<endl;
		return -6;
	}
	if (val = arg.find1("--interval="))
		env.interval = val.INT();
	else
		env.interval = 1; //default value
	if (val = arg.find1("--retry-interval="))
		env.retry_interval = val.INT();
	else
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
	CArg::ArgVal val;
	CMosquito::CEnvCrawler env;
	if (arg.found("--silence"))
		env.my_log.reset();
	else if (val=arg.find1("--cout="))
		env.my_log = auto_ptr<ostream>(new ofstream(val));
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
	if ((val=arg.find1("--config=")) && xfile.open(val)==0)
	{
		mosquito.readConfig(xfile);
		if (env.my_log.get())
			(*env.my_log)<<"Read config file \""<<val<<"\"."<<endl;
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

#if 0 //paste_board
	mosquito.loadEnv(host, port, lpaths, visited_prefix.c_str(), visited_capacity
		, cookie_file, npages, interval);

	int ret = readFromArg(arg, config);
	if (ret != 0)
		return ret;
#endif //0
