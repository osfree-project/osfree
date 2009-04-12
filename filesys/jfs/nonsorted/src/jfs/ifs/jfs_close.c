/* $Id: jfs_close.c,v 1.1 2000/04/21 10:58:05 ktk Exp $ */

static char *SCCSID = "@(#)1.5  7/30/98 14:10:42 src/jfs/ifs/jfs_close.c, sysjfs, w45.fs32, 990417.1";
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
 *
 *
 *
*/
/*
 *      jfs_close.c
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"

#include "jfs_types.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_debug.h"

#define OPEN_FLAGS_LARGEFILE    0x08000000 /* Officially defined in bsedosp.h */

/*
 * NAME:        jfs_close(vp, flag, vinfo, crp)
 *
 * FUNCTION:    release per open counts
 *
 * PARAMETER:
 *
 * RETURN:      0
 *
 *  IWRITE_LOCK must be help on entry
 */
jfs_close (
        struct vnode    *vp,    /* object vnode */
        int32           flag,   /* flags from the file pointer */
        caddr_t         vinfo,  /* for remote */
        struct ucred    *crp)   /* credential */
{
        inode_t *ip = VP2IP(vp);
        struct gnode *gp = VP2GP(vp);

NOISE(1,("jfs_close(ip:0x%08x flag:0x%08x)\n", ip, flag));

        /* keep count of opens by type
         */
        switch (flag & (OPEN_ACCESS_READWRITE | OPEN_ACCESS_WRITEONLY))
        {
            case (OPEN_ACCESS_READWRITE):
                gp->gn_wrcnt--;
                gp->gn_rdcnt--;
                break;

            case (OPEN_ACCESS_WRITEONLY):
                gp->gn_wrcnt--;
                break;

            default:
                gp->gn_rdcnt--;
        }

        /* fix up sharing mode
         */
        switch (flag & (OPEN_SHARE_DENYREAD))
        {
            case(OPEN_SHARE_DENYWRITE):
                gp->gn_dwcnt--;
                break;

            case(OPEN_SHARE_DENYREAD):
                gp->gn_drcnt--;
                break;

            case(OPEN_SHARE_DENYREADWRITE):
                gp->gn_flags &= ~GNF_NSHARE;
                break;
        }

    if ((flag & OPEN_FLAGS_LARGEFILE) == 0)
        gp->gn_lgcnt--;

NOISE(1,("jfs_close(rc:0)\n"));

        return 0;
}
#endif
