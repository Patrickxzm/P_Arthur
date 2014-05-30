/********************************************************************
 * An approximation algorithm for minimum Connected Dominating Set.
 ********************************************************************/
#include "cds.h"
#include "links.h"
#include <assert.h>
#include <stdexcept>
#include <sstream>
using std::ostringstream;
using std::runtime_error;
using std::pair;

//#define _DEBUG_20081020
#ifdef _DEBUG_20081020
#include <iostream>
#endif //_DEBUG_20081020

G_t::G_t(): maxd(0) 
{
	degree_que.push_back((Node*)0);
}

G_t::~G_t()
{
	clear();
}

grey_node* 
G_t::select()
{
	grey_node *g = degree_que[maxd]; 
	assert(g);
	unchain(g);
	assert(g != degree_que[maxd]);
	while (maxd>0 && degree_que[maxd]==0)
		maxd --;
	index.erase(g->url);
	return g;
}

void
G_t::clear()
{
	for (unsigned i=1; i<=maxd; i++)
	{
		while (degree_que[i] != 0)
		{
			grey_node *p = degree_que[i];
			unchain(p);
			delete(p);
		}
	}
	maxd = 0;
	index.clear();
}


int 
G_t::put(grey_node* g)
{
	assert(g);
	assert(!g->url.empty());
	assert(g->degree >= 0);
	if (index.find(g->url) != index.end())
		return -1;
	chain(g);
	if (g->degree > maxd)
		maxd = g->degree;
	index[g->url] = g;
	return 0;
}

int 
G_t::update(const string &url, int incre)
{
	if (index.find(url) == index.end())
		return -1;
	if (incre == 0)
		return 0;
	grey_node* p = index[url];
	if (p->degree + incre> 0)
	{
		unchain(p);
		p->degree += incre;
		chain(p);
		if (p->degree > maxd ) 
			maxd = p->degree;
		else	
		{
			while (maxd > 0 && degree_que[maxd] == 0)
				maxd --;
		}
	}
	else if (p->degree + incre == 0)
	{
		unchain(p);
		delete(p);
		index.erase(url);
		while (maxd > 0 && degree_que[maxd] == 0)
			maxd --;
	}
	else 
	{
		assert(false);
	}
	return 0;
} 

void 
G_t::chain(grey_node* p)
{
	assert(p->degree>0);
	while (p->degree >= degree_que.size())
		degree_que.push_back(0);
	p->next = degree_que[p->degree];
	p->prev = 0;
	degree_que[p->degree] = p;
	if (p->next)
		p->next->prev = p;
}

void 
G_t::unchain(grey_node* p)
{
	if (p->prev)
		p->prev->next = p->next;
	else
		degree_que[p->degree] = p->next;
	if (p->next)
		p->next->prev = p->prev;
	p->next = p->prev = 0;
}

CDSAlgorithm::CDSAlgorithm(): links(0), index(0)
{
	unsigned n;
	colored.init(n=1000000);
}

void 
CDSAlgorithm::greyen(Node* w)
{
	assert(w);
	//erase it from white node list;
	pair<W_t::iterator, W_t::iterator> r;
	r = W.equal_range(w->url);
	for (W_t::iterator it = r.first; it != r.second
	  ; W.erase(it ++))
	{
		G.update(it->second, -1);
	}
	//insert its children to white node list;
	vector<string> n = out_neighbor(w->url);
	int d = 0;
	for (unsigned i=0; i<n.size(); i++)
	{
		if (colored.has(n[i]))
			continue;
		W.insert(pair<string, string>(n[i], w->url));
		d ++;
	}
	colored.put(w->url);
	w->degree = d;
	if (w->degree > 0)
		G.put(w);
	else
		delete w;
	return ;
}

void 
CDSAlgorithm::blacken(Node* g)
{
	assert(g);
	B.push_back(g);
	vector<string> n = out_neighbor(g->url);
	for (unsigned i=0; i<n.size(); i++)
	{
		if (colored.has(n[i]))
			continue;
		Node *w = new Node(n[i]);
		greyen(w);
	}
}

B_t::~B_t()
{
	for (unsigned i=0; i<this->size(); i++)
	{
		if (this->at(i))
			delete(this->at(i));
	}
}

const B_t&
CDSAlgorithm::cal(ifstream *links, const map<string, int> *index
		, const string &root)
{
	assert(!root.empty());
	assert (links != 0 && index != 0);
	this->links = links;
	this->index = index;
	W.clear();
	G.clear();
	B.clear();
	
	Node *r = new Node(root);
	greyen(r);
	while (!G.empty())
	{
		Node *g = dynamic_cast<Node*>(G.select());
		assert(g);
		blacken(g);
	}
	return B;
}

bool 
readLink(ifstream &links, const map<string, int> &index, const string &url, CPageLink &link)
{	
	map<string, int>::const_iterator cit;
	cit = index.find(url);
	if (cit == index.end() || cit->second == -1 )
		return false;
	links.seekg(cit->second);
	if (!(links>>link) || link.url!=url)
	{
		ostringstream oss;
		oss<<"Can not read link: off("<<cit->second<<"),"
		  "for("<<url<<"), get("<<link.url
		  <<")";
		throw runtime_error(oss.str());
	}
	return true;
}

vector<string>
CDSAlgorithm::out_neighbor(const string &url)
{
	CPageLink link;
	if (!readLink(*links, *index, url, link))
		link.outlinks.clear();
	return link.outlinks;
}
