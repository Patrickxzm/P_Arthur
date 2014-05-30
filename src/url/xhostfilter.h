/************************************************************************
 * @ This is an optimized CHostfilter. When a domain-name can not pass , 
 *   its ip_address will be tested, and the result is also cached to avoid
 *   unnecessary DNS request.			03/03/2004	21:10
 ************************************************************************/
#ifndef _PAT_X_HOSTFILTER_H_030304
#define _PAT_X_HOSTFILTER_H_030304
#include "hostfilter.h"
#include "host.h"
#include <string>
#include <ext/hash_map>

using __gnu_cxx::hash;
using __gnu_cxx::hash_map;
using std::string;

class CXHostFilter  : public CHostFilter
{
public:
	CXHostFilter();
	bool pass(const char* hostname, CHost *phost=0) ;
	bool need_dns(const char* hostname) ;
private:
	hash_map<string, bool, hashstr> cache;
};
#endif //  _PAT_X_HOSTFILTER_H_030304
