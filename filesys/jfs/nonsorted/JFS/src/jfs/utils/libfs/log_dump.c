/* $Id: log_dump.c,v 1.1.1.1 2003/05/21 13:42:02 pasha Exp $ */

static char *SCCSID = "@(#)1.10  3/12/99 09:53:55 src/jfs/utils/libfs/log_dump.c, jfslib, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		log_dump.c
 *
 *   COMPONENT_NAME: 	jfslib
 *
 *   FUNCTIONS:  jfs_logdump.c: write the current log records to \JFSLOG.DMP
 *
 *   NOTES:	**************** This is a SERVICE-ONLY TOOL ****************
 *
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


#define LOGDMP_OK 	0
#define LOGDMP_FAILED	-1


#define MAKEDEV(__x,__y)        (dev_t)(((__x)<<16) | (__y))

#define LOGPNTOB(x)  ((x)<<L2LOGPSIZE)

#define LOG2NUM(NUM, L2NUM)\
{\
        if ((NUM) <= 0)\
                L2NUM = -1;\
        else\
        if ((NUM) == 1)\
                L2NUM = 0;\
        else\
        {\
                L2NUM = 0;\
                while ( (NUM) > 1 )\
                {\
                        L2NUM++;\
                        (NUM) >>= 1;\
                }\
        }\
}

/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 *       things for the log.
 */
extern int16   loglocation;     /* 1 = inlinelog, 2 = outlinelog */
extern int32    logmajor;       /* major number of log device */
extern int32    logminor;       /* minor number of log device */
int32    logserial;             /* log serial number in super block */
int32    logend;                /* address of the end of last log record */
int32    logfd;                 /* file descriptor for log */
int32    logsize;               /* size of log in pages */
logsuper_t logsup;              /* log super block */


int32 numdoblk;                 /* number of do blocks used     */

int32 numnodofile;                      /* number of nodo file blocks used  */

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * The output file.
  *
  */

FILE *outfp;

#define  output_filename  "\\JFSLOG.DMP"

int logdmp_outfile_is_open = 0;


/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 *      open file system aggregate/lv array
 *
 * logredo() processes a single log.
 * at the first release, logredo will process a single log
 * related to one aggregate. But the future release, logredo needs to
 * to process one single log related to multiple agreegates.
 * In both cases, the aggregate(logical volume) where the log stays
 * will be different from  the file system aggregate/lv.
 *
 * There will be one imap for the aggregate inode allocation map
 * and a list of imap pointers to multiple fileset inode allocation maps.
 *
 * There is one block allocation map per aggregate and shared by all the
 * filesets within the aggregate.
 *
 * the log and related aggregates (logical volumes) are all in
 * the same volume group, i.e., each logical volume is uniquely specified
 * by their minor number with the same major number,
 * the maximum number of lvs in a volume group is NUMMINOR (256).
 */
struct vopen vopen[NUMMINOR];              /* (88) */

/* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *
 *      file system page buffer cache
 *
 * for k > 0, bufhdr[k] describes contents of buffer[k-1].
 * bufhdr[0] is reserved as anchor for free/lru list:
 * bufhdr[0].next points to the MRU buffer (head),
 * bufhdr[0].prev points to the LRU buffer (tail);
 */

/* buffer header table */
struct bufhdr
{
        int16           next;           /* 2: next on free/lru list */
        int16           prev;           /* 2: previous on free/lru list */
        int16           hnext;          /* 2: next on hash chain */
        int16           hprev;          /* 2: previous on hash chain */
        char            modify;         /* 1: buffer was modified */
        char            inuse;          /* 1: buffer on hash chain */
        int16           reserve;        /* 2 */
        int32           vol;            /* 4: minor of agrregate/lv number */
        pxd_t           pxd;            /* 8: on-disk page pxd */
} bufhdr[NBUFPOOL];                     /* (24) */

/* buffer table */
struct bufpool
{
        char    bytes[PSIZE];
} buffer[NBUFPOOL - 1];


/*
 *      log page buffer cache
 *
 * log has its own 4 page buffer pool.
 */
uint8    afterdata[LOGPSIZE];   /* buffer to read in redopage data */

/*
 * Miscellaneous
 */
caddr_t prog;                           /* Program name */
int32 mntcnt, bufsize;
char *mntinfo;
int32 retcode;                          /* return code from logredo    */

char    loglockpath[MAXPATHLEN + 1];    /* log lock file                */

/*
 * external references
 */
extern  char *optarg;
extern  int optind;
extern errno;
extern int initMaps( int32 );
extern int updateMaps(int);
extern int32 findEndOfLog(void);
extern int logRead( int32 , struct lrd *, char* );
extern int32 logredoInit(void);
extern int32 alloc_storage( int32, void **, int32 * );

extern int alloc_wrksp( uint32, int, int, void ** );	/* defined in fsckwsp.c */

/*
 * forward references
 */
int32 open_outfile( void );
int ldmp_doMount( struct lrd * );
int32 ldmp_openVol( int32  );
int32 ldmp_rdwrSuper( int32 ,   struct superblock *, int32 );
int ldmp_isLogging( caddr_t , int32 , char *, int32 );
int32 ldmp_isFilesystem( caddr_t , int32 );
int32 ldmp_logOpen(void);
int ldmp_fsError( int , int , int64 );
int ldmp_logError( int , int );
int usage (void);

int32 disp_updatemap ( lrd_t * );
int32 disp_redopage ( lrd_t * );
int32 disp_noredopage ( lrd_t * );
int32 disp_noredoinoext ( lrd_t * );

void ldmp_xdump (char *,int);
int ldmp_x_scmp(char*, char*);
void ldmp_x_scpy(char*, char*);
int prtdesc(struct lrd *);


/* --------------------------------------------------------------------
 *
 * NAME:        jfs_logdump()
 *
 * FUNCTION:
 *
 */

int32  jfs_logdump( caddr_t pathname, int32 fd, int32 dump_all )
{
        int32   rc;
        int32   k,logaddr,nextaddr,lastaddr, nlogrecords;
        unsigned long   actual;
        int  syncrecord, word, bit;
        struct lrd ld;
        int logformit;
        unsigned long dummyd = 0;
        unsigned long dummyp = 0;
        logsuper_t *plogsup = &logsup;
        int64  aggsb_numpages = 0;
        int32 lowest_lr_byte = 2 * LOGPSIZE + LOGPHDRSIZE;
        int32 highest_lr_byte = 0;
        int log_has_wrapped = 0;

      rc = open_outfile();

      if( rc == 0 ) { /* output file is open */

        /*
         * loop until we get enough memory to read vmount struct
         */
        mntinfo = (char *)&bufsize;
        bufsize = sizeof(int);

        /*
         * validate that the log is not currently in use;
         */
        rc = ldmp_isLogging(pathname, fd, mntinfo, mntcnt);

        if (rc < 0 )
        {
                printf( "JFS_LOGDUMP:Error occurred when open/read device\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "JFS_LOGDUMP:Error occurred when open/read device\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                return (rc);
        }

        /*
         *      open log
         */
        logfd = ldmp_logOpen();


        /*
         * validate log superblock
         *
         * aggregate block size is for log file as well.
         */

        rc = ujfs_rw_diskblocks( logfd,
                (uint64)(vopen[logminor].logxaddr+LOGPNTOB(LOGSUPER_B)),
                (unsigned)sizeof(logsuper_t),
                (char *)&logsup,
                GET);
        if (rc != 0)
        {
                printf( "JFS_LOGDUMP:couldn't read log superblock:failure in %s\n",prog );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "JFS_LOGDUMP:couldn't read log superblock:failure in %s\n",prog );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                return (LOGSUPER_READ_ERROR);
        }

        fprintf(outfp, "JOURNAL SUPERBLOCK: \n");
        fprintf(outfp, "------------------------------------------------------\n");
        fprintf(outfp, "   magic number: x %lx \n", logsup.magic );
        fprintf(outfp, "   version     : x %lx \n", logsup.version );
        fprintf(outfp, "   serial      : x %lx \n", logsup.serial );
        fprintf(outfp, "   size        : t %ld pages (4096 bytes/page)\n", logsup.size );
        fprintf(outfp, "   bsize       : t %ld bytes/block\n", logsup.bsize );
        fprintf(outfp, "   l2bsize     : t %ld \n", logsup.l2bsize );
        fprintf(outfp, "   flag        : x %lx \n", logsup.flag );
        fprintf(outfp, "   state       : x %lx \n", logsup.state );
        fprintf(outfp, "   end         : x %lx \n", logsup.end );
        fprintf(outfp, "\n");
        fprintf(outfp, "======================================================\n");
        fprintf(outfp, "\n");

        if (logsup.magic != LOGMAGIC )
        {
            fprintf(outfp, "\n");
            fprintf(outfp, "**WARNING** %s: %s is not a log file\n", prog ,pathname);
            fprintf(outfp, "\n");
            fprintf(outfp, "======================================================\n");
            fprintf(outfp, "\n");
        }

        if (logsup.version != LOGVERSION)
        {
            fprintf(outfp, "\n");
            fprintf(outfp, "**WARNING** %s and log file %s version mismatch\n", prog ,pathname);
            fprintf(outfp, "\n");
            fprintf(outfp, "======================================================\n");
            fprintf(outfp, "\n");
        }

        if (logsup.state == LOGREDONE)
        {
            fprintf(outfp, "\n");
            fprintf(outfp, "**WARNING** %s and log file %s state is LOGREDONE\n", prog ,pathname);
            fprintf(outfp, "\n");
            fprintf(outfp, "======================================================\n");
            fprintf(outfp, "\n");
        }

        logsize = logsup.size;
        logserial = logsup.serial;

        /*
         * find the end of log
         */
        logend = findEndOfLog();
        if (logend  < 0)
        {
printf("logend < 0\n");
                ldmp_logError(LOGEND,0);
                rc = ujfs_rw_diskblocks(logfd,
                      (uint64)(vopen[logminor].logxaddr+LOGPNTOB(LOGSUPER_B)),
                                        (unsigned long)LOGPSIZE,
                                        (char *)&logsup,
                                        PUT);
                rc = logend;
                goto loopexit;
        }

        highest_lr_byte = logsup.size * LOGPSIZE - LOGRDSIZE;

        if ( (logend  < lowest_lr_byte) || (logend > highest_lr_byte) ) {
            fprintf(outfp, "\n");
            fprintf(outfp, "**ERROR** logend address is not valid for a logrec. logend: 0x0%lx\n", logend);
            fprintf(outfp, "\n");
            fprintf(outfp, "======================================================\n");
            fprintf(outfp, "\n");
            return(INVALID_LOGEND);
            }

        /*
         *      replay log
         *
         * read log backwards and process records as we go.
         * reading stops at place specified by first SYNCPT we
         * encounter.
         */
        nlogrecords = lastaddr = 0;
        nextaddr = logend;
        do
        {
                logaddr = nextaddr;
                nextaddr = logRead(logaddr, &ld, afterdata);
                fprintf( outfp, 
                         "logrec d %ld   Logaddr= x %lx   Nextaddr= x %lx   Backchain = x %lx\n",
		         nlogrecords,logaddr,nextaddr,ld.backchain);
                fprintf( outfp, "\n" );
                nlogrecords += 1;
	/*
	 *
	 * Validate the nextaddr as much as possible 
	 *
	 */
                if (nextaddr < 0)
                {
                        ldmp_logError(READERR,logaddr);
                        if (nextaddr == REFORMAT_ERROR)
                        {
                                rc = nextaddr;
                                goto loopexit;
                        }
                        break;
                }
		/*
	 	 * Certain errors we'll assume signal the end of the log
		 * since we're just dumping everything from the latest
		 * commit record to the earliest valid record.
		 */
                if ( (nextaddr  < lowest_lr_byte) || (nextaddr > highest_lr_byte) ) {
                    lastaddr = logaddr; 
                    }

                if ( nextaddr  == logaddr ) {
                    lastaddr = logaddr; 
                    }

                if(  nextaddr > logaddr ) {
                    if( log_has_wrapped ) {
                        fprintf(outfp, "\n");
                        fprintf(outfp, "**ERROR** log wrapped twice. logaddr:0x0%lx nextaddr:0x0%lx\n",logaddr, nextaddr);
                        fprintf(outfp, "\n");
                        fprintf(outfp, "======================================================\n");
                        fprintf(outfp, "\n");
                        lastaddr = logaddr; 
                        }
                    else {
                        log_has_wrapped = -1;
                        }
                    }
	/*
	 *
	 * The addresses seem ok.  Process the current record.
	 *
	 */
         if( lastaddr != logaddr )  {
            switch(ld.type) {

                case LOG_COMMIT:
                    fprintf( outfp, 
                             "LOG_COMMIT     (type = d %d)     logtid = d %ld\n",
                             ld.type, ld.logtid );
                    fprintf(outfp, "\n");
                    fprintf( outfp, "\tdata length = d %ld\n",ld.length );
 
                    break;

                case LOG_MOUNT:
                    fprintf(outfp, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                    fprintf( outfp, 
                             "LOG_MOUNT      (type = d %d)     logtid = d %ld\n",
                             ld.type, ld.logtid );
                    fprintf(outfp, "\n");
                    fprintf( outfp, "\tdata length = d %ld\n",ld.length );
                    fprintf(outfp, "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                    break;

                case LOG_SYNCPT:
                    fprintf(outfp, "****************************************************************\n");
                    fprintf( outfp, 
                             "LOG_SYNCPT     (type = d %d)     logtid = d %ld\n",
                             ld.type, ld.logtid );
                    fprintf(outfp, "\n");
                    fprintf( outfp, "\tdata length = d %ld\n",ld.length );
                    fprintf( outfp, "\tsync = x %lx\n",ld.log.syncpt.sync );
                    fprintf(outfp, "****************************************************************\n");

                    rc = 0;
                    if( !dump_all ) {	/* user just wants from last synch point forward */
                            if (lastaddr == 0) {
                                    syncrecord = logaddr;
                                    lastaddr = (ld.log.syncpt.sync == 0)
                                            ? logaddr
                                            : ld.log.syncpt.sync;
                                    }
                            }  /* end user just wants from last synch point forward */
                    break;

                case LOG_REDOPAGE:
                    fprintf( outfp, 
                             "LOG_REDOPAGE   (type = d %d)     logtid = d %ld\n",
                             ld.type, ld.logtid );
                    fprintf(outfp, "\n");
                    fprintf( outfp, "\tdata length = d %ld    ",ld.length );
                    disp_redopage( &ld );
                    break;

                case LOG_NOREDOPAGE:
                    fprintf( outfp, 
                             "LOG_NOREDOPAGE   (type = d %d)     logtid = d %ld\n",
                             ld.type, ld.logtid );
                    fprintf(outfp, "\n");
                    fprintf( outfp, "\tdata length = d %ld    ",ld.length );
                    disp_noredopage( &ld );
                    break;

                case LOG_NOREDOINOEXT:
                    fprintf( outfp, 
                             "LOG_NOREDOINOEXT (type = d %d)     logtid = d %ld\n",
                             ld.type, ld.logtid );
                    fprintf(outfp, "\n");
                    fprintf( outfp, "\tdata length = d %ld    ",ld.length );
                    disp_noredoinoext( &ld );
                    break;

                case LOG_UPDATEMAP:
                    fprintf( outfp, 
                             "LOG_UPDATEMAP    (type = d %d)     logtid = d %ld\n",
                             ld.type, ld.logtid );
                    fprintf(outfp, "\n");
                    fprintf( outfp, "\tdata length = d %ld    ",ld.length );
                    disp_updatemap( &ld );
                    break;

                default:
                    fprintf( outfp, 
                             "*UNRECOGNIZED*   (type = d %d)     logtid = d %ld\n",
                             ld.type, ld.logtid );
                    fprintf(outfp, "\n");
                    fprintf( outfp, "\tdata length = d %ld\n",ld.length );
                    fprintf(outfp, "\n");
                    fprintf(outfp, "**ERROR** unrecognized log record type\n");
                    fprintf(outfp, "\n");
                    fprintf(outfp, "======================================================\n");
                    fprintf(outfp, "\n");
                    return( UNRECOG_LOGRECTYP );
                }

            if (rc == 0)
                {
                    fprintf(outfp, "\n" );
                    if( ld.length > 0 ) {
                        ldmp_xdump( (char *) afterdata, ld.length );
                        }
                }

            fprintf(outfp, "\n" );
            fprintf(outfp, "----------------------------------------------------------------------\n");
            }        /* end if( lastaddr != logaddr )  */

        } while (logaddr != lastaddr);

loopexit:
  
          /*
           * Close the output file
           */
        if( logdmp_outfile_is_open ) {
          fclose( outfp );
          }

        if( rc == 0 ) {  /* log has been dumped successfully */
          printf( 
              "JFS_LOGDUMP: The current JFS log has been dumped into \\JFSLOG.DMP" 
                );
          }  /* end log has been dumped successfully */
        else {
          printf( "JFS_LOGDUMP:Failed in %s\n", prog );
          fprintf(outfp, "??????????????????????????????????????????????????????\n");
          fprintf(outfp, "JFS_LOGDUMP:Failed in %s\n", prog );
          fprintf(outfp, "??????????????????????????????????????????????????????\n");
          }
        }  /* end output file is open */

        return (rc < 0) ? (rc) : (0);
}


/*----------------------------------------------------------------
 *
 * NAME:        ldmp_doMount(ld)
 *
 * FUNCTION:    a log mount record is the first-in-time record which is
 *              put in the log so it is the last we want to process in
 *              logredo. so we mark volume as cleanly unmounted in vopen
 *              array. the mount record is imperative when the volume
 *              is a newly made filesystem.
 */
int
ldmp_doMount(
struct lrd * ld)        /* pointer to record descriptor */
{
        int vol, status;

        vol = 0;
        status = vopen[vol].status;

        if (status != FM_LOGREDO && status != FM_DIRTY)
                vopen[vol].status = FM_CLEAN;

        return (0);
}

/*----------------------------------------------------------------
 *
 * NAME:        ldmp_openVol(vol)
 *
 * FUNCTION:    open the aggregate/volume specified.
 *              check if it was cleanly unmounted. also check log
 *              serial number. initialize disk and inode mpas.
 */
int32 ldmp_openVol(
        int32   vol)    /* device minor number of aggregate/lv */
{
        int32   k, fd, rc, l2agsize, agsize;
        int64   fssize; /* number of aggre. blks in the aggregate/lv   */
        struct superblock       sb;

        fd = vopen[logminor].fd;

        /* read superblock of the aggregate/volume */
        if ((rc = ldmp_rdwrSuper(fd, &sb, PB_READ)) !=0)
        {
                printf( "ldmp_openVol: ldmp_rdwrSuper failed\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "ldmp_openVol: ldmp_rdwrSuper failed\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                ldmp_fsError(READERR,vol, SUPER1_B);
                vopen[vol].fd = 0;
                return(MINOR_ERROR);
        }

        /* check magic number and initialize version specific
         * values in the vopen struct for this vol.
         */
        if (strncmp(sb.s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC)) == 0)
        {
                if (sb.s_version != JFS_VERSION)
                {
                        printf( "ldmp_openVol:version number not right %d\n",vol );
                        fprintf(outfp, "??????????????????????????????????????????????????????\n");
                        fprintf(outfp, "ldmp_openVol:version number not right %d\n",vol );
                        fprintf(outfp, "??????????????????????????????????????????????????????\n");
                        vopen[vol].fd = 0;
                        return(MINOR_ERROR);
                }

                if ( loglocation == OUTLINELOG &&
                     (sb.s_flag & JFS_INLINELOG == JFS_INLINELOG ))
                {
                        printf( "ldmp_openVol: log location wrong\n" );
                        fprintf(outfp, "??????????????????????????????????????????????????????\n");
                        fprintf(outfp, "ldmp_openVol: log location wrong\n" );
                        fprintf(outfp, "??????????????????????????????????????????????????????\n");
                        vopen[vol].fd = 0;
                        return(MAJOR_ERROR);
                }
                if ( loglocation != INLINELOG)
                {
                        return(MAJOR_ERROR);
                }
                vopen[vol].lblksize = sb.s_bsize;
                vopen[vol].l2bsize = sb.s_l2bsize;
                vopen[vol].l2bfactor = sb.s_l2bfactor;
                fssize = sb.s_size >> sb.s_l2bfactor;
                vopen[vol].fssize = fssize;
                vopen[vol].agsize = sb.s_agsize;
                /* LOG2NUM will alter agsize, so use local var */
                agsize = vopen[vol].agsize;
                LOG2NUM(agsize, l2agsize);
                vopen[vol].numag = fssize >> l2agsize;
                if ( fssize & (vopen[vol].agsize - 1 ))
                        vopen[vol].numag += 1;
                vopen[vol].l2agsize = l2agsize;
        }
        else
        {
                printf( "ldmp_openVol: magic number not right %d\n",vol );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "ldmp_openVol: magic number not right %d\n",vol );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                vopen[vol].fd = 0;
                return(MINOR_ERROR);
        }

        /* set lbperpage in vopen.
         */
        vopen[vol].lbperpage = PSIZE >> vopen[vol].l2bsize;

        /* was it cleanly umounted ?
         */
        if (sb.s_state == FM_CLEAN)
        {
                vopen[vol].status  = FM_CLEAN;
                vopen[vol].fd = 0;
                return(0);
        }

        /* else get status of volume
         */
        vopen[vol].status = sb.s_state;
        vopen[vol].is_fsdirty = ( sb.s_state == FM_DIRTY );

        /* check log serial number
         */
        if (sb.s_logserial != logserial)
        {
                printf( "ldmp_openVol: bad serial number\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "ldmp_openVol: bad serial number\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                vopen[vol].fd = 0;
                ldmp_fsError(SERIALNO, vol, SUPER1_B);
                return(MINOR_ERROR);
        }

        /* initialize the disk and inode maps
         */
        if ((rc = initMaps(vol)) !=0)
        {
                printf( "ldmp_openVol: initMaps failed\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "ldmp_openVol: initMaps failed\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                ldmp_fsError(MAPERR,vol,0);
        }
        return (rc);
}


/*----------------------------------------------------------------
 *
 * NAME:        ldmp_rdwrSuper(fd, sb, rwflag)
 *
 * FUNCTION:    read or write the superblock for the file system described
 *              by the file descriptor of the opened aggregate/lv.
 *              for read, if a read of primary superblock is failed,
 *              try to read the secondary superblock. report error only
 *              when both reads failed.
 *              for write, any write failure should be reported.
 */
int32 ldmp_rdwrSuper(
        int32   fd,             /* file descriptor */
        struct  superblock *sb, /* superblock of the opened aggregate/lv */
        int32   rwflag)         /* PB_READ, PB_UPDATE   */
{
        int32 rc;
        uint32 actual;
        union {
                struct superblock super;
                char block[PSIZE];
        } super;

        /*
         * seek to the postion of the primary superblock.
         * since at this time we don't know the aggregate/lv
         * logical block size yet, we have to use the fixed
         * byte offset address SUPER1_OFF to seek for.
         */

        /*
         * read super block
         */
        if (rwflag == PB_READ)
        {
                rc = ujfs_rw_diskblocks(fd, SUPER1_OFF, 
                                        (unsigned)SIZE_OF_SUPER,
                                        super.block, GET);
                if ( rc != 0 )
                {
                        printf( "ldmp_rdwrSuper: read primary agg superblock failed. errno=%d  Continuing.\n", errno );
                        fprintf(outfp, "??????????????????????????????????????????????????????\n");
                        fprintf(outfp, "ldmp_rdwrSuper: read primary agg superblock failed. errno=%d Continuing\n", errno );
                        fprintf(outfp, "??????????????????????????????????????????????????????\n");
                        /* read failed for the primary superblock:
                         * try to read the secondary superblock
                         */
                        rc = ujfs_rw_diskblocks(fd, SUPER2_OFF,
                                                (unsigned)SIZE_OF_SUPER,
                                                super.block, GET);
                        if ( rc != 0 )
                        {
                                printf( "ldmp_rdwrSuper: read 2ndary agg superblock failed. errno=%d  Cannot continue.\n", errno );
                                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                                fprintf(outfp, "ldmp_rdwrSuper: read 2ndary agg superblock failed. errno=%d  Cannot continue.\n", errno );
                                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                                return(MAJOR_ERROR);
                        }
                }

                *sb = super.super;
        }
        /*
         * write superblock
         */
        else /* PB_UPDATE */
        {
                printf( "ldmp_rdwrSuper: log_dump attempting to write!!!\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "ldmp_rdwrSuper: log_dump attempting to write!!!\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                return(MAJOR_ERROR);
        }

        return (0);
}


/*----------------------------------------------------------------
 *
 * NAME:        ldmp_isLogging()
 *
 * FUNCTION:    Check to see if device is currently in use as a log
 *
 * NOTES: mntctl and stat are AIX system calls.  major and minor number are
 *        also AIX concept. They could be changed in OS/2 system.
 *
 * DATA STRUCTURES: global variables logmajor and logminor are altered.
 *
 * PARAMETERS:  logname - device name
 *              fd      - file handle for the logname
 *              vmt     - pointer to buffer that contains an array of
 *                        vmount sturctures, which are returned by the
 *                        mntctl system call.
 *              cnt     - number of vmount structures in buffer pointed by vmt
 *
 * RETURNS:     0       - device logname is not actively used as a log
 *              NOT_FSDEV_ERROR (-6) Not a valid fs device (from ldmp_isFilesystem)
 *              NOT_INLINELOG_ERROR (-7) Log is not an inline log  (from ldmp_isFilesystem)
 *              MAJOR_ERROR - returned from ldmp_isFilesystem().
 *              MINOR_ERROR - devices already mounted that are using logname
 */
ldmp_isLogging(
        caddr_t logname,
        int32   fd,
        char    *vmt_in,
        int32   cnt)
{
        int32 i, rc = 0;

        /*
         * determine if logname represents a file system device  or
         * a log device and set its major number
         */
        if ((rc = ldmp_isFilesystem(logname, fd)) < 0)
                return(rc);

        return (rc);
}


/*----------------------------------------------------------------
 *
 * NAME:        ldmp_isFilesystem()
 *
 * FUNCTION:     open the device to see if it's a valid filesystem.
 *               If open failed, then return MAJOR_ERROR.
 *               If open ok, and it is a valid file system,
 *               return the minor device number of the log for this
 *               filesystem. Otherwise, return MINOR_ERROR.
 *
 * PRE CONDITION: other process opened the device log should allow
 *                a O_RDONLY re-open.
 *
 * POST CONDITION: this O_RDONLY open is closed.
 *
 * PARAMETERS:  dev_name        - device name. This is the name passed to
 *                                logredo.
 *
 * RETURNS:
 *              NOT_FSDEV_ERROR (-6) Not a valid fs device
 *              NOT_INLINELOG_ERROR (-7) Log is not an inline log -
 *              MAJOR_ERROR(-2) -  open device  or stat device failure, or
 *                                 s_logdev number wrong.
 *                                 for OS/2, if it is not an inlinelog, it is
 *                                 an error.
 *              0               -  1)read fs superblock ok but dev_name does not
 *                                   represent a file system device name
 *                                 2)read fs fs superblock ok, dev_name
 *                                   represents a fs device name.
 *                                   loglocation is set up. ( INLINELOG or
 *                                   OUTLINELOG ).
 *                                 3)read fs superblock failed, but successful
 *                                   to read the device as log
 */
int32 ldmp_isFilesystem(
        caddr_t dev_name,
        int32   fd)
{
        int32 rc = 0;
        struct superblock sb;
        int32 devmajor = 0;
        int32 devminor = 0;
        uint32 Action, actual;

        /*
         * for _JFS_OS2, each file system must maintain its own in-line log.
         */

        /*
         * try the LV as file system with in-line log
         */
        if ((rc = ldmp_rdwrSuper(fd, &sb, PB_READ)) == 0)
        {
                /*
                 * is the LV a file system ?
                 */
                if (strncmp(sb.s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC))
                                         == 0 )
                {
                        /*
                         * does file system contains its in-line log ?
                         */
                        if ( ( sb.s_flag & JFS_INLINELOG ) == JFS_INLINELOG )
                        {

                                if ( !( logmajor == devmajor &&
                                        logminor == devminor ) )
                                {

                printf( "ldmp_isFilesystem: s_logdev is not a fs dev number\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "ldmp_isFilesystem: s_logdev is not a fs dev number\n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                                        return(NOT_FSDEV_ERROR);

                                }

                                loglocation = INLINELOG;
                                vopen[devminor].fd = fd;
                                vopen[devminor].log_pxd = sb.s_logpxd;
                                vopen[devminor].l2bsize = sb.s_l2bsize;
                                vopen[devminor].logxaddr =
                                      addressPXD(&sb.s_logpxd) << sb.s_l2bsize;
                        }
                        /*
                         * the FS is associated with external/out-of-line log
                         */
                        else
                        {
                                rc = NOT_INLINELOG_ERROR;
                        }
                }
                /*
                 * validation failure: the LV is not a file system:
                 */
                else
                {
                        rc = NOT_FSDEV_ERROR;
                }
        }
        /*
         * read failure: try the LV as out-of-line log
         */
        else
        {
                rc = NOT_INLINELOG_ERROR;
        }
        return (rc);
}


/*----------------------------------------------------------------
 *
 * NAME:        ldmp_logOpen()
 *
 * FUNCTION:    opens the log and returns its fd.
 *              sets logmajor to the major number of the device.
 *
 * PRE CONDITION: logmajor and logminor have been set up
 *
 * POST CONDITION: log is opened and locked by the loglock file
 *
 * PARAMETERS: NONE
 *
 * RETURNS:     >= 0    - file descriptor reported from makeOpen()
 *              < 0     - any error reported from makeOpen()
 */
int32 ldmp_logOpen()
{
        int k;
        int fd;

        fd = vopen[logminor].fd;

        return(fd);
}


extern void exit(int);


/*----------------------------------------------------------------
 *
 * NAME:        ldmp_fsError(type,vol,bn)
 *
 * FUNCTION:    error handling code for the specified
 *              aggregate/lv (filesystem).
 */
ldmp_fsError(
int type,       /* error types */
int vol,        /* the minor number of the aggregate/lv */
int64 bn)       /* aggregate block No.  */
{
        int status;

        printf( "ldmp_fsError:bad error in volume %d \n", vol );
        fprintf(outfp, "??????????????????????????????????????????????????????\n");
        fprintf(outfp, "ldmp_fsError:bad error in volume %d \n", vol );

        retcode = -1;
        vopen[vol].status = FM_LOGREDO;

        switch(type) {
        case OPENERR:
                printf( "Open failed \n" );
                fprintf(outfp, "Open failed \n" );
                break;
        case MAPERR:
                printf( "can not initialize maps \n" );
                fprintf(outfp, "can not initialize maps \n" );
                break;
        case DBTYPE:
                printf( "bad disk block number %lld\n", bn );
                fprintf(outfp, "bad disk block number %lld\n", bn );
                break;
        case INOTYPE:
                printf( "bad inode number %lld\n", bn );
                fprintf(outfp, "bad inode number %lld\n", bn );
                break;
        case READERR:
                printf( "can not read block number %lld\n", bn );
                fprintf(outfp, "can not read block number %lld\n", bn );
                break;
        case SERIALNO:
                printf( "log serial number no good /n" );
                fprintf(outfp, "log serial number no good /n" );
                break;
        case IOERROR:
                printf( "io error reading block number %lld\n", bn );
                fprintf(outfp, "io error reading block number %lld\n", bn );
                break;
        case LOGRCERR:
                printf( "UpdateMap log rec error. nxd=%d\n", bn );
                fprintf(outfp, "UpdateMap log rec error. nxd=%d\n", bn );
                 break;
        }

        fprintf(outfp, "??????????????????????????????????????????????????????\n");

        return (0);
}


/*----------------------------------------------------------------
 *
 *      ldmp_logError(type)
 *
 * error handling for log read errors.
 */
ldmp_logError(
int type,
int logaddr)
{
        int k;

        retcode = -1;
        logsup.state = LOGREADERR;

        switch(type) {
        case LOGEND:
                printf( "ldmp_logError:find end of log failed \n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "ldmp_logError:find end of log failed \n" );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                break;
        case READERR:
                printf( "log read failed 0x%x\n",logaddr );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "log read failed 0x%x\n",logaddr );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                break;
        case UNKNOWNR:
                printf( "unknown log record type \nlog read failed 0x%x\n",logaddr );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "unknown log record type \nlog read failed 0x%x\n",logaddr );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                break;
        case IOERROR:
                printf( "i/o error log reading page 0x%x\n",logaddr );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                fprintf(outfp, "i/o error log reading page 0x%x\n",logaddr );
                fprintf(outfp, "??????????????????????????????????????????????????????\n");
                break;
        case LOGWRAP:
                printf( "log wrapped...\n" );
                fprintf(outfp, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
                fprintf(outfp, "log wrapped...\n" );
                fprintf(outfp, "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
        }


        /* mark all open volumes in error
         */
        for(k = 0; k < NUMMINOR; k++)
        {
                if (vopen[k].fd && vopen[k].status != FM_CLEAN)
                        vopen[k].status = FM_LOGREDO;
        }

        return (0);
}
/*----------------------------------------------------------------
 *
 *      ldmp_xdump()
 *
 * hex dump
 */
void ldmp_xdump (
char *saddr,
int count)
{
#define LINESZ     60
#define ASCIISTRT    40
#define HEXEND       36

    int i, j, k, hexdigit;
    register int c;
    char *hexchar;
    char linebuf[LINESZ+1];
    char prevbuf[LINESZ+1];
    char *linestart;
    int asciistart;
    char asterisk = ' ';
    void ldmp_x_scpy ();
    int ldmp_x_scmp ();

    hexchar = "0123456789ABCDEF";
    prevbuf[0] = '\0';
    i = (int) saddr % 4;
    if (i != 0)
        saddr = saddr - i;

    for (i = 0; i < count;) {
        for (j = 0; j < LINESZ; j++)
            linebuf[j] = ' ';

        linestart = saddr;
        asciistart = ASCIISTRT;
        for (j = 0; j < HEXEND;) {
            for (k = 0; k < 4; k++) {
                c = *(saddr++) & 0xFF;
                if ((c >= 0x20) && (c <= 0x7e))
                    linebuf[asciistart++] = (char) c;
                else
                    linebuf[asciistart++] = '.';
                hexdigit = c >> 4;
                linebuf[j++] = hexchar[hexdigit];
                hexdigit = c & 0x0f;
                linebuf[j++] = hexchar[hexdigit];
                i++;
            }
            if (i >= count)
                break;
            linebuf[j++] = ' ';
        }
        linebuf[LINESZ] = '\0';
        if (((j = ldmp_x_scmp (linebuf, prevbuf)) == 0) && (i < count)) {
            if (asterisk == ' ') {
                asterisk = '*';
                fprintf(outfp, "    *\n");
            }
        }
        else {
            fprintf(outfp, "    %x  %s\n",linestart, linebuf);
            asterisk = ' ';
            ldmp_x_scpy (prevbuf, linebuf);
        }
    }

    return;
}


/*----------------------------------------------------------------
 *
 *      ldmp_x_scmp()
 *
 */
int ldmp_x_scmp(
register char *s1,
register char* s2)
{
    while ((*s1) && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    if (*s1 || *s2)
        return(-1);
    else
        return(0);
}


/*----------------------------------------------------------------
 *
 *      ldmp_x_scpy()
 *
 */
void ldmp_x_scpy(
register char *s1,
register char *s2)
{
    while ((*s1 = *s2) != '\0') {
        s1++;
        s2++;
    }
}


/***************************************************************************
 *
 * NAME: disp_noredopage
 *
 * FUNCTION:  
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: LOGDMP_OK
 *      failure: something else
 */
int32 disp_noredopage ( lrd_t * lrd_ptr )
{
  fprintf( outfp, "fileset = d %ld    inode = d %ld (x %lx)\n", 
           lrd_ptr->log.noredopage.fileset, 
	   lrd_ptr->log.noredopage.inode,
           lrd_ptr->log.noredopage.inode );

  switch( lrd_ptr->log.noredopage.type ) {
    case LOG_INODE:
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:INODE\n", 
                lrd_ptr->log.noredopage.type );
       break;
    case LOG_XTREE:
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:XTREE\n  ",
                lrd_ptr->log.noredopage.type );
       break;
    case (LOG_XTREE | LOG_NEW):
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:XTREE_NEW\n  ",
                lrd_ptr->log.noredopage.type );
       break;
    case (LOG_BTROOT | LOG_XTREE):
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:BTROOT_XTREE\n  ",
                lrd_ptr->log.noredopage.type );
       break;
    case LOG_DTREE:
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:DTREE\n  ",
                lrd_ptr->log.noredopage.type );
       break;
    case (LOG_DTREE | LOG_NEW):
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:DTREE_NEW \n ",
                lrd_ptr->log.noredopage.type );
       break;
    case (LOG_DTREE | LOG_EXTEND):
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:DTREE_EXTEND\n  ",
                lrd_ptr->log.noredopage.type );
       break;
    case (LOG_BTROOT | LOG_DTREE):
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:BTROOT_DTREE\n  ",
                lrd_ptr->log.noredopage.type );
       break;
    case (LOG_BTROOT | LOG_DTREE | LOG_NEW):
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:BTROOT_DTREE.NEW\n  ",
                lrd_ptr->log.noredopage.type );
       break;
    case LOG_EA:
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:EA\n", 
                lrd_ptr->log.noredopage.type );
       break;
    case LOG_ACL:
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:ACL\n", 
                lrd_ptr->log.noredopage.type );
       break;
    case LOG_DATA:
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:DATA\n", 
                lrd_ptr->log.noredopage.type );
       break;
/*
    case LOG_NOREDOFILE:
       fprintf( outfp, "\ttype = d %d NOREDOPAGE:NOREDOFILE\n", 
                lrd_ptr->log.noredopage.type );
       break;
*/
    default:
       fprintf( outfp, "\ttype = d %d ***UNRECOGNIZED***\n", 
                lrd_ptr->log.noredopage.type );
       break;
    }

  fprintf( outfp, "\tpxd length = d %ld   phys offset = x %llx  (d %lld)\n", 
           lengthPXD( &(lrd_ptr->log.noredopage.pxd) ),
           addressPXD( &(lrd_ptr->log.noredopage.pxd) ),
           addressPXD( &(lrd_ptr->log.noredopage.pxd) ) );

  return( LOGDMP_OK );
}                                 /* end of disp_noredopage() */


/***************************************************************************
 *
 * NAME: disp_noredoinoext
 *
 * FUNCTION:  
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: LOGDMP_OK
 *      failure: something else
 */
int32 disp_noredoinoext ( lrd_t * lrd_ptr )
{
  fprintf( outfp, "fileset = d %ld  \n", 
           lrd_ptr->log.noredoinoext.fileset );

  fprintf( outfp, "\tiag number = d %ld   extent index = d %ld\n", 
           lrd_ptr->log.noredoinoext.iagnum,
           lrd_ptr->log.noredoinoext.inoext_idx );

  fprintf( outfp, "\tpxd length = d %ld   phys offset = x %llx  (d %lld)\n", 
           lengthPXD( &(lrd_ptr->log.noredoinoext.pxd) ),
           addressPXD( &(lrd_ptr->log.noredoinoext.pxd) ),
           addressPXD( &(lrd_ptr->log.noredoinoext.pxd) ) );

  return( LOGDMP_OK );
}                                 /* end of disp_noredopage() */


/***************************************************************************
 *
 * NAME: disp_redopage
 *
 * FUNCTION:  
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: LOGDMP_OK
 *      failure: something else
 */
int32 disp_redopage ( lrd_t * lrd_ptr )
{
  fprintf( outfp, "fileset = d %ld    inode = d %ld (x %lx)\n", 
           lrd_ptr->log.redopage.fileset, lrd_ptr->log.redopage.inode,
           lrd_ptr->log.redopage.inode );

  switch( lrd_ptr->log.redopage.type ) {
    case LOG_INODE:
       fprintf( outfp, "\ttype = d %d REDOPAGE:INODE\n", 
                lrd_ptr->log.redopage.type );
       break;
    case LOG_XTREE:
       fprintf( outfp, "\ttype = d %d REDOPAGE:XTREE\n  ",
                lrd_ptr->log.redopage.type );
       break;
    case (LOG_XTREE | LOG_NEW):
       fprintf( outfp, "\ttype = d %d REDOPAGE:XTREE_NEW\n  ",
                lrd_ptr->log.redopage.type );
       break;
    case (LOG_BTROOT | LOG_XTREE):
       fprintf( outfp, "\ttype = d %d REDOPAGE:BTROOT_XTREE\n  ",
                lrd_ptr->log.redopage.type );
       break;
    case LOG_DTREE:
       fprintf( outfp, "\ttype = d %d REDOPAGE:DTREE\n  ",
                lrd_ptr->log.redopage.type );
       break;
    case (LOG_DTREE | LOG_NEW):
       fprintf( outfp, "\ttype = d %d REDOPAGE:DTREE_NEW \n ",
                lrd_ptr->log.redopage.type );
       break;
    case (LOG_DTREE | LOG_EXTEND):
       fprintf( outfp, "\ttype = d %d REDOPAGE:DTREE_EXTEND\n  ",
                lrd_ptr->log.redopage.type );
       break;
    case (LOG_BTROOT | LOG_DTREE):
       fprintf( outfp, "\ttype = d %d REDOPAGE:BTROOT_DTREE\n  ",
                lrd_ptr->log.redopage.type );
       break;
    case (LOG_BTROOT | LOG_DTREE | LOG_NEW):
       fprintf( outfp, "\ttype = d %d REDOPAGE:BTROOT_DTREE.NEW\n  ",
                lrd_ptr->log.redopage.type );
       break;
    case LOG_EA:
       fprintf( outfp, "\ttype = d %d REDOPAGE:EA\n", 
                lrd_ptr->log.redopage.type );
       break;
    case LOG_ACL:
       fprintf( outfp, "\ttype = d %d REDOPAGE:ACL\n", 
                lrd_ptr->log.redopage.type );
       break;
    case LOG_DATA:
       fprintf( outfp, "\ttype = d %d REDOPAGE:DATA\n", 
                lrd_ptr->log.redopage.type );
       break;
/*
    case LOG_NOREDOFILE:
       fprintf( outfp, "\ttype = d %d REDOPAGE:NOREDOFILE\n", 
                lrd_ptr->log.redopage.type );
       break;
*/
    default:
       fprintf( outfp, "\ttype = d %d ***UNRECOGNIZED***\n", 
                lrd_ptr->log.redopage.type );
       break;
    }
  fprintf( outfp, "\tl2linesize = d %ld    ", 
           lrd_ptr->log.redopage.l2linesize );
  fprintf( outfp, "pxd length = d %ld   phys offset = x %llx  (d %lld)\n", 
           lengthPXD( &(lrd_ptr->log.redopage.pxd) ),
           addressPXD( &(lrd_ptr->log.redopage.pxd) ),
           addressPXD( &(lrd_ptr->log.redopage.pxd) ) );

  return( LOGDMP_OK );
}                                 /* end of disp_redopage() */


/***************************************************************************
 *
 * NAME: disp_updatemap
 *
 * FUNCTION:  
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: LOGDMP_OK
 *      failure: something else
 */
int32 disp_updatemap ( lrd_t *lrd_ptr )
{
  int flag_unrecognized = -1;
  fprintf( outfp, "fileset = d %ld    inode = d %ld (x %lx)\n", 
  lrd_ptr->log.updatemap.fileset, lrd_ptr->log.updatemap.inode,
  lrd_ptr->log.updatemap.inode );

  fprintf( outfp, "\ttype = x %x UPDATEMAP: ", 
           lrd_ptr->log.updatemap.type );  

  if( (lrd_ptr->log.updatemap.type & LOG_ALLOCXADLIST) == LOG_ALLOCXADLIST ) {
    flag_unrecognized = 0;
    fprintf( outfp, " ALLOCXADLIST" );
    }
  if( (lrd_ptr->log.updatemap.type & LOG_ALLOCPXDLIST) == LOG_ALLOCPXDLIST ) {
    flag_unrecognized = 0;
    fprintf( outfp, " ALLOCPXDLIST" );
    }
  if( (lrd_ptr->log.updatemap.type & LOG_ALLOCXAD) == LOG_ALLOCXAD ) {
    flag_unrecognized = 0;
    fprintf( outfp, " ALLOCXAD" );
    }
  if( (lrd_ptr->log.updatemap.type & LOG_ALLOCPXD) == LOG_ALLOCPXD ) {
    flag_unrecognized = 0;
    fprintf( outfp, " ALLOCPXD" );
    }
  if( (lrd_ptr->log.updatemap.type & LOG_FREEXADLIST) == LOG_FREEXADLIST ) {
    flag_unrecognized = 0;
    fprintf( outfp, " FREEXADLIST" );
    }
  if( (lrd_ptr->log.updatemap.type & LOG_FREEPXDLIST) == LOG_FREEPXDLIST ) {
    flag_unrecognized = 0;
    fprintf( outfp, " FREEPXDLIST" );
    }
  if( (lrd_ptr->log.updatemap.type & LOG_FREEXAD) == LOG_FREEXAD ) {
    flag_unrecognized = 0;
    fprintf( outfp, " FREEXAD" );
    }
  if( (lrd_ptr->log.updatemap.type & LOG_FREEPXD) == LOG_FREEPXD ) {
    flag_unrecognized = 0;
    fprintf( outfp, " FREEPXD" );
    }
  if( flag_unrecognized ) {
    fprintf( outfp, " *** UNRECOGNIZED ***" );
    }

  fprintf( outfp, "\n" );
   
  fprintf( outfp, "\tnxd = d %ld  (number of extents)\n", 
           lrd_ptr->log.updatemap.nxd );
  fprintf( outfp, "\tpxd length = d %ld   phys offset = x %llx  (d %lld)\n", 
           lengthPXD( &(lrd_ptr->log.updatemap.pxd) ),
           addressPXD( &(lrd_ptr->log.updatemap.pxd) ),
           addressPXD( &(lrd_ptr->log.updatemap.pxd) ) );

  return( LOGDMP_OK );
}                                 /* end of disp_updatemap() */


/*****************************************************************************
 * NAME: open_outfile
 *
 * FUNCTION:  Open the output file.
 *
 * PARAMETERS:
 *      Device  - input - the device specification
 *
 * NOTES:
 *
 * RETURNS:
 *      success: XCHKLOG_OK
 *      failure: something else
 */
int32 open_outfile ( )
{
  int32    openof_rc = 0;
  
  outfp = fopen( output_filename, "w");

  if( outfp == NULL ) {  /* output file open failed */
    printf( "LOG_DUMP: unable to open output file: \\JFSLOG.DMP\n\r" );
    openof_rc = -1;
    }  /* end output file open failed */

  else {
    logdmp_outfile_is_open = -1;
    }

  return( openof_rc );
}                             /* end of open_outfile ( ) */
      
