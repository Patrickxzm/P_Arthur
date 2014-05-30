#include "redirect.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <sstream>

using std::ostringstream;
using std::runtime_error;

int CRedirect::fds_null=-1;

CRedirect::CRedirect(int from) : fds_bak(-1), to(-1)
{
	this->from = from;
}

CRedirect::~CRedirect()
{
	restore();
}

int 
CRedirect::redirect(const char* fn)
{
	if (strcmp(fn, "/dev/null") == 0)
	{
		if (fds_null == -1 
		   && (fds_null=open("/dev/null", O_WRONLY)) == -1)
		{
			ostringstream oss;
			oss<<"open(\"/dev/null\"):"<<strerror(errno);
			throw runtime_error(oss.str());
		}
		return redirect(fds_null);
	}
	if ((this->to = open(fn, O_WRONLY|O_APPEND)) == -1)
	{
		ostringstream oss;
		oss<<"open(\"\"):"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	return redirect(this->to);
}

int 
CRedirect::redirect(int to)
{
	if (fds_bak != -1)
		close(fds_bak);
	if ((fds_bak = dup(from)) == -1)  // back up from fds;
	{
		ostringstream oss;
		oss<<"dup("<<from<<"):"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	if (-1 == dup2(to, from)) // data sent to from is redirect to "to"
	{
		ostringstream oss;
		oss<<"dup2("<<to<<", "<<from<<"):"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	return 0;
}

int 
CRedirect::restore()
{
	if (fds_bak == -1)
		return fds_bak;
	if (-1 == dup2(fds_bak, from)) 
	{
		ostringstream oss;
		oss<<"dup2("<<fds_bak<<", "<<from<<"):"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	if (-1 == close(fds_bak))
	{
		ostringstream oss;
		oss<<"close("<<fds_bak<<"):"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	fds_bak = -1;
	if (to != -1 && -1 == close(to))
	{
		ostringstream oss;
		oss<<"close("<<to<<"):"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	to = -1;
	return 0;
}
