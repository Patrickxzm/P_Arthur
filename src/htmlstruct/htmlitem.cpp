/************************************************************************
 * @ url in hyperlink should be descaped.
 * 					10/28/2004	18:32
 * @ Part of the class is re-coded with STL.
 *					01/31/2004	23:16
 * @ In html document, some charactor entities is encoded in such ways:
 *   "&xxx;" or "&#ddd;"(x means a letter, d means a digit). Part of 
 *   them have ascii counterpart, and only for them there is a method to
 *   get their ascii code counterpart. For more infomation, please refer:
 *   http://www.w3.org/TR/html4/sgml/entities.html, part of html4 document.
 *					01/31/2004	13:47
 ************************************************************************/
#include "htmlitem.h"
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <string>
#include <cctype>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

using std::string;
struct escape_code {
	const char* str;
	int code;
	char ascii;
};

struct escape_code escape_table[] =
{
	{"nbsp",160, ' '}, {"brvbar",166, '|'}
	, {"quot",34, '\"'}, {"amp",38, '&'}, {"lt", 60, '<'}, {"gt",62, '>'}
	, {"empty_string", 39, '\''}, {"middot", 38, '.'} 
};

#define NELEMS(array) sizeof(array)/sizeof(array[0])
CHTMLDEscape html_descape;

CHTMLDEscape::CHTMLDEscape()
{
	for (unsigned i=0; i<NELEMS(escape_table); i++)
	{
		str2ascii[escape_table[i].str] = escape_table[i].ascii;
		code2ascii[escape_table[i].code] = escape_table[i].ascii;
	}
}

void
CHTMLDEscape::operator()(string &escaped)
{
	unsigned rpos = 0;
	unsigned wpos = 0;
	string buf;
	while (rpos < escaped.size() && read(escaped, rpos, buf))
	{
		write(escaped, wpos, buf);
	}
	escaped.erase(wpos);
	return;
}

bool
CHTMLDEscape::read(const string &str, unsigned &rpos, string &buf)
{
	buf.clear();
	while (rpos<str.size() && str[rpos] != '&')
	{
		buf.push_back(str[rpos++]);
	}
	if (!buf.empty())
		return true;
	if (rpos >= str.size())
		return false;

	buf.push_back(str[rpos++]);
	if (rpos < str.size() && str[rpos] == '#')
	{
		buf.push_back('#');
		rpos++;
		while (rpos < str.size() && isdigit(str[rpos]))
		{
			buf.push_back(str[rpos++]);
		}
		if (rpos < str.size() && str[rpos] == ';')
		{
			buf.push_back(str[rpos++]);
		}
		return true;
	}
	while (rpos < str.size() && isalpha(str[rpos]))
	{
		buf.push_back(str[rpos++]);
	}
	if (rpos < str.size() && str[rpos] == ';')
	{
		buf.push_back(str[rpos++]);
	}
	return true;
}

void 
CHTMLDEscape::write(string &str, unsigned &wpos, const string &buf)
{
	if (buf[0] == '&')
	{
		if (buf[1] == '#')
		{
			int code = 0;
			for (unsigned i=2; i<buf.size() && isdigit(buf[i]); i++)
			{
				code *= 10;
				code += buf[i] - '0';
			}			
			if (code2ascii.count(code) != 0)
			{
				str.replace(wpos++, 1, 1, code2ascii[code]);
			}
			else
			{
				str.replace(wpos, buf.size(), buf);
				wpos += buf.size();
			}
		}
		else
		{
			string s;
			if (buf[buf.size()-1] == ';')
			{
				s.assign(buf, 1, buf.size()-2);
			}
			else
			{
				s.assign(buf, 1, buf.size()-1);
			}
			if (str2ascii.count(s) != 0)
			{
				str.replace(wpos++, 1, 1, str2ascii[s]);
			}
			else
			{
				str.replace(wpos, buf.size(), buf);
				wpos += buf.size();
			}
		}
	}
	else
	{
		str.replace(wpos, buf.size(), buf);
		wpos += buf.size();
	}
	return;
}

bool 
CHTMLItem::is_wblank(char c, char d)
{
	if (c == -95 && d == -95)
		return true;
	return false;
}

bool 
CHTMLItem::is_blank(int c)
{
	if (c == '\t' || c ==' ' || c == '\r' || c == '\n')
		return true;
	return false;
}

void 
CHTMLItem::chunk_blank(string &st) 
{
	if (st.empty())
		return;
	unsigned start=0;
	while (start<st.size() && is_blank(st[start]))
		start ++;
	unsigned end=st.size() - 1;
	while (end>start && is_blank(st[end]))
		end --;
	///////////////////////////////////////////////////
	//  ______NonBlank... ... NonBlank__________
	//        ^                      ^
	//        start                  end
	//////////////////////////////////////////////////
	if (start<st.size())
	{
		string tmp;
		tmp.assign(st, start, end+1-start);
		st = tmp;
	}
	else
		st.clear();
}

void 
CHTMLItem::chunk_blank(char *st)
{
	char* start = st;
	while (*start != '\0' && is_blank(*start))
		start ++;
	char* end = st + strlen(st) - 1;
	while (end > start && is_blank(*end))
		end --;
	if (start <= end)
	{
		memmove(st, start, end+1-start);
		st[end+1-start] = '\0';
	}
	else
		st[0] = '\0';
	return;
}

void 
CHTMLItem::compress_wblank(char* str)
{
	char* wptr = str;
	bool blank = true;
	for (char *rptr = str; *rptr != '\0'; rptr++)
	{
		if (is_blank(*rptr) || is_wblank(rptr[0], rptr[1]))
		{
			if (is_wblank(rptr[0], rptr[1]))
				rptr ++;
			if (!blank)
				*(wptr++) = ' ';
			blank = true;
		}
		else
		{
			*(wptr++) = *rptr;
			blank = false;
		}
	}
	if (wptr-1 >=str && wptr[-1] == ' ')
		wptr --;
	*wptr = '\0';
	return;
}

void 
CHTMLItem::compress_wblank(string &st)
{
	int w=0;
	bool blank = true;
	for (unsigned r=0; r<st.size(); r++)
	{
		if (is_blank(st[r]) || is_wblank(st[r], st[r+1]))
		{
			if (is_wblank(st[r], st[r+1]))
				r ++;
			if (!blank)
				st[w++] = ' ';
			blank = true;
		}
		else
		{
			st[w++] = st[r];
			blank = false;
		}
	}
	if (w > 0 && st[w-1]==' ') // blank(if) has been replaced by space;
		w--;
	st.resize(w);
	return;
}

void 
CHTMLItem::compress_blank(string &st)
{
	int w=0;
	bool blank = true;
	for (unsigned r=0; r<st.size(); r++)
	{
		if (is_blank(st[r]))
		{
			if (!blank)
				st[w++] = ' ';
			blank = true;
		}
		else {
			st[w++] = st[r];
			blank = false;
		}
	}
	if (w > 0 && st[w-1]==' ') // blank(if) has been replaced by space;
		w--;
	st.resize(w);
	return;
}

void 
CHTMLItem::compress_blank(char *str)
{
	char* wptr = str;
	bool blank = true;
	for (char* rptr=str; *rptr!='\0'; rptr++)
	{
		if (is_blank(*rptr))
		{
			if (!blank)
				*(wptr++) = ' ';
			blank = true;
		}
		else {
			*(wptr++) = *rptr;
			blank = false;
		}
	}
	if (wptr-1>=str && *(wptr-1)==' ') // blank(if) has been replaced by space;
		wptr --;
	*wptr = '\0';
	return;
}

