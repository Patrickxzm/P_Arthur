#ifndef _PAT_HTML_REF_H_102102
#define _PAT_HTML_REF_H_102102

#include "htmllexic.h"
#include "ref.h"
#include "tag.h"
#include "url/url.h"
#include <vector>
#include <memory>
using std::auto_ptr;

class CHTMLRef 
{
public:
	CHTMLRef(CHTMLLexic *lexic, const string &base);
	CHTMLRef(const char* html, int len, const char *baseurl);
	CHTMLRef(const string &html, const string &base);
	CHTMLRef(const char* html, int len, const CURL &base);
	virtual ~CHTMLRef();
	auto_ptr<CRef> refer();
	vector<CRef> links();
private:
	CHTMLLexic *lexic;
	CHTMLLexic _lexic;
	CURL* base;
	CRef* ref;
	typedef std::vector<CRef*> Refbuf;
	Refbuf refs;
	bool in_script;
	//bool in_style;

private:	
	//CRef* do_tag(CTag *ptag); 
	CRef* do_tag_img(CTag *ptag);
	CRef* do_tag_area(CTag *ptag);
	CRef* do_tag_script(CTag *ptag);
	CRef* do_tag_frame(const CTag *ptag);
	CRef* do_tag_a(CTag* ptag);
	CRef* do_tag_meta(CTag* ptag);
	CRef* do_tag_end(CTag* ptag);

};
#endif /* _PAT_HTML_REF_H_102102 */
