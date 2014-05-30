#ifndef _PAT_SCHEDULER_H_12252008
#define _PAT_SCHEDULER_H_12252008
#include "envhost.h"
#include <map>
#include <vector>
#include <ostream>

using std::map;
using std::vector;
using std::auto_ptr;
using std::ostream;

class CScheduler
{
public:
	CScheduler(auto_ptr<CHostTable> hostTable, Connection *c, const string &pathExport);
	virtual ~CScheduler();
	int run();
	static pid_t run_crawler(const CEnv &env, bool call_fork, bool debug=false);
	bool collect_outlinks;
	bool collect_overflow;
	bool collect_discard;
private:
	int startCrawler();  // select a host from database and crawl.
	int cleanCrawler();  // write back to database & transfer raw pages
	int cleanCrawler(const string &path);  
	int expose(const string& fileExport);
	static int exec_crawler(const CEnv &env, bool debug=false);
	int getProcessNum();
	int append(const string &from, const string &to, const string &head, const string &tail);
	int collectRaw(const string &page_fn, const string &path, time_t now);

	typedef map<pid_t, string> crawler_pool_t; // pid --> hostport
	crawler_pool_t pool;
	vector<CEnv> hosts;
	auto_ptr<CHostTable> hostTable;
	string pathExport;
	string fileExport;
	Connection *conn;
	string crawler;
	size_t daystr_len;
	pid_t pid;
};
#endif // _PAT_SCHEDULER_H_12252008
