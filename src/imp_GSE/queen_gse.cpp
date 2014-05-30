#ifdef _GSE_IMP
#include "queen.h"
#include <iostream>
using namespace std;

class CSaveLinksOption : public CServant 
{
public:
	CSaveLinksOption() : option("--save-links")
	{
	}
	virtual const char* mos_option(const string &hostport)
	{
		return option.c_str();
	}
	virtual ~CSaveLinksOption()
	{
	}
private:
	string option;
};

void help()
{
	cout<<"Queen for Gerneral SE. --save-links enabled.\n";
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
	CSaveLinksOption save_links_option;
	queen.add_servant(&save_links_option);
	queen.run();
	return 0;
}
catch (excep &e)
{
	cerr<<e.msg<<endl;
}
#endif //_GSE_IMP
