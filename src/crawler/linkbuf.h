#ifndef _PAT_LINKBUF_H_091104
#define _PAT_LINKBUF_H_091104

#include "qmmsg.h"
#include "h_ofstream.h"
#include "h_ifstream.h"

class CLinksBuffer
{
public:
	CLinksBuffer(const char* name = "link_buf");
	bool put(const CQMMsg &link);
	bool get(CQMMsg &link);
private:
	string filename;
	h_ifstream ifs;
	h_ofstream ofs;
	int out_n;
private:
};
#endif // _PAT_LINKBUF_H_091104
