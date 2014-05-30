#include "util/arg.h"
#include "url/url.h"
#include "htmlref.h"
#include <iostream>
#include <fstream>
using namespace std;
void help();

int 
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (!arg.find1("-h").null() || !arg.find1("--help").null())
	{
		help();
		return 1;
	}
	CURL base;
	CArg::ArgVal av;
	if (!(av=arg.find1("-b")).null())
	{
		base.init(string(av));
	}
	else 
		base.init("http://net.cs.pku.edu.cn/~xzm/");
	string htmlpage;
	av = arg.find1("-f");
	if (!av.null())
	{
		ifstream ifs((const char*)av);
		if (!ifs) 
		{
			cerr<<"Can not open the file: "<<(const char*)av<<endl;
			return -1;
		}
		string st;
		while (ifs>>st)
		{
			htmlpage.append(st);
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
/* 
	CHTMLLexic lexic(htmlpage.c_str(), htmlpage.size());
	CHTMLRef htmlref(&lexic,base.str());
*/  
	CHTMLRef htmlref(htmlpage.c_str(), htmlpage.size(), base.str().c_str());
	auto_ptr<CRef> ref;
	while ((ref=htmlref.refer()).get())
		cout<<*ref<<endl;
	return 0;
}

void
help()
{
	cout <<
	"Parse the html document and output all the referrences(including their postion info.) in it.\n"
	"Usage: htmlref -fHTML -bURL\n"
	"-fHTML: Get input from HTML file(otherwise from standard input). \n"
	"-bURL : Give a Base URL to analysis relative URLs. (default: http://net.cs.pku.edu.cn/~xzm/) \n";
	return;
}

