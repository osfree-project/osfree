/* $Id: jfs_dasdlim.c,v 1.1.1.1 2003/05/21 13:36:32 pasha Exp $ */

static char *SCCSID = "@(#)1.9  11/1/99 13:01:39 src/jfs/ifs/jfs_dasdlim.c, sysjfs, w45.fs32, fixbld";
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
 *   MODULE_NAME:	jfs_dasdlim.c
 *
 *   COMPONENT_NAME: 	sysjfs
 *
*/

/*
 * Change History :
 *
  */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#define INCL_LONGLONG
#include <os2.h>
#include <fsd.h>
#include <priv/jfs_acl.h>

#include "jfs_os2.h"
#include "jfs_inode.h"
#include "jfs_dirent.h"
#include "jfs_cntl.h"
#include "jfs_proto.h"
#include "jfs_debug.h"
#include "jfs_dasdlim.h"
#include "jfs_dtree.h"
#include "jfs_superblock.h"
#include "jfs_txnmgr.h"
#include "jfs_dmap.h"
#include "uni_inln.h"

/*
 * External references
 */
extern pool_t		*dirent_pool;
extern struct ucred	*pcred;
extern pool_t		*unipool;
extern struct InfoSegGDT *GIS;
extern struct InfoSegLDT * _Seg16 pLIS;

enum_instance_t	*alloc_enum(void);
int32		free_enum(enum_instance_t *);
int32		verify_enum(enum_instance_t *);
int32		jfs_strtoUCS(UniChar *, char *, int32);
int32		jfs_strfromUCS(char *, UniChar *, int32, int32);
int32		pathlookup(struct vfs *, pathlook_t *, struct ucred *);
int32		readSuper(inode_t *, cbuf_t **bp);
struct vfs *	VPBtoVFS(SHANDLE);
struct vfs *	PATHtoVFS(char *);
void		sort_and_lock(inode_t *, int32, inode_t **);
uint32 _Optlink	percent(uint32, uint32);
int32		User_ID(void);

/*
 * Forward references
 */
void	lock_header(int32 tid, inode_t *ip);
void	modify_superblock(inode_t *ipmnt, int32 bits_on, int32 bits_off);
int32	jfs_enumDLIM(struct vnode *, struct jfs_DLIM_parms *, char *, int32 *);
void	check_alert(inode_t *ip, uint32 new_usage);
void	send_alert(inode_t *ip, uint32 freeblocks, uint32 interval);
UniChar	*build_path(inode_t *ip, UniChar *path_buf);

/*
 * Global data
 */
pool_t	*dlim_pool = 0;
pool_t	*big_dlim_pool = 0;

event_t	alert_event;
int32	server_running = 0;
struct jfs_dlim_init_info	InitInfo;
char	*sessionFlags = 0;
KernVMLock_t	session_lock_handle;
MUTEXLOCK_T	dasd_alert_lock;
pool_t		*alert_pool = 0;
struct dasd_alert	*alert_anchor = 0;
struct dasd_alert	*alert_next = 0;


#ifdef	_JFS_DEBUG
int32	NOISY = 0;
#endif	/* _JFS_DEBUG */

#define ALERT_LOCK()	MUTEXLOCK_LOCK(&dasd_alert_lock)
#define ALERT_UNLOCK()	MUTEXLOCK_UNLOCK(&dasd_alert_lock)

/*
 * NAME:	dasd_init()
 *
 * FUNCTION:	Initialize memory pools
 */
void	dasd_init(
	struct vfs	*vfsp)
{
	inode_t	*ipmnt;
	int32	rc;

	/* Return if initialization already done */
	if (big_dlim_pool)
		return;

	if (dlim_pool == 0)
	{
		rc = initpool(65536, 128, sizeof(struct dasd_usage), 0,
			      "DLIM", &dlim_pool, POOL_TAKE_LOCK);
		if (rc)
			goto alloc_failed;
	}
	rc = initpool(65536, 61, BIG_DLIM_SIZE, 0, "BIG_DLIM", &big_dlim_pool,
		      POOL_TAKE_LOCK);
	if (rc == 0)
		return;

alloc_failed:
	/*
	 * If memory allocation failed, we can't guarantee DASD usage, so
	 * lets force the mount to recalculate usage on the next boot.
	 */
	ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;
	modify_superblock(ipmnt, JFS_DASD_PRIME, 0);
	vfsp->vfs_flag &= ~VFS_DASDLIM;
	vfsp->vfs_flag |= VFS_DASDPRIME;
}

/*
 * NAME:	DASD_FSCTL()
 *
 * FUNCTION:	Handle Common DASD Limit FSCTL commands
 *
 * RETURN:	NO_ERROR - success
 */

APIRET APIENTRY DASD_FSCTL (
        union argdat    *pArgDat,
        unsigned long   iArgType,
        unsigned long   func,
        char            *pParm,
        unsigned long   lenParm,
        unsigned short  *plenParmIO,
        char            *pData,
        unsigned long   lenData,
        unsigned short  *plenDataIO)
{
	char		*apath;
	int64		avail;
	DASDCHECK0	*c0;
	DASDCHECK0PARM	*check0 = 0;
	DASDINFO0	*d0;
	int32		i;
	DASDINFO0PARM	info0;
	DASDINIT0	*i0;
	DASDINIT0PARM	init0;
	int32		j;
	inode_t		*ip;
	inode_t		*iplist[1];
	inode_t		*ipmnt;
	KernVMLock_t	lockHandle;
	uint64		min_avail;
	UniChar		*Path;
	int32		path_len;
	pathlook_t	pls;
	int32		rc;
	int32		shift;
	int32		tid;
	struct vfs	*vfsp;

jEVENT(0,("In DASD_FSCTL, func = 0x%x\n", func));

	if (func == PBDASD_ENUM)
		return ERROR_NOT_SUPPORTED;

	if (iArgType != FSCTL_PATHNAME)
		return ERROR_INVALID_PARAMETER;

	vfsp = VPBtoVFS(pArgDat->cd.pcdfsi->cdi_hVPB);
	ipmnt = ((inode_t *)vfsp->vfs_data)->i_ipmnt;

/*
 * PBDASD_CTLVOL
 */
	if (func == PBDASD_CTLVOL)
	{
		if (lenParm < sizeof(init0))
			return ERROR_INVALID_PARAMETER;

		rc = KernCopyIn(&init0, pParm, sizeof(init0));
		if (rc)
			return rc;
		i0 = &init0.dlinit_data;

		if (init0.dlinit_level != 0)
			return ERROR_INVALID_LEVEL;

jEVENT(0,("PBDASD_CTLVOL called with i0_CtlFlag = 0x%x\n", i0->i0_CtlFlag));

		switch (i0->i0_CtlFlag)
		{
		case DASD_CTL_INSTALL:
			if (vfsp->vfs_flag & VFS_DASDLIM)
			{
				init0.dlinit_rc = NERR_DASDAlreadyInstalled;
				break;
			}
			modify_superblock(ipmnt, JFS_DASD_PRIME, 0);
			vfsp->vfs_flag |= VFS_DASDPRIME;
			init0.dlinit_rc = NO_ERROR;
			break;
		case DASD_CTL_REMOVE:
			if ((vfsp->vfs_flag & (VFS_DASDLIM|VFS_DASDPRIME)) == 0)
			{
				init0.dlinit_rc = NERR_DASDNotInstalled;
				break;
			}
			modify_superblock(ipmnt, 0,
					  JFS_DASD_ENABLED|JFS_DASD_PRIME);
			vfsp->vfs_flag &= ~(VFS_DASDLIM|VFS_DASDPRIME);
			init0.dlinit_rc = NO_ERROR;
			break;
		case DASD_CTL_RECALC:
			if ((vfsp->vfs_flag & VFS_DASDLIM) == 0)
			{
				init0.dlinit_rc = NERR_DASDNotInstalled;
				break;
			}
			modify_superblock(ipmnt, JFS_DASD_PRIME, 0);
			vfsp->vfs_flag |= VFS_DASDPRIME;
			init0.dlinit_rc = NO_ERROR;
			break;
		case DASD_CTL_QUERY:
			if (vfsp->vfs_flag & VFS_DASDLIM)
				i0->i0_CtlFlag =
					(vfsp->vfs_flag & VFS_DASDPRIME)?
					DLIM_SYNCH:DLIM_RUNNING;
			else
				i0->i0_CtlFlag =
					(vfsp->vfs_flag & VFS_DASDPRIME)?
					DLIM_INSTALL:0;
			init0.dlinit_rc = NO_ERROR;
jEVENT(0,("DASD_CTL_QUERY: i0_CtlFLag = 0x%x\n", i0->i0_CtlFlag));
			break;
		default:
			init0.dlinit_rc = ERROR_INVALID_FUNCTION;
		}
jEVENT(0,("PBDASD_CTLVOL returning = 0x%x\n", init0.dlinit_rc));
        	*plenParmIO = sizeof(init0);
		rc = KernCopyOut(pParm, &init0, sizeof(init0));
		return rc;
	}
/*
 * COMMON:
 *	PBDASD_CHECK
 *	PBDASD_ADD
 *	PBDASD_DEL
 *	PBDASD_GETINFO
 *	PBDASD_SETINFO
 */
	/* At this point, func is one of: PBDASD_ADD, PBDASD_DEL,
	 * PBDASD_GETINFO, PBDASD_SETINFO, or PBDASD_CHECK.  For all of these
	 * functions, we need to resolve the path to a directory or file.
	 */
	DLIM_init(&pls.dlim, (func == PBDASD_CHECK)?DLIM_RELEASE:DLIM_READONLY);

	Path = (UniChar *)allocpool(unipool, 0);
	if (Path == 0)
		return ENOMEM;
	if (jfs_strtoUCS(Path, pArgDat->cd.pPath, MAXPATHLEN) == -1)
	{
		rc = ERROR_BUFFER_OVERFLOW;
		goto free_path;
	}
	pls.path = Path;

	rc = pathlookup(vfsp, &pls, pcred);
	if (rc)
		goto free_path;

	if (pls.dlim.num_limits == 0)
		jfs_rele(pls.dvp);

	shift = ipmnt->i_l2bfactor;
/*
 * PBDASD_CHECK
 */
	if (func == PBDASD_CHECK)
	{
		if (lenParm < sizeof(DASDCHECK0PARM))
		{
			rc = ERROR_INVALID_PARAMETER;
			goto dasdcheck_out;
		}

		/*
		 * Lock user buffer, so we won't page fault on it.
		 */
		rc = KernVMLock(VMDHL_WRITE, pParm, sizeof(DASDCHECK0PARM),
				&lockHandle, (KernPageList_t *)-1, 0);
		if (rc)
			goto dasdcheck_out;

		check0 = (DASDCHECK0PARM *)pParm;

		c0 = &check0->dlcheck_data;

		if (check0->dlcheck_level != 0)
		{
			rc = ERROR_INVALID_LEVEL;
			goto dasdcheck_out;
		}

		if ((vfsp->vfs_flag & VFS_DASDLIM) == 0)
		{
			check0->dlcheck_rc = (vfsp->vfs_flag & VFS_DASDPRIME) ?
						NERR_DASDNotRunning :
						NERR_DASDNotInstalled;
			goto dasdcheck_out;
		}

		apath = pArgDat->cd.pPath;
		path_len = strlen(apath);
		min_avail = ipmnt->i_ipbmap->i_bmap->db_nfree;
		c0->dc0_usage = (ipmnt->i_ipbmap->i_bmap->db_mapsize -
				 min_avail) << shift;

		if (pls.statlast == PL_ISDIR)
		{
			ip = VP2IP(pls.vp);
			IREAD_LOCK(ip);
			if (DASDLIMIT(&ip->i_DASD))
			{
				avail = DASDLIMIT(&ip->i_DASD) -
					DASDUSED(&ip->i_DASD);
				if (avail < 0)
					avail = 0;
				if (avail < min_avail)
				{
					min_avail = avail;
					c0->dc0_usage = ((unsigned long)
							 DASDUSED(&ip->i_DASD))
							<< shift;
				}
			}
			IREAD_UNLOCK(ip);
		}
		for (i = pls.dlim.num_limits; i > 0; i--)
		{
			ip = pls.dlim.pLimits[i-1];
			IREAD_LOCK(ip);
			if (DASDLIMIT(&ip->i_DASD))
			{
				avail = DASDLIMIT(&ip->i_DASD) -
					DASDUSED(&ip->i_DASD);
				if (avail < 0)
					avail = 0;
				if (avail < min_avail)
				{
				    min_avail = avail;
				    c0->dc0_usage = ((unsigned long)
						     DASDUSED(&ip->i_DASD))
						    << shift;
				    if (i == 1)
					path_len = 3;	/* length of "X:\" */
				    else
				    {
					/*
				 	* Starting past the first \, move
				 	* along string until we hit the
				 	* one past our directory.
				 	*/
					path_len = 4;	/* just past first \ */
					for (j = 1; j < i; j++)
					{
						while (apath[path_len] != '\\')
						{
							ASSERT(apath[path_len]);
							path_len++;
						}
					}
				    }
				}
			}
			IREAD_UNLOCK(ip);
		}
		check0->dlcheck_rc = 0;
		strncpy(check0->dlcheck_path, apath, path_len);
		check0->dlcheck_path[path_len] = 0;
		c0->dc0_avail = ((unsigned long)min_avail) << shift;
dasdcheck_out:
		if (check0)
		{
			rc = KernVMUnlock(&lockHandle);
			ASSERT(rc);
		}

		if (rc == 0)
			*plenParmIO = sizeof(DASDCHECK0PARM);
		else
			*plenParmIO = 0;

		if ((pls.statlast == PL_EXIST) || (pls.statlast == PL_ISDIR))
			jfs_rele(pls.vp);

		goto free_path;
	}

/*
 * COMMON:
 *	PBDASD_ADD
 *	PBDASD_DEL
 *	PBDASD_GETINFO
 *	PBDASD_SETINFO
 */
	if (pls.statlast != PL_ISDIR)
	{
		rc =  ERROR_FILE_NOT_FOUND;
		if (pls.statlast == PL_EXIST)
			goto release_vnode;
		else
			goto free_path;
	}

	if (lenParm < sizeof(info0))
	{
		rc = ERROR_INVALID_PARAMETER;
		goto release_vnode;
	}
	rc = KernCopyIn(&info0, pParm, sizeof(info0));
	if (rc)
		goto release_vnode;
	d0 = &info0.dlags_data;

	if (info0.dlags_level != 0)
	{
		info0.dlags_rc = ERROR_INVALID_LEVEL;
		goto release_vnode;
	}
	if ((vfsp->vfs_flag & VFS_DASDLIM) == 0)
	{
		info0.dlags_rc = (vfsp->vfs_flag & VFS_DASDPRIME) ?
				 NERR_DASDNotRunning : NERR_DASDNotInstalled;
		goto release_vnode;
	}

	ip=VP2IP(pls.vp);

	switch (func)
	{
/*
 * PBDASD_ADD
 */
	case PBDASD_ADD:
		if ((d0->d0_thresh > 99) || (d0->d0_delta > 99))
		{
			info0.dlags_rc = ERROR_INVALID_PARAMETER;
			break;
		}
		IWRITE_LOCK(ip);
		if (DASDLIMIT(&ip->i_DASD))
		{
			/* DASD Limit already set.  Not allowed. */
			IWRITE_UNLOCK(ip);
			info0.dlags_rc = NERR_LimitExists;
			goto release_vnode;
		}

		if ((d0->d0_flag & DASD_VALIDATE_LIMIT_OFF) ||
		    (DASDUSED(&ip->i_DASD) <= (d0->d0_max >> shift)))
		{
			txBegin(ipmnt, &tid, 0);
			lock_header(tid, ip);
			setDASDLIMIT(&ip->i_DASD, d0->d0_max >> shift);
			ip->i_DASD.thresh = d0->d0_thresh;
			ip->i_DASD.delta = d0->d0_delta;
			iplist[0] = ip;
			info0.dlags_rc = txCommit(tid, 1, iplist, 0);
			txEnd(tid);
		}
		else
			info0.dlags_rc = NERR_DASDMaxValidationFailed;
		d0->d0_use = DASDUSED(&ip->i_DASD) << shift;
		IWRITE_UNLOCK(ip);

		break;
/*
 * PBDASD_DEL
 */
	case PBDASD_DEL:
		IWRITE_LOCK(ip);
		if (DASDLIMIT(&ip->i_DASD) == 0)
			info0.dlags_rc = NERR_LimitNotFound;
		else
		{
			txBegin(ipmnt, &tid, 0);
			lock_header(tid, ip);
			setDASDLIMIT(&ip->i_DASD, 0);
			ip->i_DASD.thresh = 0;
			ip->i_DASD.delta = 0;
			iplist[0] = ip;
			info0.dlags_rc = txCommit(tid, 1, iplist, 0);
			txEnd(tid);
		}
		IWRITE_UNLOCK(ip);

		break;
/*
 * PBDASD_GETINFO
 */
	case PBDASD_GETINFO:
		IREAD_LOCK(ip);
		d0->d0_max = DASDLIMIT(&ip->i_DASD) << shift;
		d0->d0_use = DASDUSED(&ip->i_DASD) << shift;
		d0->d0_flag = 0;
		d0->d0_thresh = ip->i_DASD.thresh;
		d0->d0_delta = ip->i_DASD.delta;
		info0.dlags_rc = 0;
		IREAD_UNLOCK(ip);
		break;
/*
 * PBDASD_SETINFO
 */
	case PBDASD_SETINFO:
		if ((d0->d0_thresh > 99) || (d0->d0_delta > 99))
		{
			info0.dlags_rc = ERROR_INVALID_PARAMETER;
			break;
		}
		IWRITE_LOCK(ip);
		if (DASDLIMIT(&ip->i_DASD) == 0)
			info0.dlags_rc = NERR_LimitNotFound;
		else if ((d0->d0_flag & DASD_VALIDATE_LIMIT_OFF) ||
			 (DASDUSED(&ip->i_DASD) <= (d0->d0_max >> shift)))
		{
			txBegin(ipmnt, &tid, 0);
			lock_header(tid, ip);
			setDASDLIMIT(&ip->i_DASD, d0->d0_max >> shift);
			ip->i_DASD.thresh = d0->d0_thresh;
			ip->i_DASD.delta = d0->d0_delta;
			iplist[0] = ip;
			info0.dlags_rc = txCommit(tid, 1, iplist, 0);
			txEnd(tid);
		}
		else
			info0.dlags_rc = NERR_DASDMaxValidationFailed;
		d0->d0_use = DASDUSED(&ip->i_DASD) << shift;
		IWRITE_UNLOCK(ip);
		break;
	default:
		panic("func UFO in DASD_FSCTL\n");
	}
		
release_vnode:
	jfs_rele(pls.vp);
	if (rc == 0)
	{
        	*plenParmIO = sizeof(info0);
		rc = KernCopyOut(pParm, &info0, sizeof(info0));
	}
free_path:
	freepool(unipool, (caddr_t *)Path);
	DLIM_free(&pls.dlim);

	return rc;
}

/*
 * NAME:	DASD_FSCTL2()
 *
 * FUNCTION:	Handle JFS-specific DASD Limit FSCTL commands
 *
 * RETURN:	NO_ERROR - success
 */

APIRET APIENTRY DASD_FSCTL2 (
        union argdat    *pArgDat,
        unsigned long   iArgType,
        unsigned long   func,
        char            *pParm,
        unsigned long   lenParm,
        unsigned short  *plenParmIO,
        char            *pData,
        unsigned long   lenData,
        unsigned short  *plenDataIO)
{
	struct dasd_alert	*alert;
	int32		DLIM_parms_header[7];
	struct jfs_DLIM_parms	*DLIM_parms =
				(struct jfs_DLIM_parms *)&DLIM_parms_header;
	struct enum_instance	*enumInst;
	unsigned long	hEnum_save;
	inode_t		*ip;
	int32		length;
	UniChar		*pattern;
	UniChar		*Path;
	pathlook_t	pls;
	int32		rc;
	struct vfs	*vfsp;
	struct vnode	*vp;

jEVENT(0,("In DASD_FSCTL2, func = 0x%x\n", func));

/*
 * JFS_DLIM_INIT
 */
	if (func == JFS_DLIM_INIT)
	{
		if (server_running)
			return NERR_ServiceInstalled;

		if (*plenParmIO < sizeof(InitInfo))
			return ERROR_INVALID_PARAMETER;

		rc = KernCopyIn(&InitInfo, pParm, sizeof(InitInfo));
		if (rc)
			return rc;

		sessionFlags = ProcessToGlobal(InitInfo.pSessionFlagsPtr,
						InitInfo.ulSessionFlagsLen);
		if (sessionFlags == 0)
			return ERROR_INVALID_PARAMETER;

		rc = KernVMLock(VMDHL_LONG, sessionFlags,
				InitInfo.ulSessionFlagsLen,
				&session_lock_handle,
				(KernPageList_t *)-1, 0);
		if (rc)
		{
			KernVMFree(sessionFlags);
			sessionFlags = 0;
			return rc;
		}

		if (alert_pool == 0)
		{
			/*
			 * Haven't initialized data structures yet
			 */
			rc = initpool(4096, 146, sizeof(struct dasd_alert), 0,
				      "DL_ALERT", &alert_pool, 0);
			if (rc)
				return rc;

			KernAllocMutexLock(&dasd_alert_lock);
		}

		server_running = 1;
		return NO_ERROR;
	}
/*
 * JFS_DLIM_STOP
 */
	if (func == JFS_DLIM_STOP)
	{
		if (!server_running)
			return NERR_ServerNotStarted;

		server_running = 0;
		ALERT_LOCK();
		while (alert_anchor)
		{
			alert = alert_anchor;
			alert_anchor = alert->next;

			if (alert->ip)
				jfs_rele(IP2VP(alert->ip));

			freepool(alert_pool, (caddr_t *)alert);
		}
		ALERT_UNLOCK();

		KernWakeup((ULONG)&alert_event, 0, 0, 0);
		rc = KernVMUnlock(&session_lock_handle);
		KernVMFree(sessionFlags);
		sessionFlags = 0;
		return rc;
	}
/*
 * JFS_DLIM_ALERT
 */
	if (func == JFS_DLIM_ALERT)
	{
		struct JFS_alert_request	request;
		int32			alert_info_header[3];
		struct JFS_alert_info	*info = (struct JFS_alert_info *)
						alert_info_header;
		inode_t			*ip;
		UniChar			*path;
		UniChar			*path_buf;
		int32			shift;

		*plenDataIO = 0;

		if (*plenParmIO < sizeof(request))
			return ERROR_INVALID_PARAMETER;

		if (lenData < sizeof(struct JFS_alert_info))
			return ERROR_INVALID_PARAMETER;

		rc = KernCopyIn(&request, pParm, sizeof(request));
		if (rc)
			return rc;

		if (request.ulLevel != 0)
			return ERROR_INVALID_PARAMETER;

		if (! server_running)
			return NERR_ServerNotStarted;

try_again:
		ALERT_LOCK();
		while (server_running && (alert_next == 0))
		{
			/*
			 * Here's where we'll process alert queue
			 */
			KernBlock((ULONG)&alert_event, -1, T_MUTEXLOCK,
				  &dasd_alert_lock, 0);
		}
		if (! server_running)
		{
			ALERT_UNLOCK();
			return NERR_ServerNotStarted;
		}

		/*
		 * Take next alert from list
		 */
		alert = alert_next;
		alert_next = alert->next;
		ip = alert->ip;
		alert->ip = 0;		/* So nobody else releases inode */
		shift = ip->i_ipmnt->i_l2bfactor;
		if (alert->freeblocks)
		{
			info->usType = JFS_THRESH_ALERT;
			info->ulFreespace = alert->freeblocks << shift;
		}
		else
		{
			info->usType = JFS_DIRFULL_ALERT;
			info->ulFreespace = 0;
		}
		ASSERT(alert->session);
		info->usSession = alert->session;
		ALERT_UNLOCK();

		path_buf = (UniChar *)allocpool(unipool, 0);
		if (path_buf == 0)
		{
			jfs_rele(IP2VP(ip));
			return ENOMEM;
		}

		path = build_path(ip, path_buf);
		jfs_rele(IP2VP(ip));

		if (path == 0)
		{
			/*
			 * Stale directory?
			 */
			freepool(unipool, (caddr_t *)path_buf);
			goto try_again;
		}
		info->ulLevel = 0;
		rc = KernCopyOut(pData, info, sizeof(alert_info_header));
		if (rc)
			return rc;

		if (jfs_strfromUCS(((struct JFS_alert_info *)pData)->szDirname,
				   path, MAXPATHLEN, UniStrlen(path)) == -1)
			rc = ERROR_BUFFER_OVERFLOW;
		else
        		*plenDataIO = sizeof(struct JFS_alert_info);

jEVENT(NOISY,("Alert thread returning rc = %d, pData = 0x%x\n", rc, pData));
		return rc;
	}
/*
 * COMMON:
 *	JFS_DLIM_FF
 *	JFS_DLIM_FN
 *	JFS_DLIM_FC
 */
	if (iArgType != FSCTL_FSDNAME)
		return ERROR_INVALID_PARAMETER;

	if (lenParm < sizeof(DLIM_parms_header))
		return ERROR_INVALID_PARAMETER;

	*plenParmIO = sizeof(DLIM_parms_header);
	rc = KernCopyIn(DLIM_parms, pParm, sizeof(DLIM_parms_header));
	if (rc)
		return rc;

	if (DLIM_parms->ulLevel != 0)
		return ERROR_INVALID_LEVEL;

	switch (func)
	{
/*
 * JFS_DLIM_FF
 */
	case JFS_DLIM_FF:
		vfsp = PATHtoVFS(DLIM_parms->szPath);
		if (vfsp == 0)
		{
			rc = ERROR_INVALID_PARAMETER;
			break;
		}
		if ((vfsp->vfs_flag & VFS_DASDLIM) == 0)
		{
			rc = (vfsp->vfs_flag & VFS_DASDPRIME) ?
				NERR_DASDNotRunning :
				NERR_DASDNotInstalled;
			break;
		}
		DLIM_init(&pls.dlim, DLIM_READONLY);

		Path = (UniChar *)allocpool(unipool, 0);
		if (Path == 0)
		{
			rc = ENOMEM;
			break;
		}
		if (jfs_strtoUCS(Path, ((struct jfs_DLIM_parms *)pParm)->szPath,
			MAXPATHLEN)
		    == -1)
		{
			rc = ERROR_BUFFER_OVERFLOW;
			goto ff_exit_1;
		}
		pls.path = Path;

		rc = pathlookup(vfsp, &pls, pcred);
		if (rc)
			goto ff_exit_1;

		if ((pls.statlast == PL_EXIST) || (pls.statlast == PL_NOENT))
		{
			jfs_rele(pls.dvp);
			if (pls.statlast == PL_EXIST)
				jfs_rele(pls.vp);
			rc = ENOTDIR;
			goto ff_exit_1;
		}

		enumInst = alloc_enum();
		if (enumInst == 0)
		{
			if (pls.statlast == PL_ISDIR)
				jfs_rele(pls.vp);
			jfs_rele(pls.dvp);
			rc = ENOMEM;
			goto ff_exit_1;
		}

		if (pls.statlast == PL_ISDIR)
		{
			/*
			 * Default pattern set in alloc_enum is "*"
			 */
			vp = pls.vp;
			jfs_rele(pls.dvp);
		}
		else	/* pls.statlast == PL_WCARD */
		{
			vp = pls.dvp;
			/*
			 * We need to allocate a pattern buffer
			 */
			if (((enumInst->enum_pattern.namlen =
			     UniStrlen(pls.pathlast)) > 1) ||
			    (pls.pathlast[0] != '*'))
			{
				/*
				 * pattern will be freed by free_enum
				 */
				pattern = (UniChar *)allocpool(unipool, 0);
				if (pattern == NULL)
				{
					jfs_rele(vp);
					free_enum(enumInst);
					rc = ENOMEM;
					goto ff_exit_1;
				}
				UniStrcpy(pattern, pls.pathlast);
				UniStrupr(pattern);
				enumInst->enum_pattern.name = pattern;
			}
		}
		enumInst->enum_vfsp = vp->v_vfsp;
		enumInst->enum_inum = VP2IP(vp)->i_number;
		enumInst->enum_pid = pLIS->LIS_CurProcID;

		hEnum_save = DLIM_parms->hEnum;
		DLIM_parms->hEnum = (unsigned int)enumInst;
		length = lenData;

		rc = jfs_enumDLIM(vp, DLIM_parms, pData, &length);

		jfs_rele(vp);

		if (rc == 0)
			*plenDataIO = length;
		else
		{
			*plenDataIO = 0;
			DLIM_parms->hEnum = hEnum_save;
			free_enum(enumInst);
		}
ff_exit_1:
		freepool(unipool, (caddr_t *)Path);

		break;
/*
 * JFS_DLIM_FN
 */
	case JFS_DLIM_FN:
		/*
		 * Verify valid Enum handle
		 */
		enumInst = (struct enum_instance *)DLIM_parms->hEnum;
		rc = verify_enum(enumInst);
		if (rc)
			break;

		ICACHE_LOCK();
		rc = iget(enumInst->enum_vfsp, enumInst->enum_inum, &ip,
			  0);
		ICACHE_UNLOCK();
		if (rc)
		{
			rc = ERROR_INVALID_PARAMETER;
			break;
		}

		length = lenData;
		rc = jfs_enumDLIM(IP2VP(ip), DLIM_parms, pData,
				  &length);

		jfs_rele(IP2VP(ip));

		if (rc == 0)
			*plenDataIO = length;
		else
			*plenDataIO = 0;

		break;
/*
 * JFS_DLIM_FC
 */
	case JFS_DLIM_FC:

		enumInst = (struct enum_instance *)DLIM_parms->hEnum;
		rc = free_enum(enumInst);
		DLIM_parms->hEnum = JFS_ACCESS_HANDLE_UNALLOCATED;

		break;
	}
	DLIM_parms->ulReturnCode = rc;
	rc = KernCopyOut(pParm, DLIM_parms, sizeof(DLIM_parms_header));
jEVENT(0,("Exiting DASD_FSCTL2 rc = %d, ulReturnCode = %d\n", rc, DLIM_parms->ulReturnCode));
	return rc;
}

/*
 * NAME:	dasd_prime()
 *
 * FUNCTION:	Calculate current usage in all directories
 *
 * RETURN:	NO_ERROR - success
 *
 * SERIALIZATION:	Called before FS32_MOUNT returns.
 */
int32	dasd_prime(
	struct vfs	*vfsp)
{
	struct search_stack
	{
		inode_t		*ip;
		struct dirent	*dbuf;
		struct dirent	*dbufp;
		int32		tbytes;
		UniChar		*lastmatch;
		uint32		offset;
		uint64		usage;
	};
	struct dirent		*dbuf;
	struct dirent		*dbufp;
	inode_t			*dip;
	int32			Done = 0;
	int32			inode_changed;			// D233382
	inode_t			*ip;
	inode_t			*iplist[1];
	UniChar			*lastmatch;
	uint32			nmatches;
	int32			offset;
	component_t		pattern = { 1, L"*" };
	int32			rc;
	struct search_stack	*stack;
	int32			stack_size = 0;
	int32			tbytes;
	int32			tid;
	uint64			usage;

	stack = (struct search_stack *)xmalloc(sizeof(struct search_stack)*MAX_LIMITS, 0, kernel_heap);
jEVENT(0,("In dasd_prime: stack = 0x%x\n", stack));
	if (stack == 0)
		return ENOMEM;

	dip = VP2IP(vfsp->vfs_mntd);	/* Root directory */

new_search:
	usage = dip->i_nblocks;

	dbufp = dbuf = (struct dirent *)allocpool(dirent_pool, 0);
	if (dbufp == 0)
	{
		rc = ENOMEM;
		goto unwind;
	}
	offset = 0;	/* Begin search right after . & .. */
	while (TRUE)
	{
		nmatches = 256;
		tbytes = 0;
		dbufp = dbuf;
		/*
		 * We don't need to get READLOCK on inode since DLLOCK keeps
		 * any other thread from modifying the subtree
		 */
		if (offset != -1)
{
jEVENT(0,("Calling dtFind: dip = 0x%x\n", dip));
			rc = dtFind(dip, &pattern, lastmatch, &offset,
				    &nmatches, PSIZE, &tbytes, dbufp);
jEVENT(0,("dtFind returned %d, nmatches = %d\n", rc, nmatches));
}
		else
			rc = ENFILE;
		if (rc || (nmatches == 0))
		{
			inode_changed = FALSE;			// D233382

			/* If simply refreshing, don't zero limit */
			if (((vfsp->vfs_flag & VFS_DASDLIM) == 0) && // D233382
				DASDLIMIT(&dip->i_DASD))	// D233382
			{
				setDASDLIMIT(&dip->i_DASD, 0);
				inode_changed = TRUE;		// D233382
			}
// BEGIN D233382
			if (DASDUSED(&dip->i_DASD) != usage)
			{
#ifdef _JFS_DEBUG
				if (vfsp->vfs_flag & VFS_DASDLIM)
					jEVENT(1,("ip %d: Correcting DASD used from 0x%x to 0x%x\n",
					       dip->i_number,
					       (uint32)DASDUSED(&dip->i_DASD),
					       (uint32)usage));
#endif /* _JFS_DEBUG */
				setDASDUSED(&dip->i_DASD, usage);
				inode_changed = TRUE;
				 
			}
			if (inode_changed)
			{
				txBegin(dip->i_ipmnt, &tid, 0);
				lock_header(tid, dip);
				iplist[0] = dip;
				rc = txCommit(tid, 1, iplist, 0);
				txEnd(tid);
				ASSERT(rc == 0);
			}
// END D233382
			freepool(dirent_pool, (caddr_t *)dbuf);

			if (stack_size)
			{
				jfs_rele(IP2VP(dip));
				/* Pop parent search state from stack */
				stack_size--;
				dip = stack[stack_size].ip;
				dbuf = stack[stack_size].dbuf;
				dbufp = stack[stack_size].dbufp;
				tbytes = stack[stack_size].tbytes;
				lastmatch = stack[stack_size].lastmatch;
				offset = stack[stack_size].offset;
				usage += stack[stack_size].usage;
jEVENT(0,("POP: dip = 0x%x\n", dip));
			}
			else
				break;
		}
jEVENT(0,("offset = 0x%x\n", offset));
		while (tbytes)
		{
			ICACHE_LOCK();
			rc = iget((IP2VP(dip))->v_vfsp, dbufp->d_ino, &ip, 0);
			ICACHE_UNLOCK();
			if (rc)
{
jEVENT(2,("Error from iget(0x%x) = %d\n", dbufp->d_ino, rc));
				goto unwind;
}
			lastmatch = dbufp->d_name;
			tbytes -= dbufp->d_reclen;
			ASSERT(tbytes >= 0);
			dbufp = (struct dirent *)((caddr_t)dbufp +
						  dbufp->d_reclen);
			if ((ip->i_mode & IFMT) == IFDIR)
			{
jEVENT(0,("PUSH: ip = 0x%x\n", ip));
				/* Push current search state */
				assert(stack_size < MAX_LIMITS);
				stack[stack_size].ip = dip;
				stack[stack_size].dbuf = dbuf;
				stack[stack_size].dbufp = dbufp;
				stack[stack_size].tbytes = tbytes;
				stack[stack_size].lastmatch = lastmatch;
				stack[stack_size].offset = offset;
				stack[stack_size].usage = usage;
				stack_size++;
				/* Initialize search on this inode */
				dip = ip;
				goto new_search; /* Darned goto's :^) */
			}
			else	/* Regular file */
{
jEVENT(0,("Regular File: ip = 0x%x\n", ip));
				usage += ip->i_nblocks;
}
			jfs_rele(IP2VP(ip));
		}
	}
	xmfree((void *)stack, kernel_heap);

	
	modify_superblock(dip->i_ipmnt, JFS_DASD_ENABLED, JFS_DASD_PRIME);
	vfsp->vfs_flag |= VFS_DASDLIM;
	vfsp->vfs_flag &= ~VFS_DASDPRIME;

jEVENT(0,("Returning  from dasd_prime\n"));
	return 0;
unwind:
jEVENT(1,("Unwind! rc = %d\n", rc));
	if (dbuf)
		freepool(dirent_pool, (caddr_t *)dbuf);
	while(stack_size)
	{
		jfs_rele(IP2VP(dip));
		stack_size--;
		freepool(dirent_pool, (caddr_t *)stack[stack_size].dbuf);
		dip = stack[stack_size].ip;
	}
	xmfree((void *)stack, kernel_heap);
	/*
	 * Do we need to somehow notify the Ring 3 Server of the failure?
	 */
	ASSERT(rc);
	return rc;
}

/*
 * NAME:	lock_header()
 *
 * FUNCTION:	linelock the header of the DTREE, which contains the DASDlimit
 *
 * RETURN:	NONE
 */
void lock_header(
	int32	tid,
	inode_t	*ip)
{
	dtlock_t	*dtlck;
	lv_t		*lv;
	tlock_t		*tlck;

	tlck = txLock(tid, ip, (jbuf_t *)&ip->i_bxflag, tlckDTREE|tlckBTROOT);
	dtlck = (dtlock_t *)&tlck->lock;
	if (dtlck->index >= dtlck->maxcnt)
		dtlck = (dtlock_t *)txLinelock(dtlck);
	lv = (lv_t *)&dtlck->lv[dtlck->index];
	lv->offset = 0;
	lv->length = 1;
	dtlck->index++;
}

void	modify_superblock(
	inode_t	*ipmnt,
	int32	bits_on,
	int32	bits_off)
{
	cbuf_t			*bp;
	int32			rc;
	struct superblock	*sb;

	rc = readSuper(ipmnt, &bp);
	assert(rc == 0);
	sb = (struct superblock *)(bp->b_bdata);
	sb->s_flag |= bits_on;
	sb->s_flag &= ~bits_off;
	ipmnt->i_mntflag |= bits_on;
	ipmnt->i_mntflag &= ~bits_off;
#ifdef _JFS_FASTDASD
	/*
	 * Whenever DASD limits are enabled and the volume is mounted, we
	 * need the superblock to indicate that priming is requested.
	 * If we umount cleanly, the superblock prime flag will be synchronized
	 * with the proper value stored in the mount inode.
	 */
	if (bits_on & JFS_DASD_ENABLED)				// D233382
		sb->s_flag |= JFS_DASD_PRIME;			// D233382
#endif /* _JFS_FASTDASD */
	rawWrite(ipmnt, bp, 1);
}

/*
 * NAME:	dasd_write_lock()
 *
 * FUNCTION:	Lock inode & all its ancestors
 *
 * RETURN:	NONE
 */
void	dasd_write_lock(
	inode_t			*ip,
	struct dasd_usage	*du)
{
jEVENT(NOISY,("dasd_write_lock ip = 0x%x, num_limits = %d\n", ip, du->num_limits));
	ASSERT(du->num_limits);
	ASSERT(du->pLimits[0]->i_number == ROOT_I);
	if ((du->flag & DLIM_IP_IN_LIST) || (ip && (ip->i_number == ROOT_I)))
		ip = 0;
	else if (ip && (ip->i_mode & IFMT != IFDIR))
	{
jEVENT(NOISY,("locking ip = 0x%x\n", ip));
		/* Lock regular files before directories */
		IWRITE_LOCK(ip);
		ip = 0;
	}
	sort_and_lock(ip, du->num_limits, du->pLimits);
	du->flag |= DLIM_DIRS_LOCKED;
	du->first_locked = 0;					// D230860

	/*
	 * This indicates that we don't yet know how much is available.
	 */
	du->flag &= ~DLIM_ADMIN;
	du->avail = -1;
}

/*
 * NAME:	dasd_write_unlock()
 *
 * FUNCTION:	Unlock inode & all its ancestors
 *
 * RETURN:	NONE
 */
void	dasd_write_unlock(
	inode_t			*ip,
	struct dasd_usage	*du)
{
	int32	i;

jEVENT(NOISY,("dasd_write_unlock ip = 0x%x, num_limits = %d\n", ip, du->num_limits));

	ASSERT(du->num_limits);
	ASSERT(du->pLimits[0]->i_number == ROOT_I);
	if (ip &&
	    ((du->flag & DLIM_IP_IN_LIST) == 0) && (ip->i_number != ROOT_I))
	{
jEVENT(NOISY,("unlocking ip = 0x%x\n", ip));
		IWRITE_UNLOCK(ip);
	}

	if (du->flag & DLIM_DIRS_LOCKED)
	{
		for (i = du->first_locked; i < du->num_limits; i++) // D230860
		{
jEVENT(NOISY,("unlocking 0x%x\n", du->pLimits[i]));
			IWRITE_UNLOCK(du->pLimits[i]);
		}
		du->flag &= ~DLIM_DIRS_LOCKED;
	}
	else
	{
jEVENT(NOISY,("DIRS NOT LOCKED\n"));
	}
}

/*
 * NAME:	dlim_copy()
 *
 * FUNCTION:	Copy dasd usage structure from stack
 *
 *		Open files must maintain a the dasd usage structure between
 *		file system calls.  While the list is normally allocated on
 *		the stack, a copy must be allocated off the heap in order
 *		to preserve it until the file is closed.
 *
 * RETURN:	Pointer to dasd_usage structure; zero if alloc failed
 */
struct dasd_usage *dlim_copy(
	struct dasd_usage *orig)
{
	struct dasd_usage	*new;

jEVENT(NOISY,("dlim_copy - orig = 0x%x\n", orig));

	ASSERT(orig->num_limits);
	ASSERT(orig->pLimits[0]->i_number == ROOT_I);

	if (orig->flag & DLIM_LARGE)
	{
		/*
		 * Heap storage has already been allocated to facilitate large
		 * list of inodes.  Copy header information to large struct.
		 */
		new = (struct dasd_usage *)(((char *)orig->pLimits) -
					    LIMITS_OFFSET);
		memcpy(new, orig, LIMITS_OFFSET);
		/* We don't want DLIM_free() to release space or inodes */
		orig->flag &= ~(DLIM_LARGE|DLIM_RELEASE);
	}
	else
	{
		new = (struct dasd_usage *)allocpool(dlim_pool, 0);
		if (new)
		{
			memcpy(new, orig, DLIM_SIZE(orig));
			new->pLimits = new->Limits;
			/* We don't want DLIM_free() to release inodes */
			orig->flag &= ~DLIM_RELEASE;
		}
	}
	ASSERT(new->pLimits[0]->i_number == ROOT_I);
jEVENT(NOISY,("dlim_copy - returning = 0x%x\n", new));
	return new;
}

/*
 * NAME:	dlim_release()
 *
 * FUNCTION:	Releases dasd usage structure from inode when no longer needed
 *
 * RETURN:	NONE
 */
void	dlim_release(
	inode_t	*ip)
{
	int32	i;

jEVENT(NOISY,("dlim_release - ip = 0x%x\n", ip));
	if (ip->i_dasdlim && (IP2GP(ip)->gn_wrcnt == 0))
	{
		ASSERT((ip->i_dasdlim->flag & DLIM_DIRS_LOCKED) == 0)
		/* Release ancestor inodes & free dasd usage structure */
		for (i = 0; i < ip->i_dasdlim->num_limits; i++)
			jfs_rele(IP2VP(ip->i_dasdlim->pLimits[i]));
		if (ip->i_dasdlim->flag & DLIM_LARGE)
			freepool(big_dlim_pool, (caddr_t *)ip->i_dasdlim);
		else
			freepool(dlim_pool, (caddr_t *)ip->i_dasdlim);
		ip->i_dasdlim = 0;
	}
	else
	{
jEVENT(NOISY,("dlim_release - Nothing to release\n"));
	}
}

#ifndef _JFS_FASTDASD						// D233382
/*
 * NAME:	dasd_commit()
 *
 * FUNCTION:	Calls txCommit with an appropriate list of inodes to commit
 *
 * RETURN:	rc from txCommit()
 */
int32	dasd_commit(
	int32	tid,
	inode_t	*ip,
	uint32	flag)
{
	inode_t	*iparray[64];
	inode_t	**iplist;
	int32	nip;
	int32	rc;

jEVENT(NOISY,("dasd_commit - tid = %d ip = 0x%x\n", tid, ip));
	ASSERT(ip->i_dasdlim);

	ASSERT(ip->i_dasdlim->pLimits[0]->i_number == ROOT_I);
	/*
	 * If no changes were made to parent directories, only commit this
	 * inode
	 */
	if ((ip->i_dasdlim->flag & DLIM_LOGGED) == 0)
		return txCommit(tid, 1, &ip, flag);

	assert(ip->i_dasdlim->flag & DLIM_DIRS_LOCKED);

	/*
	 * Copy inodes to list which will be sorted by txCommit()
	 */
	nip = ip->i_dasdlim->num_limits;
	if (nip < 64)
		iplist = iparray;
	else
		iplist = (inode_t **)xmalloc((nip+1)*sizeof(inode_t *), 0,
					     kernel_heap);
	memcpy(iplist, ip->i_dasdlim->pLimits, nip*sizeof(inode_t *));

	/*
	 * Unless it's already there, we need to add this inode to the
	 * dasdlim list
	 */
	if ((ip->i_dasdlim->flag & DLIM_IP_IN_LIST) == 0)
		iplist[nip++] = ip;

	rc = txCommit(tid, nip, iplist, flag);

	if (iplist != iparray)
		xmfree((void *)iplist, kernel_heap);

	ip->i_dasdlim->flag &= ~DLIM_LOGGED;

	return rc;
}
#endif /* _JFS_FASTDASD */					// D233382

/*
 * NAME:	dasd_update()
 *
 * FUNCTION:	Updates directory DASD usage
 *
 * RETURN:	none
 */
void	dasd_update(
	int32	tid,
	inode_t	*ip,
	int64	delta)
{
	struct dasd_usage *du = ip->i_dasdlim;
	int32	i;
	inode_t	*inodep;
	int64	usage;
	struct vfs	*vfsp;

jEVENT(NOISY,("dasd_update - tid = %d ip = 0x%x, delta = 0x%x\n", tid, ip, (uint32)delta));

	ASSERT(du);
	ASSERT(du->pLimits[0]->i_number == ROOT_I);

	/*
	 * Inodes must already be locked when the transaction is begun
	 */
	ASSERT(du->flag & DLIM_DIRS_LOCKED);			// D230860

	for (i = 0; i < du->num_limits; i++)
	{
		inodep = du->pLimits[i];
		usage = DASDUSED(&inodep->i_DASD) + delta;
		if (usage < 0)
		{
			jEVENT(1, ("DASD usage < 0, inode %d\n",
				   inodep->i_number));
			/*
			 * We'll have to fix this on the next boot
			 */
			vfsp = IP2VP(inodep)->v_vfsp;
			if ((vfsp->vfs_flag & VFS_DASDPRIME) == 0)
			{
				modify_superblock(inodep->i_ipmnt,
						  JFS_DASD_PRIME, 0);
				vfsp->vfs_flag |= VFS_DASDPRIME;
			}
			/*
			 * Best guess is current size of directory
			 */
			setDASDUSED(&inodep->i_DASD, inodep->i_nblocks);
		}
		else
		{
			/*
			 * If the LAN server is running, directory usage has
			 * increased, & the directory has a limit, issue
			 * an alert if appropriate.
			 */
			if (server_running && (delta > 0) &&
			    DASDLIMIT(&inodep->i_DASD))
				check_alert(inodep, (uint32)usage);

			setDASDUSED(&inodep->i_DASD, usage);
		}
#ifdef _JFS_FASTDASD
		/*
		 * Mark inodes as dirty so they will be written eventually.
		 */
		inodep->i_flag |= ICHG;				// D233382
#else /* ! _JFS_FASTDASD */
		/*
		 * If first change to directory tree for this transaction,
		 * acquire tlock on the d-tree header
		 */
		if ((du->flag & DLIM_LOGGED) == 0)
			lock_header(tid, inodep);
#endif /* _JFS_FASTDASD */
	}
#ifndef _JFS_FASTDASD						// D233382
	du->flag |= DLIM_LOGGED;
#endif /* _JFS_FASTDASD */					// D233382
}

/*
 * NAME:	check_alert()
 *
 * FUNCTION:	checks if usage triggers alert and passes alert to captive
 *		alert thread
 *
 * NOTE:	Although elsewhere DASD limits & usage are coded to exceed
 *		32 bits, we know they will never exceed 32 bits.  For
 *		simplicity, this routine will work with only 32 bits.
 *
 * RETURN:	none
 */
void	check_alert(
	inode_t	*ip,
	uint32	new_usage)
{
	uint32	delta;
	uint32	limit;
	uint32	new_interval;
	uint32	new_percent;
	uint32	old_interval;
	uint32	old_percent;
	uint32	old_usage;
	uint32	thresh;
	int32	userid;

	limit = DASDLIMIT(&ip->i_DASD);
	ASSERT(limit);
	old_usage = DASDUSED(&ip->i_DASD);

	/*
	 * Check to see if user is subject to limits
	 */
	if ((sessionFlags == 0) ||
	    ((userid = User_ID()) == 0) ||
	    ((sessionFlags[userid] & R3SRV_SESFLG_DLIM) == 0))
		return;
	/*
	 * If we have reached the limit, issue an alert.
	 */
	if (new_usage >= limit)
	{
		send_alert(ip, 0, 0);
		return;
	}

	/*
	 * Return if we are below the threshold
	 */
	if ((thresh = ip->i_DASD.thresh) == 0)
		return;

	new_percent = percent(new_usage, limit);

	if (new_percent < thresh)
		return;

	/*
	 * Determine which threshold interval we have reached
	 */
	old_percent = percent(old_usage, limit);

	if ((delta = ip->i_DASD.delta) != 0)
		new_interval = (new_percent - thresh) / delta;
	else
		new_interval = 0;

	/*
	 * Send an alert if we were previously below the threshold
	 */
	if (old_percent < thresh)
	{
		send_alert(ip, limit - new_usage, new_interval);
		return;
	}

	/*
	 * No delta means we can't cross into next interval
	 */
	if (delta == 0)
		return;

	/*
	 * Determine if we reached a new threshold interval
	 */
	old_interval = (old_percent - thresh) / delta;

	if (new_interval > old_interval)
		send_alert(ip, limit - new_usage, new_interval);

	return;
}

/*
 * NAME:	send_alert()
 *
 * FUNCTION:	Pass alert information to captive alert thread
 *
 * RETURN:	none
 */
void	send_alert(
	inode_t	*ip,
	uint32	freeblocks,
	uint32	interval)
{
	struct dasd_alert	*alert;
	uint32			delay;
	struct dasd_alert	*prev;
	struct dasd_alert	*next;
	inode_t			*inodep;
	ino_t			inum;
	uint32			now;
	struct vfs		*vfsp;

	/*
	 * First, purge expired alerts from list
	 */

	prev = 0;
	now = GIS->SIS_BigTime;
	ALERT_LOCK();
	for (alert = alert_anchor; alert; alert = next)
	{
		next = alert->next;
		if (alert->expires < now)
		{
			if (prev)
				prev->next = next;
			else
				alert_anchor = next;

			inodep = alert->ip;
			freepool(alert_pool, (caddr_t *)alert);

			if (inodep)
			{
				/*
				 * Alert hasn't been sent.  We aren't going
				 * to hold onto it forever, so let it go.
				 */
				assert(alert_next == alert);
				alert_next = next;

				jfs_rele(IP2VP(inodep));
			}
			else
			{
				assert(alert_next != alert);
			}
		}
		else
			prev = alert;
	}

	/*
	 * This time we're looking for an alert on the same inode
	 */

	vfsp = IP2VP(ip)->v_vfsp;
	inum = ip->i_number;
	prev = 0;

	for (alert = alert_anchor; alert; alert = alert->next)
	{
		prev = alert;
		if ((alert->vfsp == vfsp) && (alert->inum == inum))
		{
			if (alert->freeblocks == 0)
			{
				/* Full alert delay in effect */
				ALERT_UNLOCK();
				return;
			}

			if (freeblocks == 0)
				/* This full alert overrides previous thresh */
				continue;

			if (alert->interval >= interval)
			{
				/* delay for equal or higher thresh in effect */
				ALERT_UNLOCK();
				return;
			}
		}
	}

	alert = (struct dasd_alert *)allocpool(alert_pool, 0);
	if (alert == 0)
	{
		jEVENT(1,("allocpool failure in send_alert!\n"));
		ALERT_UNLOCK();
		return;
	}
	/*
	 * Take another reference on the inode to keep it in cache.  It will
	 * be released when alert is issued or purged from list
	 */
	jfs_hold(IP2VP(ip));

	alert->vfsp = vfsp;
	alert->inum = inum;
	alert->ip = ip;
	alert->freeblocks = freeblocks;
	alert->interval = interval;
	alert->session = User_ID();

	if (freeblocks)
	  delay = InitInfo.aDelayInfo[vfsp->vfs_vpfsi->vpi_drive].ulThreshDelay;
	else
	  delay = InitInfo.aDelayInfo[vfsp->vfs_vpfsi->vpi_drive].ulDiskFullDelay;
	/*
	 * Enforce some minimum delay to prevent us from purging this from the
	 * queue before the captive alert thread can process it.
	 */
	if (delay <= 0)
		delay = 5;
	alert->expires = now + delay;

	/*
	 * Add to list
	 */
	alert->next = 0;
	if (alert_anchor)
		prev->next = alert;
	else
		alert_anchor = alert;

	if (alert_next == 0)
		alert_next = alert;

	ALERT_UNLOCK();

	KernWakeup((ULONG)&alert_event, 0, 0, 0);
	return;
}

/*
 * NAME:	dasd_umount()
 *
 * FUNCTION:	Clear sent alerts associated with a volume being unmounted
 *
 * RETURN:	none
 */
void	dasd_umount(
	struct vfs	*vfsp)
{
	struct dasd_alert	*alert;
	struct dasd_alert	*prev;
	struct dasd_alert	*next;
	inode_t			*ip;

	if (!server_running)
		return;

	prev = 0;
	ALERT_LOCK();
	for (alert = alert_anchor; alert; alert = next)
	{
		next = alert->next;
		if (alert->vfsp == vfsp)
		{
			if (prev)
				prev->next = next;
			else
				alert_anchor = next;

			ip = alert->ip;
			freepool(alert_pool, (caddr_t *)alert);

			if (ip)
			{
				/*
				 * Alert hasn't been sent.
				 */
				if (alert_next == alert)
					alert_next = next;

				jfs_rele(IP2VP(ip));
			}
			else
			{
				assert(alert_next != alert);
			}
		}
		else
			prev = alert;
	}

	ALERT_UNLOCK();

	return;
}

/*
 * NAME:	over_limit()
 *
 * FUNCTION:	checks if user is subject to DASD limits, and is exceeding a
 *		limit 
 *
 * RETURN:	TRUE if operation is not allowed, FALSE otherwise
 */
int32	over_limit(
	struct dasd_usage	*du,
	int64			nblocks,
	uint32			start_here)
{
	int32	i;
	int32	inode2;
	inode_t	*ip;
	uint64	min_avail;
	uint64	min_avail2;
	int64	space_left;
	int32	userid;

	if ((du->flag & DLIM_DIRS_LOCKED) == 0)
		dasd_write_lock(0, du);

	if (du->flag & DLIM_ADMIN)
		return FALSE;

	/*
	 * start_here can only be set on first call (in jfs_rename.c)
	 */
	ASSERT((start_here == 0) || (du->avail == -1 ))

	if (du->avail == -1)
	{
		/*
		 * First time we've been called since inodes locked.
		 */
		if ((sessionFlags == 0) ||
		    ((userid = User_ID()) == 0) ||
		    ((sessionFlags[userid] & R3SRV_SESFLG_DLIM) == 0))
		{
			/* We don't need to look at the limits, we're exempt */
			du->flag |= DLIM_ADMIN;
			return FALSE;
		}

		min_avail = min_avail2 = -2;
		for (i = 0; i < du->num_limits; i++)
		{
			ip = du->pLimits[i];
			if (DASDLIMIT(&ip->i_DASD))
			{
				space_left = DASDLIMIT(&ip->i_DASD) -
					     DASDUSED(&ip->i_DASD);
				if (space_left <= 0)
					space_left = 0;

				if (i < start_here)
				{
					if ((uint64)space_left < min_avail2)
					{
						inode2 = i;
						min_avail2 = (uint64)space_left;
					}
				}
				else
				{
					if ((uint64)space_left < min_avail)
					{
						du->limiting_inode = i;
						min_avail = (uint64)space_left;
					}
				}
			}
		}
		if (start_here)
		{
			if ((min_avail == 0) || (nblocks > min_avail))
			{
				if (server_running)
				{
					ip = du->pLimits[du->limiting_inode];
					send_alert(ip, 0, 0);
				}
				return TRUE;
			}
			min_avail -= nblocks;
			if (min_avail < min_avail2)
				du->avail = min_avail;
			else
			{
				du->avail = min_avail2;
				du->limiting_inode = inode2;
			}
			return FALSE;
		}
		du->avail = min_avail;
	}

	if ((du->avail == 0) || (nblocks > du->avail))
	{
		if (server_running)
		{
			ip = du->pLimits[du->limiting_inode];
			send_alert(ip, 0, 0);
		}
		return TRUE;
	}

	du->avail -= nblocks;
	return FALSE;
}

/*
 * NAME:	jfs_enumDLIM()
 *
 * FUNCTION:	DASD limits enumeration worker
 *
 * RETURN:	0 if successful
 *		ERROR_NO_MORE_FILES - no more directories
 *		ERROR_INSUFFICIENT BUFFER - buffer too small
 */
int32	jfs_enumDLIM(
struct vnode		*vp,
struct jfs_DLIM_parms	*dlim_parms,
char			*pData,
int32			*length)
{
	struct dasd_info_0		d0;
	struct dirent			*dbufp;
	inode_t				*dip;
	struct jfs_DLIM_enum_header	*enum_header;
	enum_instance_t			*enump;
	inode_t				*ip;
	unsigned char			namelen;
	int32				offset;
	uint32				matches;
	int32				rc;
	int32				shift;
	int32				tbytes;

jEVENT(0,("In jfs_enumDLIM\n"));
	enump = (enum_instance_t *)dlim_parms->hEnum;
	dip = VP2IP(vp);

	if (enump->enum_lastmatch[0] == 0)
		offset = 0;	/* first search */
	else
		offset = 1;	/* subsequent search */

	dbufp = (struct dirent *)allocpool(dirent_pool, 0);
	if (dbufp == 0)
		return ENOMEM;

	IREAD_LOCK(dip);

	if ((dip->i_nlink == 0) || ((dip->i_mode & IFMT) != IFDIR))
	{
		/* Stale search handle on no-longer-existing directory */
		IREAD_UNLOCK(dip);
		freepool(dirent_pool, (caddr_t *)dbufp);
		return ENOTDIR;
	}

	matches = 1;

	while (TRUE)
	{
		tbytes = 0;
		rc = dtFind(dip, &enump->enum_pattern, enump->enum_lastmatch,
			    &offset, &matches, PSIZE, &tbytes, dbufp);
		IREAD_UNLOCK(dip);

		if (rc)
			break;
		if (matches == 0)
		{
			rc = ERROR_NO_MORE_FILES;
			break;
		}

		UniStrncpy(enump->enum_lastmatch, dbufp->d_name,
			   dbufp->d_namlen);
		enump->enum_lastmatch[dbufp->d_namlen] = 0;

		ICACHE_LOCK();
		rc = iget(vp->v_vfsp, dbufp->d_ino, &ip, 0);
		ICACHE_UNLOCK();

		if (rc)
			break;

		IREAD_LOCK(ip);
		if ((ip->i_mode & IFMT) == IFDIR)
		{
			dlim_parms->cchData = sizeof(struct jfs_DLIM_enum_header) +
					      sizeof(struct dasd_info_0);
			if (dlim_parms->cchData > *length)
			{
				IREAD_UNLOCK(ip);
				jfs_rele(IP2VP(ip));
				*length = 0;
				rc = ERROR_INSUFFICIENT_BUFFER;
				break;
			}
			*length = dlim_parms->cchData;
			enum_header = (struct jfs_DLIM_enum_header *)pData;
			namelen = jfs_strfromUCS(enum_header->achName,
						 dbufp->d_name, CCHMAXPATHCOMP,
						 dbufp->d_namlen);
			if (enum_header->cchName == -1)
			{
				IREAD_UNLOCK(ip);
				jfs_rele(IP2VP(ip));
				*length = 0;
				rc = ERROR_BUFFER_OVERFLOW;
				break;
			}
			/*
			 * This is overkill, but we are dealing with a
			 * user buffer, so we don't want to page fault.
			 */
			rc = KernCopyOut(&enum_header->cchName, &namelen,
				    sizeof(namelen));
			if (rc)
			{
				jEVENT(1,("jfs_enumDLIM: copy of 1 byte failed!\n"));
				IREAD_UNLOCK(ip);
				jfs_rele(IP2VP(ip));
				*length = 0;
				break;
			}

			shift = ip->i_ipmnt->i_l2bfactor;
			d0.d0_max = DASDLIMIT(&ip->i_DASD) << shift;
			d0.d0_use = DASDUSED(&ip->i_DASD) << shift;
			d0.d0_flag = 0;
			d0.d0_thresh = ip->i_DASD.thresh;
			d0.d0_delta = ip->i_DASD.delta;

			rc = KernCopyOut(pData+sizeof(struct jfs_DLIM_enum_header),
				         &d0, sizeof(d0));
			dlim_parms->ulFlags = 0;

			IREAD_UNLOCK(ip);
			jfs_rele(IP2VP(ip));
			break;
		}
		IREAD_UNLOCK(ip);
		jfs_rele(IP2VP(ip));

		/* Re-acquire lock on directory to read next entry */
		IREAD_LOCK(dip);
	}
	freepool(dirent_pool, (caddr_t *)dbufp);
	return rc;
}

/*
 * NAME:	build_path()
 *
 * FUNCTION:	Builds unicode path for directory inode
 *
 * NOTE:	Assumes max path length of MAXPATHLEN+1
 *
 * RETURN:	pointer to beginning of path if successful
 *		NULL if not successful
 */
UniChar	*build_path(
inode_t	*ip,
UniChar	*path_buf)
{
	inode_t		*dip;
	component_t	dirname;
	ino_t		inum;
	ino_t		parent;
	int32		rc;
	UniChar		*path;
	struct vfs	*vfsp;

	IREAD_LOCK(ip);
	if ((ip->i_nlink == 0) || ((ip->i_mode & IFMT) != IFDIR))
	{
		IREAD_UNLOCK(ip);
		return NULL;
	}

	dirname.name = (UniChar *)allocpool(unipool, 0);
	if (dirname.name == 0)
		return NULL;

	path = &path_buf[MAXPATHLEN];
	*path = 0;

	inum = ip->i_number;
	parent = ip->i_parent;
	vfsp = IP2VP(ip)->v_vfsp;
	IREAD_UNLOCK(ip);

	if (inum == ROOT_I)
		*(--path) = '\\';

	while (inum != ROOT_I)
	{
		ICACHE_LOCK();
		rc = iget(vfsp, parent, &dip, 0);
		ICACHE_UNLOCK();
		if (rc)
		{
			freepool(unipool, (caddr_t *)dirname.name);
			return NULL;
		}
		IREAD_LOCK(dip);
		if ((dip->i_nlink == 0) || ((dip->i_mode & IFMT) != IFDIR))
		{
			/* Stale inode */
			IREAD_UNLOCK(dip);
			freepool(unipool, (caddr_t *)dirname.name);
			return NULL;
		}
		rc = dtFindInum(dip, inum, &dirname);
		inum = dip->i_number;
		parent = dip->i_parent;
		IREAD_UNLOCK(dip);
		ICACHE_LOCK();
		iput(dip, vfsp);
		ICACHE_UNLOCK();

		if (rc)
		{
			freepool(unipool, (caddr_t *)dirname.name);
			return NULL;
		}

		path -= dirname.namlen;
		UniStrncpy(path, dirname.name, dirname.namlen);
		*(--path) = '\\';
	}
	*(--path) = ':';
	*(--path) = vfsp->vfs_vpfsi->vpi_drive + 'A';

	freepool(unipool, (caddr_t *)dirname.name);

	return path;
}

// BEGIN D233382
#ifdef _JFS_FASTDASD
/*
 * NAME:	dasd_unlockParents()
 *
 * FUNCTION:	Unlocks parent directories which aren't to be committed.
 *		We don't want to hold inode locks on directories whose
 *		DASD usage has been modified longer than we need to.  This
 *		function unlocks the inodes that aren't otherwise modified
 *		by a transaction.
 *
 * RETURN:	None
 */
void	dasd_unlockParents(
struct dasd_usage *du)
{
	int32	i;
	inode_t	*ip;

	/*
	 * Simply return if directories not locked or if this is a rename
	 */
	if (((du->flag & DLIM_DIRS_LOCKED) == 0) || (du->flag & DLIM_RENAME))
		return;

	ASSERT(du->first_locked == 0);

	for (i = 0; i < du->num_limits; i++)
	{
		ip = du->pLimits[i];
		if (ip->i_dasdlim)
		{
			du->first_locked = i;
			return;
		}
		IWRITE_UNLOCK(ip);
	}
	/*
	 * If we get here, we've unlocked all of them
	 */
	du->flag &= ~DLIM_DIRS_LOCKED;
}
#endif /* _JFS_FASTDASD */
// END D233382
