#ifndef _PAT_HTTP_H_100502
#define _PAT_HTTP_H_100502
#include "url/url.h"
#include "http_bits.h"
#include "http_request.h"
#include "http_reply.h"
//#include "url/xhostfilter.h"
#include <sstream>
#include <fstream>
/*
 * For compatibility with old version of this package, this interface is 
 * provided.
 */
class CHttp
{
public:
	typedef enum{
		OK, DNSFailure, ConnectFailure, TransferError, ProtocolError, OverRedirect
	} result_t;
public:
	CHttp(const string &urlstr, const string& cookie_file="");
	const string& get_urlstr()
	{
		return urlstr;
	}
	void timeout(int conn, int recv)
	{
		conn_timeout = conn;
		recv_timeout = recv;
	}
	virtual ~CHttp();
	result_t sfetch();
	result_t fetch();
	char* get_header_attr(const char* attr_name);
	int get_status() const;
	const char* get_body(int &len);
	string get_head();
	const char* get_location()
	{
		return location.c_str();
	}
	const char* get_ipaddress()
	{
		return ipstr.c_str();
	}
	const char* get_lastmodifytime()
	{
		return get_header_attr("Last-Modified");
	}
	const char* get_originurl()
	{
		return urlstr.c_str();
	}
	media_t mtype()
	{
		return reply.headers.mtype();
	}

public:
	CHttpReply reply;
	string urlstr;
	string location;
	string ipstr;

private:
	int conn_timeout;
	int recv_timeout;
	string __cookie_file;
};
#endif /* _PAT_HTTP_H_100502 */
