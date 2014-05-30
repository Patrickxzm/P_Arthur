/***************************************************************************
 * @ A TCPClient import socket from outside is not elegant, so a constructer:
 *   "CTCPClient::CTCPClient(int sockfd)" is take out.
 *					10/26/2003	11:34
 ***************************************************************************/
#ifndef _CTCPCLIENT_PAT_080202
#define _CTCPCLIENT_PAT_080202
#include "TcpStream.hpp"
#include "url/host.h"
#include <string>
#include <setjmp.h>
#include <signal.h>
using std::string;

class CTCPClient : public TcpStream 
{
public:
	enum{
		Conn_OK, DNS_Fail, Conn_Fail
	} connect_result;
	CTCPClient();
	virtual ~CTCPClient();
	CTCPClient(const char* addr, int port=80);

	int ConnectServer(const char* addr, string &ipstr, int port=80);
	int ConnectServer(CHost &server, int port=80);
	int ConnectServer(int ipv4, int port);
	int ConnectServer(const void *addr, int addr_len, int port);
	void timeout(int conn, int recv);
	int conn_timeout;
private:
	static sigjmp_buf jmpbuf;
	static volatile sig_atomic_t canjump;
private:
	static void connect_alarm(int signo);
};

#endif /*_CTCPCLIENT_PAT_080202*/
