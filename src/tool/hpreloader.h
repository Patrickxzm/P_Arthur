// HPReloader.h: interface for the CHPReloader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HPRELOADER_H__6E3C5C58_3B7D_44F5_B475_242BBF48EEB0__INCLUDED_)
#define AFX_HPRELOADER_H__6E3C5C58_3B7D_44F5_B475_242BBF48EEB0__INCLUDED_


#include <string>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <iostream>
#include "hostports.h"

using namespace std;

class CHPReloader  
{
public:
	
	
	CHostports getHostports();
	int print();
	void printNum();
	
	void loadHps();
	void loadFromData();
	void loadFromDB();
	void loadFromMissed();
	
	void save( char * file );
	CHPReloader(char * dataPath);
	virtual ~CHPReloader();	
	void help(ostream & os); 
	void setSearchRoot( char *  searchPath);

private:
	int port;
	string host;
	// the container keeping the recovered hostport and its status
	CHostports hps;
	//the data root directory
	string searchroot ;
	// the missed file name
	string missedfile ;
	//the data directory prefix
	string dataPrefix ;
	//the ref file name
	string refs ;
	//the status file name
	string statusPrefix;

	DIR * rootDir;
	DIR * dataDir;
	DIR * hpDir;
	
	// the data directory in the queen directory
	struct dirent * dataDirList;
	//the hp directory in the data directory
	struct dirent * hpDirList;
	//the file in the hostport directory
	struct dirent * hpDataList;
	long readyNum ;
	long testNum;
	long testReadyNum ;
	long undealNum;
	long totalNum;
	
protected:
	bool hpDirAcceptable();
	bool dataDirAcceptable();
	hp_status getHpStatus();
	bool isHpStr(string str);
	//int str2hp(string &str , string & host , int & port);
};

#endif // !defined(AFX_HPRELOADER_H__6E3C5C58_3B7D_44F5_B475_242BBF48EEB0__INCLUDED_)
