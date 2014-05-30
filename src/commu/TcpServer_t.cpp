#include <sstream>
#include "util/arg.h"
#include "TcpServer.h"

using namespace std;
int 
main(int argc, char* argv[])
try {
	CArg arg(argc, argv);
	CTCPServer server;
	server.listen(8055);
	server.accept();
	for (int i = 0; i<10; i++)
	{
		string msg;
		if (!getline(server, msg))
		{
			cout<<"Served one client, my job finished!\n";
			break;
		}
		cout<<msg<<endl;
		cout<<"Answer client:\n";
		string answer; 
		if (getline(cin, answer))
		{
			server<<answer<<endl;
		} else 
		{
			break;
		}
		//server<<"What are you saying?\n";
		//server.flush();
	}
	//server<<"Sorry, it's just a joke. I can hear you.\n";
	//server<<"Bye-bye! I'll gone.\n";
	return 0;
}
catch (exception &e)
{
	cerr<<e.what()<<endl;
}
