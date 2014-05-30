/********************************************************************
 * @ Change the "Accept" header to any and any.
 *					02/08/2004	22:09 
 * @ Take it out from http. 		12/11/2003	23:12
 ********************************************************************/
#include "http_request.h"
#include <ctype.h>
#include <string>
#include <sstream>
#include <fstream>
#ifdef DMALLOC 
#include "dmalloc.h"
#endif
#include <stdexcept>

using std::invalid_argument;
using std::ostringstream;

CHttpRequest::CHttpRequest(const CURL& url)
{
	init(url, "GET", 0, true);
}

CHttpRequest::CHttpRequest(const CURL& url, const char *method
		,const CCookies *cookies, bool keep_alive)
{
	init(url, method, cookies, keep_alive);
}

string 
CHttpRequest::userAgent("P_Arthur.2");

int
CHttpRequest::init(const CURL& url, const char *method
		,const CCookies *cookies, bool keep_alive)
{
	this->method = method;
	if (this->method != "HEAD" && this->method != "GET")
		throw invalid_argument("Un-supported method!:"+this->method);
	local = url.localstr();
	version = "HTTP/1.1";

	headers.clear();
	ostringstream host_port;
	host_port<<url.host();
	if (url.port()!=80) 
		host_port<<':'<<url.port();
	headers.push_back(CHeader("Host", host_port.str().c_str()));
	headers.push_back(CHeader("Accept", "*/*"));
	//headers.push_back(CHeader("Accept-Encoding", "")); 
	//headers.push_back(CHeader("Accept-Charset", "utf-8")); 
	headers.push_back(CHeader("From", "xzm@net.pku.edu.cn"));
	headers.push_back(CHeader("User-Agent", userAgent));
	if (keep_alive) 
		headers.push_back(CHeader("Connection", "Keep-Alive"));
	else 
		headers.push_back(CHeader("Connection", "close"));
	if (cookies != 0)
	{	
		vector<const CCookie*> matched = cookies->match(url);
		set_cookie(matched);
	}
	return 0;
}

void 
CHttpRequest::set_cookie(const vector<const CCookie*> &coo)
{
	string value;
	for (unsigned i = 0; i<coo.size(); i++)
	{	
		if (coo[i]->name.empty())
			continue;
		if (!value.empty())
			value += "; ";
		value += coo[i]->name;
		if (coo[i]->value.size() > 0)
			value += "=" + coo[i]->value;
	}
	if (value.size() > 0)
		headers.push_back(CHeader("Cookie", value));
}


ostream& operator<<(ostream& os, const CHttpRequest& request)
{
	if (!os)
		return os;
	os<<request.method<<' '<<request.local<<' '<<request.version<<"\r\n";;
	for (unsigned i=0; i<request.headers.size(); i++)
		os<<request.headers[i]<<"\r\n";
	os<<"\r\n";
	return os;
}
