#include "debug.h"
#include <iostream>
#include <unistd.h>
using std::cout;
using std::endl;

void call_debuger(unsigned sleep_sec)
{
	cout<<"Please debug me! pid="<<getpid()<<endl;
	sleep(sleep_sec);
	return;
}
