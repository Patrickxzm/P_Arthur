#ifndef _PAT_CGI_H_090605
#define _PAT_CGI_H_090605
#include <string>
#include <vector>
using std::vector;
using std::string;

bool getvalue(const string &name, string &value);
bool getvalues(const string &name, vector<string> &values);
void descape(string &str);
#endif // _PAT_CGI_H_090605
