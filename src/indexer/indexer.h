#ifndef _PAT_INDEXER_H_062505
#define _PAT_INDEXER_H_062505
#include "pat_types.h"
#include "slex.hpp"
#include <fstream>
#include "docs.h"
#include "inv_tab.h"

using std::vector;
using __gnu_cxx::hash_map;
using namespace pat_types;
using std::ofstream;
using std::ostream;
using std::istream;
using std::ios_base;

template<typename Doc>
class CIndexer
{
public:
typedef Doc doc_type;
typedef CDocs<Doc> docs_type;
public:
	CIndexer(const char* dict_path = 0);
	virtual ~CIndexer();
	void insert(const string &content, const Doc &doc);
	bool dump(const string & path);
	bool load(const string & path);
	void query(const string &qstr, vector<Doc> &docs);
	int nword() const
	{
		return word_inv.size();
	}
	int ndoc() const
	{
		return docs.size();
	}
private:
	void query(const string &qstr, vector<unsigned> &ids);
	CInvTab word_inv; // serialized.
	CDocs<Doc> docs;  
	CSlex slex;
};

void intersect(vector<unsigned> &a, vector<unsigned> &b, vector<unsigned> &c);

template<typename Doc>
CIndexer<Doc>::CIndexer(const char* dict_path)
{
	if (dict_path == 0)
		dict_path = "Lexicons/";
	if (!slex.load_dict(dict_path))
		throw ios_base::failure(string("load dict failed:\"")+dict_path+"\"");
}

template<typename Doc>
CIndexer<Doc>::~CIndexer()
{
	slex.unload();
}

template<typename Doc>
void 
CIndexer<Doc>::insert(const string &content, const Doc &doc)
{
	int id = docs.size();
	docs.push_back(doc);
	vector<seg_tag_unit_t> result;
	slex.seg_tag(content, result);
	hash_map_str2u w_loc;
	for (unsigned i=0; i<result.size(); i++)
	{
		string &word = result[i].word;
		if (word.size() < 2)
			continue;
		w_loc[word] |= (unsigned)1<<i;
	}
	hash_map_str2u::const_iterator cit;
	for (cit = w_loc.begin(); cit != w_loc.end(); cit ++)
	{
		struct word_app_t app;
		const string &word = cit->first;
		app.doc_id = id;
		app.location = cit->second;
		word_inv[word].push_back(app);
	}
	return;
}

template<typename Doc>
bool
CIndexer<Doc>::dump(const string& path)
{
	if (!docs.dump(path+"/docs"))
		return false;
	if (!word_inv.dump(path+"/inverted"))
		return false;
	return true;
}

template<typename Doc>
void
CIndexer<Doc>::query(const string &qstr, vector<unsigned> &ids)
{
	ids.clear();
	if (qstr.empty())
	{
		for (unsigned i=0; i<docs.size(); i++)
			ids.push_back(i);
		return;
	}
	vector<seg_tag_unit_t> result;
	slex.seg_tag(qstr, result);
	for (unsigned i=0; i<result.size(); i++)
	{
		string &word = result[i].word;
		if (word_inv.count(word) == 0)
		{
			ids.clear();
			return;
		}
		word_apps_t &apps = word_inv[word];
		vector<unsigned> docs;
		for (unsigned j=0; j<apps.size(); j++)
		{
			docs.push_back(apps[j].doc_id);
		}
		if (i==0)
		{
			ids = docs;
		}
		else
		{
			vector<unsigned> tmp = ids;
			intersect(tmp, docs, ids);
		}
	}
	return;
}

template<typename Doc>
void
CIndexer<Doc>::query(const string &qstr, vector<Doc> &docs)
{
	docs.clear();
	vector<unsigned> ids;
	query(qstr, ids);
	for (unsigned j=0; j<ids.size(); j++)
	{
		docs.push_back(this->docs[ids[j]]);
	}
	return;
}

template<typename Doc>
bool
CIndexer<Doc>::load(const string &path)
{
	if (!docs.load(path+"/docs"))
		return false;
	if (!word_inv.load(path+"/inverted"))
		return false;
	return true;
}

#endif // _PAT_INDEXER_H_062505
