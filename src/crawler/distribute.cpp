#include "distribute.h"
#include "excep.h"
#include <openssl/md5.h>
#include <sstream>
#include <iostream>
using std::istringstream;

void 
CDistribute::read_arg(const string &arg)
//arg : total.part1.part2....
{
	istringstream iss(arg);
	if (!(iss>>total)) 
		throw excep("CDistirbute::read_arg():"
			"Can not get \'total\' value!\n"
			);
	char ch;
	unsigned part;
	while (iss>>ch && ch=='.' && iss>>part) 
	{
		if (part>=0 && part<total) 
			parts.push_back(part);
	}
}

bool
CDistribute::pass(const string &host)
{
	unsigned char md5[16];
	MD5((const unsigned char*)host.c_str(), host.size(), md5);
	unsigned mypart = (md5[0]*256 + md5[1]) % total;
	for (unsigned i=0; i<parts.size(); i++)
	{
		if (parts[i] == mypart)
			return true;
	}
	return false;
}

#ifdef _TEST

using namespace std;

int 
main(int argc, char* argv[])
{
	CDistribute dis;
	for (int i=1; i<argc; i++)
	{
		if (argv[i][0] != '-')
			continue;
		if (argv[i][1] == 'd')
			dis.read_arg(argv[i]+2);
	}
	cout<<"Please input your distribute string:\n";
	string str;
	while (cin>>str) 
	{
		if (dis.pass(str))
			cout<<"Passed!\n";
		else cout<<"Not pass.\n";
		cout<<"Input your distribute string again:\n";
	}
	return 0;
}
#endif // _TEST
