#ifndef _PAT_SORT_HPP_01022011
#define _PAT_SORT_HPP_01022011
#include <vector>
#include <memory>
#include <istream>
#include <ostream>
#include <map>

using std::map;
using std::ostream;
using std::istream;
using std::vector; 
using std::auto_ptr;

// Here a queue means sorted.
class CSortBuffered 
{
public:
	CSortBuffered() : sum(0)
	{}
	int sum;
	vector<size_t> queNumbers;
};

template<class Key>
int 
merge(const vector<istream*> &multiQues, ostream &outQue)
{
	typedef map<Key, CSortBuffered> sortbuf_t;
	sortbuf_t sortbuf;
	vector<size_t> readyQues;
	for (size_t i=0; i<multiQues.size(); i++)
	{
		if (multiQues[i] && (*multiQues[i]))
			readyQues.push_back(i);
	}
	while (readyQues.size()>0 || sortbuf.size() > 0)
	{
		if (readyQues.size()>0)
		{
			size_t queNum = readyQues.back();
			readyQues.pop_back();
			Key key;
			int data;
			if ((*multiQues[queNum])>>key>>data)
			{
				sortbuf[key].sum += data;
				sortbuf[key].queNumbers.push_back(queNum);
			}
		}
		else { // sortbuf.size()>0 && readQues.size()==0
			typename sortbuf_t::iterator it = sortbuf.begin();
			outQue<<it->first<<'\t'<<it->second.sum<<'\n';
			for (size_t i=0; i<it->second.queNumbers.size(); i++)
				readyQues.push_back(it->second.queNumbers[i]);
			sortbuf.erase(it);
		}
	}
	return 0;
}
#endif // _PAT_SORT_HPP_01022011
