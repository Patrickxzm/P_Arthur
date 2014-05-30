// ========================================================
//      Author          :Gongbihong
//      Description     :for increment crawler one host:port
//      History         :
//      2003-12-31      :gbh ,first establish
// ========================================================

#ifndef _GBH_REFRESH_031231
#define _GBH_REFRESH_031231

#include <stdio.h>
#include <vector>
#include <queue>
#include "dealer.h"
#include "url.h"
#include "rconfig.h"
#include "timestruct.h"
#include "cutil.h"
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

using namespace std;


struct Suffix
{
	CTime_struct current;
	CTime_struct min;
	CTime_struct max;
};

class FileSuffix
{
	public:
		int file_num;
		struct Suffix * suffix;
		ofstream* outfile;
		FileSuffix(int i)
		{
			file_num = i;
			suffix = new Suffix[i];
			outfile = new ofstream[i];
			CTime_struct null_time(0,0,0,0,0);
			for(int j=0; j<i; j++)
			{
      				suffix[j].current = null_time;
				suffix[j].min = null_time;
				suffix[j].max = null_time;				
			}				
		}
		~FileSuffix()
		{
			//std::cout<<"~FileSuffix() is called."<<endl;
			for(int i=0; i<file_num; i++)
			       if(outfile[i].is_open())
			       {
					int tmp = outfile[i].tellp();
					outfile[i].close();
					if(tmp <= 0)
					{               
					       	char ss[256];
						sprintf(ss,"unlink url.%s",suffix[i].current.time_to_string().c_str());
						system(ss);
					}
			       }
			delete[] outfile;	
			delete[] suffix;
		}
};

					 
class CFreshInfo
{		 		
	public:
		CFreshInfo(const string str,int timeout,string checkpoint,int depth,CTime_struct real_time);
		CFreshInfo();
		string str;
		CTime_struct real_time;
		int timeout;
		string checkpoint; 	
		int	depth;
};

class CRefresher : public CDealer
{
        public :
                /* get the url to refresh*/
                time_t get_url(workbench_t &bench);

                /* reschedule the url's timeout  & create the check point*/
                void deal(workbench_t &bench);

                CRefresher(const char * rconfig_file);
                ~CRefresher();
		
        private:
		//when mosquito start, record current time to variable start_time, and write it to file named "start_time"
		void set_start_time();
		
		//save the bench to the correct url.* file
		int save_to_file(workbench_t &bench, CTime_struct real_time);
		//overload this function
		int save_to_file(CFreshInfo* finfo);

		
		//create the checkpoint from this workbench
		char* create_cp(workbench_t &bench);	

		//generated digest from the pagecontent
		int parse_digest(const char * pagecontent, char * * pagedigest, int clen);

		//search the currenct directory,and find the file to refresh,
		//then insert into the refresh_buf.
		int search_directory(); 
		
		//if last_time_finish()==false, this function will be called to go on readding the url files
		//this function will ignore the config file change
		void goon_reading();

		//generate file suffix according to timeout.	
		CTime_struct generate_suffix(int timeout);
	
		//calculate timeout for generate_suffix()
		CTime_struct suffix(CTime_struct former_suffix, int i);

		//return the seconds between now and the time starting to crawl this host
		int get_passed_time();

		//return bench's real refresh time; the base time is now(not start_time of the mosquito)
		CTime_struct get_real_time(int timeout);	

		//check if config file changed or not
		bool check_config();
		
		//if refresher has changed the url files, then we should save the config
	  	//because it means that the file suffixes are correct ones, according to current config;
		void save_config();

		//check last time read all the proper url files to buffer or not
		//if read all, return true
		bool last_time_finish();
		
		//read directory to get file suffix
		void set_suffix();
	
		//init file suffix
		void init_current_suffix();
	
		//init function of refresher
		void init();	
		
/*		
		FILE * changelog;
		FILE * deletelog;
		FILE * newlog;
*/
		//keep information if config file has error or not
		//construct cannot return error,so we have to deal with it in get_url()
		bool config_error;

		//
		bool config_changed;	

		//record the urls that should be crawled again
                queue<CFreshInfo * > refresh_buf;

		
		// store all the rules to calculate the new timeout 
		RConfig rconfig;

		//record the current file suffix and the suffix range of the file
		FileSuffix* filesuffix;
		
		CTime_struct  start_time;	
		CTime_struct  last_time;
};
#endif

