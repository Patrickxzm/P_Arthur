/***************************************************************************
                          TestPool.cpp  -  description
                             -------------------
    begin                : Sat Jan 3 2004
    copyright            : (C) 2004 by 
    email                : 
 ***************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include "LinePool.h"
using namespace std;

int main(int argc, char* argv[])
{
	CLinePool iPool("/tmp/msg_pool");
	bool bOpen = iPool.isOpen();
	if(! bOpen){
		cout << "Not open." << endl;
	}
	char sBuf[4096];
	for(int i=0; i<3; i++){
		sprintf(sBuf, "url\t%03d", i);
		iPool.pushALine(sBuf);
	}
	iPool.saveData();
	string strLine;
	int nCount = 0;
	while(iPool.getCount() > 0){
		nCount ++;
		if(iPool.popALine(strLine)){
			cout << strLine << endl;
		}else{
			cout << "READ ERROR" << endl;
		}
	//	if(nCount >= 3000) break;
	}

	return 0;
}
