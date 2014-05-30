#ifndef _PAT_MSGQ_H_030703
#define _PAT_MSGQ_H_030703
#include <string>
#include <fstream>
#include "sem_sv.h"

using namespace std;
class MsgqErr:public excep
{
public:
	MsgqErr(){}
	MsgqErr(const string &m):excep("CMsgQue::"+m){}
};

class CMsgQue {
public:
	CMsgQue();
	CMsgQue(int nmsgs, int size); //short cut for anonymous msgq;
	int init(int nmsgs, int size, const char* path=0);
	int attach(const char* path);
	bool set_block(bool block);	
	virtual ~CMsgQue();
public:
	/* message is '\0' terminated, and assert( strlen(msg)<size ); */
	int get(char msg[], int *remain=0);
	int get(string &msg, int *remain=0);
	int put(const char msg[], int *remain=0);
	int put(const string &msg, int *remain=0);
	int size() {
		if (stat == 0)
			return -1;
		return stat->size;
	}
private:
	struct que_stat{
		int iread;
		int iwrite;
		int size;
		int nmsgs;
	};
	
	bool usable;
	// shared map file
	volatile void *map_block;
	volatile struct que_stat* stat;
	volatile char* msgs;
	// end of shared

	CSemaphoreS *sems;
	pid_t creater; 
	string path;
	int block_size;
	bool _block;
	
	ofstream MsgLog;
};
#endif /* _PAT_MSGQ_H_030703 */
