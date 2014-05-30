#include <iostream>
#include "util/util.h"
using namespace std;

int
main()
{
	string s;
	while (cin>>s)
		cout<<iconv_str("gbk", "utf8", s)<<' '<<flush;
	return 0;	
}
