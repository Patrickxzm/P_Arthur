#include "http.h"
#include "TcpClient.h"
#include "util/arg.h"
#include "tw_raw.h"
#include "filterMall.h"
#include <unistd.h>
using namespace std;

ostream&
help(ostream &os)
{
	os<<"Http client program, fetch web pages without redirect.\n"
	  "Usage : httpf1 [--method=...] [--cookie-file=...] [--with-mall]"
	  " [--tw-raw|--body-only] [--interval=...] [--retry-interval=...] [--disable-keep-alive]"
	  " [--h|--help]\n"
	  "\t--method= : the mothod you want in a http request (default:\"GET\").\n"
	  "\t--cookie-file= : write/read cookies to/from file.\n"
	  "\t--with-mall : output pages is requested by infomall, so page media type is checked.\n" 
	  "\t--tw-raw : output in TianWang raw format.\n"
	  "\t--body-only : output body only.\n"
	  "\t--interval= : interval in second between HTTP requests (default:1).\n"
	  "\t--retry-interval= : interval after a request failture before retry (default:600).\n" 
	  "\t--disable-keep-alive : use a new connection to get every page.\n"
	  "\t-h|--help : Print this help message.\n"
	  "\tstdin : url strings.\n"
	  "\tstdout : web pages.\n"
	  <<endl;
	return os; 
}

inline int 
fetch(CTCPClient &client, CHost &server, unsigned port
	, const CHttpRequest &request, CHttpReply &reply
	, bool keep_alive)
{
	if ((!client || !keep_alive) && client.is_open())
		client.close();
	if (!client.is_open())
	{
		clog<<"connect (in "<<client.conn_timeout<<" seconds)..."<<flush;
	  	switch (client.ConnectServer(server, port))
		{
		case -1 :
			clog<<"DNS failure."<<endl;
			return -3;  //DNS failed.
		case -2 :
			clog<<"failed."<<endl;
			return -1; //connect failed.
		default : 
			clog<<"OK."<<endl;
		}
	}
	client<<request<<flush;
	if (!(client>>reply))
	{
		clog<<"transfer failed."<<endl;
		return -2;
	}
	return 0;
}

int 
main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	if (arg.found("-h") || arg.found("--help"))
	{
		help(cout);
		return 1;
	}
	
	bool with_mall = arg.found("--with-mall");

	string cookie_file;
	CCookies cookies;
	if (arg.findLast("--cookie-file=", cookie_file))
		cookies.load(cookie_file.c_str());
	string method = "GET";
	arg.findLast("--method=", method);
	bool tw_raw = arg.found("--tw-raw");
	bool body_only = arg.found("--body-only");
    string strNum;
    unsigned interval = 1;
    if (arg.findLast("--interval=", strNum))
        interval = stoul(strNum);
    unsigned retry_interval = 600;
    if (arg.findLast("--retry-interval", strNum))
        retry_interval = stoul(strNum);
    bool keep_alive = !arg.found("--disable-keep-alive");
	string host;
	int port=-1;
	string urlstr;
	CTCPClient client;
	client.timeout(10, 10);
	CHost hostaddr;
	while (cin>>urlstr)
	{
		clog<<urlstr<<endl;
		CURL url(urlstr);
		if (url.protocol() != "http" || url.host().empty())
		{
			cerr<<"error url : "<<urlstr<<endl;
			continue;
		}
		if (with_mall && !CTypeFilterMall::accept(url))
		{
			cerr<<"URL rejected by infomall : "<<urlstr<<endl;
			continue;
		}
		if (host != url.host() || port != url.port())
		{
			host = url.host();
			port = url.port();
			if (!hostaddr.DNS(host.c_str()))
			{
				cerr<<"DNS failed!"<<host<<endl;
				return -3;
			}
			if (client.is_open())
				client.close();
		}
		CHttpRequest request;
		if (!cookie_file.empty())
			request.init(url, method.c_str(), &cookies, keep_alive);
		else 
			request.init(url, method.c_str(), 0, keep_alive);
		CHttpReply reply(request.get_method());
		int ret;
		for (unsigned i=0; i<3; i++)
		{
			if (i > 0)
			{
				clog<<"wait for "<<retry_interval<<" seconds until next retry..."
				   <<endl;
				sleep(retry_interval);
			}
			ret = fetch(client, hostaddr, port, request, reply, keep_alive);
			if (ret == 0)
				break;
		}
		if (ret != 0)
		{
			clog<<"Give up to fetch => "<<urlstr<<endl;
			continue;
		}
		if (!cookie_file.empty())
		{
			vector<string> cookie_headers;
			cookie_headers = reply.headers.values("Set-Cookie");
			for (unsigned i = 0; i<cookie_headers.size(); i++)
				cookies.add(CCookie(cookie_headers[i]));
			cookies.save(cookie_file.c_str());
		}
		if (with_mall && !CTypeFilterMall::accept(reply))
		{
			cerr<<"Reply rejected by infomall : "<<urlstr<<endl;
			continue;
		}
		if (tw_raw)
		{
			CTWRaw raw(urlstr, hostaddr.paddr(), reply);
			cout<<raw<<endl;
		}
		else if (body_only)
		{
			cout<<reply.body<<flush;
		}
		else
		{
			cout<<reply<<endl;
		}
		clog<<"Done"<<endl;
		if (interval > 0)
			sleep(interval);
	}
	return 0;
}
catch(exception &e)
{
	cerr<<e.what()<<endl;
	return -1;
}

