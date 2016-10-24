
/* This function extracts refresh url from the given string buf.
Return value may be in three cases:
1. NULL. indicating not refresh anything
2. '\0'. indicating refresh oneself
3. others. indicating refresh the returned value as url
*/

# include <ctype.h>
# include <string.h>
#ifdef DMALLOC
#include "dmalloc.h"
#endif

void trimblank(char* url); //trim blank symbols in the end of url

char* url_in_refresh(const char* content)
{
	if (content == NULL || *content == '\0') //no refresh
		return NULL;
		
	const char* index=content; //cursor to tail string
	
	while (*index == ' ' || *index == 9 || *index == '\n')  //S1
		index++ ; //omit blank symbols
	
	if( !isdigit(*index) && *index != '.' ) //S3
		return NULL;
		
	while( isdigit(*index) || *index == '.' ) // reach time field,S2
		index++; 

	if(*index != ' ' && *index != 9 && *index != '\n' && *index != ';' ) //S5
	{
		char* emp = new char;
		*emp = '\0';
		return emp; //return empty string , indicating refresh oneself
	}
	
	while( *index == ' ' || *index == 9 || *index =='\n' || *index == ';') //S4
		index++;
	
	if ( strncasecmp( "url", index,3) != 0 ) //S5
	{
		char* emp = new char;
		*emp = '\0';
		return emp;  //return empty string , indicating refresh oneself
	}
	
	else //S6
	index = index +3 ;//skip "url'
	while(*index == ' ' || *index == 9 || *index == '\n') //omit blank symbols
		index++;
	if(*index != '=' ) //S5
	{
		char * emp =new char;
		*emp = '\0';
		return emp; //return empty string, indicating refresh oneself
	}
	
	//S7
	index++ ; //skip '='
	while ( *index == ' ' || *index == 9 || *index == '\n' ) //omit blank symbols
		index++;
	if (*index != '\'') //S9
	{
		char* url = strdup(index); //the sub-string from index to the end is url  
		trimblank(url); //trim blank symbols in the end of url;
		return url;
	}
	
	//S8
	index++; //skip '\''
	while (*index == ' ' || *index == 9 || *index == '\n' ) //omit blank symbols
		index++;
	
	//S10
	const char* rquote = strchr(index, '\'') ;// find position of the right single quote
	if (rquote == NULL) // lose right single quote, S12
	{
		char* url = strdup(index); //the sub-string from index to the end is url 
		trimblank(url); //trim blank symbols in the end of url
		return url;
	}
	//S11
	{
		int len = rquote - index; // string length between single quotes
		char* url = new char[len+1];
		memcpy(url, index, len);
		url[len] = '\0';
		trimblank(url);
		return url;
	}
}


void trimblank(char* url)
{
	if(url == NULL || *url =='\0')
		return;
	
	int len = strlen(url);
	int tail = len -1 ; //the last character before '\0'
	while (tail>0 && isspace(url[tail]))
		tail--;
	url[tail+1] = '\0';
}
