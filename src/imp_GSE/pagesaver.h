#ifndef _PAT_PAGE_SAVER_H_021204
#define _PAT_PAGE_SAVER_H_021204
#include <fstream>
#include <string>
#include "dealer.h"
#include "h_ofstream.h"

typedef h_ofstream ofstream_page;

class CPageSaver : public CDealer
{
public:
	//CPageSaver(const char* saved);
	CPageSaver();
	virtual void deal(workbench_t &bench);
private:
	enum {text=0, pdf, ps, doc, ntype=4};
	ofstream_page ofs[ntype];
	int count;
	ofstream errlog;
};
#endif // _PAT_PAGE_SAVER_H_021204
