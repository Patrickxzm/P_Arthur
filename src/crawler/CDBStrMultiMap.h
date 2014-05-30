#ifndef _GWD_CDBSTRMULTIMAP_072804_
#define _GWD_CDBSTRMULTIMAP_072804_
#include <iostream>
#include "db_cxx.h"
//#include "/usr/include/db_cxx.h"

#include <vector>


#define OO_CREAT 1
#define OO_TRUNC 2

using namespace std;
using std::vector;

class CDBStrMultiMap{
public:
	CDBStrMultiMap();
	~CDBStrMultiMap();
	
	bool open( const char* name, int flags );
	bool put( const string &key,  vector<string> &values);
	bool get( const string &key, vector<string> &values);
	bool del( const string &key );
	bool get_any( string &key , vector<string> &values);
	bool get_index( vector<string> &indexvec );
	bool set_cachesize ( int kbytes, int ncache);
        bool set_pagesize ( int pagesize );
	int  get_size();  
	int  get_numof_key();
	void close();
	
private:
	Db* db;
	bool db_is_open;
};
		
#endif // _GWD_CDBSTRMULTIMAP_072804_
