/**************************************************************************
 *  * @ This class is used to record the status of every hostport and 
 *  *   store the urls belongs to this hostports.
 *  *  in this class we extends the hasp_map_str2uc class which use to 
 *  *  keep the status of every hostport .Also we use CUrltab and BerkerlyDB
 *  * to store the urls of hostports. 
 *  * In our design , the urls keep in memory for improving the utilities.When
 *  * the urls occupy more resources , we will store some urls in the berkerly 
 *  * database . Also the urls can be write the files.
 *  * It  likes to base on a three layer memory structure: memory , half
 *  * memory and half disk( berklery db ) , disk.
 *  *    when get urls from the store architecure , we first get then from the
 *  *  urltab and dbmap , if no matching hostports , we will only visit dbmap
 *  *  to search the proper records; or we will check the hostpports refresh time,
 *  *  if the refreshing time over , return it and the refresh time.
 *  *
 *  *        required and instructed by xzm       
 *  *        implemented by wdh
 *  *        tested by 
 *  * 					07/26/2004	11:40
 ***************************************************************************/
#ifndef _PAT_HOSTPORTS_H_062904_
#define _PAT_HOSTPORTS_H_062904_
#include <iostream>
#include <sstream>
#include <string>

#include "pat_types.h"
#include "excep.h"
#include "arg.h"
#include "CDBStrMultiMap.h"

using namespace std;
using namespace pat_types;

enum hp_status_field{
	hp_url_ready=0x1, hp_tested=0x2, hp_qualified=0x4, hp_active=0x8
};

class CQMMsg;

typedef unsigned char hp_status;

struct hp_env
{
	int urlMaxSize;
	double urlDumpRatio ;
	int urlDumpSize ;
	int dbMaxSize  ;
	double dbDumpRatio ;
	int dbDumpSize ;
	
	int dirHashSize ;
	string dataDir ;
	int dbCacheSize ;
	string dbName ;
	string dumpFile ;

	bool use_db ; 
	
	hp_env()
	{
		urlMaxSize = 10000;
		urlDumpRatio = 0.3 ;
		urlDumpSize = 500;
		dbMaxSize = 5000000;
		dbDumpRatio = 0.001;
		dbDumpSize = 100;
	
		dirHashSize = 30 ;
		dataDir = "data";
		dbCacheSize = 20 * 1024 ;
		dbName ="berkdb";
		dumpFile = ".hostports";
		use_db = true ;
	}
};


class CHostportsErr : public excep
{
public:
	CHostportsErr()
	{
	}
	
	CHostportsErr( const string &m):excep( "Hostports::" + m )
	{
	}
};

class CHostports : public hash_map_str2u
{
public:
	CHostports();
	CHostports(const struct hp_env &_env);
	~CHostports();
	
	void put( const CQMMsg &url );
	int get( string &hostports);

	void getEnv(struct hp_env &_env) const;
	void setEnv(const struct hp_env &_env);

	bool dump();
	bool load();

public:
	void printEnv() const;
	void printEnv( ostream &os) const;

	int putHostport(const  string & hostport ,  unsigned refresh_time=0xFFFFFFFF , hp_status status = 0 );
	
	bool getRefreshTime(const const_iterator it, unsigned &refresh_time) const;
	bool getStatus(const const_iterator it, hp_status &status) const;
	bool getRefreshTime(const string &hostport, unsigned &refresh_time) const;
	bool getStatus(const string & hostport, hp_status &status ) const;

	bool setStatus( iterator it , hp_status status );
	bool setStatus(const string &hostport ,hp_status status );
	bool setRefreshTime( iterator it , unsigned refresh_time);
	bool setRefreshTime(const string &hostport , unsigned refresh_time );

	void printHostport(const string &hostport ,ostream &os) const;
	void printHostports( const_iterator it , int size  , ostream &os) const;

	bool setBeginHostport( const string &hostport );
	bool setBeginDBHostport( const string & hostport , int moveSize=0 );
public:
	void go2dir(const string & hostport) const;
	string getHpParentDir(const string &hostport) const;

private:
	void init();	
	
	bool putMsgToTab(const CQMMsg &msg);
	bool putMsgToDB(const CQMMsg &msg);
	
	bool msgAcceptable(const CQMMsg &msg ,string &hostport )const;
	
	int tabmsgToDB(int num);
	int tabmsgToDB( const string & hostport );
	
	int tabmsgToFile( int num );
	int tabmsgToFile( const string & hostport );
	
	int dbmsgToDisk( int num);
	
	bool partDump();

	int getUrlsFromTab(const string & hostport , vector<string> & urls);
	int getUrlsFromDB(const string & hostports , vector<string> &urls );
	
	int flush_ref(const string &hostport);
	void storeUrlToFile(const string & hostport ,const vector<string> & urls );
	
	unsigned getBaseTime()const;

private:
	CDBStrMultiMap *dbmap;
	multimap_str2str urltab;

	hp_env env , old_env;	
	
	bool isDumping;
	bool isDbDumping ;
	
	long long db_size ;
	
	bool keep_loop ;
	
	string working_dir;
	
	iterator current_iter, db_iter;
	unsigned base_time;	
	// the url table position
	typedef multimap_str2str::iterator urltab_iterator ;
	typedef multimap_str2str::value_type urltab_type ;

public:
void getUrlNum( long long &initNum , long long &insertNum , long long &outNum , long long &currentNum , long long &visitHp , long long & dumpHp);
	
private:
	long long inUrls , outUrls , initUrls , visitHps , dumpHps;
};
#endif // _PAT_HOSTPORTS_H_062904_
