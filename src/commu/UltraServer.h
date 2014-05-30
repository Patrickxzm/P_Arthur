/************************************************************************* 
 * @ UltraServer deal with messages for multi-client. What is a message:
 *   A message is a string terminated with delimit string, but not include
 *   the delimit string. By default, delim string is "\n".
 *						11/18/2003	11:12
 * @ In distributed crawling System, KING communicated with lots of QUEENS
 *   as TCP server. His role need a strong CTCPServer, so get the new class
 *   name: CUltraServer.
 *						10/26/2003	15:46
 **************************************************************************/
#ifndef _CULTRASERVER_PAT_111803
#define _CULTRASERVER_PAT_111803
#include "TcpServer.h"
#include "util/excep.h"
#include <vector>
#include <queue>
#include <list>
#include <map>
using std::vector;
using std::queue;
using std::list;
using std::less;
using std::map;

class UltraErr : public std::runtime_error 
{
public:
	UltraErr(const string &m) : runtime_error("UltraErr::"+m)
	{}
};

class CUltraServer : private CTCPServer
{
public:
	CUltraServer(int port);
	virtual ~CUltraServer();
	string *rmsg(int &sock);
	void wmsg(int sock, const string &str);
private:
	void select();
	void read_sock(int sock);
	class __msg
	{
	public:
		__msg(string* _data, int _sock): data(_data), sockfd(_sock)
		{}
		string* data;
		int sockfd;
	};
	queue<__msg> msgs;
	map<int, string*, less<int> > clients;
	int max_sock;
	fd_set allset;
	string delim;
};
#endif // _CTCPSERVER_PAT_080202
