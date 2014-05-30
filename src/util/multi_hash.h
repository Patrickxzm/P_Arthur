#ifndef _PAT_MULTI_HASH_H_05142008 
#define _PAT_MULTI_HASH_H_05142008

#include <vector>
#include <string>
using std::string;
using std::vector;

class MultiHashMD5
{
public:
	MultiHashMD5();
// 2^sizeChunk = nbits = nbytes * 8 = max_items * 4;
	MultiHashMD5(int sizeChunk, int numChunk);
	void init(int sizeChunk, int numChunk);
	virtual ~MultiHashMD5();
	unsigned valueMax() const;
	vector<unsigned> operator()(const string &str) const;
private: 
	int chunk_size;
	int chunk_num;
};

#endif // _PAT_MULTI_HASH_H_05142008
