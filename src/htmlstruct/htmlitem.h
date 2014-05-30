#ifndef _PAT_HTMLITEM_H_092802
#define _PAT_HTMLITEM_H_092802
//#include <ext/hash_map>
//#include <functional>
#include "util/pat_types.h"
//using __gnu_cxx::hash_map;
//using __gnu_cxx::hash;
//using __gnu_cxx::equal_to;
using namespace pat_types;

class CHTMLDEscape
{
public:
	CHTMLDEscape();
	void operator()(string &escaped);
	//void operator()(char* escaped);
private:
	bool read(const string &str, unsigned &rpos, string &buf);
	void write(string &str, unsigned &wpos, const string &buf);
private:
	hash_map<string, char, hashstr> str2ascii;
	hash_map<int, char> code2ascii;
};

extern CHTMLDEscape html_descape;

enum Html_Item_Type
{
	Html_Tag, Html_Text, Html_Comment, Html_Encap
};

class CHTMLItem {
public:
	CHTMLItem(Html_Item_Type type):m_pos(-1), m_type(type)
	{}
	virtual ~CHTMLItem()
	{}
	Html_Item_Type type()
	{
		return m_type;
	}
	int pos() const
	{
		return m_pos;
	}
	virtual const char* get(const char* start, const char* end) = 0;

	static bool is_blank(int c);
	static bool is_wblank(char c, char d);
	static void chunk_blank(string &st);
	static void chunk_blank(char* st);
	static void compress_blank(string &str);
	static void compress_wblank(string &str);
	static void compress_wblank(char *str);
	static void compress_blank(char *str);
protected:
	int m_pos;
private:
	Html_Item_Type m_type;
};
#endif /* _PAT_HTMLITEM_H_092802 */

