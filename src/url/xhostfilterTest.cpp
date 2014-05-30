//xhostfilterTest.cpp
// add TESTFILE to xhostfilter.cpp
// input:	hostname
//	or	-f TESTFILE
// output:	xfilter.result

#include "xhostfilter.h"
#include "host.h"
#include <fstream>

#ifdef _TEST
using namespace std;
int
main(int argc, char* argv[])
{
        CXHostFilter filter;
        int ret;
        if (argc > 1)
        {
                ret = filter.init(argv[1]);
        } 
	else
                ret = filter.init("range");
        if (ret != 0)
                cerr<<"filter.init(): error!"<<endl;
        string host;
        while (cin>>host)
        {
		if (host[0] != '-'|| host[1] != 'f')
		{ 
                	if (filter.pass(host.c_str()))
                        	cout<<"PASSED!"<<endl;
                	else
                        	cout<<"NOT PASS!"<<endl;
		}
		else
		{
		        ofstream myout("xfilter.result");
       			if(myout.fail())
        		{
                		cerr<<"file open error!"<< endl;
                		return -1 ;
        		}
			ofstream my_not("not.pass");
			string testfile;
			cin>>testfile;
			//string testfile = host.substr(3);
                        
			//cout<<"file>";
                        //open the file . if failed , return
                        ifstream testfin(testfile.c_str());
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

                                if( hostname.size() == 0 || filter.pass(hostname.c_str() ))
                                {
                                        myout<<"Line "<<c_line<< '\t'<<" : "<< hostname << '\t'<<": PASSED"<<endl;
                                        c_pass++;
                                }
                                else
                                {
				        myout<<"Line "<<c_line<< '\t'<<" : "<< hostname << '\t' <<": NOT PASS"<<endl;
					my_not<<hostname<<endl;
				}
                        }
                        //statistic
                        cout<< "Total hostnames :" << '\t'<< c_line <<endl;
                        cout<< "PASSED :" <<'\t' << c_pass <<endl;
                        cout<< " NOT PASSED :" << '\t'<< c_line - c_pass <<endl;
			cout << "Details in file xfilter.result"<< endl;
	
                	myout<< "Total hostnames :" << '\t'<< c_line <<endl;
                        myout<< "PASSED :" <<'\t' << c_pass <<endl;
                        myout<< " NOT PASSED :" << '\t'<< c_line - c_pass <<endl;
			myout.close();
			my_not.close();
		}
        }
        return 0;
}
#endif //_TEST

