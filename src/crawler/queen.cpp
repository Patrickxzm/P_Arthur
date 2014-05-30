/*****************************************************************************
 * @ merge urltab into hostports(implemented by Wang, Donghai). 
 * 						07/29/2004	21:26	
 * @ merged postponed's function to hostports.
 * 						06/18/2004	11:10
 * @ To enable termination of queen, add two arguments to send_mosquitos.
 *						05/26/2004	21:50
 * @ Forgive king when he say "group size=1", just do nothing.
 *						04/19/2004	16:24
 * @ In monolithic crawling system, site2Queen map is useless.
 *						04/15/2004	20:37
 * @ If a queen want to use green_path, she had to provide a network device
 *   name, so I add an option "-gDEV".
 *						04/14/2004	11:29
 * @ If the qualify of a hostport need not dns, do it in queen instead of her
 *   child process to avoid temporary sub-directory under data/.
 *						03/20/2004	23:32
 * @ request_king take an argument "rnum" for request url number, and return the
 *   actual url number recieved from king, and rnum is not 200 as it was.
 *						03/20/2004	21:47
 * @ Non-distributed queen should use non-block msgq to get message from 
 *   mosquitos.					03/20/2004	21:29
 * @ Rename "checklist" to "postponed", --when can not issue a mosquito for 
 *   a few urls on a hostport(already a mosquito or no free mosquito), postpone
 *   the issue of the mosquito.			03/20/2004	16:01 
 * @ Add "-i" option to enable incremental crawling.
 *						03/19/2004	14:42
 * @ Because you have to register the hostport when inserting a url to urltab,
 *   can not use "urltab operater<< file" to load urls in a file. Instead a 
 *   load_urls() method is added.		03/19/2004	14:41
 * @ Use a set as checklist, in stead of a queue.
 *						03/10/2004	22:53
 * @ urltab often consumes a lot of memory, so I change the policies of its 
 *   output: if urltab is too big or we can issue more mosquitos, we draw out
 *   a urls out, and mark CMosquitos' entry to send_mosquito later. 
 *						03/09/2004	20:55
 * @ Add a new type CQMMsg for reporting denied hostport to queen from her 
 *   child.					03/09/2004	00:32
 * @ Usefull infomation about a hostport is stored in CMosquitos, if a hostport
 *   has been assigned to queen, his urls will not be hand in to king; if it's
 *   denied, urls will be write to a "miss" file.
 *						03/09/2004	00:30
 * @ To test whether a host is in a range file, CXHostFiter has to request
 *   DNS sometimes, and it is very time-consuming. So the test is putted onto
 *   a task instead of a url. When pushing a url to urltab or to King, the 
 *   test is omited; when get a new task from the urltab, the test is performed.
 *   						03/07/2004	11:36
 * @ To reduce king' load, urls discoverd by queen is filtered by range and 
 *   deny. The filter is used two times: one is when dispatched a url, one 
 *   is when discoverd a url.  Suggested by HLE.
 *  						03/03/2004	20:39 
 * @ Suggested by sh, msgq_path is added a pid suffix, so there can be several
 *   queens running on a machine.		03/01/2004	18:15
 * @ Command line option for depth is provided with "-d" kicking off another
 *   useless option.				02/21/2004	11:04
 * @ When working with a king, the range and seeds file for queen is disabled.
 *   When working alone, a deny file is added.
 *		some changes made, see  Comment 03/03/2004	20:39 
 *						02/20/2004	22:27
 * @ "to_mos()" is renamed "mos_option()", and can be several options seperated
 *   by space.					02/20/2004	21:45
 * @ Add entry to prepare for a task before issueing it to a mosquito. I 
 *   named it CServant, the servent can send a option to mosquito with 
 *   "to_mos()" method.				02/11/2004	16:49 
 * @ Crawling depth should be limited, or mosquitos will be sucked in by huge
 *   website, a monster. Defaultly, the depth limit is 10. Command line option
 *   is not provided.
 *						01/09/2004	17:28
 * @ If a task out from CURLTab has not been dispatched to the queen, it will 
 *   be send to king in distributed system, but dispatched to queen in 
 *   monolithic system.
 *						01/08/2004	18:21
 * @ Only CMosquitos contain the infomation about hostports dispatched to the
 *   Queen.					01/08/2004	17:36
 * @ Add an option to specify incremental crawling mosquitos: "-i"
 *						01/08/2004	10:54
 * @ If queen run in a distributed system, she use her msgq in a non-block
 *   way.					01/05/2004	17:42
 * @ When a mosquito exit, the entry in CMosquitos is not erased, just set the
 *   pid to -1. Refer 01/03/2004 17:31 comment in "mosquitos.h" for more info.
 *						01/03/2004	17:35
 * @ Add an option to specify queen's name for distributed crawling system. 
 * @ A "range" file is needed in distributed crawling system.
 *						01/03/2004	16:44
 * @ In distributed crawling system, queen need to communiate with a KING. Add
 *   a option "-kADDRESS:PORT" to specify the KING.
 *						01/02/2004	11:12
 * @ Mosquito changed its msg format, so queen need make change too. Refer to
 *   11/22 comment in "mosquito.cpp" for details.
 *						11/22/2003	19:11
 * @ Add a new argument to queen: "-pPRIO", to set mosquito's scheduling 
 *   priority, from -20 to 19, lower priorities means more favorable scheduling.
 *						10/22/2003	18:05
 * @ A queen of hundreds of mosquito. She sends her children(some mosquitos)
 *   to collect the Web.       			*longlong ago
 ****************************************************************************/
#include "url.h"
#include "queen.h"
#include "arg.h"
#include "qmmsg.h"
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <iostream>
#include <errno.h>

const int default_bandwidth = 2;
int CQueen::pid;

CQueen::CQueen(): max_nmos(20), mos_prio(0), depth(10)
	, nref_send(0), bandwidth(default_bandwidth)
	, green_path(0), send_kill(false), very_busy(false)
	, link_buffer("link_buffer")
{
	// modified by sh
        char buf[16];
        sprintf(buf, "%d", getpid());
        msg_path = "/tmp/queen_msg";
        msg_path += buf;
	msgq.init(1000, 256, msg_path.c_str());
	msgq.set_block(false);
	missed.open(".missed", std::ios::app|std::ios::out);

	// watch exit() for debug.
	pid = getpid();
	//atexit(debug_exit);
}

CQueen::~CQueen()
{
	if (green_path != 0)
		delete green_path;
	missed.close();
	pid = 0;
}

void
CQueen::debug_exit()
{
	if (getpid() == pid)
	{ // in parent process
		ostringstream oss;
		oss<<"touch debug."<<getpid();
		system(oss.str().c_str());
		sleep(80000);
	}
}

void
CQueen::cleardir(const string& hostport)
{
	string dir = hostports.getHpParentDir(hostport);
	dir += hostport;
	string cpfrom = dir + "/refs";
	ifstream ifs(cpfrom.c_str());
	if (ifs)
		missed_links<<ifs.rdbuf(); 
	ifs.close();

	ostringstream oss;
	oss<<"rm -rf "<<dir;	
	system(oss.str().c_str()); 
/*
	if (0 != unlink(cpfrom.c_str()))
	{
		ostringstream oss;
		oss<<"cleardir():unlink():"<<cpfrom
			<<':'<<strerror(errno);
		//throw QueenErr(oss.str());
	}
	unlink((dir+"/debug").c_str());
	if (0 != rmdir(dir.c_str()))
	{
		ostringstream oss;
		oss<<"cleardir():rmdir():"<<dir
			<<':'<<strerror(errno);
	//	throw QueenErr(oss.str());
	}
*/
}

int
CQueen::read_args(int argc, char** argv)
{
	CArg arg(argc, argv);
	CArg::ArgVal av;
	if (!arg.find1("-h").null() || !arg.find1("--help").null())
		return -1;
	/******************************/
	if (!(av=arg.find1("-n")).null())
	{
		max_nmos = av;
	}
	/******************************/
	if (!(av=arg.find1("-i")).null())
		inc_config = (string)av;
	/******************************/
	if (!(av=arg.find1("-g")).null())
		net_dev = string(av);
	/******************************/
	if (!(av=arg.find1("-d")).null())
		depth = av;
	/******************************/
	if (!(av=arg.find1("-b")).null())
		bandwidth = av;
	else
		bandwidth = default_bandwidth;
	/******************************/
	if (!(av=arg.find1("-p")).null())
		mos_prio = av;
	/******************************/
	if (!(av=arg.find1("-e")).null())
	{
		unsigned hash_size = av;
		struct hp_env env;
		hostports.getEnv(env);
		env.dirHashSize = hash_size;
		hostports.setEnv(env);
	}
	/******************************/
	// There may be a switch to turn off the log utility.
	queen_log.open("queen_log");
	/******************************/
	if (!arg.find1("--save-links").null())
	{
		missed_links.open("missed_links", 
			env.missed_links_size_limit, 
			env.missed_links_size_limit/10, 100);
	#ifdef _DEBUG
		cout<<"open missed_links."<<endl;
	#endif //_DEBUG
	}
	/******************************/
	distributed = false;
	if (!arg.find1("-k").null())
	{
		distributed = true;
		const char* king = av;
		char* ptr = strchr(king, ':');
		string address;
		int port = 0;
		if (ptr != 0)
		{
			address.assign(king, ptr - king);
			ptr++;
			sscanf(ptr, "%d", &port);
		}
		else {
			address.assign(king);
		}
		if (port <=0 )
			port = 5031;
		if (client.ConnectServer(address.c_str(), port).empty())
			throw QueenErr(string("read_args():ConnectServer():")
				+strerror(errno));
	} 
	/******************************/
	if (!(av=arg.find1("-q")).null())
		name = string(av);
	else if (distributed)
		throw QueenErr("read_args():In distributed system, "
			"You must specify Queen-name.");
	/******************************/
	init();
	return 0;
}

void
CQueen::init()
{
	int ret = hostfilter.init("range");
	if (ret==-1)
	{
		ostringstream oss;
		oss<<"read_args()::hostfilter:init():"
			"Open range file failed!";
		throw QueenErr(oss.str());
	}
	else if (ret>0) 
	{
		ostringstream oss;
		oss<<"read_args()::hostfilter:init():"
			"format error in range:"
			<<" Line."<<ret;
		throw QueenErr(oss.str());
	}
	else if (ret < -1)
	{
		ostringstream oss;
		oss<<"read_args()::hostfilter:init():"
			"range file too large!";
		throw QueenErr(oss.str());
	}
	ret = deny.init("deny");
	if (ret==-1)
	{
		// "deny" file is optional.
	}
	else if (ret>0) 
	{
		ostringstream oss;
		oss<<"read_args()::deny:init():"
			"format error in range:"
			<<" Line."<<ret;
		throw QueenErr(oss.str());
	}
	else if (ret < -1)
	{
		ostringstream oss;
		oss<<"read_args()::deny:init():"
			"deny file too large!";
		throw QueenErr(oss.str());
	}
	else if (ret == 0)
		;
	
	/******************************/
	if (!distributed) 
	{
		load_seeds("seeds");
	}
	/******************************/
	hostports.load();
	//hostports.load(".missed");
	load(site2Queen, ".site2Queen");
	return;
}

void
CQueen::load_seeds(const char* file)
{
	if (file == 0)
		return;
	ifstream seeds(file);
	string urlstr;
	while (seeds>>urlstr) 
	{
		CURL url(urlstr);
		if (url.protocol() !=  "http" || url.host().empty())
			continue;
		CQMMsg msg;
		msg.compose_REFER("http://net.pku.edu.cn/~xzm/", urlstr, "");
		accept(msg, url);
        }
}

void
CQueen::do_link_from_sister(const CQMMsg &qmmsg)
{
	string urlfrom, urlto, anchor;
	QM_MsgType type = qmmsg.getType();
	if (type == QM_REFER)
		qmmsg.retrieve_REFER(urlfrom, urlto, anchor);
	else if (type == QM_REDIRECT)
		qmmsg.retrieve_REDIRECT(urlfrom, urlto);
	CURL url(urlto);
	hash_map_str2str::const_iterator iter = site2Queen.find(url.site());
	if (iter == site2Queen.end() || iter->second != name)
	{
		forward2king(qmmsg);
	}
	else
	{
		accept(qmmsg, url);
	}
}

unsigned 
CQueen::hear_sisters()
{
	CQMMsg qmmsg;
	if (green_path == 0)
		return 0;
	int count = 0;
	while (green_path->recv(qmmsg))
	{
		count ++;
		QM_MsgType type = qmmsg.getType();
		if (type == QM_REFER || type == QM_REDIRECT)
		{
			do_link_from_sister(qmmsg);		
		}
		else 
		{
			ostringstream oss;
			oss<<"hear_sisters(): recv_msg, type="<<type<<':'
				<<qmmsg.getMsg();
			throw QueenErr(oss.str());
		}
	}
	return count;
}

unsigned
CQueen::request_king()
{
	unsigned count = 0;
	unsigned n=0;
	while ((n = request_king(n=100)) == 100)
	{
		count += n;
	}
	count += n;
	return count;
}

void
CQueen::do_link_from_king(const CLineMessage &lmsg) 
{
	int type = lmsg.getMsgType();
	string urlfrom, urlto, anchor;
	if (type==LM_REFER)
		lmsg.retrieveMsg_REFER(urlfrom, urlto, anchor);
	else if (type == LM_REDIRECT)
		lmsg.retrieveMsg_REDIRECT(urlfrom, urlto);
	else
	{
		ostringstream oss;
		oss<<"do_link():type="<<type;
		throw QueenErr(oss.str());
	}
	CURL url(urlto);
	if (urlfrom.empty())
		urlfrom = "http://net.cs.pku.edu.cn/~xzm/";
	string hostport = url.hostport();
	CQMMsg qmmsg;
	if (type==LM_REFER)
		qmmsg.compose_REFER(urlfrom, urlto, anchor);
	else 
		qmmsg.compose_REDIRECT(urlfrom, urlto);
	site2Queen[url.site()] = name;
	accept(qmmsg, url);
}

void
CQueen::group(const CLineMessage &lmsg)
{
	unsigned int size;
	lmsg.retrieveMsg_GROUP(size);
	if (size < 2)
	{
		return;
		ostringstream oss;
		oss << "request_king():retrieveMsg_GROUP():"
			<<"size="<<size;
		throw QueenErr(oss.str());
	}
	queen_log<<"build green_path(size="
		<<size<<")... ..."<<flush;
	int ret;
	if (green_path == 0)
	{
		green_path = new CGreenPath;
		ret = green_path->build(size, net_dev.c_str(), name.c_str());
	}
	else 
	{
		ret = green_path->rebuild(size);
	}
	if (ret == 0)
		queen_log<<"OK"<<endl;
	else 
		queen_log<<"failed"<<endl;
	return;
}


unsigned
CQueen::request_king(unsigned rnum)
{
	CLineMessage request;
	request.composeMsg_REQUEST(rnum);
	client<<request.getMsgString()+'\n';
	queen_log<<"request_king()"<<'\n';
	unsigned nmsgs = 0;
	CLineMessage lmsg;
	while (true)
	{
		string reply;
		if (!getline(client, reply) || reply[reply.length()-1]!='\n')
		{ // Network failed or king defunct.
			throw QueenErr("request_king():brecv():"
				"Can not get a message line.\n");
		}
		string msgstr(reply, 0, reply.length() -1);
		lmsg.setMsgString(msgstr);
		int type = lmsg.getMsgType();
		if (type==LM_REFER || type==LM_REDIRECT)
		{
			nmsgs++;
			do_link_from_king(lmsg);
		}
		else if (type == LM_LEFT)
		{
			client.clear();
			queen_log<<'\n';
			return nmsgs;
		}
		else if (type == LM_ASSIGN)
		{
			nmsgs++;
			string queen, site;
			lmsg.retrieveMsg_ASSIGN(site, queen);
			site2Queen[site] = queen;
		}
		else if (type == LM_GROUP)
		{
			nmsgs++;
			group(lmsg);
		}
	}
}

bool CQueen::terminated;

void
CQueen::sig_term(int signo)
{
	printf("caught SIGTERM by %d\n", getpid());
	terminated = true;
	return;
}

bool
CQueen::do_terminated()
{
	if (!send_kill)
	{
		killpg(0, SIGTERM);
		send_kill = true;
		term_timer = time(0);
		max_nmos = 0;
	}
	if (mosquitos.size()==0)
	{
		queen_log<<"Received SIGTERM, and no active mos."
			<<endl;
		return true;
	}
	if (time(0) - term_timer < 3600)
		return false;
	CMosquitos::iterator iter;
	for (iter=mosquitos.begin(); iter != mosquitos.end(); iter++)
	{
		int status;
		waitpid(iter->first, &status, 0);
		hostports[iter->second] |= (hp_tested | hp_qualified);
	}
	return true;
}

void 
CQueen::run()
{
	const int len = 1000;
	char cwd[len+1];
	if (getcwd(cwd, len) == 0)
	{
		ostringstream oss;
		oss<<"run():getcwd():"<<strerror(errno);
		throw QueenErr(oss.str());
	}
	workdir = cwd;	
	if (signal(SIGTERM, sig_term) == SIG_ERR)
	{
		ostringstream oss;
		oss<<"run():signal():can't catch SIGTERM:"<<strerror(errno);
		throw QueenErr(oss.str());
	}
	terminated = false;
	if (-1==mkdir("mos", 0755) && errno!=EEXIST)
		throw QueenErr(string("run()::mkdir():mos")+':'
			+strerror(errno));

	time_t tm = time(0);
	queen_log<<ctime(&tm);
	queen_log<<"Run model:\n";
	queen_log<<"distributed : "<<distributed<<endl;
	queen_log<<"incremental config file: "<<inc_config<<endl;
	queen_log<<"mosquitos' crawling depth : "<<depth<<endl;
	queen_log<<"netdevice for green_path : "<<net_dev<<endl;
	queen_log<<"working directory : "<<workdir<<endl;

	if (distributed)
	{
		CLineMessage hello;
		if (net_dev.empty())
			hello.composeMsg_HELLO(name, false);
		else 
			hello.composeMsg_HELLO(name, true);
		client<<hello.getMsgString()+'\n';
		CLineMessage load;
		load.composeMsg_LOAD(max_nmos-mosquitos.size()
			, mosquitos.size());
		client<<load.getMsgString()+'\n';
		request_king();
	}

	string msg;
	while (1)
	{
		int every = 0;
		int remain;
		while(msgq.get(msg, &remain) == 0 || link_buffer.popALine(msg))
		{ // busy status */
			if ((double)remain/env.msgq_size > env.very_busy_ratio)
				very_busy = true;
			else
				very_busy = false;
			do_msg(msg);
			every++;
			if(every % 10000 == 0 && distributed)
				request_king();
			if (every % 1000 == 0 && distributed)
				hear_sisters();
			if (every % 10000 == 0)
			{
				missed_links<<flush;
				missed_links.check();
			#ifdef _DEBUG
				cout<<"Check missed_links."<<endl;
			#endif //_DEBUG
			}
			if (terminated && do_terminated())
				break;
		}
		/* lazy status */
		if (distributed)
		{
			request_king();
			hear_sisters();
		}
		send_mosquitos();
		if (terminated && do_terminated())
			break;
		//sleep(20);
		sleep(2);
	}
	if (distributed)
	{
		CLineMessage byebye;
		byebye.composeMsg_BYEBYE();
		client<<byebye.getMsgString()+'\n';
	}
	dump(site2Queen, ".site2Queen");
	hostports.dump();
	queen_log<<"queen exit!\n";
}

void 
CQueen::load(hash_map_str2str &__map, const char* file)
{
	ifstream ifs(file);
	string str1, str2;
	while (ifs>>str1>>str2)
	{
		__map[str1] = str2;
	}
	return ;
}

void 
CQueen::dump(const hash_map_str2str &__map, const char* file)
{
	ofstream ofs(file);
	for (hash_map_str2str::const_iterator iter = __map.begin();
		iter != __map.end();
		iter ++ )
	{
		ofs<<iter->first<<'\t'<<iter->second<<endl;
	}
	return;
}

void 
CQueen::check_status(const char* file)
{
	ofstream ofs(file);
	if (!ofs)
		return;
	ofs<<"mosquitos:"<<mosquitos.size()<<"(size)"<<'\n';
	ofs<<"hostports:"<<hostports.size()<<"(size)"<<'\n';
	ofs<<"site2Queen:"<<site2Queen.size()<<"(size)"<<'\n';
	ofs<<"nConns:"<<nConns.size()<<"(size)"<<'\n';
	ofs<<"mos2stick:"<<mos2stick.size()<<"(size)"<<'\n';
	return;
}

void
CQueen::send_mosquitos()
{
	string hostport;
	int i=0;
	while (mosquitos.size() < max_nmos && hostports.get(hostport) == 0)
	{
		hp_status status;
		if (!hostports.getStatus(hostport, status))
		{
			ostringstream oss;
			oss<<"Send_mosquitos():Cann't getStatus!:"<<hostport;
			throw QueenErr(oss.str());
		}
		if ((status & hp_tested) && !(status & hp_qualified))
		{
			cleardir(hostport);
		}
		else
		{
			send_mosquito(hostport);
		}
		if (++i == 5) 
			break;
	}
	return;
}

void 
CQueen::add_servant(CServant *servant)
{
	if (servant == 0)
		return ;
	servants.push_back(servant);
}

void
CQueen::mk_mos_link()
{
	const int len = 1000;
	char cwd[len+1];
	if (getcwd(cwd, len) == 0)
	{
		ostringstream oss;
		oss<<"send_mosquito():getcwd():"<<strerror(errno);
		throw QueenErr(oss.str());
	}
	ostringstream oss;
	oss<<workdir<<"/mos/"<<getpid();
	string slink = oss.str();
	if (-1 == unlink(slink.c_str()) && errno != ENOENT)
	{
		ostringstream oss;
		oss<<"send_mosquito():unlink():"<<slink
			<<':'<<strerror(errno);
		throw QueenErr(oss.str());
	}
	if (0 != symlink(cwd, slink.c_str()))
	{
		ostringstream oss;
		oss<<"send_mosquito():symlink():"<<slink
			<<"--->"<<cwd<<':'<<strerror(errno);
		throw QueenErr(oss.str());
	}
	return; 
}

int 
CQueen::send_mosquito(const string &hostport)
{
	int ret = 0;
	hostports.go2dir(hostport);
	// xzm... check the refs file.
	ifstream ifs("refs");
	string test;
	if ((!ifs || !(ifs>>test) || test.empty()) && inc_config.empty())
	{// "refs" is on the way to disk (IMPOSSIBLE)
	 //	or deleted by mosquito already.
		ret = 0;
	}
	else 
	{
		ret = __send_mosquito(hostport);
	}
	if (-1 == chdir(workdir.c_str()))
	{
		ostringstream oss;
		oss<<"send_mosquito()::chdir():"<<workdir<<':'
			<<strerror(errno);
		throw QueenErr(oss.str());
	}
	return ret;
}

int 
CQueen::__send_mosquito(const string &hostport)
{
	// control the speed of issuing child processes.
	/*
	time_t now = time(0);
	if (last_fork_time == now)
	{
		sleep(1);
		last_fork_time = time(0);
	}
	else
		last_fork_time = now;
	*/

	hp_status status;
	hostports.getStatus(hostport, status);
	pid_t pid;
	if ((pid=fork())==0) 
	{//child
		msgq.set_block(true);
		mk_mos_link();
		string host;
		int port;
		string __hostport = hostport;
		string::size_type pos = __hostport.find(':');
        	if (pos == string::npos) // not fount!
		{
			host = __hostport;
			port = 80;
		}
		else
		{
			__hostport[pos] = ' ';
			istringstream iss(__hostport);
			iss>>host>>port;
		}
		CHost dns_host(host); 
		CHostports::const_iterator citer = hostports.find(hostport);
		if (citer == hostports.end())
		{
			throw QueenErr("send_mosquito()::hostport not found!");
		}
		hp_status status = citer->second;
		if (!(status & hp_tested) &&
			(!hostfilter.pass(host.c_str(), &dns_host) 
			|| deny.pass(host.c_str(), &dns_host) )
		)
		{
			CQMMsg miss;
        		miss.compose_BYEBYE(getpid(), hp_tested
				, numeric_limits<time_t>::max()); // never.
                	msgq.put(miss.getMsg());
			exit(-1);
		}
		const char* ip = dns_host.printable_addr();
		if (ip != 0)
		{
			ostringstream oss;
			oss<<ip<<':'<<port;
			hash_map_str2int::const_iterator it;
			it = nConns.find(oss.str());;
			if (it == nConns.end() || it->second < 10)
			{
				CQMMsg stick;
				stick.compose_STICK(ip, port, getpid());
				msgq.put(stick.getMsg());
			}
			else
			{
				CQMMsg byebye;
        			byebye.compose_BYEBYE(getpid()
					, hp_url_ready|hp_tested|hp_qualified
					, time(0) + 3600); //an hour later.
               			msgq.put(byebye.getMsg());
				exit(-1);
			}
		}
		//debug << "setpriority()" <<endl<<flush;
		if (setpriority(PRIO_PROCESS, 0, mos_prio) != 0) 
		{
			throw QueenErr(string("send_mosquito()::setpriority()")
				+strerror(errno));
		}
		vector<string> _argv;
		_argv.push_back("mosquito");
		//_argv.push_back("mosquito");
		_argv.push_back("-rrefs");
		_argv.push_back("-m"+msg_path);
		{
			ostringstream oss;
			oss<<"-b"<<bandwidth;
			_argv.push_back(oss.str());
		}
		if (!inc_config.empty())
		{
			_argv.push_back("-i../../"+inc_config);
		}
		if (depth>=0)
		{
			ostringstream oss;
			oss<<"-d"<<depth;
			_argv.push_back(oss.str());
		}
		for (unsigned i=0; i<servants.size(); i++)
		{
			const char* option = servants[i]->mos_option(hostport);
			if (option != 0 && strlen(option)>0) 
			{
				istringstream iss(option);
				string opt;
				while (iss>>opt)
					_argv.push_back(opt);
			}
		}
		char* argv[_argv.size()+1];
		for (unsigned i=0; i<_argv.size(); i++)
		{
		// Ignore "const"! I think it will be OK.
			argv[i] = (char*)_argv[i].c_str();
		}
		argv[_argv.size()] = 0;
		if (execvp("mosquito", argv) == -1)
		{
			queen_log<<"send_mosquito()::execvp()"
				<<strerror(errno)<<'\n';
		}
		CQMMsg byebye;
        	byebye.compose_BYEBYE(getpid()
			, hp_url_ready|hp_tested|hp_qualified
			, time(0)+60*30); // half an hour later.
               	msgq.put(byebye.getMsg());
		exit(-1);
	}
	if (pid>0) 
	{//parent
		status |= hp_active;
		hostports.setStatus(hostport, status);
		mosquitos[pid] = hostport;
		if (distributed)
		{
			CLineMessage load;
			load.composeMsg_LOAD(max_nmos-mosquitos.size()
				, mosquitos.size());
			client<<load.getMsgString()+'\n';
		}
		time_t tm = time(0);
		queen_log<<ctime(&tm)<<':';
		queen_log<<"send a mosquito-->"<<hostport
			<<"(pid)"<<pid<<endl;
		return pid;
	}
	//error! 
	queen_log << "Error!!! Send_mosquito()::fork() : "
		<< strerror(errno) << std::endl;
	//throw QueenErr(string("send_mosquito()::fork()")
	//	+strerror(errno));
	return -1;
}

void
CQueen::accept(const CQMMsg &qmmsg, const CURL &url)
{
	if (very_busy) 
	{
		link_buffer.pushALine(qmmsg.getMsg().c_str());
		return;
	}
	string hostport = url.hostport();
	hp_status status;
	if (!hostports.getStatus(hostport, status))
	{
		if (quick_miss(url.host().c_str()))
		{
			hostports.putHostport(hostport, 0xFFFFFFFF, hp_tested);
			if (missed_links<<qmmsg.getMsg()<<'\n')
			{
			#ifdef _DEBUG
				cout<<"write missed_links OK!"<<endl;
			#endif //_DEBUG
			}
			else
			{
			#ifdef _DEBUG
				cout<<"write missed_links failed!"<<endl;
			#endif //_DEBUG
			}
		}
		else
		{
			hostports.putHostport(hostport);
			hostports.put(qmmsg);
		}
		return;
	}
	if ((status & hp_tested) && !(status & hp_qualified))
	{
		if (missed_links<<qmmsg.getMsg()<<'\n')
		{
		#ifdef _DEBUG
			cout<<"write missed_links OK!"<<endl;
		#endif //_DEBUG
		}
		else
		{
		#ifdef _DEBUG
			cout<<"write missed_links failed!"<<endl;
		#endif //_DEBUG
		}
		return;
	}
	hostports.put(qmmsg);
	return;
}

void
CQueen::do_link_from_mos(const CQMMsg &qmmsg) 
{
	string urlfrom;
	string urlto;
	string anchor;
	QM_MsgType type = qmmsg.getType();
	if (type == QM_REFER)
		qmmsg.retrieve_REFER(urlfrom, urlto, anchor);
	else if (type == QM_REDIRECT)
		qmmsg.retrieve_REDIRECT(urlfrom, urlto);
	else 
	{
		ostringstream oss;
		oss<<"do_link_from_mos():CQMMsg:type="<<type;
		throw QueenErr(oss.str());
	}
	CURL url(urlto);
	if ("http"!=url.protocol() || url.host().empty())
		return ; //I do not know this url.
	if (!distributed)
	{
		accept(qmmsg, url);
	}
	// distributed
	hash_map_str2str::const_iterator iter = site2Queen.find(url.site()); 
	if (iter == site2Queen.end())
	{
		forward2king(qmmsg);
	}
	else if (iter->second == name)
	{
		accept(qmmsg, url);
	}
	else if (green_path == 0)
	{
		forward2king(qmmsg);
		request_king();
	}
	else if (!green_path->send(qmmsg, iter->second))
	{
		forward2king(qmmsg);
	}
}

bool
CQueen::quick_miss(const char *host)
{
	if (strlen(host) > 50) 
		return true;
	if (!deny.need_dns(host) && deny.pass(host))
		return true;
	if (!hostfilter.need_dns(host) && !hostfilter.pass(host))
		return true;
	return false;
}

void
CQueen::do_mos_exit(const CQMMsg &qmmsg)
{
	time_t tm = time(0);
	queen_log<<ctime(&tm)<<':';
	queen_log<<"msg:"<<qmmsg.getMsg()<<endl;
	QM_MsgType type = qmmsg.getType();
	hp_status report;
	time_t retry_time;
	pid_t pid;
	if (type == QM_BYEBYE)
		qmmsg.retrieve_BYEBYE(pid, report, retry_time);
	else 
	{
		ostringstream oss;
		oss<<"do_mos_exit():type="<<type;
		throw QueenErr(oss.str()); 
	}
	if (mosquitos.count(pid) == 0)
	{
		throw QueenErr("do_mos_exit():hostport's pid not Found!");
	}
	string hostport = mosquitos[pid];
	mosquitos.erase(pid);
	hash_map_int2str::iterator it_stick = mos2stick.find(pid);
	if (it_stick != mos2stick.end())
	{ // the mosquito sticked on a hostport successfully.
		string stick = it_stick->second;
		mos2stick.erase(it_stick);
		hash_map_str2int::iterator it_nConns = nConns.find(stick);
		if (it_nConns == nConns.end())
		{
		// debug {
			queen_log<<"Debug me, please! pid="<<getpid()
				<<endl<<flush;
			sleep(100000);
		// } debug
			throw QueenErr("do_mos_exit():Can not get nConns.");
		}
		if (it_nConns->second < 1)
		{
			throw QueenErr("do_mos_exit(): nConns is less than 1 ???");
		}
		if ((it_nConns->second --) == 1)
		{
			nConns.erase(it_nConns);
		}
	}
	int status;
	waitpid(pid, &status, 0);
	ostringstream __slink;
	__slink<<"mos/"<<pid;
	string slink = __slink.str();
	if (0 != unlink(slink.c_str()))
	{
		ostringstream oss;
		oss<<"do_mos_exit():unlink():"<<slink
			<<':'<<strerror(errno);
		throw QueenErr(oss.str());
	}
	
	hp_status __status;
	if (!hostports.getStatus(hostport, __status))
	{
		ostringstream oss;
		oss<<"do_mos_exit():status not Found!:"<<hostport;
		throw QueenErr(oss.str());
	}
	if (!(__status & hp_active))
	{
		ostringstream oss;
		oss<<"do_mos_exit():He is not active but want to exit!:"
			<<hostport<<'('<<__status<<')';
		throw QueenErr(oss.str());
	}
	__status |= report;
	__status &= ~hp_active;
	if ((__status & hp_tested) && !(__status & hp_qualified))
	{
		cleardir(hostport.c_str());
		missed << hostport <<'\t'<<hex<<(int)__status<<endl;
	}
	hostports.setStatus(hostport, __status);
	if (retry_time == -1)
		retry_time = numeric_limits<time_t>::max();
	hostports.setRefreshTime(hostport, retry_time);

	if (distributed)
	{
		CLineMessage load;
		load.composeMsg_LOAD(max_nmos-mosquitos.size()
			, mosquitos.size());
		client<<load.getMsgString()+'\n';
	}
	queen_log<<"defunct mosquito-->"<<hostport
		<<"(pid)"<<pid<<':'<<status<<endl;
	send_mosquitos();
	return;
}

void
CQueen::forward2king(const CQMMsg &qmmsg)
{
	CLineMessage lmsg;
	string urlfrom, urlto, anchor;
	QM_MsgType type = qmmsg.getType();
	if (type == QM_REFER)
	{
		qmmsg.retrieve_REFER(urlfrom, urlto, anchor);
		lmsg.composeMsg_REFER(urlfrom, urlto, anchor);
	}
	else if (type == QM_REDIRECT)
	{
		qmmsg.retrieve_REDIRECT(urlfrom, urlto);
		lmsg.composeMsg_REDIRECT(urlfrom, urlto);
	}
	else
	{
		ostringstream oss;
		oss<<"forward2king(): Can not forward this type of message:"
			<< type;
		throw QueenErr(oss.str());
	}
	client<<lmsg.getMsgString()+'\n';
}

void 
CQueen::do_msg(const string &msgstr)
{
	CQMMsg qmmsg;
	QM_MsgType type;
	switch (type = qmmsg.setMsg(msgstr))
	{
	case QM_BYEBYE:
		do_mos_exit(qmmsg);
		break;
	case QM_REFER:
	case QM_REDIRECT: 
		do_link_from_mos(qmmsg);
		break;
	case QM_STICK:
		do_mos_stick(qmmsg);
		break;
	default: 
		break;
	}
}

void
CQueen::do_mos_stick(const CQMMsg &qmmsg)
{
	QM_MsgType type = qmmsg.getType();
	if (type != QM_STICK)
	{
		ostringstream oss;
		oss<<"do_mos_stick():type="<<type;
		throw QueenErr(oss.str()); 
	}
	string ip;
	int port;
	pid_t pid;
	qmmsg.retrieve_STICK(ip, port, pid);
	ostringstream oss;
	oss<<ip<<':'<<port;
	hash_map_str2int::iterator it = nConns.find(oss.str());
	if (it == nConns.end())
	{
		nConns[oss.str()] = 1;
	}
	else
	{
		it->second ++;
	}
	mos2stick[pid] = oss.str();
	return;
}

void 
CQueen::help()
{
	cout<<"Queen kernel : Crawl the web. \n"
	 	<<"[-ndbpekqig] [--save-links] [-h|--help] \n"
	"Usage: \n"
	"-nNUM: issue NUM mosquitos to Crawl the web. (20: default).\n"
	"-dDEPTH: "
	"Specify the crawling depth of her mosquitos.\n"
	"-bBandwidth: "
	"Specify the Bandwith consumed every mosquito. (KB/s, 2 default)\n" 
	"-pPRIO: "
	"Set mosquito's priority when issuing it."
	"(PRIO, range:[-20, 19), "
	"the bigger the number, the lower scheduling favorate.\n" 
	"-eSIZE: Specify the hashsize of the environment dirs.\n"
	"-kADDRRESS[:PORT]: There is a king at ADDRESS:PORT, port has"
	" a default value 5031.\n"
	"-qName: Told KING the queen's name. \n"
	"-iCONFIG: Enable incremental crawling, and specify the config file. \n" 
	"-gDEV: Specify the network device for green_path.\n"
	"--save-links: Save missed links. \n"
	"-h or --help: Print this message.\n";
	cout<<"Attention:\n"
	"non-distributed queen will read \"seeds\""
	"files from the working directory.\n";
	cout<<"\"range\" file is required in all situation and \"deny\" file"
	"is optional. \n";
	return ;
}

#ifdef _TEST

int 
main(int argc, char* argv[])
try{
	CQueen queen;
	if (queen.read_args(argc, argv) != 0)
	{
		queen.help();
		return -1;
	}
	queen.run();
	return 0;
}
catch (excep &e) {
	cerr<<e.msg<<endl;
}

#endif //_TEST
