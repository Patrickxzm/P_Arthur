#include "inv_tab.h"
#include <fstream>
#include <sstream>
#include <iostream>

using std::endl;
using std::ofstream;
using std::ifstream;
using std::istringstream;

bool
CInvTab::dump(const string & fn)
{
	ofstream ofs(fn.c_str());
	if (!ofs)
		return false;
	for (const_iterator cit = begin(); cit != end(); cit ++)
	{
		ofs<<cit->first<<endl;
		const word_apps_t &word_apps = cit->second;
		for (unsigned i=0; i<word_apps.size(); i++)
		{
			ofs<<word_apps[i].doc_id<<' '
				<<word_apps[i].location<<' ';
		}
		if (!(ofs<<endl<<endl))
			return false;
	}
	return true;
}

bool
CInvTab::load(const string & fn)
{
	ifstream ifs(fn.c_str());
	if (!ifs)
		return false;
	while (true)
	{
		string word;
		word_apps_t word_apps;
		if (!getline(ifs, word))
			break;
		string line;
		if (!getline(ifs, line))
			break;
		istringstream iss(line);
		word_app_t word_app;
		while (iss>>word_app.doc_id>>word_app.location)
		{
			word_apps.push_back(word_app);
		}
		(*this)[word] = word_apps;
		while (!getline(ifs, line))
			break;
	}
	return true;
}
