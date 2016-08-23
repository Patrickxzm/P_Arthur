#include "util/xmlGet.h"
#include "util/arg.h"
#include "util/memory.hpp"
#include "util/util.h"
#include "tw_raw.h"
#include <assert.h>
#include <fstream>
#include <iconv.h>
#include <libxml/HTMLtree.h>
#include <iostream>
#include <sstream>

using namespace std;

ostream&
help(ostream &os)
{
	os<<"Extract content from TWR input, with xpath expression.\n"
	  "\tUsage: Cmd --expr=... [-h|--help]\n"
          "\t\t --expr= : xpath expression\n"
	  "\t\t -h|--help : print this message.\n"
	  <<endl;
	return os;
}

xmlDoc* getXML(const CTWRaw &raw);
vector<string> extract(xmlDoc *doc, const string &expr);

int main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	if (arg.found("-h") || arg.found("--help"))
	{
		help(cout);
		return 1;
	}
	string expr;
        if (!arg.findLast("--expr=", expr) || expr.empty())
	{
		cout<<"please specify a \"--expr=\" option."<<endl;
		help(cout);
		return 2;
	}
	CTWRaw raw;
	while (cin>>raw)
	{
		scoped_ptr4c<xmlDoc, xmlFreeDoc> doc(getXML(raw));
		if (!doc->encoding || xmlStrlen(doc->encoding)==0)
			cerr<<"Warrning: Can not detect document encoding!"<<endl;
		vector<string> contents = extract(doc.get(), expr);
		for (int i=0; i<contents.size(); i++)
		{
			cout<<contents[i]<<endl;
		}
	}
	return 0;	
}
catch (std::exception &e)
{
	cerr<<"Catch std::exception: "<<e.what()<<endl;
	return -1;
}

xmlDoc* 
getXML(const CTWRaw &raw)
{
	if (raw.reply.headers.value("Content-Type").find("text/html") == string::npos)
		throw runtime_error("Can process \"text/html\" only!");
	string encoding = htmlFindEncoding2(raw.reply.body.c_str()
	   , raw.reply.body.length(), raw.reply.headers.charset().c_str());
	xmlDoc* doc = htmlReadMemory(raw.reply.body.c_str(), raw.reply.body.length()
	   , 0, encoding.c_str(), HTML_PARSE_RECOVER);
	if (0 == doc)
		throw runtime_error("Parse document failed!");
	return doc;
}

vector<string> 
extract(xmlDoc *doc, const string &expr)
{
	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> xpathCtx;
	xpathCtx.reset(xmlXPathNewContext(doc));
	if (!xpathCtx.get())
		throw runtime_error("Error in xmlXPathNewContext()");
	scoped_ptr4c<xmlXPathObject, xmlXPathFreeObject> entries;
	xmlChar_scoped_ptr xpath(xmlCharStrdup(expr.c_str()));
	scoped_ptr4c<xmlXPathObject, xmlXPathFreeObject> result;
	result.reset(getnodeset(xpathCtx.get(), xpath.get(), 0));
	vector<string> res;
	if (!result || xmlXPathNodeSetIsEmpty(result->nodesetval))
		return res;
	assert(result->nodesetval->nodeNr > 0);
	xmlNodeSetPtr nodeset = result->nodesetval;
	for (int i=0; i<nodeset->nodeNr; i++)
	{
		xmlChar_scoped_ptr str;
		str.reset(xmlNodeGetContent(nodeset->nodeTab[i]));
		if (str.get())
			res.push_back(compress_blank((char*)str.get()));
	}
	return res;
}
