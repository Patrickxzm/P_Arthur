#include "filterMall.h"

bool
CTypeFilterMall::accept(const CURL &url)
{
	media_t type = url.mtype();
	if (url.isAbs() && (type==m_text || type==m_unknown))
		return true;
	else
		return false;
}

bool
CTypeFilterMall::accept(const CHttpReply &reply)
{
	string type = reply.headers.value("Content-Type");
	if (type.find("text") == string::npos && type.find("javascript") == string::npos)
		return false;
	else
		return true;
}
