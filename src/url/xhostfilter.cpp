#include "xhostfilter.h"
#include "url.h"

CXHostFilter::CXHostFilter()
{
}

bool
CXHostFilter::need_dns(const char* hostname)
{
	if (test_ip() && !isIP(hostname))
		return true;
	return false;
}

bool
CXHostFilter::pass(const char* hostname, CHost *phost) 
{
	if (hostname == 0)
		return false;
	bool __pass = CHostFilter::pass(hostname);
	if (!__pass && !isIP(hostname) && test_ip())
	{
		if (cache.count(hostname) != 0)
			return cache[hostname];
		if (phost != 0)
		{
			const char* ip = phost->printable_IP();
			if (ip == 0) // DNS error!
				return false;
			__pass = CHostFilter::pass(ip);
		}
		else
		{
			CHost host(hostname);
			const char* ip = host.printable_IP();
			if (ip == 0) // DNS error!
				return false;
			__pass = CHostFilter::pass(ip);
		}
		if (cache.size() < 10000)
			cache[hostname] = __pass;
	}
	return __pass;
}

#ifdef _TEST
using namespace std;
int
main(int argc, char* argv[])
{
	CXHostFilter filter;
	int ret;
	if (argc > 1) 
	{
		ret = filter.init(argv[1]);
	} else
		ret = filter.init("range");	
	if (ret != 0)
		cerr<<"filter.init(): error!"<<endl;
	string host;
	while (cin>>host)
	{
		if (filter.pass(host.c_str()))
			cout<<"PASSED!"<<endl;
		else
			cout<<"NOT PASS!"<<endl;
	}
	return 0;
}
#endif //_TEST
