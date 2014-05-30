#ifndef _PAT_REFILTER_H_022104
#define _PAT_REFILTER_H_022104
class link_filter 
{
public:
	virtual bool operator()(const CRef* ref, const CURL *url) = 0;
	virtual ~link_filter() {}
};

class headers_filter 
{
public:
	virtual bool operator()(const CURL *url, const CHeaders *headers)  = 0;
	virtual ~headers_filter() {}
};

class textonly_content_type : public headers_filter
{
public:
	virtual bool operator()(const CURL *url, const CHeaders *headers) 
	{
		if (url==0)
			return false;
		if (headers==0)
			return false;
		if (url->mtype() == m_text || headers->mtype() == m_text)
			return true;
		return false;		
	}
};

class star_textonly_link : public link_filter
{
public:
	virtual bool operator()(const CRef* ref, const CURL *url) 
	{
		if (url==0)
			return false;
		if (url->mtype() == m_text || url->mtype() == m_unknown)
			return true;
		return false;		
	}
};

#endif // _PAT_REFILTER_H_022104
