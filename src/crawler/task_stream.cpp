#include "task_stream.h"
#include <cassert>
using std::endl;

CActiveTaskFile&
CActiveTaskFile::put(const CTask& current, const CTask &origin)
{
	if (CTask::Delete & current.status
	  || CTask::Bad & current.status
	  || CTask::Stale & current.status
	)
	{
		discard<<current<<endl;
		if (origin.status & CTask::Blink)
			blink_discard ++;
		else if (origin.status & CTask::Visited)
			old_discard ++;
		else
			new_discard++;
		return *this;			
	}
	// writting to 'active' and 'overflow' is delayed until close()
	if (origin.status&CTask::Blink && current.status&CTask::Blink)
		b2b.push_back(current);
	else if (origin.status&CTask::Blink && current.status&CTask::Visited)
		b2v.push_back(current);
	else if (!(origin.status&CTask::Visited) && current.status&CTask::Visited)
		u2v.push_back(current);
	else if (!(origin.status&CTask::Visited) && !(current.status&CTask::Visited))
		u2u.push_back(current);
	else if (origin.status&CTask::Visited && current.status & CTask::Visited)
		v2v.push_back(current);
	return *this;
}

void
CActiveTaskFile::collect(vector<CTask> &source, int &which_overflow)
{
	for (unsigned i=0; i<source.size(); i++)
	{
		if (nActive<maxActive)
		{
			active << source[i] <<endl;
			nActive ++;
		}
		else
		{
			overflow << source[i] <<endl;
			which_overflow ++;
		}
	}
	source.clear();
	return;
}

void 
CActiveTaskFile::close()
{
	nActive = 0;
	if (!b2b.empty())
		collect(b2b, blink_overflow);
	if (!v2v.empty())
		collect(v2v, old_overflow);
	if (!b2v.empty())
		collect(b2v, blink_overflow);
	if (!u2v.empty())
		collect(u2v, new_overflow);
	if (!u2u.empty())
		collect(u2u, new_overflow);
	if (active.is_open())
		active.close();
	if (discard.is_open())
		discard.close();
	if (overflow.is_open())
		overflow.close();
	return;
}

void
CActiveTaskFile::open(const char* fn, const char* fn_discard
	, const char* fn_overflow)
{
	active.open(fn);
	discard.open(fn_discard);
	overflow.open(fn_overflow);
	return;
}
