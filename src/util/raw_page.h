#ifndef _PAT_RAW_PAGE_H_112904
#define _PAT_RAW_PAGE_H_112904
struct CRawPage_TW
{
	string version;
	string url;
	time_t date;
	string ip;
	int unzip_length;
	int length;       // 0 means non-zipped data.
	CHttpReply reply;
}
#endif // _PAT_RAW_PAGE_H_112904
