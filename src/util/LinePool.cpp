/***************************************************************************
                          LinePool.cpp  -  description
                             -------------------
    begin                : Fri Jan 2 2004
    copyright            : (C) 2004 by 
    email                : 
 ***************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <cassert>
#include <string>
#include "LinePool.h"

using namespace std;

CLinePool::CLinePool(const char* sPoolFile)
:m_sMarkName("LinePool 1.0"), m_nMaxFileSize(1024 * 1024 * 1024), m_nMinBeginPos(256)
{
	this->m_strFileName = sPoolFile;
	this->m_pinsPool = NULL;
	this->m_nLinesDiscarded = 0;

	this->initPoolData();

	this->m_bOpen = false;
	this->openFile();
}

CLinePool::~CLinePool()
{
	if(this->m_pinsPool != NULL){
		delete this->m_pinsPool;
	}
	if(this->m_bOpen){
		this->writeHeadData();
		this->m_ofsPool.close();
	}
}

void CLinePool::initPoolData()
{
	this->m_nRecordCount = 0;
	this->m_nCurFileSize = this->m_nMinBeginPos;
	this->m_nBeginPos = this->m_nMinBeginPos;
	this->m_nEndPos = this->m_nMinBeginPos;

	this->m_nOldCurFileSize = this->m_nCurFileSize;
	this->m_nOldBeginPos = this->m_nBeginPos;
	this->m_nOldEndPos = this->m_nEndPos;
}

void CLinePool::pushALine(std::string strLine)
{
	for(int i=strLine.length()-1; i>=0; i--){
		if(strLine[i] == '\n')strLine.erase(i);
	}
	if(! this->m_bOpen){
		this->m_nLinesDiscarded ++;
		return;
	}
	if(this->getFreeSpace() < int(strLine.length() + 256)){ // No space.
		this->m_nLinesDiscarded ++;
		return;
	}

	this->m_ofsPool.seekp(this->m_nEndPos);
	strLine += '\n';
	int nLeft = this->m_nMaxFileSize - this->m_nEndPos;
	if(nLeft < 0) nLeft = 0;
	if((int)strLine.length() <= nLeft){
		this->m_ofsPool.write(strLine.c_str(), strLine.length());
		this->m_nEndPos = this->m_ofsPool.tellp();
		this->m_nRecordCount ++;
		if(this->m_nCurFileSize < this->m_nEndPos) this->m_nCurFileSize = this->m_nEndPos;
	}else{
		string strFirst = strLine.substr(0, nLeft);
		string strSecond = strLine.substr(nLeft, strLine.length() - nLeft);
		this->m_ofsPool.write(strFirst.c_str(), strFirst.length());
		this->m_nEndPos = this->m_ofsPool.tellp();
		if(this->m_nCurFileSize < this->m_nEndPos) this->m_nCurFileSize = this->m_nEndPos;
		this->m_ofsPool.seekp(this->m_nMinBeginPos);
		this->m_ofsPool.write(strSecond.c_str(), strSecond.length());
		this->m_nEndPos = this->m_ofsPool.tellp();
		this->m_nRecordCount ++;
	}

	if(this->isDataOutOfDate()) this->writeHeadData();
}

void CLinePool::pushAPair(std::string strName, std::string strValue)
{
	if(! this->m_bOpen){
		this->m_nLinesDiscarded ++;
		return;
	}
	
	for(int i=strName.length()-1; i>=0; i--){
		if(strName[i] == '\t') strName[i] = ' ';
	}
	this->pushALine(strName+'\t'+strValue);
}

bool CLinePool::popALine(std::string& strLine)
{
	static char sLine[4096];
	if(! this->m_bOpen) return false;
	if(this->m_nRecordCount < 1) return false;

	this->m_nRecordCount --;
	int nOldBeginPos = this->m_nBeginPos;
	
	sLine[0] = '\0';
	this->m_pinsPool->clear();
	this->m_pinsPool->seekg(this->m_nBeginPos);
	int nRead = this->m_pinsPool->getline(sLine, 4095).gcount();
	this->m_nBeginPos = this->m_pinsPool->tellg();
	if(this->m_nBeginPos < this->m_nMinBeginPos){
		this->m_nBeginPos = this->m_nMinBeginPos;
	}
	if((nOldBeginPos <= this->m_nEndPos) && (this->m_nBeginPos >= this->m_nEndPos)){
		this->m_nRecordCount = 0;
	}
	if(this->m_pinsPool->eof() || (this->m_nBeginPos >= this->m_nMaxFileSize)){
		this->m_pinsPool->clear();
		this->m_pinsPool->seekg(-1, std::ios::end);
		char c;
		this->m_pinsPool->get(c);
		this->m_pinsPool->clear();
		if(c != '\n'){
			this->m_pinsPool->seekg(this->m_nMinBeginPos);
			this->m_pinsPool->getline(sLine+nRead, 4095-nRead);
			this->m_nBeginPos = this->m_pinsPool->tellg();
			if(this->m_nBeginPos < this->m_nMinBeginPos){
				this->m_nBeginPos = this->m_nMinBeginPos;
				return false;
			}
		}else{
			this->m_nBeginPos = this->m_nMinBeginPos;
		}
		if((nOldBeginPos > this->m_nEndPos) && (this->m_nBeginPos >= this->m_nEndPos)){
			this->m_nRecordCount = 0;
		}
	}
	strLine = sLine;

	if(this->m_nRecordCount == 0){
		this->m_nEndPos = this->m_nMinBeginPos;
		this->m_nBeginPos = this->m_nEndPos;
	}
	return true;
}

bool CLinePool::popAPair(std::string& strName, std::string& strValue)
{
	if(! this->m_bOpen) return false;

	string strLine;
	if(! this->popALine(strLine))return false;

	int nLen = strLine.length();
	for(int i=0; i<nLen; i++){
		if(strLine[i] == '\t'){
			strName = strLine.substr(0, i);
			strValue = strLine.substr(i+1, nLen-i-1);
			return true;
		}
	}

	return false;
}

bool CLinePool::openFile()
{
	struct stat iFileStat;
	if( stat(this->m_strFileName.c_str(), &iFileStat) != 0){
		this->m_ofsPool.open(this->m_strFileName.c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
	}else{
		this->m_ofsPool.open(this->m_strFileName.c_str(), std::ios::binary | std::ios::in | std::ios::out);
	}
	
	if(! this->m_ofsPool.is_open()){
		std::cerr << "Error: can't open file \"" << this->m_strFileName
			<< "\", message pool not available." << std::endl;
		return false;
	}
	this->m_pinsPool = new istream(this->m_ofsPool.rdbuf());
	if(! this->m_pinsPool){
		std::cerr << "Error: can't open istream for file \"" << this->m_strFileName
			<< "\", message pool not available." << std::endl;
		return false;
	}

	this->m_bOpen = true;
	if(! this->readHeadData()){
		this->initPoolData();
		this->writeHeadData();
	}
	return true;
}

void CLinePool::saveData()
{
	this->writeHeadData();
}

void CLinePool::writeHeadData()
{
	assert(this->m_bOpen);

	for(int i=0; i<255; i++){
		this->m_sHeadBuf[i] = ' ';
	}
	this->m_sHeadBuf[255] = '\n';

	THeadData* pHeadData = (THeadData*)this->m_sHeadBuf;
	strcpy(pHeadData->m_sMark, this->m_sMarkName);
	sprintf(pHeadData->m_sMaxFileSize, "max size: %d", this->m_nMaxFileSize);
	sprintf(pHeadData->m_sMinBeginPos, "head len: %d", this->m_nMinBeginPos);
	sprintf(pHeadData->m_sCurFileSize, "cur size: %d", this->m_nCurFileSize);
	sprintf(pHeadData->m_sBeginPos, "begin: %d", this->m_nBeginPos);
	sprintf(pHeadData->m_sEndPos, "end: %d", this->m_nEndPos);
	sprintf(pHeadData->m_sRecCount, "count: %d", this->m_nRecordCount);
	sprintf(pHeadData->m_sUsage, "usage: %4.1f%%",
			(this->getDataSize()/512*100.0) / (this->m_nMaxFileSize-this->m_nMinBeginPos-256));

	this->m_nOldCurFileSize = this->m_nCurFileSize;
	this->m_nOldBeginPos = this->m_nBeginPos;
	this->m_nOldEndPos = this->m_nEndPos;

	std::ios::pos_type posOld = this->m_ofsPool.tellp();
	this->m_ofsPool.seekp(0);
	this->m_ofsPool.write(this->m_sHeadBuf, this->m_nMinBeginPos);
	this->m_ofsPool.seekp(posOld);
	this->m_ofsPool.flush();
}

bool CLinePool::readHeadData()
{
	assert(this->m_bOpen);

	std::ios::pos_type posOld = this->m_pinsPool->tellg();
	this->m_pinsPool->seekg(0);
	int nCount = (this->m_pinsPool->read(this->m_sHeadBuf, this->m_nMinBeginPos)).gcount();
	if(this->m_pinsPool->fail()){
		this->m_pinsPool->clear();
		return false;
	}
	if(posOld >= 0){
		this->m_pinsPool->seekg(posOld);
	}

	if(nCount < (int)this->m_nMinBeginPos){
		return false;
	}

	THeadData* pHeadData = (THeadData*)this->m_sHeadBuf;
	if(strcmp(pHeadData->m_sMark, this->m_sMarkName) != 0) return false;
	if(sscanf(pHeadData->m_sCurFileSize, "cur size: %d", &this->m_nCurFileSize) != 1) return false;
	if(this->m_nCurFileSize > this->m_nMaxFileSize) return false;
	if(sscanf(pHeadData->m_sBeginPos, "begin: %d", &this->m_nBeginPos) != 1) return false;
	if((this->m_nBeginPos < this->m_nMinBeginPos) || (this->m_nBeginPos > this->m_nMaxFileSize)) return false;
	if(sscanf(pHeadData->m_sEndPos, "end: %d", &this->m_nEndPos) != 1) return false;
	if((this->m_nEndPos < this->m_nMinBeginPos) || (this->m_nEndPos > this->m_nMaxFileSize)) return false;
	if(sscanf(pHeadData->m_sRecCount, "count: %d", &this->m_nRecordCount) != 1) return false;
	if(this->m_nRecordCount < 0) return false;

	if(this->m_nRecordCount == 0){
		this->m_nEndPos = this->m_nMinBeginPos;
		this->m_nBeginPos = this->m_nEndPos;
	}
	
	this->m_nOldCurFileSize = this->m_nCurFileSize;
	this->m_nOldBeginPos = this->m_nBeginPos;
	this->m_nOldEndPos = this->m_nEndPos;

	return true;
}

int CLinePool::getFreeSpace()
{
	int nSize;
	if(this->m_nBeginPos <= this->m_nEndPos){
		nSize = this->m_nMaxFileSize - this->m_nMinBeginPos - this->m_nEndPos + this->m_nBeginPos;
	}else{
		nSize = this->m_nBeginPos - this->m_nEndPos;
	}
	if(nSize < 0) nSize = 0;
	return nSize;
}

int CLinePool::getDataSize()
{
	if(this->m_nBeginPos <= this->m_nEndPos){
		return this->m_nEndPos - this->m_nBeginPos;
	}else{
		return this->m_nCurFileSize - this->m_nBeginPos + this->m_nEndPos - this->m_nMinBeginPos;
	}
}

bool CLinePool::isDataOutOfDate()
{
	if(this->m_nOldBeginPos <= this->m_nOldEndPos){
		if((this->m_nEndPos >= this->m_nOldBeginPos) && (this->m_nEndPos < this->m_nOldEndPos)){
			return true;
		}
	}else{
		if(this->m_nEndPos > this->m_nOldBeginPos){
			return true;
		}
	}
	int nChange1 = this->m_nBeginPos - this->m_nOldBeginPos;
	if(nChange1 < 0) nChange1 += this->m_nMaxFileSize;
	int nChange2 = this->m_nEndPos - this->m_nOldEndPos;
	if(nChange2 < 0) nChange2 += this->m_nMaxFileSize;

	if((nChange1+nChange2) > (1 * 1024 * 1024)){
		return true;
	}else{
		return false;
	}
}
