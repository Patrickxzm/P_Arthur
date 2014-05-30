/****************************************************************************
 * @ Test code of CHTMLRef is lost, so I have to write a new one.
 *						01/27/2004	00:31
 * @ Add a new method to use CHTMLRef, user needn't to know about CHTMLLexic.
 *						01/27/2004	23:45
 ****************************************************************************/
#include "htmlref.h"
#include "tag.h"
#include "text.h"
#include "refresh.h"
#include "comment.h"
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <cstring>
#include "util/cutil.h"
#include "util/util.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif

CHTMLRef::CHTMLRef(const char* html, int len, const char *baseurl)
	:_lexic(html, len), ref(0), in_script(false), in_style(false)
{
	this->lexic = &_lexic;
	base = new CURL(baseurl);
}

CHTMLRef::CHTMLRef(const string &html, const string &base)
	:_lexic(html.c_str(), html.size()), ref(0), in_script(false), in_style(false)
{
	this->lexic = &_lexic;
	this->base = new CURL(base);
}

CHTMLRef::CHTMLRef(const char* html, int len, const CURL &base)
	:_lexic(html, len), ref(0), in_script(false), in_style(false)
{
	this->lexic = &_lexic;
	this->base = new CURL(base);
}


CHTMLRef::CHTMLRef(CHTMLLexic *lexic, const string &baseurl)
	:ref(0), in_script(false), in_style(false)
{
	assert(lexic!=NULL);
	this->lexic = lexic;
	base = new CURL(baseurl);
}

CHTMLRef::~CHTMLRef()
{
	if (base!=NULL) 
		delete(base);
	if (ref != NULL)
		delete(ref);
	Refbuf::iterator ppref;
	for (ppref=refs.begin(); ppref<refs.end(); ppref++){
		delete(*ppref);
	}
}

auto_ptr<CRef> CHTMLRef::refer()
{
	if (!refs.empty()){
		CRef* pref = refs.back();
		refs.pop_back();
		return auto_ptr<CRef>(pref);
	}
	CRef *ret = NULL;
	auto_ptr<CHTMLItem> pitem;
	for (pitem = lexic->output(); pitem.get(); pitem = lexic->output()) 
	{
		if (pitem->type()==Html_Tag){
			CTag* ptag = (CTag*)pitem.get();
			//do_tag(ptag);
			if (tolower(ptag->Name()) == "script") {
				ret = do_tag_script(ptag);
				if (ret != NULL)
					break;
				in_script = true;
			}
			else if (tolower(ptag->Name()) == "area") {
				ret = do_tag_area(ptag);
				if (ret != NULL)
					break;
			}
			else if (tolower(ptag->Name()) == "img") {
				ret = do_tag_img(ptag);
				if (ret != NULL)
					break;
			}
			else if (tolower(ptag->Name()) == "frame"
				|| tolower(ptag->Name()) == "iframe")
			{
				ret = do_tag_frame(ptag);
				if (ret != NULL)
					break;
			}
			else if (tolower(ptag->Name()) == "meta") {
				ret = do_tag_meta(ptag);
				if (ret != NULL)
					break;
			}
			else if (tolower(ptag->Name()) == "a")
			{
				ret = do_tag_a(ptag);
				if (ret!=NULL)
					break; 
			}
			else if (tolower(ptag->Name()) == "/a"
				|| tolower(ptag->Name()) == "td"
				|| tolower(ptag->Name()) == "/td"
				|| tolower(ptag->Name()) == "tr"
				|| tolower(ptag->Name()) == "/tr"
				|| tolower(ptag->Name()) == "table"
				|| tolower(ptag->Name()) == "/table"
				|| tolower(ptag->Name()) == "/script"
				) {
				ret = do_tag_end(ptag);
				if (ret!=NULL)
					break; 
			}
			else if (tolower(ptag->Name()) == "base" 
				&& !ptag->Attr("href").empty())
			{
				string base_url = ptag->Attr("href");
				CHTMLItem::chunk_blank(base_url);
				CURL *tmp = new CURL(base_url);
				if (tmp->protocol() == "http"){ /* replace base url */
					delete(base);
					base = tmp;
				} else 
					delete(tmp);
			}
		} 
		else if (pitem->type() == Html_Text){
			CText* ptext = (CText*)pitem.get();
			if (ref==NULL) 
				continue;
			if (ptext->content() == NULL)
				continue;
			string content = ptext->content();
			html_descape(content);
			ref->addref(content.c_str());
		} 
		else if (pitem->type() == Html_Comment)
		{} //nothing to do. :)
	}
	if (ret == 0 && ref != 0)
	{
		ret = ref;
		ref = 0;
	}
	return auto_ptr<CRef>(ret);
}

CRef* CHTMLRef::do_tag_area(CTag* ptag)
{
	if (ptag->Attr("href").empty())
		return 0;
	string url;
	string href = ptag->Attr("href");
	CHTMLItem::chunk_blank(href);
	html_descape(href);
	CURL newurl(href, base);
	CRef* ret = new CRef(newurl.str(), ptag->pos(), ptag->Name());
	if (ptag->Attr("alt").empty())
		return ret;
	string alt_v = ptag->Attr("alt");
	html_descape(alt_v);
	ret->addref(alt_v.c_str());
	if (ref!=0) 
	{
		refs.push_back(new CRef(ref->url(), ref->pos(), ref->tagName()));
		refs.back()->addref(alt_v.c_str());
	}
	return ret;
}

CRef* CHTMLRef::do_tag_img(CTag* ptag)
{
	if (ptag->Attr("src").empty())
		return NULL;
	string url;
	string src = ptag->Attr("src");
	CHTMLItem::chunk_blank(src);
	html_descape(src);
	CURL newurl(src, base);
	//url=base->newurl(src.c_str());
	CRef* ret = new CRef(newurl.str(), ptag->pos(), ptag->Name());
	if (ptag->Attr("alt").empty())
		return ret;
	string alt_v = ptag->Attr("alt");
	html_descape(alt_v);
	ret->addref(alt_v.c_str());
	if (ref!=0) 
	{
		refs.push_back(new CRef(ref->url(), ref->pos(), ref->tagName()));
		refs.back()->addref(alt_v.c_str());
	}
	return ret;
}

CRef* CHTMLRef::do_tag_frame(const CTag* ptag)
{
	if (ptag->Attr("src").empty()) 
		return NULL;
	string src_v = ptag->Attr("src");
	html_descape(src_v);
	CHTMLItem::chunk_blank(src_v);
	CURL url(src_v, base);
	//string url = base->newurl(src_v.c_str());
	CRef* ret = new CRef(url.str(), ptag->pos(), ptag->Name());
	ret->addref(ptag->Attr("name").c_str());
	return ret;
}

CRef* CHTMLRef::do_tag_script(CTag* ptag)
{
	if (ptag->Attr("src").empty())
		return NULL;
	string src = ptag->Attr("src");
	CHTMLItem::chunk_blank(src);
	html_descape(src);
	CURL newurl(src, base);
	CRef *ret = new CRef(newurl.str(), ptag->pos(), ptag->Name());
	return ret;
}

CRef* CHTMLRef::do_tag_a(CTag* ptag)
{
	CRef* ret=ref;
	ref=NULL;
	if (ptag->Attr("href").empty())
		return ret;
	string href_v = ptag->Attr("href");
	CHTMLItem::chunk_blank(href_v);
	html_descape(href_v);
	CURL url(href_v, base);
	ref = new CRef(url.str(), ptag->pos(), ptag->Name());
	return ret;
}

CRef* CHTMLRef::do_tag_meta(CTag* ptag)
{
	string content;
	if (ptag->Attr("http-equiv").empty()
		|| strcasecmp(ptag->Attr("http-equiv").c_str(), "refresh") != 0
		|| ptag->Attr("content").empty()
		|| (content=ptag->Attr("content")).empty()
		)
		return NULL;
	char* tmp = url_in_refresh(content.c_str());
	if (tmp==NULL)
		return NULL;
	string urlstr = tmp;
	free(tmp);
	CHTMLItem::chunk_blank(urlstr);
	html_descape(urlstr);
	CURL url(urlstr, base);
	CRef* ret = new CRef(url.str(), ptag->pos(), ptag->Name());
	return ret;
}

CRef* CHTMLRef::do_tag_end(CTag* ptag)
{
	in_script = false;
	CRef* ret = ref;
	ref = NULL;
	return ret;
}

vector<CRef>
CHTMLRef::links()
{
	vector<CRef> result;
	auto_ptr<CRef> ref;
	while ((ref=refer()).get())
		result.push_back(*ref);
	return result;
}
