/************************************************************************
 * @ Add a function to output a CText object.
 *						10/24/2003	10:46
 ************************************************************************/
#include "text.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

CText::CText(int pos) 
	: CHTMLItem(Html_Text), str(0)
{
	m_pos = pos;
}

CText::~CText()
{
	if (str != NULL)
		free(str);
}

const char* CText::get(const char* start, const char* end)
{
	assert(start<=end);
	const char* ptr = start+1;
	while (ptr<end && *ptr!='<') 
		ptr++;
	str = (char*)malloc(ptr-start+1);
	assert(str);
	memcpy(str, start, ptr-start);
	for (int i=0; i<ptr-start; i++)
		if (str[i] == '\0')
			str[i] = ' ';
	str[ptr-start] = '\0';
	return ptr;
}

const char* CText::content() const
{
	return str;
}

ostream &operator<<(ostream &os, const CText &text)
{
	os<<text.str;
	return os;
}
