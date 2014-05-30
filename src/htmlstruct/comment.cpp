#include "comment.h"
#include "util/cutil.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

CComment::CComment(int pos)
	:CHTMLItem(Html_Comment), str(0)
{
	m_pos = pos;
}

CComment::~CComment()
{
	if (str != NULL)
		free(str);
}

const char* CComment::get(const char* start, const char* end)
{
// start : <!...
	assert(start[0]=='<' && start[1]=='!');
	const char* ptr=0;
	const char* end_string = ">";
	assert(end>=start);
	if (start+3 < end && start[2] == '-' && start[3] == '-')
	{
		end_string = "-->";
	}

	if ((ptr = strnstr(start, end_string, end-start)) == 0)
	{
		end_string = ">";
		ptr = strnstr(start, end_string, end-start);
	}
	if (ptr == 0)  			// comment not ended.
		ptr = end;
	else if (*ptr == '-')		// end_string is "-->".
		ptr += 3;
	else 				// end_string is ">".
		ptr += 1; 
	str = (char*)malloc(ptr - start + 1);
	assert(str);
	memcpy(str, start, ptr -start);
	str[ptr - start] = '\0';
	return ptr;
}

const char* CComment::content() const
{
	return str;
}

using std::endl;

ostream &
operator<<(ostream &os, const CComment &com)
{
	os<<com.str;
	return os;
}
