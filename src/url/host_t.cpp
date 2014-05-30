#include "host.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <iostream>

using namespace std;

int foo(const string &host);

int 
main(){
	string host;
	cout<<">>>>>>>>Imput an address:"<<endl;
	while (cin>>host) {
		CHost h(host);
		cout<<h<<endl;
		foo(host);
		cout<<">>>>>>>>>More address:"<<endl;
	}
	return 0;
}

int
foo(const string &host)
{
	struct addrinfo hints, *res, *res0;
	int error;
	//int s;
	//const char *cause = NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	error = getaddrinfo(host.c_str(), "http", &hints, &res0);
	if (error) {
		//errx(1, "%s", gai_strerror(error));
		cerr<<"error!"<<endl;
		return -1;
		/*NOTREACHED*/
	}
	char buf[256];
	for (res = res0; res; res = res->ai_next)
	{
		const struct sockaddr_in * p = (const struct sockaddr_in *)res->ai_addr;
		cout<<"oname:";
		if (res->ai_canonname)
			cout<<res->ai_canonname;
		cout<<endl;
		if (inet_ntop(res->ai_family, &p->sin_addr, buf, 256))
			cout<<buf<<endl;
		else
			cout<<"failed"<<endl;
	}
	freeaddrinfo(res0);
	return 0;
}
