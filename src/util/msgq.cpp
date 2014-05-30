/**************************************************************************
 * @ Data in the share memory is volatile.
 *					03/18/2004	11:12
 * @ On gcc-2.96, "errno" is not declared if you don't extern it.
 *					11/16/2003	10:44
 **************************************************************************/
#include "msgq.h"
#include <cassert>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <errno.h>

//extern int errno;

CMsgQue::CMsgQue() 
	:usable(false), sems(0), creater(-1), _block(true)
{
}

CMsgQue::CMsgQue(int nmsgs, int size)
	:usable(false), sems(0), creater(-1), _block(true) 
{
	init(nmsgs, size, 0);
}

bool 
CMsgQue::set_block(bool block)
{
	bool tmp = _block;
	_block = block;
	return tmp;
}

int CMsgQue::init(int nmsgs, int size, const char* path) 
{
	using namespace std;
	assert(nmsgs>0 && size>0);
	if (path != 0)
		this->path = path;
	block_size = nmsgs*size+sizeof(struct que_stat);
	if (path == 0){ 
		//MsgLog.open("anon_msgs.log", ios::out);
		//MsgLog.close();
		//MsgLog.open("anon_msgs.log", ios::out|ios::app);
		map_block = mmap(0, block_size
				,PROT_READ|PROT_WRITE
				, MAP_NORESERVE|MAP_SHARED|MAP_ANON
				, -1, 0);
	} else { // named queue
		ostringstream oss;
		oss<<path<<".log";
		//MsgLog.open(oss.str().c_str(), ios::out);
		//MsgLog.close();
		//MsgLog.open(oss.str().c_str(), ios::out|ios::app);

		int fd;
		if ((fd=open(path, O_RDWR|O_CREAT, 0644)) == -1)
			throw MsgqErr(string("init():open():")
					+path+':'
					+strerror(errno));
		ftruncate(fd, block_size);
		map_block = mmap(0, block_size
				, PROT_READ|PROT_WRITE
				, MAP_NORESERVE|MAP_SHARED
				, fd, 0);
		close(fd);
	}
	if (map_block == MAP_FAILED)
		throw MsgqErr(string("CMsgQue::init():mmap():")+strerror(errno));
	creater = getpid();
	stat = (struct que_stat*)map_block;
	msgs = (char*)map_block + sizeof(struct que_stat);

	stat->iread = stat->iwrite = 0;
	stat->size  = size;
	stat->nmsgs = nmsgs;

	int vals[4];
	vals[0] = 0; 		// There is no msg to read.
	vals[1] = nmsgs;	// nmsgs msgs can be written to queue.
	vals[2] = 1; 		// Only one can read at the same time.
	vals[3] = 1;		// Only one can write at the same time.
	sems=new CSemaphoreS;
	sems->init(vals, 4, path);
	usable = true;
	return 0;
}

int CMsgQue::attach(const char* path)
{
	using namespace std;
	assert(path!=0);
	assert(creater == -1);
	this->path = path;
	
	ostringstream oss;
	oss<<path<<".log";
	//MsgLog.open(oss.str().c_str(), ios::out|ios::app);

	int fd;
	if ((fd=open(path, O_RDWR|O_CREAT, 0644)) == -1)
		throw MsgqErr(string("attach():open():")
				+path+':'
				+strerror(errno));
	struct stat buf;
	fstat(fd, &buf);
	block_size = buf.st_size;
	map_block = mmap(0, block_size, PROT_READ|PROT_WRITE
			  , MAP_NORESERVE|MAP_SHARED
			  , fd, 0);
	stat = (struct que_stat*)map_block;
	msgs = (char*)map_block + sizeof(struct que_stat);
	sems=new CSemaphoreS;
	if (0 != sems->attach(path))
		throw MsgqErr(string("attach():attach():")
				+path+':'
				+strerror(errno));
	usable = true;
	return 0;
}

CMsgQue::~CMsgQue()
{
	if (sems != 0) 
		delete(sems);
	if (map_block != 0)
		munmap((void*)map_block, block_size);
	if (getpid() == creater && !path.empty()) 
		unlink(path.c_str());
}

int CMsgQue::get(char msg[], int *remain)
{
	if (!usable) return -1;
	int size = stat->size;
	int nmsgs = stat->nmsgs;
	volatile int &iread = stat->iread;
	volatile int &iwrite = stat->iwrite;
	if (sems->P(0, _block) || sems->P(2))
		return -1;
	strcpy(msg, (char*)msgs+iread*size);
	//MsgLog<<"(<"<<iread<<','<<iwrite<<"):"<<msg<<endl;
	iread = (iread + 1) % nmsgs;
	if (remain != 0)
	{
		*remain = iwrite-iread;
		if (*remain < 0)
			*remain += nmsgs;
	}
	if (sems->V(2) || sems->V(1))
		return -1;
	return 0;
}

int CMsgQue::get(string &msg, int *remain)
{
	if (!usable) return -1;
	int size = stat->size;
	int nmsgs = stat->nmsgs;
	volatile int &iread = stat->iread;
	volatile int &iwrite = stat->iwrite;

	if (sems->P(0, _block) || sems->P(2))
		return -1;
	msg = (char*)msgs+iread*size;
	//MsgLog<<"(<"<<iread<<','<<iwrite<<"):"<<msg<<endl;
	if (msg.size()==0) {
		string msg1;
		msg1.assign((char*)msgs+iread*size, size-1);
		//MsgLog<<"------"<<msg1<<endl;
		cout<<"debug me!:"<<getpid()<<endl;
		sleep(30);
	}
	iread = (iread + 1) % nmsgs;
	if (remain != 0)
	{
		*remain = iwrite-iread;
		if (*remain < 0)
			*remain += nmsgs;
	}
	if (sems->V(2) || sems->V(1))
		return -1;
	return 0;
}

using std::cout;

int CMsgQue::put(const char msg[], int *remain)
{
	if (!usable) return -1;
	int size = stat->size;
	int len;
	for (len = 0; len < 2*size; len++)
	{
		if (msg[len] == '\0')
			break;
	}
	if (len == 0 || len > size-1)
		return -1;
	int nmsgs = stat->nmsgs;
	volatile int &iread = stat->iread;
	volatile int &iwrite = stat->iwrite;
	if (sems->P(1, _block) || sems->P(3))
		return -1;
	memcpy((char*)msgs+iwrite*size, msg, len);
	memset((char*)msgs+iwrite*size+len, 0, size-len);
	//MsgLog<<"(>"<<iread<<','<<iwrite<<')'<<':'<<msg<<endl;
	iwrite = (iwrite + 1) % nmsgs;
	if (remain != 0)
	{
		*remain = iwrite-iread;
		if (*remain < 0)
			*remain += nmsgs;
	}
	if (sems->V(3) || sems->V(0))
		return -1;
	return 0;
}

int CMsgQue::put(const string& msg, int *remain)
{
	return put(msg.c_str());
}

