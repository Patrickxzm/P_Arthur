#ifndef _PAT_XMLGET_H_20081206
#define _PAT_XMLGET_H_20081206
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xmlsave.h>
#include <libxml/HTMLparser.h>
#include <string>
#include <vector>
#include <ostream>
#include "memory.hpp"
using std::vector;
using std::string;
using std::basic_string;
using std::ostream;

void xmlFreeChar(xmlChar *p);   
typedef scoped_ptr4c<xmlChar, xmlFreeChar> xmlChar_scoped_ptr;
typedef vector_ptr4c<xmlChar, xmlFreeChar> xmlChar_ptr_vector;

xmlChar_ptr_vector* xmlGetMultiStr(xmlXPathContextPtr ctx, const xmlChar *xpath, int num=0, xmlNodePtr curr=0);
xmlChar* xmlGetStr(xmlXPathContextPtr ctx, const xmlChar *xpath, xmlNodePtr curr=0);
xmlXPathObjectPtr getnodeset(xmlXPathContextPtr ctx, const xmlChar *xpath, xmlNodePtr curr=0);
int register_namespaces(xmlXPathContextPtr xpathCtx, const char* nsfile);
ostream& operator<<(ostream& os, xmlNodePtr node);

string htmlFindEncoding2(const char* html, int size, const char* encoding);
#endif // _PAT_XMLGET_H_20081206
