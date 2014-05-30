#ifndef _PAT_INV_TAB_H_112805
#define _PAT_INV_TAB_H_112805
#include "pat_types.h"
#include <vector>
using std::vector;
using pat_types::hashstr;

struct word_app_t
{
	int doc_id;   // urlno
	unsigned int location;
};

typedef vector<word_app_t> word_apps_t;

class CInvTab : public hash_map<string, word_apps_t, hashstr>
{
public:
	bool dump(const string & fn);
	bool load(const string & fn);
private:
};
#endif // _PAT_INV_TAB_H_112805
