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
		v->push_back(xmlNodeGetContent(node));
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
