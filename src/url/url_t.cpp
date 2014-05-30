#include "url.h"
#include "util/arg.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <memory>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

using namespace std;

ostream& 
help(ostream& os)
{
	os<<"URL parsing program, output all infomation about this URL by default.\n"
	"\tUsage: Cmd [--base=...] [--format=... [--sep=] ] [-h|--help]\n"
	"\t\t--base= : base url to parse relative URL string.\n"
	"\t\t--format= : Format string to select infomation for the output.\n"
	"\t\t\t 's' means site, 'h' means host, 'p' means port.\n"
	"\t\t--sep= : seperate string for each value in output. (default:'\\t')\n"
	"\t\t-h|--help : print this message.\n"
	"\t\tcin : url string.\n"
	"\t\tcout : CURL content.\n"
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
	CArg::ArgVal val; 
	auto_ptr<CURL> base;
	if (val=arg.find1("--base="))
		base = auto_ptr<CURL>(new CURL(string(val)));
	string format;
	if (val=arg.find1("--format="))
		format = val;
	string sep("\t");
	if (val=arg.find1("--sep="))
		sep = val;

	string urlstr;
	while (getline(cin, urlstr))
	{
		CURL url(urlstr, base.get());
		if (format.size()>0)
		{
			for (unsigned i=0; i<format.size(); i++)
			{
				switch(format[i]) 
				{
				case 's':
					cout<<url.site()<<sep;
					break;
				case 'h':
					cout<<url.host()<<sep;
					break;
				case 'p':
					cout<<url.port()<<sep;
					break;
				default:
					cerr<<"Unknown char for format: "<<format[i]<<endl;
					return -1;
				}
			}
			cout<<endl;
			continue;
		}
		else {
			//normal output
			cout<<url.str()<<endl;
			url.explain(cout);
			cout<<"site(): "<<url.site()<<endl;
			cout<<"localpath(): "<<url.localpath()<<endl;
			cout<<"mtype(): "<<url.mtype()<<endl;
			cout<<"ext(): "<<url.local_ext()<<endl;
		}
	}
	return 0;
}

