#ifndef _VITO_ROBOTS_H_111004
#define _VITO_ROBOTS_H_111004
#include <string>
#include "commu/http.h"

using std::string;

class CRobots
{
public:
	CRobots(const string &agent="");
	
	int init(const string &host, int port);
	int init();
	bool allow(const string &localstr) const;
	inline int crawl_delay() const
	{
		return _crawl_delay<20 ? _crawl_delay : 20;
	}
	void import(const vector<string> &allow, const vector<string> &disallow);
	void import(istream &is);
private:
	int parse(CHttp &http);
	void clear();
	istream& read(istream& is, pair<string, string> &p);
private:
	bool f_init;
	int _crawl_delay;
	vector<string> _disallow;
	vector<string> _allow;
	string user_agent;
};
#endif // _VITO_ROBOTS_H_111004
