/***************************************************************************
                          LineMessage.cpp  -  description
                             -------------------
    begin                : Thu Jan 1 2004
    copyright            : (C) 2004 by 
    email                : 
 ***************************************************************************/
#include "LineMessage.h"
#include <ctype.h>
using namespace std;

CLineMessage::CLineMessage()
{
	m_nType = LM_UNKNOWN;
	
	m_nRequestNum = -1;
	m_nFree = -1;
	m_nLoad = -1;
	m_nLeft = -1;
	m_nData = -1;
}

CLineMessage::~CLineMessage()
{
}

int CLineMessage::setMsgString(const std::string& strMsg)
{
	this->m_strMsgString = strMsg;
	this->m_nType = this->parseMsg(this->m_strMsgString, this->m_lstrParamList);

	switch(m_nType)
	{
	case LM_HELLO:
		break;
	case LM_BYEBYE:
		break;
	case LM_REFER:
		break;
	case LM_REDIRECT:
		break;
	case LM_REQUEST:
		if(sscanf(m_lstrParamList[1].c_str(), "%d", &this->m_nRequestNum) != 1){
			this->m_nType = LM_INCORRECT;
			break;
		}
		break;
	case LM_LOAD:
		if(sscanf(m_lstrParamList[1].c_str(), "%d", &this->m_nFree) != 1){
			this->m_nType = LM_INCORRECT;
			break;
		}
		if(sscanf(m_lstrParamList[2].c_str(), "%d", &this->m_nLoad) != 1){
			this->m_nType = LM_INCORRECT;
			break;
		}
		break;
	case LM_LEFT:
		if(sscanf(m_lstrParamList[1].c_str(), "%d", &this->m_nLeft) != 1){
			this->m_nType = LM_INCORRECT;
			break;
		}
		break;
	case LM_GROUP:
		if(sscanf(m_lstrParamList[1].c_str(), "%d", &this->m_nData) != 1){
			this->m_nType = LM_INCORRECT;
			break;
		}
		break;
	case LM_ASSIGN:
		break;
	case LM_ASK:
	case LM_ANSWER:
		break;
	}

	return this->m_nType;
}

int CLineMessage::parseMsg(std::string& strMsg, std::vector<std::string>& lstrParamList)
{
	if(strMsg.length() < 1) return LM_INCORRECT;
	if(strMsg[strMsg.length()-1] == '\r'){
		strMsg.erase(strMsg.length()-1);
	}
	lstrParamList.clear();
	int nParamNum = 0;
	
	const char sDelim = '\t';
	int nBegin = 0;
	for(unsigned int i=0; i<strMsg.length(); i++){
		if(strMsg[i] == sDelim){
			lstrParamList.push_back(strMsg.substr(nBegin, i-nBegin));
			nBegin = i + 1;
			nParamNum ++;
		}
	}
	lstrParamList.push_back(strMsg.substr(nBegin, strMsg.length()-nBegin));
	nParamNum ++;
	/// Check if error exist and return command code.
	if(lstrParamList[0] == "hello"){
		if(nParamNum == 3){
			return LM_HELLO;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "byebye"){
		return LM_BYEBYE;
	}else if(lstrParamList[0] == "exit"){
		return LM_EXIT;
	}else if(lstrParamList[0] == "ref"){
		if(nParamNum == 4){
			return LM_REFER;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "request"){
		if(nParamNum == 2){
			return LM_REQUEST;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "load"){
		if(nParamNum == 3){
			return LM_LOAD;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "redir"){
		if(nParamNum == 3){
			return LM_REDIRECT;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "left"){
		if(nParamNum == 2){
			return LM_LEFT;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "ask"){
		if(nParamNum >= 2){
			return LM_ASK;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "ans"){
		if(nParamNum >= 2){
			return LM_ANSWER;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "group"){
		if(nParamNum == 2){
			return LM_GROUP;
		}else{
			return LM_INCORRECT;
		}
	}else if(lstrParamList[0] == "assign"){
		if(nParamNum == 3){
			return LM_ASSIGN;
		}else{
			return LM_INCORRECT;
		}
	}
	
	return LM_UNKNOWN;
}

void CLineMessage::composeMsg(std::string& strMsg, std::vector<std::string>& lstrParamList)
{
	strMsg.clear();
	int nNum = lstrParamList.size();
	for(int i=0; i<nNum; i++){
		if(i > 0) strMsg += '\t';
		strMsg += lstrParamList[i];
	}
}

std::string& CLineMessage::safeURL(const std::string& strURL)
{
	static std::string strTmpURL;
	strTmpURL = strURL;
	for(int i=strTmpURL.length()-1; i>=0; i--){
		if((strTmpURL[i] == '\t') || (strTmpURL[i] == '\r') || (strTmpURL[i] == '\n')) strTmpURL.erase(i);
	}
	return strTmpURL;
}

std::string& CLineMessage::safeText(const std::string& strText)
{
	static std::string strTmpText;
	strTmpText = strText;
	for(int i=strTmpText.length()-1; i>=0; i--){
		if((strTmpText[i] == '\t') || (strTmpText[i] == '\r') || (strTmpText[i] == '\n')) strTmpText[i] = ' ';
	}
	return strTmpText;
}

/// The following for compose message.
bool CLineMessage::composeMsg_HELLO(const std::string& strQueenName, bool bGroup)
{
	if(strQueenName.length() < 1) return false;
	// Queen name must be composed by alphabetic and number characters.
	unsigned int nLen = strQueenName.length();
	for(unsigned int i=0; i<nLen; i++){
		if(! (isalnum(strQueenName[i]) || (strQueenName[i]=='_'))) return false;
	}

	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("hello");
	this->m_lstrParamList.push_back(strQueenName);
	if(bGroup){
		this->m_lstrParamList.push_back("true");
	}else{
		this->m_lstrParamList.push_back("false");
	}
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_HELLO;
	return true;
}

bool CLineMessage::composeMsg_BYEBYE()
{
	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("byebye");
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_BYEBYE;
	return true;
}

bool CLineMessage::composeMsg_EXIT()
{
	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("exit");
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_EXIT;
	return true;
}

bool CLineMessage::composeMsg_REFER(const std::string& strReferer,
		const std::string& strURL, const std::string& strAnchorText)
{
	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("ref");
	this->m_lstrParamList.push_back(this->safeURL(strReferer));
	this->m_lstrParamList.push_back(this->safeURL(strURL));
	this->m_lstrParamList.push_back(this->safeText(strAnchorText));
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_REFER;
	return true;
}

bool CLineMessage::composeMsg_REDIRECT(const std::string& strOriginURL,
		const std::string& strURL)
{
	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("redir");
	this->m_lstrParamList.push_back(this->safeURL(strOriginURL));
	this->m_lstrParamList.push_back(this->safeURL(strURL));
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_REDIRECT;
	return true;
}

bool CLineMessage::composeMsg_REQUEST(int nRequestNum)
{
	static char sTmp[64];
	this->m_lstrParamList.clear();
	sprintf(sTmp, "%d", nRequestNum);
	this->m_lstrParamList.push_back("request");
	this->m_lstrParamList.push_back(sTmp);
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_REQUEST;
	return true;
}

bool CLineMessage::composeMsg_LOAD(int nFree, int nLoad)
{
	static char sTmp[64];
	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("load");
	sprintf(sTmp, "%d", nFree);
	this->m_lstrParamList.push_back(sTmp);
	sprintf(sTmp, "%d", nLoad);
	this->m_lstrParamList.push_back(sTmp);
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_LOAD;
	return true;
}

bool CLineMessage::composeMsg_LEFT(int nLeft)
{
	static char sTmp[64];
	this->m_lstrParamList.clear();
	sprintf(sTmp, "%d", nLeft);
	this->m_lstrParamList.push_back("left");
	this->m_lstrParamList.push_back(sTmp);
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_LEFT;
	return true;
}

bool CLineMessage::composeMsg_ASK(const std::string& strQuestion)
{
	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("ask");
	this->m_lstrParamList.push_back(strQuestion);
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_ASK;
	return true;
}

bool CLineMessage::composeMsg_ANSWER(const std::string& strAnswer)
{
	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("ans");
	this->m_lstrParamList.push_back(strAnswer);
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_ANSWER;
	return true;
}

bool CLineMessage::composeMsg_GROUP(unsigned int nSize)
{
	static char sTmp[64];
	this->m_lstrParamList.clear();
	sprintf(sTmp, "%d", nSize);
	this->m_lstrParamList.push_back("group");
	this->m_lstrParamList.push_back(sTmp);
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_LEFT;
	return true;
}

bool CLineMessage::composeMsg_ASSIGN(const std::string& strSite, const std::string& strQueen)
{
	this->m_lstrParamList.clear();
	this->m_lstrParamList.push_back("assign");
	this->m_lstrParamList.push_back(this->safeText(strSite));
	this->m_lstrParamList.push_back(this->safeText(strQueen));
	this->composeMsg(this->m_strMsgString, this->m_lstrParamList);
	this->m_nType = LM_ASSIGN;
	return true;
}

/// The following for retrieve message.
bool CLineMessage::retrieveMsg_HELLO(std::string& strQueenName) const
{
	if(this->m_nType != LM_HELLO) return false;
	strQueenName = this->m_lstrParamList[1];
	return true;
}

bool CLineMessage::retrieveMsg_HELLO(std::string& strQueenName, bool& bGroup) const
{
	if(this->m_nType != LM_HELLO) return false;
	strQueenName = this->m_lstrParamList[1];
	if(this->m_lstrParamList[2] == "true"){
		bGroup = true;
	}else{
		bGroup = false;
	}
	return true;
}

bool CLineMessage::retrieveMsg_REFER(std::string& strReferer,
		std::string& strURL, std::string& strAnchorText) const
{
	if(this->m_nType != LM_REFER) return false;
	strReferer = this->m_lstrParamList[1];
	strURL = this->m_lstrParamList[2];
	strAnchorText = this->m_lstrParamList[3];
	return true;
}

bool CLineMessage::retrieveMsg_REDIRECT(std::string& strOriginURL,
		std::string& strURL) const
{
	if(this->m_nType != LM_REDIRECT) return false;
	strOriginURL = this->m_lstrParamList[1];
	strURL = this->m_lstrParamList[2];
	return true;
}

bool CLineMessage::retrieveMsg_REQUEST(int& nRequestNum) const
{
	if(this->m_nType != LM_REQUEST) return false;
	nRequestNum = this->m_nRequestNum;
	return true;
}

bool CLineMessage::retrieveMsg_LOAD(int& nFree, int& nLoad) const
{
	if(this->m_nType != LM_LOAD) return false;
	nFree = this->m_nFree;
	nLoad = this->m_nLoad;
	return true;
}

bool CLineMessage::retrieveMsg_LEFT(int& nLeft) const
{
	if(this->m_nType != LM_LEFT) return false;
	nLeft = this->m_nLeft;
	return true;
}

bool CLineMessage::retrieveMsg_ASK(std::string& strQuestion) const
{
	if(this->m_nType != LM_ASK) return false;
	strQuestion = this->m_lstrParamList[1];
	return true;
}

bool CLineMessage::retrieveMsg_ANSWER(std::string& strAnswer) const
{
	if(this->m_nType != LM_ANSWER) return false;
	strAnswer = this->m_lstrParamList[1];
	return true;
}

bool CLineMessage::retrieveMsg_GROUP(unsigned int& nSize) const
{
	if(this->m_nType != LM_GROUP) return false;
	nSize = this->m_nData;
	return true;
}

bool CLineMessage::retrieveMsg_ASSIGN(std::string& strSite, std::string& strQueen) const
{
	if(this->m_nType != LM_ASSIGN) return false;
	strSite = this->m_lstrParamList[1];
	strQueen = this->m_lstrParamList[2];
	return true;
}

