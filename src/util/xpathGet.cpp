#include "xmlGet.h"
#include "arg.h"
#include "memory.hpp"
#include "util.h"
#include <assert.h>
#include <fstream>
#include <iconv.h>
#include <libxml/HTMLtree.h>
#include <iostream>

using namespace std;

ostream&
help(ostream &os)
{
	os<<"Get content with xpath expression from xml document.\n"
	  "\tUsage: Cmd --doc=... [--encode=...] [--html] [--ns=...] [--dump2=...] [-h|--help]\n"
	  "\t\t --doc= : file name of the xml document.\n"
	  "\t\t --encode= : encoding of the input document. (default: auto-detect)\n"
	  "\t\t --html : document is in html format.\n"
	  "\t\t --ns= : namespace specification file in xml format.\n"
	  "\t\t --dump2= : dump the xml(html) document to file. For html \"GB18030\" \n"
	  "\t\t\t encoding is specified, bacause UTF8 encoding for htmlSaveFileEnv() has bug in libxml2.7.8\n"
	  "\t\t -h|--help : print this message.\n"
	  "\t\t cin : command strings following.\n"
	  "\t\t\t cd xpath \n"
	  "\t\t\t next \n"
	  "\t\t\t content xpath \n"
	  "\t\t\t tree xpath \n"
	  "\t\t\t help \n"
	  "\t\t cout : results in UTF-8.\n"
	  <<endl;
	return os;
}

int main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	if (arg.find1("-h") || arg.find1("--help"))
	{
		help(cout);
		return 1;
	}
	ostringstream docBuf;
	CArg::ArgVal val;
	if (val = arg.find1("--doc="))
	{
		ifstream ifs(val);
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
		string encoding = htmlFindEncoding2(docBuf.str().c_str()
		   , docBuf.str().length(), arg.find1("--encode="));
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
		doc.reset(xmlReadDoc(xmlBody.get(), 0, arg.find1("--encode="), 0));
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

	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> xpathCtx;
	xpathCtx.reset(xmlXPathNewContext(doc.get()));
	if (!xpathCtx.get())
	{
		cerr<<"Error in xmlXPathNewContext()"<<endl;
		return -3;
	}
	if ((val=arg.find1("--ns=")) 
	    && 0!=register_namespaces(xpathCtx.get(), val.get()))
	{
		cerr<<"Error in register_namespace(\""<<val.get()<<"\")"
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
	
	if ((val = arg.find1("--dump2=")).get())
	{
		int resDump;
		if (arg.found("--html"))
			resDump = htmlSaveFileEnc(val.get(), doc.get(), "gb18030");
/*********************************************************************************
//   The following two functions have bug: UTF8ToHtml conversion is used enstead.
			//resDump = htmlSaveFileEnc(val.get(), doc.get(), "utf-8");
			//resDump = htmlSaveFile(val.get(), doc.get());
 *********************************************************************************/
		else
			//resDump = xmlSaveFileEnc(val.get(), doc.get(), "utf-8");
			resDump = xmlSaveFile(val.get(), doc.get());
		if (resDump == -1)
			cerr<<"Dump to \""<<val.get()<<"\" failed."<<endl;
		else
			cout<<resDump<<" bytes dump to \""<<val.get()<<"\"."
			   <<endl;
	}
	return 0;	
}
catch (std::exception &e)
{
	cerr<<"Catch std::exception: "<<e.what()<<endl;
	return -1;
}
