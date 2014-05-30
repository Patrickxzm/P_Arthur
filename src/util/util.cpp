#include "util.h"
#include "cutil.h"
#include "memory.hpp"
#include <sys/stat.h>
#include <iconv.h>
#include <sstream>
#include <assert.h>
#include <cctype>
#include <climits>
#include <stdexcept>
#include <errno.h>
#include <limits>
#include <cstdlib>
#include <cstring>

using std::numeric_limits;
using std::ostringstream;
using std::runtime_error;

string 
tolower(string const &s)
{
	string res(s);
	for (unsigned i = 0; i<res.size(); i++)
	{
		res[i] = tolower(res[i]);
	}
	return res;
}

string 
toupper(string const &s)
{
	string res(s);
	for (unsigned i = 0; i<res.size(); i++)
	{
		res[i] = toupper(res[i]);
	}
	return res;
}

string
tostring(int i)
{
	// 3 : one for sign, one for terminated '\0', and one more digit.
	const int buf_size = numeric_limits<int>::digits10+3;
	char buf[buf_size];
	char* ret = intoa(i, buf, buf_size);
	assert(ret != 0);
	return string(buf);
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
	iconv_t env;
	env = iconv_open(to, from);
	char* pin = (char*)in.c_str();
	size_t leftIn = in.size();
	size_t leftOut = leftIn * 4;
	char buffer[leftOut];
	char* pout = buffer;
	string result;
	if (size_t(-1) == iconv(env, &pin, &leftIn, &pout, &leftOut))
		result = strerror(errno);
	else
		result = string(buffer, pout - buffer);
	iconv_close(env);
	return result;
}

wstring 
wc(const char* mb_code, const string& in, const char* wc_code)
{
	size_t result;
	iconv_t env;
	env = iconv_open(wc_code, mb_code);
	char* pin = (char*)in.c_str();
	size_t leftIn = in.size();
	wchar_t wcs[leftIn];
	char* pout = (char*)wcs;
	size_t leftOut = sizeof(wcs);
	result = iconv(env, &pin, &leftIn, &pout, &leftOut);
	iconv_close(env);
	return wstring(wcs, (sizeof(wcs) - leftOut)/sizeof(wchar_t));
}

string 
mb(const char* mb_code, const wstring& in, const char* wc_code)
{
	size_t result;
	iconv_t env;
	env = iconv_open(mb_code, wc_code);
	char* pin = (char*)in.c_str();
	size_t leftIn = in.size() * sizeof(wchar_t);
	size_t leftOut = leftIn;
	char outBuf[leftOut];
	char* pout = outBuf;
	result = iconv(env, &pin, &leftIn, &pout, &leftOut);
	iconv_close(env);
	return string(outBuf, pout - outBuf);
}

int 
file_size(const string &file_name)
{
	struct stat info;
	if (0 != stat(file_name.c_str(), &info))
		return -1;
	return info.st_size;
}

istream& 
operator>>(istream& is, token& t)
{
	is>>std::ws;
	char ch;
	while (is.get(ch))
	{
		if (isalpha(ch) || ch=='_' || (isdigit(ch) && !t.empty()))
		{
			t.push_back(ch);
		}
		else {
			is.unget();
			break;
		}
	}
	if (!t.empty() && is.eof())
		is.clear(is.rdstate() & ~std::ios::failbit);
	return is;
}

//
// escape with backslash is not supported here.
//
istream& 
operator>>(istream& is, quoted_string& qs)
{
	char ch;
	if (!(is>>ch))
		return is;
	if (ch == '\'' || ch == '\"')
	{
		qs.quote = ch;
	}
	else 
	{
		is.unget();
		is.setstate(std::ios::failbit);
		return is;
	}
	qs.unquote.clear();
	while (is.get(ch) && ch!=qs.quote)
		qs.unquote.push_back(ch);
	return is;
}

//
// Remove blanks at begin and end, replace sequence blanks with one SPACE
//
string 
compress_blank(const string &in, const char* encode)
{
	wstring wIn = wc(encode, in);
	wstring wOut;
	bool accept_blank=false;
	for (size_t i=0; i<wIn.size(); i++)
	{
		if (iswspace(wIn[i]) 
		   || wIn[i]==L'\xA0') // &nbsp;
		{
			if (accept_blank)
			{
				wOut.push_back(L' ');
				accept_blank = false;
			}
			continue;
		}
		wOut.push_back(wIn[i]);
		accept_blank = true;
	}
	if (iswspace(wOut[wOut.size()-1]) 
		|| wOut[wOut.size()-1]==L'\xA0') // &nbsp;
		wOut.resize(wOut.size()-1);
	return mb(encode, wOut);
}

