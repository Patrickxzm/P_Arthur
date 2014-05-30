#ifndef _PAT_TASK_H_09252009
#define _PAT_TASK_H_09252009
#include <ostream>
#include <istream>
#include <string>
#include "page_status.h"
using std::string;
using std::ostream;
using std::istream;

class CTask
{
public:
	//enum {Visited=0, Unvisit=1, Stale=2, Bad=3, Unknown=4, Blink=8};
	enum {Visited=1, Unknown=2, Stale=4, Bad=8, Blink=16, Delete=32};
	typedef string key_type;
	typedef unsigned short status_type;
public:
	CTask() : towait(0), waited(0), status(Unknown), newContentSum(0)
	{}
	CTask(const string &lp, unsigned short tw=0, unsigned short wd=0, status_type s=Unknown)
	: localstr(lp), towait(tw), waited(wd), status(s), newContentSum(0)
	{}
	bool isReady() const;
	void reschedule(const page_status_t &page_status);
	inline const key_type &key() const
	{
		return localstr;
	}
	friend ostream& operator<<(ostream& os, const CTask &task);
	friend istream& operator>>(istream& is, CTask &task);
	friend istream& read_old(istream& is, CTask &task);
public:
	key_type localstr;
	unsigned short towait;
	unsigned short waited;
	status_type status;
private:
	unsigned  newContentSum;
private:
	void fadeOut(unsigned f);
};

ostream& operator<<(ostream& os, const CTask &task);
istream& operator>>(istream& is, CTask &task);
istream& read_old(istream& is, CTask &task);

#endif // _PAT_TASK_H_09252009
