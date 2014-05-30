/******************************************************************
  Utilities about time: 822 time, 850 time, ansi-C time
 ******************************************************************/
#ifndef _PAT_TM_H_080203_
#define _PAT_TM_H_080203_
#include <time.h>
#include <string>
#include <iostream>

enum tm_type { t822, t850, t_ansic, t_unknown };
using std::string;
using std::ostream;
typedef struct tm s_tm;

class CTime : public s_tm{
// used field : tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday
// unused field : tm_yday, tm_isdst, tm_zone, tm_gmtoff
public:
	CTime();
	CTime(const string& tmstr);
	tm_type rdtime(const string& tmstr);
	bool operator>(const CTime& t) const;
	bool operator==(const CTime& t) const;
	bool valid() const;
private:
	tm_type rd822(const string& tmstr); 
	tm_type rd850(const string& tmstr);
	tm_type rdansi(const string& tmstr);
	void invalid();
};

ostream& operator<<(ostream& os, const CTime& tm);
#endif // _PAT_TM_H_080203_
