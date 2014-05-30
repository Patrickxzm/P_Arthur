/***************************************************************************
                          QueenInfo.h  -  description
                             -------------------
    begin                : Wed Dec 31 2003
    copyright            : (C) 2003 by 
    email                : 
 ***************************************************************************/

#ifndef QUEENINFO_H
#define QUEENINFO_H

#include <string>
#include <list>
#include "UltraServer.h"
#include "LinePool.h"

/**
  *@author 
  */

class CQueenBox {
public:
	CQueenBox(std::string strBoxPath, std::string strQueenName);
	~CQueenBox();
	const std::string& getQueenName(){
		return m_strQueenName;
	}
	void enableFd(int nFd, bool bGroup);
	int getFd(){
		return m_nFd;
	}
	int getFreeNum(){
		if(this->m_nActiveReq == 0){
			return 0;
		}else{
			return m_nFree;
		}
	}
	void increaseSiteNum(){
		m_nSiteNum ++;
	}
	int getSiteNum(){
		return m_nSiteNum;
	}
	int getLoadNum(){
		return m_nLoad;
	}
	void decreaseFreeNum(){
		if((this->m_nFree > 1) || (this->m_nLoad > 20)){
			this->m_nFree --;
		}
		this->m_nActiveReq = 0;
	}
	void makeActiveReq(){
		this->m_nActiveReq ++;
	}
	void increaseRecvNum(bool bNew);

	void disableFd();
	void setLoadInfo(int nFree, int nLoad);

	void addOutMessage(std::string& strOutMsg);
	bool sendMessage(CUltraServer* pServer, int nMaxMsgNum = 100);
	void sayGoodbye(CUltraServer* pServer);
	void redirectSite(const std::string& strSite, const std::string& strQueen);

	bool isPoolOpen();
	bool isWorkFinished();

	void clearGroupMark(){
		m_bGroupMark = false;
	}
	void setGroupMark(){
		if(m_bGroup) m_bGroupMark = true;
	}
	bool getGroupMark(){
		return m_bGroupMark;
	}
	void sendGroupMsg(unsigned int nSize);
	void deleteGroupMsg();

protected:
	std::string m_strBoxPath;
	std::string m_strQueenName;
	std::list<std::string> m_lstrOutMsg;
	int m_nFd;
	bool m_bGroup;
	CLinePool* m_pQueenPool;

	int m_nFree;
	int m_nLoad;
	int m_nMaxLoad;
	int m_nSiteNum;
	unsigned int m_nActiveReq;
	long long m_nMessageSent;
	long long m_nMessageRecv;

	bool m_bGroupMark;

	int m_nImportantMsg;

protected:
	void exchangePool();
	void saveMsgToPool();
};

#endif
