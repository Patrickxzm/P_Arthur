#include "shadow.h"
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdexcept>
#include <sstream>
#include <assert.h>
#include "util.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif 

#ifndef MAP_ANONYMOUS 
#define MAP_ANONYMOUS MAP_ANON
#endif 
using std::ostringstream;
using std::istringstream;
using std::runtime_error;
using std::range_error;
using std::overflow_error;

unsigned
CStrSetShadow::index_chunk(unsigned body_size)
{
/***************************************************************
 *  body_size * 8 = 2 ^ index_chunk ==> body_size = 2 ^ (index_chunk - 3)
 ***************************************************************/
	unsigned chunk = 3;
	while (body_size > 1)
	{
		chunk ++;
		body_size /= 2;
	}
	return chunk;
}


unsigned
CStrSetShadow::header_size()
{
	// shadow header is like "capacity 'tab' size 'eol'"
	unsigned unsigned_width = 10;
	return 2 * unsigned_width + 2;  
}

CStrSetShadow::CStrSetShadow()
: _M_p(0), _M_pmap(0), _size(-1)
{
}

CStrSetShadow::CStrSetShadow(unsigned capacity, const char* filename)
: _M_p(0), _M_pmap(0)
{
	open(filename, capacity, Create);
}

unsigned 
CStrSetShadow::body_size(unsigned capacity)
{
/***************************************************************
 *   nbyte >= 2 * capacity 
 ***************************************************************/
	unsigned nbyte = 1;
	while (nbyte < 2 * capacity)
	{
		nbyte *= 2;
	}
	return nbyte;
}

int
CStrSetShadow::open(const char* mapfile, unsigned &capacity, int oflag)
{
	if (!mapfile)
	{  // Anonymous mode
		this->_capacity = body_size(capacity)/2;
		capacity = this->_capacity;
		this->_size = 0;
		_M_p = (unsigned char*)mmap(0, memory_size(capacity), PROT_READ|PROT_WRITE,
				MAP_NORESERVE|MAP_SHARED|MAP_ANONYMOUS, -1, 0);
		init(true);
		return Anonymous;
	}
	int fd=-1;
	if ((fd=::open(mapfile, O_RDWR)) != -1 && !(oflag & Overwrite)) 
	{
		load_header(fd);
		capacity = _capacity;
		// Check file's actual size.
		int fsize = file_size(mapfile);
		if (fsize == -1)
		{
			ostringstream oss;
			oss<<"file_size(\""<<mapfile<<"\") failed!:"
				<<strerror(errno);
			throw runtime_error(oss.str());
		}
		if ((unsigned)fsize != memory_size(_capacity))
		{
			ostringstream oss;
			oss<<"Bad shadow file_size("
			  <<fsize<<") doesn't match capacity("<<capacity
			  <<")";
			throw runtime_error(oss.str());
		}
		_M_p = (unsigned char*)mmap(0, memory_size(capacity), PROT_READ|PROT_WRITE, 
				MAP_NORESERVE|MAP_SHARED, fd, 0);
		::close(fd);
		init(false);
		return Attach;
	}
	// Create or Overwrite.
	int ret;
	if (fd ==-1)
	{
		if (!(oflag & Create) || (fd = ::open(mapfile, O_CREAT|O_RDWR, 0644)) == -1)
		{
			ostringstream oss;
			oss<<"CStrSetShadow():open shadow file("<<mapfile
				<<") failed!:"<<strerror(errno);
			throw runtime_error(oss.str());
		}
		ret = Create;
	}
	else
		ret = Overwrite;
	if (capacity == 0)
	{
		ostringstream oss;
		oss<<"CStrSetShadow()::open():Can not create a shadow file with \"0\" capacity: "
			<<mapfile;
		throw runtime_error(oss.str());
	}
	this->_capacity = body_size(capacity)/2;
	capacity = this->_capacity;
	this->_size = 0;
	ftruncate(fd, memory_size(capacity));
	_M_p = (unsigned char*)mmap(0, memory_size(capacity), PROT_READ|PROT_WRITE, 
			MAP_NORESERVE|MAP_SHARED, fd, 0);
	::close(fd); 
	init(true);
	return ret;
}

void
CStrSetShadow::init(bool reset)
{
	if (_M_p == MAP_FAILED) 
	{
		ostringstream oss;
		oss<<"CStrSetShadow():mmap failed!:"<<strerror(errno);
		_M_p = 0;
		throw runtime_error(oss.str());
	}
	_M_pmap = new CBitmap(_M_p+header_size(), body_size(_capacity));
	if (reset) { 
		_M_pmap->reset();
	}
	unsigned chunk_size = this->index_chunk(body_size(_capacity));
	if (chunk_size > (int)numeric_limits<unsigned>::digits)
	{
		throw range_error("CStrSetShadow::init():"
		  " Not support such a large capacity.");
	}
	unsigned chunk_num = 128 / chunk_size;
	if (chunk_num > 8)
		chunk_num = 8;
	_M_f.init(chunk_size, chunk_num);
	return;
}

CStrSetShadow::~CStrSetShadow()
{
	close();
}

void 
CStrSetShadow::close()
{
	if (_M_p != 0 && _M_p != MAP_FAILED)
	{
		save_header();
		msync(_M_p, memory_size(_capacity), MS_SYNC);
		munmap(_M_p, memory_size(_capacity));
	}
	if (_M_pmap != 0)
		delete(_M_pmap);
}

int 
CStrSetShadow::load_header(int fd)
{
	char buf[header_size()+1];
	lseek(fd, 0, SEEK_SET);
	read(fd, buf, header_size());
	istringstream iss(buf);
	if (!(iss>>_capacity>>_size))
		return -1;		
	if (_size < 0)
		return -2;
	return 0;
}

void
CStrSetShadow::save_header()
{
	ostringstream header;
	header<<_capacity<<'\t'<<_size<<'\n';
	assert(header.str().length() <= header_size());
	memcpy(_M_p, header.str().c_str(), header.str().length());	
	return;
}

bool 
CStrSetShadow::has(const string& s) const
{
	vector<unsigned> shadow = _M_f(s);
	for (unsigned i=0; i<shadow.size(); i++)
	{
		if (_M_pmap->val(shadow[i]) == 0)
			return false;
	}
	return true;
}

int 
CStrSetShadow::put(const string& s)
{
	assert(_size >= 0);
	// cast to unsigned be dangerous without assert(_size >= 0)
	if ((unsigned)_size >= _capacity)   
		return putFull;
	bool freebit = false;
	vector<unsigned> shadow = _M_f(s);
	for (unsigned i=0; i<shadow.size(); i++)
	{
		if (_M_pmap->val(shadow[i]) == 0)
		{
			freebit = true;
			_M_pmap->set(shadow[i]);
		}
	}
	if (freebit)
	{
		_size ++;
		return putOK;
	}
	return putExist;
}

