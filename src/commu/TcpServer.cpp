/**********************************************************************
 * @ On linux platform, "errno" is not declared if not include "errno.h",
 *   so I include it.				11/03/2003 09:46
 * @ A newly implemented method in CTCPCommu is used to receive a msg. And
 *   a "\n" is added to the end of a sending msg.
 *						10/20/2003 21:55
 * @ When server receive a message, he assume it's not longer than 128 bytes.
 *						10/18/2003 22:43
 * @ The Server just listens the socket from port 8055 now.
 *						10/18/2003 22:38
 * @ To show the usage of class CTCPServer, I write a demo program in _TEST.
 *   First, Server listens to a port and accepts a connection with the client.
 *   Server echos every message he got from the client, and send back a msg
 *   "What are you saying?". After 10 times, maybe the client is tired, server
 *   says sorry to client "Sorry, It is just a joke. I can hear you.", then
 *   server close the connection.		10/18/2003 22:35
 ************************************************************************/

#include "TcpServer.h"
#include <stdexcept>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

using std::runtime_error;

CTCPServer::CTCPServer()
{
	listen_sock = -1;
}

CTCPServer::~CTCPServer()
{
	if (listen_sock >= 0)
		::close(listen_sock);
}

int CTCPServer::listen(int port)
{
	struct sockaddr_in srv_addr;
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_sock < 0) {
		throw runtime_error(
			string("CTCPServer::listen():socket():")
		       	+strerror(errno)
		);
		return -1;
	}
	bzero(&srv_addr, sizeof(struct sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port=htons(port);
    	srv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    	if (bind(listen_sock,(struct sockaddr *)&srv_addr,sizeof(srv_addr))<0){
		throw runtime_error(string("listen():bind():")
			+strerror(errno));
       		return (-1);
    	}
	if (::listen(listen_sock,5)<0){
		throw runtime_error(string("listen()::listen():")
			+strerror(errno));
		return -1;
	} else 
		return 0;
}

void
CTCPServer::stop_listen()
{
	::close(listen_sock);
	listen_sock = -1;
}

int CTCPServer::accept()
{
    	struct sockaddr_in client;
	socklen_t addrlen;
	addrlen=sizeof(client);
    	int &sockfd = _M_tcpbuf.sockfd;
	sockfd = ::accept(listen_sock,(struct sockaddr*)&client,&addrlen);
    	if (sockfd<0){
		throw runtime_error(string("listen()::accept():")
			+strerror(errno));
       		return (-1);
	} else
        	return (0);
}

