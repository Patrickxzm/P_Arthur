#include "TcpBuf.hpp"
#include <signal.h>
#include <errno.h>

int 
TcpBuf::recv(void* data, int len)
{
	if (sockfd < 0)
		return -1;

	if (recv_timeout > 0) {
		struct timeval timeout;
		timeout.tv_sec = recv_timeout;
		timeout.tv_usec = 0;

		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		int ret;
		while ((ret=select(sockfd+1, &rset, NULL, NULL, &timeout)) < 0)
		{
			if (errno == EINTR) 
				continue;
			return -1;
		}
		if (ret==0)
		{
			errno = ETIMEDOUT;
			return -1;
		}
	}		
	return ::recv(sockfd, data, len, 0);
}

int 
TcpBuf::send(const void* data, unsigned len)
{
	if (sockfd < 0) 
		return -1;
	int num = 0;
	while (len > 0) { 
		void (*handler)(int);
		handler = signal(SIGPIPE, SIG_IGN);
		int l = ::send(sockfd, data, len, 0);
		signal(SIGPIPE, handler);
		if (l <= 0) 
			return -1;
		data = (char*)data + l;
		len -= l;
		num += l;
	}
	return num;
}

