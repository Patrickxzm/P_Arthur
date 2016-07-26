#include "util/arg.h"
#include "tag.h"
#include "comment.h"
#include "text.h"
#include "encapsuled.h"
#include "htmllexic.h"
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

void help();

int 
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (arg.found("-h") || arg.found("--help"))
	{
		help();
		return 1;
	}

	string htmlpage;
	string fn;
	if (arg.findLast("-f", fn))
	{
		ifstream ifs(fn.c_str());
		if (!ifs) 
		{
			cerr<<"Can not open the file: "<<fn<<endl;
			return -1;
		}
		string st;
		while (ifs>>st)
		{
			htmlpage += st;
			htmlpage += ' ';
		}
	}
	else {
		string st;
		while (cin>>st)
		{
			htmlpage += st;
			htmlpage += ' ';
		}
	}
	CHTMLLexic lexic(htmlpage.c_str(), htmlpage.size());
	auto_ptr<CHTMLItem> pitem;
	while ((pitem = lexic.output()).get()) 
	{
		if (pitem->type() == Html_Comment) 
		{
			cout<<*(CComment*)pitem.get();
		}
		else if (pitem->type() == Html_Text) 
		{
			cout<<*(CText*)pitem.get();
		}
		else if (pitem->type() == Html_Encap) 
		{
			cout<<*(CEncapsuled*)pitem.get();
		}
		else if (pitem->type() == Html_Tag) 
		{
			cout<<*(CTag*)pitem.get();
		}
		cout<<"\n*************************************************\n";
	}
	
	return 0;
}

void
help()
{
	cout <<
	"Parse the html document and output all the items in it.\n"
	"Usage:\n"
	"-fHTML: Get input from HTML file(otherwise from standard input). \n";
	return;
}

