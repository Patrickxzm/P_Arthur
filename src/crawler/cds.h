#ifndef _PAT_CDS_H_10122008
#define _PAT_CDS_H_10122008
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "util/shadow.h"
using std::ifstream;
using std::multimap;
using std::map;
using std::vector;
using std::string;

class grey_node 
{
public:
	grey_node(const string&u) : url(u), prev(0), next(0)
	{}
	virtual ~grey_node()
	{}
	string url;
	unsigned degree;
public:
	grey_node *prev;
	grey_node *next;
};

typedef grey_node Node;

class G_t
{
public:
	G_t();
	virtual ~G_t();

	grey_node* select();
	int put(grey_node* g);
	int update(const string &url, int incre);
	void clear();

	bool empty() const
	{
		return index.empty();
	}
private:
	void chain(grey_node* p);
	void unchain(grey_node* p);
private:
	vector<grey_node*> degree_que;
	map<string, grey_node*> index;
	unsigned maxd;
};

class B_t : public vector<Node*>
{}

typedef multimap<string, string> W_t;

class CDSAlgorithm
{
public:
	CDSAlgorithm();
	virtual ~CDSAlgorithm()
	{}
	const B_t& cal(ifstream *links, const map<string, int> *index
		, const string &root);
	int ncolored()
	{
		return colored.size();
	}
private:
	void blacken(Node* n);
	void greyen(Node* n);
	vector<string> out_neighbor(const string &url);
private:
	W_t W;
	G_t G;
	B_t B;
	CStrSetShadow colored;
// input:
	ifstream *links;
	const map<string, int> *index;
};
#endif // _PAT_CDS_H_10122008
