/************************************************************************
 * @ In Keep_Alive mode, Does an empty-line need to be sent after a http-
 *   request? The answer is NO--use sniffer to detect IE communicate data.
 *					02/09/2004	08:55
 * @ After transfering http reply data successfully, some web-server close
 *   the Tcp connection. The sockfd is "-1" in CTCPCommu, but the http
 *   request is well acomplished.
 *					02/01/2004	16:26
 * @ Http package can handle more than one request method now. The method
 *   can be "HEAD", other than "GET".
 *					01/23/2004	00:09
 ************************************************************************/

#include "http.h"
#include "TcpClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#ifdef DMALLOC
#include "dmalloc.h"
#endif
using std::flush;

CHttp::CHttp(const string &urlstr, const string& cookie_file)
	: conn_timeout(-1), recv_timeout(-1), __cookie_file(cookie_file)
{
	this->urlstr=urlstr;
	location = urlstr;
}

CHttp::~CHttp()
{
}

using std::ios;

CHttp::result_t
CHttp::sfetch()
{
	CURL url(location);
	if (url.protocol() != "http")
		return ProtocolError;
	CTCPClient client;
	CHost server(url.host());
	client.timeout(conn_timeout, recv_timeout);
	int res;
	if ((res=client.ConnectServer(server, url.port())) == 0)
		ipstr = server.paddr();
	else if (res == -1)
		return DNSFailure;
	else if (res == -2)
		return ConnectFailure;
	CCookies cookies;
	if (__cookie_file.size()>0)
	{
		cookies.load(__cookie_file.c_str());
		if (!(client<<CHttpRequest(url, "GET", &cookies, false)<<flush))
			return TransferError;
	}
	else
	{
		if (!(client<<CHttpRequest(url, "GET", 0, false)<<flush))
			return TransferError;
	}
	if (!(client>>reply))
	{
		return TransferError;
	}
	if (__cookie_file.size()>0)
	{
		vector<string> cookie_headers;
		cookie_headers = reply.headers.values("Set-Cookie");
		if (cookie_headers.size() > 0)
		{
			for (unsigned i = 0; i<cookie_headers.size(); i++)
				cookies.add(CCookie(cookie_headers[i]));
			cookies.save(__cookie_file.c_str());
		}
	}
	return OK;
}

CHttp::result_t
CHttp::fetch()
{
	result_t res;
	int nstep = 0;
	while ((res=sfetch()) == OK) {
		int status = get_status();
		if (status<300 || status>=400)
			return OK;
		CURL base(location);
		location=base.newurl(reply.headers.value("Location").c_str());
		if (nstep++ > 3)
			return OverRedirect;
	}
	return res;
}

char* CHttp::get_header_attr(const char* attr_name)
{
	const string& value = reply.headers.value(attr_name);
	if (value.empty())
		return 0;
	else return strdup(value.c_str());
}
	
int CHttp::get_status() const
{
	return reply.status.code;
}

const char* CHttp::get_body(int &len)
{
	len = reply.body.size();
	if (len==0)
		return 0;
	return reply.body.c_str();
}
	
string CHttp::get_head()
{
	std::ostringstream head;
	head<<reply.status<<"\r\n";
	for(unsigned i=0; i<reply.headers.size(); i++)
		head<<reply.headers[i]<<"\r\n";
	return head.str();
}

