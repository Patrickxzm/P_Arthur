#ifndef _PAT_DNS_H_051503
#define _PAT_DNS_H_051503
#include <hash_map.h>
#include <string>

namespace dns {
	using std::string;
	struct eqstr {
		bool operator()(string s1, string s2) const
		{
			return s1==s2;
		}
	};

	typedef hash_multimap<string, string, hash<string>, eqstr> map_type;
};

class CDNS {
public:
	CDNS(const char* datafile);
	virtual ~CDNS();
	std::string& gethostbyname(const char*);
private:
	dns::map_type abc;
};
#endif /* _PAT_DNS_H_051503 */
