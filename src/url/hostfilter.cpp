/**************************************************************************
 * @ Big changes made by PAT.
 *						03/06/2004	19:42
 **************************************************************************/
#include "hostfilter.h"
#include "url.h"
#include <sstream>
#include <fstream>

#ifdef DMALLOC
#include "dmalloc.h"
#endif
using std::istringstream;
using std::ifstream;

CHostFilter::CHostFilter() : flag_all(false), usable(false), __have_ip(false)
{
}

bool
CHostFilter::test_ip()
{
	return __have_ip;
}

bool
CHostFilter::ipstr2uint(const string &str, unsigned &ipv4) const
{	
	int j = 0;
	for (int i=0; i<4; i++)
	{
		unsigned n=0;
		char ch;
		for (ch=str[j++]; ch>='0' && ch<='9'; ch=str[j++])
		{
			n *= 10;
			n += ch-'0';
		}
		if (ch!='.' && ch!='\0')
			return false;
		if (n>=256)
			return false;
		ipv4 *= 256;
		ipv4 += n;
	}
	return true;
}

int CHostFilter:: init(const char* ctlfile)
{
	/* 
	open the config_file with filename and check the format of the file ;
	if done successfully , return  0 ;
	if open file failed , return -1 ;
	if wrong format , return the no. of wrong line .
	if array overflow , return the contrariety of the line no.
	*/
							
	int line_num  = 0; 	//current line number
	enum line_type __type;	//the type of current line
	const int line_size = 256;
	char line[line_size];
	
	usable = false;
	__have_ip = false;
	//open the file . if failed , return 
	ifstream myfin(ctlfile) ;
	if (!myfin)
	{
		return -1;
	}

	//read the file line by line , check the format
	while(myfin.getline(line, line_size))
	{
	    	line_num++;		//start from 1
		__type = check(line); 
/*
		if(__type == wrong_format)
		{
			return -line_num-1 ;
		}
*/
		if (__type == iprange_line || __type == solo_ip
			|| __type == ip_dot )
		{
			__have_ip = true;
		}
		if (__type == all_line)
		{
			flag_all = 1;
			usable = true;
			return 0;
		}
		if(__type == wrong_format)
			return line_num;
	}	
	usable = true;
	return 0;
}

bool CHostFilter:: pass (const char* hostname ) const
{
	if (!usable)
		return false;
	// if flag_all is valid
	if(flag_all)
		return true;
	string host = hostname;
	if (!isIP(host))
	{
		if (names.count(host) == 1)
			return true;
		for (unsigned i=0; i<dot_domains.size(); i++)
		{
			if (host.size() < dot_domains[i].size())
				continue;
			if (host.compare(host.size()-dot_domains[i].size()
				, dot_domains[i].size()
				, dot_domains[i]) == 0
			)
				return true;
		}
		return false;
	}
	// is an IP address.
	if (ips.count(host) == 1)
		return true;
	unsigned i;
	for (i=0; i<ip_dots.size(); i++)
	{
		if (host.size() < ip_dots[i].size())
			continue;
		if (host.compare(0
			, ip_dots[i].size()
			, ip_dots[i]) == 0
		)
			return true;
	}
	unsigned ipv4;
	if (!ipstr2uint(host, ipv4))
		return false;
	for (i=0; i<ip_ranges.size(); i++)
	{
		if (ip_ranges[i].net == (ip_ranges[i].mask & ipv4))
			return true;
	}
	return false;
}

line_type
CHostFilter::check(char* thisline)
{
	if (thisline == 0)
		return empty_line;
	istringstream iss(thisline);
	
	string s1;
	if (!(iss>>s1))
		return empty_line;
	if (s1[0] == '#')
		return comment_line;
		
	if (s1 == "ALL")
	{
		flag_all = true;
		return all_line;
	}
	if (s1[0] == '.')
	{
		dot_domains.push_back(s1);
		return dot_domain;
	}
	if (s1[s1.size() - 1] == '.')
	{
		ip_dots.push_back(s1);	
		return ip_dot;
	}

	if (isIP(s1))
	{
		ips.insert(s1);
		return solo_ip;
	}
	
	string::size_type pos = s1.find('/');
	if (pos == string::npos)
	{
		names.insert(s1);
		return domain_name;
	}
	string net_str(s1, 0, pos);
	string mask_str(s1, pos+1);
	struct ipv4_range range;
	if (isIP(net_str) && isIP(mask_str) 
		&& ipstr2uint(net_str, range.net) 
		&& ipstr2uint(mask_str, range.mask)
	)
	{
		ip_ranges.push_back(range);
		return iprange_line;
	}
	return wrong_format;
}

