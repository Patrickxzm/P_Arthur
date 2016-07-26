#include "rmdirtree.h"
#include "arg.h"
#include <assert.h>
#include <iostream>

using namespace std;

ostream&
help(ostream &os)
{
	os<<"Remove a directory including all files and sub-directories in it.\n"
	  "\tUsage: Cmd --dir=... [-h|--help]\n"
	  "\t\t --dir= : directory to remove.\n"
	  "\t\t --h|--help : print this message.\n"
	  <<endl;
	return os;
}

int 
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (arg.found("-h") || arg.found("--help"))
	{
		help(cout);
		return 1;
	}
	string dir;
        if (!arg.findLast("--dir=", dir))
	{
		cerr<<"Please give a \"--dir=\" option."<<endl;
		return -1;
	}
	switch (rmdirtree(dir))
	{
	case 0: 
		cout<<"Done."<<endl;
		break;
	case -1:
		cerr<<"Can not list a directory."<<endl;
		return -1;
	case -2:
		cerr<<"Can not unlink a file."<<endl;
		return -2;
	case -3:
		cerr<<"Can not remove a dir."<<endl;
		return -3;
	default: 
		assert(false);
	}
	return 0;
}
