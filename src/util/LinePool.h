/***************************************************************************
                          LinePool.h  -  description
                             -------------------
    begin                : Fri Jan 2 2004
    copyright            : (C) 2004 by 
    email                : 
 ***************************************************************************/

#ifndef __LINEPOOL_H__
#define __LINEPOOL_H__

/**
  *@author HLE
  */

#include <string>
#include <fstream>
#include <list>

// CLinePool use a file to store lots of line messages.

class CLinePool
{
public: 
	CLinePool(const char* sPoolFile);
	~CLinePool();

	bool isOpen(){
		return m_bOpen;
	}
	unsigned int getCount(){
		return m_nRecordCount;
	}
	long long getDiscardedCount(){
		return m_nLinesDiscarded;
	}

	void pushALine(std::string strLine);
	void pushAPair(std::string strName, std::string strValue);
	bool popALine(std::string& strLine);
	bool popAPair(std::string& strName, std::string& strValue);

	void saveData();

protected:
	struct THeadData
	{
		char m_sMark[16];
		char m_sMaxFileSize[24];
		char m_sMinBeginPos[24];
		char m_sCurFileSize[24];
		char m_sBeginPos[24];
		char m_sEndPos[24];
		char m_sRecCount[16];
		char m_sUsage[16];
	};
protected:
	const char* m_sMarkName;
	const int m_nMaxFileSize;
	const int m_nMinBeginPos;
	int m_nCurFileSize;
	int m_nBeginPos;
	int m_nEndPos;

	int m_nOldCurFileSize;
	int m_nOldBeginPos;
	int m_nOldEndPos;
	
	int m_nRecordCount;
	long long m_nLinesDiscarded;
	std::string m_strFileName;
	std::ofstream m_ofsPool;
	std::istream* m_pinsPool;
	bool m_bOpen;

	char m_sHeadBuf[256];

protected:
	void initPoolData();
	bool openFile();
	void writeHeadData();
	bool readHeadData();

	int getFreeSpace();
	int getDataSize();
	bool isDataOutOfDate();
};

#endif
