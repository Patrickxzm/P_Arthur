#ifndef _PAT_SHADOW_CHAIN_H_11302010
#define _PAT_SHADOW_CHAIN_H_11302010
#include <vector>
#include <string>
#include "util/shadow.h"
using std::vector;
using std::string;

class CShadowChain
{
public:
	CShadowChain()
	{}
	CShadowChain(const string &prefix, unsigned hintCapacity);
	~CShadowChain();
	// Return number of shadow chained, -1 on failure
	int open(const string &prefix, unsigned hintCapacity);
	void close();
	size_t size() const;

	bool put(const string &str);
	bool has(const string &str);
private:
	typedef CStrSetShadow* CStrSetShadowPtr;
	vector<CStrSetShadowPtr> _chain;
	string prefix;
};


#endif // _PAT_SHADOW_CHAIN_H_11302010
