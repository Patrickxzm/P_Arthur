#include "arg.h"
#include <iostream>
using namespace std;

int 
main(int argc, char* argv[])
try{
	CArg arg(argc, argv);
	CArg::ArgVal av;
	if (!(av=arg.find1("--b=")).null())
	{
		string a(av);
		cout<<"a="<<a<<endl;
	}
	return 1;
}
catch (exception &e)
{
	cerr<<e.what()<<endl;
}

