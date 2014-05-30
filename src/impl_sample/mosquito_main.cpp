#include "mosquito.h"
#include <iostream>

using namespace std;
class my_headers_filter : public headers_filter
{
public:
	bool operator()(const CURL *url, const CHeaders *headers)
	{
		return true;
	}
} my_headers_filter;

class my_link_filter : public link_filter
{
public:
	bool operator()(const CRef* ref, const CURL *url)
	{
		if (url == 0)
			return false;
		return true;
	}
} my_link_filter;

class my_CDealer : public CDealer
{
public:
	void deal(workbench_t &bench);
private:
} my_CDealer;

void 
my_CDealer::deal(workbench_t &bench)
{
	const vector<CRef*> &links = bench.links;
	for (unsigned i=0; i<links.size(); i++)
	{
#if 0
		if (is_abri_name_flag(links[i]->url()))
		{
			if (links[i]->ref() == 0)
				continue;
			abri<<links[i]->ref()<<endl;
			continue;
		}
		if (is_company_name_flag(links[i]->url()))
		{
			if (links[i]->ref() == 0)
				continue;
			company<<links[i]->ref()<<endl;
			continue;
		}
#endif // 0
	}	
	return;
}	

int
main()
{
	cout<<"This is a mosquito for conameu pages on yahoo!"<<endl; 
	CURLQue urlque(24);
	CMosquito mosquito(&urlque, &my_headers_filter, &my_link_filter);
	mosquito.add_dealer(&my_CDealer);
	mosquito.run();
}
