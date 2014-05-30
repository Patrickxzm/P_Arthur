#ifndef _PAT_LINKS_H_092208
#define _PAT_LINKS_H_092208
#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <map>
using std::map;
using std::istream;
using std::ostream;
using std::vector;
using std::string;
class CPageLink
{
public:
	CPageLink()
	{}
	CPageLink(const string &u)
	: url(u)
	{}
	virtual ~CPageLink()
	{}
public:
	string url;
	vector<string> outlinks;
};
ostream& operator<<(ostream& os, const CPageLink &link);
istream& operator>>(istream& is, CPageLink &link);
int load_index(map<string, int> &index, const string& fnIndex);
#endif // _PAT_LINKS_H_092208
