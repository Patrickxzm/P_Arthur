#include <iostream>
#include <sstream>
#include "util/arg.h"
#include "task.h"

using namespace std;

ostream& 
help(ostream &os)
{
	os << "Convert {./hostport eol {task eol}* eol}* to {url eol}* \n"
	   "\tUsage: cmd [--skip-new] [--skip-old] [-h|--help] \n"
	   "\t\t --skip-new : not output new urls \n"
	   "\t\t --skip-old : not output old urls \n"
	   "\t\t -h|--help : print this message.\n"
	   "\t\t cin : tasks.\n"
	   "\t\t cout : urls.\n"
	  <<endl;
	return os;
}

int
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (arg.found("-h") | arg.found("--help"))
	{
		help(cout);
		return 1;
	}
	bool skip_new = arg.found("--skip-new");
	bool skip_old = arg.found("--skip-old");
	string line;
	while (getline(cin, line) && line.size()>0)
	{
		if (line[0]!='.' || line[1]!='/')
		{
			cerr<<"unknown input ./hostport :"<<line<<endl;
			return -1;
		}
		for (unsigned i=2; i<line.size(); i++)
		{
			char ch = line[i];
			if (isblank(ch) || ch=='/')
			{
				cerr<<"unknown input ./hostport :"<<line<<endl;
				return -3;
			}
		}
		string hostport = line.substr(2);
		while (getline(cin, line) && line.size()>0)
		{
			istringstream iss(line);
			CTask task;
			if (!(iss >> task))
			{
				cerr<<"unknown input task :"<<line<<endl;
				return -2;
			}
			if (!(task.status & CTask::Unknown) 
			   && skip_old && (task.status & CTask::Visited))
				continue;
			if (!(task.status & CTask::Unknown)
			   && skip_new && !(task.status & CTask::Visited))
				continue;
			cout<<"http://"<<hostport<<task.localstr<<'\n';
		}
	}
	return 0;
}
