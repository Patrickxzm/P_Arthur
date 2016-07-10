#ifndef  _PAT_PAT_TYPES_H_030604_
#define  _PAT_PAT_TYPES_H_030604_
#include <unordered_map>
#include <string>
#include <set>
#include <map>
//using __gnu_cxx::hash;
//using __gnu_cxx::hash_map;
using std::multimap;
using std::less;
using std::string;
using std::set;
using std::map;
using std::unordered_map;
namespace pat_types {
#if 0
struct hashstr
{
	size_t operator()(const string &s) const
	{
		hash<const char*> H;
		return H(s.c_str());
	}
};
#endif //0

}; //namespace pat_types
#endif //  _PAT_PAT_TYPES_H_030604_

