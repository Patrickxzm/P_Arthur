#include "tag.h"
#include "util/cutil.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

CTag::CTag(int pos) :CHTMLItem(Html_Tag)
{
	m_pos = pos;
}

const char* 
CTag::get(const char* start, const char* end)
{
	start++;
	const char* ptr = start+1;
	while (ptr<end && isalnum(*ptr))
		ptr++;
	this->name.assign(start, ptr-start);
	return getattrs(ptr, end);
}

CTag::~CTag()
{
	for (unsigned i = 0; i<attrlist.size(); i++)
	{
		delete attrlist[i];
	}
}

static bool seperator(char ch)
{
	if (ch==' ' || ch=='\r' || ch=='\n' || ch=='\t')
		return true;
	else return false;
}

const char* CTag::getattrs(const char* start, const char* end)
{
	assert(start<=end);
	for(;;) {
		while(start<end && seperator(*start))
			start++;
		if (start == end) return end;
		if (*start == '>') return start+1;
	
		const char* ptr = start+1;
		while (ptr<end && *ptr!='=' && *ptr!='>' && !seperator(*ptr))
			ptr++;
		CAttr *pattr = new CAttr(start, ptr-start);
		assert(pattr!=NULL);

		while (ptr<end && seperator(*ptr)) ptr++;
		if (ptr==end) 
			return end;
		if (*ptr == '=') 
			start = pattr->getvalue(ptr+1, end);
		else 
			start=ptr;
		attrlist.push_back(pattr);
	}
}

string
CTag::Attr(const string& name) const
{
	for (unsigned i=0; i<attrlist.size(); i++)
	{
		if (strcasecmp(attrlist[i]->Name().c_str(), name.c_str()) == 0)
			return attrlist[i]->Value();
	}
	return "";
}

bool 
CTag::Attr(const string& name, string &value) const
{
	for (unsigned i=0; i<attrlist.size(); i++)
	{
		if (strcasecmp(attrlist[i]->Name().c_str(), name.c_str()) == 0)
		{
			value = attrlist[i]->Value();
			return true;
		}
	}
	return false;
}

const string &CTag::Name() const
{
	return name;
}

ostream &
operator<<(ostream &os, const CTag &tag)
{
	os<<'<'<<tag.name;
	for (unsigned i=0; i<tag.attrlist.size(); i++)
	{
		os<<" "<<*tag.attrlist[i];
	}
	os<<'>';
	return os;
}
