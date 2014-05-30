#ifndef _PAT_PAGE_STATUS_H_03042011
#define _PAT_PAGE_STATUS_H_03042011

class page_status_t 
{
public:
	page_status_t()
	  : getPage(false), newURL(0), newAnchor(0), newParagraph(0)
	  , outlinkSaved(0) , newScript(false), newPlain(false)
	{}
	inline unsigned newContentNum() const
	{
		if (!getPage)
			return 0;
		if (newScript || newPlain)
			return 1;
		return newURL + newAnchor + newParagraph + outlinkSaved;
	}
	inline unsigned newTextNum() const
	{
		if (!getPage)
			return 0;
		if (newScript || newPlain)
			return 1;
		return newAnchor + newParagraph;
	}
public:
	bool getPage;
	unsigned newURL;
	unsigned newAnchor;
	unsigned newParagraph;
	unsigned outlinkSaved;
	bool newScript;
	bool newPlain;
};

#endif  // _PAT_PAGE_STATUS_H_03042011
