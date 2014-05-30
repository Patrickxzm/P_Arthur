#ifndef _PAT_COMMENT_H_092802
#define _PAT_COMMENT_H_092802

#include "htmlitem.h"
#include <ostream>
using std::ostream;
class CComment : public CHTMLItem
{
public:
	CComment(int pos);
	virtual ~CComment();
	const char* get(const char* start, const char* end);
	const char* content() const;
	friend ostream& operator<<(ostream& os, const CComment& com);
private:
	char* str;
};

ostream & operator<<(ostream &os, const CComment &com);

#endif /* _PAT_COMMENT_H_092802 */
