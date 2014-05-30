
// ===============================================
//	Author		:Gongbihong
//	Description	:for refresh config 
//	History:
//	 2003-12-31	:gbh,first establesh
// ================================================


#ifndef _GBH_RCONFIG_031231
#define _GBH_RCONFIG_031231

#include <stdio.h>
#include <string>
#include <vector>
#include <set>
#include <math.h>


using namespace std;

#define TRIM(EX) while ((EX)[0] ==' ') (EX)++;

class CFreshRule{
	public:
		string  rule;
		int	type;
		int	weight;
		
		CFreshRule();
		~CFreshRule();
};
struct lt_rule
{
	bool operator() (const CFreshRule& f1,const CFreshRule& f2)const
	{
		//printf(" cmm the f1.rule %s and f2.rule %s\n",f1.rule,f2.rule);
		return (f1.rule < f2.rule) ;
	}	
};
class RConfig
{
	public:
		RConfig();
		RConfig(char * filename);
		~RConfig();
		int calculate(const char * url,int old_timeout,bool changed,int depth);
		bool init(const char * filename);
     
	       	int min_timeout;	     
	  	int max_timeout;
		int step;	
		int privilege;
		
	private:
		bool is_privilege(const char * url,int link_depth);

		// read the define section from one line,and initial the min,max time etc.
		int  get_define(const char * line);
		int  get_rules(const char * line);

		set <CFreshRule,lt_rule> frules;
		
		char * 	filename;
		FILE * log;
};
#endif
