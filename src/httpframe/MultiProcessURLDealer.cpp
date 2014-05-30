#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "MultiProcessURLDealer.h"
#include "URLSource.h"
#include "WPageDealer.h"

CMultiProcessURLDealer::CMultiProcessURLDealer(CURLSource &source, 
				CWPageDealer &dealer,
				int nprocess, CXHostFilter *xfilter)
	: msgq(100, 200)
{
	this->nprocess = nprocess;
	psrc = &source;
	pdealer = &dealer;
	trace = stdout;
	debug = NULL;
	path1 = NULL;
	path2 = NULL;
	this->xfilter = xfilter;

}

CMultiProcessURLDealer::~CMultiProcessURLDealer()
{
	char* buf = (char*)malloc(256);
	assert(buf);
	sprintf(buf,"rm -f %s", path1);
	system(buf);
	sprintf(buf,"rm -f %s", path2);
	system(buf);
	free(buf);
	free(path1);
	free(path2);
}

int CMultiProcessURLDealer::init(const char* path1, const char* path2)
{
	assert(path1);
	assert(path2);
	this->path1 = strdup(path1);
	assert(this->path1);
	this->path2 = strdup(path2);
	assert(this->path2);

	int len = strlen(path1);
	if (len<strlen(path2)) 
		len = strlen(path2);

	char* buf = (char*)malloc(len+10);
	assert(buf);
	sprintf(buf,"touch %s", path1);
	system(buf);
	sprintf(buf,"touch %s", path2);
	system(buf);
	free(buf);
	return 0;
}

void CMultiProcessURLDealer::set_outstream(FILE* trace, FILE* debug)
{
	this->trace = trace;
	this->debug = debug;

}
	
const int max_nprocess = 800;

int CMultiProcessURLDealer::run()
{
	assert(psrc);
	assert(pdealer);
	assert(nprocess>0);
	assert(nprocess <= max_nprocess);
	assert(path1!=NULL && path2!=NULL);

	int pid;
	int* children = (int*)malloc(nprocess * sizeof(int)); 
	assert(children);

	for (int i=0; i<nprocess; i++){
		if ((pid = fork()) == 0) { /*child process */
			if (debug != NULL){
				fprintf(debug, "child %d pid working...\n", getpid());
				fflush(debug);
				sleep(30);
			}

			free(children);
			children = NULL;
			//char* url = (char*)malloc(256);		
			string urlstr;
			pdealer->revive();

			while(msgq.get(urlstr)==0 && urlstr != "@")
			{
				CHttp http(urlstr);
				if (xfilter != 0)
					http.set_hostfilter(xfilter);
				if (debug != NULL) {
					fprintf(debug, "url: %s \n", urlstr.c_str());
					fflush(debug);
				}
				http.fetch();
				if (trace != NULL) {
					fprintf(trace, "+");
					fflush(trace);
				}
				pdealer->deal(http);
				if (trace != NULL) {
					fprintf(trace, "-");
					fflush(trace);
				}
			}			
			if (trace!=NULL) {
				fprintf(trace, "child %d exit!\n", getpid());	
				fflush(trace);
			}
			pdealer->destroy();
			exit(0);	
		} 
		assert(pid>=0);
		children[i] = pid;
	}

	/* parents */
	char* url = (char*)malloc(256);
	memset(url,0,256);
	
	assert(url);
	while (psrc->get(url), strncmp(url, "@" ,256)){
		msgq.put(url);
	}

	free(url);
	url==NULL;

	/* send "@" as ending signal to child processes */	
	if (debug!=NULL){
		fprintf(debug, "parents send no job signal !\n");
		fflush(debug);
	}
	for (int i=0; i<nprocess; i++){
		msgq.put("@");
	}

	for (int i=0; i<nprocess; i++){
		waitpid(children[i],NULL,0);
	}
	return 0;
}
