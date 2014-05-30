#include "timestruct.h"
#include <stdlib.h>
#include <fstream>
#include <string>
#include <iomanip>
using std::string;
using std::setw;
#ifdef _DMALLOC
#include "dmalloc.h"
#endif //_DMALLOC
/**********************************************************************************************************************************/
CTime_struct::CTime_struct()
{
	year = 0;
	month = 0;
	day = 0;
	hour = 0;
	minute = 0;
}

CTime_struct::CTime_struct(const CTime_struct& another)
{
	year = another.year;
	month = another.month;
	day = another.day;
	hour = another.hour;
	minute = another.minute;
}
					
CTime_struct::CTime_struct(int year1, int month1, int day1, int hour1, int minute1)
{
	year = year1;
	month = month1;
	day = day1;
	hour = hour1;
	minute = minute1;
}

CTime_struct::CTime_struct(string str)
{
	if(str.length()==12 || str.length()==13)
	{
		string year1 = str.substr(0,4); 
		string month1 = str.substr(4,2);
		string day1 = str.substr(6,2);
		string hour1 = str.substr(8,2);
		string minute1 = str.substr(10,2);

		std::istringstream tmp1(year1);	
		std::istringstream tmp2(month1);	
		std::istringstream tmp3(day1);	
		std::istringstream tmp4(hour1);	
		std::istringstream tmp5(minute1);
			 	 
		tmp1 >> year;
		tmp2 >> month;
		tmp3 >> day;
		tmp4 >> hour;
		tmp5 >> minute;
	}
	else
	{
		year = 0;
		month = 0;
		day = 0;
		hour = 0;
		minute = 0;
	}
}

CTime_struct::CTime_struct(char* ptr)
{
		 
	 //the length of string i create is 12, but read from file is 13
	string	str(ptr);
	if(str.length()==12 || str.length()==13)
	{
		string year1 = str.substr(0,4); 
		string month1 = str.substr(4,2);
		string day1 = str.substr(6,2);
		string hour1 = str.substr(8,2);
		string minute1 = str.substr(10,2);

		std::istringstream tmp1(year1);	
		std::istringstream tmp2(month1);	
		std::istringstream tmp3(day1);	
		std::istringstream tmp4(hour1);	
		std::istringstream tmp5(minute1);

		tmp1 >> year;
		tmp2 >> month;
		tmp3 >> day;
		tmp4 >> hour;
		tmp5 >> minute;
	}
	else
	{
		year = 0;
		month = 0;
		day = 0;
		hour = 0;
		minute = 0;
	}
}

void CTime_struct::operator=(const CTime_struct& another)
{
	year = another.year;
	month = another.month;
	day = another.day;
	hour = another.hour;
	minute = another.minute;
}

//return seconds between 2 times 
int CTime_struct::operator-(const CTime_struct& another)
{
	time_t timep;
	struct tm *p;
	
	p = gmtime(&timep);
	
	p->tm_year = year - 1900;
	p->tm_mon = month - 1;
	p->tm_mday = day;
	p->tm_hour = hour;
	p->tm_min = minute;
	putenv("TZ=GMT0");
	tzset();
	int time1 = mktime(p);
		  
	p->tm_year = another.year - 1900;
	p->tm_mon = another.month - 1;
	p->tm_mday = another.day;
	p->tm_hour = another.hour;
	p->tm_min = another.minute;
	putenv("TZ=GMT0");
	tzset();
	int time2 = mktime(p);

	return time1-time2;
}

//if com_time<this object return -1; if = return 0; if > return 1;
int CTime_struct::compare(CTime_struct com_time)
{
	if(year>com_time.year)
		return 1;
	if(year<com_time.year)
		return -1;
	if(month>com_time.month)
		return 1;
	if(month<com_time.month)
		return -1;
	if(day>com_time.day)
		return 1;
	if(day<com_time.day)
		return -1;
	if(hour>com_time.hour)
		return 1;
	if(hour<com_time.hour)
		return -1;
	if(minute>com_time.minute)
		return 1;
	if(minute<com_time.minute)
		return -1;
	return 0;
}


void CTime_struct::second_to_time(time_t timep)
{
	struct tm *p;

	p = gmtime(&timep);

	year = p->tm_year+1900;
	month = p->tm_mon+1;
	day =  p->tm_mday;
	hour =  p->tm_hour;
	minute =  p->tm_min;
}

int CTime_struct::time_to_second()
{
	time_t timep;
	struct tm *p;
	
	p = gmtime(&timep);
	
	p->tm_year = year - 1900;
	p->tm_mon = month - 1;
	p->tm_mday = day;
	p->tm_hour = hour;
	p->tm_min = minute;
	
	putenv("TZ=GMT0");
	tzset();
	return mktime(p);
}

string CTime_struct::time_to_string()
{
	ostringstream oss;
	oss<<year<<setw(2)<< std::setfill('0')<<month
		<<setw(2)<< std::setfill('0')<<day
		<<setw(2)<< std::setfill('0')<<hour
		<<setw(2)<< std::setfill('0')<<minute;

	return oss.str();
}

void CTime_struct::normalize(int unit)
{
	time_t tmp_s = time_to_second();
	tmp_s = (tmp_s/unit)*unit;
	second_to_time(tmp_s); 
}
