#include "encapsuled.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util/cutil.h"
#include "util/util.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif


CEncapsuled::CEncapsuled(auto_ptr<CHTMLItem> tag):CHTMLItem(Html_Encap)
{
	assert(tag.get() && tag->type()==Html_Tag);
	m_pos = tag->pos();
	this->first = tag;
}

CEncapsuled::~CEncapsuled()
{
	if (str!=NULL)
		free(str);
}

const char* CEncapsuled::get(const char* start, const char* end)
{
	const char *end_tag = 0;
	string tagName = tolower(((CTag *)first.get())->Name());
	if (tagName == "script")
		end_tag = "</script>";
	else if (tagName == "style")
		end_tag = "</style>";
	else
		assert(false);
	const char* tmp;
	for (tmp = start; tmp + strlen(end_tag) <= end; tmp++)
	{
		if (strncasecmp(tmp, end_tag, strlen(end_tag))==0) 
			break;
	}
	if (tmp + strlen(end_tag) <= end) {
		str = (char*)malloc(tmp - start +1);
		assert(str!=NULL);
		memcpy(str, start, tmp-start);
		str[tmp-start] = '\0';
		return tmp + strlen(end_tag);
	}
	str = (char*)malloc(end - start + 1);
	assert(str!=NULL);
	memcpy(str, start, end-start);
	str[end-start] = '\0';
	return end;
}

const char* CEncapsuled::content() const
{
	return str;
}

using std::endl;

ostream &
operator<<(ostream &os, const CEncapsuled &scr)
{
	if (scr.first.get())
		os<<*((CTag*)scr.first.get())<<endl;
	os<<scr.str;
	return os;
}
