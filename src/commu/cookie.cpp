/************************************************************************
 * @ This is a very simple cookie: Just the path is in consider. It is used
 *   in mosquito in which there is only a host. Attributes such as "domain"
 *   is no use.					02/10/2004	23:44
 ************************************************************************/
#include "cookie.h"
#include "util/util.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
using std::istringstream;
using std::ifstream;
using std::ofstream;
using std::ws;


namespace {
	unsigned max_cookie_length = 2000;
	unsigned max_cookie_num = 20;
	
	// name=value;
	istream& 
	operator>>(istream& is, pair<string, string> &p)
	{
		is>>ws;
		p.first.clear();
		p.second.clear();

		char ch;
		while (is.get(ch) && ch!='=' && ch!=';' && !isblank(ch))
			p.first.push_back(ch);
		if (p.first.empty())
		{
			is.setstate(std::ios::failbit);
			return is;
		}
		// allows whitespace between the attribute and the = sign
		if (is && isblank(ch))
			is>>ch;
		if (!is) 
		{
			is.clear();
			return is;
		}
		if (ch == '=')
		{
			is>>ch;
			if (ch=='\'' || ch=='\"')
			{
				is.putback(ch);
				quoted_string qs;
				if (is>>qs)
					p.second = qs;
				else
					is.setstate(std::ios::badbit);
			}
			else
			{
				p.second.push_back(ch);
				while (is.get(ch) && ch!=';')
					p.second.push_back(ch);
				if (is)
					is.putback(ch);
				else
					is.clear();	
			}
		}
		else 
		{
			is.putback(ch);
		}
		return is;
	}
}

//
// RFC2965 for referrence
// Set-Cookie: name=value; Path=/; Expires=...
// Set-Cookie: JSESSIONID=149C9E631B269C1B41BACE7DECAAC65F; Path=/eecswww
//
int 
CCookie::set(const string &line)
{
	clear();
	
	if (line.length() > max_cookie_length)
		return -3;
	istringstream iss(line);
	pair<string, string> p;
	if (!(iss>>p))
		return -1;
	name=p.first;
	value=p.second;
	
	char ch;
	while (iss>>ch && ch==';' && iss>>p)
	{
		if (strcasecmp(p.first.c_str(), "Path") == 0)
			path = p.second;
		else if (strcasecmp(p.first.c_str(), "Host") == 0)
			host = p.second;
		else if (strcasecmp(p.first.c_str(), "Domain") == 0)
			domain = p.second;
		else if (strcasecmp(p.first.c_str(), "Host") == 0)
			host = p.second;
		else if (strcasecmp(p.first.c_str(), "Expires") == 0)
			expires = p.second;
		else 
			attrs.push_back(p);
	}
	if (iss.eof())
		return 0;
	else
		return -2;
}

ostream& 
operator<<(ostream& os, const CCookie& coo)
{
	if (coo.name.empty())
	{
		os.setstate(std::ios::failbit);
		return os;
	}
	os<<coo.name;
	if (coo.value.size() > 0)
		os<<"="<<coo.value;
	if (coo.path.size() > 0)
		os<<"; Path="<<coo.path;
	if (coo.host.size() > 0)
		os<<"; Host="<<coo.host;
	if (coo.domain.size() > 0)
		os<<"; Domain="<<coo.domain;
	if (coo.expires.size() > 0)
		os<<"; Expires="<<coo.expires;
	for (unsigned i=0; i<coo.attrs.size(); i++)
	{
		if (coo.attrs[i].first.empty())
			continue;
		os<<"; "<<coo.attrs[i].first;
		if (coo.attrs[i].second.size()>0)
			os<<"="<<coo.attrs[i].second;
	}
	return os;
}

CCookies::CCookies(const char* fn)
{
	load(fn);
}

CCookies::~CCookies()
{
}

int
CCookies::load(const char* fn)
{
	clear();

	ifstream ifs(fn);
	if (!ifs)
		return -1;
	string line;
	while (getline(ifs, line))
	{
		CCookie coo(line);
		if (check_expires(coo.expires))
			add(coo);
	}
	return 0;
}

int
CCookies::save(const char* fn)
{
	if (this->size() == 0)
		return 1;
	ofstream ofs(fn);
	for (const_iterator cit = begin(); cit != end(); cit ++)
	{
		if (check_expires(cit->second.expires))
			ofs<<cit->second<<'\n';
	}
	return 0;
}

vector <const CCookie*> 
CCookies::match(const CURL &url) const
{
	vector<const CCookie*> result;
	for (const_iterator cit = this->begin(); cit != this->end(); cit++)
	{
		const CCookie &cookie = cit->second;
		const string &path = cookie.path;
		if (url.localpath().compare(0, path.size(), path) != 0)
			continue;

		const string &host = cookie.host;
		string url_host = url.host();
		if (host.size() > 0 && url_host != host)
			continue;

		const string &domain = cookie.domain;
		if (domain.size() > 0)
		{
			if (url_host.size() < domain.size())
				continue;
			if (url_host.compare(url_host.size()-domain.size(), domain.size(), domain) != 0)
				continue;    // more code needed according to rfc 2965
		}

		if (cookie.expires.size()>0 && !check_expires(cookie.expires))
			continue;

		result.push_back(&cookie);
	}
	return result;
}

int 
CCookies::add(const CCookie &coo)
{
	if (coo.name.empty())
		return -1;
	iterator it = this->find(coo.name);
	if (it == this->end())
	{
		if (this->size() >= max_cookie_num)
		{
			this->erase(q.front());
			q.pop();
		}
		this->insert(pair<string, CCookie>(coo.name, coo));
		q.push(coo.name);
		return 1; // add 
	}
	else
	{
		it->second = coo;
		return 2; // update
	}
}

bool 
CCookies::check_expires(const string &estr)
{
	if (estr.empty())
		return false;
	//Tue, 22-Sep-2009 08:26:04 GMT	
	istringstream iss(estr);
	string wday;
	iss>>wday;
	struct tm expires;
	char dash;
	string month;
	int year;
	iss>>expires.tm_mday>>dash>>std::setw(3)>>month>>dash>>year;
	if (month=="Jan")
		expires.tm_mon=0;
	else if (month=="Feb")
		expires.tm_mon=1;
	else if (month=="Mar")
		expires.tm_mon=2;
	else if (month=="Apr")
		expires.tm_mon=3;
	else if (month=="May")
		expires.tm_mon=4;
	else if (month=="Jun")
		expires.tm_mon=5;
	else if (month=="Jul")
		expires.tm_mon=6;
	else if (month=="Aug")
		expires.tm_mon=7;
	else if (month=="Sep")
		expires.tm_mon=8;
	else if (month=="Oct")
		expires.tm_mon=9;
	else if (month=="Nov")
		expires.tm_mon=10;
	else if (month=="Dec")
		expires.tm_mon=11;
	else 
		return false;
	if (year<1900)
		return false;
	expires.tm_year=year-1900;
	char colon;
	iss>>expires.tm_hour>>colon>>expires.tm_min>>colon>>expires.tm_sec;
	if (!iss)
		return false;
	return timegm(&expires) > time(0);
}
