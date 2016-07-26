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
	if (arg.found("--help") || arg.found("-h"))
	{
		help(cout);
		return 1;
	}
	string pathExport;
	if (!arg.findLast("--export-dir=", pathExport))
	{
		cerr<<"Please specify the export dir for raw web page."<<endl;
		return -1;
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
		cout<<"Connect envhost database "<<db_name<<"@"<<db_host<<" with "
                   <<db_user<<"/****** failed!"<<endl;
		return -1;
	}
	auto_ptr<CHostTable> hostTable(new CHostTable(&conn));
	CScheduler queen(hostTable, &conn, pathExport);
	queen.collect_outlinks=arg.found("--collect-outlinks");
	queen.collect_overflow=arg.found("--collect-overflow");
	queen.collect_discard=arg.found("--collect-discard");
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
