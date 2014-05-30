#include "links.h"
#include <fstream>
#include <stdexcept>

using std::ifstream;
using std::numeric_limits;
using std::runtime_error;
using std::streamoff;


istream& 
operator>>(istream& is, CPageLink &link)
{
	unsigned numoflinks;
	if (!(is>>link.url>>numoflinks))
		return is;
	is.ignore(numeric_limits<int>::max(), '\n');
	link.outlinks.clear();
	string line;
	for (unsigned i=0; i<numoflinks; i++)
	{
		if (!getline(is, line))
			return is;
		link.outlinks.push_back(line);
	}
	return is;
}

ostream& 
operator<<(ostream& os, const CPageLink &link)
{
	os<<link.url<<'\t'<<link.outlinks.size()<<std::endl;
	for (unsigned i=0; i<link.outlinks.size(); i++)
		os<<link.outlinks[i]<<std::endl;
	return os;
}

int
load_index(map<string, int> &index, const string& fnIndex)
{
	ifstream ifs(fnIndex.c_str());
	if (!ifs)
		throw runtime_error("Can not open index file:"+fnIndex);
	string url;
	streamoff off;
	index.clear();
	int i=0;
	while (ifs>>url>>off)
	{
		i++;
		index[url] = off;
	}
	return i;
}


