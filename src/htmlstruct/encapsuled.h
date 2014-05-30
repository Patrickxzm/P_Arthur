/*************************************************************************
 * @ STYLE has the same attribute as SCRIPT: encapsuled by a pair of tags.
 *   So I get a class CEncapsuled to replace the class CScript. In fact, 
 *   CScript is a subclass of CEncapsuled, but I don't implement it.
 *					10/26/2003	08:18  
 *************************************************************************/
#ifndef _PAT_SCRIPT_H_102502
#define _PAT_SCRIPT_H_102502
#include "tag.h"
#include "memory"

using std::auto_ptr;

class CEncapsuled : public CHTMLItem{
public:
	CEncapsuled(auto_ptr<CHTMLItem> tag);
	virtual ~CEncapsuled();
	const char* get(const char* start, const char* end);
	auto_ptr<CHTMLItem> first;
	//auto_ptr<CHTMLItem> last;
	const char* content() const;

	friend ostream &operator<<(ostream& os, const CEncapsuled &scr);
private:
	char *str;
};

ostream &operator<<(ostream &os, const CEncapsuled &scr);

#endif /* _PAT_SCRIPT_H_102502 */
