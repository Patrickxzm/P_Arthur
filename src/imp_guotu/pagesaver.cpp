/**************************************************************************
 * @ According to HLE's reply, make two changes: "time"-->"date"; "\n" 
 *   instead of "\r\n" to break header line.
 *					02/14/2004	16:46
 **************************************************************************/
#include "pagesaver.h"
#include "cutil.h"
#include "zlib.h"
#include <sstream>
#include <cstring>
#include <streambuf>
using std::ifstream;
using std::ostringstream;
using std::streambuf;
using std::cout;
using std::endl;

CPageSaver::CPageSaver() 
{
}

void
CPageSaver::deal(workbench_t &bench)
{
	if (bench.url == 0 || bench.reply == 0)
		return;
	const CHttpReply &reply = *bench.reply;
	const CURL &url = *bench.url;
	if (reply.status.code != 200 && reply.status.code != 203)
		return;
	media_t mt = reply.headers.mtype();
	if (mt == m_unknown)
		mt = url.mtype();
	switch (mt) {
	case m_text:
		if (!(ofs_page[text].handle.is_open()))
			ofs_page[text].handle.open("text", 20, 2, 100, false);
		save2(bench, ofs_page[text]);
		break;
	case m_pdf:
		if (!(ofs_page[pdf].handle.is_open()))
			ofs_page[pdf].handle.open("pdf", 100, 10, 100, false);
		save2(bench, ofs_page[pdf]);
		break;
	case m_ps:
		if (!(ofs_page[ps].handle.is_open()))
			ofs_page[ps].handle.open("ps", 50, 10, 100, false);
		save2(bench, ofs_page[ps]);
		break;
	case m_doc:
		if (!(ofs_page[doc].handle.is_open()))
			ofs_page[doc].handle.open("doc", 50, 10, 100, false);
		save2(bench, ofs_page[doc]);
		break;
	case m_image:
		if (!(ofs_page[image].handle.is_open()))
			ofs_page[image].handle.open("image",50,10, 100, false);
		save2(bench, ofs_page[image]);
		break;
	case m_video:
		if (!(ofs_page[video].handle.is_open()))
			ofs_page[video].handle.open("video",50,10,100,false);
		save2(bench, ofs_page[video]);
		break;
	case m_audio:
		if (!(ofs_page[audio].handle.is_open()))
			ofs_page[audio].handle.open("audio",50, 10, 100, false);
		save2(bench, ofs_page[audio]);
		break;
	default:
		break;
	}
	return;
}

bool
CPageSaver::save2(workbench_t &bench, ofstream_page &a_ofs_page)
{
	const CHttpReply &reply = bench.reply;
	const CURL &url = bench.url;
	h_ofstream &ofs = a_ofs_page.handle;
	if (++a_ofs_page.w_count > 1000) 
	{
		ofs.check();
		a_ofs_page.w_count = 0;
	}
	ofs<<"version: 1.0\n";
	ofs<<"url: "<<url.str()<<"\n";
	// rfc1123-date
	char buf[128];
	memset(buf,0,128);
	time_t  timev;
	time(&timev);
	strftime(buf, 128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&timev));
	ofs<<"date: "<<buf<<"\n";
	const char* addr;
	if ((addr = bench.host->printable_addr_used()) != 0)
		ofs<<"ip: "<<addr<<"\n";
	
	ostringstream oss;
	oss<<reply.status<<"\r\n";
	for (unsigned i = 0; i<reply.headers.size(); i++)
		oss<<reply.headers[i]<<"\r\n";
	string header = oss.str();
	int origin_len = header.size() + 2 + reply.body.size();
	ofs<<"unzip-length: "<<origin_len<<"\n";
	// 10 more bytes for overflow 
	char *original_body = (char*)malloc(origin_len*sizeof(char)+1);
	//char *original_body = (char*)malloc(origin_len*sizeof(char));
	header.copy((char*)original_body, header.size());
	strcpy(original_body+header.size(), "\r\n");
	reply.body.copy(original_body+header.size() + 2);
      	//Compressed the http-reply;
	uLong compressedLen = origin_len * 3 + 32;
	char *compressed_body = (char *)malloc(compressedLen);
	// unit change char to Byte.
	compressedLen = compressedLen * sizeof(char) / sizeof(Byte);
	origin_len = origin_len * sizeof(char) / sizeof(Byte);
	int err = compress((Byte*)compressed_body, &compressedLen, (Byte*)original_body, origin_len);
	if (err != Z_OK)
	{
		if (!errlog.is_open())
			errlog.open("SavePage.err");
		if (err == Z_MEM_ERROR)
		{
			errlog<<"Not enough memory!\n";
		}
		else if (err == Z_BUF_ERROR)
		{
			errlog<<"Not enough memory in the output buffer!\n";
		}
		errlog<<std::flush;
		compressedLen = 0;
	}
	ofs<<"length: "<<compressedLen<<"\n\n";
	ofs.write(compressed_body, compressedLen);
	ofs<<"\n"<<std::flush;
	free(compressed_body);
	free(original_body);
	return true;
}
