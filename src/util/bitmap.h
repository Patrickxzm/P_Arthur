#ifndef _PAT_URLBITMAP_H_032103
#define _PAT_URLBITMAP_H_032103
#include <iostream>
#include <cstring>
#include <string>
#include <limits>
#include "excep.h"
using std::string; 
using std::numeric_limits;

class CBitmap 
{
public:
	CBitmap(unsigned nbyte);
	CBitmap(unsigned char* ext, unsigned nbyte);
	virtual ~CBitmap();
	void reset();
	int val(unsigned idx);
	int set(unsigned idx);
	unsigned size() const 
	{	
		return _M_length * numeric_limits<unsigned char>::digits; 
	}
private:
	unsigned char* _M_p;
	bool _M_p_ext;
	unsigned _M_length;
};

std::ostream& operator<<(std::ostream& out, CBitmap& map);

#endif /* _PAT_URLBITMAP_H_032103 */
