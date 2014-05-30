#include "shadow.h"
#include "arg.h"
#include <iostream>

using namespace std;

const int default_capacity = 1000;

ostream&
help(ostream& os)
{
	os<<"Try to put key strings into a CStrSetShadow object or test key strings against it.\n"
	  "Usage: Command [--file=] [--capacity=...] [--start=...] [--end=...] [--test [--reverse]] [--help|-h] \n"
	  "\t--file= : name of file associated with the shadow.\n"
	  "\t--capacity= :  required capacity. (default: "<<default_capacity<<")\n"
	  "\t--start= : start position used as key string. (default:0)\n"
	  "\t--end= : end posistion not included used as key string. (default:max)\n"
	  "\t--test : Test the string weather in the shadow. \n" 
	  "\t--reverse : Output the lines whose keys are not in the shadow. \n"
	  "\t--help|-h : print this message.\n"
	  "\tcin : lines to put into the shadow or to test. each string per line.\n"
	  "\tcout : lines put into the shadow, or pass the test.\n"
	  "\tclog, cerr : ... \n"
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
	int start=-1, end=-1;
	if (val=arg.find1("--start="))
	{
		start = val.INT();
		if (start < 0)
		{
			cerr<<"start value should not be less than zero.\n";
			return -1;
		}
	}
	if (val=arg.find1("--end="))
	{
		end = val.INT();
		if (end <= 0)
		{
			cerr<<"end value must be greater than zero.\n";
			return -2;
		}
	}
	if (start>=0 && end>0 && start >= end)
	{
		cerr<<"start value should be less than end value.\n";
		return -2;
	}
	bool test = arg.found("--test");
	bool reverse = arg.found("--reverse");
	CStrSetShadow shadow;
	int res = shadow.open(arg.find1("--file="), capacity, CStrSetShadow::Create);
	if (res == CStrSetShadow::Create)
		clog<<"New Shadow file created."<<endl;
	clog<<"Shadow Capacity : "<<shadow.capacity()<<endl;
	clog<<"Shadow Size : "<<shadow.size()<<endl;
	clog<<"mapped file size : "<<shadow.memory_size()<<endl;
	if (test)
		clog<<"Please input the strings that you want to test against the shadow... "<<endl;
	else
		clog<<"Please input the strings that you want to put into the shadow... "<<endl;
	string line;
	while (getline(cin, line))
	{
		string key;
		if (start == -1 && end == -1)
		{
			key = line;
		}
		else {
			istringstream iss(line);
			string s;
			for (int pos = 0; iss>>s; pos++)
			{
				if (start != -1 && pos<start)
					continue;
				if (end != -1 && pos >= end)
					break;
				if (!key.empty())
					key += ' ';
				key += s;
			}
		}
		if (!test)
		{
			switch (shadow.put(key))
			{
			case CStrSetShadow::putOK :
				cout<<line<<endl;
				break;
			case CStrSetShadow::putFull :
				throw runtime_error("shaow overflow.");
			}
			continue;
		}	
		if (shadow.has(key) ^ reverse)
			cout<<line<<endl;
	}
	return 0;
}
catch (exception &e)
{
	cerr<<"Catch Exception:"<<e.what()<<endl;
	return -1;
}
