
#define _PAT_DISTRIBUTE_H_100903

/* 
 * A simple way to distribute Web hosts among crawling nodes.
 * We just devide the web hosts into serval parts staticly, and distribute them
 * to each node.
 */

#include <string>
#include <vector>
using std::string;
using std::vector;

class CDistribute
{
public:
	void read_arg(const string &arg);
	bool pass(const string &host);
private:
	unsigned total;
	vector<unsigned> parts;
};
#endif // _PAT_DISTRIBUTE_H_100903
