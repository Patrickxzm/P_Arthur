#include "attr.h"
#include "htmlitem.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

CAttr::CAttr(const char* name, int len)
{
	assert(len>0);
	this->name.assign(name, len);
}

CAttr::~CAttr()
{
}

const char* CAttr::getvalue(const char* start, const char* end)
{
	value.clear();
	while(end>start && CHTMLItem::is_blank(*start))
		start++;
	if (end==start) return end;
	if (*start == '\"' || *start == '\'')  { // quoted string
		const char* ptr = start + 1;
		while (ptr<end && *ptr!=*start)  // find matched quote
			ptr++;
		if (ptr==end) {
			return end;
		} 
		///////////////////////////////////////
		//   "value string ... ... "
		//   ^                     ^
		//   start                 ptr 
		///////////////////////////////////////
		value.assign(start+1, ptr-start-1);
		return ptr+1;
	}
	const char* ptr = start+1;
	while (ptr<end && !CHTMLItem::is_blank(*ptr) && *ptr!='>')
		ptr++;
	///////////////////////////////////////////////////
	//      value ... ... string 
	//      ^                   ^
	//      start               ptr
	///////////////////////////////////////////////////
	value.assign(start, ptr-start);
	return ptr;
}

const string &CAttr::Value() const
{
	return value;
}

const string &CAttr::Name() const
{
	return name;
}

ostream &
operator<<(ostream &os, const CAttr &attr)
{
	os<<attr.name;
	if (!attr.value.empty())
		os<<" = "<<attr.value;
	return os;
}

