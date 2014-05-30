#include "URLSource.h"
#include <assert.h>
#include <unistd.h>

CURLSource::CURLSource()
{
	limit = 0;
	start_time = time(NULL);
}

int CURLSource::set_limit(int limit)
{
	assert(limit>=0);
	int tmp = this->limit;
	this->limit = limit;
	return tmp;
}

int CURLSource::control()
{
	if (limit == 0) return 0;

	const int interval = 10;
	if (count++ < limit * interval) 
		return count;
	int sleeptime;
	if ((sleeptime = interval - (time(NULL) - start_time)) > 0)
		sleep(sleeptime);
	
	/* re-count */
	count = 0;
	start_time = time(NULL);
	return 0;
}
	
