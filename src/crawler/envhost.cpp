#include "envhost.h"
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <zlib.h>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include "common.h"
#include "url/url.h"
#include "util/xmlfile.h"
#include "util/shadow.h"
#include "util/util.h"

using std::ostringstream;
using std::istringstream;
using std::ifstream;
using std::ofstream;
using std::runtime_error;
using std::endl;
using std::flush;

using namespace mysqlpp;

#define MYSQLPP_TRY(action, sqlstr) \
	try { \
		action; \
	} \
	catch (Exception &e) \
	{ \
		ostringstream oss;\
		oss<<__FILE__<<":"<<__LINE__<<" "<<e.what()<<"\nSQL:"<<sqlstr;\
		throw runtime_error(oss.str());\
	}

//#define OLD_CTASK
//# CTask used to take a very complicated encoding for status, towait, waited... refer read_old()

const char* host_cols = "SELECT ID, name, port, zip_root, cookies, special_branch"
	", page_wanted, page_interval, crawler FROM host ";

Lock::Lock(Connection *c, const string& table)
{
	this->conn = c;
	Query q = conn->query();
	q<<"LOCK TABLES "<<table<<" WRITE";
	if (!q.exec())
	{
		ostringstream oss;
		oss<<"lock "<<table<<" table failed!";
		throw runtime_error(oss.str());
	}
}

Lock::~Lock()
{
	Query q = conn->query();
	if (!q.exec("UNLOCK TABLES"))
		throw runtime_error("unlock host table failed!");		
}

CHostTable::CHostTable(Connection *c) 
{
	conn = c;
	char* h = getenv("HOSTNAME");
	if (!h) throw runtime_error("Can not get env \"HOSTNAME\".");
	crawler = h;
}

int 
CHostTable::saveShadowChain(const string &path, const string& prefix, const string &host
   , unsigned port)
{
	// Parse task file
#if 0
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	doc.reset(xmlParseFile((path+"/"+task_file).c_str()));
	if (!doc.get())
		throw runtime_error("Can not open xml file: "+path+"/"+task_file);
	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> ctx;
	ctx.reset(xmlXPathNewContext(doc.get()));
	if (!ctx.get())
		throw runtime_error("xmlXPathNewContext() failed.");
#endif //0
	CXMLFile xfile((path+"/"+task_file).c_str());
	int nShadow;
	if (!xfile.getInt("//Shadows/Number", nShadow))
		throw runtime_error("No \"//Shadows/Number\" value in task file.");
	int last_ID, last_size;
	if (nShadow > 0)
	{
		if (!xfile.getInt("//LastOne/ID", last_ID))
			throw runtime_error("No \"//LastOne/ID\" value in task file.");
		assert(last_ID > 0);
		if (!xfile.getInt("//LastOne/size", last_size))
			throw runtime_error("No \"//LastOne/size\" value in task file.");
		assert(last_size > 0);
	}
	if (nShadow > 0)
	{
		string shadow_fn = path + "/" + prefix + "." + tostring(nShadow-1);
		CStrSetShadow shadow;
		unsigned capacity=0;
		if (shadow.open(shadow_fn.c_str(), capacity) != CStrSetShadow::Attach)
		{
			ostringstream oss;
			oss<<"Attach shadow \""<<shadow_fn<<"\" failed.";
			throw runtime_error(oss.str());
		}
		if ((int)shadow.size() > last_size)  // new items shadowed.
			saveShadow(shadow_fn.c_str(), last_ID, host, port);
	}
	for ( ;file_size(path+"/"+prefix+"."+tostring(nShadow)) >= 0; nShadow++)
		saveShadow((path+"/"+prefix+"."+tostring(nShadow)).c_str(), -1, host, port);
	return nShadow;
}

int 
CHostTable::saveShadow(const char* shadow_fn, int shadow_id, const string &host, unsigned port)
{
	CStrSetShadow shadow(0, shadow_fn);
	if (shadow.size() == 0)   // an empty shadow is useless.
		return -1;
	ifstream visited(shadow_fn);
	if (!visited)
	{
		ostringstream oss;
		oss<<"Can not open input file: "<<shadow_fn;
		throw runtime_error(oss.str());
	}
	ostringstream oss_visited;
	oss_visited<<visited.rdbuf();
	if (shadow_id > 0)
	{// update
		Query update = conn->query();
		update<<"UPDATE memory SET shadow="<<quote<<oss_visited.str()
			<<", size="<<shadow.size()
			<<" WHERE ID="<<shadow_id;
		if (!update)
		{
			ostringstream oss;
			oss<<"Build the update memory query failed, ID="<<shadow_id;
			throw runtime_error(oss.str());
		}
		SimpleResult res;
		MYSQLPP_TRY( res = update.execute(), update.str());
		if (!res)
		{
			ostringstream oss;
			oss<<"Memory table update failed, ID="<<shadow_id;
			throw runtime_error(oss.str());
		}
		return 0;
	}
	// insert
	Query insert = conn->query();
	insert<<"INSERT INTO memory(hostID, capacity, size, shadow,"
	   " create_time) ";
	insert<<"SELECT ID, "<<shadow.capacity()<<", "<<shadow.size()
	   <<", "<<quote<<oss_visited.str()<<", NOW()"
	   " FROM host WHERE name="<<quote<<host<<" AND port="<<port;
	if (!insert)
	{
		ostringstream oss;
		oss<<"Build the insert memory table query failed.";
		throw runtime_error(oss.str());
	}
	SimpleResult res;
	MYSQLPP_TRY(res = insert.execute(), insert.str());
	if (res.rows() != 1)
	{
		ostringstream oss;
		oss<<"Memory table insert failed.";
		throw runtime_error(oss.str());
	}
	return res.insert_id();
}

int
CHostTable::reportCrash(const string &hostport)
{
	if (!conn->connected())
		return -1;
	string host;
	int port;
	CURL::split("http", hostport, host, port);
	Query report = conn->query();
	report<<"UPDATE host SET crawler="<<quote<<crawler+":crashed"<<" WHERE "
	  "name="<<quote<<host<<" AND port="<<port;
	SimpleResult res;
	MYSQLPP_TRY(res = report.execute(), report.str());
	if (res.rows() == 1)
		return 0;
	else
		return -2;
}

int
CHostTable::saveResult(const string &path)
{
	if (!conn->connected())
		return -1;
	// Parse report file.
	CXMLFile xfile((path+"/"+report_file).c_str());
#if 0
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	doc.reset(xmlParseFile((path+"/"+report_file).c_str()));
	if (!doc.get()) 
		throw runtime_error("Can not open xml file: "+path+"/"+report_file);
	scoped_ptr4c<xmlXPathContext, xmlXPathFreeContext> ctx;
	ctx.reset(xmlXPathNewContext(doc.get()));
	if (!ctx.get())
		throw runtime_error("xmlXPathNewContext() failed.");
#endif //0

	string serverStatus;
	if (!xfile.getStr("//ServerStatus", serverStatus))
		throw runtime_error("No \"//ServerStatus\" value in report.");
	int newly_shadowed;
	if (!xfile.getInt("//NewlyShadowed", newly_shadowed))
		throw runtime_error("No \"//NewlyShadowed\" value in report.");
	int blink_get;
	if (!xfile.getInt("//BlinkURL//GET", blink_get))
		throw runtime_error("No \"//BlinkURL//GET\" value in report file.");
	int old_get;
	if (!xfile.getInt("//OldURL//GET", old_get))
		throw runtime_error("No \"//OldURL//GET\" value in report file.");
	int old_changed;
	if (!xfile.getInt("//OldURL//Changed", old_changed))
		throw runtime_error("No \"//OldURL//Changed\" value in report file.");
	int new_get;
	if (!xfile.getInt("//NewURL//GET", new_get))
		throw runtime_error("No \"//NewURL//GET\" value in report file.");
	string host;
	if (!xfile.getStr("//host", host))
		throw runtime_error("No host info in report file");
	int port;
	if (!xfile.getInt("//port", port))
		port = 80;

	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc1;
	doc1.reset(xmlParseFile((path+"/"+task_file).c_str()));
	if (!doc1.get())
		throw runtime_error("Can not open xml file: "+path+"/"+task_file);
	// ***** save visited memorys ********
	if (-1 == saveShadowChain(path, visited_prefix, host, port))
		return -4;
#if 0
	if (newly_shadowed > 0)
	{
		// Read latest shadow file into memory.
		string fn = path+"/"+visited_prefix+".latest";
		saveShadow(fn.c_str(), latest_shadow, host, port);
	}
#endif //0

	static cutem zip_buf;
	int zip_len=0;
// upload {vars ... } to database on {conditions ...}
	Query update = conn->query();
	update<<"UPDATE host SET name=name";
	// ****** page_num *********
	if (new_get+blink_get+old_changed)
		update<<"\n, page_num=page_num+"<<new_get+blink_get+old_changed;
	// ******* zip_root **********
	int towait_min = -1;  // means NULL value.
	string roots;
	if (serverStatus != "Failure")
	{
		// Read acitve hub file into memory.
		ifstream ifs_roots((path+"/"+hub_file).c_str());
		if (!ifs_roots)
			throw runtime_error("Can not open input file: "+path+"/"+hub_file);
		CTask task;
		while (ifs_roots >> task)
		{
			if (!(task.status & CTask::Visited))
				towait_min = 0;
			else if (task.towait <= task.waited)
				towait_min = 0;
			else if (towait_min == -1 || towait_min > task.towait - task.waited)
				towait_min = task.towait - task.waited;
		}	
		ifs_roots.clear();
		ifs_roots.seekg(0);
		ostringstream oss_roots;
		while (ifs_roots >> task)
		{
			task.waited += towait_min;
			oss_roots << task << endl;
		}
		roots = oss_roots.str();
		if (!roots.empty())
		{  
			zip_len=zip(roots.c_str(), roots.length(), zip_buf);
			if (zip_len < 0)
				return -2;
			update << "\n, zip_root="<<quote<<string(zip_buf.ptr(), zip_len);
		}
		else
		{
			update << "\n, zip_root=NULL";
		}
	}
	// *********towait_hour***************
	// very complex, here! :(, load_time - schedule_time + towait is the real time gap.
	if (old_changed == 0 && blink_get == 0 && old_get > 10) // slow re-visit
		update << "\n, towait_hour="
		  "(HOUR(TIMEDIFF(load_time, schedule_time))+towait_hour)*2";
	else if (old_changed > 7 || old_changed*5 > old_get || blink_get > 70) 
		// try speedup revisit
		update << "\n, towait_hour="
		  "IF(HOUR(TIMEDIFF(load_time, schedule_time)) > towait_hour"
		  ", towait_hour, (HOUR(TIMEDIFF(load_time, schedule_time))+towait_hour)/2)";
	// *********schedule_time**************
	if (!roots.empty() && serverStatus != "Failure")
	{ 
		assert(towait_min >= 0);
		update << "\n, schedule_time=NOW()+INTERVAL towait_hour * "<<towait_min+1<<" HOUR";
	}
	else if (serverStatus == "Failure")
	{
		int rd = random()% 24;
		update<<"\n, schedule_time=NOW()+INTERVAL POWER(2, failure_num)*(24+"
		  <<rd<<") HOUR";
	}
	else if (roots.empty())
	{
		update << "\n, schedule_time=NULL";
	}
	// ********* failure_time & failure_num********
	if (serverStatus == "Failure")
	{
		update<<"\n, failure_num=failure_num+1, failure_time="
		  "IF(failure_time is null, NOW(), failure_time)";
	}
	else
	{
		update<<"\n, failure_num=0, failure_time=NULL";
	}
	// ********crawler & load_time & refresh_num ********
	update<<"\n, crawler=NULL, load_time=NULL, refresh_num=refresh_num+1";
        // ****************** cookie file ****************
	if (file_size(path+"/"+cookie_fn) > 0)
	{
		ostringstream oss;
		ifstream ifs((path+"/"+cookie_fn).c_str());
		oss<<ifs.rdbuf();
		update<<"\n, cookies="<<quote<<oss.str();
	}
	// ***************** WHERE CLAUSE *****************
	update<<"\n WHERE name="<<quote<<host<<" AND port="<<port;
	SimpleResult res;
	MYSQLPP_TRY(res = update.execute(), update.str());
	if (res.rows() == 1)
		return 0;
	else
		return -3;
}

int 
CHostTable::selectStartEnv(vector<CEnv> &hosts, const string &up_dir)
{
	if (!conn->connected())
		return -1;
	StoreQueryResult res;
	{ // lock area
		Lock lock(conn, "host");
		Query select = conn->query(); 
		select<<host_cols
			<<"WHERE schedule_time<NOW() HAVING crawler IS NULL LIMIT 64";
		MYSQLPP_TRY(res = select.store(), select.str());
		if (!res || res.size() == 0)
			return -2;
		Query mark = conn->query();
		mark<<"UPDATE host SET crawler="<<quote<<crawler
		  <<", load_time=NOW()"
		  <<" WHERE false";
		for (unsigned i=0; i<res.size(); i++)
		{
			int host_id = res[i]["ID"];
			mark<<" OR ID="<<host_id;
		}
		SimpleResult res1;
		MYSQLPP_TRY(res1 = mark.execute(), mark.str());
		if (!res1)
		{
			ostringstream oss;
			oss<<"mark "<<res.size()<<" hosts failed.";
			throw runtime_error(oss.str());
		}
	}
	hosts.reserve(res.size());
	vector<int> fails;
	for (unsigned i=0; i<res.size(); i++)
	{
		string hostport = CURL::hostport("http", (string)res[i]["name"], (int)res[i]["port"]);
		string path;
		path = up_dir+"/"+hostport;
		if (0 != mkdir(path.c_str(), 0770) && errno != EEXIST)
		{
			ostringstream oss;
			oss<<"mkdir(\""<<path<<"\") failed: "<<strerror(errno);
			throw runtime_error(oss.str());
		}
		CEnv env;
		if (loadStartEnv(env, path, res[i]))
			hosts.push_back(env);
		else
			fails.push_back(res[i]["ID"]);
	}
	if (fails.empty())
		return 0;
	Query mark = conn->query();
	mark<<"UPDATE host set crawler="<<quote<<crawler+":loadFail"<<" WHERE false";
	for (unsigned i=0; i<fails.size(); i++)
		mark<<" OR ID="<<fails[i];
	SimpleResult res1;
	MYSQLPP_TRY(res1 = mark.execute(), mark.str());
	if (!res1)
	{
		ostringstream oss;
		oss<<"mark "<<res.size()<<" hosts failed.";
		throw runtime_error(oss.str());
	}
	return fails.size();
}

bool 
CHostTable::loadStartEnv(CEnv &env, const string &path, const string &host, int port, bool fMark)
{
	/*====================Read Database record =====================*/
	if (!conn->connected())
		return false;
	StoreQueryResult res;
	{ // lock area 
		Lock lock(conn, "host"); 
		Query select = conn->query();
		select<<host_cols<<"WHERE name="<<quote<<host<<" AND port="<<port;
		if (fMark)
			select<<" AND crawler is NULL";
		MYSQLPP_TRY(res = select.store(), select.str());
		if (!res || res.size() == 0)
			return false;
		if (fMark)
		{
			Query mark = conn->query();
			mark<<"UPDATE host SET crawler="<<quote<<crawler
			  <<", load_time=NOW()"
			  <<" WHERE name="<<quote<<host<<" AND port="<<port;
			SimpleResult res;
			MYSQLPP_TRY(res = mark.execute(), mark.str());
			if (!res)
			{
				ostringstream oss;
				oss<<"mark host \""<<host<<":"<<port<<"\" failed.";
				throw runtime_error(oss.str());
			}
		}
	}
	if (loadStartEnv(env, path, res[0]))
		return true;
	if (fMark)
	{
		Query mark = conn->query();
		mark<<"UPDATE host SET crawler="<<quote<<crawler+":loadFail"
		  <<" WHERE ID="<<res[0]["ID"];
		SimpleResult res;
		MYSQLPP_TRY(res = mark.execute(), mark.str());
		if (!res)
		{
			ostringstream oss;
			oss<<"load host \""<<host<<":"<<port<<"\" failed.";
			throw runtime_error(oss.str());
		}
	}
	return false;
}

int 
CHostTable::loadShadow(unsigned host_id, const string& path, xmlNodePtr shadow_node)
{
	Query select = conn->query();
	select<<"SELECT ID, capacity, size, shadow FROM memory WHERE hostID="<<host_id
		<<" ORDER BY create_time";
	UseQueryResult res;
	MYSQLPP_TRY( res = select.use(), select.str());

	int ID;
	unsigned size, capacity;
	Row row;
	int nShadow = 0;
	while (row=res.fetch_row())
	{
		size = row["size"];
		capacity = row["capacity"];
		ID = row["ID"];
#if 0
		if (nshadow==0 && size>=capacity)
		{// shadow overflow
			CStrSetShadow shadow;
			unsigned new_capacity = capacity < 640000 ? capacity*10 : capacity ;
			shadow.open((path+"/"+visited_prefix+".latest").c_str(), new_capacity
			   , CStrSetShadow::Create | CStrSetShadow::Overwrite);
			nshadow++;
		}
#endif //0
		// visited.0, visited.1, ..., visited.n, ...
		string fn = path+"/"+visited_prefix+"."+tostring(nShadow);
		ofstream ofs_visited(fn.c_str());
		if (!ofs_visited)
			throw runtime_error("Can not open shadow file: "+fn);
		const String &shadow = row["shadow"];
		if (shadow.length() != CStrSetShadow::memory_size(capacity))
			throw runtime_error("CHostTable::loadShadow():Inconsistent shadow size in database.");
		if (!ofs_visited.write(shadow.c_str(), shadow.size()))
			throw runtime_error("CHostTable::loadShadow():Can not write shadow file.");
 		nShadow++;
	}
	xmlNewChild(shadow_node, NULL, BAD_CAST "Number", BAD_CAST tostring(nShadow).c_str());
	if (nShadow > 0)
	{
		xmlNodePtr lastOne = xmlNewChild(shadow_node, NULL, BAD_CAST "LastOne", NULL);
		xmlNewChild(lastOne, NULL, BAD_CAST "ID", BAD_CAST tostring(ID).c_str());
		xmlNewChild(lastOne, NULL, BAD_CAST "capacity"
		   , BAD_CAST tostring(capacity).c_str());
		xmlNewChild(lastOne, NULL, BAD_CAST "size"
		   , BAD_CAST tostring(size).c_str());
	}
	string fn = path+"/"+visited_prefix+"."+tostring(nShadow);
	if (-1 == unlink(fn.c_str()) && errno != ENOENT)
	{
		ostringstream oss;
		oss<<"Can not delete terminating (null)shadow \""<<fn<<"\":"<<strerror(errno);
		throw runtime_error(oss.str());
	}
#if 0
	{// no shadow 
		CStrSetShadow shadow;
		shadow.open((path+"/"+visited_prefix+".0").c_str(), start_capacity
		   , CStrSetShadow::Create | CStrSetShadow::Overwrite);
		nshadow++;
	}
#endif // 0
	return nShadow;
}

bool 
CHostTable::loadStartEnv(CEnv &env, const string &path, const Row& host)
{
	assert(host);
	// ************************ Create task file *************
	scoped_ptr4c<xmlDoc, xmlFreeDoc> doc;
	doc.reset(xmlNewDoc(BAD_CAST "1.0"));
	xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST "CrawlerTask");
	xmlDocSetRootElement(doc.get(), root_node);
	
	xmlNodePtr shadow_node = xmlNewChild(root_node, NULL, BAD_CAST "Shadows", NULL);
	/*============= Load visited shadow =============================*/
	//unsigned start_capacity = 640 > host["page_wanted"]*10 ? 640 : host["page_wanted"]*10;
	//int latest_id, nshadow;
	//nshadow = loadShadow((unsigned)host["ID"], latest_id, path, start_capacity);
	loadShadow((unsigned)host["ID"], path, shadow_node);
	/*=================Load lpaths file for root urls =================*/
	ofstream ofs_roots((path+"/"+root_file).c_str());
	if (!ofs_roots)
	{
		ostringstream oss;
		oss<<"CHostTable::loadStartEnv():Cann't open output file(\""
		  <<path<<"/"<<root_file<<"\"):"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	String zip_root = host["zip_root"];
	if (zip_root.is_null())
		ofs_roots<<CTask("/", 0, 0, CTask::Unknown)<<std::endl;
	else
	{
		int unzip_size;
		try {
			unzip_size = unzip(zip_root.c_str(), zip_root.length()
					, unzip_buf);
		}
		catch (std::exception &e)
		{
			return false;
		}
		if (!ofs_roots.write(unzip_buf.ptr(), unzip_size))
		{
			ostringstream oss;
			oss<<"CHostTable::loadStartEnv():Can not write root file:"
			  <<strerror(errno);
			throw runtime_error(oss.str());
		}
	}
	//***************** write cookie file ********************
	String cookies = host["cookies"];
	if (!cookies.is_null())
	{
		ofstream cookie_file((path+"/"+cookie_fn).c_str());
		if (!cookie_file)
		{
			ostringstream oss;
			oss<<"CHostTable::loadStartEnv():Cann't open output file(\""
			  <<path<<"/"<<cookie_fn<<"\"):"<<strerror(errno);
			throw runtime_error(oss.str());
		}
		cookie_file<<cookies<<flush;
	}
	//***************** write exclude file ********************
	String excludes = host["special_branch"];
	if (!excludes.is_null())
	{
		ofstream exclude_file((path+"/"+special_fn).c_str());
		if (!exclude_file)
		{
			ostringstream oss;
			oss<<"CHostTable::loadStartEnv():Cann't open output file(\""
			  <<path<<"/"<<special_fn<<"\"):"<<strerror(errno);
			throw runtime_error(oss.str());
		}
		exclude_file<<excludes<<flush;
	}
	// ******************************************************************
	env.host = host["name"].c_str();
	env.port = host["port"];
	env.pageNum = host["page_wanted"];
	env.interval = host["page_interval"];
	Query select = conn->query();
	select<<"SELECT host.name as host, host.port as port FROM trans_link, host"
	  " WHERE trans_link.target=host.ID AND trans_link.source="<<host["ID"];
	StoreQueryResult res;
	MYSQLPP_TRY(res = select.store(), select.str());
	for (unsigned i=0; i<res.size(); i++)
		env.save_link_of.insert(CURL::hostport("http", (string)res[i]["host"], (int)res[i]["port"]));
	// ************************ Write task file to disk *************
	xmlSaveFormatFileEnc((path+"/"+task_file).c_str(), doc.get(), "UTF-8", 1);
	xmlMemoryDump();
	return true;
}


int
CHostTable::import(const CHost4Import &rec, ostream *report, bool force)
{
	int hostID;
	if (report)
		(*report)<<"import \""<<rec.host<<"\" ... "<<flush;
	int res = import(rec.host, rec.port, hostID, force);
	if (res == -2)
		throw runtime_error("CHostTable::import():\"INSERT INTO site ...\" failed");
	else if (res == -4)
		assert(false);
		//throw runtime_error("CHostTable::import():\"UPDATA site SET host_num ...\" failed");
	else if (res == -5)
		throw runtime_error("CHostTable::import():\"INSERT INTO host ...\" failed");
	else if (res == -3)
	{
		if (report)
			(*report)<<"failed, limited by maximum hostnum of site."<<endl;
		return -3;
	}
	else if (res == -1)
	{
		if (report)
			(*report)<<"already exists("<<hostID<<")."<<endl;
	}
	else 
	{
		if (report)
			(*report)<<"OK("<<hostID<<")."<<endl;
	}
	if (rec.seeds.size() > 0)
		moreRoot(hostID, rec.seeds, false, report);
	//insert trans_link record.
	for (unsigned i=0; i<rec.trans_link.size(); i++)
		transLink(hostID, rec.trans_link[i], report);
	return 0;
}

const int default_max_host_num=20;

int
CHostTable::import(const string &host, int port, int& ID, bool force)
{
	// Check whether host exists.
	Query host_exist = conn->query();
	host_exist << "SELECT ID FROM host"
	  " WHERE name="<<quote<<host<<" AND port="<< port;
	StoreQueryResult res;
	MYSQLPP_TRY(res=host_exist.store(), host_exist.str());
	if (res.size() > 0)
	{
		ID = res[0]["ID"];
		return -1; 
	}
	// read site from db
	string site = CURL::host2site(host);
	Query test_site = conn->query();
	test_site <<"SELECT ID, max_host_num FROM site WHERE site.name="<<quote<<site;
	int siteID=0, max_host_num=0;
	StoreQueryResult resSite;
	MYSQLPP_TRY(resSite=test_site.store(), test_site.str());
	if (resSite.size() == 0)
	{// site not exist
		Query insertSite = conn->query();
		insertSite<<"INSERT INTO site(name, max_host_num) VALUES"
		   "("<<quote<<site<<", "<<default_max_host_num<<")";
		SimpleResult res;
		MYSQLPP_TRY(res=insertSite.execute(), insertSite.str());
		if (!res) 
			return -2;
		siteID = insertSite.insert_id();
		max_host_num = default_max_host_num;
	} 
	else {
		siteID = resSite[0]["ID"];
		max_host_num = (int)resSite[0]["max_host_num"];
	}
	// count hostnum of this site
	Query count_hostnum = conn->query();
	count_hostnum << "SELECT COUNT(*) as host_num FROM host WHERE siteID="<<siteID;
	StoreQueryResult resHost;
	MYSQLPP_TRY(resHost=count_hostnum.store(), count_hostnum.str());
	// check host_num against site's max_host_num. 
	if ((int)resHost[0]["host_num"] >= max_host_num && !force)
		return -3;
	
	//
	// Insert host
	Query insertHost = conn->query();
	insertHost<<"INSERT INTO host(name, port, siteID";
	insertHost<<", schedule_time) "
	  "VALUES("<<quote<<host<<", "<<port<<", "<<siteID;
	insertHost<<", NOW() )";
	SimpleResult res_insertHost;
	MYSQLPP_TRY(res_insertHost=insertHost.execute(), insertHost.str());
	if (!res_insertHost)
		return -5;
	ID = insertHost.insert_id();
	return 0;
}

int
CHostTable::moreRoot(int ID, const vector<pair<string, CTask::status_type> > &seeds
	   , bool create, ostream *report)
{
	if (seeds.empty())
		return 1;
	if (report)
		(*report)<<"\tmoreRoot... "<<flush;
	ostringstream roots;
	cutem buf;
	int r0 = 0;
	if (!create)
	{ // load out first ... 
		Query select = conn->query();
		select<<"SELECT zip_root FROM host WHERE ID="<<ID;
		StoreQueryResult res;
		MYSQLPP_TRY(res = select.store(), select.str());
		if (!res || res.size() == 0)
			return -1;
		String zip_root = res[0]["zip_root"];
		if (!zip_root.is_null())
		{
			int unzip_size = unzip(zip_root.c_str(), zip_root.length(), buf);
			istringstream iss(string(buf.ptr(), unzip_size));
			CTask task;
#ifdef OLD_CTASK
			while (read_old(iss, task))
#else
			while (iss >> task)
#endif 
			{
				r0 ++;
				roots<<task<<endl;
			}
			if (r0 == 0 && unzip_size>0)
				throw runtime_error("Read task from zip_root in wrong format");
		}
	}
	if (!roots) 
		throw runtime_error("write roots(ostringstream) failed!");
	for (unsigned i=0; i<seeds.size(); i++)
	{
		CTask t(seeds[i].first, 0, 0, seeds[i].second);
		roots<<t<<endl;
	}
	if (report)
	{
		if (!create)
			(*report)<<r0<<'+';
		(*report)<<seeds.size()<<"..."<<flush;
	}
	// write back.
	Query update = conn->query();
	update<<"UPDATE host SET zip_root=";
	if (roots.str().empty())
	{
		update<<"NULL";
	}
	else
	{
		int zip_len;
		zip_len = zip(roots.str().c_str(), roots.str().length(), buf);
		if (zip_len < 0)
		{
			if (report)
				(*report)<<"zip failed."<<endl;
			return -3;
		} 
		update<<quote<<string(buf.ptr(), zip_len);
	}
	update<<"\n, schedule_time=IF(schedule_time IS NULL, NOW(), schedule_time)"
	  <<" WHERE ID="<<ID;
	SimpleResult res;
	MYSQLPP_TRY(res = update.execute(), update.str());
	if (res.rows() != 1)
	{
		if (report)
			(*report)<<"sql operation failed."<<endl;
		return -4;
	}
	if (report)
		(*report)<<"OK"<<endl;
	return 0;	
}

int 
CHostTable::transLink(int ID, const string &target, ostream *report)
{
	if (report)
		(*report)<<"transfer link to "<<target<<" ..."<<flush;
	string host;
	int port;
	CURL::split("http", target, host, port);
	Query select = conn->query();
	select<<"SELECT ID FROM host WHERE name="<<quote<<host<<" AND port="<<port;
	StoreQueryResult res;
	MYSQLPP_TRY(res = select.store(), select.str());
	if (!res || res.size() < 1)
	{
		if (report)
			(*report)<<"target doesn't exist."<<endl;
		return -2;
	}
	int targetID = res[0]["ID"];
	if (report)
		(*report)<<targetID<<"."<<flush;
	Query insert = conn->query();
	insert<<"INSERT INTO trans_link(source, target) VALUES("<<ID<<", "<<targetID<<")"
	  " ON DUPLICATE KEY UPDATE source=source"; 
	SimpleResult res_insert;
	MYSQLPP_TRY(res_insert=insert.execute(), insert.str());
	if (res_insert.rows() == 1)
	{
		if (report)
			(*report)<<"OK"<<endl;
		return 0;
	}
	else
	{
		if (report)
			(*report)<<"Duplicated"<<endl;
		return -1;
	}
}

