#ifndef _CURLFILTER_XZM_062402
#define _CURLFILTER_XZM_062402

#include "url.h"
class CURLFilter {
public:
	CURLFilter();
	virtual ~CURLFilter();
	virtual int pass(const char* urlstr);
	virtual int pass(CUrl* url);
private:
};
#endif /* _CURLFILTER_XZM_062402 */
