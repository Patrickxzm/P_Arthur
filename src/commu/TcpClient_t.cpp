#include "util/arg.h"
#include "TcpClient.h"

using namespace std;
int
main(int argc, char* argv[])
try{
	CArg arg(argc, argv);
	CArg::ArgVal av;
	int port;
	if (!(av=arg.find1("-p")).null())
	{
		port = av;
	} 
	else {
		port = 8055;
	}
	CTCPClient client;
	client.timeout(25, 25);
	string ipstr;
	if (client.ConnectServer("localhost", ipstr, port) != 0) 
	{
		cout<<"Can not connect with server!\n";
		return -1;
	}
	cout<<"Connect server("<<ipstr<<") OK! send msg to host:"<<endl;
	string msg;
	while (getline(cin, msg))
	{
		string test(2000, 'c');
		client << test <<endl;
		//client << msg <<endl;
		if (!client) {
			cout << "Server is gone. \n"
				"He can not hear you any more. \n";
			break;	
		}	
		string reply;
		if (!getline(client, reply))
		{
			cout << "Server is gone. \n"
				"He can not hear you any more. \n";
			break;	
		}
		cout << reply<<endl;
		cout << "Send msg to host:" << endl;
	}
	return 0;
}
catch (exception &e)
{
	cerr<<e.what()<<endl;
}
