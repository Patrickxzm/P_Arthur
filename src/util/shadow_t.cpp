#include "shadow.h"
#include "arg.h"
#include <iostream>

using namespace std;

const int default_capacity = 1000;

ostream&
help(ostream& os)
{
	os<<"Put strings into a CStrSetShadow  or just make a test.\n"
	  "Usage: Command [--file=] [--capacity=...] [--test] [--help|-h] \n"
	  "\t--file= : name of file associated with the shadow.\n"
	  "\t--capacity= :  required capacity. (default: "<<default_capacity<<")\n"
	  "\t--test : Not put, Just a test. \n"
	  "\t--help|-h : print this message.\n"
	  "\tcin : strings to put into the shadow, one string per line.\n"
	  "\tcout : strings put.\n"
	  <<endl;
	return os;
}

int
main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	if (arg.find1("-h") || arg.find1("--help"))
	{
		help(cout);
		return 1;
	}
	CArg::ArgVal val;
	unsigned capacity;
	if (val=arg.find1("--capacity="))
		capacity = val.INT();
	else 
		capacity = default_capacity;
	bool test = arg.found("--test");
	CStrSetShadow shadow;
	int res = shadow.open(arg.find1("--file="), capacity, CStrSetShadow::Create);
	if (res == CStrSetShadow::Create)
		clog<<"New Shadow file created."<<endl;
	clog<<"Shadow Capacity : "<<shadow.capacity()<<endl;
	clog<<"Shadow Size : "<<shadow.size()<<endl;
	clog<<"mapped file size : "<<shadow.memory_size()<<endl;
	string line;
	while (getline(cin, line))
	{
		if (test)
		{
			if (!shadow.has(line))
				cout<<line<<endl;
			continue;
		}
		switch (shadow.put(line))
		{
		case CStrSetShadow::putOK :
			cout<<line<<endl;
			break;
		case CStrSetShadow::putFull :
			cerr<<"shaow overflow"<<endl;
			return -1;
		}
	}
	return 0;
}
catch (exception &e)
{
	cerr<<"Catch Exception:"<<e.what()<<endl;
	return -1;
}
