 /*************************************************************************
 * @ a string and a int construct a new type __str_num. We used this type
 *   at two places: in __counter,  the int is used to remember how many times
 *   the string appeared in the text; in str_map, the int is used to index 
 *   the string in __counter.
 *				10/31/2003	09:36
 *************************************************************************/
#ifndef _PAT_STRCOUNTER_H_103003
#define _PAT_STRCOUNTER_H_103003

#include <string>
#include <vector>
#include <ext/rope>
#include <ext/hash_map>

using std::string;
using std::vector;
using __gnu_cxx::crope;
using __gnu_cxx::hash;
using __gnu_cxx::hash_multimap;

struct __str_num{
	__str_num(const string &_str, int _n)
	{
		str = _str;
		n = _n;
	}
	string str;
	int n;
};

class CStrCounter
{
public:
	CStrCounter(const vector<string> &strings);
	const vector<struct __str_num> &count(const crope &text) ;
	const vector<struct __str_num> &count(const string &text) ;
	const vector<struct __str_num> &count(const char* text, unsigned len);
private:
	vector<struct __str_num> __counter;
	struct eqchr
	{
		bool operator()(char c1, char c2) const
		{
			return c1 == c2;
		}
	};
	typedef hash_multimap<char, __str_num, hash<char>, eqchr> map_type;
	map_type str_map;
};
#endif // _PAT_STRCOUNTER_H_103003
