#include "path.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

int command(const char* _command);
void help();
using namespace std;

int main(int argc, char* argv[])
{
	CPath *base = NULL;
	if (argc == 1) {
		help();
		return 0;
	}

	for (int i=1; i<argc; i++)
	{
		if (argv[i][0] != '-') 
			continue;
		char* absolute;
		switch (argv[i][1]) {
		case 'b':
			if (base != NULL) 
				delete(base);
			base = new CPath(argv[i]+2);
			cout<<argv[i]+2<<"--->";
			if (base->path() == NULL) 
				cout<<"error base path. \n";
			else cout<<base->path()<<endl;
			break;
		case 'r':
			cout<<argv[i]+2<<"--->";
			if (base == NULL){
				cout<<"input base first!\n";
				break;
			}
			absolute = base->newpath(argv[i]+2);
			if (absolute != NULL)
				cout<<absolute<<endl;
			else cout<<"error relative pathname.\n";
			if (absolute!=NULL)
				free(absolute);
			break;
		case '-':
			command(argv[i]+2);
			break;
		default:
			help();
			break;
		}
	}
	if (base != NULL)
		delete(base);
}

void help()
{
	cout<<"Usage: (pathname operation program)\n";
	cout<<"-bBASE   :    ";
	cout<<"tidy this absolute pathname[BASE], and use it as a base path.\n";
	cout<<"-rRELATIVE     :     ";
	cout<<"get absolute pathname of the [RELATIVE] pathname with the base.\n";
	cout<<"--help   :    ";
	cout<<"Help\n";
	return;
}

int command(const char* _command)
{
	if (strcmp(_command, "help") == 0) {
		help();
		return 0;
	}
	return 0;
}

