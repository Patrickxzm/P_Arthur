#include "tm.h"
#include <iomanip>
#include <sstream>
using namespace std;

const char* week[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun"
			, "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

CTime::CTime()
{
	invalid();
}

CTime::CTime(const string& tmstr)
{
	rdtime(tmstr);
}

tm_type CTime::rdtime(const string& tmstr)
{
	invalid();
	istringstream tm_is(tmstr);
	string weekday;
	tm_is>>weekday;
	switch (weekday.size()) {
	case 4:
		return rd822(tmstr);
	case 3:
		return rdansi(tmstr);
	default:
		return rd850(tmstr);
	}
	return t_unknown;
}

bool CTime::operator>(const CTime& t) const
{
	if (tm_year>t.tm_year) 
		return true;
	else if (tm_mon<t.tm_mon)
		return false;
	if (tm_mon>t.tm_mon) 
		return true;
	else if (tm_mon<t.tm_mon)
		return false;
	if (tm_mday>t.tm_mday) 
		return true;
	else if (tm_mday<t.tm_mday)
		return false;
	if (tm_hour>t.tm_hour) 
		return true;
	else if (tm_hour<t.tm_hour)
		return false;
	if (tm_min>t.tm_min) 
		return true;
	else if (tm_min<t.tm_min)
		return false;
	if (tm_sec>t.tm_sec) 
		return true;
	else if (tm_sec<t.tm_sec)
		return false;
	return false;
}

bool CTime::operator==(const CTime& t) const
{
	if (tm_year==t.tm_year && tm_mon==t.tm_mon && tm_mday==t.tm_mday
		&& tm_hour==t.tm_hour && tm_min==t.tm_min && tm_sec==t.tm_sec)
		return true;
	return false;
}

tm_type CTime::rd822(const string& tmstr)
{
	istringstream tm_is(tmstr);
	string weekday;
	tm_is>>weekday;
	weekday.resize(3);
	int i;
	tm_wday=-1;
	for (i=0; i<7; i++)
		if (week[i]==weekday) {
			tm_wday=i;
			break;
		}
	tm_is>>tm_mday;
	string monstr;
	tm_is>>monstr;
	tm_mon=-1;
	for (i=0; i<12; i++) 
		if (month[i]==monstr) {
			tm_mon=i;
			break;
		}
	tm_is>>tm_year;
	tm_year -= 1900;
	tm_is>>tm_hour;
	char ch;
	tm_is>>ch;
	tm_is>>tm_min;
	tm_is>>ch;
	tm_is>>tm_sec;
	if (valid()) 
		return t822;
	else 
		return t_unknown;
}

tm_type CTime::rd850(const string& tmstr)
{
	istringstream tm_is(tmstr);
	string weekday;
	tm_is>>weekday;
	weekday.resize(3);
	int i;
	tm_wday=-1;
	for (i=0; i<7; i++)
		if (week[i]==weekday) {
			tm_wday=i;
			break;
		}
	tm_is>>tm_mday;
	char ch;
	tm_is>>ch;
	string monstr;
	tm_is>>setw(3)>>monstr;
	tm_mon=-1;
	for (i=0; i<12; i++) 
		if (month[i]==monstr) {
			tm_mon=i;
			break;
		}
	tm_is>>ch;
	tm_is>>tm_year;
	if (tm_year>=0 && tm_year<90)
		tm_year += 100;
	tm_is>>tm_hour;
	tm_is>>ch;
	tm_is>>tm_min;
	tm_is>>ch;
	tm_is>>tm_sec;
	if (valid()) 
		return t850;
	else 
		return t_unknown;
}

tm_type CTime::rdansi(const string& tmstr)
{
	istringstream tm_is(tmstr);
	string weekday;
	tm_is>>weekday;
	int i;
	tm_wday=-1;
	for (i=0; i<7; i++)
		if (week[i]==weekday) {
			tm_wday=i;
			break;
		}
	string monstr;
	tm_is>>monstr;
	tm_mon=-1;
	for (i=0; i<12; i++) 
		if (month[i]==monstr) {
			tm_mon=i;
			break;
		}
	tm_is>>tm_mday;
	tm_is>>tm_hour;
	char ch;
	tm_is>>ch;
	tm_is>>tm_min;
	tm_is>>ch;
	tm_is>>tm_sec;
	tm_is>>tm_year;
	if (valid()) 
		return t_ansic;
	else 
		return t_unknown;
}

bool CTime::valid() const
{
	if (tm_sec<0 || tm_sec>59)
		return false;
	if (tm_min<0 || tm_min>59)
		return false;
	if (tm_hour<0 || tm_hour>23)
		return false;
	if (tm_mday<1 || tm_mday>31)
		return false;
	if (tm_mon<0 || tm_mon>11)
		return false;
	if (tm_year < 0)
		return false;
	if (tm_wday<0 || tm_wday>6)
		return false;
	return true;
}

void CTime::invalid()
{
	tm_sec=-1;
	tm_min=-1;
	tm_hour=-1;
	tm_mday=-1;
	tm_mon=-1;
	tm_year=-1;
	tm_wday=-1;
}

ostream& operator<<(ostream& os, const CTime& t)
{
	if (!t.valid()) 
		return os;
	os<<week[t.tm_wday]<<','<<' '<<setw(2)<<setfill('0')<<t.tm_mday;
	os<<' '<<month[t.tm_mon]<<' '<<setw(4)<<t.tm_year+1900<<' ';
	os<<setw(2)<<t.tm_hour<<':'<<setw(2)<<t.tm_min<<':'<<setw(2)<<t.tm_sec;
	os<<' '<<"GMT";
	return os;
}

#ifdef _TEST
using namespace std;
int 
main(int argc, char* argv[])
{
	for (int i=1; i<argc; i++) {
		CTime tm;
		tm.rdtime(argv[i]);
		cout<<tm<<endl;
	}
	return 0;
}
#endif // _TEST
