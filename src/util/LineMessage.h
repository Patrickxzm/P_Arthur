/***************************************************************************
                          LineMessage.h  -  description
                             -------------------
    begin                : Thu Jan 1 2004
    copyright            : (C) 2004 by 
    email                : 
 ***************************************************************************/

#ifndef LINEMESSAGE_H
#define LINEMESSAGE_H

/**
  *@author HLE
  */
#include <string>
#include <vector>
  
// CLineMessage is the class encoding and decoding messages for king and queen.
// Messages are encoded into a string line, not including '\n'.

class CLineMessage
{
public: 
	CLineMessage();
	~CLineMessage();
	
	// Get message string, after composing message.
	std::string getMsgString() const{
		return m_strMsgString;
	}
	// Set message string, in order to retrieving message infomation.
	// Return message type.
	int setMsgString(const std::string& strMsg);
	// Get message type. Messages supported are listed in the folowing.
	int getMsgType() const {
		return m_nType;
	}

public:
	// Functions for composing message.
	// Return true if successful, false else.
	bool composeMsg_HELLO(const std::string& strQueenName, bool bGroup = false);
	bool composeMsg_BYEBYE();
	bool composeMsg_REFER(const std::string& strReferer,
			const std::string& strURL, const std::string& strAnchorText);
	bool composeMsg_REDIRECT(const std::string& strOriginURL,
			const std::string& strURL);
	bool composeMsg_REQUEST(int nRequestNum);
	bool composeMsg_LOAD(int nFree, int nLoad);
	bool composeMsg_EXIT();
	bool composeMsg_LEFT(int nLeft);
	bool composeMsg_ASK(const std::string& strQuestion);
	bool composeMsg_ANSWER(const std::string& strAnswer);
	bool composeMsg_GROUP(unsigned int nSize);
	bool composeMsg_ASSIGN(const std::string& strSite, const std::string& strQueen);
	
	// Functions for retrieving message.
	// Return true if successful, false else.
	bool retrieveMsg_HELLO(std::string& strQueenName) const;
	bool retrieveMsg_HELLO(std::string& strQueenName, bool& bGroup) const;
	bool retrieveMsg_REFER(std::string& strReferer,
			std::string& strURL, std::string& strAnchorText) const;
	bool retrieveMsg_REDIRECT(std::string& strOriginURL,
			std::string& strURL) const;
	bool retrieveMsg_REQUEST(int& nRequestNum) const;
	bool retrieveMsg_LOAD(int& nFree, int& nLoad) const;
	bool retrieveMsg_LEFT(int& nLeft) const;
	bool retrieveMsg_ASK(std::string& strQuestion) const;
	bool retrieveMsg_ANSWER(std::string& strAnswer) const;
	bool retrieveMsg_GROUP(unsigned int& nSize) const;
	bool retrieveMsg_ASSIGN(std::string& strSite, std::string& strQueen) const;

protected:
	int m_nType;
	std::string m_strMsgString;
	std::vector<std::string> m_lstrParamList;

    
//	std::string m_strQueenName;
//	std::string m_strReferer;
//	std::string m_strURL;
//	std::string m_strAnchorText;
//	std::string m_strMsgText;
	int m_nRequestNum;
	int m_nFree;
	int m_nLoad;
	int m_nLeft;
	int m_nData;
protected:
	int parseMsg(std::string& strMsg, std::vector<std::string>& lstrParamList);
	void composeMsg(std::string& strMsg, std::vector<std::string>& lstrParamList);
	std::string& safeURL(const std::string& strURL);
	std::string& safeText(const std::string& strText);
};

/// Message types.
const int LM_INCORRECT = -1;
const int LM_UNKNOWN = 0;
// For King.
const int LM_HELLO = 1;
const int LM_BYEBYE = 2;
const int LM_REFER = 3;
const int LM_REQUEST = 4;
const int LM_LOAD = 5;
const int LM_EXIT = 6;
const int LM_REDIRECT = 7;
// For Queen.
const int LM_LEFT = 34;
const int LM_GROUP = 35;
const int LM_ASSIGN = 36;
// Ask and answer.
const int LM_ASK = 65;
const int LM_ANSWER = 66;

#endif
