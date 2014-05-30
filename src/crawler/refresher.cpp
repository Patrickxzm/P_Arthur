#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "cutil.h"
#include "refresher.h"
#include "dealer.h"
#ifdef _DMALLOC
#include "dmalloc.h"
#endif //_DMALLOC


	const int DIGEST_LEN = 8;
	const int CKPOINT_LEN = 64;
	const int BASE_TIME = 290000*60;
	const int time_unit = 60; //minute
	const char* last_config = "last.cf";
	const char* last_read = "file_not_read";
	const unsigned int BUF_SIZE = 500;

	void sort(CTime_struct*, int);
	
	int power1(int x, int y)
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

	struct compare_small_first
	{
		bool operator()(CTime_struct x, CTime_struct y)
		{
			if(x.compare(y)>0)
				return true;
			return false;	 
		}
	};

	struct compare_big_first
	{
		bool operator()(CTime_struct x, CTime_struct y)
		{
			if(x.compare(y)<0)
				return true;
			return false;	 
		}
	};
	/**************************************************************************************************************************/
	CFreshInfo::CFreshInfo()
	{
		timeout = 0;
		depth = 0;
		CTime_struct null_time(0,0,0,0,0);
		real_time = null_time;
	}

	/**************************************************************************************************************************/
	CRefresher::CRefresher(const char * rconfig_file)
	{		  
	   //by cm   
		config_error = false;	
		if( !rconfig.init(rconfig_file) )
		{
#ifdef _TEST
			cout << "config error" << endl
			     << "please check your config file" << endl;
#endif
			config_error = true;
		}
		else
		{	
			//compute file suffix:
			float tmp = log10((float(rconfig.max_timeout))/(float(rconfig.min_timeout))) / log10(float(rconfig.step));
			float suffix = tmp -(int)tmp;
			int file_num;
			if(suffix >= 0.5) //sometimes tmp == 4.9999....so have to do this
				file_num = 2 + (int)tmp;
			else
				file_num = 1 + (int)tmp;
	   
			filesuffix = new FileSuffix( file_num );
/*
			changelog = fopen("log.change","a");
			deletelog = fopen("log.delete","a");
			newlog = fopen("log.new","a");
*/
			config_changed = false;	
			if(check_config())
				config_changed = true;
			init();
		}
	}

	CRefresher::~CRefresher()
	{		 
	//	cout<<"CRefresher::~CRefresher() is called."<<endl;
		if(filesuffix)
			delete filesuffix;
/*
		fclose(changelog);
		fclose(deletelog);
		fclose(newlog);
*/
	}

	void CRefresher::deal(workbench_t &bench)
	{
		char * tmp;
		tmp = create_cp(bench);
		string new_cp(tmp);
		
		bool changed = false;
		int old_timeout = bench.timeout;
#ifndef _TEST
		//if there is no url=bench.url in our file, we write delete log too
		if (bench.reply.status.code == 404)	
		{
			time_t timep;
			time(&timep);
//			fprintf(deletelog,"delete url %s %s\n",
//				bench.url->str().c_str(), ctime(&timep));
		return;
	}
#endif
	if (bench.timeout == 0)
	{
		changed = true;	//first time crawled
		time_t timep;
		time(&timep);
//		fprintf(newlog,"new url %s %s\n",
//			bench.url->str().c_str(), ctime(&timep));
	}
	else{
		if (new_cp != bench.checkpoint)
		{				//changed
			changed = true;
	
			time_t timep;
			time(&timep);
//			fprintf(changelog,"change url %s %s\n",
//				bench.url->str().c_str(), ctime(&timep));
		}
	}
	
	bench.changed = changed;	
	bench.timeout = rconfig.calculate(bench.url.str().c_str(),old_timeout,changed,bench.depth);
	
	CTime_struct real_time = get_real_time(bench.timeout);
#ifdef _TEST
	cout << "new real_time = " << real_time.time_to_string() << endl;
#endif
	
#ifndef _TEST
	bench.checkpoint = new_cp;
#endif

	save_to_file(bench,real_time);
}

//
void CRefresher::set_start_time()
{
#ifndef _TEST
	start_time = generate_suffix(0);
#endif

#ifdef _TEST
	//for debug:
	char fname0[256];
	strcpy(fname0,"set_time");	
	ifstream ifile0(fname0);
	if ( ifile0.is_open() )		 
	{
		char buf[50];
		ifile0>>buf;
		CTime_struct tmp(buf);
		start_time = tmp;
		ifile0.close();
	}
#endif	
	start_time.normalize(rconfig.min_timeout*time_unit);
	char fname[256];
	strcpy(fname,"start_time");
	ifstream ifile(fname);

	if ( !ifile.is_open() )		 
		last_time = start_time;
	else
	{
		char buf[50];
		ifile>>buf;
		CTime_struct tmp(buf);
		last_time = tmp;
		ifile.close();
		last_time.normalize(rconfig.min_timeout*time_unit);
		unlink(fname);
	}
	ofstream ofile(fname);
	if(!ofile.is_open())
	{
		cout << "error happen in function set_start_time()" << endl
		     << "we can not open the file named : \"start_time\"" << endl;
		return;
	}
	ofile<<start_time.time_to_string();		  
	ofile.close();

	return;
}

//return the seconds between now and the time starting to crawl this host(this turn)
int CRefresher::get_passed_time()
{
	time_t timep;
	int start = start_time.time_to_second();
	time(&timep); 
	return timep - start;
}


CTime_struct CRefresher::get_real_time(int timeout)
{
	time_t timep;
	struct tm *p;
	time(&timep);
#ifndef _TEST
	timep = timep + timeout*rconfig.min_timeout*time_unit;// - get_passed_time();
#endif

#ifdef _TEST
	timep = start_time.time_to_second() + timeout*rconfig.min_timeout*time_unit;
#endif
	p = gmtime(&timep);

	int year = 1900+p->tm_year;
	int month = 1+p->tm_mon;
	int day = p->tm_mday;
	int hour = p->tm_hour;	  
	int minute = p->tm_min;

	CTime_struct itmp(year,month,day,hour,minute);
	itmp.normalize(rconfig.min_timeout * time_unit);
	
	return itmp;
}

time_t CRefresher::get_url(workbench_t &bench)
{
	if(config_error)
		return -1;
	if ( refresh_buf.empty() )
	{
		if(last_time_finish())
			search_directory();
		else
			goon_reading();
	}


	
	if ( ! refresh_buf.empty() )
	{
		//return the task ,and fill the workbench
		CFreshInfo * rfinfo ;	
		rfinfo = refresh_buf.front();

		//bench.url = new CURL(rfinfo->str.c_str());
		bench.url.init(rfinfo->str);
		bench.checkpoint = rfinfo->checkpoint;
		bench.timeout = rfinfo->timeout;
		bench.depth = rfinfo->depth;
		
		refresh_buf.pop();

		delete(rfinfo);
#ifdef _TEST
		cout << "url = " << bench.url.str() << endl
	    	 << "checkpoint = " <<bench.checkpoint << endl
	    	 << "timeout = " << bench.timeout  << endl
	    	 << "depth = " << bench.depth  <<endl
		 << "real_time = " << rfinfo->real_time.time_to_string() << endl;
#endif
		return 0;
	}
	else//this turn no file_time <= start_time
	{//but we should check if there is a file, whose time <= now()
		DIR 		*dir;
		struct dirent 	*ptr;

		//get the filename which is less than current time
		priority_queue <CTime_struct, std::vector<CTime_struct>, compare_small_first > file_to_deal;
		priority_queue <CTime_struct, std::vector<CTime_struct>, compare_small_first > all_url_files;
		int url_file_num = 0;

		dir = opendir("./");
		while ( (ptr = readdir(dir)) != NULL)
		{
			if ( strncmp(ptr->d_name,"url.",4) == 0)
			{
				char * cptr = ptr->d_name +4;
		
				CTime_struct ftime(cptr);
			
				string teststr1(cptr);
				string teststr2 = ftime.time_to_string();	
				if ( teststr1 != teststr2 )
					continue;
				
				all_url_files.push(ftime);
				url_file_num++;
			}
		}
		closedir(dir);

		CTime_struct null_time(0,0,0,0,0);
		CTime_struct tmp(0,0,0,0,0);
		int size = 0;
		while(!all_url_files.empty())
		{	
			tmp = all_url_files.top();
			all_url_files.pop();
			string str = "url." + tmp.time_to_string();
			const char* fname = str.c_str();
			ofstream ofile(fname,std::ios::app|std::ios::out);
			if(!ofile.is_open())
				continue;
			size = ofile.tellp();
			ofile.close();
			if(size > 5) //we believe that this file should contain some urls
				break;
		}
		
		if(size>5 && tmp.compare(null_time) != 0)
		{
			tmp.normalize(rconfig.min_timeout * time_unit);
#ifndef _TEST
			CTime_struct current = generate_suffix(0);
#endif
#ifdef _TEST
			CTime_struct current = start_time;
#endif
			current.normalize(rconfig.min_timeout * time_unit);
		
			//now the time of the first file ,which should be updated, > start_time. that is why refresh_buf.empty() 
			//but <= current_time, that is why tmp.compare(current)<=0
			if(tmp.compare(current) <= 0)
			{
				init();	
				return get_url(bench);			
			}
			else	//return how many minutes left for queen to restart this mosquito
			{
				return tmp.time_to_second();
			}
		}
		
		//no url file exsits, so size<=5 or tmp==null_time
		return -2;	
	}
			
}

//if timeout=0 return current time;
CTime_struct CRefresher::generate_suffix(int timeout)
{
	time_t timep;
	struct tm *p;
	
	if(timeout == 0)//return now()
		time(&timep);
	else		
		timep = start_time.time_to_second() + timeout*rconfig.min_timeout*time_unit;
	
	p = gmtime(&timep);

	int year = 1900+p->tm_year;	
	int month = 1+p->tm_mon;
	int day = p->tm_mday;
	int hour = p->tm_hour;
	int minute = p->tm_min;
	
	CTime_struct tmp(year,month,day,hour,minute);
	tmp.normalize(rconfig.min_timeout*time_unit);
	return tmp;
}

int CRefresher::save_to_file(CFreshInfo * finfo)
{
	int index = 0;
	if( filesuffix->file_num != 1)
	{
		for(int i=0; i<filesuffix->file_num-1; i++)
		{
			if(finfo->real_time.compare(filesuffix->suffix[i+1].current) < 0)
			{
				index = i;
				break;
			}
			else if(i == filesuffix->file_num-2)
				index = i+1;
		}
	}
#ifdef _TEST
	cout << "save to file : " << filesuffix->suffix[index].current.time_to_string() << endl;
#endif
	if( !filesuffix->outfile[index].is_open() )
	{
		cout << "********************************************" << endl
		     << "error in save_to_file" << endl
		     << "index = " << index << endl
		     << "file name = " 
		     << filesuffix->suffix[index].current.time_to_string() << endl
		     << "********************************************" << endl;
		return 0;
	}
	filesuffix->outfile[index]<<finfo->str<<endl;
	filesuffix->outfile[index]<<finfo->checkpoint<<endl;
	filesuffix->outfile[index]<<finfo->timeout<<endl;
	filesuffix->outfile[index]<<finfo->depth<<endl;
	filesuffix->outfile[index]<<finfo->real_time.time_to_string()<<endl;

	return 1;
}

int CRefresher::save_to_file(workbench_t &bench, CTime_struct real_time)
{
	CFreshInfo * finfo = new CFreshInfo();
	finfo->str = bench.url.str();
	finfo->checkpoint = bench.checkpoint;
	finfo->timeout = bench.timeout;
	finfo->depth = bench.depth;
	finfo->real_time = real_time;
	int r_value = save_to_file(finfo);
	delete finfo;
	
	return r_value;
}

CTime_struct CRefresher::suffix(CTime_struct last_suffix, int i)
{
	CTime_struct suffix;

	if(i == 0)
		suffix = generate_suffix(1);
	else
	{
		int temp = (rconfig.step-1) * power1(rconfig.step,i-1);
		int unit = time_unit*rconfig.min_timeout;
		
		int last_time_out = ( last_suffix.time_to_second() - start_time.time_to_second() ) / unit;	
		int min = ( filesuffix->suffix[i].min.time_to_second() - start_time.time_to_second() ) / unit;
		int max = ( filesuffix->suffix[i].max.time_to_second() - start_time.time_to_second() ) / unit;
		int time_out = 0;
/*		cout << "min = " << min << ", max = " << max 
		     << ", last_time_out = " << last_time_out << ", temp = " << temp << endl;
*/		while(last_time_out >= 0)
		{
			last_time_out -= temp;
			if(last_time_out>= min && last_time_out<=max)
			{
				time_out = last_time_out;
//				cout << "new timeout = " << time_out << endl;
				break;
			}
		}
		suffix = generate_suffix(time_out);
	}
	return suffix;
}
/*
CTime_struct CRefresher::suffix(CTime_struct* former_suffix, int i)
{
#ifdef _TEST
	cout << i << "'s suffix is new" << endl;
#endif
	
	CTime_struct suffix;

	if(i == 0)
		suffix = generate_suffix(1);
	else
	{
		int temp;
		int* intervals = new int[filesuffix->file_num-1];
		intervals[0] = rconfig.step - 1;
		
		for(int j=1; j<filesuffix->file_num-1; j++)
			intervals[j] = intervals[j-1] * rconfig.step;
		
		int unit = time_unit*rconfig.min_timeout;
		int time_passed = start_time.time_to_second() - last_time.time_to_second();

		temp = (filesuffix->suffix[i].max.time_to_second() - start_time.time_to_second() ) / unit
			- ( time_passed - ((former_suffix[i]-last_time)
						-(filesuffix->suffix[i].min-start_time)) - unit) / unit
			% intervals[i-1];

		suffix = generate_suffix(temp);
		delete[] intervals;
	}
	return suffix;
}
*/
char * CRefresher::create_cp(workbench_t &bench)
{
	char *new_cp = (char * )malloc(CKPOINT_LEN+1);
#ifndef _TEST
	const char * body = bench.reply.body.c_str();
	parse_digest(body, &new_cp, CKPOINT_LEN+1);
#endif
	return new_cp;
}


int CRefresher::search_directory()
{
	DIR 		*dir;
	struct dirent 	*ptr;

	//get the filename which is less than current time
	priority_queue <CTime_struct, std::vector<CTime_struct>, compare_small_first > file_to_deal;

	priority_queue <CTime_struct, std::vector<CTime_struct>, compare_small_first > all_url_files;
	int url_file_num = 0;

	dir = opendir("./");
	while ( (ptr = readdir(dir)) != NULL)
	{
		if ( strncmp(ptr->d_name,"url.",4) == 0)
		{
			char * cptr = ptr->d_name +4;
		
			CTime_struct ftime(cptr);
			
			string teststr1(cptr);
			string teststr2 = ftime.time_to_string();	
			if ( teststr1 != teststr2 )
				continue;

			all_url_files.push(ftime);
			url_file_num++;
		}
	}
	closedir(dir);

	
	CTime_struct* url_file_array  = new CTime_struct[url_file_num];
	int index = 0;
	while( !all_url_files.empty() )
	{
		url_file_array[index] = all_url_files.top();
		all_url_files.pop();
		index++;
	}

	for(int i=0; i<url_file_num; i++)
	{
		CTime_struct tmp = url_file_array[i];
		tmp.normalize(rconfig.min_timeout * time_unit);
		if(tmp.compare(start_time) <= 0)
			file_to_deal.push(url_file_array[i]);
	}

	//read directory finish, and find that no file need to be freshed...
	if( file_to_deal.empty() )
	{
		unlink(last_read);
		delete[] url_file_array;	
		return 0;
	}
	
	//process the oldest urlfile

	while ( ! file_to_deal.empty() )
	{
		CTime_struct suffix_str(file_to_deal.top());	
		char fname[256];
		strcpy( fname,("url."+suffix_str.time_to_string()).c_str() );
		
		file_to_deal.pop();	

		for(int k=0; k<filesuffix->file_num; k++)
			if(suffix_str.compare(filesuffix->suffix[k].current)==0 && filesuffix->outfile[k].is_open())
			{
				filesuffix->outfile[k].close();
				cout << "********************************************" << endl
				     << "in search_directory" << endl
				     << "now we are closing a file, which should not be closed" << endl
				     << "k = " << k << endl
				     << "file name = " << filesuffix->suffix[k].current.time_to_string() << endl
				     << "start_time = " << start_time.time_to_string() << endl
				     << "********************************************" << endl;
			}
		
		ifstream ifile(fname);	
	
		if( !ifile.is_open() )
			printf("Error in open the file %s\n",fname);

		string line;

		int excursion = 0;	
		while (getline(ifile,line))
		{
			//read url
			CFreshInfo * finfo = new CFreshInfo();
			const char* temp = line.c_str();
			excursion += line.length()+1;
			TRIM(temp);
			finfo->str = temp;
			
			if(!getline(ifile,line))
			{
				delete finfo;
			      	break;
			}
			excursion += line.length()+1;
			finfo->checkpoint = line;		
			
			//read timeout
			if(!getline(ifile,line))
			{
				delete finfo;
	      			break;
			}
			excursion += line.length()+1;
			std::istringstream iss1(line);
			iss1 >> finfo->timeout;			

			//read depth	
			if(!getline(ifile,line))
			{
				delete finfo;
				break;
			}
			excursion += line.length()+1;
			std::istringstream iss2(line);
			iss2 >> finfo->depth;

			//read real time		
			if(!getline(ifile,line))
			{
		  		delete finfo;		
				break; 
			}
			excursion += line.length()+1;
			CTime_struct tmp(line);			
			finfo->real_time = tmp;//atoi(line);


			//this url should not be revisit now, so rewrite if to the proper file
			if( finfo->real_time.compare(filesuffix->suffix[0].current) > 0 )
			{
				save_to_file(finfo);
				delete finfo;
			}
			else
			{
	      			refresh_buf.push(finfo);
				if(refresh_buf.size() >= BUF_SIZE)
				{//delte former last_read file , and write current read point to it
			       		ofstream of(last_read);
			       		of << excursion << endl;
				       	of << suffix_str.time_to_string() << endl;
		       			while( !file_to_deal.empty() )			 
					{
						suffix_str = file_to_deal.top();
						of << suffix_str.time_to_string() << endl;
						file_to_deal.pop();
					}
					of.close();
					delete[] url_file_array;
					return 1;		 
			       	}
			}
		}
		ifile.close();
		unlink(fname);
		
	}
	unlink(last_read);

	delete[] url_file_array;
	
	return 1;
}

int CRefresher::parse_digest(const char * pagecontent, char * * pagedigest, int clen)
{
        if (pagecontent==NULL) return -1;
        if(clen<=DIGEST_LEN*4)
        {
                printf("digest is too short!\n");
                return -1;
        }
        memset(*pagedigest,0,clen);

        char *digest = (char *)malloc(clen);
        char    origin_digest[DIGEST_LEN+1];

        memset(digest,0,clen);
        memset(origin_digest,0,DIGEST_LEN+1);

        int len = strlen(pagecontent);
        int token = len/DIGEST_LEN;

        for(int i=0;i<DIGEST_LEN;i++)
                origin_digest[i]=pagecontent[token*i];
        sprintf(digest,"%x%x%x%x%x%x%x%x",abs(origin_digest[0]),abs(origin_digest[1]),abs(origin_digest[2]),abs(origin_digest[3]),abs(origin_digest[4]),abs(origin_digest[5]),abs(origin_digest[6]),abs(origin_digest[7]));
        strcpy(*pagedigest, digest);

        free(digest);

        return 0;
}


void sort(CTime_struct* array, int len)
{
	for(int i=0; i<len; i++)
	{
		int index = 0;
		for(int j=0; j<len-i; j++)
			if( array[j].compare(array[index])>0 )  //means array[j] > array[index]
				index = j;
		CTime_struct temp(array[len-i-1]);
		array[len-i-1] = array[index];
		array[index] = temp;
	}
}

bool CRefresher::check_config()
{
	//read last.cf
	ifstream ifile(last_config);
	if( !ifile.is_open() )  // first time to crawl
		return false;
	for(;ifile.is_open();)
	{
		string line;
		int temp;
		if(!getline(ifile,line))
		{
			ifile.close();	
			return true;
		}
		std::istringstream iss1(line);
      		iss1 >> temp;
		if(temp != rconfig.min_timeout)
			return true;
		if(!getline(ifile,line))
		{
			ifile.close();	
			return true;
		}
		std::istringstream iss2(line);
		iss2 >> temp;
		if(temp != rconfig.max_timeout)
			return true;
		if(!getline(ifile,line))
		{
			ifile.close();	
			return true;;
		}
		std::istringstream iss3(line);
		iss3 >> temp;
		if(temp != rconfig.step)
			return true;
		ifile.close();	
		return false;
	}	
	return false;
}

void CRefresher::save_config()
{
	unlink(last_config);
	ofstream ofile(last_config);
	ofile << rconfig.min_timeout << endl;
	ofile << rconfig.max_timeout << endl;
	ofile << rconfig.step << endl;
	ofile.close();
}

bool CRefresher::last_time_finish()
{
	ifstream ifile(last_read);
	if(ifile.is_open())
	{
		ifile.close();
		return false;
	}
	else
		return true;
}

void CRefresher::goon_reading()
{
	ifstream ifile(last_read);
	string line;
	if(!ifile.is_open())
		return;

	set_suffix();
		  
	if(!getline(ifile,line))
	{
		ifile.close();
		return;
	}
	std::istringstream iss(line);
	int excursion;
	iss >> excursion; 

	priority_queue <CTime_struct, std::vector<CTime_struct>, compare_small_first > file_to_deal;		  
	while(getline(ifile,line))
	{
		CTime_struct tmp_t(line);
		file_to_deal.push(tmp_t);
	}
	ifile.close();
		  


	//open first file and set excursion
	CTime_struct suffix = file_to_deal.top();
	file_to_deal.pop();
	string str = "url."+suffix.time_to_string();
	const char* fname = str.c_str();
	ifstream ifs;
	
	ifs.open(fname);
	if( !ifs.is_open() )
	{
		cout << "open file error1" << endl;		  	
		return;
	}
	
	ifs.seekg(excursion);
	
	int new_excursion = excursion;
	while( !file_to_deal.empty() || ifs.is_open())
	{
		if(!ifs.is_open())//not the first file, so open new one
		{
			suffix = file_to_deal.top();
			file_to_deal.pop();
			str = "url."+suffix.time_to_string();
			fname = str.c_str();
			
			ifs.open(fname);
			if(!ifs.is_open())
			{
				cout << "open file error2" << endl;
				return;
			}
		}
		
		for(int k=0; k<filesuffix->file_num; k++)
			if(suffix.compare(filesuffix->suffix[k].current)==0 && filesuffix->outfile[k].is_open())
			{
				filesuffix->outfile[k].close();
				cout << "********************************************" << endl
				     << "in goon_reading" << endl
				     << "now we are closing a file, which should not be closed" << endl
				     << "k = " << k << endl
				     << "file name = " << filesuffix->suffix[k].current.time_to_string() << endl
				     << "start_time = " << start_time.time_to_string() << endl
				     << "********************************************" << endl;
			}

		string line;

		while (getline(ifs,line))		
	       	{			 		
			//read url						
			CFreshInfo * finfo = new CFreshInfo();
			const char* temp = line.c_str();
			new_excursion += line.length() + 1;//there is a '\n' in each line, but string.length() does not count that
			
			TRIM(temp);
			finfo->str = temp;
			
			//read checkpoint
			if ( !getline(ifs,line) )
			{
				delete finfo;
				break;
			}
			new_excursion += line.length() + 1;
			finfo->checkpoint = line;		
			
			//read timeout			
			if ( !getline(ifs,line) )			
			{
				delete finfo;
				break;
			}
			new_excursion += line.length() + 1;
			std::istringstream iss1(line);
			iss1 >> finfo->timeout;			
			
			//read depth
			if ( !getline(ifs,line) )
			{
				delete finfo;
				break;
			}
			new_excursion += line.length() + 1;
			std::istringstream iss2(line);
			iss2 >> finfo->depth;
		
			//read real time		
			if ( !getline(ifs,line) )                             
				break; 
			new_excursion += line.length() + 1;
			CTime_struct tmp(line);			
			finfo->real_time = tmp;//atoi(line);
			

			//this url should not be revisit now, so rewrite if to the proper file	

			if( finfo->real_time.compare(filesuffix->suffix[0].current) > 0 )
			{	
				save_to_file(finfo);
				delete finfo;
			}
			else			
			{	
				refresh_buf.push(finfo);
				if(refresh_buf.size() >= BUF_SIZE)
				{//delte former last_read file , and write current read point to it
					unlink(last_read);
					ofstream of(last_read);
					of << new_excursion << endl;
					of << suffix.time_to_string() << endl;
					while( !file_to_deal.empty() )								 
					{
						suffix = file_to_deal.top();
						of << suffix.time_to_string() << endl;
						file_to_deal.pop();
					}	
					of.close();
					return;	 					
				}
						
			}
		}
		ifs.clear();
		ifs.close();
		unlink(fname);
					 
		//from second time, at first the excursion is 0.
		new_excursion = 0;
		  
	}
	unlink(last_read);
}

void CRefresher::set_suffix()
{	
	DIR 		*dir;
	struct dirent 	*ptr;
	dir = opendir("./");

	int url_file_num = 0;	
	priority_queue <CTime_struct, std::vector<CTime_struct>, compare_big_first > all_url_files;
	
	while ( (ptr = readdir(dir)) != NULL)
	{
		if ( strncmp(ptr->d_name,"url.",4) == 0)
		{
			char * cptr = ptr->d_name +4;
		
			CTime_struct ftime(cptr);
			
			string teststr1(cptr);
			string teststr2 = ftime.time_to_string();	
			if ( teststr1 != teststr2 )
				continue;

			all_url_files.push(ftime);
			url_file_num++;
		}
	}
	closedir(dir);

	if(url_file_num < filesuffix->file_num-1)
	{
		cout << "error happen in function set_suffix" << endl
			<< "url_file_num = " << url_file_num << ",   " 
			<< "filesuffix->file_num = " << filesuffix->file_num << endl;	 
		return ;
	}
	for(int i=filesuffix->file_num-1; i>=0; i--)
	{
		filesuffix->suffix[i].current = all_url_files.top();
		all_url_files.pop();
		if(filesuffix->outfile[i].is_open())
			filesuffix->outfile[i].close();
		string fname = "url." + filesuffix->suffix[i].current.time_to_string();
		filesuffix->outfile[i].open( fname.c_str(), std::ios::app|std::ios::out );
	}
}

void CRefresher::init_current_suffix()
{
	DIR 		*dir;
	struct dirent 	*ptr;

	CTime_struct null_time(0,0,0,0,0);

	for(int i=0; i<filesuffix->file_num; i++)
	       	filesuffix->suffix[i].current = null_time;	  

	//get the filename which is less than current time

	priority_queue <CTime_struct, std::vector<CTime_struct>, compare_small_first > all_url_files;
	int url_file_num = 0;

	dir = opendir("./");
	while ( (ptr = readdir(dir)) != NULL)
	{
		if ( strncmp(ptr->d_name,"url.",4) == 0)
		{
			char * cptr = ptr->d_name +4;
		
			CTime_struct ftime(cptr);
			
			string teststr1(cptr);
			string teststr2 = ftime.time_to_string();	
			if ( teststr1 != teststr2 )
				continue;

			all_url_files.push(ftime);
			url_file_num++;
		}
	}
	closedir(dir);

	
	CTime_struct* url_file_array  = new CTime_struct[url_file_num];
	int index = 0;
	while( !all_url_files.empty() )
	{
		url_file_array[index] = all_url_files.top();
		all_url_files.pop();
		index++;
	}
	
	//check if url files right or not, if not , fix them
	bool error_happen = false;
	if( !config_error )	
		save_config();
	else
	{
		delete[] url_file_array;
		return;
	}

	if( !config_changed )		// means that the config does not change
	{
#ifdef _TEST
		cout << "config does not change!" << endl;
		//calculate current file suffix ...
		if(filesuffix->file_num != url_file_num)
			cout << "file number not right" << endl
			     <<	"first time crawl this host?" << endl
			     << "or just changed config?" << endl;
#endif
		bool total_generate = true;
		for(int i=0; i<url_file_num; i++)
		{
			CTime_struct tmp = url_file_array[i];
			tmp.normalize(rconfig.min_timeout * time_unit);
			if(tmp.compare(start_time) <= 0)
				continue;
						
		       	for(int j=0; j<filesuffix->file_num-1; j++)				
       			{
				if(tmp.compare(filesuffix->suffix[j].min)>=0 			
						&& tmp.compare(filesuffix->suffix[j].max)<=0)
				{
					if(filesuffix->suffix[j].current.compare(null_time)!=0)
					{
						cout << "file suffix conflict :" << endl
						     << "file url." << filesuffix->suffix[j].current.time_to_string()
						     << " and file url." << url_file_array[i].time_to_string()
						     << " are too close!!" << endl;
						cout << "j = " << j << endl;
						for(int k=0; k<filesuffix->file_num-1; k++)
						{
						      cout<<"min "<<k<<" = " << filesuffix->suffix[k].min.time_to_string() << endl
						     	  <<"max "<<k<<" = "<< filesuffix->suffix[k].max.time_to_string() << endl;
						}
						cout << "************************************************" << endl;
						error_happen = true;;
					}
					filesuffix->suffix[j].current = url_file_array[i];
					if(filesuffix->outfile[j].is_open())
						filesuffix->outfile[j].close();
					string fname = "url." + filesuffix->suffix[j].current.time_to_string();
					filesuffix->outfile[j].open( fname.c_str(), std::ios::app|std::ios::out );
					total_generate = false;
					break;				
				}
				else if(j>=filesuffix->file_num-2)
				{
					if(filesuffix->suffix[filesuffix->file_num-1].current.compare(null_time)!=0)
					{
						cout << "file suffix conflict :" << endl
						     << "file url." 
						     << filesuffix->suffix[filesuffix->file_num-1].current.time_to_string()
						     << " and file url." << url_file_array[i].time_to_string()
						     << " are too close!!" << endl;
						cout << "last one" << endl;
						for(int k=0; k<filesuffix->file_num-1; k++)
						{
						      cout<<"min "<<k<<" = " << filesuffix->suffix[k].min.time_to_string() << endl
						     	  <<"max "<<k<<" = "<< filesuffix->suffix[k].max.time_to_string() << endl;
						}
						cout << "************************************************" << endl;
						error_happen = true;
					}
					filesuffix->suffix[filesuffix->file_num-1].current = url_file_array[i];
					if(filesuffix->outfile[filesuffix->file_num-1].is_open())
						filesuffix->outfile[filesuffix->file_num-1].close();
					string fname = "url." + filesuffix->suffix[filesuffix->file_num-1].current.time_to_string();
					filesuffix->outfile[filesuffix->file_num-1].open( fname.c_str(), std::ios::app|std::ios::out );
					total_generate = false;
				}
			}		  
		}
      		//if all the files timeout or first time to crawl this hostport
		//no need calculate fuffix (in fact , there will be some problems if calculate them)		  	 
	       	if( total_generate && !error_happen)
		{
#ifdef _TEST
			cout << "totally generate suffix" << endl;
#endif
			for(int i=0; i<filesuffix->file_num; i++)
			{			 
				filesuffix->suffix[i].current = filesuffix->suffix[i].max;
				filesuffix->suffix[i].current.normalize(rconfig.min_timeout*time_unit);

				if(filesuffix->outfile[i].is_open())
					filesuffix->outfile[i].close();
				string fname = "url." + filesuffix->suffix[i].current.time_to_string();
				filesuffix->outfile[i].open( fname.c_str(), std::ios::app|std::ios::out );
/*				int tmp = filesuffix->outfile[i].tellp();
				if(tmp <= 0)
					filesuffix->outfile[i]<<' ';		  	
				filesuffix->outfile[i].flush();
*/			}	  
		}
      		else if( !error_happen )//if this hostport has been icrementally creawled before
		{	   //we have to calculate the new file suffix			
#ifdef _TEST
			cout << "partially generate suffix" << endl;
#endif
			
			if(filesuffix->suffix[filesuffix->file_num-1].current.compare(null_time) == 0)
			{//calculate last suffix first
				for(int i=filesuffix->file_num-2; i>=0; i--)
				{
					if(filesuffix->suffix[i].current.compare(null_time) != 0)
					{
						int unit = time_unit*rconfig.min_timeout;
						int time_out = ( filesuffix->suffix[i].current.time_to_second() 
							- start_time.time_to_second() ) / unit;
						int temp = (rconfig.step-1) * power1(rconfig.step,i);
						
						for(int j=i; j<=filesuffix->file_num-2 ;j++)
						{
							time_out += temp;
							temp *= rconfig.step;
						}
						filesuffix->suffix[filesuffix->file_num-1].current = generate_suffix(time_out);
						string fname = "url." + 
							filesuffix->suffix[filesuffix->file_num-1].current.time_to_string();
						filesuffix->outfile[filesuffix->file_num-1].open( fname.c_str(), 
							std::ios::app|std::ios::out );
/*						int tmp = filesuffix->outfile[filesuffix->file_num-1].tellp();
						if(tmp == 0)
							filesuffix->outfile[filesuffix->file_num-1]<<' ';		  
						filesuffix->outfile[filesuffix->file_num-1].flush();
*/						break;
					}
				}
			}
			
			for(int i=filesuffix->file_num-2; i>=0; i--)
			{
				if(filesuffix->suffix[i].current.compare(null_time) == 0)
				{
					filesuffix->suffix[i].current = suffix(filesuffix->suffix[i+1].current,i);
					if(filesuffix->outfile[i].is_open())					
						filesuffix->outfile[i].close();
					string fname = "url." + filesuffix->suffix[i].current.time_to_string();
					filesuffix->outfile[i].open( fname.c_str(), std::ios::app|std::ios::out );
/*					int tmp = filesuffix->outfile[i].tellp();
					if(tmp == 0)
						filesuffix->outfile[i]<<' ';		  
					filesuffix->outfile[i].flush();
*/				}
			}		
		}		  
	}
	//by cm
	
	if(config_changed || error_happen)
	{
#ifdef _TEST
		if(config_changed)
			cout << "config changed!" << endl;
		if(error_happen)
			cout << "error happen, former file suffix not right!" << endl;
#endif
		int temp = 1;
		for(int i=0; i<filesuffix->file_num; i++)
		{
			filesuffix->suffix[i].current = generate_suffix(temp);
			if(filesuffix->outfile[i].is_open())
				filesuffix->outfile[i].close();
			string fname = "url." + filesuffix->suffix[i].current.time_to_string();
			filesuffix->outfile[i].open( fname.c_str(), std::ios::app|std::ios::out );
/*			int tmp = filesuffix->outfile[i].tellp();
			if(tmp == 0) 
				filesuffix->outfile[i]<<' ';		  	
			filesuffix->outfile[i].flush();
*/			temp *= rconfig.step;
		}
	 		 
		int counter = url_file_num - 1; 
		while( counter>=0 )
		{
			bool nth_to_do = false;
      			CTime_struct tmp = url_file_array[counter];
			for(int i=0; i<filesuffix->file_num; i++)		  
			{			
	      			if(tmp.compare(filesuffix->suffix[i].current) == 0)			
		       		{
					nth_to_do = true;
					break;
				}
			}						 
			tmp.normalize(rconfig.min_timeout*time_unit); 
			if(tmp.compare(start_time) <= 0)
				break;
			if(nth_to_do)
			{
				counter--;
				continue;
			}
				 	 
			string str = "url." + url_file_array[counter].time_to_string();
			const char* fname = str.c_str();
			ifstream ifile1(fname);	 
			if( !ifile1.is_open() )
				cout << "error in open file, file name = "
				     << fname << endl;
		       	string line;		
			
			//each "while" deal one record
			while(getline(ifile1,line))
			{
				CFreshInfo* finfo = new CFreshInfo();
				finfo->str = line;
				
				if(!getline(ifile1,line))
				{
					delete finfo;
					break;	
				}					
				finfo->checkpoint = line;
				
				if(!getline(ifile1,line))
				{
					delete finfo;
					break;
				}
				std::istringstream iss1(line);
			        iss1 >> finfo->timeout;
				
				if(!getline(ifile1,line))
				{
					delete finfo;
					break;
				}
				std::istringstream iss2(line);
				iss2 >> finfo->depth; 

				if(!getline(ifile1,line))
				{
					delete finfo;
					break;
				}
				CTime_struct tmp(line);
				finfo->real_time = tmp;


				//calculate which url file should this url be put
				save_to_file(finfo);
				delete finfo;
			}
	      		ifile1.close();
			unlink(fname);
			counter--;		 
	       	}				  
	}
		
#ifdef _TEST
	for(int i=0; i<filesuffix->file_num; i++)
	{
	       	int diff = (filesuffix->suffix[i].current.time_to_second() - start_time.time_to_second() )
			/ (rconfig.min_timeout*time_unit);
		cout << diff << endl;
	}
#endif	
	config_changed = false;
	delete[] url_file_array;
}

void CRefresher::init()
{
	set_start_time();
	int	max = rconfig.min_timeout;
	CTime_struct null_time(0,0,0,0,0);
	for(int i=0; i<filesuffix->file_num; i++)
	{
		filesuffix->suffix[i].max.second_to_time(max*60 + start_time.time_to_second());
		if(i == 0)
			filesuffix->suffix[i].min = null_time;
		else
			filesuffix->suffix[i].min.second_to_time(filesuffix->suffix[i-1].max.time_to_second() 
					+ rconfig.min_timeout*60);
		max *= rconfig.step;
	}
	//cant get current suffix here, but we can compute min and max suffix for all url. files according to the config
	init_current_suffix();	

}

#ifdef _TEST

int main(int argc,char * argv[])
{
	CRefresher refresher("my.config");

	
	workbench_t bench;


	int tmp = refresher.get_url(bench);	
	while (tmp==0)	
	{
		refresher.deal(bench);
		tmp = refresher.get_url(bench); 
	}
/*

	cout << "********************************************" << endl;
	CTime_struct time1(2005,1,5,3,0);
	cout << "outside:" << time1.time_to_second() << endl;
	cout << "outside:" << time1.time_to_string() << endl;
	time1.normalize(14400);
	cout << "outside:" << time1.time_to_second() << endl;
	cout << "outside:" << time1.time_to_string() << endl;
	
	cout << "********************************************" << endl;
        
	time_t timep = 0;
        struct tm *p;
	putenv("TZ=GMT0");
	tzset();
        p=gmtime(&timep);
        p->tm_year = 2005-1900;
        p->tm_mon = 1-1;
        p->tm_mday = 5;
        p->tm_hour = 3;
        p->tm_min = 0;
        timep = mktime(p);
        printf("%d\n",timep);
        printf("%d %d %d %d %d %d %s\n",p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,p->tm_zone);	


        timep = (timep/14400)*14400;
        printf("%d\n",timep);
        p=gmtime(&timep);
        printf("%d %d %d %d %d %d %s\n",p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec,p->tm_zone);	
*/
	return 1;
}

#endif // _TEST
