#ifndef _PAT_IFS_HUGE_H_040909
#define _PAT_IFS_HUGE_H_040909

#include <fstream>
#include <iostream>

using std::ifstream;
using std::ofstream;
using std::string;

class h_ifstream : public ifstream
{
public:
	h_ifstream( char* filename );
	bool open( char* filename );
	bool check();
	virtual ~h_ifstream();
	void close();
	char*  getfilename();
	char* get_rdp_filename();
private:
	void  clearstate( int state );
	char* _filename;
	char* _current_file;
	std::ios::pos_type readpos;
	ofstream readpos_ofile;
	ifstream readpos_ifile;
	char* readpos_filename;
		
};

#endif // _PAT_IFS_HUGE_H_040909
