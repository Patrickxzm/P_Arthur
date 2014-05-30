#include <sstream>
#include <assert.h>
#include <limits>
#include "mcast.h"
extern "C" {
#include "unp.h"
#undef max
};

//#define _DEBUG

#ifdef _DEBUG
#endif // _DEBUG

#ifdef _DEBUG
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#endif //_DEBUG

using std::ostringstream;
using std::istringstream;

const int CMultiCast::max_fragment_length=1024;
//const int CMultiCast::max_fragment_num=1024;
//const int CMultiCast::max_message_num=1024;
//const int CMultiCast::max_buf_size=CMultiCast::max_fragment_length+20;

CMultiCast::CMultiCast()
   : ifindex(0), sa(0), recvfd(-1), max_buffer_size(5000), sendfd(-1), message_num(0)
{
}

int
CMultiCast::setBufferSize(int size)
{
	int tmp = max_buffer_size;
	max_buffer_size = size;
	return tmp;
}

CMultiCast::CMultiCast(const string &maddr, const string &serv)
   : ifindex(0), sa(0), recvfd(-1), sendfd(-1), message_num(0)
{
	init(maddr, serv);
}

void
CMultiCast::init(const string &maddr, const string &serv)
{
	this->maddr = maddr;
	this->serv = serv;
	return;
}

void
CMultiCast::set_join_if(const string &ifname, unsigned ifindex)
{
	this->ifname = ifname;
	this->ifindex = ifindex;
	return;
}

CMultiCast::~CMultiCast()
{
	if (sendfd >= 0)
	{
		sendfrag("", 0, message_num, -1);  // end of session.
		close(sendfd);
	}
	if (recvfd >= 0)
		close(recvfd);
	if (sa != 0)
		free(sa);
}

void
CMultiCast::send(const string &message)
{
	if (sendfd == -1)
	{
		sendfd = Udp_client(maddr.c_str(), serv.c_str()
		   , &sa, &salen);
		Mcast_set_loop(sendfd, 1);
	}
	int sent = 0;
	int frag_num = 0;
	while (true)
	{
		int length = message.size() - sent > 
			   (unsigned)max_fragment_length ? 
			   max_fragment_length : message.size() - sent;
		sendfrag(message.c_str()+sent, length
		   , this->message_num, frag_num++);
		sent += length;
		// DO NOT block the sender
		// usleep(100);
		//
		if (length < max_fragment_length) // the last fragment
			break;
	}
	this->message_num ++;
	return ;
}
/*
// Return a pointer to the session from which we got a message.
// session->msgNo >=0 : we got a complete message.
//		  -1  : this session is over
 */
const CMultiCastSession*
CMultiCast::recv(string &message, struct sockaddr *from, socklen_t *addrlen
	, void (*onStatChange)(const CMultiCastSession*
	   , CMultiCastSession::status_t))
{
	if (recvfd == -1)
	{
		recvfd = Udp_client(maddr.c_str(), serv.c_str()
		   , &sa, &salen);
		const int 		on=1;
		Setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		Bind(recvfd, sa, salen);
		const char* if_name = 0;
		if (this->ifname.size() > 0)
			if_name = this->ifname.c_str();
		Mcast_join(recvfd, sa, salen, if_name, this->ifindex);
	}
	while (true)
	{
		while (recv_buffer.size() > (unsigned)max_buffer_size)
		{
			recv_buffer.pop_front(removed);
			removed.stat.recalc(removed.msgNo+1); //include msgNo
			if (onStatChange)
				onStatChange(&removed, CMultiCastSession::Out);
		}
		int message_num, frag_num, length;
		string frag;
		recvfrag(frag, message_num, frag_num, length
		   , from, addrlen);
		string from_str = Sock_ntop(from, *addrlen);
#ifdef _DEBUG
		std::clog<<"("<<message_num<<", "<<frag_num<<")\n";
#endif // _DEBUG
		recv_buffer_t::iterator session = recv_buffer.find(from_str);
		if (session == recv_buffer.end())
			session = recv_buffer.push_back(from_str);
		if (frag_num == -1)
		{// end of this session
			removed.from = from_str;
			recv_buffer.remove(removed);
			removed.stat.recalc(message_num); //exclude msgNo
			if (onStatChange)
				onStatChange(&removed, CMultiCastSession::Shutdown);
			continue;
		}
		if (message_num < session->msgNo)
		{ // mcast_session reset
			removed.from = from_str;
			recv_buffer.remove(removed);
			removed.stat.recalc(removed.msgNo+1);
			if (onStatChange)
				onStatChange(&removed, CMultiCastSession::Reset);
			session = recv_buffer.push_back(from_str);
		}
		if (session->stat.start_msgNo==CMultiCastSession::Initial)
			session->stat.recalc(message_num);
	   	else if (message_num >= session->stat.start_msgNo+session->stat.interval)
		{
			session->stat.recalc(message_num);
			if (onStatChange)
				onStatChange(session.operator->(), CMultiCastSession::Recalc);
		}
		if (session->assemble(message, frag, message_num, frag_num, length))
		{
			// put session to queue tail.
			recv_buffer.splice(recv_buffer.end(), recv_buffer, session);
			return session.operator->();
		}
	}
	assert(false);
}

void 
CMultiCast::sendfrag(const char* frag, size_t length
   , int message_num, int frag_num)
{
//
// fragment:  message_num frag_num length '\n' data
//
	ostringstream oss;
	oss<<message_num<<' '<<frag_num<<' '<<length<<'\n';
	oss.write(frag, length);
	Sendto(sendfd, oss.str().c_str(), oss.str().length(), 0
	   , sa, salen);
	return;
}

void
CMultiCast::recvfrag(string &frag, int &message_num, int &frag_num
   , int &length, struct sockaddr *from, socklen_t *addrlen)
{
	int max_buf_size = max_fragment_length+20;
	char 			buf[max_buf_size];
	int n = Recvfrom(recvfd, buf, max_buf_size, 0, from, addrlen);
	istringstream iss(string(buf, n));
	iss>>message_num>>frag_num>>length;
	iss.ignore(std::numeric_limits<int>::max(), '\n');
	iss.read(buf, length);
	frag = string(buf, length);
	return; 
}
