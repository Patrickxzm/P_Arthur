/**********************************************************************
 * @ Add code to deal with client closing the socket.
 *						11/05/2003 14:09
 * @ I read a book "UNIX network programming V1", and it's very useful. 
 *   The book provide a server mode, it use select to do with many clients
 *   within one process. So I copy it into my CUltraServer.
 *						11/04/2003 16:14
 * @ Before accepting one new connection, the ultra-server has to listen to
 *   socket again ?? I'm choked!		11/03/2003 15:04 
 * @ Using "-u" option means testing CUltraServer, the UltraServer accept
 *   connections from clients by listening to port 8056. Getting a msg from 
 *   one socket, and reply the message with the client's number.
 *   						11/03/2003 14:38 
 ************************************************************************/

#include "UltraServer.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#ifdef DMALLOC 
#include "dmalloc.h"
#endif

using std::ostringstream;

CUltraServer::CUltraServer(int port)
{
	listen(port);
	FD_ZERO(&allset);
	FD_SET(listen_sock, &allset);
	max_sock = listen_sock;
	delim = "\n";
}

CUltraServer::~CUltraServer()
{
	while (!msgs.empty())
	{
		delete(msgs.front().data);
		msgs.pop();
	}
	map<int, string*, less<int> >::iterator it;
	for (it = clients.begin(); it != clients.end(); it++)
	{
		delete(it->second);
	}
	clients.clear();
}

string *
CUltraServer::rmsg(int &sock)
{
	while (msgs.empty())
	{
		select();	
	}
	string *ret = msgs.front().data;
	sock = msgs.front().sockfd;
	msgs.pop();
	return ret;
}

void 
CUltraServer::select()
{
	fd_set rset = allset;		// structure assignment	
	int nready ;
	while ((nready=::select(max_sock+1, &rset, 0, 0, 0)) < 0)
	{
		if (errno != EINTR) 
			throw UltraErr(string("select():select():")
				+ strerror(errno));
	}
	if (FD_ISSET(listen_sock, &rset)) // new client connection
	{
		int &sockfd = _M_tcpbuf.sockfd;
		accept();
		clients[sockfd] = 0;
		FD_SET(sockfd, &allset);
		if (sockfd > max_sock)
			 max_sock = sockfd;
		sockfd = -1;
		if (--nready <= 0) 
			return ; 
	}
	int count = 0;
	map<int, string*, less<int> >::const_iterator it;
	for (it = clients.begin(); it != clients.end(); it ++)
	{
		int sock = it->first;
		if (!FD_ISSET(sock, &rset)) 
			continue;
		read_sock(sock);
		count ++;
		if (--nready <= 0)
			break;
	}
	return ;
}

void
CUltraServer::read_sock(int sock)
{
	const int buf_size = 128;
	char buf[buf_size];
	int len = ::read(sock, buf, buf_size);
	if (len == 0)  // the client closed its socket.
	{
		::close(sock);
		clients.erase(sock);
		FD_CLR(sock, &allset);
	}
	else if (len > 0) 
	{
		if (clients[sock] == 0) {
			clients[sock] = new string;
		}
		string indata(buf, len);
		string::size_type begin(0), end;
		while ((end=indata.find(delim, begin))!=string::npos)
		{
			clients[sock]->append(indata, begin, end-begin);
			msgs.push(__msg(clients[sock], sock));
			clients[sock] = new string;
			begin = end + delim.size();
		}
		clients[sock]->append(indata, begin, indata.size());
	}
	else if (len < 0)
	{
		throw UltraErr(string("read_sock():read():")
			+ strerror(errno));
	}
}

void
CUltraServer::wmsg(int sock, const string &str)
{
	string msg = str+delim;
	_M_tcpbuf.sockfd = sock;
	unsigned len = msg.size();
	if (!write(msg.c_str(), len))
	{
		ostringstream oss;
		oss<<"wmsg():write() failed!: msg.size()="<<msg.size();
		throw UltraErr(oss.str());
	}
	//std::cout<<"ultra::write(): len="<<len<<std::endl;
}

