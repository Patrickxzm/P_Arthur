#include "rmdirtree.h"
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdio>

int 
rmdirtree(const string &path)
{
	DIR *dp = opendir(path.c_str());
	if (dp == NULL)
		return -1;
	struct dirent *entp;
	while ((entp=readdir(dp)) != NULL)
	{
		if(strcmp(entp->d_name,".")==0 || strcmp(entp->d_name,"..")==0)
			continue;
		struct stat statbuf;
		string fn = path+"/"+entp->d_name;
		lstat(fn.c_str(), &statbuf);
		if (statbuf.st_mode & S_IFDIR)
		{
			int res = rmdirtree(fn);
			if (res != 0)
				return res;
		}
		else
		{
			if (remove(fn.c_str()) != 0)
				return -2;
		}
	}
	closedir(dp);
	if (remove(path.c_str()) != 0)
		return -3;
	return 0;
}
