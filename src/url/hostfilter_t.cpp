//hostfilter_t.cpp
#include <iostream>
#include "hostfilter.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif

using namespace std;
enum mode 
{
    file,help,null
};

void helpfile();

int main(int argc, char* argv[])
{
	
	enum mode mode = null;
    char* filename = NULL;

	for (int i=1; i<argc; i++)
	{
		if (argv[i][0] != '-')
	    	continue;
	    switch (argv[i][1])
		{
		case 'f':
		    mode = file;
		    filename = argv[i+1];
			break;
		case 'h':
		    mode = help;
		    break;
		default: 
		    mode = help;
			break;
		}
	}
	if(mode == null)
		mode = help;
	if(mode == file)
	{
		CHostFilter hostfilter;
		int run = hostfilter.init(filename);
		if(run == -1)
		{
			cout<<"Can not open the file:"<<filename<<endl;
			return -1;
		}
		else if( run  < -1)
		{
			cout <<"Error, array overflew at Line No."<<-run<<endl;
			cout<< "Please divide into small files."<<endl;
		}
        if(run  > 0 )
		{
			cout<<"Format wrong in Line No."<<run<<endl; 
			return -1;
		}
		
		while(1)
		{
			char hostname[128];
			cout<<"host>";
			cin.getline(hostname,sizeof(hostname));
			if( hostname[0] =='\0' )	//crtl+d,exit
			{
				cout<<endl;
				return 0;
			}
			if(hostfilter.pass(hostname))
				cout<<"PASSED"<<endl;
			else 
				cout<<"NOT PASS"<<endl;
		}
	}
	else helpfile();
	
};

void helpfile()
{
	cerr<<"################ USAGE ###################"<<endl;
	cerr<<"./hostfilter -f FILENAME to open the ip_config file."<<endl;
	cerr<<"./hostfilter -h          to see the help file."<<endl;
	cerr<<">host HOSTNAME           to check the hostname."<<endl;
	cerr<<"Ctrl+D                   to exit."<<endl;
	cerr<<"##########################################"<<endl;

 };	
