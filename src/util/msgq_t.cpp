/**************************************************************************
 * @ Split msgq.cpp into msgq.cpp & msgq_t.cpp, test code is put into 
 *   msgq_t.cpp.
 *					08/20/2005	14:06
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

#include <iostream>
int anonymous_msgq();
void help();

int declare_msgq(const char* path)
{
	const int nmsgs = 10;
	const int size = 125;
	CMsgQue que;
	que.init(nmsgs, size, path);
	char ch='a';
	while(ch!='q'){
		std::cin>>ch;
	}
	return 0;
}

int send_msg(const char* path, const char* msg)
{
	using namespace std;
	CMsgQue que;
	if (que.attach(path)){
		cout<<"failed to attach msgque.\n";
		return -1;
	}
	if (*msg != '\0')
		que.put(msg);
	else {
		string msg;
		cin>>msg;
		que.put(msg.c_str());
	}
	return 0;
}

int
rcv_msg(const char* path)
{
	using namespace std;
	CMsgQue que;
	if (que.attach(path)){
		cout<<"failed to attach msgque.\n";
		return -1;
	}
	char* buf = new char[que.size()];
	if (que.get(buf))
		cout<<"failed to get msg!\n";
	else cout<<buf<<std::endl;
	delete[] buf;
	return 0;
}

int 
main(int argc, char* argv[])
{
	using namespace std;
	const char* msg = 0;
	bool a_flag = false, o_flag = false, s_flag = false;
	bool r_flag = false;
	
	if (argc==1) {
		help();
		return 0;
	}
	for (int i=1; i<argc; i++){
		if (argv[i][0] != '-')
			continue;
		switch (argv[i][1]){
		case 'a':
			a_flag = true;
			break;
		case 'o':
			o_flag = true;
			break;
		case 's':
			s_flag = true;
			msg = argv[i]+2;
			break;
		case 'r':
			r_flag = true;
			break;
		default:
			help();
			break;
		}
	}
	const char* path = "/tmp/queen_msg";
	try {
		if (a_flag) 
			return anonymous_msgq();
		if (o_flag)
			return declare_msgq(path);
		if (s_flag)
			return send_msg(path, msg);
		if (r_flag)
			return rcv_msg(path);
	} catch (excep &e){
		cerr<<e.msg<<endl;
	}
	return 0;
}

void help()
{	using namespace std;
	cout<<"Testing program of capsuled messge queque.\n"
		"Usage: msgq \n"
		"-h: display this message.\n"
		"-a: test anonymous queue.\n"
		"-sMSG: send a MSG to PATH (named queue).\n"
		"-r: receive a msg from PATH, and output it.\n"
		"-o: declare a named msgq.(default\"/tmp/queen_msg\")\n";
	return ;
}

void sendmsg(CMsgQue &que, int nmsg)
{
	int i;
	char* buf = new char[que.size()];
	for (i=0; i<nmsg; i++){
		sprintf(buf, "No.%d Hello from process %d.", i, getpid());
		que.put(buf);
	}
	sprintf(buf, "exitmsg from process %d.", getpid());
	que.put(buf);
	delete[] buf;
}

int anonymous_msgq()
{
	const int nchild = 10;
	int buf_size = 125;
	using namespace std;
	CMsgQue que(10, buf_size);
	int i;
	for (i=0; i<nchild; i++) {
		pid_t pid = fork();
		if (pid<0) {
			cerr<<"fork()\n";
			return -1;
		} else if (pid==0) { //child.
			cout<<"child:"<<getpid()<<endl;
			sendmsg(que, 10);
			return 0;
		}
	}
	// parent
	char buf[buf_size];
	i=0;
	for(;;){
		if (que.get(buf))
			cout<<"failed to get msg!\n";
		else cout<<buf<<endl;
		if (strstr(buf,"exitmsg"))
			i++;
		if (i==nchild) break;
	}
	
	for (i=0; i<nchild; i++){
		int status;
		wait(&status);
	}
	return 0;
}
