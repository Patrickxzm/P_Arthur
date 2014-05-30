#ifndef _PAT_INDEX_QUEUE_HPP_08282011
#define _PAT_INDEX_QUEUE_HPP_08282011

#include <list>
#include <map>

template <class _indexed>
class index_queue : public std::list<_indexed>
{
public: 
	//precondition ==> _indexed::key_type _indexed::key() const;
	typedef _indexed				value_type;
	typedef typename value_type::key_type		key_type;
	typedef std::list<value_type> 			base_type;
private:
	typedef std::map<key_type, typename base_type::iterator> index_type;
public:
	//queue-based.
	bool pop_front(value_type &v)
	{
		if (!this->empty())
		{
			v = this->base_type::front();
			this->base_type::pop_front();
			index.erase(v.key());
			return true;
		}
		return false;
	}
	typename base_type::iterator push_back(const value_type &v)  //end(): key already exists.
	{
		if (has(v.key()))
			return this->end();
		this->base_type::push_back(v);
		typename base_type::iterator iter = this->end();
		--iter;
		index[iter->key()] = iter;
		return iter;
	}
	//index-based.
	inline bool has(const key_type &key) const
	{
		return index.find(key) != index.end();
	}
	
	typename base_type::iterator find(const key_type &key)
	{
		typename index_type::const_iterator cit = index.find(key);
		if (cit == index.end())
			return this->end();
		return cit->second;
	}
	
	bool find(value_type &v) const
	{
		typename index_type::const_iterator cit = index.find(v.key());
		if (cit == index.end())
			return false;
		v = *cit->second;
		return true;
	}
	// false: key not found; true: copy removed value back
	bool remove(value_type &v) 
	{
		typename index_type::iterator iter = index.find(v.key());
		if (iter==index.end())
			return false;
		v = *iter->second;
		erase(iter->second);
		index.erase(iter);
		return true;
	}
private: 
	index_type index;
};
#endif // _PAT_INDEX_QUEUE_HPP_08282011
