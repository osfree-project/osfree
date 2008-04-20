#include "md5.h"
#include "global.h"

enum boolean checkdigests(md5_byte_t *digest1, md5_byte_t *digest2)
{
	short i;
	for(i=0;i<16;i++)
	{
		if(digest1[i]!=digest2[i]) return(false);
	}
	return(true);
}
