#include "task.h"
#include <assert.h>
#include <cstdlib>
#include <sstream>
using std::istringstream;
using std::ostringstream;

namespace {
	const unsigned short random_base = 4;
	const unsigned short game_over_towait = random_base*8;
	const unsigned short unchanged_fade_out = 2;
	const unsigned short access_error_fade_out = 4;
}

bool 
CTask::isReady() const
{
	assert(!(status&Stale) && !(status&Bad) && !(status&Unknown));
	if (!(status&Visited))
		return true;
	return (waited >= towait);
}

void 
CTask::reschedule(const page_status_t &page_status)
{
	assert(!(status&Stale) && !(status&Bad) && !(status&Unknown));
	
	if (page_status.getPage)
	{
		newContentSum += page_status.newContentNum();
		if (!(status&Visited))
		{
			if (page_status.newTextNum() > 0)
				towait = random() % random_base;
			else
				status |= Stale;
			waited = 0;
			status |= Visited;
			status &= ~Blink;
			return;
		}
		// visited url
		if (page_status.newContentNum()>0)
		{ // changed
			towait = waited/2 < towait ? waited/2 : towait;
			waited = 0;
			return;
		}
		// unchanged 
		fadeOut(unchanged_fade_out);
		waited = 0;
		return;
	}
	fadeOut(access_error_fade_out);
	waited ++;
	return ;
}

void
CTask::fadeOut(unsigned f)
{
	if (waited * f > game_over_towait)
		status |= Stale;
	else if (towait == 0)
		towait = 1;
	else 
		towait *= f;
	return;
}

istream& operator>>(istream& is, CTask &task)
{
	string line;
	if (!(is>>std::ws))
		return is;
	if (!getline(is, line))
		return is;
	istringstream iss(line);
	if (!(iss>>task.localstr>>task.status>>task.towait>>task.waited>>task.newContentSum))
		is.setstate(std::ios::failbit);
	return is;
}

ostream& operator<<(ostream& os, const CTask &task)
{
	os<<task.localstr<<'\t'<<task.status<<'\t'<<task.towait<<'\t'<<task.waited
	  <<'\t'<<task.newContentSum;
	return os;
}

istream& read_old(istream& is, CTask &task)
{
	string line;
	if (!(is>>std::ws))
		return is;
	if (!getline(is, line))
		return is;
	istringstream iss(line);
	if (!(iss>>task.localstr))
	{
		is.setstate(std::ios::failbit);
		return is;
	}	
	task.status = CTask::Unknown;
	task.towait = 0;
	task.waited = 0;
	task.newContentSum = 0;
	short old_towait;
	if (!(iss>>old_towait))
		return is;
	task.status = 0;
	if (old_towait >= 0)
	{
		task.status = CTask::Visited;
		task.towait = old_towait;
	}
	else {
		unsigned short s = -old_towait;
		if (s & 8)
			task.status |= CTask::Blink;
		s &= (~8);
		if (s == 0)
			task.status |= CTask::Visited;
		else if (s == 1)
			;
		else if (s == 2)
			task.status |= CTask::Stale;
		else if (s == 3)
			task.status |= CTask::Bad;
		else if (s == 4)
			task.status |= CTask::Unknown;
	}
        if (!(iss>>task.waited))
		return is;
        if (!(iss>>task.newContentSum))
		return is;
        return is;
}
