/*************************************************************************
 * @ Add members to record the server address and port.
 *						02/12/2004	18:16
 * @ In testing code(main()), a message is '\n' terminated, and the brecv
 *   (delim, len) method is used to get msg.
 *						01/05/2004	11:59
 * @ When used on Yanwang4 system, this module make process exit(by assert):
 *   When return -1 from connect, alarm(0) is not called. The next time to
 *   call alarm() will get the remaining timeout. It is not Zero, so assert
 *   (timeout==0) will failed. They are corrected. 
 * @ __errno is not used , so it is deleted.
 *						12/02/2003	12:24
 * @ You can use "-pPORT", then localhost:PORT is connected. 
 *  						11/03/2003  14:53
 * @ A newly implemented method in CTCPCommu is used to recieve msg, and a "\n"
 *   is used to terminate a msg before sending it.
 *						10/20/2003  21:58
 * @ When client receive a msg, he assume it is 128 bytes long, as the same
 *   as server.					10/19/2003  10:18
 * @ Connect with localhost(addr:"localhost") on port 8055, send msg to him,
 *   and get reply, echo the reply. If you don't want to send msg to host
 *   any more, input ctrl^d.			10/19/2003  9:51
 *************************************************************************/
#include "TcpClient.h"
#include "url/host.h"
#include <stdexcept>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <setjmp.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef DMALLOC 
#include "dmalloc.h"
#endif

using std::runtime_error;
//#define _DEBUG
#ifdef _DEBUG
using std::endl;
using std::cerr;
#endif //_DEBUG

CTCPClient::CTCPClient():conn_timeout(-1)
{
}

CTCPClient::CTCPClient(const char* addr, int port):conn_timeout(-1)
{
	string ipstr;
	if (ConnectServer(addr, ipstr, port) != 0)
		throw runtime_error("ConnectServer() failed!");
}

CTCPClient::~CTCPClient()
{
}

/* 
 * Code on how to timeout a connection is adopted from UNIX Network programming,
 * Vol.1, Ch.26.6
 */

sigjmp_buf CTCPClient::jmpbuf;
volatile sig_atomic_t CTCPClient::canjump;

void 
CTCPClient::connect_alarm(int signo)
{
	if (canjump == 0)
		return;
	canjump = 0;
	siglongjmp(jmpbuf, 1);
}

int 
CTCPClient::ConnectServer(const void *addr, int addr_len, int port)
{
	if (this->is_open())
	{
		this->setstate(std::ios::failbit);
		return -2;
	}
	int ret;
	int tv = 0;
	close();
	int &sockfd = _M_tcpbuf.sockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		this->setstate(std::ios::failbit);
		return -3;
	}
	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(struct sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);
	memcpy(&(srv_addr.sin_addr), addr, addr_len);

	typedef void (*sighandler_t) (int);
	sighandler_t old = signal(SIGALRM, connect_alarm);
	if (sigsetjmp(jmpbuf, 1)) {
		ret = -4;
		errno = ETIMEDOUT;
		goto CONN_RET;
	}
	canjump = 1;  // siglongjmp is now OK */
	if (conn_timeout>0)
		tv = alarm(conn_timeout);
	else 
		tv = alarm(0);
	ret=connect(sockfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	// ret is 0, or -1;	
CONN_RET:
	alarm(tv);	
	signal(SIGALRM, old);
	if (ret < 0) 
		this->setstate(std::ios::failbit);
	else 
		this->clear();
	return ret;
}

int
CTCPClient::ConnectServer(CHost &server, int port)
{	
	if (server.naddr() < 1)
		return -1;
	for (unsigned i=0; i<server.naddr(); i++) 
	{
#ifdef _DEBUG
		cerr<<"connect try: "<<i<<endl;
#endif //_DEBUG
		string addr = server.address();
		if (addr.empty())
			throw runtime_error("Empty net_addr string from DNS.");
#ifdef _DEBUG
		time_t start = time(0);
#endif //_DEBUG
		if (ConnectServer(addr.c_str(), addr.size(), port)==0)
		{
#ifdef _DEBUG
			cerr<<"Use "<<time(0)-start<<" seconds, Success!"<<endl;
#endif //_DEBUG
			return 0;
		}
		server.next();
#ifdef _DEBUG
		cerr<<"Use "<<time(0)-start<<" seconds, Failed!"<<endl;	
#endif //_DEBUG
	}
	return -2;
}

int
CTCPClient::ConnectServer(const char *addr, string &ipstr, int port)
{	
#ifdef _DEBUG
	time_t start = time(0);
#endif //_DEBUG
	CHost host(addr);
#ifdef _DEBUG
	cerr<<"DNS cost "<<time(0)-start<<" seconds."<<endl;
#endif //_DEBUG
	int ret;
	if ((ret=ConnectServer(host, port)) == 0)
		ipstr = host.paddr();
	return ret;
}

int CTCPClient::ConnectServer(int ipv4, int port)
{
	return ConnectServer(&ipv4, sizeof(ipv4), port);
}

void CTCPClient::timeout(int conn, int recv)
{
	conn_timeout = conn;
	_M_tcpbuf.timeout(recv);
}
