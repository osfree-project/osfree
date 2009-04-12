/* $Id: log_read.c,v 1.1.1.1 2003/05/21 13:42:12 pasha Exp $ */

static char *SCCSID = "@(#)1.7  9/1/99 08:43:03 src/jfs/utils/libfs/log_read.c, jfslib, w45.fs32, currbld";
/*
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
 *   MODULE_NAME:		log_read.c
 *
 *   COMPONENT_NAME: 	jfslib
 *
 *   FUNCTIONS:  
 *              findEndOfLog()
 *              pageVal
 *              getLogpage
 *              setLogpage
 *              logRead
 *              moveWords()
 *
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>
#include "sysbloks.h"
#include <extboot.h>
#include "bootsec.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_xtree.h"
#include "jfs_logmgr.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_debug.h"
#include "jfs_cntl.h"
#include "logredo.h"
#include "devices.h"
#include "debug.h"

#include "fsckmsgc.h"		/* for chkdsk message logging facility */


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For the chkdsk message logging facility
  *
  *      defined in xchkdsk.c
  */
extern char message_parm_0[];
extern char message_parm_1[];
extern char message_parm_2[];
extern char message_parm_3[];
extern char message_parm_4[];
extern char message_parm_5[];
extern char message_parm_6[];
extern char message_parm_7[];
extern char message_parm_8[];
extern char message_parm_9[];

extern char *msgprms[];
extern int16 msgprmidx[];

extern char *terse_msg_ptr;
extern char *verbose_msg_ptr;

extern char *MsgText[];


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *    R E M E M B E R    M E M O R Y    A L L O C    F A I L U R E
  *
  */
extern int32  Insuff_memory_for_maps;
extern char  *available_stg_addr;
extern int32  available_stg_bytes;
extern char  *bmap_stg_addr;
extern int32  bmap_stg_bytes;


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *   L O C A L   M A C R O    D E F I N I T I O N S
  *
  */
#define BTOLOGPN(x)  ((unsigned)(x) >> L2LOGPSIZE)

#define LOGPNTOB(x)  ((x)<<L2LOGPSIZE)


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *    S T U F F    F O R    T H E    L O G 
  *
  *       externals defined in logredo.c
  */
extern int16   loglocation;     /* 1 = inlinelog, 2 = outlinelog */
extern int32    logminor;       /* minor number of log device */
extern int32    logfd;          /* file descriptor for log */
extern int32    logsize;        /* size of log in pages */
int32    loglastp;              /* last page of log read       */
int32    lognumread;            /* number of log pages read    */

/*
 *      open file system aggregate/lv array
 *     Defined in logredo.c
 */
extern struct vopen vopen[];              /* (88) */


/*
 *      log page buffer cache
 *
 * log has its own 4 page buffer pool.
 *   externals defined in logredo.c
 */

int    nextrep;                 /* next log buffer pool slot to replace */
int    logptr[4];               /* log pages currently held in logp */
logpage_t logp[4];              /* log page buffer pool  */

/*
 * external references
 */
extern errno;
extern int logError( int , int );
extern int32 alloc_storage( int32, void **, int32 * );

extern void fsck_send_msg( int, int, int );		/* defined in fsckmsg.c */

/*
 * forward references
 */
int32 findEndOfLog(void);
int pageVal(int, int*, int*);
int32 getLogpage(int);
int setLogpage(int32 pno,  int32 *,   int32 *, int32 );
int logRead( int32 , struct lrd *, char* );
int32 moveWords( int32  , int32 *, int32 *,  int32 *);


/*
 * NAME:        findEndOfLog()
 *
 * FUNCTION:    Returns the address of the end of the last record in the log.
 *              (i.e. the address of the byte following its descriptor).
 *
 *        Note: At the first release, log page is not written in a ping pong
 *              manner, so the logend is the binary search result
 *
 *              The end of the log is found by finding the page with the
 *              highest page number and for which h.eor == t.eor and
 *              h.eor > 8 (i.e. a record ends on the page).
 *              Page numbers are compared by comparing their difference
 *              with zero (necessary because page numbers are allowed to wrap.)
 *
 * RETURNS:     >0              - byte offset of last record
 *              REFORMAT_ERROR(-3)      - i/o error, reformat the log
 *              MAJOR_ERROR(-2) - other major errors other than EIO.
 */
int32
findEndOfLog()
{

        int32   left, right, pmax, pval, eormax, eorval, k, rc;

        /* binary search for logend
         */
        left = 2;               /* first page containing log records
                                   since page 0 is never used, page 1 is
                                   log superblock */
        right = logsize - 1;    /* last page containing log records */

        if ((rc = pageVal(left, &eormax, &pmax)) < 0)
        {
                sprintf( message_parm_0, "(d) %d", rc  );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_FEOLPGV1FAIL, 0, 1 );
                return(rc);
        }

        while ((right - left) > 1)
        {
                k = (left + right ) >> 1;
                if ((rc = pageVal(k, &eorval, &pval)) < 0)
                {
                        sprintf( message_parm_0, "(d) %d", rc  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_FEOLPGV2FAIL, 0, 1 );
                        return(rc);
                }

                if (pval - pmax > 0 )
                {       left = k;
                        pmax = pval;
                        eormax = eorval;
                }
                else
                        right = k;
        }
        if ((rc = pageVal(right, &eorval, &pval)) < 0)
        {
                sprintf( message_parm_0, "(d) %d", rc  );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_FEOLPGV3FAIL, 0, 1 );
                return(rc);
        }

        /*
         * the last thing to determine is whether it is the first page of
         * the last long log record and system was crashed when its second
         * page is written. If the eor of the chosen page is LOGPHDRSIZE,
         * then this page contains a partial log record, ( otherwise, the
         * the long log record's second page should be chosen ).
         * This page should be thrown away. its previous page will be
         * the real last log page.
         */

        if (( pval - pmax ) > 0 )
        {
                if (eorval == LOGPHDRSIZE )
                {
                        if ((rc = pageVal(right-1, &eorval, &pval)) < 0)
                        {
                           sprintf( message_parm_0, "(d) %d", rc  );
                           msgprms[0] = message_parm_0;
                           msgprmidx[0] = 0;
                           fsck_send_msg( lrdo_FEOLPGV4FAIL, 0, 1 );
                           return(rc);
                        }
                        return(LOGPNTOB(right-1) + eorval);
                } else
                        return(LOGPNTOB(right) + eorval);
        }
        else  /* pmax will be returned */
        {
                if ( eormax == LOGPHDRSIZE )
                {
                        left = (left == 2) ? logsize -1 : left - 1;
                        if ((rc = pageVal(left, &eormax, &pmax)) < 0)
                        {
                           sprintf( message_parm_0, "(d) %d", rc  );
                           msgprms[0] = message_parm_0;
                           msgprmidx[0] = 0;
                           fsck_send_msg( lrdo_FEOLPGV4AFAIL, 0, 1 );
                           return(rc);
                        }
                }
                return(LOGPNTOB(left) + eormax);
        }

}


/*
 * NAME:        pageVal(pno, eor, pmax)
 *
 * FUNCTION:    Read the page into the log buffer pool and call setLogpage
 *              to form consistent log page.
 *
 * RETURNS:     0                       - ok
 *              REFORMAT_ERROR(-3)      - I/O error, reformat the log
 *              MAJOR_ERROR(-2)         - other major errors other than EIO.
 */
pageVal(int pno,        /* page number in log           */
        int *eor,       /* corresponding eor value      */
        int *pmax)      /* pointer to returned page number */
{
        int     rc,
                buf0;           /* logp[] buffer element number         */

        /* Read the page into the log buffer pool.
         */
        if ((buf0 = getLogpage(pno)) < 0)
        {
                sprintf( message_parm_0, "(d) %d", pno  );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                sprintf( message_parm_1, "(d) %d", rc  );
                msgprms[1] = message_parm_1;
                msgprmidx[1] = 0;
                fsck_send_msg( lrdo_PVGETPGFAIL, 0, 2 );

                return(buf0);
        }
        return(setLogpage(pno, eor, pmax, buf0));
}


/*
 * NAME:        getLogpage(pno)
 *
 * FUNCTION:    if the specified log page is in buffer pool, return its
 *              index. Otherwise read log page into buffer pool.
 *
 * PARAMETERS:  pno -   log page number to look for.
 *
 * RETURNS:     0 - 3   - index of the buffer pool the page located
 *              REFORMAT_ERROR(-3)      - I/O error, reformat the log
 *              MAJOR_ERROR(-2)         - other major errors other than EIO.
 */
int32 getLogpage(
int pno)  /* page of log */
{
        int32 k, rc;
        uint32 actual;

        /*
         * is it in buffer pool ?
         */
        for (k = 0; k <= 3; k++)
                if (logptr[k] == pno) return(k);

        /*
         * read page into buffer pool into next slot
         * don't have to use llseek() here.  log dev will never be > 2 gig
         */
        nextrep = (nextrep + 1) % 4;
        if ( loglocation == INLINELOG )
                rc = ujfs_rw_diskblocks( logfd,
                        (uint64)(vopen[logminor].logxaddr+LOGPNTOB(pno)),
                        (unsigned)LOGPSIZE,
                        (char *)&logp[nextrep],
                        GET);
        else
                rc = ujfs_rw_diskblocks( logfd,
                        (uint64)LOGPNTOB(pno),
                        (unsigned)LOGPSIZE,
                        (char *)&logp[nextrep],
                        GET);

        if (rc !=  0 )
        {
                return(JLOG_READERROR1);
        }

        logptr[nextrep] = pno;
        return(nextrep);
}


/*
 * NAME:        setLogpage(pno, eor, pmax, buf)
 *
 * FUNCTION:    Forms consistent log page and returns eor and pmax values.
 *
 *              During the first release the following conditions are
 *              assumed:
 *              1) No corrupted write during power failure
 *              2) No split write
 *              3) No out-of-order sector write
 *
 *              If the header and trailer in the page are not equal, a
 *              system crash happened during this page write. It
 *              is reconciled as follows:
 *
 *              1) if h.page != t.page, the smaller value is taken and
 *                 the eor fields set to LOGPHDSIZE.
 *                 reason: This can happen when a old page is over-written
 *                 by a new page and the system crashed. So this page
 *                 should be considered not written.
 *              2) if h.eor != t.eor, the smaller value is taken.
 *                 reason: The last log page was rewritten for each
 *                 commit record. A system crash happened during the
 *                 page rewriting. Since we assume that no corrupted write
 *                 no split write and out-of-order sector write, the
 *                 previous successfuly writing is still good
 *              3) if no record ends on the page (eor = 8), still return it.
 *                 Let the caller determine whether a) a good long log record
 *                 ends on the next log page. or b) it is the first page of the
 *                 last long log record and system was crashed when its second
 *                 page is written.
 *
 *
 * RETURNS:     0                       - ok
 *              REFORMAT_ERROR(-3)      - I/O error, reformat log
 *              MAJOR_ERROR(-2)         - other major error
 */
setLogpage(int32 pno,   /* page number of log           */
        int32 *eor,     /* log header eor to return     */
        int32 *pmax,    /* log header page number to return */
        int32 buf)      /* logp[] index number for page */
{
        int32 diff1, diff2, rc;
        unsigned long  actual;

        /* check that header and trailer are the same
         */
        if ((diff1 = (logp[buf].h.page - logp[buf].t.page)) != 0)
        {       if (diff1 > 0)
                        logp[buf].h.page = logp[buf].t.page;
                else
                        logp[buf].t.page = logp[buf].h.page;

                logp[buf].h.eor = logp[buf].t.eor = LOGPHDRSIZE;
                                                /* empty page */
        }

        if ((diff2 = (logp[buf].h.eor - logp[buf].t.eor)) != 0)
        {       if (diff2 > 0)
                        logp[buf].h.eor = logp[buf].t.eor;
                else
                        logp[buf].t.eor = logp[buf].h.eor;

        }

        /* if any difference write the page out
         */
        if (diff1 || diff2)
        {
                rc = ujfs_rw_diskblocks(logfd,
                        (uint64)(vopen[logminor].logxaddr+LOGPNTOB(pno)),
                                        (unsigned long)LOGPSIZE,
                                        (char *)&logp[buf],
                                        PUT);
                if ( rc != 0 )
                {
                        sprintf( message_parm_0, "(d) %d", pno  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        sprintf( message_parm_1, "(d) %d", rc  );
                        msgprms[1] = message_parm_1;
                        msgprmidx[1] = 0;
                        fsck_send_msg( lrdo_SLPWRITEFAIL, 0, 2 );

                        return(JLOG_WRITEERROR1);
                }
        }

        /*
         * At this point, it is still possible that logp[buf].h.eor
         * is LOGPHDRSIZE, but we return it anyway. The caller will make
         * decision.
         */

        *eor = logp[buf].h.eor;
        *pmax = logp[buf].h.page;

        return (0);
}

 /*
 * NAME:        logRead(logaddr , ld, dataptr)
 *
 * FUNCTION:    reads the log record addressed by logaddr and
 *              returns the address of the preceding log record.
 *
 * PARAMETERS:  logaddr -  address of the end of log record to read
 *                                 Note: log is read backward, so this is
 *                                 the address starting to read
 *              ld      - pointer to a log record descriptor
 *              dataptr - pointer to data buffer
 *
 * RETURNS:     < 0     - there is an i/o error in reading
 *              > 0     - the address of the end of the preceding log record
 */
logRead(
int32    logaddr,       /* address of log record to read */
struct lrd *ld,  /* pointer to a log record descriptor */
char    * dataptr)    /* pointer to buffer.  LOGPSIZE long */
{
        int buf,off,rc,nwords, pno;

        /* get page containing logaddr into log buffer pool
         */
        pno = BTOLOGPN(logaddr);
        if (pno != loglastp)
        {
                loglastp = pno;
                lognumread += 1;
                if (lognumread > logsize - 2)
                {
                        logError(LOGWRAP,0);

                        sprintf( message_parm_0, "(d) %d", lognumread  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_LRLOGWRAP, 0, 1 );

                        return(JLOG_LOGWRAP);
                }
        }

        buf = getLogpage(pno);
        if (buf < 0)
        {
                sprintf( message_parm_0, "(d) %d", pno  );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                sprintf( message_parm_1, "(d) %d", buf  );
                msgprms[1] = message_parm_1;
                msgprmidx[1] = 0;
                fsck_send_msg( lrdo_LRREADFAIL, 0, 2 );

                return(buf);
        }

        /* read the descriptor */
        off = logaddr & (LOGPSIZE - 1) ;  /* offset just past desc. */
        rc = moveWords(LOGRDSIZE/4, (int32 *)ld, &buf, &off);
        if (rc < 0)
        {
                sprintf( message_parm_0, "(d) %d", rc  );
                msgprms[0] = message_parm_0;
                msgprmidx[0] = 0;
                fsck_send_msg( lrdo_LRMWFAIL1, 0, 1 );
                return(rc);
        }

        /* read the data if there is any */
        if (ld->length > 0)
        {
                if( ld->length > LOGPSIZE ) {                         /* @D1 */
                      rc = READLOGERROR;                              /* @D1 */
                      sprintf( message_parm_0, "(d) %d", pno  );      /* @D1 */
                      msgprms[0] = message_parm_0;                    /* @D1 */
                      msgprmidx[0] = 0;                               /* @D1 */
                      fsck_send_msg( lrdo_LRMWFAIL3, 0, 1 );          /* @D1 */
                      return(rc);                                     /* @D1 */
                      }                                               /* @D1 */

                nwords = (ld->length + 3)/4; /* if length is partial word, still
                                                read it   */
                rc = moveWords(nwords,(int32 *)dataptr,&buf,&off);
                if (rc < 0)
                {
                        sprintf( message_parm_0, "(d) %d", rc  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        fsck_send_msg( lrdo_LRMWFAIL2, 0, 1 );
                        return(rc);
                }
        }

        return(LOGPNTOB(logptr[buf]) + off);
}


/*
 * NAME:        moveWords()
 *
 * FUNCTION:    moves nwords from buffer pool to target. data
 *              is moved in backwards direction starting at offset.
 *              If partial log record is on the previous page,
 *              or we have exhaust the current page (all bytes were read),
 *              the previous page is read into the buffer pool.
 *              On exit buf will point to this page in the buffer pool
 *              and offset to where the move stopped.
 *
 *              Note: the previous page is fetched whenever
 *              the current page is exhausted (all bytes were read)
 *              even if all the words required to satisfy this move
 *              are on the current page.
 *
 * PARAMETERS:  nwords  - number of 4-byte words to move
 *              target  - address of target (begin address)
 *              buf     - index in buffer pool of current page
 *              offset  - initial offset in buffer pool page, this offset
 *                        includes the page head size
 *
 * RETURNS:     = 0             - ok
 *              < 0             - error returned from getLogpage
 */
int32
moveWords(
int32  nwords,  /* number of 4-byte words to move */
int32 *target,  /* address of target (begin address) */
int32 *buf,     /* index in buffer pool of current page */
int32 *offset)  /* initial offset in buffer pool page */
{
        int n,j,words,pno;
        int * ptr;

        j = (*offset - LOGPHDRSIZE)/4 - 1; /* index in log page data area
                                               of first word to move      */
        words  = min(nwords,j + 1);  /* words on this page to move */
        ptr = target + nwords - 1; /* last word of target */
        for (n = 0; n < words; n++)
        {
                *ptr = logp[*buf].data[j];
                j = j - 1;
                ptr = ptr - 1;
        }
        *offset = *offset - 4*words;

        /*
         * If partial log record is on the previous page,
         * or we have read all the log records in the current page,
         * get the previous page
         */

        if ( words != nwords    /* we get less than nwords */
              || j < 0)         /* or exhaust the page, so offset is just */
                                /* the page head, then j < 0              */
        {
                /* get previous page */
                pno = logptr[*buf];
                pno = pno - 1;
                /* if we hit beginning location of the log, go wrapped,
                   read log record from the end location of the log   */
                if (pno == 1) pno = logsize - 1;
                *buf  = getLogpage(pno);
                if (*buf < 0) {
                        sprintf( message_parm_0, "(d) %d", pno  );
                        msgprms[0] = message_parm_0;
                        msgprmidx[0] = 0;
                        sprintf( message_parm_1, "(d) %d", (*buf)  );
                        msgprms[1] = message_parm_1;
                        msgprmidx[1] = 0;
                        fsck_send_msg( lrdo_MWREADFAIL, 0, 2 );

                        return(*buf);
                }
                *offset = LOGPSIZE - LOGPTLRSIZE;
                j = LOGPSIZE/4 - 4 - 1; /* index last word of data area */
                /* move rest of nwords if any. this will never
                exhaust the page.                          */
                for (n = 0; n < nwords - words ; n++) {
                        *ptr = logp[*buf].data[j];
                        j = j - 1;
                        ptr = ptr - 1;
                }
                *offset = *offset - 4*(nwords - words);
        }

        return(0);
}


