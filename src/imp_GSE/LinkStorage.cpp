
#include "LinkStorage.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

const int MAX_FILE_LENGTH = 200000000;
//const int MAX_FILE_LENGTH = 20000; 	//for testing

void CLinkStorage::write_head(ofstream * out)
{

	time_t timep;
	time(&timep);

	(*out) <<"version : "<<version	<<"\n"
		   <<"url : "	<<url		<<"\n"
		   <<"origin : "<<origin	<<"\n"	//?? what 's origin
		   <<"date : "	<<ctime(&timep)
		   <<"ip : "	<<ip		<<"\n"
		   <<"charset : "<<charset	<<"\n"
		   <<"outdegree : "<<outdegree	<<"\n"
		   <<"length : "<<length	<<"\n"
		   <<"\n";

}

void CLinkStorage::write_anchor(ofstream * out,workbench_t &bench)
{
	int nnum = bench.links.size();
	if (nnum ==0)
		return ;	

	for (int i = 0;i < nnum; i++)
	{
		CRef * reflink = bench.links[i];
		(*out)  <<"[" <<reflink->tagName()<<":href]"
			<<reflink->url()<<"\n";

		if (reflink->ref().empty())
			continue;
		(*out)	<<"#"<<reflink->ref()<<"\n";
	}

	(*out) <<"\n";
	
}
void CLinkStorage::get_from(workbench_t &bench)
{
	version = "1.0";
	url = bench.url.str();	
	origin = bench.url.str();		//  workbench has no info about origin
	ip = bench.host->printable_addr_used();		
	charset = "gb2312";	//  no info about charset ...
	outdegree = bench.links.size();

	int total_length = 0;
	int nnum = bench.links.size();
	for (int i = 0 ; i < nnum ; i++)
	{
		CRef * reflink = bench.links[i];
		total_length += 7 + reflink->tagName().size() + reflink->url().size();

		if (reflink->ref().empty())
			continue;
		total_length += 1 + reflink->ref().size();
	}
	length = total_length;		
	
	unzip_length = total_length;	
}
void CLinkStorage::read_from_disk(ifstream * in)
{
}
CLinkStorage::CLinkStorage()
{
/*
	filenum = 0;
  	stringstream s;
        s << filenum;
        fname = "link"+s.str()+".dat.gz";

	while (access(fname.c_str(),F_OK) == 0 )
	{
		fname.clear();
		s.str("");

		filenum ++;
		s << filenum;
       	 	fname = "link"+s.str()+".dat.gz";
	}

	fname = "link"+s.str()+".dat";
        outfile = new ofstream(fname.c_str());	
	if (!(*outfile))
	{
		cout << "error open file"<<fname<< endl;
	}
*/
	string fname = "link.dat";
	outfile = new h_ofstream(fname.c_str(), 10, 1, 100);
}
/*
CLinkStorage::CLinkStorage(ofstream *out)
{
	outfile = out;
}
*/
CLinkStorage::~CLinkStorage()
{
	delete outfile;

//	string stmp = fname+".gz";
/*	while (access(stmp.c_str(),F_OK) == 0)
	{
		fname.clear();
		filenum ++;
		stringstream s ;
		s << filenum;
		fname = "link"+s.str()+".dat";
		stmp = fname+".gz";
	}
*/
/*	string ss = "gzip "+fname;
	system(ss.c_str());
*/
}

void CLinkStorage::deal(workbench_t &bench)
{
	if (!bench.url)
		return;
	if(!outfile->check())
		return;
	get_from(bench);
	write_head(outfile);
	write_anchor(outfile,bench);
/*
	struct stat statbuf;
	
	if (stat(fname.c_str(),&statbuf) >=0 )
	{
		if (statbuf.st_size > MAX_FILE_LENGTH )
		{
			delete outfile;

			string ss = "gzip "+fname;
			system(ss.c_str());	

			filenum ++;

			stringstream s;
			s << filenum;
			fname = "link"+s.str()+".dat";
			outfile = new ofstream(fname.c_str());
		}
	}
*/	
}

#ifdef _TEST

int main(int argc,char * argv[])
{
	CLinkStorage linkstore;

	workbench_t bench;

	linkstore.deal(bench);
	return 1;
}
#endif
