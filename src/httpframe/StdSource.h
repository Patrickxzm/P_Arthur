#ifndef CSTDSOURCE_XZM_062702
#define CSTDSOURCE_XZM_062702

#include "URLSource.h"

class CSTDSource : public virtual CURLSource{
public:
	CSTDSource() {
	}
	int get(char* url){
		fgets(url, 127, stdin);
		return 0;
	}
};
#endif /* CSTDSOURCE_XZM_062702 */
