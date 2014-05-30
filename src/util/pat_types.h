#ifndef  _PAT_PAT_TYPES_H_030604_
#define  _PAT_PAT_TYPES_H_030604_
#include <ext/hash_map>
#include <string>
#include <set>
#include <map>
using __gnu_cxx::hash;
using __gnu_cxx::hash_map;
using std::multimap;
using std::less;
using std::string;
using std::set;
using std::map;
namespace pat_types {

struct hashstr
{
	size_t operator()(const string &s) const
	{
		hash<const char*> H;
		return H(s.c_str());
	}
};

}; //namespace pat_types
#endif //  _PAT_PAT_TYPES_H_030604_

