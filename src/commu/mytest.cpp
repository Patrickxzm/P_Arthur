#include <iostream>
#include <sstream>

using namespace std;

int
main()
{
	// 1. test on istream::read();
	char buf[100];
	istringstream iss("xzmxzm");
	if (iss.read(buf, 6))
	{
		cout<<"OK"<<endl;
	}
	else
	{
		cout<<"failed"<<endl;
	}
	return 0;	
}
