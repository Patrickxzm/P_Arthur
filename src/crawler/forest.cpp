#include "util/arg.h"
#include "links.h"
#include "cds.h"
#include <iostream>
#include <fstream>
#include <set>
using namespace std;

ostream &
help(ostream &os)
{
	os<<"Produce depth-2 forest from links file & its index.\n"
	"Usage: Cmd --links-file=... --index=... --root=... [--root-only] [--help|-h]\n"
	"\t--links-file= : name of the links file.\n"
	"\t--index= : name of the file containing the index.\n"
	"\t--root= : root url of this site.\n"
	"\t--root-only : only output the root nodes of the forest.\n"
	"\t--help|-h : print this message.\n"
	<<endl;
	return os;
}

void print(const B_t& roots, ostream &os, int count, ifstream &links, bool rootOnly)
{
	os<<"<html>\n"
	"<head><title>WebSite Forest Output</title></head>\n"
	"<body>\n"<<endl;
	set<string> cds;
	for (unsigned i=0; i<roots.size(); i++)
		cds.insert(roots[i]->url);
	CStrSetShadow colored(1000000);
	os<<"Number of root nodes: "<<roots.size()<<"<br>"<<endl;
	links.clear();
	links.seekg(0);
	CPageLink link;
	for (int nloop = 0; links>>link; )
	{
		if (cds.find(link.url) == cds.end())
		{
			continue;
		}
		int num = 0;
		vector<string> children;
		for (unsigned i=0; i<link.outlinks.size(); i++)
		{
			if (cds.find(link.outlinks[i]) != cds.end())
				continue;
			if (colored.put(link.outlinks[i]))
			{
				children.push_back(link.outlinks[i]);
				num ++;
			}
		}
		if (nloop ++ < count)
		{
			os<<"&lt;"<<num<<"&gt;:&nbsp<a href=\""
				<<link.url<<"\">" <<link.url<<"</a><br>"<<endl;
			for (unsigned i=0; i<children.size()&&!rootOnly; i++)
			{
				os<<"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp"
				  "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp"
				  "=&gt;"<<children[i]<<"<br>"<<endl;
			}
		}
	}
	os<<"Number of leaf nodes: "<<colored.nitems()<<"<br>"<<endl;
	if (!links.eof())
		os<<"Error!: Links file not finish."<<"<br>"<<endl;
	os<<"</body></html>\n"<<endl;
	return ;
}

/*
 Input: links, index
 Out:	depth-2 forest 
 */
int 
main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	CArg::ArgVal val;

	if (!arg.find("--help").empty() || !arg.find("-h").empty())
	{
		help(cerr);
		return 1;
	}
	ifstream links;
	if ((val = arg.find1("--links-file=")).null())
	{
		cerr<<"Please input the name of the links file."<<endl;
		return -1;
	}
	else
	{
		string fn = string(val);
		links.open(fn.c_str());
		if (!links)
		{
			cerr<<"Can not open the links file:"<<fn<<endl;
			return -2;
		}
	}
	bool rootOnly = !arg.find("--root-only").empty();

	map<string, int> index;
	if ((val = arg.find1("--index=")).null())
	{
		cerr<<"Please input the name of the index file."<<endl;
		return -3;
	}
	else
	{
		string fn = string(val);
		load_index(index, fn);
	}
	string root;
	if ((val = arg.find1("--root=")).null())
	{
		cerr<<"Please input the root url."<<endl;
		return -5;
	}
	else
	{
		root = string(val);
	}
	if (root.empty())
	{
		cerr<<"input error: root is an empty string!"<<endl;
		return -6;
	}
	CDSAlgorithm cds;
	const B_t& roots = cds.cal(&links, &index, root);
	//cout<<"# of nodes colored is "<<cds.ncolored()<<endl;
	print(roots, cout, 800, links, rootOnly);
}
catch (std::exception &e)
{
	cerr<<"std::exception : "<<e.what()<<endl;
	return -4;
}
catch (...)
{
	cerr<<"Unknown exception : "<<endl;
	return -5;
}
