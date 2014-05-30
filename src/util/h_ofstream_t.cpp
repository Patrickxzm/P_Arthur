#include "arg.h"
#include "h_ofstream.h"

using namespace std;

const char* file_prefix = 0;
unsigned size_limit;
unsigned file_number;
bool compress;
unsigned size_limit1;

void help()
{

	std::cout << endl 

		<< "************************************************************" << endl

		<< "example command :" << endl

		<< "run -fdeleteme -s4 -n10 -c0 -l2" << endl

		<< endl

		<< "run is the exe file name" << endl

		<< "-f means the file prefix" << endl

		<< "-s means each file size(M)" << endl

		<< "-n means total file number" << endl

		<< "-c means compress(1) or not(0)" << endl

		<< "-l will set size_limit1, which decides " << endl
		<< "   the size of the file(M) that will be" << endl
	        << "   compressed when we quit from h_ofstream" << endl

		<< "************************************************************" << endl

		<< endl;

}

int main(int argc, char** argv)
{
	CArg arg(argc, argv);
	if (!(arg.find1("--help")).null())
	{
		help();
		return 0;
	}	
	////////////////////////////////////////////////
	CArg::ArgVal av;
	const char* filename = "deleteme";
	if (!(av=arg.find1("-f")).null())
		filename = av;
	////////////////////////////////////////////////
	size_limit = 4;
	if (!(av=arg.find1("-s")).null())
		size_limit = av;
	////////////////////////////////////////////////
	file_number = 10;
	if (!(av=arg.find1("-n")).null())
		file_number = av;
	////////////////////////////////////////////////
	compress = true;
	if (!(av=arg.find1("-c")).null())
	{
		int tmp = av;
		if(tmp == 0)
			compress = false;
	}
	////////////////////////////////////////////////
	size_limit1 = 2;
	if (!(av=arg.find1("-l")).null())
		size_limit1 = av;
	////////////////////////////////////////////////
	
	std::cout << endl
		<< "************************************************************" << endl
		<< "file name = " << filename << endl
		<< "file size limit = "<< size_limit << "M" << endl
		<< "file number = " << file_number << endl
		<< "compress = true" << endl
		<< "file size limit1 = " << size_limit1 << "M" << endl
		<< "************************************************************" << endl
		<< endl;

	h_ofstream _h_ofstream(filename,size_limit,size_limit1,file_number,compress);

	int msg = 8;

	char choise;
	while(choise != '3')
	{
		if(choise == '1')//write
		{	
			std::cout << "enter the size of file you want to write (M)" << endl;
			int size;
			std::cin >> size;
			for (int i=0; i<size*1024; i++)
		  	{
	       			if(_h_ofstream.is_open()) 
					for(int j=0; j<1024; j++)
						_h_ofstream<<msg;	
			 	else
				{						
				 	std::cout << "check() return false" << endl
						  <<_h_ofstream.current_suffix<<endl;
			       		return 1;		
	       			} 
		 	}
		}	
		else if(choise == '2')//check
		{
			if(_h_ofstream.check())
				std::cout << "check() == true" << endl;
			else
				std::cout << "check() == false" << endl;
		}

		std::cout << endl
			<< "choose your choise :" << endl
			<< "1  : write file" << endl
			<< "2  : call check function" << endl
			<< "3  : exit" << endl
			<< endl;
		std::cin >> choise;
	}
}
