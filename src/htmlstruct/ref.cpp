/**************************************************************************
 * @ As requested by SuHang, I Add routines about what tag a ref comes out 
 *   from. 					01/28/2004	21:15
 **************************************************************************/
#include "ref.h"
#include "htmlitem.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif


CRef::CRef(const string &url, int pos, const string &tagName)
	:_pos(pos), _tagName(tagName)
{
	_url = url;
}

CRef::~CRef()
{
}

void CRef::addref(const char* ref)
{
	if (ref==NULL) 
		return;
	if (strlen(ref)==0) 
		return;
	if (_ref.empty())
	       _ref = ref;
	else
	{
		_ref += ' ';
		_ref += ref;
	}	
	CHTMLItem::compress_blank(_ref);
}

ostream &operator<<(ostream &os, const CRef &ref)
{
	os<<ref.url()<<" <== ";
	os<<ref.ref();
	os<<" : "<<ref.pos()<<" : <"<<ref.tagName()<<'>';
	return os;
}

