#include "shadow_chain.h"
#include "util/util.h"
#include <memory>
#include <sstream>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

using std::runtime_error;
using std::auto_ptr;
using std::ostringstream;

CShadowChain::CShadowChain(const string &prefix, unsigned hintCapacity)
{
	open (prefix, hintCapacity);
}

CShadowChain::~CShadowChain()
{
	close();
}

int 
CShadowChain::open(const string &prefix, unsigned hintCapacity)
{
	while (file_size(prefix+"."+tostring(_chain.size())) >= 0)
	{
		auto_ptr<CStrSetShadow> shadow(new CStrSetShadow);
		unsigned capacity;
		int result = shadow->open((prefix+"."+tostring(_chain.size())).c_str(), capacity);
		if (result != CStrSetShadow::Attach)
		{
			ostringstream oss;
			oss<<"Attach shadow file \""<<prefix<<'.'<<_chain.size()<<"\" failed"
			  ", code:"<<result;
			throw runtime_error(oss.str());
		}
		_chain.push_back(shadow.release());
	}
	if (errno != ENOENT)
	{
		ostringstream oss;
		oss<<"Get size of file \""<<prefix<<"."<<_chain.size()<<"\" failed:"
		   <<strerror(errno);
		throw runtime_error(oss.str());
	}
	if (_chain.empty())
	{
		auto_ptr<CStrSetShadow> shadow(new CStrSetShadow);
		int result = shadow->open((prefix+".0").c_str(), hintCapacity
		   , CStrSetShadow::Create|CStrSetShadow::Overwrite);
		if (result != CStrSetShadow::Create)
		{
			ostringstream oss;
			oss<<"Create shadow file \""<<prefix<<".0 failed, code:"<<result;
			throw runtime_error(oss.str());
		}
		_chain.push_back(shadow.release());
	}
	this->prefix = prefix;
	return _chain.size();
}

void 
CShadowChain::close()
{
	for (size_t i=0; i<_chain.size(); i++)
		delete(_chain[i]);
	if (prefix.size() > 0 && -1 == unlink((prefix+"."+tostring(_chain.size())).c_str()) 
	   && errno != ENOENT)
	{
		ostringstream oss;
		oss<<"Can not delete terminating (null)shadow \""<<prefix<<"."<<_chain.size()
		   <<"\":"<<strerror(errno);
		throw runtime_error(oss.str());
	}
	_chain.clear();
	return;
}

bool
CShadowChain::has(const string &str)
{
	for (size_t i=0; i<_chain.size(); i++)
		if (_chain[i]->has(str))
			return true;
	return false;
}

bool
CShadowChain::put(const string &str)
{
	if (this->has(str))
		return false;
	assert(_chain.size()>0);
	if (_chain.back()->full())
	{//  Create a new shadow
		unsigned capacity = _chain.back()->capacity();
		unsigned new_capacity = capacity < 640000 ? capacity*10 : capacity;
		auto_ptr<CStrSetShadow> new_shadow(new CStrSetShadow);
		int result = new_shadow->open((prefix+"."+tostring(_chain.size())).c_str()
		   , new_capacity, CStrSetShadow::Create | CStrSetShadow::Overwrite);
		if (result < 0)
		{
			ostringstream oss;
			oss<<"Create new shadow \""<<prefix<<"."<<_chain.size()<<"\" failed"
			  ", code:"<<result;
			throw runtime_error(oss.str());
		}
		_chain.push_back(new_shadow.release());
	}
	return _chain.back()->put(str) == CStrSetShadow::putOK;
}

size_t
CShadowChain::size() const
{
	size_t sum(0);
	for (size_t i=0; i<_chain.size(); i++)
		sum += _chain[i]->size();
	return sum;
}
