/***********************************************************************
 * @ give_ref() will trave through two multimap, it's time-consuming. Use 
 *   it carefully.			11/23/2003	23:25
 * @ The great King lives in a town, queens come here exchanging data.
 *					11/23/2003	22:44
 ***********************************************************************/
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include "king.h"
#include "url.h"
#include "LineMessage.h"
#include "excep.h"

using namespace std;
int xzm = 10;
const int GROUP_CHECK_TIME = 90;
void SigINT_Handler(int sig);
void SigChld_Handler(int sig);

void usage(){
	std::cout << "usage: king -d data_path [-p port(default:8054)] "
		"[-s seed_file(default:seeds)]" << std::endl;
	std::cout << "       king stop [port(default:8054)]" << std::endl;
}

int main(int argc, char* argv[])
{
	char* sDataPath = "/tmp/king";
	char* sSeedFile = NULL;
	int nServerPort = 8054;
	if((argc > 1) && (strcmp(argv[1], "stop") == 0)){
		cout << "Try to stop king ... ";
		if(argc > 2){
			if(sscanf(argv[2], "%d", &nServerPort) != 1){
				std::cout << "invalid parameter: port must be an integer." << std::endl;
				return -1;
			}
		}
		CKing::stop(nServerPort);
		return 0;
	}
	while((argc > 1) && (argv[1][0] == '-')){
		if((strcmp(argv[1], "-d") == 0) && (argc > 2)){
			sDataPath = argv[2];
			argc -= 2;
			argv += 2;
		}else if((strcmp(argv[1], "-p") == 0) && (argc > 2)){
			if(sscanf(argv[2], "%d", &nServerPort) != 1){
				std::cout << "invalid parameter: port must be an integer." << std::endl;
				return -1;
			}
			argc -= 2;
			argv += 2;
		}else if((strcmp(argv[1], "-s") == 0) && (argc > 2)){
			sSeedFile = argv[2];
			argc -= 2;
			argv += 2;
		}else if((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)){
			usage();
			return 0;
		}else{
			std::cout << "Unsupported option \'" << argv[1] << "\'." << std::endl;
			return -1;
		}
	}
	//signal(SIGTERM, SigINT_Handler);
	signal(SIGINT, SigINT_Handler);
	try{
		CKing iKing(sDataPath, nServerPort, sSeedFile);
		iKing.run();
	}catch(excep& e){
		std::cout << "Exception: " << e.msg << std::endl;
	}catch(...){
		std::cout << "Exception catched while creating king. king exit." << std::endl;
	}
	return 0;
}

void SigINT_Handler(int sig)
{
	std::cout << "Signal INT received." <<xzm<< std::endl;
	std::cout << "Please send exit command to king instead." << std::endl;
}

CKing::CKing(const char* sDataPath, int nServerPort, const char* sSeedFile)
: m_iServer(nServerPort)
{
	this->m_bEnd = false;
	this->m_strDataPath = sDataPath;
	this->m_nServerPort = nServerPort;
	this->m_sSeedFile = sSeedFile;
	if(this->m_strDataPath[this->m_strDataPath.length()-1] == '/'){
		this->m_strDataPath.erase(this->m_strDataPath.length()-1, 1);
	}
	if(! this->makeSureDir(this->m_strDataPath.c_str())){
		std::cout << "Fatal error: can't open dir \""
				<< this->m_strDataPath << "\"." << std::endl;
		this->m_bEnd = true;
	}
	this->m_strQueenBoxPath = this->m_strDataPath + "/queenbox";
	if(! this->makeSureDir(this->m_strQueenBoxPath.c_str())){
		std::cout << "Fatal error: can't open dir \""
				<< this->m_strQueenBoxPath << "\"." << std::endl;
		this->m_bEnd = true;
	}
	/*
	/// If range file exist, load it.
	this->m_pFilter_Allow = NULL;
	this->m_pFilter_Deny = NULL;
	this->m_strRangeFile = this->m_strDataPath + "/range";
	struct stat iStatBuf;
	if((stat(this->m_strRangeFile.c_str(), &iStatBuf) == 0)
			&& (S_ISREG(iStatBuf.st_mode))){
		this->m_pFilter_Allow = new CHostFilter();
		if(this->m_pFilter_Allow->init(this->m_strRangeFile.c_str()) == -1){
			std::cout << "Fatal error: can't open range file \""
					<< this->m_strRangeFile << "\"." << std::endl;
			this->m_bEnd = true;
		}
		std::cout << "range file loaded." << std::endl;
	}
	/// if deny file exist, load it.
	this->m_strDenyFile = this->m_strDataPath + "/deny";
	if((stat(this->m_strDenyFile.c_str(), &iStatBuf) == 0)
			&& (S_ISREG(iStatBuf.st_mode))){
		this->m_pFilter_Deny = new CHostFilter();
		if(this->m_pFilter_Deny->init(this->m_strRangeFile.c_str()) == -1){
			std::cout << "Fatal error: can't open deny file \""
					<< this->m_strRangeFile << "\"." << std::endl;
			this->m_bEnd = true;
		}
		std::cout << "deny file loaded." << std::endl;
	}
	*/
	/// Open message pool file.
	this->m_pMessagePool = NULL;
	this->m_pMessagePool = new CLinePool((this->m_strDataPath+"/message_pool").c_str());
	if(this->m_pMessagePool->getCount() > 0){
		std::cout << this->m_pMessagePool->getCount() << " url messages found in message pool." << std::endl;
	}
	this->exchangePool();
}

CKing::~CKing()
{
	TQueenList::iterator itr;
	for(itr=this->m_liQueenBoxes.begin(); itr!=this->m_liQueenBoxes.end(); itr++){
		itr->second->sayGoodbye(&this->m_iServer);
		delete itr->second;
	}
	this->saveMsgToPool();
	if(this->m_pMessagePool != NULL){
		delete m_pMessagePool;
	}
}

void CKing::run()
{
	if(m_bEnd) return;
	if(! this->loadQuenBoxes()) return;
	if(! this->loadSiteMap()) return;
	if(! this->loadSeeds()) return;

	string* msg;
	int nFd, nLastFd=0;
	string strSite, strOutMsg;
	CQueenBox* pQueenBox = NULL;
	CLineMessage iQueenMsg;
	string strQueenName, strReferer, strURL, strAnchorText, strQuestion;
	int nRequestNum, nFree, nLoad;
	bool bGroup = false;
	std::cout << "king ready." << std::endl;
	while (true)
	{
		try{
			msg = this->m_iServer.rmsg(nFd);
		}catch(excep& e){
			std::cout << "Exception: " << e.msg << std::endl;
			continue;
		}catch(...){
			std::cout << "Exception catched while reading message." << std::endl;
			break;
		}
		if (msg == NULL){
			std::cout << "NULL message received. Exit." << std::endl;
			break;
		}
		// Safety check{
		try{
		// }
		string& strMsg = *msg;

		int nCmd = iQueenMsg.setMsgString(strMsg);
		// Debug output {
		if((nCmd != LM_REQUEST) && (nCmd != LM_REFER) && (nCmd != LM_REDIRECT) && (nCmd != LM_LOAD)){
			std::clog << "{rcv: " << iQueenMsg.getMsgString() << '}';
		}
		// }
		switch(nCmd)
		{
		case LM_HELLO:
			iQueenMsg.retrieveMsg_HELLO(strQueenName, bGroup);
			this->enableQueenBox(nFd, strQueenName, bGroup);
			// Debug output {
			std::clog << std::endl << "[msg]" << std::flush;
			std::cout << "queen " << strQueenName << " join." << std::endl;
			// }
		case LM_GROUP:
			this->startGroup();
			break;
		case LM_BYEBYE:
			this->disableQueenBox(nFd);
			this->startGroup();
			break;
		case LM_REDIRECT:
			iQueenMsg.retrieveMsg_REDIRECT(strReferer, strURL);
		case LM_REFER:
			if(nCmd == LM_REFER) iQueenMsg.retrieveMsg_REFER(strReferer, strURL, strAnchorText);
			/// Counting.
			pQueenBox = this->getQueenBoxByFd(nFd);
			if(pQueenBox == NULL){
				std::cout << "Error: queen not registered. send hello first." << std::endl;
				break;
			}
			if(this->m_bGroupStart)this->checkGroup(pQueenBox);
			pQueenBox->increaseRecvNum(nFd != nLastFd);
			nLastFd = nFd;
			/// Forward message.
			strSite = this->getSiteOfUrl(strURL);
			if(strSite.empty()){
				cerr << "Warning: find a null site. URL: " << strURL << endl;
				break;
			}
			if(this->m_mapSite2Queen.find(strSite) == this->m_mapSite2Queen.end()){
				// save it to site message buffer.
				this->m_mapMessageBuf.insert(make_pair(strSite, strMsg));
				this->exchangePool();
			}else{
				this->passMsgToQueen(this->m_mapSite2Queen[strSite], strMsg);
				pQueenBox->redirectSite(strSite, this->m_mapSite2Queen[strSite]);
			}
			break;
		case LM_REQUEST:
			iQueenMsg.retrieveMsg_REQUEST(nRequestNum);
			if(nRequestNum <= 0) nRequestNum = 1;
			pQueenBox = this->getQueenBoxByFd(nFd);
			if(pQueenBox == NULL){
				std::cout << "Error: queen not registered. send hello first." << std::endl;
				break;
			}
			if(this->m_bGroupStart)this->checkGroup(pQueenBox);
			pQueenBox->sendMessage(&this->m_iServer, nRequestNum);
			assignSitesToQueen();
			if(this->isWorkFinished()){
				this->m_bEnd = true;
				std::cout << "Work finished!" << std::endl;
			}
			break;
		case LM_LOAD:
			iQueenMsg.retrieveMsg_LOAD(nFree, nLoad);
			pQueenBox = this->getQueenBoxByFd(nFd);
			if(pQueenBox == NULL){
				std::cout << "Error: queen not registered. send hello first." << std::endl;
			}else{
				pQueenBox->setLoadInfo(nFree, nLoad);
				if(this->m_bGroupStart)this->checkGroup(pQueenBox);
			}
			assignSitesToQueen();
			break;
		case LM_ASK:
			iQueenMsg.retrieveMsg_ASK(strQuestion);
			replyAnswer(nFd, strQuestion);
			break;
		case LM_UNKNOWN:
			std::cout << "Warning: unknown command received. msg: " << strMsg << std::endl;
			break;
		case LM_INCORRECT:
			std::cout << "Error: incorrect command received. msg: " << strMsg << std::endl;
			break;
		case LM_EXIT:
			m_bEnd = true;
			std::cout << "Exit command received." << std::endl;
			break;
		}

		// Safety check{
		}catch(excep& e){
			std::cout << "Exception: " << e.msg << std::endl;
		}catch(...){
			std::cout << "Exception catched while processing message." << std::endl;
		}
		// }
		delete msg;
		if(m_bEnd) break;
	}
	std::cout << "king exit." << std::endl;
}

bool CKing::isWorkFinished()
{
	if(this->m_mapMessageBuf.size() > 0) return false;
	TQueenList::iterator itr;
	for(itr=this->m_liQueenBoxes.begin(); itr!=this->m_liQueenBoxes.end(); itr++){
		if(! itr->second->isWorkFinished()) return false;
	}
	return true;
}

void CKing::stop(int nServerPort)
{
        /// Init socket.
        int nFd = socket(AF_INET, SOCK_STREAM, 0);
        if(nFd < 0){
                cout << "Fail to create socket." << endl;
                return;
        }
        struct sockaddr_in iSockAddr;
        bzero(&iSockAddr, sizeof(iSockAddr));
        iSockAddr.sin_family = AF_INET;
        inet_pton(AF_INET, "127.0.0.1", &iSockAddr.sin_addr);
        iSockAddr.sin_port = htons(nServerPort);
        /// Connect to server.
        if(connect(nFd, (sockaddr*)&iSockAddr, sizeof(iSockAddr)) != 0){
                cout << "king not runing." << endl;
                return;
        }
	/// Send "exit" to server
	int nBytes = write(nFd, "exit\n", 5);
	if(nBytes != 5){
		cout << "Fail to send message." << endl;
	}
	close(nFd);
	cout << "ok" << endl;
}

bool CKing::loadQuenBoxes()
{
	if(! this->makeSureDir(this->m_strQueenBoxPath.c_str())){
		cout << "Error: can't open dir \"" << this->m_strQueenBoxPath << "\"." << endl;
		return false;
	}
	DIR* pDir = opendir(this->m_strQueenBoxPath.c_str());
	if(pDir == NULL){
		std::cout << "Error: can't open directory \"" << this->m_strQueenBoxPath << "\"" << std::endl;
		return false;
	}
	dirent* pDirent;
	while((pDirent = readdir(pDir)) != NULL){
		if((strcmp(pDirent->d_name, ".") == 0)
				|| (strcmp(pDirent->d_name, "..") == 0)){
			continue;
		}
		string strQueenName = pDirent->d_name;

		bool bQueenValid = true;
		for(unsigned int i=0; i<strQueenName.length(); i++){
			if(! isalnum(strQueenName[i])){
				std::cout << "Warning: queen name \"" << strQueenName
						<< "\" is invalid. must be alphabetic or number characters." << std::endl;
				bQueenValid = false;
				break;
			}
		}
		if(! bQueenValid) continue;

		if(this->m_liQueenBoxes.find(strQueenName) == this->m_liQueenBoxes.end()){
			this->m_liQueenBoxes[strQueenName] = new CQueenBox(this->m_strQueenBoxPath, strQueenName);
		}
		if(! this->m_liQueenBoxes[strQueenName]->isPoolOpen()) return false;
	}
	return true;
}

bool CKing::enableQueenBox(int nFd, string& strQueenName, bool bGroup)
{
	// if not exist, create it.
	if(this->m_liQueenBoxes.find(strQueenName) == this->m_liQueenBoxes.end()){
		this->m_liQueenBoxes[strQueenName] = new CQueenBox(this->m_strQueenBoxPath, strQueenName);
	}
	this->m_liQueenBoxes[strQueenName]->enableFd(nFd, bGroup);
	return true;
}

void CKing::disableQueenBox(int nFd)
{
	TQueenList::iterator itr;
	for(itr=this->m_liQueenBoxes.begin(); itr!=this->m_liQueenBoxes.end(); itr++){
		if(itr->second->getFd() == nFd){
			itr->second->disableFd();
			// Debug output {
			std::clog << std::endl << "[msg]" << std::flush;
			std::cout << "queen " << itr->second->getQueenName() << " leave." << std::endl;
			// }
			break;
		}
	}
}

CQueenBox* CKing::getQueenBoxByFd(int nFd)
{
	TQueenList::iterator itr;
	for(itr=this->m_liQueenBoxes.begin(); itr!=this->m_liQueenBoxes.end(); itr++){
		if(itr->second->getFd() == nFd){
			return itr->second;
		}
	}
	return NULL;
}

std::string CKing::getSiteOfUrl(std::string strURL)
{
	CURL iURL(strURL);
	return iURL.site();
}

void CKing::passMsgToQueen(std::string& strQueenName, std::string strOutMsg)
{
	CQueenBox* pQueenBox = NULL;
	if(this->m_liQueenBoxes.find(strQueenName) == this->m_liQueenBoxes.end()){
		pQueenBox = new CQueenBox(this->m_strQueenBoxPath, strQueenName);
		this->m_liQueenBoxes[strQueenName] = pQueenBox;
		if(! this->saveSiteMap(strQueenName, pQueenBox->getQueenName())){
			cout << "Error: can't save sitemap." << std::endl;
		}
	}else{
		pQueenBox = this->m_liQueenBoxes[strQueenName];
	}
	pQueenBox->addOutMessage(strOutMsg);
}

// Assign 1 or 0 site to a queen.
int CKing::assignSitesToQueen()
{
	if(this->m_mapMessageBuf.size() < 1) return 0;
	
	TMessageBuf::iterator itr;
	itr = this->m_mapMessageBuf.begin();
	string strSite = itr->first;
	string strQueenName;
	CQueenBox* pQueenBox = NULL;
	int nFreeMax = 0; int nSiteNum = 0;
	// Check if this site is already assigned.
	if(this->m_mapSite2Queen.find(strSite) == this->m_mapSite2Queen.end()){
		// Not assigned. Set this site to be assigned.
		TQueenList::iterator itrQ;
		for(itrQ=this->m_liQueenBoxes.begin(); itrQ!=this->m_liQueenBoxes.end(); itrQ++){
			if((itrQ->second->getFreeNum() > nFreeMax)
					|| ((itrQ->second->getFreeNum() == nFreeMax) && (itrQ->second->getSiteNum() < nSiteNum))){
				pQueenBox = itrQ->second;
				nFreeMax = pQueenBox->getFreeNum();
				nSiteNum = pQueenBox->getSiteNum();
			}
		}
		if((pQueenBox == NULL) || (nFreeMax <= 0)) return 0;
		strQueenName = pQueenBox->getQueenName();
		this->m_mapSite2Queen[strSite] = strQueenName;
		this->saveSiteMap(strQueenName, strSite);
		pQueenBox->increaseSiteNum();
		pQueenBox->decreaseFreeNum();
		// Debug output {
		std::clog << std::endl << "[msg]" << std::flush;
		std::cout << "assign site " << strSite << " to queen " << strQueenName << std::endl;
		// }
	}else{
		// Assigned. Get the QueenBox. 
		strQueenName = this->m_mapSite2Queen[strSite];
		if(this->m_liQueenBoxes.find(strQueenName) == this->m_liQueenBoxes.end()){
			std::cerr << "Error: queen " << strQueenName << " not found." << std::endl;
			return 0;
		}
		pQueenBox = this->m_liQueenBoxes[strQueenName];
	}
	// Process all messages of the same site.
	while(itr != this->m_mapMessageBuf.end()){
		if(itr->first != strSite) break;
		pQueenBox->addOutMessage(itr->second);
		this->m_mapMessageBuf.erase(itr++);
	}
	this->exchangePool();
	return 1;
}

bool CKing::saveSiteMap()
{
/*	string strSiteMapPath = this->m_strDataPath + "/sitemap";
	map<string, ofstream*> mapSiteFiles;
	TSite2QueenMap::iterator itr;
	for(itr=this->m_mapSite2Queen.begin(); itr!=this->m_mapSite2Queen.end(); itr++){

	}
	//ofstream* pFile =
*/
	return false;
}

bool CKing::saveSiteMap(std::string& strQueenName, std::string strSite)
{
	string strSiteMapPath = this->m_strDataPath + "/sitemap";
	if(! this->makeSureDir(strSiteMapPath.c_str())){
		cout << "Error: can't open dir \"" << strSiteMapPath << "\"." << endl;
		return false;
	}
	string strSiteMapFile = strSiteMapPath + '/' + strQueenName;
	ofstream ofsQueenSite(strSiteMapFile.c_str(), std::ios::binary | std::ios::out | std::ios::app);
	if(! ofsQueenSite){
		cout << "Error: can't open file \"" << strSiteMapFile << "\"." << endl;
		return false;
	}
	ofsQueenSite << strSite << '\n';
	ofsQueenSite.close();
	return true;
}


bool CKing::loadSiteMap()
{
	string strSiteMapPath = this->m_strDataPath + "/sitemap";
	if(! this->makeSureDir(strSiteMapPath.c_str())){
		cout << "Error: can't open dir \"" << strSiteMapPath << "\"." << endl;
		return false;
	}
	DIR* pDir = opendir(strSiteMapPath.c_str());
	if(pDir == NULL){
		std::cout << "Error: can't open directory \"" << strSiteMapPath << "\"" << std::endl;
		return false;
	}
	dirent* pDirent;
	while((pDirent = readdir(pDir)) != NULL){
		if((strcmp(pDirent->d_name, ".") == 0)
				|| (strcmp(pDirent->d_name, "..") == 0)){
			continue;
		}
		string strQueenName = pDirent->d_name;
		string strFileName = strSiteMapPath + '/' + strQueenName;
		ifstream ifsQueenSite(strFileName.c_str(), std::ios::binary  | std::ios::in | std::ios::out);
		if(! ifsQueenSite){
			std::cout << "Error: can't open file \"" << strFileName << "\"" << std::endl;
			return false;
		}
		while(! ifsQueenSite.eof()){
			static char sBuf[4096];
			ifsQueenSite.getline(sBuf, 4095);
			if(ifsQueenSite.fail()) break;
			string strSite = sBuf;
			this->m_mapSite2Queen.insert(make_pair(strSite, strQueenName));
			/// increase sitenum.
			CQueenBox* pQueenBox = NULL;
			if(this->m_liQueenBoxes.find(strQueenName) == this->m_liQueenBoxes.end()){
				pQueenBox = new CQueenBox(this->m_strQueenBoxPath, strQueenName);
				this->m_liQueenBoxes[strQueenName] = pQueenBox;
			}else{
				pQueenBox = this->m_liQueenBoxes[strQueenName];
			}
			pQueenBox->increaseSiteNum();
		}
	}
	return true;
}

bool CKing::loadSeeds()
{
	if(this->m_sSeedFile == NULL) return true;
	ifstream ifsSeedFile(this->m_sSeedFile, std::ios::binary  | std::ios::in | std::ios::out);
	if(! ifsSeedFile){
		std::cout << "Error: can't open seed file \"" << this->m_sSeedFile << "\"" << std::endl;
		return false;
	}
	bool bSeedsFound = false;
	while(! ifsSeedFile.eof()){
		static char sBuf[4096];
		ifsSeedFile.getline(sBuf, 4095);
		if(ifsSeedFile.fail()) break;
		string strURL = sBuf;
		CLineMessage iOutMsg;
		iOutMsg.composeMsg_REFER("", strURL, "");
		string strSite = this->getSiteOfUrl(strURL);
		if(strSite.empty()){
			cerr << "Warning: find a null site. URL: " << strURL << endl;
		}else{
			bSeedsFound = true;
			if(this->m_mapSite2Queen.find(strSite) == this->m_mapSite2Queen.end()){
				this->m_mapMessageBuf.insert(make_pair(strSite, iOutMsg.getMsgString()));
			}else{
				this->passMsgToQueen(this->m_mapSite2Queen[strSite], iOutMsg.getMsgString());
			}
		}
	}
	if(! bSeedsFound){
		std::cout << "Error: seeds file contains no seeds." << std::endl;
		return false;
	}
	std::cout << "seeds file loaded." << std::endl;
	return true;
}

void CKing::exchangePool()
{
	if(this->m_mapMessageBuf.size() > 200000){
		TMessageBuf::iterator itr;
		itr = this->m_mapMessageBuf.begin();
		for(int i=0; i<500; i++){
			this->m_pMessagePool->pushALine(itr->second);
			this->m_mapMessageBuf.erase(itr++);
		}
	}else if(this->m_mapMessageBuf.size() < 1000){
		string strSite, strMsg;
		static CLineMessage iMsg;
		static string strReferer, strURL, strAnchorText;
		for(int i=0; i<500; i++){
			if(this->m_pMessagePool->getCount() <= 0) break;
			if(this->m_pMessagePool->popALine(strMsg)){
				iMsg.setMsgString(strMsg);
				if((iMsg.getMsgType() == LM_REFER) || (iMsg.getMsgType() == LM_REDIRECT)){
					if(iMsg.getMsgType() == LM_REFER){
						iMsg.retrieveMsg_REFER(strReferer, strURL, strAnchorText);
					}else{
						iMsg.retrieveMsg_REDIRECT(strReferer, strURL);
					}
					strSite = this->getSiteOfUrl(strURL);
					if(strSite.empty()){
						cerr << "Warning: find a null site. URL: " << strURL << endl;
					}else{
						if(this->m_mapSite2Queen.find(strSite) == this->m_mapSite2Queen.end()){
							this->m_mapMessageBuf.insert(make_pair(strSite, strMsg));
						}else{
							this->passMsgToQueen(this->m_mapSite2Queen[strSite], strMsg);
						}
					}
				}else{
					cerr << "Warning: find an incorrect URL msg. Msg: " << strMsg << endl;
					break;
				}
			}
		}
	}
}

void CKing::saveMsgToPool()
{
	if(! this->m_pMessagePool->isOpen()){
		std::cerr << "Message pool is not open." << std::endl;
		return;
	}
	TMessageBuf::iterator itr;
	for(itr=this->m_mapMessageBuf.begin(); itr != this->m_mapMessageBuf.end(); itr++){
		this->m_pMessagePool->pushALine(itr->second);
	}
	this->m_mapMessageBuf.clear();
	this->m_pMessagePool->saveData();
	std::cout << "Site-undeterminded messages saved, "
			<< this->m_pMessagePool->getCount() << " url messages left in messag pool." << std::endl;
	if(this->m_pMessagePool->getDiscardedCount() > 0){
		std::cout << "Totally " << this->m_pMessagePool->getDiscardedCount() << " url messages "
				<< " discarded because of queen box fullness or not opening." << std::endl;
	}
}

void CKing::replyAnswer(int nFd, std::string strQuestion)
{
	CLineMessage iAnsMsg;
	iAnsMsg.composeMsg_ANSWER("Sorry, no answer.");
	this->m_iServer.wmsg(nFd, iAnsMsg.getMsgString());
}

bool CKing::makeSureDir(const char* sDirName)
{
	std::string strDir = sDirName;
	if(strDir[strDir.length()-1] == '/'){
		strDir.erase(strDir.length()-1, 1);
	}
	int nPos = strDir.rfind('/');
	if( nPos > 0){
		std::string strSupDir = strDir.substr(0, nPos);
		if(! makeSureDir(strSupDir.c_str())){
			return false;
		}
	}
	DIR* pTDir = opendir(sDirName);
	if(pTDir == NULL){
		if(mkdir(sDirName, 0755) != 0){
			return false;
		}
	}else{
		closedir(pTDir);
	}
	return true;
}

void CKing::startGroup()
{
	this->m_bGroupStart = true;
	TQueenList::iterator itr;
	for(itr=this->m_liQueenBoxes.begin(); itr!=this->m_liQueenBoxes.end(); itr++){
		itr->second->clearGroupMark();
	}
	time(&this->m_tGroupStart);
}

void CKing::checkGroup(CQueenBox* pQueenBox)
{
	if(! this->m_bGroupStart) return;
	pQueenBox->setGroupMark();
	time_t tNow;
	time(&tNow);
	if(tNow > this->m_tGroupStart + GROUP_CHECK_TIME){
		this->m_bGroupStart = false;
		unsigned int nSize = 0;
		TQueenList::iterator itr;
		for(itr=this->m_liQueenBoxes.begin(); itr!=this->m_liQueenBoxes.end(); itr++){
			if(itr->second->getGroupMark()) nSize ++;
		}
		if(nSize > 1){
			// Debug output {
			std::clog << std::endl << "[msg]" << std::flush;
			std::cout << "start group " << nSize << ":";
			// }
			for(itr=this->m_liQueenBoxes.begin(); itr!=this->m_liQueenBoxes.end(); itr++){
				if(itr->second->getGroupMark()){
					itr->second->sendGroupMsg(nSize);
					// Debug output {
					std::cout << ' ' << itr->second->getQueenName();
					// }
				}else{
					itr->second->deleteGroupMsg();
				}
			}
			// Debug output {
			std::cout << std::endl;
			// }
		}
	}
}

