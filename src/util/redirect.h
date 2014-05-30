#ifndef _PAT_REDIRECT_OUTPUT_H_07052011
#define _PAT_REDIRECT_OUTPUT_H_07052011

class CRedirect
{
public:
	CRedirect(int from);
	virtual ~CRedirect();
	int redirect(int to);
	int redirect(const char* fn);
	int restore();
private:
	int fds_bak;
	int from;
	int to;
	static int fds_null;
};

#endif // _PAT_REDIRECT_OUTPUT_H_07052011
