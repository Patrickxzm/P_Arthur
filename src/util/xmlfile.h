#ifndef _PAT_XMLFILE_H_20110914
#define _PAT_XMLFILE_H_20110914
#include "xmlGet.h"

//C-like APIs in "xmlGet.h" are more efficent & powerful, but the following 
// C++ class is MORE convenient.
class CXMLFile
{
public:
	CXMLFile()
	{}
	CXMLFile(const char* fn);
	int open(const char* fn);
	vector<string> getMultiStr(const char* xpath);
	bool getInt(const char* xpath, int &n);
	bool getStr(const char* xpath, string &s);
private:
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> ctx;
};
#endif // _PAT_XMLFILE_H_20110914
