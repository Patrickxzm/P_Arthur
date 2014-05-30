#include "host4import.h"
#include <sstream>
#include "url/url.h"
using std::ostringstream;
using std::istringstream;

string
CHost4Import::explain()
{
	ostringstream oss;
	oss<<
	"\t\t     { host[:port] eol \n"
	"\t\t 	    {lpath status eol}*   ### seeds: lpath & taskStatus(default: Unknown).\n"
        "                                 ### start with '/' "
	"\t\t 	    {>host[:port] eol}*   ### add trans_link records.\n"
        "                                 ### start with '>' "
	"\t\t       eol                   ### empty line \n"
	"\t\t     }* \n";
	return oss.str();
}

istream& 
operator>>(istream &is, CHost4Import &import)
{
	string line;
	if (!getline(is, line))
		return is;
	istringstream iss(line);
	string hostport;
	if (!(iss >> hostport))
	{
		is.setstate(std::ios::failbit);
		return is;
	}
	CURL::split("http", hostport, import.host, import.port);
	
	import.seeds.clear();
	import.trans_link.clear();
	while (getline(is, line) && !line.empty())
	{
		istringstream iss(line);
		string s;
		if (!(iss>>s))
                    break;
		if (s[0] == '/')
		{
                    string lpath(s);
		    CTask::status_type status;
		    if (!(iss>>status))
		        status = CTask::Unknown;
		    import.seeds.push_back(
                      pair<string, CTask::status_type>(lpath, status));
		}
		else if (s[0] == '>') 
		{  // the starting '>' will be excluded.
			import.trans_link.push_back(string(s, 1));
		}
		else
		{
			is.setstate(std::ios::failbit);
			return is;
		}
	}
	is.clear();
	return is;
}

ostream&
operator<<(ostream &os, const CHost4Import &import)
{
	os<<import.host<<':'<<import.port<<'\n';
	for (unsigned i=0; i<import.seeds.size(); i++)
		os<<import.seeds[i].first<<'\t'<<import.seeds[i].second<<'\n';
	for (unsigned i=0; i<import.trans_link.size(); i++)
		os<<'>'<<import.trans_link[i]<<'\n';
	os<<std::flush;
	return os;
}
