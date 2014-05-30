#ifndef _PAT_HOST4IMPORT_H_12282010
#define _PAT_HOST4IMPORT_H_12282010
#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include "task.h"
using std::string;
using std::vector;
using std::pair;
using std::istream;
using std::ostream;

class CHost4Import
{
public:
	string host;
	int port;
	vector<pair<string, CTask::status_type> > seeds; // pair of lpath and status;
	vector<string> trans_link;
	static string explain();
};

istream& operator>>(istream &is, CHost4Import &host);
ostream& operator<<(ostream &os, const CHost4Import &host);

#endif // _PAT_HOST4IMPORT_H_12282010
