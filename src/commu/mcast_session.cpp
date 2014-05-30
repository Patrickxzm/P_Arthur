#include "mcast_session.h"
#include "mcast.h"
#include <sstream>
#include <assert.h>
#include <errno.h>
#include <stdexcept>
#include <cstring>

//#define _DEBUG
#ifdef _DEBUG
#include <iostream>
using std::clog;
using std::endl;
#endif //_DEBUG
using std::runtime_error;
using std::ostringstream;

const int CMultiCastSession::CStat::interval = 1000; // of messages

/*
 * There are three situations:
 *	1. start_msgNo == Initial 
 *	2. message_num > start_msgNo + interval
 *	3. message_num <= start_msgNo + interval
 */
bool 
CMultiCastSession::CStat::recalc(int message_num)
{	
	if (start_msgNo==Initial)
	{// start the first stat;
		if (-1 == gettimeofday(&start_time, 0))
		{
			ostringstream oss;
			oss<<"gettimeofday():"<<strerror(errno);
			throw runtime_error(oss.str());
		}
		start_msgNo = message_num - message_num % interval;
		return false;
	}
	int end_msgNo;
	if (message_num <= start_msgNo + interval)
		end_msgNo = message_num;
	else
		end_msgNo = message_num - message_num % interval;
#ifdef _DEBUG
	clog<<'['<<start_msgNo<<'-'<<end_msgNo<<"):"<<msgCount
	   <<endl;
#endif //_DEBUG
	lostRatio = end_msgNo-start_msgNo-msgCount;
	lostRatio *= 100;  // % 
	lostRatio /= end_msgNo-start_msgNo;

	struct timeval now;
	if (-1 == gettimeofday(&now, 0))
	{
		ostringstream oss;
		oss<<"gettimeofday():"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	double sb;
	sb = byteCount;
	sb /= (now.tv_sec - start_time.tv_sec) 
	   + (double)(now.tv_usec - start_time.tv_usec)/1000000;
	speedBytes = (int)sb;

	msgCount = 0;
	byteCount = 0;
	start_time = now;
	start_msgNo = end_msgNo;
	return true;
}

bool
CMultiCastSession::assemble(string &message, const string &frag
   , int message_num, int frag_num, int length)
{
	if (message_num < this->msgNo)
	{// message is FIFO
		ostringstream oss;
		oss<<"From["<<from<<"]: message_num("<<message_num
		   <<") is smaller than msgNo("<<msgNo<<")";
		throw runtime_error(oss.str());
	}

	if (message_num==msgNo && fragNo>=0 && frag_num==fragNo+1)
	{// append fragment to current message
		this->fragNo = frag_num;
		this->msgBuffer.append(frag);
	}
	else if (message_num>msgNo && frag_num==0)
	{// start a new message
		this->msgNo = message_num;
		this->fragNo = 0;
		this->msgBuffer = frag;
	}
	else {
		this->msgNo = message_num;
		this->fragNo = Initial;  // discard message and fragment.
	}

	if (msgNo >= 0 && fragNo >= 0 && length < CMultiCast::max_fragment_length)
	{// got a message.
		stat.countMsg(this->msgBuffer.size());
		message = this->msgBuffer;
		this->msgBuffer.clear();
		this->fragNo = Initial;
		return true;
	}
	return false;
}
