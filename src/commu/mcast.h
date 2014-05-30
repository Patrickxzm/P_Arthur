#ifndef _PAT_MCAST_H_110608
#define _PAT_MCAST_H_110608
#include <string>
#include <map>
#include <sys/socket.h>
#include "mcast_session.h"

using std::string;
using std::map;


class CMultiCast
{
public:
	CMultiCast();
	CMultiCast(const string &maddr, const string &serv);
	virtual ~CMultiCast();

	void init(const string &maddr, const string &serv);
	void set_join_if(const string &ifname, unsigned ifindex);

	void send(const string &message);		

	// recv messages from multi-senders;
	const CMultiCastSession* recv(string &message, struct sockaddr *from
	  , socklen_t *addrlen, void (*onStatChange)(const CMultiCastSession*
	     , CMultiCastSession::status_t));
	int setBufferSize(int size);
public:
	static const int max_fragment_length;
	//static const int max_fragment_num;
	//static const int max_message_num;
private:
	void recvfrag(string &frag, int &message_num, int &frag_num, int &length
	   , struct sockaddr *from, socklen_t *addrlen);
	void sendfrag(const char* frag, size_t length, int message_num, int frag_num);
private:
	string maddr;
	string serv;
	string ifname;
	unsigned ifindex;
	struct sockaddr *sa;
	socklen_t salen;

	int recvfd; // init on first recv();
	typedef index_queue<CMultiCastSession> recv_buffer_t;
	recv_buffer_t recv_buffer;
	int max_buffer_size;
	CMultiCastSession removed;

	int sendfd; // init on first send();
	int message_num; // for send();
};

#endif // _PAT_MCAST_H_110608
