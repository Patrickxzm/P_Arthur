#include "scheduler.h"
#include "util/arg.h"
#include "util/util.h"
#include <fstream>
#include <iostream>

using namespace std;

ostream& 
help(ostream& os)
{
	os<<"Scheduler program to run host_crawler.\n"
	"Usage: cmd --export-dir= --db-host= --db-name= --db-user= --db-pass= "
	"[--collect-outlinks] [--collect-discard] [--collect-overflow] [--help|-h] \n"
	"\t --export-dir= : raw page export directory for Web InfoMall.\n"
	"\t --db-host= : database server address.\n"
	"\t --db-name= : database name containing crawling infomation.\n"
	"\t --db-user= : database user name.\n"
	"\t --db-pass= : database password.\n"
	"\t --collect-outlinks : collect outlinks files from each host crawler.\n"
	"\t --collect-discard : collect discard files from each host crawler.\n"
	"\t --collect-overflow : collect overflow files from each host crawler.\n"
	"\t --help|-h : print this message.\n"
	"More info: create a keep_the_env file to reserve a host_crawler's environment.\n"
	"Changelog: When ServerFailure, don't change its zip_root. 11/01/2009\n"
	<<endl;
	return os;
}

int 
main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	if (arg.find("--help").size()>0 || arg.find("-h").size()>0 )
	{
		help(cout);
		return 1;
	}
	CArg::ArgVal val;
	if (!(val=arg.find1("--export-dir=")))
	{
		cerr<<"Please specify the export dir for raw web page."<<endl;
		return -1;
	}
	string pathExport(val);
	string db_host;
	if (val=arg.find1("--db-host="))
		db_host=string(val);
	string db_name;
	if (val=arg.find1("--db-name="))
		db_name=string(val);
	string db_user;
	if (val=arg.find1("--db-user="))
		db_user=string(val);
	string db_pass;
	if (val=arg.find1("--db-pass="))
		db_pass=string(val);
	Connection conn;
	if (!conn.connect(db_name.c_str(), db_host.c_str(), db_user.c_str()
	  , db_pass.c_str()))
	{
		cout<<"Connect envhost database failed!"<<endl;
		return -1;
	}
	auto_ptr<CHostTable> hostTable(new CHostTable(&conn));
	CScheduler queen(hostTable, &conn, pathExport);
	if (arg.find1("--collect-outlinks"))
		queen.collect_outlinks=true;
	if (arg.find1("--collect-overflow"))
		queen.collect_overflow=true;
	if (arg.find1("--collect-discard"))
		queen.collect_discard=true;
	return queen.run();
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
