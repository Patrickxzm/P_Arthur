#ifndef _PAT_QUEEN_H_100203_
#define _PAT_QUEEN_H_100203_
#include "excep.h"
#include "task.h"
#include "TcpClient.h"
#include "mosquitos.h"
#include "hostports.h"
#include "msgq.h"
#include "xhostfilter.h"
#include "h_ofstream.h"
#include "green_path.h"
#include "LineMessage.h"
#include "pat_types.h"
#include "qmmsg.h"
#include "url.h"
#include "LinePool.h"
#include <string>

using std::string;
class QueenErr:public excep
{
public: 
	QueenErr(){}
	QueenErr(const string &m):excep("CQueen::"+m)
	{
	}
};

struct queen_env
{
	int missed_links_size_limit;
	hp_env hps;
	int msgq_size;
	double very_busy_ratio;	
	queen_env()
		: missed_links_size_limit(20), msgq_size(1000)
		, very_busy_ratio(0.9)
	{}
};

//////////////////////////////////////////////////////
// used to give options to mosquito through the queen.
class CServant
{
public:
        virtual const char* mos_option(const string &hostport) = 0;
	virtual ~CServant() {}
};

class CQueen 
{
public:
	CQueen();
	virtual ~CQueen();
public:
	void set_env(const struct queen_env &env)
	{
		this->env = env;
		hostports.setEnv(env.hps);
	}
	void get_env(struct queen_env &env)
	{
		hostports.getEnv(this->env.hps);
		env = this->env;
	}

	int read_args(int argc, char** argv); // 0 : OK 
	void run();
	void help();
	void add_servant(CServant *servant);
private:
	int send_mosquito(const string &hostport);
	int __send_mosquito(const string &hostport);
	void send_mosquitos();
	void do_msg(const string &msgstr);
	void do_link_from_king(const CLineMessage &lmsg);
	void do_link_from_mos(const CQMMsg &qmmsg);
	void do_link_from_sister(const CQMMsg &qmmsg);
	void do_mos_exit(const CQMMsg &qmmsg);
	void do_mos_stick(const CQMMsg &qmmsg);
	void cleardir(const string &hostport);
	// Return: how many messages recved exactly.
	unsigned request_king(unsigned rnum);  
	// Return: how many messages recved.
	unsigned request_king(); 
	// Return: how many messages recved from sisters.
	unsigned hear_sisters();
	void group(const CLineMessage &lmsg);
	void load_seeds(const char* file);
	void forward2king(const CQMMsg &qmmsg);
	bool quick_miss(const char* host);
	void accept(const CQMMsg &qmmsg, const CURL &url);
	void check_status(const char* file);
	void load(strset &__set, const char* file);
	void load(hash_map_str2str &__map, const char* file);
	void dump(const hash_map_str2str &__map, const char* file);
	void mk_mos_link();
	bool do_terminated();
	void init();
	int read_links_buffer(int n);
private:
	static void debug_exit();
	static void sig_term(int signo);
	
private:
	CMosquitos mosquitos; 
	CHostports hostports;   // should be saved before terminate.
	string msg_path;
	CMsgQue msgq;
	ofstream queen_log;
	unsigned max_nmos;
	int mos_prio;
	CXHostFilter hostfilter;
	CXHostFilter deny;
	CTCPClient client;
	string name;
	bool distributed;
	string inc_config;
	int depth;
	int nref_send;
	vector <CServant *> servants;
	int bandwidth; 
	h_ofstream missed_links;
	CGreenPath *green_path;
	string net_dev;
	hash_map_str2str site2Queen;  //should be saved.
	string workdir;
	hash_map_str2int nConns;
	hash_map_int2str mos2stick;
	static pid_t pid;
	static bool terminated;
	bool send_kill;
	time_t term_timer;
	struct queen_env env;
	time_t last_fork_time;
	bool very_busy;
	CLinePool link_buffer;
private:
	ofstream missed;
};
#endif // _PAT_QUEEN_H_100203_
