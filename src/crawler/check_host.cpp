#include <iostream>
#include "util/arg.h"
#include "util/shadow.h"
#include "commu/tw_raw.h"
#include "commu/http.h"
#include "htmlstruct/htmlref.h"
#include <mysql++.h>
using namespace mysqlpp;
using namespace std;

ostream&
help(ostream& os)
{
	os<<"Test the hostports to see whether they are crawlable by fetching their root URLs.\n"
	  "\tUncrawlable may be in the following reason: \n"
	  "\t\tRedirect away;\n"
	  "\t\tNo page; \n"
	  "\t\tDNS Failure; \n"
	  "\t\tConnect Failed; \n"
	  "\t\tTransfer error; \n"
	  "\t\tNone text; \n"
	  "\t\tNone Link; \n"
	  "\n"
	  "\tUsage: Cmd [--hostport= ] [--help|-h] \n"
	  "\t\t--hostport=fn : hostport filename. (default: cin)\n"
	  "\t\t--help|-h : print this message.\n"
	  "\t\tcin : input hostport(s) without \"--input=\" option.\n" 
	  "\t\t\t| input  | OK    | fail    | raw pages | \n"
	  "\t\t\t| cin    | cout  | cerr    | pages.raw | \n"
	  "\t\t\t| fn     | fn.OK | fn.fail | fn.raw    | \n"
	  <<endl;
	return os;
}

namespace {
	bool check_page(CHttp &http, ostream& OK, ostream &fail, ostream &raw);
};

int
main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	if (arg.find1("--help") || arg.find1("-h"))
	{
		help(cout);
		return 1;
	}
	string fn;
	CArg::ArgVal val;
	if (val=arg.find1("--hostport="))
		fn = val;
	auto_ptr<istream> myin;
	auto_ptr<ostream> myOK, myFail, myRaw;

	if (fn.empty())
	{
		myin = auto_ptr<istream>(new istream(cin.rdbuf()));
		myOK = auto_ptr<ostream>(new ostream(cout.rdbuf()));
		myFail = auto_ptr<ostream>(new ostream(cerr.rdbuf()));
		myRaw = auto_ptr<ostream>(new ofstream("pages.raw"));
	}
	else
	{
		myin = auto_ptr<istream>(new ifstream(fn.c_str()));
		myOK = auto_ptr<ostream>(new ofstream((fn+".OK").c_str()));
		myFail = auto_ptr<ostream>(new ofstream((fn+".fail").c_str()));
		myRaw = auto_ptr<ostream>(new ofstream((fn+".raw").c_str()));
	}
	if (!myin->good())
	{
		cerr<<"Can not open input istream."<<endl;
		return -1;
	}
	if (!myOK->good())
	{
		cerr<<"Can not open output ostream for OK hostport."<<endl;
		return -2;
	}
	if (!myFail->good())
	{
		cerr<<"Can not open output ostream for Fail hostport."<<endl;
		return -3;
	}
	if (!myRaw->good())
	{
		cerr<<"Can not open output ostream for raw pages."<<endl;
		return -4;
	}

	string hostport;
	while (*myin >> hostport)
	{
		string urlstr("http://"+hostport+"/");
		CURL target(urlstr);
		if (target.host().length() > 64)
		{
			cerr<<"host str too long(>64): "<<target.host()<<endl;
			continue;
		}
		CHttp http(urlstr);
		http.timeout(30, 30);
		CHttp::result_t res = http.fetch();
		switch (res)
		{
		case CHttp::OK :
			check_page(http, *myOK, *myFail, *myRaw);
			break;
#if 0
			if (http.get_status()>=200 && http.get_status()<300)
			{
				CURL result(http.get_location());
				if (result.hostport() != target.hostport())
				{
					fail<<hostport<<" Redirect away: "<<result.hostport()<<endl;
					break;
				}
				if (http.reply.headers.mtype() != m_text)
				{
					fail<<hostport<<" None text"<<endl;
					break;
				}
				CHTMLRef htmlref(http.reply.body, http.
				string refresh = refresh_meta(http);
				if (!refresh.empty())
				{
					CURL r(refresh);
					if (r.hostport() != target.hostport())
					{
						fail<<hostport<<" Redirect away: "
						   <<r.hostport()<<endl;
						break;
					}
				}
				assert(http.get_ipaddress());
				OK<<hostport<<" "<<http.get_ipaddress()<<endl;
				break;
			}
			fail<<hostport<<" no page, status="<<http.get_status()<<endl;
			break;
#endif //0
		case CHttp::DNSFailure :
			*myFail<<hostport<<" DNS Failure"<<endl;
			break;
		case CHttp::ConnectFailure :
			*myFail<<hostport<<" Connect Failed"<<endl;
			break;
		case CHttp::TransferError :
			*myFail<<hostport<<" Transfer error"<<endl;
			break;
		case CHttp::OverRedirect :
		case CHttp::ProtocolError :
			*myFail<<hostport<<" Redirect away to none-http"<<endl;
			break;
		default :
			*myFail<<hostport<<'('<<res<<')'<<endl;
			assert(false);
		}
	}
	return 0;
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

namespace {
bool 
check_page(CHttp &http, ostream& OK, ostream &fail, ostream &raw)
{
	raw<<CTWRaw(http.location, http.ipstr, http.reply)<<endl;
	int status = http.get_status();
	string hostport = CURL(http.urlstr).hostport();
	if (status >= 200 && status < 300)
	{
		if (http.location != http.urlstr)
		{
			string hostport1 = CURL(http.location).hostport();
			if (hostport != hostport1)
			{
				fail << hostport<<" Redirect away: "<< hostport1<<endl;
				return false;
			}
		}
		if (http.reply.headers.mtype() != m_text)
		{
			fail<<hostport<<" None text"<<endl;
			return false;
		}

		int len;
		const char* body = http.get_body(len);
		CHTMLRef htmlref(body, len, http.location);
		vector<CRef> links=htmlref.links();
		bool fLink = false;
		for (size_t i=0; i<links.size(); i++)
		{
			string urlstr = links[i].url();
			CURL url(urlstr);
			if (url.hostport() != hostport)
				continue;
			if (urlstr != http.urlstr && urlstr != http.location)
			{
				fLink = true;
				break;
			}
		}
		if (!fLink)
		{
			fail<<hostport<<" None Link"<<endl;
			return false;
		}
		assert(http.get_ipaddress());
		OK<<hostport<<" "<<http.get_ipaddress()<<endl;
		return true;
	}
	fail<<hostport<<" no page, status="<<status<<endl;
	return false;
}
}; // namespace
