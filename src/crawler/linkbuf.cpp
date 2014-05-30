#include "linkbuf.h"

CLinksBuffer::CLinksBuffer(const char* name): filename(name)
{
}

bool 
CLinksBuffer::put(const CQMMsg &link)
{
	if (ifs.is_open())
	{
		ifs.close();
	}
	if (!ofs.is_open())
	{
		ofs.open(filename.c_str(), 100, 100, 100, false);
		out_n = 0;
	}
	if (out_n >= 5000)
	{
		if (!ofs.check())
			return false;
		out_n = 0;
	}
	out_n++;
	return ofs<<link.getMsg()<<endl;
}

bool
CLinksBuffer::get(CQMMsg &link)
{
	if (ofs.is_open())
	{
		ofs.close();
		out_n = 0;
	}
	if (!ifs.is_open())
	{
		ifs.open(filename.c_str());
	}
	const int max_link_len = 256;
	char link_buffer[max_link_len];
	if (!ifs.getline(link_buffer, max_link_len))
	{
		if (!ifs.check())
			return false;
	}
	if (!ifs.getline(link_buffer, max_link_len))
	{
		return false;
	}
	return link.setMsg(link_buffer);
}

//#ifdef _TEST

int
main(int argc, char* argv[])
{
}

//#endif //_TEST
