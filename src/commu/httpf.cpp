#include "http.h"
#include "util/arg.h"
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <fstream>

ostream& help(ostream &os);
using namespace std;

int main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (arg.find1("-h") || arg.find1("--help"))
	{
		help(cout);
		return 1;
	}
	CArg::ArgVal val;
	const char* cookie_fn = arg.find1("--cookie-file=");
	string urlstr;
	if (val=arg.find1("--url="))
		urlstr = string(val);
	CHttp http(urlstr, cookie_fn);
	http.timeout(30,15);
	int ret = http.fetch();
	switch (ret) {
	case -1: 
		cerr<<"Bad url!\n";
		break;
	case -2:
		cerr<<"connect failed!\n";
		break;
	case -3:
		cerr<<"request failed!\n";
		break;
	case -4:
		cerr<<"reply failed!\n";
		break;
	case -5:
		cerr<<"too many redirect!\n";
		break;
	}
	if (ret != 0)
		return ret;
	int body_len;
	const char* body = http.get_body(body_len);
	cout<<http.get_location()<<endl;
	cout<<http.get_head()<<endl;
	cout<<"body_len ="<< body_len<<endl;
	if (body != 0)
		cout.write(body, body_len);
	cout<<"\r\n";
	return 0;
}

ostream& 
help(ostream &os)
{
	os<<"CHttp test program, fetch a page.\n"
	  "Usage: Cmd --url= [--cookie-file= ] [--help|-h]\n"
	  "\t--url= : Fetch a page by a url, following redirections.\n"
	  "\t--cookie-file= : Read/Write cookies from/to this file.\n"
	  "\t--help|-h  : Print this message.\n";
	return os;
}

