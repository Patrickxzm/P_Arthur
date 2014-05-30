// maskproducer.cpp
// convert the subnet file SOURCE to the standard form file TARGET
// ./maskproducer SOURCE TARGET

#include<string>
#include<iostream>
#include<fstream>

#ifdef DMALLOC
#include "dmalloc.h"
#endif

using namespace std;

int main(int argc, char* argv[])
{
	// if open source file error, return -1
	// if open source file error, return -2
	// if source file format error, return the line no.
	// others, return 0;

        string sourcefile = argv[1];
	string targetfile = argv[2];
	
	const string delims(" \t");

        ifstream myfin(sourcefile.c_str()) ;
	ofstream myfout(targetfile.c_str());
        if(myfin.fail())
        {
                cerr<<"Open SOURCE file failed!"<<endl;
                return -1;
        }
	if(myfout.fail())
	{
               	cerr<<"Open TARGET file failed!"<<endl;
               	return -2;
	}
        //read the file line by line , check the format
	
	string sourceline, outline;
	int c_line = 0;
        while(!myfin.eof())
        {	
		c_line++;
		getline( myfin, sourceline);
//              cerr<<"get line: "<< line << endl;
		string::size_type beg, end;
		beg = sourceline.find_first_not_of(delims);
		
		if(beg == string::npos)
			return c_line;
		for (int i = 1 ; i<=3; i++)
		{
			end = sourceline.find_first_of(delims, beg);
			if( end == string::npos)
				// end of this line
				end = sourceline.length();
			if( i != 2)
				outline.append( sourceline.substr(beg, end-beg));
			else
				outline.push_back('/');
			beg = sourceline.find_first_not_of(delims, end);
		}
		myfout<< outline <<endl;
		outline.clear();
		
        }

        myfout.close();
	return 0;
};
