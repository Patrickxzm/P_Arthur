#ifndef _PAT_20110618_MCAST_SESSION_H
#define _PAT_20110618_MCAST_SESSION_H

#include <string>
#include <sys/time.h>
#include "util/index_queue.hpp"
using std::string;

class CMultiCastSession
{
public:
	//enum {Initial=-1, Closed=-2, Popped=-3};
	enum status_t{ Initial=-1, Recalc=-2, Shutdown=-3, Reset=-4, Out=-5 };
	class CStat {
	public:
		CStat() : start_msgNo(-1), msgCount(0), byteCount(0)
		   , totalMesgs(0), totalBytes(0)
		   , speedBytes(-1), lostRatio(-1)
		{}
		void countMsg(size_t msgSize)
		{
			msgCount ++;
			byteCount += msgSize;
			totalMesgs ++;
			totalBytes += msgSize;
		}
		bool recalc(int message_num);
		static const int interval;
		struct timeval start_time;
		int start_msgNo; // a multiple of stat_interval;
		unsigned msgCount;
		unsigned byteCount;
		unsigned totalMesgs;
		unsigned totalBytes;
		int speedBytes;  // bytes per second
		int lostRatio; // % of messages
		
	};
	typedef string				key_type;
public:
	CMultiCastSession(const string &from1="")
	   : from(from1), msgNo(Initial), fragNo(-1)
	{
	}
	bool assemble(string &message, const string &frag, int message_num
	   , int frag_num, int length);
	virtual ~CMultiCastSession()
	{}
	inline const string& key() const
	{
		return from;
	}
public:	
	string from;
	int msgNo; // Initial or >= 0
	int fragNo; // -1 means message is returned or discarded.

	//unsigned totalBytes;
	//unsigned totalMesgs;
	CStat stat;
private:
	string msgBuffer;
};


#endif // _PAT_20110618_MCAST_SESSION_H
