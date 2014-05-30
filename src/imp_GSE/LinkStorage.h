

//###############################################################
// Author 		: Gongbihong
// Description		: To save the link structure 
// Time	  		: 2004-04-19
// History		: 
//##############################################################

#ifndef _GBH_LINKSTORE_040419
#define _GBH_LINKSTORE_040419

#include <stdio.h>
#include "dealer.h"
#include "url.h"
#include "h_ofstream.h"
#include <fstream>

class CLinkStorage : public CDealer
{
	public :

		string version;
		string url;
		string origin;
		string date;
		string ip;
		string charset;
		int    outdegree ;
		int    unzip_length ;
		int    length ;

		/* get the refrence info from bench and store to disk */
//		CLinkStorage(ofstream * out);

		CLinkStorage();
		~CLinkStorage();

		void deal (workbench_t &bench);
		void read_from_disk(std::ifstream * in);

	private:


		h_ofstream * outfile;
//		int filenum;
//		string fname;

		void get_from(workbench_t &bench);
		void write_head(ofstream * out);
		void write_anchor(ofstream * out,workbench_t &bench);

};


#endif
