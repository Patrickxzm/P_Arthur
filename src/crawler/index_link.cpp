#include "util/arg.h"
#include "links.h"
#include <iostream>
#include <map>
#include <fstream>
#include <sstream>

using namespace std;

ostream &
help(ostream& os)
{
	os<<"If there is no index file, build a index for the link file inputed "
	"and output it from stdout; else read url from stdin, find out its link data.\n"
	"Usage: Cmd --link-file=... [--index-file=...] [--help|-h]\n"
	"\t--link-file=... :  name of link file that will be indexed.\n"
	"\t--index-file=... :  index file used to index link file.\n"
	"\t[--help|-h] : print this message.\n"
	  <<endl;
	return os;
}

int
search(const map<string, int> &index, ifstream &data, const string& url, ostream &output)
{
	map<string, int>::const_iterator cit = index.find(url);
	if (cit == index.end())
	{
		output<<"Not Found!"<<endl;
		return -1;
	}
	data.seekg(cit->second);
	unsigned numoflinks;
	string surl;
	data>>surl>>numoflinks;
	data.ignore(numeric_limits<int>::max(), '\n');
	output<<surl<<'\t'<<numoflinks<<endl;
	for (unsigned i=0; i<numoflinks; i++)
	{
		string line;
		if (getline(data, line))
			output<<line<<endl;
		else
			break;
	}
	return 0;
}

bool
try_insert(map<string, int> &index, const string &url, int pos)
try {
	return index.insert(pair<string, int>(url, pos)).second;
}
catch (std::bad_alloc &e)
{
	ostringstream oss;
	oss<<e.what()<<" where index.size()="<<index.size();
	throw std::runtime_error(oss.str());	
}

int
build_index(ifstream &data, ostream &output)
{
	map<string, int> index;
	unsigned numoflinks;
	streampos pos = data.tellg();
	CPageLink link;
	while (data>>link)
	{
		if (index.find(link.url) == index.end())
		{
			//index[link.url] = pos;
			index.insert(pair<string, int>(link.url, pos));
			//try_insert(index, link.url, pos);
		}
		else
		{
			ostringstream oss;
			oss<<"Duplicated url:"<<link.url;
			throw runtime_error(oss.str());
		}
		pos = data.tellg();
	}
	data.clear();
	data.seekg(0, ios::beg);
	while (data>>link)
	{
		for (unsigned i=0; i<link.outlinks.size(); i++)
		{
			string target = link.outlinks[i];
			if (index.find(target)==index.end())
			{
#ifdef _DEBUG_09192008
				cerr<<'['<<target<<']'<<endl;
#endif // _DEBUG_09192008
				//index[target]=-1;
				index.insert(pair<string, int>(target, -1));
				//try_insert(index, target, -1);
			}
		}
	}
	for (map<string, int>::const_iterator cit = index.begin()
	  ; cit != index.end(); cit ++)
	{
		output<<cit->first<<'\t'<<cit->second<<endl;
	}
	return index.size();
}

int 
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	CArg::ArgVal val;
	if (!arg.find1("-h").null() || !arg.find1("--help").null())
	{
		help(cerr);
		return -2;
	}
	if ((val=arg.find1("--link-file=")).null())
	{
		cerr<<"please specify the link file."<<endl;
		return -1;
	}
	string fn = string(val);
	ifstream ifs(fn.c_str());
	if (!ifs)
	{
		cerr<<"CAN not open link file :\""<<fn<<"\""<<endl;
		return -3;
	}
	if (!(val=arg.find1("--index-file=")).null())
	{
		string fnIndex=string(val);
		map<string, int> index;
		load_index(index, fnIndex);
		string url;
		cout<<'>'<<flush;
		while (cin>>url)
		{
			cout<<"******************************"<<endl;
			search(index, ifs, url, cout);
			cout<<'>'<<flush;
		}
	}
	else
	{
		build_index(ifs, cout);
	}
	return 0;
}
