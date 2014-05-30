#include "cgi.h"
#include <cstdlib>
#include <sstream>
using std::istringstream;

void
descape(string &str)
{
	unsigned i;
	for (i=0; i<str.size(); i++)
	{
		if (str[i]=='%' && isxdigit(str[i+1]) && isxdigit(str[i+2]))
		{
			char ch=0;
			char ch1 = toupper(str[i+1]); 
			char ch2 = toupper(str[i+2]);	
			if (isdigit(ch1))
			{
				ch=ch1 - '0';
			}
			else 
			{
				ch=ch1 - 'A' + 10;
			}
			ch *= 16;
			if (isdigit(ch2))
			{
				ch += ch2 -'0';
			}
			else
			{
				ch += ch2 - 'A' + 10;
			}
			str[i] = ch;
			str.erase(i+1, 2);	
		} 
	}
	return;
}

bool
getvalues(const string &name, vector<string> &values)
{
	values.clear();
	char *data;
	data = getenv("QUERY_STRING");
	
	if (data == 0)
	{
		return false;
	}
	istringstream iss(data);
	string attr;
	string value;
	while (getline(iss, attr, '=') && getline(iss, value, '&'))
	{
		if (attr == name)
		{
			//return true;
			values.push_back(value);
		}
	}
	if (values.empty())
		return false;
	return true;
}

bool
getvalue(const string &name, string &value)
{
	char *data;
	data = getenv("QUERY_STRING");
	
	if (data == 0)
	{
		return false;
	}
	istringstream iss(data);
	string attr;
	while (getline(iss, attr, '=') && getline(iss, value, '&'))
	{
		if (attr == name)
			return true;
	}
	return false;
}
