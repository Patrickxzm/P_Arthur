#ifndef _CM_TIMESTRUCT_040615
#define _CM_TIMESTRUCT_040615

#include <time.h>

#include <string>
#include <sstream>
using namespace std;
class CTime_struct
{

	public:
		CTime_struct::CTime_struct();
		CTime_struct(const CTime_struct& another);
		CTime_struct(int year1, int month1, int day1, int hour1, int minute1);
		CTime_struct(char*);
		CTime_struct(string);
		
		//normalize the time according to the unit of time(such as 60 seconds)		
		void normalize(int unit);
		//if com_time<this object return -1; if = return 0; if > return 1;
		int compare(CTime_struct com_time);
		
		void operator=(const CTime_struct& another);
		int operator-(const CTime_struct& another);
		
		void second_to_time(time_t timep);
		int time_to_second();
		string time_to_string();  
					 
		int year;
		int month;
		int day;
		int hour;
		int minute;
};

#endif
