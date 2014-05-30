#include "urlfilter.h"
#include "url.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const char* non_txt_ext[] = {
	/* image */
	"gif", "ai", "jpg", "tif", "png", "jpe", "icojpg", "bmp", "ico", "jpeg", 
	"pcx", "tiff", "wmf", "psd", "tga", "pic", "pcd", "dib", "rle", "iff",
	"lbm", "jif", "dcx",
	/* audio */
	"wav", "mp3", "mid", "wma", "mp2", "mp1", "aif", "au", "snd", "cda",
	"mpl", "m3u", "mjf", "as", "voc", "xm", "s3m", "stm", "mod", "dsm",
	"far", "ult", "mtm", "mpga", "mpa",
	/* vedio */
	"rm", "ra", "avi", "asf", "wmv", "mpg", "mpe", "mpeg",  "mpv", 
	"mov", "qt", "ram", 
	/* compressed */
	"tar", "zip", "gtar", "ustar", "gz", "z", "arj", "rar",
	/* binary */
	"bin", "lib", "a", "o", "dll",
	/* other */
	"java", "reg", "jav", "asm", "latex", "ps", "pdf", "doc", "css" 
	/* "cgi" is deleted. */
};
static int not_sort = 1;

#define NELEMS(array) (sizeof(array) / sizeof(array[0]))
int scmp(const void *p1, const void *p2);

CURLFilter::CURLFilter()
{
}

CURLFilter::~CURLFilter()
{
}

int CURLFilter::pass(const char* urlstr)
{
	CUrl url(urlstr);
	if (url.init() == -1) return false;
	return pass(&url);
}

int CURLFilter::pass(CUrl* purl)
{
	if (not_sort){
		qsort(non_txt_ext, NELEMS(non_txt_ext), sizeof(non_txt_ext[0]), scmp);
		not_sort = false;
	}
	const char* ext = purl->local_ext();
	if (ext == NULL) return true;
	if ((char*)bsearch(&ext, 
				non_txt_ext, NELEMS(non_txt_ext), sizeof(non_txt_ext[0]), 
				scmp) != NULL)
		return false;
	if (strlen(purl->str) > 200) return false;
	/*if (strstr(purl->local, "cgi-bin")) return false; */
	if (strstr(purl->local, "javascript")) return false;

	return true;
	
}

/* scmp: string compare of *p1 and *p2 */
int scmp(const void *p1, const void *p2)
{
	char *v1, *v2;
	v1 = *(char **) p1;
	v2 = *(char **) p2;
	return strcasecmp(v1, v2);
}

