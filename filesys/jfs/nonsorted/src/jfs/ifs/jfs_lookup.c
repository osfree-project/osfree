/* $Id: jfs_lookup.c,v 1.1 2000/04/21 10:58:13 ktk Exp $ */

static char *SCCSID = "@(#)1.15  6/15/99 16:17:10 src/jfs/ifs/jfs_lookup.c, sysjfs, w45.fs32, fixbld";
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
 * Module: jfs_lookup.c
 *
 */

/*
 * Change History :
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#ifdef _JFS_OS2
#include "jfs_dirent.h"
#endif /* _JFS_OS2 */
#include "jfs_dnlc.h"
#include "jfs_dtree.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

#include "uni_inln.h"

/* external references
 */
int get_vattr(struct inode *, struct vattr *);
#ifdef _JFS_OS2
struct vfs *VPBtoVFS(SHANDLE hVPB);
int32 dirlookup(struct vfs *vfsp, pathlook_t *plp, struct ucred *crp);
extern pool_t	*dirent_pool;
extern pool_t	*unipool;
#endif


/*
 * NAME:        jfs_lookup(dvp, vpp, pname, flag, vattrp, crp)
 *
 * FUNCTION:    resolve <pname> in directory <dvp> to vnode <vpp>
 *              with a reference acquired and attribute <vattrp>.
 *
 * PARAMETERS:  dvp     _ directory vnode
 *              vpp     - object vnode (out)
 *              pname   - object name
 *              flag    -
 *              vattrp  - object attribute (out)
 *              crp     - credential
 *
 * RETURN:      errors from subroutines.
 */
jfs_lookup(
        struct vnode    *dvp,
        struct vnode    **vpp,
	UniChar		*pname, /* NULL terminated */
        int32           flag,
        struct vattr    *vattrp,
        struct ucred    *crp)
{
        int32   rc = 0;
        struct vfs      *vfsp = dvp->v_vfsp;
        inode_t *dip = VP2IP(dvp);      /* parent directory inode */
        ino_t   ino;            /* object i_number */
        inode_t *ip;            /* object inode */
        component_t     dname;  /* object name */
        ncookie_t       ncookie;
        btstack_t       btstack;

NOISE(1,("jfs_lookup: dip:0x%08x name:%s\n", dip, pname));

        *vpp = NULL;

        /* <dvp> must be a directory */
        if ((dip->i_mode & IFMT) != IFDIR)
                return ENOTDIR;

        IREAD_LOCK(dip);

        if (dip->i_nlink == 0)
        {
                rc = ENOENT;
                goto out;
        }

        /*
         * resolve name to i_number via dnlc/directory lookup
         */
getInumber:

        /*
         * for "." or "..", lookup directory inode
         */
        if (pname[0] == '.')
        {
                /* looking up ".." */
                if (pname[1] == '.' && pname[2] == '\0')
                {
                        ino = dip->i_parent;
                        goto getInode;
                }
                /* looking up "." */
                else if (pname[1] == '\0')
                {
                        ip = dip;

                        jfs_hold(dvp);

                        *vpp = dvp;
                        goto getAttribute;
                }
        }

        /*
         * search dnlc/directory
         */
        dname.name = pname;
	dname.namlen = UniStrlen(pname);
        if ((ino = ncLookup(dip->i_ipimap, dip->i_number, &dname, &ncookie)) == 0)
        {
                /*
                 * dnlc miss: search directory
                 */
                if (rc = dtSearch(dip, &dname, &ino, &btstack, JFS_LOOKUP))
                        goto out;

                /* insert name entry to dnlc */
                ncEnter(dip->i_ipimap, dip->i_number, &dname, ino, &ncookie);
        }

        /*
         * resolve i_number to inode/vnode with a reference
         */
getInode:
        ICACHE_LOCK();
        rc = iget(vfsp, ino, &ip, 0);
        ICACHE_UNLOCK();
        if (rc)
                goto out;

        *vpp = IP2VP(ip);

        /*
         * get attribute
         */
getAttribute:
        if (vattrp != NULL)
                get_vattr(ip, vattrp);

out:
        IREAD_UNLOCK(dip);

NOISE(1,("jfs_lookup: rc:%d\n", rc));
        return rc;
}


#ifdef _JFS_OS2
/*
 * NAME:        pathlookup
 *
 * FUNCTION:    resolve <path> to vnode <vpp>
 *              with a reference acquired.
 *
 * PARAMETERS:  vfsp    - pointer to vfs
 *              plp     - path lookup structure
 *              crp     - credential
 *
 * RETURN:      errors from subroutines.
 *              -1 for a create when last component does not exist
 */
pathlookup(
        struct vfs   *  vfsp,
        pathlook_t   *  plp,
        struct ucred *  crp)
{
        int             rc = 0;
        struct vnode *  vp = NULL;

        /* map the volume handle to a vfs structure.
         */
        if (vfsp == NULL)
                return ENOTDIR;         /* probably should panic */

	if (vfsp->vfs_flag & VFS_ACCEPT)
		return ENOTDIR;		/* We're not really mounted */

        /* Get the vnode for the last directory in the path.
         */
        rc = dirlookup(vfsp, plp, crp);
        if (rc)
                return rc;

        /* special handling for last component of name.
         */

        /* if the last component is null, then we are looking at
         * the root.
         */
        if (*plp->pathlast == '\0')
        {
                /* add a hold to the directory vnode so that it
                 * can be released twice by the caller.
                 */
                vp = plp->dvp;
                jfs_hold(vp);
                plp->statlast = PL_ISDIR;
        }
        else
        {
                /* if there is a wild card in the last component, then
                 * set status accordingly.  Otherwise, call lookup to see
                 * if the name exists and get a vnode.
                 */
		/* This could be done more efficiently */
		if (UniStrchr(plp->pathlast, '*') ||
		    UniStrchr(plp->pathlast, '?'))
                {
                        plp->statlast = PL_WCARD;
                        rc = 0;
                }
                else
                {
                        rc = jfs_lookup(plp->dvp, &vp,
                                        plp->pathlast, 0, NULL, crp);

                        /* set the status of the last component.
                         */
                        switch (rc)
                        {
                        case(0):
                                if ((VP2IP(vp)->i_mode & IFMT) == IFDIR)
                                        plp->statlast = PL_ISDIR;
                                else
                                        plp->statlast = PL_EXIST;
                                break;

                        case(ENOENT):
                                plp->statlast = PL_NOENT;
                                rc = 0;
                                break;

                        default:
                                /* general lookup failure;
                                 * release the vnodes and report an error
                                 */
				if (plp->dlim.num_limits == 0)	// F226941
                                	jfs_rele(plp->dvp);
                                break;
                        }
                }
        }

        /* set the output pointers and return.
         */
        plp->vp = vp;

        return rc;
}


/*
 * NAME:        dirlookup
 *
 * FUNCTION:    Resolve <path> to vnode <vpp>
 *              with a reference acquired.
 *              The last component of the path is not looked up.
 *
 * PARAMETERS:  vfsp    - file system anchor
 *              path    - full path name
 *              last    - last component of path name (out)
 *              vpp     - where to put the new vnode (out)
 *              crp     - credential
 *
 * RETURN:      errors from subroutines.
 */
int32 dirlookup(
        struct vfs *    vfsp,
        pathlook_t *    plp,
        struct ucred *  crp)
{
        int32		rc = 0;
        struct vnode *  dvp;
        struct vnode *  vp;
	UniChar *	compn;
	UniChar *	compp = NULL;
        uint32          complen;
	UniChar		*component;
	struct dasd_usage	*du = &plp->dlim;		// F226941
	inode_t		*ip;					// F226941

	/*
	 *  F226941 - Only accumulate dasd info if limits enabled & operation
	 *  may affect dasd usage
	 */
	if (((vfsp->vfs_flag & VFS_DASDLIM) == 0) || (du->flag & DLIM_READONLY))
		du = 0;

	component = (UniChar *)allocpool(unipool, 0);

        /* get the root vnode as the starting directory.
         */
        vp = vfsp->vfs_mntd;
        jfs_hold(vp);

        /* Address the beginning of the path, skipping over <d>:.
         * Note that the path has been put into canonical form
         * by the IFSM; that is, it has no extraneous backslashes,
         * and . and .. references have been removed.
         */
	compn = UniStrchr(plp->path, '\\');

        /* for each component of the path up to the last:
         *   parse the component name out of the path string,
         *   pass it to jfs_lookup to get a vnode for it.
         */
        for(;;)
        {

// BEGIN F226941
		if (du)
		{
			DLIM_add(du, VP2IP(vp), rc)
			if (rc)
			{
				jfs_rele(vp);
				break;
			}
		}
// END F226941

                /* find end of component; if last component
		 * then UniStrchr will return NULL.
                 */
                compp = compn + 1;
		compn = UniStrchr(compp, '\\');

                /* end loop when we reach the last component
                 */
                if (compn == NULL)
                        break;

                /* copy the string to the component buffer.
                 */
                complen = compn - compp;
		UniStrncpy(component, compp, complen);
                component[complen] = '\0';

                /* promote current vnode to directory vnode.
                 */
                dvp = vp;

                /* call lookup to get a vnode for the component.
                 */
                rc = jfs_lookup(dvp, &vp, component, 0, NULL, crp);

                /* release hold on the directory
                 */
		if (du == 0)					// F226941
			jfs_rele(dvp);

                if (rc != 0)
                   {
                          if (rc == ENOENT)
                             rc = ENOTDIR;

                        break;
                   }
        }

        /* set the output pointers and return
         */
        plp->pathlast = compp;
        plp->dvp = vp;

	freepool (unipool, (caddr_t *)component);

        return rc;
}

/*
 * NAME:        pathlookup_pc
 *
 * FUNCTION:    Lookup path, preserving case.
 *              Lookup path, copying each component into output string
 *		preserving its case.
 *
 * PARAMETERS:  vfsp    - pointer to VFS
 *              path    - full path name
 *              path_pc - output - full path name w/preserved case
 *
 * RETURN:      errors from subroutines.
 *
 * NOTES:	We assume path is in canonical form: "X:\<path>" where there
 *		are no extraneous backslashes and . and .. have been removed.
 *
 *		dtFind is not very efficient for finding a directory entry
 *		without wildcards, but infolevel 7 does not seem to actually
 *		be used anywhere, so it must not be too important.
 */
int32	pathlookup_pc(
	struct vfs	*vfsp,
	UniChar		*path,		/* input - Path */
	UniChar		*path_pc)	/* output - path w/preserved case */
{
	int32		comp_len;
	uint32		count;
	UniChar		*component;
	struct dirent	*dbuf;
	inode_t		*ip;
	int32		offset;
	UniChar		*outptr = path_pc;
	component_t	pattern;
	int		rc;
	UniChar		*slash;
	int32		tbytes;
	struct vnode	*vp;

	if (vfsp == NULL)
	{
		jERROR(2,("pathlookup_pc: invalid VPB!\n"));
		return ENOTDIR;
	}
	/* Copy "X:\" to output string */
	UniStrncpy(outptr, path, 3);
	outptr += 3;
	path += 3;

	if (*path == 0)	
	{
		/* Trivial case "X:\" */
		*outptr = 0;
		return NO_ERROR;
	}

	component = (UniChar *)allocpool(unipool, 0);
	if (component == 0)
		return ENOSPC;
	pattern.name = component;

	dbuf = (struct dirent *)allocpool(dirent_pool, 0);
	if (dbuf == 0)
	{
		freepool(unipool, (caddr_t *)component);
		return ENOSPC;
	}

	vp = vfsp->vfs_mntd;	/* vnode of root directory */
	jfs_hold(vp);

	while (path)
	{
		slash = UniStrchr(path, '\\');
		if (slash)
		{
			comp_len = slash - path;
			slash++;
		}
		else
			comp_len = UniStrlen(path);
		UniStrncpy(component, path, comp_len);
		component[comp_len] = 0;
		UniStrupr(component);		/* Convert to upper case */
		pattern.namlen = comp_len;
		path = slash;

		offset = 0;
		count = 1;
		tbytes = 0;
		rc = dtFind(VP2IP(vp), &pattern, 0, &offset, &count, PSIZE,
			    &tbytes, dbuf);
		jfs_rele(vp);
		if (rc || (count == 0))
		{
			freepool(dirent_pool, (caddr_t *)dbuf);
			freepool(unipool, (caddr_t *)component);
			return ENOENT;
		}
		UniStrncpy(outptr, dbuf->d_name, dbuf->d_namlen);
		outptr += dbuf->d_namlen;

		if (path)
		{
			ICACHE_LOCK();
			rc = iget(vfsp, dbuf->d_ino, &ip, 0);
			ICACHE_UNLOCK();
			if (rc)
			{
				freepool(dirent_pool, (caddr_t *)dbuf);
				freepool(unipool, (caddr_t *)component);
				return rc;
			}
			vp = IP2VP(ip);

			*(outptr++) = '\\';
		}
		else
			*outptr = 0;
	}
	freepool(dirent_pool, (caddr_t *)dbuf);
	freepool(unipool, (caddr_t *)component);
	return NO_ERROR;
}

#endif /* _JFS_OS2 */
