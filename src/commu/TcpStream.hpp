#ifndef _PAT_TCPSTREAM_HPP_030807
#define _PAT_TCPSTREAM_HPP_030807
#include "TcpBuf.hpp"
#include <unistd.h>
#include <iostream>

using std::iostream;

class TcpStream : public iostream
{
public:
	typedef std::iostream iostream_type;
	TcpStream():iostream_type(0)
	{
		this->init(&_M_tcpbuf);
	}
	virtual ~TcpStream()
	{}
	void close()
	{
		if (!_M_tcpbuf.close())
			this->setstate(ios_base::failbit);
	}
	bool is_open()
	{
		return _M_tcpbuf.is_open();
	}
protected:
	TcpBuf _M_tcpbuf;
};
#endif // _PAT_TCPSTREAM_HPP_030807
