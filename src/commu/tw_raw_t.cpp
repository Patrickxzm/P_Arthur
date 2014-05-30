#include "tw_raw.h"
#include "util/arg.h"
#include "util/util.h"
#include "http_reply.h"
#include <iostream>

using namespace std;
ostream&
help(ostream &os)
{
	os<<"Tool for reading CTW_Raw pages and output them.\n"
	  "\tUsage: cmd [--prefix=...]* [--select=...] [--num=...] \n"
	  "\t\t  [--one-by-one] [--package] \n"
	  "\t\t  [--http-reply | --body-only | --list-only] [-h|--help]\n"
	  "\t\t--prefix= : just select urls with this prefix.\n"
	  "\t\t--select= : filename of the url list to output.\n"
	  "\t\t--num= : output num pages at most.\n"
	  "\t\t--one-by-one : instead of cout, write output to files with prefix: \n"
	  "\t\t\t\t \"1.\", \"2.\", \"3.\", ...\n"
	  "\t\t\t\t suffix will be raw, html, or reply\n"
	  "\t\t--package : output record in package:\"length\\n body\\n\"\n" 
	  "\t\t--http-reply : output is in http-reply format, tw_raw headers are discarded.\n"
	  "\t\t--body-only : only http body is output.\n"
	  "\t\t--list-only : only list url and date of raw page.\n"
	  "\t\t-h|--help : print this message.\n"
	  "\t\tcin : CTWRaw pages.\n"
	  "\t\tcout : output if \"--one-by-one\" option is not set.\n"
	  <<endl;
	return os;
}

int 
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (arg.find1("-h") || arg.find1("--help"))
	{
		help(cout);
		return 1;
	}
	vector<string> prefixes;
	vector<CArg::ArgVal> vals = arg.find("--prefix=");
	for (unsigned i=0; i<vals.size(); i++)
	{
		if (strlen(vals[i]) > 0)
			prefixes.push_back(string(vals[i]));
	}
	int num=0;
	CArg::ArgVal val;
	set<string> selects;
	if (val = arg.find1("--select="))
	{
		ifstream ifs(val);
		if (!ifs)
		{
			cerr<<"Can not open select file:\""<<(const char*)val<<"\"."<<endl;
			return -6;
		}
		string urlstr;
		while (ifs>>urlstr)
			selects.insert(urlstr);
	}
	if (val=arg.find1("--num="))
		num = val.INT();
	bool oneByOne = arg.found("--one-by-one");
	bool body_only = arg.found("--body-only");
	bool http_reply = arg.found("--http-reply");
	bool list_only = arg.found("--list-only");
	bool package = arg.found("--package");
	if (oneByOne && list_only || package && oneByOne 
	   || package && list_only) 
	{
		cerr<<"options \"--one-by-one\", \"--list-only\""
		  ", \"--package\" is conflict with each other"
		  <<endl;
		return -1;
	}
	CTWRaw raw;
	for (unsigned nloop=0, i=0; (num<=0 || (int)i<num) && cin>>raw; nloop++) 
	{
		if (prefixes.size() > 0 || selects.size() > 0)
		{
			bool match = false;
			for (size_t i=0; i<prefixes.size(); i++)
			{
				if (raw.url.compare(0, prefixes[i].length(), prefixes[i]) == 0)
				{
					match = true;
					break;
				}
			}
			if (!match && selects.find(raw.url)==selects.end())
				continue;
		}
		i++;

		if (list_only)
		{
			cout<<i<<": "<<raw.url<<'\n'<<raw.date<<'\n'<<endl;
			continue;
		}
		ostream os(cout.rdbuf());
		ofstream ofs;
		if (oneByOne)
		{
			if (body_only)
			{
				string ext = raw.reply.headers.ext();
				if (ext.empty())
					ext = CURL(raw.url).local_ext();
				if (ext.empty())
					ext = "html";
				ofs.open((tostring(i)+"."+ext).c_str());
			}
			else if (http_reply)
				ofs.open((tostring(i)+".reply").c_str());
			else
				ofs.open((tostring(i)+".raw").c_str());
			os.rdbuf(ofs.rdbuf());
		}
		if (package)
		{
			if (body_only)
			{
				os<<raw.reply.body.length()<<'\n'
				   <<raw.reply.body<<endl;
				continue;
			}
			ostringstream oss;
			if (http_reply)
				oss<<raw.reply;
			else
				oss<<raw;
			os<<oss.str().length()<<'\n'<<oss.str()<<endl;	
			continue;
		}
		if (body_only)
			os<<raw.reply.body<<endl;
		else if (http_reply)
			os<<raw.reply<<endl;
		else
			os<<raw<<endl;
	}
	return 0;
}
