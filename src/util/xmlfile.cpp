#include "xmlfile.h"
#include "util.h"
#include <sstream>
#include <stdexcept>
#include <assert.h>
#include <memory>

using std::ostringstream;
using std::runtime_error;
using std::auto_ptr;

int
CXMLFile::open(const char* fn)
{
	if (file_size(fn) <= 0)
		return -3;
	doc.reset(xmlParseFile(fn));
	if (!doc.get())
		return -1;
	ctx.reset(xmlXPathNewContext(doc.get()));
	if (!ctx.get())
		return -2;
	return 0;
}

CXMLFile::CXMLFile(const char* fn)
{
	int ret = this->open(fn);
	if (ret == -1)
	{
		ostringstream oss;
		oss<<"Can not open xmlfile:"<<fn;
		throw runtime_error(oss.str());
	}
	else if (ret == -2)
	{
		throw runtime_error("Can not get xpath context");
	}
	assert(ret == 0);
}

vector<string>
CXMLFile::getMultiStr(const char* xpath)
{
	vector<string> ret;
	auto_ptr<xmlChar_ptr_vector> v;
	v.reset(xmlGetMultiStr(ctx.get(), BAD_CAST xpath));
	for (size_t i=0; i<v->size(); i++)
		ret.push_back((char*)v->operator[](i));
	return ret;
}

bool
CXMLFile::getInt(const char* xpath, int &n)
{
	string s;
	if (getStr(xpath, s))
	{
		n = atoi(s.c_str());
		return true;
	}
	return false;
	//return xmlGetInt(ctx.get(), BAD_CAST xpath, n);
}

bool
CXMLFile::getStr(const char* xpath, string &s)
{
	xmlChar_scoped_ptr xstr(xmlGetStr(ctx.get(), BAD_CAST xpath));
	if (xstr)
	{
		s.assign((const char*)xstr.get());
		return true;
	}
	return false;
}
