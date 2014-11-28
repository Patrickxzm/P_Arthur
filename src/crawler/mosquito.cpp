/*
 *  mosquito.cpp
 *  In shadow files are there the following items: 
 *	http body of script, http body of plain text
 *	localstr of selflinks, full url of outlinks
 *	anchor_text, paragraph,
 */
#include "mosquito.h"
#include "common.h"
#include "htmlstruct/htmlref.h"
#include "commu/tw_raw.h"
#include "util/xmlGet.h"
#include "util/util.h"
#include "common.h"
#include "htmlstruct/refresh.h"
#include <assert.h>
#include <stdexcept>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sstream>
#include <fstream>
#include <iostream>

using std::endl;
using std::flush;
using std::logic_error;
using std::runtime_error;
using std::ostringstream;
using std::ifstream;

namespace {
	class longer
	{
	public:
		bool operator()(const string &s1, const string &s2) const
		{		
			return s1.length() > s2.length();
		}
	};
}

COutputPart::COutputPart()
{
	rawOut.open(raw_file);
}

CHttpPart::CHttpPart()
 : retry_interval(600), keep_alive(0), robots("Googlebot")   // Sorry! Google.
 , fetch_result(RequestOK), nfetch(0), contFetchFailure(0), contPageError(0)
 , sleep_interval(0)
{
}

CStatPart::CStatPart()
 : old_total(0), old_try(0), old_get(0), old_changed(0)
 , new_total(0), new_try(0), new_get(0)
 , blink_total(0), blink_try(0), blink_get(0)
{
}

CMosquito::CMosquito()
  :fDebug(false)
{
	conn.timeout(45, 45);
	const char* fake_crawler_name = "Mozilla/5.0"
           "(compatible;Googlebot/2.1;+http://www.google.com/bot.html)";
	CHttpRequest::userAgent = fake_crawler_name;
}

CMosquito::~CMosquito()
{
}

#if 0
void 
CMosquito::loadEnv(const string &host, int port, const vector<CTask> &tasks, 
	const string &shadow_prefix, unsigned shadow_capacity,
	const string &cookie_file, unsigned npages, unsigned interval)
#endif //0
void
CMosquito::loadEnv(CEnvCrawler &env)
{
	this->host = env.host;
	this->port = env.port;
	this->my_log = env.my_log;
	this->my_error = env.my_error;
	shadows.open(env.shadow_prefix, env.shadow_capacity);
	shadow_size = shadows.size();
	//Should better check the shadow_size with the "task.xml" file.
	
	//get robots infomation before readin tasks
	if (my_log.get())
		(*my_log)<<"Get robots.txt... "<<flush;
	int res = robots.init(host, port);
	if (my_log.get())
	{
		if (res == 0)
			(*my_log)<<"OK."<<endl;
		else 
			(*my_log)<<"Failed."<<endl;
	}
	ifstream ifs(special_fn); // load host.special_branch in database.
	if (ifs)
		robots.import(ifs);
		
	for (unsigned i=0; i<env.tasks.size(); i++)
	{
		CTask task = env.tasks[i];
		// to get rid of "jsessionid=" in localstr;	
		CURL url(task.localstr);
		task.localstr = url.localstr();
		
		if (!robots.allow(task.localstr))
			continue;
		const string &localstr = task.localstr;
		if (queBlink.has(localstr) 
		   || queVisited.has(localstr) 
		   || queUnvisit.has(localstr)) 
			continue;
		// Skip task with wrong status
		if (task.status&CTask::Stale || task.status&CTask::Bad || task.status&CTask::Delete)
			continue;
		if (task.status & CTask::Unknown)
		{ // Unknown means "imported from other host" here.
			if (!shadows.put(task.localstr)) 
				continue; 		// discard urls visited
			task.status &= ~(CTask::Visited | CTask::Unknown);
		}
		shadows.put(task.localstr); // urls in "forgotten" memory are remembered here.
		if (task.status & CTask::Visited)
			queVisited.push_back(task);
		else if (task.status & CTask::Blink)
			queBlink.push_back(task);
		else
			queUnvisit.push_back(task);
	}
	if (cookie_file.size() > 0)
	{
		this->cookie_file = cookie_file;
		cookies.load(cookie_file.c_str());
	}
	this->npages = env.pageNum;
	this->interval = env.interval > robots.crawl_delay() 
	   ? env.interval : robots.crawl_delay();
	set<string>::const_iterator cit;
	for (cit=env.save_link_of.begin(); cit!=env.save_link_of.end(); cit++)
		outlink_save.insert(make_pair(*cit, outlink_save_t::mapped_type()));
	return;
}

#if 0 // moved to loadEnv(env)
bool
CMosquito::saveLinkOf(const string &host, int port)
{
	string hostport = host;
	if (port != 80)
		hostport += ':'+tostring(port);
	if (outlink_save.find(hostport) != outlink_save.end())
		return false;
	outlink_save[hostport];
	return true;
}
#endif //0

bool 
CMosquito::getTask(CTaskQue &blink, CTaskQue &visited, CTaskQue &unvisit)
{
	if (queBlink.pop_front(this->task))     
	{
		blink_total ++;
		return true;
	}
	else if (queVisited.pop_front(this->task))
	{
		old_total ++;
		return true;
	}
	else if (queUnvisit.pop_front(this->task))
	{
		this->task.status &= ~CTask::Unknown; //remove "Unknown" tag
		new_total ++;
		return true;
	}
	return false;
}

int
CMosquito::run()
{
	if (queBlink.empty() && queVisited.empty() && queUnvisit.empty())
	{
		if (my_error.get())
			(*my_error)<<"(host:"<<host<<")No seeds to start crawling."<<endl;
	}
	else if (!hostaddr.DNS(host.c_str()))
	{
		if (my_error.get())
			(*my_error)<<"DNS failed: "<<host<<endl;
		fetch_result = ServerFailure;
	}
	total_retry = 0;
	//ofstream rawOut(raw_file); 
	hubActive.open(hub_file, discard_file, overflow_file);
	int count_interval = 0;
	while (getTask(queBlink, queVisited, queUnvisit)) // ++"???_total"
	{
		CTask origin = this->task;
#if 0
	* 2011/01 refresh host news.cyu.edu.cn, we get the following result:
	* user.asp?* 404
	* ReadNews.asp?* 404
	* ... ... a lot of 404.
	* Because the host is changed totally to another template, 
	* so no ???_get is not a ServerFailure.
	****************************************************
		if (old_try+new_try+blink_try > 30 && old_get+new_get+blink_get==0)
			fetch_result = ServerFailure;
#endif 
		if (fetch_result == ServerFailure)
			break;
		if (total_retry > 20)  // cost too much time: 20*6 min = 2 hrs
			fetch_result = ServerDOS;
		if (blink_try+new_try+old_try >= npages*10)
			fetch_result = ServerBusy;
		if (!this->task.isReady()        // skip it to next turn.
		   || blink_get+new_get+old_changed>=npages  // enough pages
		   || fetch_result == ServerBusy
		   || fetch_result == ServerDOS 
		)
		{
			this->task.waited ++;
			hubActive.put(this->task, origin);
			continue;  
		}
		if (my_log.get()) 
                    (*my_log)<<this->task.localstr<<endl;
		this->current.init("http://"+host+":"+tostring(port)
		   +this->task.localstr);
		CHttpReply reply;
		reply.set_max_size(2000000);  // 2M maximim a page.
		fetch_result = fetch(reply, 3); // ++ "???_try"
	/*
         *  把多个interval集中在一起sleep
	 *  1、减少sleep系统调用数量和sleep信息的输出。
	 *  2、连续的fetch操作，有利于使用keep-alive connection。
	 *  3、爆发式的fetch操作，更象人使用浏览器时的习惯。
         */
		if (++count_interval >= sleep_interval)
		//if ((sum_interval+=interval) > interval * 40)
		{
			if (my_log.get())
 				(*my_log)<<"Sleep "<<count_interval*interval<<" seconds."<<endl;
 			conn.close();
 			sleep(count_interval*interval);
 			count_interval = 0;
 		}
		if (RequestOK != fetch_result)
		{
			this->task.reschedule(page_status_t());  
			hubActive.put(this->task, origin); 
			if (my_log.get())
				(*my_log)<<"Http fetch failed."<<endl;
			continue;
		}
		int reply_status = CAnalysisPart::check(reply);
		if (PAGE_UNWANTED==reply_status || PAGE_BAD==reply_status)
		{
			this->task.status |= CTask::Bad;
			hubActive.put(this->task, origin);
			if (my_log.get())
				(*my_log)<<"Bad page."<<endl;
			continue;
		}
		else if (PAGE_ERROR == reply_status)
		{
			if (contPageError ++ > 20)
				fetch_result = ServerDOS;
			this->task.reschedule(page_status_t());
			hubActive.put(this->task, origin);
			if (my_log.get())
				(*my_log)<<"DOS from server."<<endl;
			continue;
		}
		else if (PAGE_NOT_FOUND == reply_status 
		   || PAGE_GONE == reply_status) 
		{
			this->task.status |= CTask::Delete;	
			hubActive.put(this->task, origin);
			if (my_log.get())
				(*my_log)<<"Page deleted."<<endl;
			continue;
		}
		else if (reply_status==PAGE_OK || reply_status==PAGE_REDIRECT)
		{
			if (CTask::Visited & this->task.status)
				old_get++;
			else if (CTask::Blink & this->task.status)
				blink_get++;
			else
				new_get++;
		}
		assert(reply_status==PAGE_OK || reply_status==PAGE_REDIRECT);
		contPageError = 0;
		page_status_t page_status = parse(reply, reply_status);
		if (my_log.get())
		{
			if (this->task.status&CTask::Visited)
				(*my_log)<<"Refreshing, ";
			else if (this->task.status&CTask::Blink)
				(*my_log)<<"Blinking, ";
			else  
				(*my_log)<<"First crawling, ";
			if (page_status.newURL> 0)
				(*my_log)<<page_status.newURL<<" new urls found; ";
			if (page_status.newAnchor > 0)
				(*my_log)<<page_status.newAnchor<<" new anchors found; ";
			if (page_status.newParagraph > 0)
				(*my_log)<<page_status.newParagraph<<" new paragraph found; ";
			if (page_status.outlinkSaved > 0)
				(*my_log)<<page_status.outlinkSaved<<" outlinks saved; ";
			if (page_status.newScript)
				(*my_log)<<"javascript changed.";
			(*my_log)<<endl;
		}
		if (page_status.newContentNum()>0 
		   || !(this->task.status&CTask::Visited) )
			output_raw(reply, current.str(), hostaddr.paddr());
		if (page_status.newContentNum()>0 
		   && this->task.status&CTask::Visited)
			old_changed ++;
		this->task.reschedule(page_status);
		hubActive.put(this->task, origin); //++ _overflow and _discard
		if (my_log.get()) 
                    (*my_log)<<endl;
	}
	hubActive.close();
	if (cookie_file.size() > 0)
		cookies.save(cookie_file.c_str());
	return 0;
}


CMosquito::fetch_result_t 
CMosquito::fetch(CHttpReply &reply, int num_try)
{
	if (CTask::Visited & ((CMosquito*)this)->task.status) 
	{
		old_try ++;
	}
	else 
	{
		if (this->task.status & CTask::Blink)
			blink_try++;
		else
			new_try ++;
	}

	int fetch_ret[num_try];
	bool getReply = false;
	static string prev_localstr;
	for (int i=0; i<num_try && !getReply; i++)
	{
		if (i>0)
		{
			if (my_log.get())
				(*my_log)<<"wait for "<<retry_interval<<" seconds, retry... "<<endl;
			sleep(retry_interval);
			total_retry ++;
		}
		switch (fetch_ret[i]=fetch(reply))
		{
		case Reply_OK :
			getReply = true;
			if (i==0 && !prev_localstr.empty() && keep_alive==0)
				keep_alive = 1;
			if (i==1 && !prev_localstr.empty() && fetch_ret[0]==-2 
				&& keep_alive==0)
				keep_alive = -1;
			if (keep_alive==-1 && conn.is_open())
				conn.close();   // close() right after use.
			break;
		case Conn_Fail:
		case Trans_Fail:
		case Request_Fail:
		case DNS_Fail:	// impossible because hostaddr has been checked in run();
			hostaddr.next();
			if (conn.is_open())
				conn.close();	// close() right after use.
			break;
		default :
			assert(false);
		}
	}
	if (!getReply && my_error.get())
		(*my_error)<<" Failed to get page : "<<this->current.str()<<endl;
	prev_localstr = this->task.localstr;
	nfetch ++;
	if (getReply)
	{
		contFetchFailure = 0;
		return RequestOK;
	}
	else
	{
		contFetchFailure ++;
	}
	if (nfetch == 1)
		return ServerFailure;
	if (contFetchFailure >= 3)
		return ServerDOS;
	return RequestFailure;
}


int 
CMosquito::fetch(CHttpReply &reply)
{
	assert(!host.empty());
	if (!conn.is_open())
	{
		if (my_log.get()) (*my_log)<<"Connect "<<host<<':'<<port<<" (in "
		   <<conn.conn_timeout<<"*"<<hostaddr.naddr()<<" seconds)..."<<flush;
		switch(conn.ConnectServer(hostaddr, port))
		{
                case -1 :
			if (my_log.get()) (*my_log)<<"DNS failure."<<endl;
                        return DNS_Fail;
                case -2 :
			if (my_log.get()) (*my_log)<<"failed."<<endl;
                        return Conn_Fail; 
                case 0 :
                        if (my_log.get()) (*my_log)<<"("<<hostaddr.paddr()<<")OK."<<endl;
			break;
		default :
			assert(false);
                }
	}
	CHttpRequest request(this->current, "GET", &cookies, keep_alive!=-1);
	if (this->fDebug && my_log.get())
		(*my_log)<<request<<endl;	
	if (!(conn<<request<<flush))
	{
		if (my_log.get()) (*my_log)<<"Request failed."<<endl;
		return Request_Fail;
	}
	if (conn>>reply)
	{
		if (my_log.get()) (*my_log)<<"Get Reply."<<endl;
		vector<const string*> cookie_headers;
		cookie_headers = reply.headers.values("Set-Cookie");
		for (unsigned i = 0; i<cookie_headers.size(); i++)
			cookies.add(CCookie(*cookie_headers[i]));
		return Reply_OK;
	}
	else {
		if (my_log.get()) (*my_log)<<"Transfer failed."<<endl;
		return Trans_Fail;
	}
} 


bool
CMosquito::inQue(const string &localstr, unsigned check)
{
	if (check & is_new)
	{
		if (this->task.status & CTask::Visited)
			queBlink.push_back(CTask(localstr, 0, 0, CTask::Blink));
		else if (this->task.status & CTask::Blink)
			// "Unknown" marks that it may be blink
			queUnvisit.push_back(CTask(localstr, 0, 0, CTask::Unknown));
		else
			queUnvisit.push_back(CTask(localstr, 0, 0, 0));
		return true;
	}
	// url not new.
	CTask t(localstr);
	if (this->task.status&CTask::Visited 
	   && queUnvisit.find(t) 
	   && t.status&CTask::Unknown)
	{ // "Unknown" mark is used here
		queUnvisit.remove(t);
		t.status = CTask::Blink;
		queBlink.push_back(t);
		return true;
	}
	return false; 
}

void
CMosquito::collectURL(const xmlChar_ptr_vector *urls, unsigned &newURL
	, unsigned &outlinkSaved)
{
	for (size_t i=0; i<urls->size(); i++)
	{
		string urlstr = (const char*)urls->operator[](i);
		html_descape(urlstr);
		CHTMLItem::compress_blank(urlstr);
		CURL url(urlstr);
		if (!url.isAbs())
			url.toabs(this->current);
		unsigned r = check(url);
		if (!r&right_type)
			continue;
		if (r&same_hostport && inQue(url.localstr(), r))
		{
			newURL ++;
			if (fDebug && my_log.get())
				(*my_log)<<"\t*"<<url.localstr()<<"\n";
		}
		else if (!(r&same_hostport) && saveOutlink(url))
		{
			outlinkSaved ++;
			if (fDebug && my_log.get())
				(*my_log)<<"\t*"<<url.str()<<"\n";
		}
	}
	return;
}

page_status_t
CMosquito::parse(const CHttpReply &reply, int status)
{
	page_status_t result;
	//assert(PAGE_OK == status || PAGE_REDIRECT == status);
	result.getPage = true;
	if (status == PAGE_REDIRECT)
	{
		auto_ptr<xmlChar_ptr_vector> urls(new xmlChar_ptr_vector);
		string location = reply.headers.value("Location");
		urls->push_back(xmlCharStrdup(location.c_str()));
		collectURL(urls.get(), result.newURL, result.outlinkSaved);
		return result;
	}
	string type = reply.headers.value("Content-Type");
	if (type.find("javascript") != string::npos)
	{
		result.newScript = shadows.put(reply.body);
		return result;
	}
	else if (type.find("text/plain") != string::npos)
	{
		result.newPlain = shadows.put(reply.body);
		return result;
	}
	else if (type.find("text/html") == string::npos)
	{
		return result;
	}
	else if (reply.headers.content_disposition().type == "attachment")
	{
		// TODO: result.newAttachment = shadows.put(reply.body);
		return result;
	}
	else if (reply.body.empty())
		return result;

	// "text/html" type page:
	if (encoding.empty())
		encoding = reply.headers.charset();
	encoding = htmlFindEncoding2(reply.body.c_str(), reply.body.length()
				, encoding.c_str());
	if (encoding.empty() && my_error.get())
		(*my_error)<<"Document encoding Unknown: "
		   <<this->current.str()<<endl;
	// get xml Document
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	doc.reset(htmlReadMemory(reply.body.c_str(), reply.body.length()
	   , this->current.str().c_str(), encoding.c_str()
	   , HTML_PARSE_NOWARNING|HTML_PARSE_NOERROR|HTML_PARSE_RECOVER));
	if (!doc.get() && my_error.get())
		(*my_error)<<"Read html doc failed: "<<this->current.str()<<endl;
	if (!doc.get())
		return result;
	if ((!doc->encoding||xmlStrlen(doc->encoding)==0) && my_error.get())
	{
		(*my_error)<<"CharacterSet is Unknown: "<<this->current.str()
		   <<endl;
	}
	// Get XPath Context
	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> ctx;
	ctx.reset(xmlXPathNewContext(doc.get()));
	if (!ctx.get()) 
		throw runtime_error("Error in xmlXPathNewContext()");
	
	// links in <a>, <area> and <refresh>
	auto_ptr<xmlChar_ptr_vector> links;
	links.reset(xmlGetMultiStr(ctx.get(), BAD_CAST "//a/@href|//area/@href|"
                                  "//img/@data-original|//img/@src"));
	xmlChar_scoped_ptr content;
	content.reset(xmlGetStr(ctx.get()
	   , BAD_CAST "//meta[@http-equiv='refresh']/@content"));
	char* tmp = url_in_refresh((const char*)content.get());
	if (tmp != NULL)
	{
		links->push_back(xmlCharStrdup(tmp));
		free(tmp);
	}
	collectURL(links.get(), result.newURL, result.outlinkSaved);
	// anchor text in <a>
	if (encoding.size() > 0)
	{
		auto_ptr<xmlChar_ptr_vector> anchors;
		anchors.reset(xmlGetMultiStr(ctx.get(), BAD_CAST "//a"));
		result.newAnchor = collectAnchor(anchors.get());
		auto_ptr<xmlChar_ptr_vector> paras;
		paras.reset(xmlGetMultiStr(ctx.get(), BAD_CAST "//p|//li|//text()"));
		result.newParagraph = collectParagraph(paras.get());
	}
	// remember encoding of the previous page.
	if (doc->encoding && xmlStrlen(doc->encoding)>0)
		encoding = (char*)doc->encoding;
#if 0 // Use libxml2 instead of htmlstruct package.
	CHTMLRef htmlref(reply.body.c_str(), reply.body.length()
             ... ... ... ...
			result.outlinkSaved ++;
	}
#endif //0
	return result;
}

void
CMosquito::report() const
{
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	doc.reset(xmlNewDoc(BAD_CAST "1.0"));
	xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "CrawlerReport");
	xmlDocSetRootElement(doc.get(), root_node);

	xmlNewChild(root_node, NULL, BAD_CAST "host", BAD_CAST host.c_str());
	if (port != 80)
		xmlNewChild(root_node, NULL, BAD_CAST "port", BAD_CAST tostring(port).c_str());
	time_t now = time(0);
	xmlNewChild(root_node, NULL, BAD_CAST "issueTime", BAD_CAST ctime(&now));
	xmlNewChild(root_node, NULL, BAD_CAST "numIPAddr", BAD_CAST tostring(hostaddr.naddr()).c_str());
	const xmlChar * server_status;
	switch (fetch_result)
	{
	case ServerFailure:
		server_status = BAD_CAST "Failure";
		break;
	case ServerDOS:
		server_status = BAD_CAST "DOS";
		break;
	case ServerBusy:
		server_status = BAD_CAST "Busy";
		break;
	default :
 		server_status = BAD_CAST "OK";
		break;
	}
	xmlNewChild(root_node, NULL, BAD_CAST"ServerStatus", server_status);
	xmlNewChild(root_node, NULL, BAD_CAST"TotalRetry", BAD_CAST tostring(total_retry).c_str());
	xmlNewChild(root_node, NULL, BAD_CAST"NewlyShadowed"
	   , BAD_CAST tostring(shadows.size()-shadow_size).c_str());
	xmlNodePtr blink_url = xmlNewChild(root_node, NULL, BAD_CAST "BlinkURL", NULL);
	xmlNewChild(blink_url, NULL, BAD_CAST "GET", BAD_CAST tostring(blink_get).c_str());
	xmlNewChild(blink_url, NULL, BAD_CAST "TRY", BAD_CAST tostring(blink_try).c_str());
	xmlNewChild(blink_url, NULL, BAD_CAST "Discard", BAD_CAST tostring(hubActive.blink_discard).c_str());
	xmlNewChild(blink_url, NULL, BAD_CAST "Overflow", BAD_CAST tostring(hubActive.blink_overflow).c_str());
	xmlNewChild(blink_url, NULL, BAD_CAST "Total", BAD_CAST tostring(blink_total).c_str());
	xmlNodePtr old_url = xmlNewChild(root_node, NULL, BAD_CAST "OldURL", NULL);
	xmlNewChild(old_url, NULL, BAD_CAST "GET", BAD_CAST tostring(old_get).c_str());
	xmlNewChild(old_url, NULL, BAD_CAST "TRY", BAD_CAST tostring(old_try).c_str());
	xmlNewChild(old_url, NULL, BAD_CAST "Changed", BAD_CAST tostring(old_changed).c_str());
	xmlNewChild(old_url, NULL, BAD_CAST "Discard", BAD_CAST tostring(hubActive.old_discard).c_str());
	xmlNewChild(old_url, NULL, BAD_CAST "Overflow", BAD_CAST tostring(hubActive.old_overflow).c_str());
	xmlNewChild(old_url, NULL, BAD_CAST "Total", BAD_CAST tostring(old_total).c_str());
	xmlNodePtr new_url = xmlNewChild(root_node, NULL, BAD_CAST "NewURL", NULL);
	xmlNewChild(new_url, NULL, BAD_CAST "GET", BAD_CAST tostring(new_get).c_str());
	xmlNewChild(new_url, NULL, BAD_CAST "TRY", BAD_CAST tostring(new_try).c_str());
	xmlNewChild(new_url, NULL, BAD_CAST "Discard", BAD_CAST tostring(hubActive.new_discard).c_str());
	xmlNewChild(new_url, NULL, BAD_CAST "Overflow", BAD_CAST tostring(hubActive.new_overflow).c_str());
	xmlNewChild(new_url, NULL, BAD_CAST "Total", BAD_CAST tostring(new_total).c_str());
	ofstream outlinks(out_neighbor_file);
	if (!outlinks)
	{
		ostringstream oss;
		oss<<"Can not open output file:\""<<out_neighbor_file<<"\"";
		throw runtime_error(oss.str());
	}
	for (map<string, int>::const_iterator cit=outlink_count.begin(); cit!=outlink_count.end(); cit++)
	{
		outlinks<<host;
		if (port != 80)
			outlinks<<":"<<port;
		outlinks<<" ==> ";
		outlinks<<cit->first<<'\t'<<cit->second<<endl;	
	}
	for (outlink_save_t::const_iterator cit = outlink_save.begin()
		; cit != outlink_save.end(); cit ++)
	{
		if (cit->second.empty())
			continue;
		xmlNewChild(root_node, NULL, BAD_CAST"SavedLinkFile", BAD_CAST cit->first.c_str());
		ofstream ofs(cit->first.c_str());
		for (set<CTask, less_task>::const_iterator ccit=cit->second.begin()
		  ; ccit != cit->second.end(); ccit++)
			ofs<<*ccit<<endl;
	}

	xmlSaveFormatFileEnc(report_file, doc.get(), "UTF-8", 1);
	xmlMemoryDump();
	return;
}

int
COutputPart::output_raw(const CHttpReply &reply, const string &urlstr
   , const string &ipstr)
{
	if (reply.body.size() > 10000000) // more than 10MB
		return -5;
	CTWRaw raw(urlstr, ipstr, reply);
	if (!(rawOut<<raw<<endl))
		return -4;
	if (mc.get())
	{
		ostringstream oss;
		if (oss<<raw)
			mc->send(oss.str());
	}
	return 0;
}

unsigned
CMosquito::check(const CURL &newurl)
{
	unsigned r(0);
	media_t type = newurl.mtype();
	if (newurl.isAbs() && (type==m_text || type==m_image || type==m_unknown))
		r |= right_type;
	else
		return r;
	if (newurl.host()==current.host() && newurl.port()==current.port())
	{
		r |= same_hostport;
		if (!robots.allow(newurl.localstr()))
		{
			r &= ~right_type;
			return r;
		}
		if (shadows.put(newurl.localstr()))
			r |= is_new;
	} 
	return r;
}

bool
CMosquito::saveOutlink(const CURL &newurl)
{
	string hostport = newurl.host();
	if (newurl.port() != 80)
		hostport += ':'+tostring(newurl.port());		
	map<string, int>::iterator it = outlink_count.find(hostport);
	if (it==outlink_count.end())
		outlink_count[hostport] = 1;
	else 
		it->second ++;
	outlink_save_t::iterator save_it = outlink_save.find(hostport);
	if (save_it != outlink_save.end() && shadows.put(newurl.str()))
	{
		CTask t(newurl.localstr());
		t.status = CTask::Unknown;
		if (this->task.status & CTask::Visited)
			t.status |= CTask::Blink;
		save_it->second.insert(t);
		return true;
	}
	return false;
}

int 
CAnalysisPart::check(const CHttpReply &reply)
{
	int code = reply.status.code;
	if (code >= 300 && code < 400)
	{
		return PAGE_REDIRECT;
	}
	else if (code >= 200 && code < 300)
	{
		string type = reply.headers.value("Content-Type");
		if (type.find("text") != string::npos)
			return PAGE_OK;
	 	if (type.find("javascript") != string::npos)
			return PAGE_OK;
		if (type.find("image") != string::npos)
			return PAGE_OK;
		else
			return PAGE_UNWANTED;
	}
	else if (code == 400) 
		return PAGE_BAD;
	else if (code == 410)
		return PAGE_GONE; 
	else if (code == 404)
		return PAGE_NOT_FOUND;
	else 
		return PAGE_ERROR;
}

unsigned
CMosquito::collectAnchor(const xmlChar_ptr_vector *anchors)
{
	unsigned newAnchor = 0;
	for (size_t i=0; i<anchors->size(); i++)
	{
		string anchor_text = (const char*)anchors->operator[](i);
		html_descape(anchor_text);
		anchor_text = compress_blank(anchor_text);
		if (shadows.put(anchor_text))
		{
			newAnchor ++;
			if (fDebug && my_log.get())
				(*my_log)<<"\t+"<<anchor_text<<"\n";
		}
	}
	return newAnchor;
}

unsigned
CMosquito::collectParagraph(const xmlChar_ptr_vector *pgs)
{
	const unsigned maxNum = 20;
	const unsigned minLen = 80;
	
	vector<string> vParas;
	for (size_t i=0; i<pgs->size(); i++)
	{
		string pg = (const char*)pgs->operator[](i);
		html_descape(pg);
		pg = compress_blank(pg);
		if (pg.size() < minLen)
			continue;
		vParas.push_back(pg);
	}

	unsigned newPara = 0;
	std::sort(vParas.begin(), vParas.end(), longer());
	for (size_t i=0; i<vParas.size() && i<maxNum; i++)
	{
		string para = vParas[i];
		//html_descape(para);
		//para = compress_blank(para);
		if (shadows.put(para))
		{
			newPara ++;
			if (fDebug && my_log.get())
				*my_log<<"\t^"<<para<<"\n";
		}
	}
	return newPara;
}

void
CMosquito::readConfig(CXMLFile &xfile)
{
	string maddr, serv;
	if (xfile.getStr("/config/crawler/multicast/address", maddr)
	   && xfile.getStr("/config/crawler/multicast/port", serv))
		mc.reset(new CMultiCast(maddr, serv));
	int num;
	if (xfile.getInt("/config/crawler/sleep_interval", num))
		sleep_interval = num;
	return;
}

bool
CMosquito::setDebug(bool f)
{
	bool oldValue = fDebug;
	fDebug = f;
	return oldValue;
}
