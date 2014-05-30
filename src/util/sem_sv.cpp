#include "portable/sys.h"
#include "sem_sv.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string>
#include <iostream>

using std::string;

extern int errno;

#ifdef _SEM_SEMUN_DEFINED_
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val;                  /* value for SETVAL */
	struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
	unsigned short *array;    /* array for GETALL, SETALL */
				  /* Linux specific part: */
	struct seminfo *__buf;    /* buffer for IPC_INFO */
};
#endif

CSemaphoreS::CSemaphoreS() : semid(-1), creater(-1)
{
}

static const int proj_id = 042603;
static const int perms = 0600;

int CSemaphoreS::init(int vals[], int nsems, const char* path)
{
	assert(semid == -1);
	assert(nsems>=0);

	if (path != 0){
		key_t key = ftok(path, proj_id);
		if (key==-1) 
			throw SemErr(string("CSemaphoreS::init:ftok():")+strerror(errno));
		union semun sem;
		if ((semid=semget(key, 0, 0)) != -1)
			semctl(semid, 0, IPC_RMID, sem);
		semid=semget(key, nsems, perms|IPC_CREAT|IPC_EXCL);
	} else {
		semid=semget(IPC_PRIVATE, nsems, perms);
	}
	if (semid==-1)
		throw SemErr(string("CSemaphoreS::init:semget():")+strerror(errno));
	creater = getpid();
	union semun sem;
	for (int i=0; i<nsems; i++)
	{
		sem.val = vals[i];
		if (semctl(semid, i, SETVAL, sem) == -1)
			throw SemErr(strerror(errno));
	}
	return 0;
}

int CSemaphoreS::attach(const char* path)
{
	assert(semid == -1);
	key_t key = ftok(path, proj_id);
	if (key == -1)
		throw SemErr(string("CSemaphoreS::attach:ftok():")+strerror(errno));
	semid = semget(key, 0, 0);
	if (semid == -1)
		return -1;
	return 0;
}
 
CSemaphoreS::CSemaphoreS(int vals[], int nsems):semid(-1), creater(-1)
{
	init(vals, nsems);
}

CSemaphoreS::~CSemaphoreS()
{
	union semun sem;
	if (getpid() == creater && semid != -1){
		if (semctl(semid, 0, IPC_RMID, sem)==-1)
			throw SemErr(string("CSemaphoreS::~:semctl():")+strerror(errno));
	}	
}

int CSemaphoreS::P(int semnum, bool block)
{
	//std::cout<<"P("<<semnum<<")\n";
	assert(semid != -1);
	struct sembuf sop;
	sop.sem_num = semnum;
	sop.sem_op = -1;
	if (block)
		sop.sem_flg = 0;
	else 
		sop.sem_flg = IPC_NOWAIT;
	int ret = semop(semid, &sop, 1);
	while (-1 == ret && errno == EINTR)
	{
		ret = semop(semid, &sop, 1);
	}
	return ret;
	//return semop(semid, &sop, 1);
}

int CSemaphoreS::V(int semnum)
{
	//std::cout<<"V("<<semnum<<")\n";
	assert(semid != -1);
	struct sembuf sop;
	sop.sem_num = semnum;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	return semop(semid, &sop, 1);
}

