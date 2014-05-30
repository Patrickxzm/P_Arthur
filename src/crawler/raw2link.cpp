#include "commu/tw_raw.h"
#include "util/arg.h"
#include "commu/http_reply.h"
#include "htmlstruct/htmlref.h"
#include "links.h"
#include <iostream>

using namespace std;

ostream& 
help(ostream &os)
{
	os<<"Parse link infomation from web pages.\n"
	"Usage: Cmd \n"
	"\tstdin: web pages in Tianwang raw format.\n"
	"\tstdout: link file.\n"
	  <<endl;
	return os;
}

int 
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (!arg.find("--help").empty() || !arg.find("-h").empty())
	{
		help(cerr);
		return 1;
	}
	CTWRaw raw;
	while (cin>>raw)
	{
		CPageLink link(raw.url);
		CURL base(raw.url);
		set<string> outlinks;
		CHTMLRef htmlref(raw.reply.body.c_str(), raw.reply.body.size(), raw.url);
		for (auto_ptr<CRef> ref = htmlref.refer(); ref.get(); ref=htmlref.refer())
		{
			CURL newurl(ref->url());
			if (newurl.isAbs()
			  && newurl.host() == base.host() 
			  && newurl.port() == base.port()
			  && (newurl.mtype() == m_text || newurl.mtype() == m_unknown)
			)
				outlinks.insert(newurl.str());
		}
		for (set<string>::const_iterator cit = outlinks.begin()
		  ; cit != outlinks.end(); cit ++)
		{
			link.outlinks.push_back(*cit);
		}
		cout<<link<<endl;
	}
	if (!cin.eof())
	{
		cerr<<"Still more input data, make sure it is OK."<<endl;
		return -2;
	}
	return 0;
}
