#ifndef FILTER_MALL_H_05082010
#define FILTER_MALL_H_05082010
#include "url/url.h"
#include "http_reply.h"

// Tell what type of urls or replies are wanted by infomall.
// used in httpf1;
// used in mosquito;

class CTypeFilterMall
{
public:
	static bool accept(const CURL &url);
	static bool accept(const CHttpReply &reply);
};
#endif // FILTER_MALL_H_05082010
