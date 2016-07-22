#ifndef _PAT_HTTP_REPLY_H_121103
#define _PAT_HTTP_REPLY_H_121103
#include "http_bits.h"
#include <istream>
#include <fstream>
#include "TcpClient.h"
using std::ofstream;
using std::istream;

class CHttpReply{
public:
	CHttpReply();
	CHttpReply(const string &method, int max=2000000000);
	virtual ~CHttpReply()
	{}
	int set_max_size(int max);
public:
	CStatus status;
	CHeaders headers;
	string body;
public:
	friend CTCPClient& operator>>(CTCPClient& commu, CHttpReply& reply);
private:
	static int get_hex(const char* str, int n);
	int max_size;
	string req_method;
};
CTCPClient& operator>>(CTCPClient& is, CHttpReply& reply);
ostream& operator<<(ostream& os, const CHttpReply& reply);

#endif // _PAT_HTTP_REPLY_H_121103
