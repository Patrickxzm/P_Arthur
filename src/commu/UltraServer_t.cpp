#include <sstream>
#include "util/arg.h"
#include "UltraServer.h"
using namespace std;
int 
main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	CUltraServer ultra(8056);
	while (1) {
		int sock;
		string* msg;
		msg = ultra.rmsg(sock);
		cout<<*msg<<endl;
		if (*msg == "halt!")
			break;
		delete(msg);
		ostringstream oss;
		oss<<"You are No."<<sock;
		ultra.wmsg(sock, oss.str());
	}
	return 0;
}
catch (exception &e)
{
	cerr<<e.what()<<endl;
	return -1;
}
