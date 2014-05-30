#ifndef CWPAGEDEALER_XZM_052802 
#define CWPAGEDEALER_XZM_052802 

#include "http.h" 
class CWPageDealer{ 
public: 
	virtual int deal(CHttp& http) = 0;
	virtual int revive()
	{ 
		return 0;
	}
	virtual int destroy()
	{
		return 0;
	}
};

#endif /* CWPAGEDEALER_XZM_052802 */
