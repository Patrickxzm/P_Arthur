#ifndef _MWX_HOSTFILTER_H_100303_
#define _MWX_HOSTFILTER_H_100303_

#include <string>
#include <ext/hash_set>
#include <vector>
#include "util/pat_types.h"
using std::string;
using __gnu_cxx::hash_set;
using __gnu_cxx::hash;
using std::equal_to;
using std::vector;
using namespace pat_types;
enum line_type
{
	comment_line, iprange_line, solo_ip, domain_name, dot_domain
	, ip_dot, empty_line, all_line, over_flow, wrong_format
};

class CHostFilter
{
public:
	CHostFilter();
	int init(const char* ctlfile);
		/* open the config_file with filename and check the format of the file ;
		if done successfully , return  0 ;
		if open file failed , return -1 ;
		if wrong format , return the no. of wrong line .
		*/

	bool pass (const char* hostname) const;
		/*according to the config_file ,check whether the hostname can pass
		*/
	bool test_ip();
private:
	struct ipv4_range
	{
		unsigned int net;
		unsigned int mask;
	};
	bool flag_all; 	//if 1, all pass
	bool usable;
	bool __have_ip;
	hash_set<string, hashstr, equal_to<string> > ips;
	hash_set<string, hashstr, equal_to<string> > names;
	vector<struct ipv4_range> ip_ranges;
	vector<string> dot_domains;
	vector<string> ip_dots;

	line_type check(char* thisline) ;
	bool ipstr2uint(const string &str, unsigned &ipv4) const;
};

#endif // _MWX_HOSTFILTER_H_100303_
