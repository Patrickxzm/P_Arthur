#include "cutil.h"
#include "rconfig.h"
#ifdef _DMALLOC
#include "dmalloc.h"
#endif //_DMALLOC


const int PLUS_SIGN	= 1;
const int SUB_SIGN	= 2;
const int MULTI_SIGN	= 3;
const int DIV_SIGN	= 4;
const int EQUAL_SIGN	= 5;
const int UPBOUND_SIGN  = 6;

int power(int x, int y)
{
	if(y == 0)
		return 1;
	int r_value = x;

	//error : no need ....
	if(y < 0)
		return -1;
	for(int i=1; i<y; i++)
		r_value *= x;
	return r_value;
}

CFreshRule::CFreshRule()
{
	type = 0;	
	weight = 0;
}
CFreshRule::~CFreshRule()
{
}
RConfig::RConfig()
{
	//set default values here;     
	min_timeout = 4*60;//4 hours
	max_timeout = 1024*4*60;//nearly = 180*24*60 = half year
	step = 4;//when get prediction error,we should make new_timeout = step*old_timeout
		 //(if timeout does not reach the max)
	srand((int)time(0));
	privilege = 1;
/*	
	log = fopen("log.config","a");
	if (!log)
		printf("Error open log.config file ! \n");
	else
		fprintf(log,"Init the RConfig object .....\n");
*/
	frules.clear();
}

RConfig::~RConfig()
{
}


int RConfig::calculate(const char *  url,int old_timeout,bool changed,int depth)
{
/*
	if(old_timeout==0)
		printf("first time to crawl %s\n",url);
	else if(changed)
		printf("web file changed, url = %s\n",url);
	else
		printf("web file did not change, url = %s\n",url);
*/	
	int timeout = 0;
	

	//in fact , it concerns the file number;
	int max_len = (int)(log10(max_timeout/min_timeout) / log10(step));
	
	//1.common rules;
	if (old_timeout == 0)
	{
		int tmp = rand() % 4;
		timeout = power(step,tmp);		
	}
	else	
	{
#ifndef _TEST
		timeout = changed ? 1 : old_timeout * step;
#endif

#ifdef _TEST	
		int tmp = rand();
		int judge = tmp % 2;

		if(judge == 1)
			timeout = old_timeout*step;
		else	timeout = old_timeout/step;

#endif
	}
	
	//2.if previlege

	if (is_privilege(url,depth))
	{
		if (timeout > privilege )
			timeout = privilege;
	}
	//3.use rules to change the timeout
	int slen = strlen(url);
	CFreshRule ftest ;
	ftest.rule = string (url);
	
	set<CFreshRule>::iterator iter;
	slen = ftest.rule.length();

	while (slen >7 )	// strlen(http://)  = 7
	{
		iter = frules.find(ftest);
		if (iter != frules.end() )
		{
			//if find then change the timeout and return 
			switch (iter->type) {
			case PLUS_SIGN : timeout += iter->weight;break;
			case SUB_SIGN  : timeout = timeout - iter->weight;break;
			case MULTI_SIGN: timeout = timeout * iter->weight;break;
			case DIV_SIGN :  timeout = timeout / iter->weight;break;
			case EQUAL_SIGN: timeout = iter->weight; break;
			case UPBOUND_SIGN: if (timeout > iter->weight) 
						timeout = iter->weight;
						break;
			default:break;
			}

			if (timeout > min_timeout*power(step,max_len)) timeout = min_timeout * power(step,max_len);
//			fprintf(log,"calculate the url %s timeout = %d old = %d\n",url,timeout,old_timeout);
			if(timeout < 1)
					  timeout = 1;
			assert(timeout > 0);
			return timeout;
		}
	
		//else strrstr(/) ,get the new string ,and new slen
		std::string::size_type pt = ftest.rule.rfind('/');
		if (pt != std::string::npos)
		{
			ftest.rule.erase(pt);
			slen = ftest.rule.length();
		}else slen = 0;
	}
	if (timeout > min_timeout*power(step,max_len)) timeout = min_timeout * power(step,max_len);
//	fprintf(log,"calculate the url %s timeout = %d old = %d\n",url,timeout,old_timeout);
	if(timeout < 1)
			  timeout = 1;
	assert(timeout > 0);
	return timeout;
}
bool RConfig::is_privilege(const char * url,int link_depth)
{
	//1.link depth
	if (link_depth == 0)
		return true;
	
	//2.dir depth
	int dir_depth = 0;
	
		
	const char * p = url;
	
	do {
		p = strchr(p,'/');
		if (p!=0)
		{
			dir_depth++;
			p = p+1;
		}
	}while (p!= 0);

	dir_depth = dir_depth - 2;
	if (dir_depth <= 1 )
		return true;

	return false;
}
bool RConfig::init(const char * filename)
{//ok
		
	FILE * fp = fopen(filename,"r");
	if (!fp)
	{}//		fprintf(log,"Error open the config file %s\n",filename);
	else
	{
//	     	fprintf(log, "reading the config file %s\n",filename);

		char * line = NULL;
		size_t len = 0;
		bool define_section = false;
		bool rule_section = false;
	
		while (getline(&line,&len,fp) != -1)
		{	
			char * cp = line;
			TRIM(cp);
			if ( strlen(cp) == 0 )
				continue;
	
			if (cp[0] == '#')
				continue;		//comment
	
			const char* tmp = "[define]";
			if (strncasecmp(cp, tmp, 8) == 0)
			{
				define_section = true;
				rule_section = false;
				continue;
			} 
			if (strncasecmp(cp,"[rule]",6) == 0)
			{
				rule_section = true;
				define_section = false;
				continue;
			}
			if (define_section) 
				get_define(cp);
		
			if (rule_section)  
				get_rules(cp);
		}
		if (line)
			free(line);
	
		fclose(fp);
	}
	if(min_timeout > max_timeout)
	{
		printf("current config : min_timeout > max_timeout\n");
		return false;
	}
	if(min_timeout<=0 || max_timeout<=0 || step<=1 || privilege<=0)
	{
		printf("min_timeout = %d\n",min_timeout);
		printf("max_timeout = %d\n",max_timeout);
		printf("step = %d\n",step);
		printf("privilege = %d\n",privilege);
		return false;
	}
	return true;
}
int RConfig::get_define(const char * line)
{

	const char * cp = line;
						        
	if (strncasecmp(cp,"MIN_TIMEOUT=",12) == 0 )
	{
			  min_timeout = atoi(cp+12);
//			  fprintf(log,"the MIN_TIMEOUT = %d\n",min_timeout);
			  return 1;
	}
	if (strncasecmp(cp,"MAX_TIMEOUT=",12) == 0)
	{
			  max_timeout = atoi(cp+12);
//			  fprintf(log,"the MAX_TIMEOUT = %d\n",max_timeout);
			  return 1;
	}
	if(strncasecmp(cp,"STEP=",5) == 0)
	{
			  step = atoi(cp+5);			
//			  fprintf(log,"the STEP = %d\n",step);
			  return 1; 												
	}
	if (strncasecmp(cp,"PRIVILEGE=",10) == 0)
	{
		privilege = atoi(cp+10);
//		fprintf(log,"the PRIVILEGE = %d\n",privilege);
		return 1;
	}
			
	return 0;
}
int RConfig::get_rules(const char * line)
{
	CFreshRule frule;
	frule.weight = 0;
	frule.type = 0;

	switch (line[0]) 	{
	case '+' :frule.type = PLUS_SIGN;break;
	case '-' :frule.type = SUB_SIGN;break;
	case '*' :frule.type = MULTI_SIGN;break;
	case '/' :frule.type = DIV_SIGN;break;
	case '=' :frule.type = EQUAL_SIGN;break;
	case '^' :frule.type = UPBOUND_SIGN;break;
//	default:  fprintf(log,"Error parsing the rule type %s\n",line);
	}	

	char * cp = strchr(line,' ');
	cp++;
	
	if (cp[strlen(cp)-1] == '\n')
		cp[strlen(cp)-1]='\0';
	
	frule.rule = string(cp);
	
	sscanf(line+1,"%d",&frule.weight);
		
	if ((frule.weight<= 0) || (frule.type == 0))
	{
//		fprintf(log,"Error parseing the rule type %s\n",line);
		return 0;
	}
	
#ifdef _TEST
	printf("read one rule %d %d %s\n",frule.type,frule.weight,frule.rule.c_str());
#endif

	frules.insert(frule);

	return 1;
}
