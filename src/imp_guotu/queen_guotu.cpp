#ifdef _IMP_GUOTU
#include "queen.h"
#include <iostream>
using namespace std;

void help()
{
	cout<<"Queen for Guotu project.\n";
}

int 
main(int argc, char* argv[])
try{
	CQueen queen;
	// Porfermance tuning ...
	struct queen_env env;
	queen.get_env(env);
	env.missed_links_size_limit = 200;
	env.hps.urlMaxSize = 20000;
	env.hps.urlDumpRatio = 0.1;

	env.hps.dbMaxSize = 3000000;
	env.hps.dbDumpRatio = 0.01;

	queen.set_env(env);

	if (queen.read_args(argc, argv) != 0)
	{
		help();
		queen.help();
		return -1;
	}
	queen.run();
	return 0;
}
catch (excep &e)
{
	cerr<<e.msg<<endl;
}
#endif //_IMP_GUOTU
