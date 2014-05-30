#include "charset.h"
#include "util.h"
#include <iconv.h>
#include <errno.h>
#include <assert.h>

namespace {
	const char* encoding_list[] = {
	/* oname, alias, ... , alias, 0 */
	   "UTF-8", "UTF8", 0
	   , "GB18030", "GBK", "GB2312", 0
	   , "BIG5", 0
	   , "ISO-8859-1", 0
	};
	#define NELEMS(array) sizeof(array)/sizeof(array[0])
};

CMap2Oname::CMap2Oname()
{
	string oname;
	for (size_t i=0; i<NELEMS(encoding_list); i++)
	{
		if (encoding_list[i] == 0)
		{
			oname.clear();
			continue;
		}
		if (oname.empty())
			oname = encoding_list[i];
		this->operator[](encoding_list[i]) = oname;
	}
	return;
}

CMap2Oname CCharset::map;

string
CCharset::oname(const string &alias) 
{
	CMap2Oname::const_iterator cit;
	if ((cit = map.find(toupper(alias))) == map.end())
		return alias;
	return cit->second;
}

int
iconv_test(const char* encode, const char* buffer, int size)
{
	if (encode == 0 || buffer == 0)
		return -1;
	char iconv_buf[size];
	size_t leftIn, leftOut;
	char *pin, *pout;
	iconv_t env;
	env = iconv_open(encode, encode);
	leftIn = leftOut = size;
	pin = (char*)buffer;
	pout = iconv_buf;
	size_t result = iconv(env, &pin, &leftIn, &pout, &leftOut);
	if (result == (size_t)-1)
	{
		assert(errno != E2BIG);
		return pin - buffer;
	}
	return size - leftIn;
}

CIconvTester::CIconvTester(const char* text, int size)
   : maxIconv(0)
{
	this->text =text;
	this->size = size;
}

string 
CIconvTester::try_encode(const char* encoding)
{
	string oname = CCharset::oname(encoding);
	if (tested.insert(oname).second) 
	{
		int resIconv = iconv_test(oname.c_str(), text, size);
		if (resIconv == size)
			return oname;
		if (resIconv > maxIconv)
		{
			nearest_encoding = oname;
			maxIconv = resIconv;
		}
	}
	return "";
}
