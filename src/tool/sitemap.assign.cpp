#include "arg.h"
#include <iostream>
#include <fstream>
#include <string>
#include "pat_types.h"

using namespace std;

void help();

int
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (*arg.find("-h") || *arg.find("--help"))
	{
		help();
		return -2;
	}
	char** tmp;
	string list_file;
	if (*(tmp=arg.find("-f")) != 0)
	{
		list_file = *tmp;
	}
	if (list_file.empty())
	{
		cerr<<"You must give a LIST_FILE name."<<endl;
		return -1;
	}
	string queen;
	if (*(tmp=arg.find("-q")) != 0)
	{
		queen = *tmp;
	}
	if (queen.empty())
	{
		cerr<<"You must give a Queen name."<<endl;
		return -1;
	}
	string map_file;
	if (*(tmp=arg.find("-t")) != 0)
	{
		map_file = *tmp;
	}
	if (map_file.empty())
	{
		map_file = ".site2Queen";
	}
	bool truncate = false;
	if (*arg.find("-TRUNC") != 0)
	{
		truncate = true;
	}

	ifstream ifs(list_file.c_str());
	string site;
	hash_map_str2str site2Queen;
	while (ifs>>site)
	{
		site2Queen[site] = queen;
	}
	ofstream target;
	if (truncate)
		target.open(map_file.c_str(), ios::out);
	else
		target.open(map_file.c_str(), ios::app|ios::out);
	for (hash_map_str2str::const_iterator cit=site2Queen.begin()
		; cit != site2Queen.end(); cit++)
	{
		target<<cit->first<<'\t'<<cit->second<<'\n';
	}
	return 0;
}

void
help()
{
	cout<<"Usage: command -fLIST_FILE -qQUEEN [-tMAP_FILE] [-TRUNC]\n";
	cout<<"MAP_FILE has a default value \".site2Queen\""<<endl;
}
