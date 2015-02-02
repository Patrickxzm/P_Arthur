#include "tw_raw.h"
#include "sys/types.h"
#include "util/cutem.h"
#include "util/xmlGet.h"
#include "util/charset.h"
#include <ctime>
#include <sstream>

using std::gmtime;
using std::strftime;
using std::ostringstream;
using std::istringstream;
using std::flush;
using std::ws;


CTWRaw::CTWRaw(const string &u, const string &ip, const CHttpReply &r)
	: url(u), ipaddr(ip), reply(r)
{
	version = "1.0";

	const int bufsize=128;
	char buf[bufsize];
	time_t timev;
	time(&timev);
	// rfc1123-date: 
	strftime(buf, bufsize, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&timev));
	date=buf;
}


ostream& operator<<(ostream &os, const CTWRaw &raw)
{
	os<<"version: "<<raw.version<<'\n';
	os<<"url: "<<raw.url<<'\n';
	os<<"date: "<<raw.date<<'\n';
	if (!raw.ipaddr.empty())	
		os<<"ip: "<<raw.ipaddr<<'\n';
	map<string, string>::const_iterator cit;
	for (cit = raw.ext.begin(); cit != raw.ext.end(); cit++)
		os<<cit->first<<": "<<cit->second<<'\n';

	const CHttpReply &reply = raw.reply;
	ostringstream oss;
	oss<<reply.status<<"\r\n";
	for (unsigned i = 0; i<reply.headers.size(); i++)
		oss<<reply.headers[i]<<"\r\n";
	oss<<"\r\n";
	oss<<reply.body;
	
	os<<"length: "<<oss.str().length()<<"\n\n";
	os<<oss.str()<<flush;
	return os;
}

istream& operator>>(istream &is, CTWRaw &raw)
{
	string key, value;
	int length = -1;
	is>>ws;
    raw.ext.clear();
	while (getline(is, key, ':') && is>>ws && getline(is, value))
	{
		if ("version" == key)
		{ 
			raw.version = value;
		}
		else if ("url" == key)
		{
			raw.url = value;
		}
		else if ("date" == key)
		{ 
			raw.date = value;
		}
		else if ("data" == key)
		{ 
			raw.date = value;
		}
		else if ("ip" == key)
		{
			raw.ipaddr= value;
		}
		else if ("length" == key)
		{
			istringstream iss(value);
			if (!(iss>>length) || length <= 0)
				is.setstate(std::ios::failbit);
			break;
		}
		else 
		{
			raw.ext[key] = value;
		}
	}
	if (!is)
		return is;
	string empty;
	getline(is, empty);
	cutem buf;
	buf.reserve(length+1);
	if (!is.read(buf.ptr(), length))
		return is;
	buf.ptr()[length] = '\0';
	istringstream iss(string(buf.ptr(), length));
	CHttpReply &reply = raw.reply;
	if (!(iss>>reply.status))
		return is;
	CHeader header;
	reply.headers.clear();
	while (iss>>header)
		reply.headers.push_back(header);
	iss.clear();
	ostringstream oss;
	oss<<iss.rdbuf();
	reply.body = oss.str();
	return is;
}

bool
CTWRaw::iconv(const string &target)
{
    string source;
    if (this->get_charset_from_header(source)
        || this->get_charset_from_http_header(source)
        || this->get_charset_from_html(source)
      )
    {
        if (this->convert_charset(source, target))
        {
            this->set_charset_to_header(target);
            return true;
        }
    }
    return false;
}

bool
CTWRaw::convert_charset(const string &source, const string &target)
{
    string converted, errmsg;
    size_t result = ::convert_charset(source.c_str(), target.c_str()
       , this->reply.body, converted, errmsg);
    if (result == (size_t)-1)
        return false;
    this->reply.body = converted;
    return true;
}

bool
CTWRaw::get_charset_from_html(string &charset)
{
    charset = htmlFindEncoding2(this->reply.body.c_str(), 
             this->reply.body.length(), NULL);
    if (charset.empty())
        return false;
    return true;
}

bool
CTWRaw::get_charset_from_header(string &charset)
{
    ext_type::const_iterator cit = this->ext.find("charset");
    if (cit == this->ext.end())
        return false;
    charset = cit->second;
    return true;
}

void
CTWRaw::set_charset_to_header(const string &charset)
{
    this->ext["charset"] = charset;
}

bool
CTWRaw::get_charset_from_http_header(string &charset)
{
    charset = this->reply.headers.charset();
    if (charset.empty())
        return false;
    return true;
}
