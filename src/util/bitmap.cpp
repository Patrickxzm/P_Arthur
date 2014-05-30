#include "bitmap.h"
#include <assert.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sstream>
#include <fstream>
#include <iostream>
using std::ostringstream;

CBitmap::CBitmap(unsigned nbyte)
:_M_p_ext(false), _M_length(nbyte)
{
	_M_p = new unsigned char[nbyte];
	assert(nbyte > 0);
}

CBitmap::CBitmap(unsigned char* ext, unsigned nbyte)
:_M_p(ext), _M_p_ext(true), _M_length(nbyte)
{
	assert(nbyte > 0);
}

CBitmap::~CBitmap()
{
	if (!_M_p_ext)
		delete[] _M_p;
}

void CBitmap::reset()
{
	memset(_M_p, 0, _M_length);
}

int CBitmap::val(unsigned idx)
{
	assert(idx>=0 && idx<size());
	return _M_p[idx>>3] & (128>>(idx&07)) ? 1:0;
}

int CBitmap::set(unsigned idx)
{
	int ret = val(idx);
	_M_p[idx/8] |= (128>>(idx&07));
	return ret;
}

std::ostream& operator<<(std::ostream& out, CBitmap &map)
{
	for(unsigned i=0; i<map.size(); i++)
		out<<map.val(i);
	out<<"\n";
	return out;
}

