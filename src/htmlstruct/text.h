#ifndef _PAT_TEXT_H_092802
#define _PAT_TEXT_H_092802

#include "htmlitem.h"
#include <ostream>
using std::ostream;
class CText : public CHTMLItem
{
public:
	CText(int pos);
	const char* get(const char* start, const char* end);
	virtual ~CText();
	const char* content() const;

	friend ostream &operator<<(ostream &os, const CText &text);
private:
	char* str;
};

ostream &
operator<<(ostream &os, const CText &text);
#endif /* _PAT_TEXT_H_092802 */

