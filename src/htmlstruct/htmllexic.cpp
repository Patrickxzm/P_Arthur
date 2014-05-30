/*************************************************************************
 * @ Two mode of using CHTMLLexic: copying htmltext and non-copying. Copying 
 *   is more secure but resource-costing, so most time it is useless.
 *					01/26/2004	11:29
 * @ I have to produce an excutable file to test this html lexic module.
 *					10/23/2003	15:13
 *************************************************************************/
#include "htmllexic.h"
#include "tag.h"
#include "comment.h"
#include "text.h"
#include "encapsuled.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "util/cutil.h"
#include "util/util.h"


CHTMLLexic::CHTMLLexic() : start(0), ptr(0), end(0), html(0)
{
}

CHTMLLexic::CHTMLLexic(const char* html, int len) : html(0)
{
	input(html, len);
}

CHTMLLexic::CHTMLLexic(const string &html) : html(0)
{
	input(html);
}

CHTMLLexic::~CHTMLLexic()
{
	if (html!=NULL) free(html);
}

int CHTMLLexic::input(const char* html, int len)
{
//Don't use strndup() here, there may be '\0' in a text/html. 
	if (this->html != 0)
		delete(this->html);
	this->html = new char[len+1];
	memcpy(this->html, html, len);
	this->html[len] = '\0';
	ptr = start = this->html;
	end = start+len;
	return 0;
}

int 
CHTMLLexic::input(const string &html)
{
	return input(html.c_str(), html.length());
}

static bool _isblank(char ch)
{
	if (ch==' ' || ch=='\t' || ch=='\r' || ch=='\n')
		return true;
	else return false;
}

auto_ptr<CHTMLItem> CHTMLLexic::output()
{
	auto_ptr<CHTMLItem> r;
	if (start == 0)
		return r;
	while (ptr < end && _isblank(*ptr)) 
		ptr++;
	if (ptr >= end)
		return r;

	if (ptr+1<end && ptr[0]=='<' 
		&& (ptr[1]=='/' || isalnum(ptr[1]))
	)
	{
		r = auto_ptr<CHTMLItem>(new CTag(ptr-start));
		ptr = r->get(ptr, end);
		string tagName = tolower(((CTag*)r.get())->Name());
		if ( tagName == "script" || tagName == "style")
		{
			r = auto_ptr<CHTMLItem>(new CEncapsuled(r));
			ptr = r->get(ptr, end);
		}
		return r;
	}

	if (ptr+1<end && ptr[0]=='<' && ptr[1]=='!')
	{
		r = auto_ptr<CHTMLItem>(new CComment(ptr-start));
		ptr = r->get(ptr, end);
		return r;
	}

	r = auto_ptr<CHTMLItem>(new CText(ptr-start));
	ptr = r->get(ptr, end);
	return r;
}

