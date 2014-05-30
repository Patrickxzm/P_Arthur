/**************************************************************************
 * @ To test a few mosquito's new fetures(HEAD for vedio) requested by SuHang, 
 *   I write a tester which act as a dealer.
 *					02/01/2004	00:33
 **************************************************************************/
#ifndef _PAT_TESTER_H_020104
#define _PAT_TESTER_H_020104
#include "dealer.h"
#include "excep.h"

class TestErr : public excep
{
public :
	TestErr() {}
	TestErr(const string &m) : excep("CTest::"+m)
	{
	}
};
class CTester : public CDealer
{
public:
	CTester();
	void init();
	virtual void deal(workbench_t &bench);
private:
	ofstream log;
};
#endif // _PAT_TESTER_H_020104


