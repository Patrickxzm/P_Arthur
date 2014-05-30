#ifndef _PAT_REF_H_102402
#define _PAT_REF_H_102402
#include <string>
#include <ostream>
using std::ostream;
using std::string;

class CRef {
public:
	CRef(const string &url, int pos, const string &tag);
	virtual ~CRef();
	void addref(const char* ref);
	const string& url() const
	{
		return _url;
	}
	const string &urlstr() const;
	const string &anchor_text() const
	{
		return _ref;
	}
	const string &ref() const
	{
		return _ref;
	}
	int pos() const
	{
		return _pos;
	}
	const string &tagName() const
	{
		return _tagName;
	}
	friend ostream &operator<<(ostream &os, const CRef &);
private:
	string _url;
	string _ref;
	int _pos;
	string _tagName;
};
ostream &operator<<(ostream &os, const CRef &);
#endif /* _PAT_REF_H_102402 */
