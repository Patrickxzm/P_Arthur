#include "util/xmlGet.h"
#include "util/arg.h"
#include "util/memory.hpp"
#include "util/util.h"
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

vector<string> extract(const CTWRaw& raw, const string &expr);

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
		vector<string> contents = extract(raw, expr);
		for (int i=0; i<contents.size(); i++)
		{
			cout<<contents[i]<<endl;
		}
	}
	ostringstream docBuf;
        vector<string> val;
        string fn_doc;
        val = arg.find("--doc=");
        if (arg.findLast("--doc=", fn_doc))
	{
		ifstream ifs(fn_doc);
		docBuf<<ifs.rdbuf();
	}
	else
	{
		cerr<<"Please specify the file name of the xml document whith \"--doc=\"."<<endl;
		return -3;
	}
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	if (arg.found("--html"))
	{
                string arg_encode;
                string encoding;
                if (arg.findLast("--encode=", arg_encode))
		    encoding = htmlFindEncoding2(docBuf.str().c_str()
		       , docBuf.str().length(), arg_encode.c_str());
                else
		    encoding = htmlFindEncoding2(docBuf.str().c_str()
		       , docBuf.str().length(), 0);
		if (encoding.size() > 0)
		{
			doc.reset(htmlReadMemory(docBuf.str().c_str(), docBuf.str().length()
			  , 0, encoding.c_str(), HTML_PARSE_RECOVER));
		}
		else
		{
			cerr<<"Document encoding Unknown!"<<endl;
			return -4;
		}
	}
	else
	{
		xmlChar_scoped_ptr xmlBody;
		xmlBody.reset(xmlCharStrndup(docBuf.str().c_str(), docBuf.str().length()));
		doc.reset(xmlReadDoc(xmlBody.get(), 0
                   , arg.find("--encode=").size()>0 ?
                       arg.find("--encode=")[0].c_str() : 0
                   , 0));
	}
	if (!doc.get())
	{
		cerr<<"Parse document failed!"<<endl;
		return -2;
	}
	if (!doc->encoding || xmlStrlen(doc->encoding)==0)
		cerr<<"Warrning: Can not detect document encoding!"<<endl;
	else
		cout<<"Document Encoding: "<<doc->encoding<<endl;

        string nsfile;
	if (arg.findLast("--ns=", nsfile)
	    && 0!=register_namespaces(xpathCtx.get(), nsfile.c_str()))
	{
		cerr<<"Error in register_namespace(\""<<nsfile<<"\")"
		   <<endl;
		return -4;
	}
	
	string command_line;
	int curr_index = -1;
	scoped_ptr4c<xmlXPathObject, xmlXPathFreeObject> entries;
	for (cout<<">> "<<flush; getline(cin, command_line); cout<<">> "<<flush)
	{
		istringstream iss(command_line);
		string command, xpathstr;
		if (!(iss>>command))
			continue;
		xmlChar_scoped_ptr xpath;
		scoped_ptr4c<xmlXPathObject, xmlXPathFreeObject> result;
		if (command == "cd" || command == "tree" || command == "content")
		{
			iss>>ws;
			if (!getline(iss, xpathstr))
			{
				cerr<<"xpath is missed."<<endl;
				continue;
			}
			xpath.reset(xmlCharStrdup(xpathstr.c_str()));
			xmlNodePtr curr = 0;
			if (curr_index >= 0 && entries.get())
				curr = entries->nodesetval->nodeTab[curr_index];
			result.reset(getnodeset(xpathCtx.get(), xpath.get(), curr));
			if (!result || xmlXPathNodeSetIsEmpty(result->nodesetval))
			{
				cout<<"XPath no match."<<endl;
				continue;
			}
			assert(result->nodesetval->nodeNr > 0);
		}
		if (command == "cd")
		{
			curr_index = 0;
			entries.reset(result.release());
			cout<<"get "<<entries->nodesetval->nodeNr<<" entry(ies)"<<endl;
		}
		else if (command == "next")
		{
			if (curr_index < 0)
				cout<<"No entries"<<endl;
			else if (curr_index >= entries->nodesetval->nodeNr - 1)
				cout<<"last one entry already"<<endl;
			else
			{
				curr_index ++;
				cout<<"next OK"<<endl;
			}
		}
		else if (command == "content")
		{
			xmlNodeSetPtr nodeset = result->nodesetval;
			cout<<"content num: "<< nodeset->nodeNr<<endl;
			for (int i=0; i<nodeset->nodeNr; i++)
			{
				xmlChar_scoped_ptr str;
			//	str.reset(xmlNodeListGetString(doc.get(),
			//		nodeset->nodeTab[i]->xmlChildrenNode, 1));
				str.reset(xmlNodeGetContent(nodeset->nodeTab[i]));
				cout<<i<<": ";
				if (str.get())
					cout<<compress_blank((char*)str.get())<<endl;
				else
					cout<<"(null)"<<endl;
			}
		}
		else if (command == "tree")
		{
			xmlNodeSetPtr nodeset = result->nodesetval;
			cout<<"tree num: "<<nodeset->nodeNr<<endl;
			for (int i=0; i<nodeset->nodeNr; i++)
				cout<<nodeset->nodeTab[i]<<endl;
		}
		else if (command == "help")
		{
			help(cout);
		}
		else
		{
			cerr<<"Wrong command."<<endl;
		}
	}
	string fn;
        if (arg.findLast("--dump2=", fn))
	{
		int resDump;
		if (arg.found("--html"))
			resDump = htmlSaveFileEnc(fn.c_str(), doc.get(), "gb18030");
/*********************************************************************************
//   The following two functions have bug: UTF8ToHtml conversion is used instead.
			//resDump = htmlSaveFileEnc(val.get(), doc.get(), "utf-8");
			//resDump = htmlSaveFile(val.get(), doc.get());
 *********************************************************************************/
		else
			//resDump = xmlSaveFileEnc(val.get(), doc.get(), "utf-8");
			resDump = xmlSaveFile(fn.c_str(), doc.get());
		if (resDump == -1)
			cerr<<"Dump to \""<<fn<<"\" failed."<<endl;
		else
			cout<<resDump<<" bytes dump to \""<<fn<<"\"."
			   <<endl;
	}
	return 0;	
}
catch (std::exception &e)
{
	cerr<<"Catch std::exception: "<<e.what()<<endl;
	return -1;
}

vector<string>
extract(const CTWRaw& raw, const string &expr)
{
	if (raw.reply.headers.value("Content-Type").find("text/html") == string::npos)
		throw exception("Can only process \"text/html\");
	string encoding = htmlFindEncoding2(raw.reply.body.c_str()
	   , raw.reply.body.length(), raw.reply.headers.charset());
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	doc.reset(htmlReadMemory(docBuf.str().c_str(), docBuf.str().length()
	   , 0, encoding.c_str(), HTML_PARSE_RECOVER));
        if (!doc.get())
		throw exception("Parse document failed!");
	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> xpathCtx;
	xpathCtx.reset(xmlXPathNewContext(doc.get()));
	if (!xpathCtx.get())
		throw exception("Error in xmlXPathNewContext()");
	scoped_ptr4c<xmlXPathObject, xmlXPathFreeObject> entries;
	xmlChar_scoped_ptr xpath = 
}
