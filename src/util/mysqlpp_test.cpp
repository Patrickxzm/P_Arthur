#include <mysql++.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace mysqlpp;

Connection conn;

void 
test_quote(const string &str)
{
	Query q = conn.query();
	q<<quote<<str;
	cout<<q.str()<<endl;
	char buf[256];
	strcpy(buf, q.str().c_str());
	cout<<buf<<endl;
	return;
}

void
test_escape(const string &str)
{
	Query q = conn.query();
	q<<"\""<<escape<<str<<"\"";
	cout<<q.str()<<endl;
	char buf[256];
	strcpy(buf, q.str().c_str());
	cout<<buf<<endl;
	return;
}

void 
test_mysql_escape(const string &str)
{
	char buf[256];
	mysql_escape_string(buf, str.c_str(), 200);
	cout<<buf<<endl;
	return;
}

int 
main()
{
	if (!conn.connect("crawler", "webdigest", "crawler", "crawler2008"))
	{
		cerr<<"Connect \"crawler\" database failed!"<<endl;
		return -1;
	}
	ifstream ifs("quote.buf");
	if (!ifs)
	{
		cerr<<"Can not open file \"quote.buf\""<<endl;
		return -2;
	}
	ostringstream oss;
	oss<<ifs.rdbuf();
	test_quote(oss.str());
	test_escape(oss.str());
	test_mysql_escape(oss.str());
	return 0;
}
