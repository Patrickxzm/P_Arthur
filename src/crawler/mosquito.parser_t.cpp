#include "mosquito.h"
#include "commu/tw_raw.h"
#include "util/arg.h"
#include "util/util.h"
#include <iostream>

using namespace std;

ostream& 
help(ostream &os)
{
	os<<"Test CMosquito::parse() with tw raw pages\n"
	  "Usage: cmd [--host=... [--port=...]] (--save-link=)*\n"
	  "\t\t --host=, --port= : ... (default: host, port of the first page.)\n"
	  "\t\t --save-link= : save urls of this host, if found.\n"
	  "\t\t cin : TianWang raw pages.\n"
	  "\t\t cout : result.\n"
	  <<endl;
	return os;
}

int
mosquito_parser_t_main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (arg.found("--help") || arg.found("-h"))
	{
		help(cout);
		return 1;
	}

	CMosquito mosquito;
	mosquito.my_log = auto_ptr<ostream>(new ostream(cout.rdbuf()));
	mosquito.my_error = auto_ptr<ostream>(new ostream(cerr.rdbuf()));
    mosquito.setDebug(true);
	mosquito.robots.init();
	
	vector<CArg::ArgVal> target = arg.find("--save-link=");
	for (unsigned i=0; i<target.size(); i++)
	{
		string hostport = target[i].get();
		string host;
		int port;
		CURL::split("http", hostport, host, port);
		if (port == 80)
			hostport = host;
		mosquito.outlink_save.insert(
		   make_pair(hostport, CMosquito::outlink_save_t::mapped_type()));
		//env.save_link_of.insert(hostport);
#if 0
		string ss(target[i]);
		istringstream iss(ss);
		string host;
		int port;
		getline(iss, host, ':');
		if (!(iss>>port))
			port = 80;
		mosquito.saveLinkOf(host, port);
#endif //0
	}

	string host;
	int port;
	CArg::ArgVal val;
	if (val=arg.find1("--host="))
	{
		host = string(val);
		if (val=arg.find1("--prot="))
			port = val.INT();
		else
			port = 80;
		mosquito.shadows.open("visited", 100000);
	}
	CTWRaw raw;
	while (cin>>raw)
	{
		CURL &current = mosquito.current;
		current.init(raw.url);
		if (host.empty())
		{
			host = current.host();
			port = current.port();
			mosquito.shadows.open("visited", 100000);
		}
		else if (host != current.host() || port != current.port())
		{
			continue;
		}
		page_status_t status;
		int reply_status = mosquito.CAnalysisPart::check(raw.reply);
		assert(reply_status==CMosquito::PAGE_OK 
		   || reply_status==CMosquito::PAGE_REDIRECT);
		status = mosquito.parse(raw.reply, reply_status);
		cout<<raw.url<<endl;
		if (status.newURL> 0)
			cout<<status.newURL<<" new urls found; ";
		if (status.newAnchor > 0)
			cout<<status.newAnchor<<" new anchors found; ";
		if (status.newParagraph > 0)
			cout<<status.newParagraph<<" new paragraph found; ";
		if (status.outlinkSaved > 0)
			cout<<status.outlinkSaved<<" outlinks saved; ";
		if (status.newScript)
			cout<<"new javascript; ";
		if (status.newPlain)
			cout<<"new plain text; ";
		if (!status.newScript && !status.newPlain 
		   && status.newTextNum()+status.outlinkSaved == 0)
			cout<<"useless! ";
		cout<<endl;
	}
	return 0;
}

int
main(int argc, char* argv[])
{
	return mosquito_parser_t_main(argc, argv);
}

