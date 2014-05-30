#ifndef _PAT_MOSQUITO_H_04162008 
#define _PAT_MOSQUITO_H_04162008
#include <string>
#include <map>
#include "commu/http_reply.h"
#include "commu/cookie.h"
#include "util/index_queue.hpp"
#include "util/shadow.h"
#include "util/xmlfile.h"
#include "commu/TcpClient.h"
#include "commu/mcast.h"
#include "task_stream.h"
#include "shadow_chain.h"
#include "robots.h"
#include "page_status.h"
#include "envhost.h"

using std::greater;
using std::string;
using std::multimap;
using std::pair;
using std::auto_ptr;

typedef index_queue<CTask> CTaskQue;

class less_task 
{
public:
	bool operator()(const CTask &t1, const CTask &t2) const
	{
		return t1.localstr < t2.localstr;
	}
};

class COutputPart
{  
public:
	COutputPart();
	auto_ptr<CMultiCast> mc;
protected:
	int output_raw(const CHttpReply &reply, const string &urlstr, const string &ipstr);
	auto_ptr<ostream> my_error;
	auto_ptr<ostream> my_log;		
	typedef map<string, set<CTask, less_task> > outlink_save_t; 
	outlink_save_t outlink_save;
	CActiveTaskFile hubActive;
	map<string, int> outlink_count;
	ofstream rawOut;
};

class CAnalysisPart
{
public:
	enum {
		PAGE_OK, PAGE_REDIRECT, PAGE_BAD, PAGE_UNWANTED, PAGE_NOT_FOUND
		, PAGE_GONE, PAGE_ERROR
	};
	enum check_result_t {
		is_new=0x01 
		, right_type=0x02 
		, same_hostport=0x04
		//, 0x08, 
	};
	static int check(const CHttpReply &reply);
protected:
	string encoding;
};

class CHttpPart
{
public:
	CHttpPart();
	unsigned retry_interval;
protected:
	enum fetch_result_t
	{
		RequestOK=0,
		RequestFailure=-3,
		ServerFailure=-1,
		ServerDOS=-2,
		ServerBusy=-4
	};
	enum { 
		DNS_Fail, Conn_Fail, Request_Fail, Trans_Fail, Reply_OK
	};
protected:
	string host;
	int port;
	CHost hostaddr;
	CTCPClient conn;
	int keep_alive;  // 0: uncertain; 1: support; -1: not
	CRobots robots;
	fetch_result_t fetch_result;
	int nfetch, contFetchFailure, contPageError;
	int total_retry;
	int interval;
	int sleep_interval;
};

class CStatPart
{
public:
	CStatPart();
protected:
// for the host, of the pages;
	int old_total, old_try, old_get, old_changed;
	int new_total, new_try, new_get;
	int blink_total, blink_try, blink_get;
};

class CMosquito: public COutputPart, public CAnalysisPart
   , public CHttpPart, public CStatPart
{
public:
	class CEnvCrawler : public CEnv
	{
	public:
		vector<CTask> tasks;
		string shadow_prefix;
		unsigned shadow_capacity;
		string cookie_file;
		auto_ptr<ostream> my_log;
		auto_ptr<ostream> my_error;
		int retry_interval;
	};

	CMosquito();
	virtual ~CMosquito();
#if 0 //replaced
	void loadEnv(const string &host, int port, const vector<CTask> &tasks 
		, const string& shadow_prefix, unsigned shadow_capacity 
		, const string& cookie_file, unsigned npages, unsigned interval);
#endif //0
	void loadEnv(CEnvCrawler &env);
	//bool saveLinkOf(const string &host, int port=80);
	int run();
	void report() const;
	friend int mosquito_parser_t_main(int argc, char* argv[]);
	page_status_t parse(const CHttpReply &reply, int status);
	void readConfig(CXMLFile &xfile);
	bool setDebug(bool f);
private:
	void collectURL(const xmlChar_ptr_vector *urls, unsigned &newURL
	   , unsigned &outlinkSaved);
	unsigned collectAnchor(const xmlChar_ptr_vector *anchors);
	unsigned collectParagraph(const xmlChar_ptr_vector *pgs);
	bool inQue(const string &localstr, unsigned check);  
	bool saveOutlink(const CURL &newurl);
	//int save(const CHttpReply &reply, ostream &os);
	// return whether is_new, right_type, same_hostport
	unsigned check(const CURL &newurl); 
	bool getTask(CTaskQue &blink, CTaskQue &visited, CTaskQue &unvisit);
	bool isReady(CTask &task);
	bool exclude(const string &localstr);
	int fetch(CHttpReply &reply);
	fetch_result_t fetch(CHttpReply &reply, int num_try);
private:
	int npages;
	CShadowChain shadows;
	CTaskQue queBlink, queVisited, queUnvisit;
	CCookies cookies;
	string cookie_file;
	unsigned shadow_size;
	CURL current;
	CTask task;
	bool fDebug; 
};

#endif //_PAT_MOSQUITO_H_04162008

