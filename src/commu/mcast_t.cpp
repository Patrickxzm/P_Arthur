#include <iostream>
#include <assert.h>
#include <limits>
#include "util/arg.h"
#include "util/cutem.h"
#include "util/util.h"
#include "mcast.h"
extern "C" {
#include "unp.h"
#undef max
};

#define SA struct sockaddr

using namespace std;
namespace {
	// 224.0.0.0 to 224.0.0.255 are Link-local multicast Addresses.
	const char *default_mgroup_addr = "224.0.0.251"; 
	const char *default_mgroup_port = "9877";
	const int maxline = 400056;
};

class CMsgNoRecord
{
public:
	CMsgNoRecord():startNo(-1), endNo(-1)
	{}
	string output()
	{
		ostringstream oss;
		oss<<list;
		if (list.size() > 0)
			oss<<", ";
		oss<<startNo;
		if (endNo > startNo)
			oss<<'-'<<endNo;
		list.clear();
		startNo = endNo = -1;
		return oss.str();
	}
	void count(int msgNo)
	{
		if (endNo == -1)
		{
			startNo = endNo = msgNo;
		}
		else if (endNo + 1 == msgNo)
		{
			endNo ++;
		}
		else
		{
			if (list.size() > 0)
				list += ", ";
			list += tostring(startNo);
			if (endNo > startNo)
				list += '-'+tostring(endNo);
			startNo =  endNo = msgNo;
		}
		return ;
	}
private:
	int startNo, endNo;
	string list;
};

map<string, CMsgNoRecord> msgNoListMap;  //map: fromstr->list

ostream& 
help(ostream& os)
{
	os<<"multicast test program(IP version indepentent), to send or receive messages.\n"
	  "\tUsage : cmd (--recv [--ifname= ] [--stat] [--abstract|--fulltext]) \n"
	  "\t\t  | (--send [--max-speed= ] [--package|--line]) [--maddr= ] [--mport= ] [-h|--help]\n"
	  "\t\t --recv : process will recieve lines from multicast group, and write them to cout\n"
	  "\t\t --ifname= : name of network interface to join\n"
	  "\t\t --stat : output statistic infomation about each multicast session.\n"
	  "\t\t --abstract : output message's abstract, which contains only head and tail of the message.\n"
	  "\t\t --fulltext : output the whole message.\n"
	  "\t\t --send : process will read lines from cin, and send them to multicast group\n"
	  "\t\t --max-speed= : of sending data in bytes/sec.\n"
	  "\t\t --package : messages in package:\"length\\n body\\n\"\n"
	  "\t\t --line : messages in line. (default)\n"
	  "\t\t --maddr= : multicast group ip-address. [default: "<<default_mgroup_addr<<"]\n"
	  "\t\t --prot= : multicast group port or service name. [default: "<<default_mgroup_port<<"]\n"
	  "\t\t cin, cout : message-lines multicasted in the group.\n"
	  "\t\t -h|--help : print this message.\n"
	  <<endl;
	return os;
}

int mrecv(CMultiCast &mc, bool abstract, bool stat, bool fulltext);
int msend(CMultiCast &mc, bool package, int max_speed);

int 
main(int argc, char* argv[])
{
	CArg arg(argc, argv);
	if (arg.found("-h") || arg.found("--help"))
	{
		help(cout);
		return 1;
	}
	CArg::ArgVal val;
	string maddr;
	if ((val=arg.find1("--maddr=")).get())
	{
		maddr = string(val);
	}
	else
	{
		maddr = default_mgroup_addr;
	}
	string serv;
	if ((val=arg.find1("--mport=")).get())
	{
		serv = string(val);
	}
	else
	{
		serv = default_mgroup_port;
	}
	CMultiCast mc(maddr, serv);

	if (arg.found("--recv"))
	{
		if ((val = arg.find1("--ifname=")))
			mc.set_join_if(val.get(), 0);
		mrecv(mc, arg.found("--abstract"), arg.found("--stat"), arg.found("--fulltext"));
		return 0;
	}
	else if (arg.found("--send"))
	{
		int max_speed;
		if ((val = arg.find1("--max-speed=")).get())
			max_speed = val.INT();
		else
			max_speed = -1;
		clog<<msend(mc, arg.found("--package"),  max_speed)
		   <<" messages sent."<<endl;
		return 0;
	}
	help(cerr);
	return -1;
}

void
show_status(const CMultiCastSession *session, CMultiCastSession::status_t status)
{
	clog << "From["<<session->from<<"]";
	if (status == CMultiCastSession::Shutdown)
	{
		clog<<", Shutdown";
	}
	else if (status == CMultiCastSession::Out)
	{
		clog<<", Out";
	}
	else if (status == CMultiCastSession::Reset)
	{
		clog<<", Reset";
	}
	else if (status == CMultiCastSession::Recalc)
	{
		clog<<", Recalc";
	}
	clog <<":totalBytes="<<session->stat.totalBytes
	   <<", totalMesgs="<<session->stat.totalMesgs
	   <<", speedBytes="<<session->stat.speedBytes
	   <<", lostRatio="<<session->stat.lostRatio<<"%"<<endl;
	CMsgNoRecord &record = msgNoListMap[session->from];
	clog << record.output()<<endl;
	return;
}

string
nobr(const string &s)
{
	string ss;
	for (size_t i=0; i<s.size(); i++)
	{
		if (s[i] == '\n')
			ss.append("\\n");
		else
			ss.push_back(s[i]);
	}
	return ss;
}

int 
mrecv(CMultiCast &mc, bool abstract, bool stat, bool fulltext)
{
	while (true)
	{
		string message;
		struct sockaddr_storage from;
		socklen_t addrlen = sizeof(from);
		const CMultiCastSession* session;
		//void (*onStatusChange)(const CMultiCastSession*, CMultiCastSession::status_t) = 0;
		//onStatusChange = show_status; 
		if (stat)
		{
			session = mc.recv(message, (SA *)&from, &addrlen, show_status);
			CMsgNoRecord &record = msgNoListMap[session->from];
			record.count(session->msgNo);
		}
		else {
			session = mc.recv(message, (SA *)&from, &addrlen, 0);
		}
		if (abstract)
		{
			cout<<"From["<<session->from<<"]\n";
			assert(session->msgNo >= 0);
			size_t len = message.length();
			if (len<70)
				cout<<nobr(message)<<endl;
			else
				cout<<len<<" bytes:"<<nobr(message.substr(0,20))
				   <<"..."<<nobr(message.substr(len-20, 20))<<endl;
		}
		else if (fulltext)
		{
			cout<<"From["<<session->from<<"]\n";
			assert(session->msgNo >= 0);
			cout<<message<<endl;
		}
	}
	return 0;
}

int 
msend(CMultiCast &mc, bool package, int max_speed)
{	
	int count = 0;
	int bytesCount = 0;
	while (true)
	{
		struct timeval start;
		if (max_speed > 0 && -1 == gettimeofday(&start, 0))
		{
			ostringstream oss;
			oss<<"gettimeofday():"<<strerror(errno);
			throw runtime_error(oss.str());
		}

		string message;
		if (package)
		{
			static cutem buf;
			int length;
			if (!(cin>>length))
				break;
			buf.enlarge(length);
			cin.ignore(std::numeric_limits<int>::max(), '\n');
			if (!cin.read(buf.ptr(), length))
				break;
			message = string(buf.ptr(), length);
		}
		else 
		{
			if (!getline(cin, message))
				break;
		}
		mc.send(message);
		bytesCount += message.length();
		count ++;

		struct timeval end;
		if (max_speed > 0 && -1 == gettimeofday(&end, 0))
		{
			ostringstream oss;
			oss<<"gettimeofday():"<<strerror(errno);
			throw runtime_error(oss.str());
		}
		if (max_speed > 0)
		{
			double bytesQuota;
			bytesQuota = max_speed;
			bytesQuota *= (end.tv_sec - start.tv_sec)
			   + (double)(end.tv_usec - start.tv_usec)/1000000;
			if (message.length() > bytesQuota) 
				usleep((unsigned)((message.length()-bytesQuota)/max_speed*1000000));
		}
	}
	return count;
}

