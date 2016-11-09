#ifndef _PAT_HTTP_REQUEST_H_121103
#define _PAT_HTTP_REQUEST_H_121103
#include <vector>
#include "url/url.h"
#include "http_bits.h"
#include "cookie.h"

using std::vector;

class CHttpRequest
{
public:
	CHttpRequest()
	{}
	virtual ~CHttpRequest()
	{}
	CHttpRequest(const CURL& url, const string& method="GET"
		, const CCookies *cookies=0, bool keep_alive=true);
	int init(const CURL& url, const string& method
		, const CCookies *cookies, bool keep_alive);
	const string &get_method() const
	{
		return method;
	}
	friend ostream& operator<<(ostream& os, const CHttpRequest& request); 
	static string userAgent;
private:
	void set_cookie(const vector<const CCookie*> &coo);
	//string request_line;
	string method;
	string local;
	string version;
	CHeaders headers;
};
ostream& operator<<(ostream& os, const CHttpRequest& request); 

#endif // _PAT_HTTP_REQUEST_H_121103
