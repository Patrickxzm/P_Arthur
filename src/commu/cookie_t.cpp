#include "cookie.h"
#include "util/arg.h"
#include <iostream>

using namespace std;

ostream&
help(ostream &os)
{
	os<<"Testing the cookie utility.\n"
	  "\tUsage: Cmd [--file=...] (--url=... | --set=...) [-h|--help] \n"
	  "\t\t --file= : file name to load/save cookies.(default: .cookies)\n"
	  "\t\t --url= : url which need cookies.\n"
	  "\t\t --set= : cookies to set.\n"
	  "\t\t -h|--help : print this message.\n"<<endl;
	return os;
}

int
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (!arg.find1("-h").null() || !arg.find1("--help").null())
	{
		help(cout);
		return 1;
	}
	CArg::ArgVal val = arg.find1("--file");
	string fn;
	if (!val.null())
		fn = string(val);
	else
		fn = ".cookies";
	CCookies coos(fn.c_str());
	val = arg.find1("--url=");
	if (!val.null())
	{
		string url = string(val);
		vector<const CCookie*> m = coos.match(url);
		if (m.empty())
		{
			cout<<"no cookie."<<endl;
			return 2;
		}
		for (unsigned i=0; i<m.size(); i++)
		{
			cout<<*m[i]<<endl;
		}
		return 3;
	}
	val = arg.find1("--set=");
	if (!val.null())
	{
		string coo = string(val);
		coos.add(CCookie(coo));
		coos.save(fn.c_str());
		return 4;
	}
	
	cerr<<"Please specify either \"--set=\" or \"--url=\" to run this program."<<endl;
	return -1;
}
