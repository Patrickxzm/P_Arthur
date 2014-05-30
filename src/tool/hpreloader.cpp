// HPReloader.cpp: implementation of the CHPReloader class.
//
//////////////////////////////////////////////////////////////////////

#include "hpreloader.h"
#include "arg.h"
#include <sstream>
#include <iostream>
#include <time.h>
#include "CDBStrMultiMap.h"
#include <fstream>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHPReloader::CHPReloader(char * dataPath=".")
{ 
	searchroot = dataPath;
	// the data directory prefix
	dataPrefix = "data_";
	//the ref file name
	refs = "refs";
	//the status file name
	statusPrefix = "run.status.";
	testNum = 0 ;
	readyNum = 0 ;
	testReadyNum = 0 ;
	undealNum = 0 ;
	totalNum = 0 ;
	missedfile = ".missed";
}

CHPReloader::~CHPReloader()
{

}

void CHPReloader::loadHps()
{
	loadFromMissed();
	loadFromData();
	loadFromDB();
	save("");
}


void CHPReloader::loadFromDB()
{
	vector<string> vct ;
	CDBStrMultiMap dbmap;
	if(!dbmap.open( "berkdb", OO_CREAT ) ){
		throw CHostportsErr( "error in open berklery db " );
	}
	dbmap.get_index( vct );
	int now = time(0);
	for( int i = 0 ; i < (int)vct.size() ; i ++ )
	{
		hps.putHostport( vct[i] , now , hp_url_ready );
	}
}

void CHPReloader::loadFromMissed()
{
	ifstream ifs(missedfile.c_str() );
	string hostport;
	unsigned status; 
	while (ifs>>hostport>>hex>>status)
	{
		 hps.putHostport( hostport ,0xFFFFFFF0 , status );
	}
}

void CHPReloader::loadFromData()
{
	this->rootDir = opendir( this->searchroot.c_str() );
	if( rootDir == NULL ){
			  cerr << " open queen directory error :" << this->searchroot << " \n";
			  return ;
	}
	while( ( dataDirList = readdir(this->rootDir) ) != NULL){
		if( dataDirAcceptable() ){
			//the current file is a directory and the file name prefix is the value of dtaprefix
			string str = this->searchroot + "/" + string(dataDirList->d_name);
			this->dataDir = opendir( str.c_str() );
			cout << "visit the data directory " << str << endl;
			cout << "******the current hostport num is :\n" ;
			int now = time(0);
			int refresh_time = time(0) + 6 * 60 * 60 ;
			while( ( hpDirList = readdir(this->dataDir) ) != NULL){
				if( hpDirAcceptable() ){
	  				totalNum ++;					  
					hp_status status = getHpStatus();
					if( status & hp_tested )
						hps.putHostport( hpDirList->d_name , refresh_time , status );
					else
						hps.putHostport( hpDirList->d_name , now , status );
				}
			}
			closedir(dataDir);
		  	cout <<"\n\r total is :" << totalNum <<endl;
		}
	}
	closedir( rootDir );
}

CHostports CHPReloader::getHostports()
{
	return this->hps;
}


bool CHPReloader::isHpStr(string str)
{
	if( str.find(":" ) != string::npos )
		return true;
	return false;
}

/**
*get the current hostports status.
*/
hp_status CHPReloader::getHpStatus()
{
	string dirPath = this->searchroot + "/" + dataDirList->d_name + "/" + hpDirList->d_name ;
	hpDir = opendir( dirPath.c_str() );
	hp_status status = 0x0;
	string prefix1 = ".run.status";
	string prefix2 = "run.status";
	while( ( hpDataList = readdir(this->hpDir) ) != NULL){
		if(( strncasecmp(hpDataList->d_name,prefix1.c_str() , prefix1.length()) == 0 )
		|| ( strncasecmp(hpDataList->d_name,prefix2.c_str() , prefix2.length()) == 0 ))
		{
			status |= ( hp_tested | hp_qualified );			
		}
		else if( strncasecmp(hpDataList->d_name,this->refs.c_str() , refs.length()) == 0 )
		{			
			status |= hp_url_ready;	
		}		
	}
	if( status == ( hp_url_ready | hp_tested | hp_qualified ) )
			  testReadyNum ++ ;
	else if( status == ( hp_tested | hp_qualified ) )
			  testNum ++ ;
	else if( status == hp_url_ready )
			  readyNum ++ ;
	else
			  undealNum ++ ;
	closedir( hpDir);
	return status;
}


void CHPReloader::help(ostream &os )
{
		 os<<" this class is for rebuilding the hostports status.\n"
			  " -h,--help , put this help information into the osstream os.\n"
		  	  " -pFILE , given the current data location.the default data path is the current path\n"
			  " -sFILE , given the file name stored the hostports.\n";
		 return;
}
/*
* check the current data directory is can be accepted .
*/
bool CHPReloader::dataDirAcceptable()
{
	if( dataDirList->d_type == 4 &&
		strncasecmp(dataDirList->d_name,this->dataPrefix.c_str(), dataPrefix.length()) == 0 )
		return true;
	return false;
}

/*
* check the current hp directory is can be accepted .
*/
bool CHPReloader::hpDirAcceptable()
{
	if( (hpDirList->d_type == 4) && this->isHpStr(hpDirList->d_name) )
		return true;
	return false;
}


int CHPReloader::print()
{
		  CHostports::const_iterator it ;
		  for( it = hps.begin(); it != hps.end();it ++ )
					cout << it->first <<"  " << it->second << endl;
		  return 0;
}

void CHPReloader::printNum()
{
		  cout << "*******************************************\n"
					 "the result is :\n";
		  cout << " total hostports is :" << (testNum + readyNum + testReadyNum + undealNum) <<endl;
		  cout << " undeaded hostports is :" << undealNum << endl;
		  cout <<" ready hostports is:" << readyNum << endl;
		  cout <<" tested and qualified hostports is :" << testNum << endl;
		  cout <<" readly ,tested and qualified hosports is :" << testReadyNum << endl;
		  cout <<"****************** OVER *******************\n" ;
}

void CHPReloader::setSearchRoot( char *  searchPath){
		this->searchroot =string( searchPath );
		cout << "root is:" << this->searchroot<<endl;
}

void CHPReloader::save(char * file)
{
		  hps.dump();
}

	

int main(int argc , char ** argv)
{
		  CArg arg(argc , argv);
		  if( *arg.find("-h") || *arg.find("--h") ){
					 CHPReloader rr;
					 rr.help(cout);
					 return 1;
		  }
		  char ** path = arg.find("-p");
		  CHPReloader reloader ;
		  if( *path != 0 ){
		 	reloader.setSearchRoot(*path);
		  }			 
		  reloader.loadHps();
		  
		  reloader.printNum();
		  printf(" ok here.\n");
		  return 1;
}
