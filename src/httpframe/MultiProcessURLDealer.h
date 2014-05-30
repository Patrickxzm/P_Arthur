#ifndef CMULTIPROCESSURLDEALER_XZM_052800
#define CMULTIPROCESSURLDEALER_XZM_052800

#include "WPageDealer.h"
#include "URLSource.h"
#include "msgq.h"

extern int max_nprocss;
class CMultiProcessURLDealer 
{
public:
	CMultiProcessURLDealer(CURLSource &source, CWPageDealer &dealer
			, int nprocess, CXHostFilter *xfilter = 0);
	int init(const char* path1, const char* path2);
	int run();
	void set_outstream(FILE* trace, FILE* debug=NULL);

	virtual ~CMultiProcessURLDealer();
private:
	CURLSource* psrc;
	CWPageDealer* pdealer;
	int nprocess;
	CMsgQue msgq;
	FILE *trace;
	FILE *debug;
	char* path1;
	char* path2;
private:
	CXHostFilter *xfilter;
};
#endif /* CMULTIPROCESSURLDEALER_XZM_052800 */
