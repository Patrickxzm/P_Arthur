//hostfilter_a.cpp
// test all the hostnames or ipaddresses in one config file
// print the results line orderly and report the statistic result

#include "hostfilter.h"
#include<string>
#include <iostream>
#include <fstream>

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
    	string filename;

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
                int run = hostfilter.init(filename.c_str());
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
                        string testfile;
                        cout<<"file>";
                        if( ! getline( cin, testfile ) )        //crtl+d,exit
                        {
                                cout<<endl;
                                return 0;
                        }
	              	//open the file . if failed , return
        		ifstream testfin(testfile.c_str() ) ;
        		//cerr<<"Opening file:"<<testfile<<endl;
        		if(testfin.fail())
        		{
                	//cerr<<"Open file failed!"<<endl;
                	return -1;
        		}

        		//read the file line by line , check PASS or not
			int c_line = 0;
			int c_pass = 0;
			string hostname;
        		while(!testfin.eof())
        		{
                		getline( testfin, hostname );
                		c_line++;             //start from 1
		                
				if( hostname.size() == 0 || hostfilter.pass(hostname.c_str() ))
				{
                                	cout<<"Line "<<c_line<< '\t'<<": PASSED"<<endl;
                        		c_pass++;
				}
				else
                                	cout<<"Line "<<c_line<< '\t'<<": NOT PASS"<<endl;
                	}
			//statistic
			cout<< "Total hostnames :" << '\t'<< c_line <<endl;
			cout<< "PASSED :" <<'\t' << c_pass <<endl;
			cout<< " NOT PASSED :" << '\t'<< c_line - c_pass <<endl;
		}
	}
        else helpfile();

};

void helpfile()
{
        cerr<<"################ USAGE ###################"<<endl;
        cerr<<"./hostfilter_a -f CONGFILE to open the ip_config file."<<endl;
        cerr<<"File> TESTFILE             to check the hostnames in the file."<<endl;
        cerr<<"Ctrl+D		          to exit."<<endl;
        cerr<<"##########################################"<<endl;

 };
