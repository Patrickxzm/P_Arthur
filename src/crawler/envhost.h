#ifndef _PAT_ENVBANK_H_08262008
#define _PAT_ENVBANK_H_08262008
#include <string>
#include <mysql++.h>
#include <set>
#include <istream>
#include <ostream>
#include <libxml/tree.h>
#include "util/shadow.h"
#include "util/cutem.h"
#include "host4import.h"
using std::string;
using std::vector;
using std::set;
using std::istream;
using std::ostream;
using mysqlpp::Connection;
using mysqlpp::Row;

class CEnv
{
public:
	string host;
	int port;
	int pageNum;
	int interval;
	set<string> save_link_of;
};


class CHostTable
{
public:
	CHostTable(Connection *c);
	virtual ~CHostTable()
	{}
	/*
 	 * Return value: -1, connect database server failed.
	 *		-2, compress roots failed.
	 *		-3, no db rows to update.
	 *		-4, save shadows failed.
	 *	0, OK
	 */
	int saveResult(const string &path); 
	/*
	 * Return value: -1, connect database server failed.
	 *		-2, write database table failed.
	 *	0, OK
	 */
	int reportCrash(const string &hostport);
	/*
	 * Select hosts (by scheduler) to crawl.
	 *   Return value : 0, OK
	 *	-1, connect database server failed.
	 *	-2, no host in database to select
	 *	>0,  number of hosts failed to load.
	 */
	int selectStartEnv(vector<CEnv> &hosts, const string &up_dir); 
	/*
	 * Load environment of a host for "debug" purpose.
	 */
	bool loadStartEnv(CEnv &env, const string &path, const string &host, int port=80, bool fMark=false);
	/* 
	 * Import a host to crawl database, return value:
	 *  0  => OK, or already exists;
	 *  -3 => the site has already maximum hosts;
	 */
	int import(const CHost4Import &host, ostream *report, bool force=false);
private:
	/* 
	 * Import a host to crawl database, return value:
	 *  0  => OK;
	 *  -1 => host already exists;
	 *  -2 => execute "INSERT INTO site ..." failed;
	 *  -3 => the site has already maximum hosts;
	 *  -4 => execute "UPDATA site SET host_num ..." failed;
	 *  -5 => execute "INSERT INTO host ..." failed;
	 */
	int import(const string &host, int port, int &ID, bool force=false);
	int moreRoot(int ID, const vector<pair<string, CTask::status_type> > &seeds, bool create
		, ostream* report);
	int transLink(int ID, const string &target, ostream* report);
	bool loadStartEnv(CEnv &env, const string &path, const Row& row);
	/*
	 * shadow_id >0: update existing DB row, else insert a new row
	 * Return 0: update; >0: insert_id; -1: empty shadow
         */
	int saveShadow(const char* shadow_fn, int shadow_id, const string &host, unsigned port);
	/*
	 * Return >=0: Number of shadows in chain. -1: error
	 */
	int saveShadowChain(const string& path, const string &prefix, const string &host, unsigned port);
	int loadShadow(unsigned host_id, const string &path, xmlNodePtr shadow);
	Connection *conn;
	string crawler;
	cutem unzip_buf;
};

class Lock 
{
public:
	Lock(Connection *c, const string& table);
	virtual ~Lock();
private:
	Connection* conn;
	string table;
};

#endif // _PAT_ENVBANK_H_08262008
