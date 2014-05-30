#include "arg.h"
#include <cstring>
#include <stdlib.h>
using std::strlen;
using std::strncmp;
using std::logic_error;
using std::ostringstream;

CArg::CArg(int argc, char** argv)
{
	this->argc = argc;
	this->argv = argv;
}

CArg::~CArg()
{
}

CArg::ArgVal
CArg::find1(const char* prefix) const
{
	for (int i=1; i<argc; i++)
	{
		if (strncmp(argv[i], prefix, strlen(prefix)) == 0)
		{
			return argv[i]+strlen(prefix);
		}
	}
	return 0;
}

vector<CArg::ArgVal> 
CArg::follow(const char* option) const
{
	vector<ArgVal> res;
	bool found = false;
	for (int i=1; i<argc; i++)
	{
		if (found)
		{
			res.push_back(argv[i]);
			continue;
		}
		if (strcmp(argv[i], option) == 0)
			found = true;
	}
	return res;
}

vector<CArg::ArgVal>
CArg::find(const char* prefix) const
{
	vector<ArgVal> res;
	for (int i=1; i<argc; i++) 
	{
		if (strncmp(argv[i], prefix, strlen(prefix)) == 0)
		{
			res.push_back(argv[i] + strlen(prefix));
		}
			
	}
	return res;
}

int
CArg::ArgVal::INT() const
{
	if (charPtr == 0) 
		throw runtime_error("Convert from null.");
	return atoi(charPtr);
}

