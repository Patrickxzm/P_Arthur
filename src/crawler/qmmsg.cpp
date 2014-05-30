/**************************************************************************
 * @ QM_BYEBYE is changed again:
 *   	1. "byebye!" SP pid SP status SP retry_time
 * @ To enable incremental crawling, QM_BYEBYE is changed again:
 *   	1. "byebye!" SP host SP port SP status SP retry_time
 *   retry_time is time_t.			07/31/2004	11:04
 * @ QM_MISS & QM_DELAY is merged into QM_BYBYE. and QM_BYEBYE's format is
 *   changed:
 *   	1. "byebye!" SP host SP port SP status 
 * 						06/29/2004	11:23
 * @ Add the seventh type of message to show mosquito's unwilling quit:
 *	7. "delay" SP host SP port
 * @ Add the sixth type of message to limit connection number to single IP:
 *	6. "stick_on" SP host SP port SP pid
 * @ In the message string, specicial chars as '\r', '\n', or '\0' will make
 *   trouble, so they are erased in method safeStr().
 *					04/28/2004	09:37
 * @ The fifth type of message:
 *	5. "mynameIs:" SP name
 * @ There is the fourth type of message:
 * 	4. "miss!" SP host SP port
 * @ There is three types of message now:
 *	1. "byebye!" SP host SP  port
 *	2. "ref:" SP urlto SP <== SP urlfrom SP [{ SP anchor SP }]
 *	3. "redirect:" SP urlto SP <== SP urlfrom
 *					02/17/2004	16:47
 **************************************************************************/

#include "qmmsg.h"

#include <sstream>
using std::istringstream;
using std::ostringstream;

QM_MsgType
CQMMsg::setMsg(const string &strMsg)
{
	__Msg = strMsg;
	__Type = QM_BAD;
	istringstream iss(__Msg);
	string s1;
	iss>>s1;
	if (s1=="byebye!") 
	{ 
		unsigned status;
		if (iss>>__pid>>status>>__retry_time)
		{
			__Type = QM_BYEBYE;
			__status = status;
			return __Type;
		}
	}
	else if (s1=="ref:") 
	{
		iss>>__urlto;
		string tmp;
		while (iss>>tmp && tmp!="<==")
			;
		if (!(iss>>__urlfrom))
		{
			return __Type;
		}
		__Type = QM_REFER;
		while (iss>>tmp && tmp!="{") //"}"
			;
		if (iss)
			iss>>__anchor;
		__Type = QM_REFER;
	}
	else if (s1=="redirect:")
	{
		iss>>__urlto;
		string tmp;
		while (iss>>tmp && tmp!="<==")
			;
		if (!(iss>>__urlfrom))
		{
			return __Type;
		}
		__Type = QM_REDIRECT;
	}
	else if (s1=="mynameIs:")
	{
		if (!(iss>>__name))
			return __Type;
		__Type = QQ_HELLO;
	}
	else if (s1=="stick_on")
	{
		if (iss>>__host>>__port>>__pid)
		{
			__Type = QM_STICK;
			return __Type;
		}
	}
	return __Type;
}

void
CQMMsg::safeStr(string &str)
{
	for (int i=str.length()-1; i>=0; i--)
	{
		if (str[i] == '\r' || str[i] == '\n' || str[i] == '\0')
			str.erase(i);
	}
	return;
}

bool
CQMMsg::compose_REFER(const string &urlfrom, const string &urlto
	, const string &anchor)
{
	__urlfrom = urlfrom;
	__urlto = urlto;
	__anchor = anchor;
	safeStr(__urlto);
	safeStr(__urlfrom);
	safeStr(__anchor);
	if (__urlto.empty() || __urlfrom.empty())
	{
		__Type = QM_BAD;
		return false;
	}
	ostringstream oss;
	oss<<"ref: "<<__urlto<<" <== "<<__urlfrom;
	if (!__anchor.empty())
		oss<<" { "<<__anchor<<" }";
	__Msg = oss.str();
	__Type = QM_REFER;
	return true;
}

bool
CQMMsg::compose_REDIRECT(const string &urlfrom, const string &urlto)
{
	__urlfrom = urlfrom;
	__urlto = urlto;
	safeStr(__urlto);
	safeStr(__urlfrom);
	if (__urlto.empty() || __urlfrom.empty())
	{
		__Type = QM_BAD;
		return false;
	}
	ostringstream oss;
	oss<<"redirect: "<<__urlto<<" <== "<<__urlfrom;
	__Msg = oss.str();
	__Type = QM_REDIRECT;
	return true;
}

bool 
CQMMsg::compose_BYEBYE(const pid_t &pid, hp_status status, time_t retry_time)
{
	__pid = pid;
	__status = status;
	ostringstream oss;
	oss<<"byebye! "<<__pid<<' '
		<<(unsigned)status<<' '<<retry_time;
	__Msg = oss.str();
	__Type = QM_BYEBYE;
	return true;
}

bool 
CQMMsg::compose_STICK(const string &ip, int port, pid_t pid)
{
	__host = ip;
	__port = port;
	__pid = pid;
	safeStr(__host);
	if (__host.empty())
	{
		__Type = QM_BAD;
		return false;
	}
	ostringstream oss;
	oss<<"stick_on "<<__host<<' '<<__port<<' '<<__pid;
	__Msg = oss.str();
	__Type = QM_STICK;
	return true;
}

bool
CQMMsg::compose_HELLO(const string &myname)
{
	__name = myname;
	safeStr(__name);
	if (__name.empty())
	{
		__Type = QM_BAD;
		return false;
	}
	__Msg = "mynameIs: " + __name;
	__Type = QQ_HELLO;
	return true;
}


bool 
CQMMsg::retrieve_REFER(string &urlfrom, string &urlto, string &anchor) const
{
	if (__Type != QM_REFER)
		return false;
	urlfrom = __urlfrom;
	urlto = __urlto;
	if (urlto.empty())
		return false;
	anchor = __anchor;
	return true;
}

bool 
CQMMsg::retrieve_REDIRECT(string &urlfrom, string &urlto) const
{
	if (__Type != QM_REDIRECT)
		return false;
	urlfrom = __urlfrom;
	urlto = __urlto;
	if (urlto.empty())
		return false;
	return true;
}

bool 
CQMMsg::retrieve_BYEBYE(pid_t &pid, hp_status &status, time_t &retry_time) const
{
	if (__Type != QM_BYEBYE)
		return false;
	pid = __pid;
	status = __status;
	retry_time = __retry_time;
	return true;
}

bool 
CQMMsg::retrieve_STICK(string &host, int &port, pid_t &pid) const
{
	if (__Type != QM_STICK)
		return false;
	host = __host;
	if (host.empty())
		return false;
	port = __port;
	pid = __pid;
	return true;
}

bool 
CQMMsg::retrieve_HELLO(string &name) const
{
	name = __name;
	if (__name.empty())
		return false;
	return true;
}



