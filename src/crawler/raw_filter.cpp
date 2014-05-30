#include "commu/tw_raw.h"
#include <iostream>
#include <stdexcept>
#include <sstream>

using namespace std;

int
main(int argc, char* argv[])
{
	CTWRaw raw;
	while (cin>>raw)
	{
		if (raw.url.length() < 4)
		{
			ostringstream oss;
			oss<<"Bad url, length<4: "<<raw.url;
			throw runtime_error(oss.str());
		}
		string ext(raw.url, raw.url.length()-4);
		if (strcasecmp(ext.c_str(), ".flv") == 0)
		{
			cerr << raw.url <<endl;
			continue;
		}
		cout << raw <<endl;
	}
	return 0;
}
