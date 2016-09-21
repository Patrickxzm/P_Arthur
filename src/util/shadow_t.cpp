#include "shadow.h"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <cassert>

using namespace std;

const int default_capacity = 1000;

ostream&
help(ostream& os)
{
	os<<"Put strings into a CStrSetShadow  or just make a test.\n"
	  "Usage: Command [--file=] [--capacity=...] [--help|-h] \n"
	  "\t--file= : name of file associated with the shadow.\n"
	  "\t--capacity= :  required capacity. (default: "<<default_capacity<<")\n"
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
	if (arg.found("-h") || arg.found("--help"))
	{
		help(cout);
		return 1;
	}
        string strCap;
	unsigned capacity;
	if (arg.findLast("--capacity=", strCap))
        {
		capacity = stoul(strCap);
        }
	else 
		capacity = default_capacity;
	CStrSetShadow shadow;
	int res;
        string fn;
        if (arg.findLast("--file=", fn))
            res = shadow.open(fn.c_str(), capacity, CStrSetShadow::Create);
        else
            res = shadow.open(0, capacity, CStrSetShadow::Create);
	if (res == CStrSetShadow::Create)
		clog<<"New Shadow file created."<<endl;
	else if (res == CStrSetShadow::Attach)
		clog<<"Shadow file Attached."<<endl;
	else if (res == CStrSetShadow::Anonymous)
		clog<<"Anonymous shadow used."<<endl;
	else
		assert(false);
	clog<<"Shadow Capacity : "<<shadow.capacity()<<endl;
	clog<<"Shadow Size : "<<shadow.size()<<endl;
	clog<<"mapped file size : "<<shadow.memory_size()<<endl;
	string line;
	while (getline(cin, line))
	{
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
