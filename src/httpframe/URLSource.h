#ifndef CURSOURCE_XZM_052802
#define CURSOURCE_XZM_052802
#include <time.h>

class CURLSource {
public:
	virtual int get(char* url) = 0;
	CURLSource();
	int set_limit(int limit);   /* number of URLs can get from it per second.*/
protected:
	int control();
private:
	int limit;
	int count;
	time_t start_time;
	
};

#endif /* CURSOURCE_XZM_052802 */
