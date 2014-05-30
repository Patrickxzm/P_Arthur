/***************************************************************************
 * @ Add two interfaces to CUContainer to enable dump records to and load 
 *   records from a file. This two interface will be called by mosquito when
 *   forced to quit(by user or queen), and when restart.
 *						03/03/2004	18:27
 * @ To enable more control on mosquito's crawling behavior, like a depth-first
 *   crawling in a vedio site scanning system(SuHang), we use an interface
 *   "CUContainer" in mosquito instead of CURLQue. As the same with CURLQue,
 *   CUContainer use a bitmap to maintain the URLs have been putted into. 
 *						01/18/2004  19:05
 **************************************************************************/
#ifndef _PAT_U_CONTAINER_011404
#define _PAT_U_CONTAINER_011404
#include "dealer.h"
#include "bitmap.h"
class urlinfo 
{
public:
	urlinfo(const string &_str, int _depth=0)
		:str(_str), depth(_depth)
	{}
	string str;
	int depth;  
private:
};

class CUContainer  // interface;
{
public:	
	CUContainer(unsigned mapsize=24);
	virtual ~CUContainer()
	{
	}
	virtual int dump(const char *file) = 0;
	virtual int load(const char *file) = 0;
	virtual int push2(const string &urlstr, int depth=0);
	virtual int push2(const urlinfo &info);
	virtual bool get_url(workbench_t &bench) = 0;
private:
	CURLBitmap map;
};
#endif // _PAT_U_CONTAINER_011404
