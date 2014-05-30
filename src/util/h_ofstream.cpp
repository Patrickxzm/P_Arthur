/**************************************************************************
 * @ This class is splitted out from CPageSaver to do deal with huge date 
 *   to be saved to an ofsteam. 	03/03/2004	22:53
 **************************************************************************/
#include "h_ofstream.h"
#include <fstream>
using std::ifstream;
using std::ostringstream;


h_ofstream::h_ofstream()
{
	filename[0] = '\0';
}

h_ofstream::~h_ofstream()
{
	if( is_open() )
		close();
}

void h_ofstream::close()
{
	ofstream::close();

	char fname[256];                             
	sprintf(fname,".%s.%d",filename,current_suffix);
	ifstream ifs(fname);
	if(ifs)
	{
		ifs.close();
		ostringstream oss;
		oss<<filename<<'.'<<current_suffix;
		string tmp = oss.str();
		const char* path = tmp.c_str();
		tmp = "." + tmp;
		const char* path1 = tmp.c_str();
		char ss[256];
		sprintf(ss,"mv %s %s",path1,path);
		system(ss);
	}

	if(current_suffix >= number_limit)
		return;
	if(current_file_size()>=size_limit1 && compress)
	{
	#ifdef _DEBUG
		if(!file_exist())
		{
			cout << "cm's error in h_ofstream::close()" << endl
			     << "current_file_size() = " << current_file_size() << endl
			     << "number_limit = " << number_limit << endl
			     << "size_limit = " << size_limit << endl
			     << "size_limit1 = " << size_limit1 << endl
			     << "filename = " << filename << endl;
			cout<<"Please debug me! pid = "<<getpid()<<endl;
			sleep(60);
		}
	#endif //_DEBUG
		char ss[256];
		sprintf(ss,"gzip %s.%d",filename,current_suffix);
		system(ss);
	}
}

h_ofstream::h_ofstream(const char* _filename, unsigned _size_limit, unsigned _size_limit1 , unsigned _number_limit, bool _compress)
{
	open(_filename,_size_limit,_size_limit1,_number_limit,_compress);
}

bool h_ofstream::open(const char* _filename, unsigned _size_limit, unsigned _size_limit1, unsigned _number_limit, bool _compress)
{
	if (fail())
		return false;
	strcpy(filename,_filename);
	size_limit = _size_limit * 1024 * 1024;
	size_limit1 = _size_limit1 * 1024 * 1024;
	number_limit = _number_limit;
	compress = _compress;
	current_suffix = 0;
	file_size = 0;
	
	//check if there is an unfinished file left.
	//if there is, continue to write it,
	// other than to write from current_suffix = 0
	if(compress)
	{
		for(current_suffix=0; current_suffix<number_limit; current_suffix++)
		{
			if(current_file_size() >= size_limit)
			{
				char ss[256];
			#ifdef _DEBUG
				if(!file_exist())
				{
					cout << "cm's error in h_ofstream::open(***)" << endl
					     << "current_file_size() = " << current_file_size() << endl
					     << "size_limit = " << size_limit << endl
			     		     << "filename = " << filename << endl;
					cout<<"Please debug me! pid = "<<getpid()<<endl;
					sleep(60);
				}
			#endif //_DEBUG
				sprintf(ss,"gzip %s.%d",filename,current_suffix);
				system(ss);
			}
		}
	}
		  
	current_suffix = 0;
	while( current_file_size() == 0 ) 
	{
		current_suffix ++;
		if(current_suffix == number_limit)
		{
			current_suffix = 0;
			break;
		}
	}
		  
	if(!open())
		printf("open file error, filesuffix = %d",current_suffix);
	
	return is_open();
}


bool
h_ofstream::open()
{
 	current_suffix = current_suffix % number_limit;
	if (strlen(filename) == 0)		
      		return false;   
	if(current_file_size()<size_limit && is_open())           
		return true;
	if(current_file_size()>=size_limit && is_open())
	{
		ofstream::close();				         
		ostringstream oss;
		oss<<filename<<'.'<<current_suffix;
		string tmp = oss.str();
		const char* path = tmp.c_str();
		tmp = "." + tmp;
		const char* path1 = tmp.c_str();
		char ss[256];
		sprintf(ss,"mv %s %s",path1,path);
		system(ss);
		
		
      		if(compress)
		{				 
			char ss[256];
		#ifdef _DEBUG
			if(!file_exist())
			{
				cout << "cm's error in h_ofstream::open()" << endl
				     << "current_file_size() = " << current_file_size() << endl
				     << "size_limit = " << size_limit << endl
			     	     << "filename = " << filename << endl;
				
				cout<<"Please debug me! pid = "<<getpid()<<endl;
				sleep(60);
			}
		#endif //_DEBUG
			sprintf(ss,"gzip %s.%d",filename,current_suffix);
			system(ss);
		}
     		current_suffix++;
	}

	for ( unsigned i=0; i<number_limit && !is_open(); i++)
	{
     		current_suffix = current_suffix % number_limit;
		if(current_file_size()<size_limit && !compressed_file_exist())
		{
			ostringstream oss;
			oss<<filename<<'.'<<current_suffix;
			string tmp = oss.str();
			const char* path = tmp.c_str();
			tmp = "." + tmp;
			const char* path1 = tmp.c_str();
        
			ifstream ifs(path);
		       	if(ifs)
			{
				ifs.close();
				char ss[256];
				sprintf(ss,"mv %s %s",path,path1);
				system(ss);
			}
			ofstream::open(path1, std::ios::app|std::ios::out);

			return true;
		}
		current_suffix++;
	}
	return false;
}

bool h_ofstream::check()
{
	if (fail())
		return false;
	file_size = current_file_size();
	return open();
}

bool h_ofstream::compressed_file_exist()
{
	char fname[256];                             
	sprintf(fname,"%s.%d.gz",filename,current_suffix);
	ifstream ifs(fname);
	if(ifs)
	{
		ifs.close();
		return true;
	}
	else
		return false;
}

bool h_ofstream::file_exist()
{
	char fname[256];                             
	sprintf(fname,"%s.%d",filename,current_suffix);
	ifstream ifs(fname);
	if(ifs)
	{
		ifs.close();
		return true;
	}
	else
		return false;
}

unsigned h_ofstream::current_file_size()
{
	if(is_open())
	{
		int r_value = tellp();
		if (r_value < 0)
		{
	#ifdef _DEBUG
		cout<<"Please debug me! pid = "<<getpid()<<endl;
		sleep(60);
	#endif //_DEBUG
		}
		return r_value;
	}
	else if(strlen(filename)==0)
		return 0;
	else
	{
		char fname[256];										     
		sprintf(fname,"%s.%d",filename,current_suffix);
		ofstream::open(fname, std::ios::app|std::ios::out);
		int r_value = tellp();
		if (r_value < 0)
		{
	#ifdef _DEBUG
		cout<<"Please debug me! pid = "<<getpid()<<endl;
		sleep(60);
	#endif //_DEBUG
		}
		ofstream::close();
		if(r_value==0)
			unlink(fname);
		return r_value;
	}
}

