#ifndef _PAT_DOCS_H_112405
#define _PAT_DOCS_H_112405
#include <ext/hash_map>
#include <fstream>

using __gnu_cxx::hash_map;
using __gnu_cxx::hash;
using std::ofstream;
using std::ifstream;
using std::endl;
template<typename Doc>
class CDocs : public vector<Doc>
// urlno --> doc
{
public:
	bool dump(const string & fn);
	bool load(const string & fn);
private:

};

template<typename Doc>
bool
CDocs<Doc>::dump(const string & fn)
{
	ofstream ofs(fn.c_str());
	if (!ofs)
		return false;
	for (unsigned i=0; i<this->size(); i++)
	{
		ofs<<(*this)[i];
		if (!(ofs<<endl))  // write the empty line.
			return false;
	}
	return true;
}

template<typename Doc>
bool 
CDocs<Doc>::load(const string & fn)
{
	this->clear();
	ifstream ifs(fn.c_str());
	if (!ifs)
		return false;
	string empty_line;
	Doc doc;	
	while (ifs>>doc && getline(ifs, empty_line))
	{
		push_back(doc);
		if (!empty_line.empty())
		{
			return false;
		}
	}
	return true;
}

#endif // _PAT_DOCS_H_112405
