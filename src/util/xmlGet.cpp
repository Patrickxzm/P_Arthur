#include "xmlGet.h"
#include "redirect.h"
#include "memory.hpp"
#include "util.h"
#include "cutil.h"
#include "charset.h"

#include <libxml/xpathInternals.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <memory>
using std::auto_ptr;
using std::istringstream;
using std::ostringstream;
using std::runtime_error;

void 
xmlFreeChar(xmlChar* p)
{
	xmlFree(p);
}

xmlXPathObjectPtr 
getnodeset (xmlXPathContextPtr ctx, const xmlChar *xpath, xmlNodePtr curr)
{
	xmlXPathObjectPtr result;
	if (ctx == 0)
		return 0;
	ctx->node = curr;
	result = xmlXPathEvalExpression(xpath, ctx);
	return result;
}

#if 0
bool xmlGetInt (xmlXPathContextPtr ctx, const xmlChar *xpath, int& result, xmlNodePtr curr)
{
	xmlString s;
	if (xmlGetStr(ctx, xpath, s, curr))
	{
		result = atoi((char*)s.c_str());
		return true;
	}
	return false;
}
#endif //0

#if 0
bool 
getStr(xmlXPathContextPtr ctx, const xmlChar *xpath, string &result, xmlNodePtr curr)
{
	auto_ptr<xmlChar_ptr_vector> v;
	v.reset(xmlGetMultiStr(ctx, xpath, 1, curr));
	if (v->empty() || (*v)[0]==0)
		return false;
	result = (const char*)(*v)[0];
	return true;
}
#endif //0

xmlChar*
xmlGetStr(xmlXPathContextPtr ctx, const xmlChar *xpath, xmlNodePtr curr)
{
	auto_ptr<xmlChar_ptr_vector> v;
	v.reset(xmlGetMultiStr(ctx, xpath, 1, curr));
	if (v->empty() || (*v)[0]==0)
		return 0;
	xmlChar* result = (*v)[0];
	(*v)[0] = 0;
	return result;
}

xmlChar_ptr_vector*
xmlGetMultiStr(xmlXPathContextPtr ctx, const xmlChar *xpath, int num, xmlNodePtr curr)
{
	xmlChar_ptr_vector *v = new xmlChar_ptr_vector;	
	scoped_ptr4c<xmlXPathObject, xmlXPathFreeObject> r(getnodeset(ctx, xpath, curr));
	if (!r || xmlXPathNodeSetIsEmpty(r->nodesetval))
		return v;
	xmlNodeSetPtr nodeset = r->nodesetval;
	for (int i=0; i<nodeset->nodeNr && (num<=0 || i<num); i++)
	{
		xmlNodePtr node = nodeset->nodeTab[i];
		//xmlChar_scoped_ptr str(xmlNodeListGetString(doc, node->xmlChildrenNode, 1));
		v->push_back(xmlNodeGetContent(node));
#if 0  // change return value from vector<string> to xmlChar_ptr_vector*
		xmlChar_scoped_ptr str(xmlNodeGetContent(node));
		if (str.get())
			v.push_back((char*)str.get());
#endif //0
	}
	return v;
}

ostream&
operator<<(ostream& os, xmlDocPtr doc)
{
	os<<"<?xml version=\""<<doc->version<<"\" encoding=\""<<doc->encoding<<"\"?>\n";
	for (xmlNodePtr curr = doc->children; curr; curr=curr->next)
		os<<curr;
	return os;
}

ostream&
operator<<(ostream& os, xmlAttrPtr attr)
{
	if (!attr)
		return os;
	switch (attr->type)
	{
	case XML_ATTRIBUTE_NODE:
		os<<attr->name;
		if (attr->children)
			os<<"=\"";
		else
			break;
		for (xmlNodePtr curr = attr->children; curr; curr=curr->next)
			os<<curr;
		os<<"\"";
		break;
	default :
		throw runtime_error("Unknow type for xmlAttr:"+std::to_string(attr->type));
	}
	return os;
}

ostream& 
operator<<(ostream& os, xmlNodePtr node)
{
	if (!node) 
		return os;
	switch (node->type)
	{
	case XML_ELEMENT_NODE: 
		os<<"<"<<node->name;
		for (xmlAttrPtr curr = node->properties; curr; curr=curr->next)
			os<<" "<<curr;
		os<<">";
		if (node->children)
		{
			for (xmlNodePtr curr = node->children; curr; curr=curr->next)
				os<<curr;
			os<<"</"<<node->name<<">"; //end_tag
		}
		break;
	case XML_TEXT_NODE:
		os<<node->content;
		break;
	case XML_CDATA_SECTION_NODE:
		os<<"<![CDATA["<<node->content<<"]]>";
		break;
	case XML_COMMENT_NODE:
		os<<"<!--"<<node->content<<"-->";
		break;
	case XML_HTML_DOCUMENT_NODE:
		for (xmlNodePtr curr = node->children; curr; curr=curr->next)
			os<<curr;
		break;
	case XML_DOCUMENT_NODE:
		os<<(xmlDocPtr)node;
		break;
	default :
		ostringstream oss;
		oss<<"Unkown type for xmlNode: "<<node->type;
		throw runtime_error(oss.str());
	}
	return os;
}

#if 0
htmlDocPtr 
htmlReadDoc2(const xmlChar * cur, const char* URL, const char * encoding, int options)
{
	string encode;
	if (encoding != 0 && strlen(encoding)>0)
		encode = encoding;
	if (encode.size() > 0)
		return htmlReadDoc(cur, URL, encode.c_str()
			, options);

	// try to find the encoding from the html document.
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc_utf8; //default "utf8"
	doc_utf8.reset(htmlReadDoc(cur, URL, 0, options)); 
	if (!doc_utf8.get())
		return 0;
	if (doc_utf8->encoding != 0 && doc_utf8->encoding[0]!='\0')
		encode = (char*)doc_utf8->encoding;
 	if (encode.empty())
	{ // Make a guess : "gbk" !
		scoped_ptr4c<xmlDoc, xmlFreeDoc> doc_gbk;
		scoped_ptr4c<xmlParserCtxt, htmlFreeParserCtxt> ctxt;
		ctxt.reset(htmlNewParserCtxt());
		//doc_gbk.reset(
		 //htmlCtxtReadDoc(ctxt.get(), cur, URL, "gbk", options)); 
		doc_gbk.reset(
		   htmlReadDoc(cur, URL, "gbk", options)); 
		if (!doc_gbk.get())
			return 0;
		if (ctxt->errNo == XML_ERR_INVALID_ENCODING) // guess failed!
			return doc_utf8.release();
		else
			return doc_gbk.release();
	}
	else if (strcasecmp(encode.c_str(), "utf8")==0)
		return doc_utf8.release();
	else { // Read document with the encoding just found.
		if (strcasecmp(encode.c_str(), "gb2312") == 0)
			encode = "gbk"; // gb2312 < gbk < gb18030
		return htmlReadDoc(cur, URL, encode.c_str(), options);
	}
}
#endif //0

namespace {
	// There is a similar function in libxml2
	char* htmlFindEncoding(const char* html, int size) 
	{
		if (html == 0) 
			return 0;

		const char* cur = strncasestr(html, "HTTP-EQUIV", size);
		if (cur == 0)
			return 0;
		cur = strncasestr(cur, "CONTENT", html+size-cur);
		if (cur == 0)
			return 0;
		cur = strncasestr(cur, "CHARSET=", html+size-cur);
		if (cur == 0)
			return 0;
		cur += 8; // 8 is the strlen of "CHARSET=";
		const char* start = cur;
		while (isalnum(*cur) || *cur=='-' || *cur=='_'
		   || *cur==':' || *cur=='/')
			cur ++;
		if (start == cur)
			return 0;
		return strndup(start, cur - start);
	}
}

string 
htmlFindEncoding2(const char* html, int size, const char* encoding)
{
	CIconvTester test(html, size);
	string oname;
	if (encoding != 0 && strlen(encoding)>0)
	{
		oname = CCharset::oname(encoding);
		if (oname != "ISO-8859-1")
		{ // Make "ISO-8859-1" the last choice;
			oname = test.try_encode(encoding);
			if (oname.size() > 0)
				return oname;
		}
	}
	scoped_ptr4c<char, myFree<char> > encode_find;
	encode_find.reset(htmlFindEncoding(html, size));
	if (encode_find.get())
	{
		oname = test.try_encode(encode_find.get());
		if (oname.size() > 0)
			return oname;
	}
	oname = test.try_encode("gb18030");
	if (oname.size() > 0)
		return oname;
	oname = test.try_encode("utf-8");
	if (oname.size() > 0)
		return oname;
	oname = test.try_encode("BIG5");
	if (oname.size() > 0)
		return oname;
	oname = test.try_encode("ISO-8859-1");
	if (oname.size() > 0)
		return oname;
	return "";
}

int 
register_namespaces(xmlXPathContextPtr xpathCtx, const char* nsfile)
{
	scoped_ptr4c<xmlDoc, xmlFreeDoc> nsdoc;
	nsdoc.reset(xmlParseFile(nsfile));
	if (!nsdoc.get())
	{
		ostringstream oss;
		oss<<"Error in xmlParseFile(\""<<nsfile
		   <<"\")";
		throw runtime_error(oss.str());
	}
	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> ctx;
	ctx.reset(xmlXPathNewContext(nsdoc.get()));
	if (!ctx.get())
	{
		ostringstream oss;
		oss<<"Error in xmlXPathNewContext(nsdoc)";
		throw runtime_error(oss.str());
	}
	scoped_ptr4c<xmlXPathObject, xmlXPathFreeObject> r;
	r.reset(getnodeset(ctx.get(), BAD_CAST "/excelNamespace/namespace"));
	if (!r.get() || xmlXPathNodeSetIsEmpty(r->nodesetval))
		return 0;
	xmlNodeSetPtr nodeset = r->nodesetval;
	for (int i=0; i<nodeset->nodeNr; i++)
	{
		xmlNodePtr node = nodeset->nodeTab[i];
		xmlChar_scoped_ptr position, prefix, href;
		position.reset(xmlGetStr(ctx.get(), BAD_CAST"position", node));
		prefix.reset(xmlGetStr(ctx.get(), BAD_CAST"prefix", node));
		href.reset(xmlGetStr(ctx.get(), BAD_CAST"href", node));
		if (!prefix || prefix[0]==0)
			xmlXPathRegisterNs(xpathCtx, position.get(), href.get());
		else
			xmlXPathRegisterNs(xpathCtx, prefix.get(), href.get());
	}
	return 0;
}

#if 0  // replaced by htmlFindEncoding2() + htmlReadMemory()
htmlDocPtr 
htmlReadMemory2(const char* buffer, int size, const char* URL
   , const char * encoding, int options)
{
	// disable error output
	//CRedirect error_output(2);
	//error_output.redirect("/dev/null");
	CIconvTester test(buffer, size);
	string oname;
	if (encoding != 0 && strlen(encoding)>0)
	{
		oname = test.try_encode(encoding);
		if (oname.size() > 0)
			return htmlReadMemory(buffer, size, URL, oname.c_str(), options);
	}
	// try to find the encoding from the html document.
	scoped_ptr4c<char, myFree<char> > encode_find;
	encode_find.reset(htmlFindEncoding(buffer, size));
	if (encode_find.get())
	{
		oname = test.try_encode(encode_find.get());
		if (oname.size() > 0)
			return htmlReadMemory(buffer, size, URL, oname.c_str(), options);
	}
	oname = test.try_encode("gb18030");
	if (oname.size() > 0)
		return htmlReadMemory(buffer, size, URL, oname.c_str(), options);
	oname = test.try_encode("utf-8");
	if (oname.size() > 0)
		return htmlReadMemory(buffer, size, URL, oname.c_str(), options);
	return 0;
	if (test.nearest().size() > 0)
		return htmlReadMemory(buffer, size, URL, test.nearest().c_str(), options);
	return 0;
}
#endif //0

