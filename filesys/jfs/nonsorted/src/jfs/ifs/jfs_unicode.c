/* $Id: jfs_unicode.c,v 1.1 2000/04/21 10:58:17 ktk Exp $ */

static char *SCCSID = "@(#)1.10  12/8/98 13:54:20 src/jfs/ifs/jfs_unicode.c, sysjfs, w45.fs32, 990417.1";
/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#define INCL_NOPMAPI
#include <os2.h>
#include <jfs_os2.h>
#include <jfs_types.h>
#include <jfs_filsys.h>
#include <unidefk.h>
#include <jfs_debug.h>

/*
 * uniupr.h defines the tables used to convert to upper or lower case
 */
#define static
#define UNIUPR_NOLOWER
#include <uniupr.h>

#define UNICASERANGE_DEFINED
#include <uni_inln.h>

/* Character string pool */
extern pool_t   *string_pool;

/*
 * This function converts a unicode string to a character string.
 *
 * target may be a user buffer, so we must use KernCopyOut to write to it.
 */

int32 jfs_strfromUCS(
char *      target,
UniChar *   source,
int32       len,
int32       source_len)
{
    int32   rc;
    int32   newlen;
    char    *pinned_string;

    pinned_string = (char *)allocpool(string_pool, 0);
    if (pinned_string == 0)
        return -1;

    rc = KernStrFromUcs(0,
                        pinned_string,
                        source,
                        MIN(JFS_PATH_MAX+1, len),
                        source_len);

    jEVENT((rc != 0), ("KernStrFromUcs returned %d\n", rc));

    if (! rc)
    {
        newlen = strlen(pinned_string);
        rc = KernCopyOut(target, pinned_string, newlen+1);
    }

    freepool(string_pool, (caddr_t *)pinned_string);

    if (rc)
        return -1;
    else
        return newlen;
}

/*
 * This function converts a character string to a UniChar string.
 */

int32 jfs_strtoUCS(
UniChar *   target,
char *      source,
int32       len)
{
    int32   rc;
    int32   source_len;
    int32   newlen;

    source_len = strlen(source);
    rc = KernStrToUcs(0, target, source, len+1, source_len);
    if (rc)
    {
        jEVENT(1, ("KernStrToUcs return %d\n", rc));
        return -1;
    }
    else
//      return UniStrlen(target);   /* Nobody uses this anyway */
        return 0;
}

#ifdef MMIOPH
/* d201828
 * Converts a unicode string to a character string.
 * Used by performance trace hooks.
 */
void PerfCpyStr(char *tgt, UniChar *src, int32 len) {
   while (len > 0) {
      *(tgt++) = *(src++);
      len--;
   }
   return;
}
#endif      //MMIOPH
