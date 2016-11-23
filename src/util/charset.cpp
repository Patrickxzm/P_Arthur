#include "charset.h"
#include "util.h"
#include <iconv.h>
#include <cstdlib>
#include <errno.h>
#include <assert.h>
#include <cstring>

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

wstring
wc1(const string &in, const char *locale)
{
	setlocale(LC_CTYPE, locale);
	wchar_t buf[in.size()+1];
	mbstowcs(buf, in.c_str(), in.size()+1);
	return buf;
}

string
iconv_str(const char* from, const char* to, const string &in)
{
    string out, errmsg;
    if (size_t(-1) == convert_charset(from, to, in, out, errmsg))
        return errmsg;
    return out;
}

size_t
convert_charset(const char* from, const char* to, const string &in
     , string &out, string& errmsg)
{
    iconv_t env;
    env = iconv_open(to, from);
    char* pin = (char*)in.c_str();
    size_t leftIn = in.size();
    size_t leftOut = leftIn * 4;
    char buffer[leftOut];
    char* pout = buffer;
    size_t result = iconv(env, &pin, &leftIn, &pout, &leftOut);
    if (size_t(-1) == result)
        errmsg = strerror(errno);
    else
        out = string(buffer, pout - buffer);
    iconv_close(env);
    return result;
}

wstring 
wc(const char* mb_code, const string& in, const char* wc_code)
{
	iconv_t env;
	env = iconv_open(wc_code, mb_code);
	char* pin = (char*)in.c_str();
	size_t leftIn = in.size();
	wchar_t wcs[leftIn];
	char* pout = (char*)wcs;
	size_t leftOut = sizeof(wcs);
	iconv(env, &pin, &leftIn, &pout, &leftOut);
	iconv_close(env);
	return wstring(wcs, (sizeof(wcs) - leftOut)/sizeof(wchar_t));
}

string 
mb(const char* mb_code, const wstring& in, const char* wc_code)
{
	iconv_t env;
	env = iconv_open(mb_code, wc_code);
	char* pin = (char*)in.c_str();
	size_t leftIn = in.size() * sizeof(wchar_t);
	size_t leftOut = leftIn;
	char outBuf[leftOut];
	char* pout = outBuf;
	iconv(env, &pin, &leftIn, &pout, &leftOut);
	iconv_close(env);
	return string(outBuf, pout - outBuf);
}

