#include "strcounter.h"
using std::pair;

CStrCounter::CStrCounter(const vector<string> &strings)
{
	for (unsigned i=0; i < strings.size(); i++)
	{
		if (strings[i].empty())
			continue;
		for (unsigned j=0; j<__counter.size(); j++)
		{	
			if (strings[i] == __counter[j].str)
				continue;
		}
		__counter.push_back(__str_num(strings[i], 0));	
		str_map.insert(map_type::value_type(strings[i][0], 
			__str_num(strings[i], __counter.size()-1)));
	}
}

const vector<struct __str_num> &
CStrCounter::count(const char* text, unsigned len) 
{
	unsigned i;
	for (i=0; i<__counter.size(); i++)
	{	
		__counter[i].n=0;
	}
	for (i=0; i<len; i++)
	{
		pair<map_type::const_iterator, map_type::const_iterator> p 
			= str_map.equal_range(text[i]);
		map_type::const_iterator iter;
		for (iter = p.first; iter != p.second; ++iter)
		{
			const string &str = (*iter).second.str;
			if (strncmp(str.c_str(), text+i, str.size()) == 0)
			{
				__counter[iter->second.n].n++;
			}
		}
	}
	return __counter;
}

const vector<struct __str_num> &
CStrCounter::count(const crope &text) 
{
	return count(text.c_str(), text.size());
}

const vector<struct __str_num> &
CStrCounter::count(const string &text) 
{
	return count(text.c_str(), text.size());
}

#ifdef _TEST
#include <fstream>
#include <iostream>
using namespace std;
int 
main()
{
	vector<string> strings;
	ifstream keywords("keywords");
	string keyword;
	while (keywords>>keyword)
	{
		strings.push_back(keyword);
	}
	CStrCounter counter(strings);
	ifstream page("page");
	string buf;
	crope text;
	while (page>>buf) 
	{
		buf += ' ';
		text.append(buf.c_str(), buf.size());
	}
	
	const vector<struct __str_num> &result = counter.count(text);
	for (unsigned i=0; i<result.size(); i++)
	{
		cout<<result[i].str<<" : "<<result[i].n<<endl;
	}
	
	return 0;
	
}
#endif // _TEST
