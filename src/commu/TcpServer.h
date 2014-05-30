/************************************************************************* 
 * @ This module is useless in my system now, evev causes some problem, 
 *   so it is excluded in Makefile.          ? long long ago
 **************************************************************************/
#ifndef _CTCPSERVER_PAT_080202
#define _CTCPSERVER_PAT_080202
#include "TcpStream.hpp"
#include <vector>
#include <queue>
#include <list>
#include <string>
using std::string;
using std::vector;
using std::queue;
using std::list;

class CTCPServer : public TcpStream
{
public:
	CTCPServer();
	virtual ~CTCPServer();

	int listen(int port);
	int accept();
	void stop_listen();
protected:
	int listen_sock;
};

#endif /* _CTCPSERVER_PAT_080202 */
