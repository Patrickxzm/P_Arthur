/***********************************************************************
 * @ CTester is enforced to surport some statistic work on a given website.
 *					02/25/2004	11:43	
 ***********************************************************************/
#include "tester.h"

using std::endl;
CTester::CTester()
{
}

void
CTester::init()
{
	log.open("tester.log");
}

void
CTester::deal(workbench_t &bench)
{
	const CHttpReply &reply = bench.reply;
	const CURL &url = bench.url;
	log<<url.str()<<endl;
	log<<"depth from the seeds url: "<<bench.depth<<endl;
	log<< reply.status<<endl;
	int code = reply.status.code;
	if (code != 200 && code != 203)
		return;
	log<<"Content-Type: "<<reply.headers.value("Content-Type")<<endl;
	log<<"Retrived-Data-Len: "<<reply.body.size()<<endl;
	log<<endl;
	return;
}
