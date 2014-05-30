/***********************************************************************
 * @ A GSE has to crawl websites upto a million. Give them a 2M bitmap each
 *   is too luxury, so I change its size to 100K. 100K bitmap means 50K 
 *   urls, use 20 bits indexer. 
 *					02/21/2004	23:15 
 * @ Use a more specific webpage saver. 
 *					02/12/2004	22:56
 * @ page-md5-bmp for some site.
 * @ Here is a GSE(General Search Engine) implement using mosquito core.
 *					02/09/2004	11:06
 * @ It's the time to split the mosquito in two: one is the core code; the
 *   other is the application code. 	02/09/2004	08:57
 ************************************************************************/

#ifdef _IMP_GUOTU
#include "urlque.h"
#include "tester.h"
#include "config.h"
#include "arg.h"
#include "mosquito.h"
#include "pagesaver.h"
#include "LinkStorage.h"

class content_need_saved : public headers_filter
{
public:
	bool operator()(const CURL *url, const CHeaders *headers) 
	{
		if (url==0)
			return false;
		media_t mt = url->mtype();
		if (mt==m_text || mt==m_ps|| mt==m_pdf || mt==m_doc
			|| mt==m_image)
			return true;
		if (headers == 0)
			return false;
		mt = headers->mtype();
		if (mt==m_text || mt==m_ps|| mt==m_pdf || mt==m_doc 
			|| mt==m_image)
			return true;
		if (mt==m_audio || mt==m_video)
		{
			int length = headers->content_length();
			if (length>=0 && length<=20000000)
				return true;
		}
		return false;
	}
};

class push_to_que : public link_filter
{
public:
	bool operator()(const CRef* ref, const CURL *url) 
	{
		if (url==0)
			return false;
		media_t mt = url->mtype();
		if (mt==m_text || mt==m_ps|| mt==m_pdf || mt==m_doc
			|| mt==m_image || mt==m_audio || mt==m_video
		        || mt==m_unknown)
			return true;
		return false;
	}
};


void help()
{
	cout<<"Mosquito: Crawl and download web. "
	"This is a Guotu implementation of mosquito. \n" 
	"-sSIZE: Specify bitmap size in URL container(CURLQue) for this site.\n"
	"******************************\n";
	CMosquito::help(cout);
}

int 
main(int argc, char** argv)
try{
	CArg arg(argc, argv);
	if (*arg.find("--help") != 0) 
	{
		help();
		return 0;
	}
	/************************************************/
	unsigned map_size = 20; //default value; 
	char** _map_size = arg.find("-s");
	if (*_map_size != 0)
	{
		sscanf(*_map_size, "%u", &map_size);
		if (map_size == 0)
			map_size = 20;
	}
	CURLQue urlque(map_size);
	/************************************************/
	content_need_saved _need_saved;
	push_to_que _push_to_que;
	CMosquito mosquito(&urlque, &_need_saved, &_push_to_que);
	/************************************************/
	CPageSaver save;
	mosquito.add_dealer(&save);
	/************************************************/
	CLinkStorage link_storage;
	mosquito.add_dealer(&link_storage);
	/************************************************/
	if (mosquito.read_args(argc, argv) != 0)
	{
		mosquito.help(cout);
		return 0;
	}
	CConfig config;
	config.open("pa.cnf", 0);
	
	return mosquito.run();
}
catch (excep &e) {
	cerr<<e.msg<<endl;
}


#endif // _IMP_GUOTU
