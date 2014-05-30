#include "ucontainer.h"

CUContainer::CUContainer(unsigned mapsize): map(mapsize, 4, ".url.bmp")
{
}

int
CUContainer::push2(const urlinfo &info)
{
	return map.put(info.str);
	
}

int
CUContainer::push2(const string &urlstr, int depth)
{
	return map.put(urlstr);
}
