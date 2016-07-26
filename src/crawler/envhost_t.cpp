#include "envhost.h"
#include "scheduler.h"
#include "url/url.h"
#include "util/arg.h"
#include "util/util.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>

using namespace std;

ostream& 
help(ostream& os)
{
	os<<"Test program for CHostTable class, this program can save/load a host's "
	  "crawling environment to/from a remote database\n"
	"Usage: cmd (\t--save [--path=]\n"
	"\t\t| --load --host= [--port=] [--crawl] [--mark] \n"
	"\t\t| --import [--force] ( --host-file= | --host= [--port=] ) ) \n" 
	"\t--db-host= --db-name= --db-user= --db-pass= "
	"[--help|-h] \n"
	"\t --save : save crawling infomation to database.\n"
	"\t --load : load crawling infomation from database to \"./hostport\".\n"
	"\t --path= : environment directory to save. [default: ./]\n"
	"\t --host= : name of host to crawle. \n"
	"\t --port= : port on which the httpd service is provided.[default:80]\n"
	"\t --crawl : run a crawler on the env just loaded on debug purpose. [default:NOT]\n"
	"\t --mark : lock the host record in database with machine's name. [default:NOT]\n"
	"\t --import : import host for later crawling.\n"
	"\t --force : import host ignore max_host_num limit in site table.\n"
	"\t --host-file= : file containing hosts to import, in the CHost4Import format: \n\n"
	<<CHost4Import::explain()
	<<"\n"
	"\t --db-host= : database server address.\n"
	"\t --db-name= : database name containing crawling infomation.\n"
	"\t --db-user= : database user name.\n"
	"\t --db-pass= : database password.\n"
	"\t --help|-h : print this message.\n"
	<<endl;
	return os;
}


int
import(CHostTable &hostTable, const string &host_fn, bool force)
{
	ifstream ifs_host(host_fn.c_str());
	if (!ifs_host)
	{
		ostringstream oss;
		oss<<"Cann't open input host_fn file:\""<<host_fn<<"\".";
		throw runtime_error(oss.str());
	}
	CHost4Import host;
	while (ifs_host >> host)
		hostTable.import(host, &cout, force);
	return 0;
}

int
main(int argc, char* argv[])
try {
	//return run_mysql_test();
	CArg arg(argc, argv);
	if (arg.found("--help") || arg.found("-h"))
	{
		help(cout);
		return 1;
	}
	string db_host;
	arg.findLast("--db-host=", db_host);
	string db_name;
	arg.findLast("--db-name=", db_name);
	string db_user;
	arg.findLast("--db-user=", db_user);
	string db_pass;
	arg.findLast("--db-pass=", db_pass);
	Connection conn;	
	if (!conn.connect(db_name.c_str(), db_host.c_str(), db_user.c_str()
	  , db_pass.c_str()))
	{
		cout<<"Connect envhost database failed!"<<endl;
		return -1;
	}
	CHostTable hostTable(&conn);
	
	string host;
	arg.findLast("--host=", host);
	int port=CURI::default_port("http");
	arg.findLastInt("--port=", port);
	if (arg.found("--save"))
	{
		string path;
		if (!arg.findLast("--path=", path))
			path = "./";
		switch (hostTable.saveResult(path))
		{
		case 0:
			cout<<"save OK"<<endl;
			return 0;
		case -1: 
			cout<<"connect database server failed!"<<endl;
			return -1;
		case -2: 
			cout<<"compress roots failed!"<<endl;
			return -2;
		case -3:
			cout<<"no db rows to update."<<endl;
			return -3;	
		default:
			assert(false);
		}
	}
	else if (arg.found("--load"))
	{
		if (host.empty())
		{
			cerr<<"Please input a host name with \"--host=\" option."<<endl;
			return -4;
		}
		bool mark = arg.found("--mark");
		string path = "./"+CURL::hostport("http", host, port);
		if (0 != mkdir(path.c_str(), 0770) && errno != EEXIST)
			cerr<<"Can not mkdir \""<<path<<"\":"<<strerror(errno)<<endl;
		CEnv env;
		if (hostTable.loadStartEnv(env, path.c_str(), host, port, mark))
			cout<<"load OK"<<endl;
		else
		{
			cout<<"load failed"<<endl;
			return -3;
		}
		if (arg.found("--crawl"))
			return CScheduler::run_crawler(env, true, true);
		return 0;
	}
	else if (arg.found("--import"))
	{
		string host_fn;
		bool force = arg.found("--force");
		if (arg.findLast("--host-file=", host_fn))
			return import(hostTable, host_fn, force);
		else if (!host.empty())
		{
			CHost4Import import;
			import.host = host;
			import.port = port;
			hostTable.import(import, &cout, force);
			return 0;
		}
		else
		{
			cerr<<"Please specify \"--host-fn=\" or \"--host=\" to import."<<endl;
			return -5;
		}
	}
	else {
		cerr<<"Please specify your operation, \"--save\", \"--load\" "
		  "or \"--import\""<<endl;
		return -10;
	}
	assert(false);
}
catch (std::exception &e)
{
	cerr<<"Catch std::exception, "<<e.what()<<endl;
	return -5;
}
catch (...)
{
	cerr<<"Catch other exception."<<endl;
	return -6;
}
