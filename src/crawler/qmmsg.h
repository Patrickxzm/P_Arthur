/***************************************************************************
 * @ The BYEBYE message is changed: host & port ====> pid.
 * @ QM_MISS & QM_DELAY is merged into QM_BYBYE. 
 * 						06/29/2004	11:23
 * @ Queens use CQMMsg to hello each other.	04/08/2004	15:34
 * @ Messages between Queen and Mosquito, similar to HLE's CLineMessage, which
 *   is messages between King and Queen.
 *						02/17/2004	16:13
 ***************************************************************************/
#ifndef _PAT_Q_M_MSG_021704
#define _PAT_Q_M_MSG_021704
#include <string>
#include <hostports.h>
using std::string;

enum QM_MsgType{
	QM_BYEBYE=0, QM_REFER=1, QM_REDIRECT=2
	, QM_BAD=4, QQ_HELLO=5, QM_STICK=6
};

using std::string;
class CQMMsg  
{
public:
	// Get message string, after composing message.
	const string& getMsg() const{
		return __Msg;
	}
        // Set message string, in order to retrieving message infomation.
        // Return message type.
        QM_MsgType setMsg(const string& strMsg);
        // Get message type. Messages supported are listed in the folowing.
        QM_MsgType getType() const{
                return __Type;
        }
public:
	// method for composing a message.
	bool compose_REFER(const string &urlfrom, const string &urlto
		, const string &anchor);
	bool compose_REDIRECT(const string &urlfrom, const string &urlto);
	bool compose_BYEBYE(const pid_t &pid, hp_status status, time_t retry_time);
	bool compose_HELLO(const string &myname);
	bool compose_STICK(const string &ip, int port, pid_t pid);

public:
	// method for taking infomation in message out accoring to msg type.
	bool retrieve_REFER(string &urlfrom, string &urlto, string &anchor) const;
	bool retrieve_REDIRECT(string &urlfrom, string &urlto) const;
	bool retrieve_BYEBYE(pid_t &pid, hp_status &status, time_t &retry_time) const;
	bool retrieve_STICK(string &ip, int &port, pid_t &pid) const;
	bool retrieve_HELLO(string &name) const;
private:
	void safeStr(string &str);
	
private:
	string __Msg;
	QM_MsgType __Type;

	string __host;
	pid_t __port;
	string __urlfrom;
	string __urlto;
	string __anchor;

	hp_status __status;
	time_t __retry_time;

	string __name;
	pid_t __pid;
};
#endif // _PAT_Q_M_MSG_021704
