/*********************************************************************
 * @ Queen use her IP address as ID. If one queen crashed, data flown to
 *   her will be backuped by King, until admin interfere. No communication
 *   between queens.
 *					11/21/2003	16:14
 * @ <<<<<<<< 
 *   In distributed crawler system, if one queen crashed, other queens will
 *   consult King for a substitution. if King crashed, queens just wait.
 *					11/20/2003	15:49
 * @ In Arthur's distributed system, there is a King to control(manage) the
 *   Queens. Each Queen run on one distributed node, she can also collect 
 *   web pages independently as a monolithic system. To manage all the queens,
 *   the King has a table recording by which queen a host is crawled.
 *					10/20/2003   22:47  
 **********************************************************************/
#ifndef _PAT_KING_H_102003_
#define _PAT_KING_H_102003_

#include <string>
#include <map>
#include <vector>
#include <ctime>
#include "QueenData.h"
#include "UltraServer.h"
#include "LinePool.h"


class CKing
{
public:
	CKing(const char* sDataPath, int nServerPort = 8054, const char* sSeedFile = NULL);
	~CKing();
	void run();
	static void stop(int nServerPort = 8054);

private:
	std::string m_strDataPath;
	std::string m_strQueenBoxPath;
	std::string m_strRangeFile;
	std::string m_strDenyFile;
	const char* m_sSeedFile;
	int m_nServerPort;
	CUltraServer m_iServer;
	// The list of queens an their information.
	// Each queen must has a unique name as its id.
	typedef std::map<std::string, CQueenBox*> TQueenList;
	TQueenList m_liQueenBoxes;
	// The map of sites to queens.
	typedef std::map<std::string, std::string> TSite2QueenMap;
	TSite2QueenMap m_mapSite2Queen;
	// The buffer of those messages whose site not determinded yet.
	typedef std::multimap<std::string, std::string> TMessageBuf;
	TMessageBuf m_mapMessageBuf;
	CLinePool* m_pMessagePool;

	// Status variables
	bool m_bEnd; 

	// Group variables
	bool m_bGroupStart;
	time_t m_tGroupStart;

private:
//	int parseMsg(string& strMsg, int& nParamNum, vector<string>& lstrParamList);
	bool enableQueenBox(int nFd, string& strQueenName, bool bGroup);
	void disableQueenBox(int nFd);
	bool loadQuenBoxes();
	CQueenBox* getQueenBoxByFd(int nFd);
	std::string getSiteOfUrl(std::string strURL);
	void passMsgToQueen(std::string& strQueenName, std::string strOutMsg);
	int assignSitesToQueen();
	bool saveSiteMap();
	bool saveSiteMap(std::string& strQueenName, std::string strSite);
	bool loadSiteMap();
	bool loadSeeds();
	void exchangePool();
	void saveMsgToPool();

	bool makeSureDir(const char* sDirName);
	void replyAnswer(int nFd, std::string strQuestion);
	
	// Judge if work is finished.
	bool isWorkFinished();

	void startGroup();
	void checkGroup(CQueenBox* pQueenBox);

/**
	void give_ref(const string &queen);
	void give_ref(int sock);

	typedef map<string, string, less<string> > m_str2str;
	typedef map<int, string, less<int> > m_int2str;
	typedef multimap<string, string, less<string>> mm_str2str;

	m_str2str hostm2queen;
	m_int2str sockm2queen;
	mm_str2str queenm2host;
	mm_str2str hostm2ref;
**/
};


#endif // _PAT_KING_H_102003_
