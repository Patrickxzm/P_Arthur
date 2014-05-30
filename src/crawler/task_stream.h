#ifndef _PAT_TASK_STREAM_H_08242009
#define _PAT_TASK_STREAM_H_08242009
#include <fstream>
#include <vector>
#include "task.h"
//#include "urlque.h"

using std::ofstream;
using std::vector;

class CActiveTaskFile 
{
public:
	CActiveTaskFile() 
	  : new_overflow(0), old_overflow(0), new_discard(0), old_discard(0)
	  , blink_overflow(0), blink_discard(0)
	  , nActive(0), maxActive(100000)
	{}
	virtual ~CActiveTaskFile()
	{
		close();
	}
	void close();
	void open(const char* fn, const char* fn_discard="discard"
		, const char* fn_overflow="overflow");
	operator bool() const 
	{
		return *this && discard && overflow;
	}
	CActiveTaskFile& put(const CTask &task, const CTask &bak);
public:
	int new_overflow, old_overflow;
	int new_discard, old_discard;
	int blink_overflow, blink_discard;
private:
	void collect(vector<CTask> &source, int &which_overflow);
private:
	unsigned nActive;
	unsigned maxActive;
	vector<CTask> b2b, b2v, u2v, u2u, v2v;
	ofstream active;
	ofstream discard;
	ofstream overflow;
};
#endif // _PAT_TASK_STREAM_H_08242009
