#include <ext/hash_set>
#include <iostream>
using namespace std;
using __gnu_cxx::hash;
int 
main(int argc, char* argv[])
{
	hash<const char*> H;
	unsigned hash_size;
	sscanf(argv[1], "%u", &hash_size);
	cout<<H(argv[2])%hash_size<<endl;
}
