#include "robots.h"
#include "util/util.h"
#include <sstream>
#include <cstdlib>
using std::istringstream;

CRobots::CRobots(const string &agent) : f_init(false), user_agent(agent)
{
}

int
CRobots::init()
{
	f_init = true;
	clear();
	return 0;
}

int
CRobots::init(const string &host, int port)
{
	f_init = true;
	clear();
	CHttp http("http://"+host+":"+tostring(port)+"/robots.txt");
	http.timeout(45, 45);
	CHttp::result_t result = http.sfetch();
	if (result == CHttp::OK)
		return parse(http);
	else
		return -1;
}

void
CRobots::clear()
{
	_allow.clear();
	_disallow.clear();
	_crawl_delay = 0;
}

int 
CRobots::parse(CHttp &http)
{
	if (http.get_status() < 200 || http.get_status() >= 300)
		return -1;
	string content_type = http.reply.headers.value("Content-Type");
	if (strcasestr(content_type.c_str(), "text/plain") == 0)
		return -2;

	enum {Exact, Wild, None} match = None;
	istringstream iss(http.reply.body);
	pair<string, string> mypair;
	while (this->read(iss, mypair))
	{
		if (strcasecmp(mypair.first.c_str(), "User-agent") == 0)
		{
			if (user_agent == mypair.second)
			{
				match = Exact;
				clear();  // overwrite "Usage-agent: *"
			}
			else if ("*" == mypair.second && match == None)
			{
				match = Wild;
			}
			else
			{
				match = None;
			}
			continue;
		}
		if (match == None)
			continue;
		if (strcasecmp(mypair.first.c_str(), "Allow") == 0)
		{
			if (mypair.second.size() > 0)
				_allow.push_back(mypair.second);
		}
		else if (strcasecmp(mypair.first.c_str(), "Disallow") == 0)
		{
			if (mypair.second.size() > 0)
				_disallow.push_back(mypair.second);
		}
		else if (strcasecmp(mypair.first.c_str(), "Crawl-delay") == 0)
		{
			_crawl_delay = atoi(mypair.second.c_str());
		}
	}
	return 0;
}

bool
CRobots::allow(const string &localstr) const
{
	assert(f_init);
	for (size_t i=0; i<_allow.size(); i++)
	{
		if (localstr.compare(0, _allow[i].size(), _allow[i]) == 0)
			return true;
	}
	for (size_t i=0; i<_disallow.size(); i++)
	{
		if (localstr.compare(0, _disallow[i].size(), _disallow[i]) == 0)
			return false;
	}
	return true;
}

istream&
CRobots::read(istream& is, pair<string, string> &p)
{
	string line;
	while (getline(is, line))
	{
		if (line.empty())
			continue;
		if (line[0] == '#')
			continue;
		istringstream iss(line);
		if (!getline(iss, p.first, ':'))
			continue;
		iss>>p.second;
		return is;
	}
	return is;
}

void
CRobots::import(istream &is)
{
	pair<string, string> mypair;
	while (this->read(is, mypair))
	{
		if (strcasecmp(mypair.first.c_str(), "Allow") == 0)
		{
			if (mypair.second.size() > 0)
				_allow.push_back(mypair.second);
		}
		else if (strcasecmp(mypair.first.c_str(), "Disallow") == 0)
		{
			if (mypair.second.size() > 0)
				_disallow.push_back(mypair.second);
		}
	}
	return ;
}

void
CRobots::import(const vector<string> &allow, const vector<string> &disallow)
{
	_allow.insert(_allow.end(), allow.begin(), allow.end());
	_disallow.insert(_disallow.end(), disallow.begin(), disallow.end());
	return ;
}

