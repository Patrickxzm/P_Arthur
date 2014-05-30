/***************************************************************************
                          QueenInfo.cpp  -  description
                             -------------------
    begin                : Wed Dec 31 2003
    copyright            : (C) 2003 by 
    email                : 
 ***************************************************************************/

#include <iostream>
#include "QueenData.h"
#include "LineMessage.h"

CQueenBox::CQueenBox(std::string strBoxPath, std::string strQueenName)
{
	this->m_strBoxPath = strBoxPath;
	this->m_strQueenName = strQueenName;
	this->m_nFd = -1;
	this->m_bGroup = false;
	this->m_nFree = -1;
	this->m_nLoad = -1;
	this->m_nMaxLoad = -1;
	this->m_nSiteNum = 0;
	this->m_nActiveReq = 0;
	this->m_nMessageRecv = 0;
	this->m_nMessageSent = 0;
	this->m_nImportantMsg = 0;
	this->m_pQueenPool = NULL;
	this->m_pQueenPool = new CLinePool((strBoxPath+'/'+strQueenName).c_str());
	if(this->m_pQueenPool->getCount() > 0){
		std::cout << this->m_pQueenPool->getCount() << " url messages found in queen box for "
				<< this->m_strQueenName << "." << std::endl;
	}
}

CQueenBox::~CQueenBox()
{
	this->saveMsgToPool();
	if(this->m_pQueenPool != NULL){
		delete this->m_pQueenPool;
	}
}

void CQueenBox::enableFd(int nFd, bool bGroup)
{
	this->m_nFd = nFd;
	this->m_bGroup = bGroup;
	this->deleteGroupMsg();
	this->exchangePool();
}

void CQueenBox::disableFd()
{
	this->m_nFd = -1;
	this->saveMsgToPool();
}

void CQueenBox::setLoadInfo(int nFree, int nLoad)
{
	//this->makeActiveReq();
	this->m_nFree = nFree;
	this->m_nLoad = nLoad;
	if(nLoad > this->m_nMaxLoad) this->m_nMaxLoad = nLoad;
	// Debug output {
	std::clog << '{' << this->m_strQueenName << ">load:" << nFree << ',' << nLoad << '}';
	// }
}

bool CQueenBox::isPoolOpen()
{
	return this->m_pQueenPool->isOpen();
}


void CQueenBox::addOutMessage(std::string& strOutMsg)
{
	this->m_lstrOutMsg.push_back(strOutMsg);
	this->exchangePool();
}

void CQueenBox::increaseRecvNum(bool bNew)
{
	this->m_nMessageRecv ++;
	// Debug output {
	if(bNew || (this->m_nMessageRecv % 100 == 0)){
		std::clog << '{' << this->m_strQueenName << '>' << this->m_nMessageRecv << '}';
	}
	// }
}

bool CQueenBox::sendMessage(CUltraServer* pServer, int nMaxMsgNum)
{
	if(this->m_nFd < 0) return false;
	this->makeActiveReq();
	if(this->m_lstrOutMsg.size() > 0){
		for(int i=0; (i<nMaxMsgNum) && (this->m_lstrOutMsg.size()>0); i++){
			pServer->wmsg(this->m_nFd, this->m_lstrOutMsg.front());
			if(this->m_nImportantMsg > 0){
				this->m_nImportantMsg --;
				// Debug output {
				std::clog << std::endl << "[msg]" << std::flush;
				std::cout << m_nMessageSent << '>' << this->getQueenName() << ": "
						<< this->m_lstrOutMsg.front() << std::endl;
				// }
			}
			this->m_lstrOutMsg.pop_front();
			this->m_nMessageSent ++;
		}
	}
	CLineMessage iMsg_LEFT;
	iMsg_LEFT.composeMsg_LEFT(this->m_lstrOutMsg.size()+this->m_pQueenPool->getCount());
	pServer->wmsg(this->m_nFd, iMsg_LEFT.getMsgString());
	// Debug output {
	std::clog << '{' << this->m_nMessageSent << '>' << this->m_strQueenName << '}';
	// }

	this->exchangePool();
	return true;
}

void CQueenBox::sayGoodbye(CUltraServer* pServer)
{
	if(this->m_nFd < 0) return;
	CLineMessage iMsg;
	iMsg.composeMsg_BYEBYE();
	try{
		pServer->wmsg(this->m_nFd, iMsg.getMsgString());
	}catch(...){}
}

void CQueenBox::redirectSite(const std::string& strSite, const std::string& strQueen)
{
	if(! this->m_bGroup) return;
	static std::map<std::string, int> mapSites;
	if((mapSites.find(strSite) == mapSites.end()) || (mapSites[strSite] >= 200)){
		std::list<std::string>::iterator itr = this->m_lstrOutMsg.begin();
		if((this->m_nImportantMsg > 0) && (itr != this->m_lstrOutMsg.end())){
			itr ++;
		}
		CLineMessage iMsg;
		iMsg.composeMsg_ASSIGN(strSite, strQueen);
		this->m_lstrOutMsg.insert(itr, iMsg.getMsgString());
		mapSites[strSite] = 0;
		this->m_nImportantMsg ++;
	}else{
		mapSites[strSite] ++;
	}
}


void CQueenBox::exchangePool()
{
	if(this->m_lstrOutMsg.size() > 10000){
		for(int i=0; i<500; i++){
			this->m_pQueenPool->pushALine(this->m_lstrOutMsg.back());
			this->m_lstrOutMsg.pop_back();
		}
	}else if((this->m_nFd >= 0) && (this->m_lstrOutMsg.size() < 1000)){
		for(int i=0; i<500; i++){
			if(this->m_pQueenPool->getCount() <= 0) break;
			string strMessage;
			if(this->m_pQueenPool->popALine(strMessage)){
				this->m_lstrOutMsg.push_back(strMessage);
			}
		}
	}
}

void CQueenBox::saveMsgToPool()
{
	if(! this->m_pQueenPool->isOpen()){
		std::cerr << "Message pool for queen " << this->m_strQueenName
				<< " is not open." << std::endl;
		return;
	}
	std::list<std::string>::iterator itr;
	CLineMessage iMsg;
	for(itr=this->m_lstrOutMsg.begin(); itr != this->m_lstrOutMsg.end(); itr++){
		if(this->m_nImportantMsg > 0){
			this->m_nImportantMsg --;
			continue;
		}
		this->m_pQueenPool->pushALine(*itr);
	}
	this->m_lstrOutMsg.clear();
	this->m_nImportantMsg = 0;
	this->m_pQueenPool->saveData();
	/// Output queen statistics.
	std::cout << "Messages for queen " << this->m_strQueenName << " saved, "
			<< this->m_pQueenPool->getCount() << " url messages left in queen box." << std::endl;
	if(this->m_pQueenPool->getDiscardedCount() > 0){
		std::cout << "Totally " << this->m_pQueenPool->getDiscardedCount() << " url messages for " << this->m_strQueenName
				<< " discarded because of queen box fullness or not opening." << std::endl;
	}
	std::cout << "Totally " << this->m_nMessageRecv << " url messages received from "
			<< this->m_strQueenName << "." << std::endl;
	std::cout << "Totally " << this->m_nMessageSent << " url messages sent to "
			<< this->m_strQueenName << "." << std::endl;
	std::cout << "Currently queen " << this->m_strQueenName << " owns "
			<< this->m_nSiteNum << " sites." << std::endl;
}

bool CQueenBox::isWorkFinished()
{
	if(m_nFd < 0)return true;
	if(this->m_lstrOutMsg.size() > 0)return false;
	return (m_nMaxLoad > 0) && (m_nLoad <= 0);
}

void CQueenBox::sendGroupMsg(unsigned int nSize)
{
	this->deleteGroupMsg();
	CLineMessage iGroupMsg;
	iGroupMsg.composeMsg_GROUP(nSize);
	this->m_nImportantMsg ++;
	this->m_lstrOutMsg.push_front(iGroupMsg.getMsgString());
}

void CQueenBox::deleteGroupMsg()
{
	if(this->m_lstrOutMsg.empty() || (this->m_nImportantMsg <= 0)) return;
	CLineMessage iMsg;
	if(iMsg.setMsgString(this->m_lstrOutMsg.front()) == LM_GROUP){
		this->m_lstrOutMsg.pop_front();
		this->m_nImportantMsg --;
	}
}
