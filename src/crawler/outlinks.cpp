#include <iostream>
#include <map>
#include <assert.h>
#include <fstream>
#include "url/url.h"
#include "util/sort.hpp"
#include "util/arg.h"
#include "util/shadow.h"

using namespace std;

ostream&
help(ostream& os)
{
	os<<"To count or merge outlinks data.\n"
	  "\tUsage: Cmd ([--from=...] [--to=...]) | ([--merge {filesSorted}*]) [--help|-h]\n"
	  "\t\t--from=, --to=: 2 ends of a link, may be host, site, all or (null).\n"
	  "\t\t--merge: merge sorted link files.\n"
	  "\t\tfilesSorted: name of sorted link file to merge.\n"
	  "\t\t-h|--help: print this message.\n"
	  "\t\tcin: outlinks data for count.\n"
	  <<endl;
	return os;
}
namespace 
{ 
//   ********************* start of namespace ***************
class CLink
{
public:
	string from;
	string to;
};

ostream&
operator<<(ostream& os, const CLink &link)
{
	return os<<link.from<<'\t'<<link.to;
}

istream&
operator>>(istream& is, CLink &link)
{
	return is>>link.from>>link.to;
}

bool
operator< (const CLink &l1, const CLink &l2)
{
	return l1.from+'\t'+l1.to < l2.from+'\t'+l2.to;
}

class CLinkCount : public CLink
{
public:
	size_t num;
};
//   ********************* end of namespace *************** 
}

string
group(const string &url, const string &desc)
{
	if (desc == "all")
		return "all";
	if (desc.empty())
		return url;
	CURL myurl("http://"+url+"/");
	if (desc == "site")
		return myurl.site();
	if (desc == "host")
		return myurl.host();
	throw runtime_error("Unknown group description:"+desc);
	return "";
}

int
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (arg.found("-h") || arg.found("--help"))
	{
		help(cout);
		return 3;
	}
	
	bool fMerge = arg.found("--merge");
	string from;
	CArg::ArgVal val;
	if (val=arg.find1("--from="))
		from=val;
	string to;
	if (val=arg.find1("--to="))
		to = val;
	if (fMerge)
	{
		vector<istream*> sorted;
		vector<CArg::ArgVal> vals = arg.follow("--merge");
		for (size_t i=0; i<vals.size(); i++)
			sorted.push_back(new ifstream(vals[i].get()));
		merge<CLink>(sorted, cout);
		for (size_t i=0; i<sorted.size(); i++)
			delete sorted[i];
	}
	else {
		CLinkCount link;
		typedef map<CLink, size_t> links_t;
		links_t links;
		while (cin>>link.from>>link.to>>link.num)
		{
			link.from = group(link.from, from);
			link.to = group(link.to, to);
			links_t::iterator it = links.find(link);
			if (it != links.end())
				links[link] += link.num;
			else	
				links[link] = link.num;
		}
		for (links_t::const_iterator cit=links.begin(); cit != links.end(); cit++)
			cout<<cit->first<<'\t'<<cit->second<<endl;
	}
	return 0;
}

