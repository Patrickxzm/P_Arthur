#include "links.h"
#include "util/arg.h"
#include <iostream>
using namespace std;

void help(ostream &os)
{
	os<<"Convert url list from link data.\n"
	"Usage: Cmd [-h|--help]\n"
	"stdin: link data.\n"
	"stdout: url list.\n"
	"-h|--help: print this message.\n"
	<<endl;
}

int
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (!arg.find("-h").empty() || !arg.find("--help").empty())
	{
		help(cerr);
		return -1; 
	}
	CPageLink link;
	while (cin>>link)
	{
		cout<<link.url<<endl;
	}
	return 0;
}
