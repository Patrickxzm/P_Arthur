#include "urlque.h"
#include "tester.h"
#include "mosquito.h"
#include "arg.h"

class content_need_saved : public headers_filter
{
public:
	bool operator()(const CURL *url, const CHeaders *headers) 
	{
		if (url==0)
			return false;
		media_t mt = url->mtype();
		if (mt==m_text || mt==m_ps|| mt==m_pdf || mt==m_doc)
			return true;
		if (headers==0)
			return false;
		mt = headers->mtype();
		if (mt==m_text || mt==m_ps|| mt==m_pdf || mt==m_doc)
			return true;
		return false;
	}
};

class link_push_to_que : public link_filter
{
public:
	bool operator()(const CRef* ref, const CURL *url)
	{
		if (url==0)
			return false;
		media_t mt = url->mtype();
		if (mt==m_text || mt==m_ps|| mt==m_pdf || mt==m_doc
			|| mt==m_video || mt==m_audio 
			|| mt==m_unknown)
			return true;
		return false;
	}
};


void help()
{
	cout<<"Mosquito: Crawl and download web. \n" 
	"***** This is just a sample about how to use the CMosquito *****\n"
	"-tFILE: save pages downloaded to FILE.\n"
	"-sSIZE: Specify bitmap size in URL container for this site.\n"
	"-T : run the test code.\n";
	cout<<"******************************\n";
	CMosquito::help(cout);
}

int 
main(int argc, char** argv)
try{
	CArg arg(argc, argv);
	if (!arg.find1("--help").null()) 
	{
		help();
		return 0;
	}
	/************************************************/
	unsigned map_size = 24; //default value; 
	CArg::ArgVal av;
	if (!(av=arg.find1("-s")).null())
		map_size = av;
	CURLQue urlque(map_size);
	/************************************************/
	content_need_saved  _need_saved;
	link_push_to_que _push_to_que;
	CMosquito mosquito(&urlque, &_need_saved, &_push_to_que);
	/************************************************/
	CSavePage save;
	if (!(av=arg.find1("-t")).null())
	{
		string target(av);
		if (target.empty())
		{
			ostringstream oss;
			oss<<"pages."<<getpid()<<'.'<<time(0);
			save.init(oss.str().c_str());
		}
		else
			save.init(target.c_str());
		mosquito.add_dealer(&save);
	}
	/************************************************/
	CTester tester;
	if (!arg.find1("-T").null())
	{
		tester.init();
		mosquito.add_dealer(&tester);
	}

	/************************************************/
	if (mosquito.read_args(argc, argv) != 0)
	{
		mosquito.help(cout);
		return 0;
	}
	
	return mosquito.run();
}
catch (excep &e) {
	cerr<<e.msg<<endl;
}

