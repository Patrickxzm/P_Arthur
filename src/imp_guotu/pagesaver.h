#ifndef _PAT_PAGE_SAVER_H_021204
#define _PAT_PAGE_SAVER_H_021204
#include <fstream>
#include <string>
#include "dealer.h"
#include "h_ofstream.h"

typedef struct {
	h_ofstream handle;
	int w_count;
} ofstream_page;

class CPageSaver : public CDealer
{
public:
	//CPageSaver(const char* saved);
	CPageSaver();
	virtual void deal(workbench_t &bench);
private:
	bool save2(workbench_t &bench, ofstream_page &a_ofs_page);
	enum {text=0, pdf, ps, doc, image, video, audio, ntype=7};
	ofstream_page ofs_page[ntype];
	ofstream errlog;
};
#endif // _PAT_PAGE_SAVER_H_021204
