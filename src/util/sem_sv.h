#ifndef _PAT_SEM_H_030703
#define _PAT_SEM_H_030703
/* 
 * In System V, A semaphoreis not just a single nonnegative value. Instead
 * we have to define a semaphore as a set of one or more semaphore values.
 * When we create a semaphore we specify the number of values in the set.
 *						----W.Richard Stevens
 */
#include <sys/types.h>
#include <unistd.h>
#include "excep.h"
class SemErr: public excep{
public:
	SemErr()
	{}
	SemErr(const std::string& m) : excep(m)
	{}
};

class CSemaphoreS{
public:
	CSemaphoreS(); 
	int init(int vals[], int nsems, const char* path=0);
	int attach(const char* path);
	CSemaphoreS(int vals[], int nsems);
	virtual ~CSemaphoreS();
	int P(int semnum, bool block=true);
	int V(int semnum);
private:
	int semid;
	pid_t creater;
};
#endif /* _PAT_SEM_H_030703 */
