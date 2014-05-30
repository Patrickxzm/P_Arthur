#ifndef _PAT_TAG_H_092802
#define _PAT_TAG_H_092802

#include "htmlitem.h"
#include "attr.h"
#include <ostream>
#include <vector>
using std::ostream;
using std::vector;

class CTag : public CHTMLItem 
{
public:
	CTag(int pos);
	const char* get(const char* start, const char* end);
	virtual ~CTag();
	string Attr(const string& name) const;
	bool Attr(const string& name, string &value) const;
	const string &Name() const;

	friend ostream &operator<<(ostream &os, const CTag &tag);
private:
	const char* getattrs(const char* start, const char* end);
private:
	string name;
	vector<CAttr*> attrlist;
};

ostream &operator<<(ostream &os, const CTag &tag);

	
#endif /* _PAT_TAG_H_092802 */
