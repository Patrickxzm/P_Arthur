/***********************************************************************
 * @ Take infomation about hostport out of CMosquitos to a new class 
 *   CHostports. CMosquitos is just to record the pid of every mosquito.
 *   					06/18/2004	10:53
 * @ We can miss() a hostport when it is quali-active status because the
 *   network is unstable.		04/15/2004	21:25
 * @ When a hostport is first assigned(introduced) to this queen, it becomes
 *   a newcomer from a stranger to queen.
 *					04/15/2004	20:39
 * @ We can miss() a hostport when it is in newcomer status.
 *					04/12/2004	20:39
 * @ When there is refs to the hostport, we said it is ready and its value
 *   is ???. I define a enum type to represent these const.
 *		--- I have changed my idea!
 *					03/10/2004	13:22
 * @ When a hostport cannot pass filters, we said it is missed and its value
 *   is -3.				03/09/2004	00:07
 * @ Heavily use CMosquitos more than recording running mosquitos's pid. 
 *   When a hostport is gived by the king, there is an entry for it, and 
 *   its value is -2, we said it is assigned; When a hostport is crawled 
 *   by a mosquito once(passed the filters), value is -1, we said it is 
 *   qualified; When a mosquito is crawling it, OK, value is the mosquito's 
 *   pid, we said it is actived.
 *					03/07/2004	11:49
 * @ Number of active mosquitos is needed. At first it equals the size of 
 *   mosquitos, now that there is also an entry for non_active mosquito in
 *   class CMosquitos, we need a method to get number of active mosquitos.
 *					01/04/2004	15:19
 * @ CMosquitos contains hostports info. These infos should be write to disk
 *   when CMosquitos destruct, and be read from disk when CMosquito init.
 *   Currently a file ".hostport" is used to store these hostports.
 *					01/03/2004	17:49
 * @ At first, CMosquitos is only used to manage mosquitos(recording their
 *   pids). Now Queen use this class to manage hostports belong to her. 
 *   When no mosquito is attached to a hostport, there is also an entry 
 *   for it in class CMosquitos, and pid in the entry is -1;
 *					01/03/2004	17:31
 ***********************************************************************/
#ifndef _PAT_MOSQUITOS_H_100203_
#define _PAT_MOSQUITOS_H_100203_
#include <string>
#include <ext/hash_map>
#include "pat_types.h"
#include "excep.h"
using namespace pat_types;

class MossErr:public excep
{
public:
        MossErr(){}
        MossErr(const string &m):excep("CMosquitos::"+m)
        {
        }
};

// map : pid ----> hostport
class CMosquitos : public hash_map_int2str
{
public:
	CMosquitos() 
	{
	}
	virtual ~CMosquitos()
	{}
};
#endif // _PAT_MOSQUITOS_H_100203_
