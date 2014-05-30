#include "htmlref.h"
#include "util/arg.h"
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <cassert>

using namespace std;

ostream&
help(ostream& os)
{
	os<<"Extraction links from html document.\n"
	  "\tUsage: Cmd --base=... [-h|--help] \n" 
	  "\t\t --base= : Produce absolute URL from this base URL.\n"
	  "\t\t -h|--help : Print this message.\n"
	  "stdin: html document.\n"
	  "stdout: linkage.\n"
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
	string base;
	CArg::ArgVal val;
	if (!(val=arg.find1("--base=")).get())
	{
		cerr<<"Please specify the base url."<<endl;
		return -1;
	}
	base = string(val);

	ostringstream oss;
	oss<<cin.rdbuf();
	CHTMLLexic lexic(oss.str());
	CHTMLRef htmlref(&lexic, base);
	auto_ptr<CRef> ref;
	while ((ref = htmlref.refer()).get()) 
		cout<<*ref<<endl;
	return 0;
}

