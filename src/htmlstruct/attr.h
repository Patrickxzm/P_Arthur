#ifndef _PAT_ATTR_H_092802
#define _PAT_ATTR_H_092802

#include <ostream>
#include <string>
using std::string;
using std::ostream;

class CAttr 
{
public:
	CAttr(const char* name, int len);
	virtual ~CAttr();
	const char* getvalue(const char* start, const char* end);
	const string& Value() const;
	const string& Name() const;

	friend ostream &operator<<(ostream &os, const CAttr &attr);
private:
	string name; 
	string value;
};

ostream &operator<<(ostream &os, const CAttr &attr);

#endif /* _PAT_ATTR_H_092802 */
