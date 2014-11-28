#include "util.h"
#include "charset.h"
#include <iostream>

using namespace std;

int
main(int argc, char* argv[])
{
	string in = "中国 aaa\xc2\xa0""bbb\r日本\n\t";
	cout<<compress_blank(in)<<endl;
	wstring wstr = wc("utf8", in, "UCS-4LE");
#ifdef _TEST_WCOUT
	std::wcout.imbue(std::locale("C"));
	wcout<<wstr<<endl;
#else
	cout.write((char*)wstr.c_str(), wstr.size()*sizeof(wchar_t));
	cout<<endl;
#endif // _TEST_WCOUT
	wstr = wc1(in);
#ifdef _TEST_WCOUT
	wcout<<wstr<<endl;
#else
	cout.write((char*)wstr.c_str(), wstr.size()*sizeof(wchar_t));
	cout<<endl;
#endif // _TEST_WCOUT
	return 0;
}
