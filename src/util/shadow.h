#ifndef _PAT_STR_SET_SHADOW_H_05152008
#define _PAT_STR_SET_SHADOW_H_05152008
///////////////////////////////////////////////////////////////////
// If a filename is given, a mapped file will be created explicitly,
// and you need to delete it manully.
//
#include "multi_hash.h"
#include "bitmap.h"
#include "assert.h"

class CStrSetShadow 
{
public:
	enum {Attach=0, Create=1, Overwrite=2, Anonymous=3};
	enum {putOK, putExist, putFull};
public:
	CStrSetShadow();
	CStrSetShadow(unsigned capacity, const char* mapfile=0);
	virtual ~CStrSetShadow();
	//
	// oflag : ORs of Create, Overwrite. 
	// Return Attach, Create, Overwrite, Anonymous
	// <0 , failed.
	int open(const char* mapfile, unsigned &capacity, int oflag=Attach);
	void close();
	inline unsigned size() const
	{ 
		return _size; 
	}
	inline unsigned capacity() const
	{
		return _capacity;
	}	
	inline bool full() const
	{
		assert(_size >= 0);
		return (unsigned)_size >= _capacity;
	}
	unsigned memory_size() const
	{
		return body_size(_capacity) + header_size();
	}
	static unsigned memory_size(unsigned capacity)
	{
		return body_size(capacity) + header_size();
	}
	
	int put(const string& s);
	bool has(const string& s) const;
private:
	int load_header(int fd);
	void save_header();
	void init(bool reset);
	static unsigned body_size(unsigned capacity);
	static unsigned header_size();
	static unsigned index_chunk(unsigned body_size);
private:
	// Blocked copyability - DO NOT IMPLEMENT 
	CStrSetShadow(CStrSetShadow const&); 
	CStrSetShadow& operator=(CStrSetShadow const&);
private:
	MultiHashMD5 _M_f;
	unsigned char* _M_p;
	CBitmap *_M_pmap;
	int _size;   // use '-1' to present invalid shadow file
	unsigned _capacity;
};
#endif // _PAT_STR_SET_SHADOW_H_05152008
