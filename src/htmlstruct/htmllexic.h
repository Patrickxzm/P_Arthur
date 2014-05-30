#ifndef PAT_CHTMLLEXIC_H_092702
#define PAT_CHTMLLEXIC_H_092702

#include "htmlitem.h"
#include <memory>
using std::auto_ptr;

class CHTMLLexic {
public:
	CHTMLLexic();
	CHTMLLexic(const char* html, int len);
	CHTMLLexic(const string &html);
	virtual ~CHTMLLexic();
	int input(const char* html, int len);
	int input(const string &html);
	auto_ptr<CHTMLItem> output();
private:
	const char* start;
	const char* ptr;
	const char* end;
	char* html;
};

#endif /* PAT_CHTMLLEXIC_H_092702 */
