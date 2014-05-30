#ifndef _PAT_OFS_HUGE_H_030304
#define _PAT_OFS_HUGE_H_030304
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include <iostream>

using std::ofstream;
using std::string;
class h_ofstream : public ofstream
{
public:
	virtual ~h_ofstream();
	h_ofstream();
	h_ofstream(const char* filename, unsigned size_limit, unsigned size_limit1, unsigned number_limit, bool compress=true);
	//default size limit = 100M, limit number of files = 100
	bool check();
	//description: If the current file grows out of SIZE LIMIT, open another file.If there is no serial number available, return false;
	bool open(const char* filename, unsigned size_limit, unsigned size_limit1, unsigned number_limit, bool compress=true);

	void close();
	unsigned current_suffix;
private:
	unsigned current_file_size();
	//description: return the size of file that is open now, bytes
	bool compressed_file_exist();
	//when we want to compress the files, we should check compress file too
	bool file_exist();
	
//	void open(const char* filename);
	bool open();
	
	
	char filename[256];
	unsigned number_limit;
	unsigned size_limit;
	unsigned size_limit1;
	unsigned file_size;
	bool compress;
};

#endif // _PAT_OFS_HUGE_H_030304
