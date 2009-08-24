// ISTRING.C - INI file string insertion routine for 4xxx / TCMD
//   Copyright 1992 - 1997, JP Software Inc., All Rights Reserved

#include <string.h>
#include <stdio.h>

#include "product.h"

#include "4all.h"

#include "inifile.h"

// store a string; remove any previous string for the same item
int ini_string(INIFILE *InitData, int *dataptr, char *string, int slen)
{
	unsigned int i;
	int old_len, ptype, move_cnt;
	unsigned int offset;
	unsigned int *fixptr;
	char *old_string;

	// calculate length of previous string, change in string space
	old_len = ((offset = (unsigned int)*dataptr) == INI_EMPTYSTR) ? 0 : (strlen((old_string = InitData->StrData + offset)) + 1);

	// holler if no room
	if ((InitData->StrUsed + slen + 1 - old_len) > InitData->StrMax)
		return 1;

	// if there is an old string in the middle of the string space, collapse
	// it out of the way and adjust all pointers
	if (offset != INI_EMPTYSTR) {

		if ((move_cnt = InitData->StrUsed - (offset + old_len)) > 0) {

			memmove(old_string, old_string + old_len, move_cnt);
			for (i = 0; (i < guINIItemCount); i++) {

				fixptr = (unsigned int *)((char *)InitData + ((char *)gaINIItemList[i].pItemData - (char *)&gaInifile));
				ptype = (int)(INI_PTMASK & gaINIItemList[i].cParseType);

				if (((ptype == INI_STR) || (ptype == INI_PATH)) && (*fixptr != INI_EMPTYSTR) && (*fixptr > offset))
					*fixptr -= old_len;
			}
		}

		InitData->StrUsed -= old_len;
	}

	// put new string in place and adjust space in use
	if (slen > 0) {
		memmove(InitData->StrData + InitData->StrUsed, string, slen + 1);
		*dataptr = InitData->StrUsed;
		InitData->StrUsed += (slen + 1);
	} else
		*dataptr = INI_EMPTYSTR;

	return 0;
}

