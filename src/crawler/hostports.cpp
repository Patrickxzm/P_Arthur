#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <limits>
#include <errno.h>

#include "hostports.h"
#include "qmmsg.h"
#include "url.h"
#include "host.h"
#include "arg.h"

#define BASE_TIME 1072933200

CHostports::CHostports()
{
	dbmap = NULL;	
	isDumping = false;
	isDbDumping = false;
	init();	
}

CHostports::CHostports(const struct hp_env &_env)
{
	dbmap = NULL;
	env = _env;	
	old_env = env;
	isDumping = false;
	isDbDumping = false;
	init();	

}	

CHostports::~CHostports()
{
	if( dbmap != NULL )
		dbmap->close();
	delete dbmap ;
}

void CHostports::setEnv(const hp_env &_env)
{
	old_env = env;
	env = _env;
	init();
}

void CHostports::getEnv( hp_env &_env)const
{
	_env = env;
}

void CHostports::init()
{
	if( env.dbName.length() == 0 )
		throw CHostportsErr( "no db to open. berklery name size is zero " );
	if( dbmap != NULL  ){
		dbmap->close();
		delete dbmap;
	}
	       	
	dbmap = new CDBStrMultiMap();
	if(!dbmap->open( env.dbName.c_str(), OO_CREAT ) ){
		throw CHostportsErr( "error in open berklery db " + env.dbName );
	}
	dbmap->set_cachesize( env.dbCacheSize, 1 );
	
	current_iter = begin();
	db_iter = begin();
	
	char buf[100];
	if(NULL == getcwd( buf , sizeof(buf)) && errno !=EEXIST )
		throw CHostportsErr(string("CHostports()::getcwd():")+buf+':'
			+strerror(errno));
	working_dir = buf;
	keep_loop = false;
	
	base_time = BASE_TIME;

	db_size = dbmap->get_size() ;
	initUrls = db_size ;
	inUrls = 0 ;
	outUrls = 0 ;
	visitHps = 0 ;
	dumpHps = 0 ;
}


void CHostports::put( const CQMMsg &url )
{
	if(putMsgToTab( url ))
		inUrls ++ ;
	if( env.urlMaxSize > 0 && (int)urltab.size() >= env.urlMaxSize ){
		isDumping = true;
	}
	partDump();

}

bool CHostports::putMsgToTab(const CQMMsg &msg)
{
	string hostport;
	if( msgAcceptable( msg, hostport)){
		if( !putHostport( hostport ,0xFFFFFFFF , hp_url_ready) )
		{
			setRefreshTime( hostport , time(0) + 60 * 60 );	
		}
		urltab.insert( urltab_type( hostport, msg.getMsg()));
	
		return true;
	}
	return false;
}

bool CHostports::partDump()
{
	if( !env.use_db )
	{
		if( isDumping )
			tabmsgToFile( env.urlDumpSize );
		
		if( urltab.size() < env.urlMaxSize *(1 - env.urlDumpRatio) )
			isDumping = false;
		
		return true;
	}	
	if( isDumping )
	{
		db_size += tabmsgToDB( env.urlDumpSize );
		
		if( urltab.size() < env.urlMaxSize *(1 - env.urlDumpRatio) )
			isDumping = false;
		
		if( db_size >= env.dbMaxSize )
			isDbDumping = true;
		return true;
	}
	if( isDbDumping )
	{
		int total = dbmsgToDisk( env.dbDumpSize );
		db_size -= total ;
		
		if ( total == 0 )
			isDbDumping = false;
		else if ( db_size <=0 || db_size < env.dbMaxSize * (1  - env.dbDumpRatio) )
			isDbDumping = false;
	}
	return true;
	
}
int CHostports::tabmsgToDB(int num)
{
	urltab_iterator it = urltab.begin() ;
	int total = 0;
	while ( (total <= num || num < 0 ) && (it != urltab.end()) ){
		string hostport = it->first;
		total += tabmsgToDB( hostport );
		it = urltab.begin();
	}
	return total ;
}

int CHostports::tabmsgToDB( const string & hostport )
{
	vector<string> urls;
	pair< urltab_iterator, urltab_iterator > pos ;
	pos = urltab.equal_range( hostport );
	for( ; pos.first != pos.second ; urltab.erase( pos.first ++ ))
		urls.push_back( pos.first->second );
	
	dbmap->put( hostport ,urls );
	return urls.size();
}

int CHostports::tabmsgToFile(int num)
{               
        urltab_iterator it = urltab.begin() ;
        int total = 0;
        while ( (total <= num || num < 0 ) && (it != urltab.end()) ){
                string hostport = it->first;
                tabmsgToFile( hostport );
                it = urltab.begin();
                total ++ ;
        }
        return total ;
}

int CHostports::tabmsgToFile( const string & hostport )
{       
	vector<string> urls;
	pair< urltab_iterator, urltab_iterator > pos ;
	pos = urltab.equal_range( hostport );
	for( ; pos.first != pos.second ; urltab.erase( pos.first ++ ))
		urls.push_back( pos.first->second );
	storeUrlToFile( hostport ,urls );
	return urls.size();
}

int CHostports::dbmsgToDisk(int num)
{
	int total = 0;
	string hostport ;
	vector <string > urls;
	if( db_iter == end() )
		db_iter =  begin();
	iterator cur_it = db_iter;
		
	while( num < 0 || num > total )	{
		urls.clear();
		hostport = db_iter->first ;
		dbmap->get( hostport , urls );
		
		if( urls.size() > 0 ){
			total += urls.size();
			pair< urltab_iterator, urltab_iterator > pos ;
			pos = urltab.equal_range( hostport );
			for( ; pos.first != pos.second ; urltab.erase( pos.first ++ ))
				urls.push_back( pos.first->second );
			
			storeUrlToFile( hostport ,urls );
		}
		db_iter++ ;
		if( db_iter == end() )
			db_iter = begin();

		if( db_iter == cur_it ){
			cout << " visit all hostports . no enough record found. " << endl;
			return total;
		}
	}
	return total;
}


bool CHostports::putMsgToDB(const CQMMsg &msg)
{
	string hostport;
	if( msgAcceptable( msg, hostport)){
		
		if(!putHostport( hostport, 0xFFFFFFFF , hp_url_ready )){
			iterator it = find( hostport );
			it->second = it->second | hp_url_ready ;
		}
		
		vector<string> urls;
		urls.push_back( msg.getMsg() );
		dbmap->put( hostport, urls );
		inUrls ++;
		return true;
	}
	return false;
}

bool CHostports::msgAcceptable(const CQMMsg &msg ,string &hostport )const
{
	QM_MsgType type = msg.getType();
	string urlfrom, urlto, anchor;
	if (type == QM_REFER)
	{
		if (!msg.retrieve_REFER(urlfrom, urlto, anchor))
			return false;
	}
	else if (type == QM_REDIRECT)
	{
		if (!msg.retrieve_REDIRECT(urlfrom, urlto))
			return false;
	}
	else 
	{
		return false;
	}
	CURL url(urlto);
	if ( url.protocol()!=string("http"))
		return false;
	if (url.host().length() == 0)
		return false;
	hostport = url.hostport();
	return true;
}

int CHostports::get( string &hostport )
{
	bool finded = false;
	unsigned latest_time = numeric_limits<unsigned>::max() ;
	if( size() == 0 )
		return -1;
	if( current_iter == end() )
		current_iter = begin();
	iterator it = current_iter;
	while( !finded )
	{
		if(!( it->second & hp_active ))
		{
			hostport = it->first;
			unsigned refresh_time ;
		       	getRefreshTime( it, refresh_time );
			latest_time = latest_time < refresh_time ? latest_time:refresh_time ;
				
			if( refresh_time < (unsigned)time(0) ){
				flush_ref( hostport );
				it->second = 0xFFFFFFF0 | (it->second & ~hp_url_ready) ;
				finded = true;
			}					
		}
		it ++ ;
		if( it == end() )
			it = begin();
		if( it == current_iter ){
			break;
		}
	}
	current_iter = it ;
	if( finded ){
		visitHps ++;
		return 0;
	}
	return (int)latest_time;
		
}

int CHostports::getUrlsFromTab(const string & hostport , vector<string> & urls)
{
	int num = 0 ;
	pair<urltab_iterator,urltab_iterator> pos ;
	pos = urltab.equal_range( hostport );	
	for( ; pos.first != pos.second ; urltab.erase( pos.first++ ) ){
		urls.push_back( pos.first->second );
		num ++ ;
	}
	return num;
}


int CHostports::getUrlsFromDB(const string & hostport , vector<string> &urls )
{
	int size = urls.size() ;
	dbmap->get( hostport , urls ) ;
	size = urls.size() - size ;
	
	return size;
}

int CHostports::flush_ref(const string &hostport)
{
	vector<string> urls;
	int result = 0;
	
	if( env.use_db )
		result = getUrlsFromDB( hostport , urls );
	
	db_size -= result ;
	
	result += getUrlsFromTab( hostport , urls );
	if( result == 0 )
		return 0;
	storeUrlToFile( hostport , urls );
	return result;	
}

void CHostports::storeUrlToFile(const string & hostport ,const vector<string> & urls )
{
	go2dir(hostport);
	ofstream refs("refs", std::ios::app|std::ios::out);
	for (unsigned int i=0; i< urls.size(); i++)
		refs << urls[i] << endl;
	refs.close();
	if (-1 == chdir(working_dir.c_str()))
	{
		ostringstream oss;
		oss<<"flush_ref()::chdir():"<<working_dir<<':'<<strerror(errno);
		throw CHostportsErr(oss.str());
	}
	outUrls += urls.size();
	dumpHps ++ ;
	return;
}


void
CHostports::go2dir(const string & hostport)const
{
		
	string dir = getHpParentDir( hostport );
	
	if (-1==mkdir(dir.c_str(), 0740) && errno!=EEXIST)
		throw CHostportsErr(string("go2dir()::mkdir():")+dir+':'
			+strerror(errno));
	if (-1==chdir(dir.c_str()))
		throw CHostportsErr(string("go2dir()::chdir():")+dir+':'
			+strerror(errno));
	if (-1==mkdir(hostport.c_str(), 0740) && errno != EEXIST)
		throw CHostportsErr(string("go2dir()::mkdir():")+hostport+" : "
			+strerror(errno));
	if (-1==chdir(hostport.c_str()))
		throw CHostportsErr(string("go2dir()::chdir():")+hostport+" : "
			+strerror(errno));
	
	return ;
}

string CHostports::getHpParentDir(const string &hostport)const
{
	ostringstream oss;
	oss<< env.dataDir;
	if( env.dirHashSize > 0)
	{
		hash<const char*> H;
		unsigned sn = H(hostport.c_str()) % env.dirHashSize;
		oss<<'_'<<sn;
	}
	oss<<'/';
	return oss.str();
}


unsigned CHostports::getBaseTime() const
{
	return base_time;
}

bool CHostports::getStatus(const string & hostport , hp_status & status )const
{
	const_iterator it = find( hostport );
	return getStatus( it , status );
}

bool CHostports::getStatus( const_iterator it , hp_status & status )const
{
	if( it == end() )
		return false;
       	int value = it->second;
	status =  value & 0xf ;	
	return true;
}

bool CHostports::getRefreshTime(const string &hostport , unsigned &refresh_time )const
{
	const_iterator it = find( hostport);
	return getRefreshTime( it , refresh_time );
}

bool CHostports::getRefreshTime(const_iterator it , unsigned &refresh_time )const
{
	if( it == end() )
		return false;
	refresh_time = it->second;
	if( refresh_time >=0xFFFFFF00){
		refresh_time &=0x7FFFFFFF ;
		return true;
	}
	refresh_time = refresh_time >> 4;
	refresh_time = refresh_time * 60 +  getBaseTime();
	return true;
}

bool CHostports::setRefreshTime(const string &hostport , unsigned refresh_time )
{
	if( hostport.length() == 0)
		return false;
	iterator it = find( hostport);
	return setRefreshTime( it , refresh_time );
}

bool CHostports::setRefreshTime( iterator it , unsigned refresh_time)
{
	if( it == end() )
		return false;
	hp_status status;	
	unsigned val = (refresh_time - getBaseTime()) / 60 ;
	val = val << 4;
	getStatus( it ,status);
	val |= status;
	if(val < it->second )
		it->second = val ;
	return true;
}

bool CHostports::setStatus(const string &hostport , hp_status status )
{
	if( hostport.length() == 0)
		return false;
	iterator it = find( hostport);
	return setStatus( it , status );
}

bool CHostports::setStatus( iterator it , hp_status status )
{
	if( it == end() )
		return false;
	
	it->second &= 0xFFFFFFF0;
	it->second |= status;
	return true;
}

int CHostports::putHostport(const string & hostport , unsigned refresh_time , hp_status status )
{
	iterator it = find( hostport );
	if( it != end() ){
		return 0;
	}
	if( refresh_time < 0xFFFFFFFF ){
		refresh_time = refresh_time > getBaseTime() ? (refresh_time - getBaseTime()):0;
		refresh_time /= 60;
	}
	else{
		refresh_time = ( time(0) - getBaseTime()) / 60 ;
	}
	
	unsigned value = (refresh_time << 4 );	
	value |= status;
	insert(pair<string ,unsigned>(hostport , value));
	return 1;
}


bool CHostports::setBeginHostport( const string &hostport )
{
	iterator iter ;
	iter = hostport.length() > 0 ? find( hostport) : end();
	if( iter != end () )
	{
		current_iter = iter ;
		return true;
	}
	return false;
}

bool CHostports::setBeginDBHostport( const string & hostport , int moveSize)
{
	iterator iter ;
	iter = hostport.length() > 0 ? find( hostport) : end();
	if( iter != end () )
	{
		db_iter = iter ;
		for( int i = 0 ; i < moveSize ; i ++ )
		{
			db_iter ++;
			if( db_iter == end() )
				db_iter = begin();
		}
		return true;
	}
	return false;
}

bool CHostports::dump()
{
	tabmsgToDB( -1 );
	
	ofstream ofs(env.dumpFile.c_str() );
	const_iterator citer;
	for (citer = begin(); citer != end(); citer++)
	{
		ofs<<citer->first<<'\t'<<hex<<(unsigned)citer->second<<'\n';
	}
	if( current_iter == end() )
		current_iter = begin();
	if( db_iter != end() )
		db_iter == begin();
	if( current_iter != end() )		
		ofs<<current_iter->first << '\t' << hex << (unsigned)current_iter->second <<'\n';
	if( db_iter != end() )
		ofs<<db_iter->first << '\t' << hex << (unsigned)db_iter->second <<'\n';
	return true;
}

bool CHostports::load()
{
	ifstream ifs(env.dumpFile.c_str() );
	string hostport;
	unsigned status;
	vector<string> vct;
	while (ifs>>hostport>>hex>>status)
	{
		if( find( hostport ) == end() )
			operator[](hostport) = status;
		else
			vct.push_back( hostport );
	}
	if( vct.size() > 1 ){
		current_iter = find( vct[0] );
		db_iter = find( vct[1] );
	}
	else if( vct.size() == 1)
	{
		current_iter = find( vct[0] );
		if( current_iter == end() )
			current_iter = begin() ;
		db_iter = current_iter ;
		for( int i = 0 ; i < 1000 ; i ++ )
		{
			if( db_iter == end() )
				db_iter = begin();
			db_iter++ ;
		}
	}
	return true;
}

void CHostports::printHostport(const string &hostport , ostream &os)const
{
	const_iterator it = find( hostport);
	return printHostports( it ,1 , os );
}

void CHostports::printHostports( const_iterator it , int size, ostream &os )const
{
	unsigned rtime ;
	hp_status status;
	time_t refresh_time ;
	if( it == end() )
		return ;
	for( int i = 0 ; (i < size || size <= 0 ) && it != end() ; i++ , it ++ )
	{
		getRefreshTime( it , rtime);
		getStatus( it , status );
		refresh_time = rtime ;
		if( refresh_time <0 )
			refresh_time = 0x7FFFFFFF;	
		os << (i + 1) << ":" << it->first <<"\t"<< ctime( &refresh_time ) 
			<<"\t"	<< (int)status  << endl;
	}
}

void CHostports::printEnv() const
{
	printEnv( cout );
}

void CHostports::printEnv( ostream &os) const
{
	os << " url Max size : " << env.urlMaxSize << endl;
	os << " url dump ratio :" << env.urlDumpRatio<<endl;
	os << " url dump size : " << env.urlDumpSize<<endl;
	os << " db max sizes : " << env.dbMaxSize<<endl;
	os << " db dump ratio: " << env.dbDumpRatio<<endl;
	os << " db dump size :" << env.dbDumpSize<<endl;	
	os << " dir hash size :" << env.dirHashSize << endl;
	os << " data dir : " << env.dataDir << endl;
	os << " cache size:" << env.dbCacheSize << endl;
	os << " db name : " << env.dbName << endl;
	os << " dump file :" << env.dumpFile << endl ;
	os << " url size :" << urltab.size() << endl;
	os << " db size :" << db_size << endl;
	os << " hostports size:" << size() << endl;

	os << " init url num :" << initUrls << endl;
	os << " insert urls :"  << inUrls << endl;
	os << " out urls :" << outUrls << endl;
	os << " current urls :" << urltab.size() + db_size << endl;
	
}



void CHostports::getUrlNum( long long &initNum , long long &insertNum , long long &outNum , long long &currentNum , long long &visitHp , long long &dumpHp )
{
	initNum = initUrls;
	insertNum = inUrls ;
	outNum = outUrls ;
	currentNum = urltab.size() + db_size ;
	visitHp = visitHps;
	dumpHp = dumpHps;
}
