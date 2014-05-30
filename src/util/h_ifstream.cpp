/*************************************************************************
 ************************************************************************/

#include "h_ifstream.h"
#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include "arg.h"
#include <sstream>

using namespace std;

h_ifstream::h_ifstream( char* filename )
{
	_current_file = new char[128];
	_filename = new char[128];
	readpos_filename = new char[128];
	open( filename );
}

bool h_ifstream::open ( char* filename )
{
	_filename = filename;

	DIR		*dir;
	struct dirent	*ptr;

	bool is_ptr_null = true;
	
	dir = opendir(".");

	while( ( ptr = readdir(dir)) != NULL )
	{
		if( strncmp( ptr->d_name, _filename, strlen(_filename)) == 0 )
		{
			//cout<<"Find a file: "<<ptr->d_name<<endl;
			char tmp1[128];
			tmp1[0] = '\0';
			strcpy( tmp1, ".rdp_" );

			char temp4[128];
			temp4[0] = '\0';
			strcpy( temp4, ptr->d_name );

			char tmp2[128];
			tmp2[0]='\0';
			strcpy( tmp2, ptr->d_name );
			strcat( tmp1, tmp2 );

			//readpos_filename = tmp1;
			readpos_filename[0] = '\0';
			strcpy( readpos_filename, tmp1 );
			int end = strlen(tmp1);
			readpos_filename[end] = '\0';
			
			//cout<<"readpos_filename: "<<tmp1<<"  :  "<<readpos_filename<<endl;
			//cout<<"get rdp filename : "<<get_rdp_filename()<<endl;

			DIR		*dir1;
			struct dirent 	*ptr1;
			dir1 = opendir( "." );
			while( (ptr1 = readdir(dir1)) != NULL )
			{
				if( strncmp( ptr1->d_name, tmp1, strlen(tmp1)) == 0 ){
					if( !readpos_ifile.is_open() ){
						readpos_ifile.open( tmp1 );
					}
					readpos_ifile.clear();
					readpos_ifile.seekg( 0 );

					//char buf[8];
				       	//buf = readpos_ifile.rdbuf();
					//filebuf* fb = readpos_ifile.rdbuf();
					//cout<<" read:  "<< readpos_ifile.rdbuf()<<endl;

					string readpos_str;
					std::getline( readpos_ifile, readpos_str );

					//cout<<"read read pos str:  "<<readpos_str<<endl;
					istringstream iss( readpos_str );
					int readpos_int;
					iss>>readpos_int;
					readpos = (ios::pos_type)readpos_int;

					//cout<<"read pos int:  "<<readpos<<endl;
					if( readpos_ifile.is_open() )
						readpos_ifile.close();
				}
			}
			
			char temp1[128] ;
			temp1[0]='\0';
			
			int pos = strlen(ptr->d_name)-2;
			
			char* gz = ptr->d_name + pos;
			
			if( strcmp( gz, "gz") == 0 )
			{
				string temp = "gunzip ";
				string tempa = temp4;
				string unzip = temp + tempa;
				system( unzip.c_str() );

				int end = strlen(ptr->d_name);
				strncpy( temp1, temp4, end - 3 );
				temp1[end-3] = '\0';
			}
			else{
				int end = strlen(temp4);
				strcpy( temp1, temp4 );
				temp1[end] = '\0';
			}	
			//cout<<"temp1: "<<temp1<<endl;

			string temp2 = ".";
			string temp5 = temp1;
			string hide_file = temp2 + temp5;
			//cout<<"New file name: "<<hide_file<<endl;
			string ss = "mv " + temp5 + " " + hide_file;

			const char* temp = hide_file.c_str();
			strcpy( _current_file, temp);
			int len = strlen(temp);
			_current_file[len] = '\0';
			
			//_current_file = hide_file.c_str();
			system(ss.c_str());
			if( !is_open() ){
				//cout<<"current hide file: "<<_current_file<<endl;
				ifstream::open( _current_file );
			}
			seekg( readpos );
			is_ptr_null = false;
			break;
		}
	}
	if( is_ptr_null == true  ){
		//cout<<"No such file."<<endl;
		return false;
	}
	return true;
}

char* h_ifstream :: getfilename( )
{
	return _filename;
}

char* h_ifstream :: get_rdp_filename( )
{
	return readpos_filename;
}

void h_ifstream :: clearstate( int state )
{
	bool goodbit = false;
	bool badbit = false;
	bool eofbit = false;
	bool failbit = false;
	
	if( good() ) goodbit = true;
	if( eof() ) eofbit = true;
	if( bad() ) badbit = true;
	if( fail() ) failbit = true;
	
	clear();
	
	if( state == 1 ){
		if( eofbit )
			setstate( ios::eofbit );
	}
	if( state == 2)
	{
		if( failbit )
			setstate( ios::failbit );
	}
	if( goodbit )
		setstate( ios::goodbit );
	if( badbit )
		setstate( ios::badbit );
		
}

bool h_ifstream::check()
{
	if( fail() ){
		clearstate( 1 );
	}
	if( eof() )
	{
		//cout<<"EOF"<<endl;
		close();
		char* unhide_file = _current_file+1;
		unlink( unhide_file );
		clearstate( 2 );
		if(open( _filename ))return true;
	}
	/*
	if( !eof() )
	{
		cout<<"Not eof"<<endl;
	}*/

	return false;
}

h_ifstream::~h_ifstream()
{
	close();
	free(_current_file);
	//free(_filename);
	free(readpos_filename);

}

void h_ifstream::close()
{


	if( !is_open() )
		return ;
	
	char* unhide_file = _current_file+1;
	
	string temp1 = _current_file;
	string temp2 = unhide_file;
	string temp3 = "mv ";
	string temp4 = " ";
	string ss = temp3+temp1+temp4+temp2;
	
	readpos = tellg();
	
       	if( is_open() )
	{       
		system( ss.c_str() );
		ifstream::close();
	}
	
	if( eof() ) 
	{
		DIR		*dir2;
		struct dirent 	*ptr2;
		dir2 = opendir( ".");

		char* fn = get_rdp_filename();
		
		while( (ptr2 = readdir(dir2)) != NULL )
		{
			if( strncmp( ptr2->d_name, fn, strlen(fn)) == 0 ){
				unlink( fn );
			//	cout<<"unlink "<<fn<<endl;
			}
		}
		return ;
	}
	
	ostringstream oss;
	oss<<readpos;
	string temp = oss.str();

	if( temp == "0" ) return;
 
	
	if( !readpos_ofile.is_open() ){
	//	cout<<"open ofile."<<endl;
		readpos_ofile.open( get_rdp_filename() );
	}
	
	readpos_ofile<<temp;

	//cout<<"write read pos:  "<<temp<<endl;
	if( readpos_ofile.is_open() )
	{
		readpos_ofile.close();
	}
}

#ifdef _TEST

using std::ofstream;

void help()
{
	cout<<"Use \" ./hifs filename \""<<endl;
}

int main( int argc, char** argv)
{

	if( argc>2 ){
		cout<<"Invalid command."<<endl;
		return 0;
	}
	if( argc == 1 && strncmp( argv[0], "./hifs" ,6)==0 )
	{
		help();
		return 0;
	}
	
	CArg arg( argc, argv );
	if(*arg.find("--help") != 0)
	{
		help();
		return 0;
	}

	char* filename = argv[1];
	
	h_ifstream h_i( filename );

	
	cout<<"What do you want to do? Choose it: "<<endl;
	cout<<"1: read file"<<endl;
	cout<<"2: check"<<endl;
	cout<<"3: open a h_ifstream and read the front part. "<<endl;
	cout<<"4: exit"<<endl;
	cout<<"> ";
	
	int choise;
	cin>>choise;
	string temp1 = "bak_";
	string temp = filename;
	string o_filename = temp1 + temp;
	ofstream testof;
	string line;


	
	cin.clear();
	
	int for_test = 0;  //do not read all lines of the file
	int readline;

	char cur_filename[128];
	cur_filename[0] = '\0';
	int end;
	
	while( choise != 4)
	{
		switch(choise){
			case 1:
				if( !testof.is_open() ){
					testof.open( o_filename.c_str(), ios::app );
				}
				while( getline( h_i, line ) )
				{
					testof<<line<<"\n";
				}
				break;
			case 2:	
				h_i.check();
				break;
			case 3:
				cout<<"Input the line numbers that you want to read > ";
				cin>>readline;
				
				strcpy( cur_filename , h_i.getfilename() );
				end = strlen( h_i.getfilename());
				cur_filename[end] = '\0';
				
				//h_i.close();
			
				//cout<<"Filename: "<< cur_filename <<endl;
				//h_i.open( cur_filename );
				
				h_i.check();
				if( !testof.is_open() ){
					testof.open( o_filename.c_str(), ios::app );
				}
				while( for_test< readline )
				{
					if( getline( h_i,line ) );
					testof<<line<<"\n";
					for_test ++ ;
				}
				for_test = 0;
				h_i.close();
				h_i.open( cur_filename );
				break;
			default:
				break;
		}
		cout<<"What do you want to do? Choose it: "<<endl;
     		cout<<"1: read file"<<endl;
	        cout<<"2: check"<<endl;
		cout<<"3: open a h_ifstream and read the front part."<<endl;
	        cout<<"4: exit"<<endl;
		cout<<"> ";
		cin>>choise;		
		cin.clear();
	}
	cin.clear();
	h_i.close();
	if( testof.is_open() )
		testof.close();
	return 0;
}

#endif
