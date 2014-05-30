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

CPageSaver::CPageSaver() : count(0)
{
}

void
CPageSaver::deal(workbench_t &bench)
{
	const CHttpReply &reply = bench.reply;
	const CURL &url = bench.url;
	if (reply.status.code != 200 && reply.status.code != 203)
		return;
	const string &fv = reply.headers.value("Content-Type");
	ofstream_page* ofs = 0;
	if (strcasestr(fv.c_str(), "text") != 0)
	{
		ofs = &(this->ofs[text]);
		if (!ofs->is_open())
			ofs->open("text", 20, 2, 100, false);
	}
	else if (strcasestr(fv.c_str(), "pdf") != 0)
	{
		ofs = &(this->ofs[pdf]);
		if (!ofs->is_open())
			ofs->open("pdf", 100, 10, 100, false);
	}
	else if (strcasestr(fv.c_str(), "postscript") != 0)
	{
		ofs = &(this->ofs[ps]);
		if (!ofs->is_open())
			ofs->open("ps", 50, 10, 100, false);
	}
	else if (strcasestr(fv.c_str(), "msword") != 0)
	{
		ofs = &(this->ofs[doc]);
		if (!ofs->is_open())
			ofs->open("doc", 50, 10, 100, false);
	}
	else 
	{
		return;
	}
	if (count++ % 1000 == 0)
		ofs->check();
	if (!(*ofs)) 
		return;
	(*ofs)<<"version: 1.0\n";
	(*ofs)<<"url: "<<url.str()<<"\n";
	// rfc1123-date
	char buf[128];
	memset(buf,0,128);
	time_t  timev;
	time(&timev);
	strftime(buf, 128, "%a, %d %b %Y %H:%M:%S GMT", gmtime(&timev));
	(*ofs)<<"date: "<<buf<<"\n";
	const char* addr;
	if ((addr = bench.host->printable_addr_used()) != 0)
		(*ofs)<<"ip: "<<addr<<"\n";
	
	ostringstream oss;
	oss<<reply.status<<"\r\n";
	for (unsigned i = 0; i<reply.headers.size(); i++)
		oss<<reply.headers[i]<<"\r\n";
	string header = oss.str();
	int origin_len = header.size() + 2 + reply.body.size();
	(*ofs)<<"unzip-length: "<<origin_len<<"\n";
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
	(*ofs)<<"length: "<<compressedLen<<"\n\n";
	(*ofs).write(compressed_body, compressedLen);
	(*ofs)<<"\n"<<std::flush;
	free(compressed_body);
	free(original_body);
	return;
}
