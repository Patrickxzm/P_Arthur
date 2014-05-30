#ifndef _PAT_COOKIE_H_121203
#define _PAT_COOKIE_H_121203
#include <string>
#include <vector>
#include <queue>
#include "url/url.h"

using std::string;
using std::vector;
using std::pair;
using std::queue;

// RFC2965 for referrence
// Set-Cookie: name=value;path=/;expires=...;
class CCookie
{
public:
	CCookie()
	{}
	CCookie(const string &line)
	{
		set(line);
	}
	int set(const string &line);
private:
	void clear()
	{
		name.clear();
		value.clear();
		path.clear();
		host.clear();
		domain.clear();
		expires.clear();
		attrs.clear();
	}
public:
	string name;
	string value;
	string path;
	string host;
	string domain;
	string expires;
	vector<pair<string, string> > attrs;
};

ostream& operator<<(ostream& os, const CCookie& coo);

class CCookies : private map<string, CCookie>
{
public:
	CCookies(const char* fn);
	CCookies()
	{}
	~CCookies();
	int load(const char* fn);
	int save(const char* fn);
	int add(const CCookie &coo);
	vector<const CCookie*> match(const CURL &url) const;
private:
	static bool check_expires(const string &estr);
private:
	queue<string> q;
};

#endif // _PAT_COOKIE_H_121203
