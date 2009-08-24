#include <string.h>
#include <io.h>
#include "md5.h"
#include "global.h"

enum boolean checkdigests(md5_byte_t *digest1, md5_byte_t *digest2)
{
	short i;
	for(i=0;i<16;i++)
	{
		if(digest1[i]!=digest2[i])
			return(false);
	}
	return(true);
}

/* Avoiding printf completely, as VPRINTER is almost 2k of overhead  */
/* Could use the small (%f-free) PRF.C as easier to use replacement. */
void PutString(char * stri)
{
	if (stri==0L)
		return;
	(void)write(1 /* stdout */, stri, strlen(stri));
}

