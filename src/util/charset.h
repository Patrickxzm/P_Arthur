#ifndef _PAT_CHARSET_H_051311
#define _PAT_CHARSET_H_051311
#include <map>
#include <string>
#include <set>
using std::map;
using std::string;
using std::set;
using std::wstring;

wstring wc(const char* mb_code, const string &in, const char* wc_code = "UCS-4LE");
wstring wc1(const string &in, const char *locale="zh_CN.UTF-8");
string mb(const char* mb_code, const wstring &in, const char* wc_code = "UCS-4LE");
string iconv_str(const char* from, const char* to, const string &in);
size_t convert_charset(const char* from, const char* to, const string &in
     , string &out, string& errmsg);

class CMap2Oname: public map<string, string>
{
public:
	CMap2Oname();
};

class CCharset
{
public:
	static string oname(const string &alias);
private:
	static CMap2Oname map;
};

int iconv_test(const char* encode, const char* buffer, int size);

class CIconvTester
{
public:
	CIconvTester(const char* text, int size);
	//
	// each encoding than be tried only once;
	// oname of the right encoding is returned.
	string try_encode(const char* encoding); 
	inline string nearest() const
	{
		return nearest_encoding;
	}
private: 
	const char* text;
	int size;
	int maxIconv;
	string nearest_encoding;
	set<string> tested; // To avoid multiple testing on error encoding.
};

#endif //_PAT_CHARSET_H_051311
