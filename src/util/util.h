#ifndef _PAT_UTIL_H_20081213
#define _PAT_UTIL_H_20081213
#include <string>
#include <istream>

using std::istream;
using std::string;
using std::wstring;

string tolower(string const &s);
string toupper(string const &s);
string tostring(int i);

/* ON unix/linux/mac with x86:  wc_code should be UCS-4LE, 
 * UTF-32LE is the same.
 */
wstring wc(const char* mb_code, const string &in, const char* wc_code = "UCS-4LE");
wstring wc1(const string &in, const char *locale="zh_CN.UTF-8");
string mb(const char* mb_code, const wstring &in, const char* wc_code = "UCS-4LE");
int file_size(const string &file_name);
string compress_blank(const string &in, const char* encode="UTF-8");
string iconv_str(const char* from, const char* to, const string &in);

class token : public string
{};
istream& operator>>(istream& is, token& t);

class quoted_string 
{
public: 
	operator string() const
	{
		return quote+unquote+quote;
	}
	char quote;
	string unquote;
};
istream& operator>>(istream& is, quoted_string& q);


#endif // _PAT_UTIL_H_20081213
