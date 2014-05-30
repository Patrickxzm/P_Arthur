#include "multi_hash.h"
#include <limits>
#include <numeric>
#include <stdexcept>
#include <openssl/md5.h>

using std::overflow_error;

unsigned 
MultiHashMD5::valueMax() const
{
	if (chunk_size > std::numeric_limits<unsigned>::digits)
		throw overflow_error("chunk_size bigger than unsigned.");
	unsigned res = 0;
	for (int i=0; i<chunk_size; i++)
	{
		res <<= 1;
		res |= 1;
	}
	return res;
}

vector<unsigned> 
MultiHashMD5::operator()(const string &s) const
{
	unsigned char buf[16];
	MD5((const unsigned char*)s.c_str(), s.size(), buf);
	
	vector<unsigned> res;
	unsigned ibyte=0, ibit=0;
	for (int i=0; i<chunk_num; i++)
	{
		unsigned chunk=0;
		unsigned remain=chunk_size;
		while (remain > 0)
		{
			if (ibyte >= 16)
				throw overflow_error("Out of 16 bytes md5.");
			unsigned char u= buf[ibyte];
			unsigned nbits = remain<=8-ibit ? remain : 8-ibit ;
			u <<= ibit;
			u >>= 8 - nbits;
			chunk <<= nbits;
			chunk |= u;
			if (ibit + nbits == 8)
			{
				ibit = 0;
				ibyte ++;
			}
			else
				ibit += nbits;
			remain -= nbits;
		}
		res.push_back(chunk);
	}
	return res;
}

MultiHashMD5::MultiHashMD5(int sizeChunk, int numChunk)
:chunk_size(sizeChunk), chunk_num(numChunk)
{
}

void 
MultiHashMD5::init(int sizeChunk, int numChunk)
{
	chunk_size = sizeChunk;
	chunk_num = numChunk;
}

MultiHashMD5::MultiHashMD5()
{
}

MultiHashMD5::~MultiHashMD5()
{
}
