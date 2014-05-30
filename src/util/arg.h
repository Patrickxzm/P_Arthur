#ifndef _PAT_ARG_H_100603_
#define _PAT_ARG_H_100603_
#include <stdexcept>
#include <sstream>
#include <vector>
#include <string>
using std::string;
using std::vector;
using std::istringstream;
using std::runtime_error;

class CArg 
{
public:
	class ArgVal
	{
	public:
		ArgVal():charPtr(0)
		{}
		ArgVal(const char* in)
		{
			charPtr = in;
		}
		inline const char* get() const
		{
			return charPtr;
		}
		inline operator const char* () const
		{
			return charPtr;
		}
		int INT() const;
	private:
		const char* charPtr;
	};
	CArg(int argc, char** argv);
	virtual ~CArg();
	vector<ArgVal> find(const char* prefix) const;
	ArgVal find1(const char* prefix) const;
	bool found(const char* prefix) const
	{
		return find(prefix).size() > 0;
	}
	vector<ArgVal> follow(const char* option) const;
private:
	char** argv;
	int argc;
};

#endif // _PAT_ARG_H_100603_
