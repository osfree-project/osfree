/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <processes.h>
#include <descrip.h>
#include <dvidef.h>
#include <clidef.h>
#include <climsgdef.h>
#include <ssdef.h>
#include <iodef.h>
#include <jpidef.h>
#include <rmsdef.h>

#include "rexx.h"
#include "strings.h"

struct mbox_status {
   unsigned short status ;
   unsigned short size ;
   int pid ;
} ;

#define BUFSIZE 128
#define NUMBUFS  1

/* #define VMS_DEBUG */

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define MAX_SYM_LENGTH 256

typedef struct { /* vms_tsd: static variables of this module (thread-safe) */
   volatile int       ichan ;
   volatile int       ochan ;
   volatile int       pid ;
   volatile int       oflag ;
   volatile int       comp_stat ;
   int                dead ;
   volatile int       queue ;
   char               buffer[BUFSIZE] ;
   struct mbox_status ostat ;
   struct mbox_status istat ;
   streng *           kill ;
} vms_tsd_t; /* thread-specific but only needed by this module. see
              * init_vms
              */


/* init_vms initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_vms( tsd_t *TSD )
{
   vms_tsd_t *vt;

   if (TSD->vms_tsd != NULL)
      return(1);

   if ((vt = TSD->vms_tsd = MallocTSD(sizeof(vms_tsd_t))) == NULL)
      return(0);
   memset(vt,0,sizeof(vms_tsd_t));  /* correct for all values */
   return(1);
}

#ifdef VMS_DO_COMMAND
/*
 *  At least with OpenVMS 7.3-1 on Alpha, the Posix way seems to work.
 *  So there is no need to redirect on (now) bogus code.
 *  But I keep the code here in case I didn't see something.
 */

static void vms_error (const int error_code)
{
   LIB$SIGNAL(error_code);
   return;
}

static void complain (const tsd_t *TSD, const int rc)
{
   vms_tsd_t *vt;

   vt = TSD->vms_tsd;
#  ifdef VMS_DEBUG
      printf ("About to complain ... rc=%d, pid=%d, ochan=%d, ichan=%d\n",
               rc, vt->pid, vt->ochan, vt->ichan) ;
#  endif

   if ((rc != SS$_NORMAL) && vt->pid)
      sys$delprc( &vt->pid, NULL ), vt->pid=0 ;

/*
   if (vt->ochan) sys$dassgn ((short)vt->ochan), vt->ochan=0 ;
*/
   if (vt->ichan) sys$dassgn ((short)vt->ichan), vt->ichan=0 ;

#  ifdef VMS_DEBUG
      printf( "No more complains left ...about to give error\n" ) ;
#  endif

   if (rc && (rc != SS$_NORMAL)) vms_error (rc) ;

#  ifdef VMS_DEBUG
      printf ("Exiting complain\n") ;
#  endif

   return ;
}

static void read_in_ast( const int read )
{
   streng *ptr ;
   int rc = 0;
   tsd_t *TSD;
   vms_tsd_t *vt;

   TSD = __regina_get_tsd(); /* This seems to be a system callback function.
                              * The TSD must be fetched directly.
                              */
   vt = TSD->vms_tsd;

   if (read) {
      switch ( vt->ostat.status ) {
         case SS$_NORMAL:
            if (vt->ostat.size >= BUFSIZE)
               complain( TSD, SS$_NORMAL ) ;
            ptr = Str_makeTSD( vt->ostat.size ) ;
            ptr = Str_ncatstrTSD( ptr, vt->buffer, vt->ostat.size ) ;
            tmp_stack( TSD, ptr, 1 ) ;
            break ;

         case SS$_ENDOFFILE:
            rc = sys$dassgn( (short)vt->ochan ) ;
            if (rc != SS$_NORMAL) complain( TSD, rc ) ;
            vt->ochan = 0 ;
            rc = sys$setef( vt->oflag ) ;
            if (rc != SS$_NORMAL) complain( TSD, rc ) ;
            break ;

         default:
            fprintf( stderr,
                "sys$qio() return unexpected status value %d\n",
                vt->ostat.status ) ;

            complain( TSD, rc ) ;
      }
   }

   if (vt->ochan) {
      rc = sys$qio(0, (short)vt->ochan, IO$_READVBLK, &vt->ostat,
                    read_in_ast, 1,
                    vt->buffer, BUFSIZE, 0, 0, 0, 0 ) ;
#     ifdef VMS_DEBUG
         printf( "I" ) ;
         fflush( stdout ) ;
#     endif

      if (rc != SS$_NORMAL) complain( TSD, rc ) ;
   }

   return;
}

static void write_out_ast()
{
   int rc, len ;
   tsd_t *TSD;
   vms_tsd_t *vt;

   TSD = __regina_get_tsd(); /* This seems to be a system callback function.
                              * The TSD must be fetched directly.
                              */
   vt = TSD->vms_tsd;

   if (vt->queue++)
      return ;

   start:

   if (vt->kill) {
      FreeTSD( vt->kill ) ;
      vt->kill = NULL ;
   }

   if (! stack_empty( TSD )) {
      vt->kill = popline( TSD, NULL, NULL, 0 ) ;

      if (!vt->ichan) return ;
      rc = sys$qio(0, vt->ichan, IO$_WRITEVBLK, &vt->istat,
                   write_out_ast, 0, vt->kill->value, Str_len(vt->kill), 0, 0,
                   0, 0 ) ;
#     ifdef VMS_DEBUG
         printf( "O" ) ;
         fflush( stdout ) ;
#     endif

      if (rc != SS$_NORMAL) complain( TSD, rc ) ;
   } else {
      if (vt->dead++ >= 5) {
         vt->dead = 0 ;
         return ;
      }
      if (!vt->ichan) return ;
      rc = sys$qio(0, vt->ichan, IO$_WRITEOF, &vt->istat,
                   write_out_ast, 0, 0, 0, 0, 0, 0, 0 ) ;
      if (rc == SS$_IVCHAN) return ;
      if (rc != SS$_NORMAL) complain( TSD, rc ) ;
   }

   if (--vt->queue) goto start ;

   return;
}

#define in (io_flags == REDIR_INPUT)
#define out (io_flags == REDIR_OUTPUT)
#define fout (io_flags == REDIR_OUTFIFO)
int vms_do_command( tsd_t *TSD, const streng *cmd, int io_flags, environment *env, Queue *redir )
{
   struct dsc$descriptor_s name, input, output, prc_name ;
   int fdin[2], fdout[2], strval[2], strval2[2], lim=0, max=0 ;
   int rc, rc1, child, status, fin, eflag, olen, ilen ;
   char line[128], obuf[32], buf2[32], ibuf[32], nbuf[32] ;
   struct mbox_status stat ;
   vms_tsd_t *vt;

   vt = TSD->vms_tsd;
   name.dsc$w_length = Str_len( cmd ) ;
   name.dsc$b_dtype = DSC$K_DTYPE_T ;
   name.dsc$b_class = DSC$K_CLASS_S ;
   name.dsc$a_pointer = (char *)cmd->value ;

   vt->ichan = vt->ochan = 0 ;
   if (in) {
      vt->dead = vt->queue = 0 ;
      rc = sys$crembx(0, &vt->ichan, BUFSIZE, BUFSIZE*NUMBUFS, 0, 0, 0) ;
      if (rc != SS$_NORMAL) complain( TSD, rc ) ;
      strval[0] = sizeof(ibuf) ;
      strval[1] = (int) ibuf ;
      rc = lib$getdvi( &DVI$_DEVNAM, &vt->ichan, 0, 0, strval, &ilen) ;
      if (rc != SS$_NORMAL) complain( TSD, rc ) ;

      input.dsc$w_length = ilen ;
      input.dsc$b_dtype = DSC$K_DTYPE_T ;
      input.dsc$b_class = DSC$K_CLASS_S ;
      input.dsc$a_pointer = ibuf ;

   }

   if (out || fout) {
      rc = sys$crembx(0,&vt->ochan,BUFSIZE,BUFSIZE*NUMBUFS,0,0,0) ;
      if (rc != SS$_NORMAL) complain( TSD, rc ) ;

#     ifdef VMS_DEBUG
         printf( "sys$crembx() ochan=%d, rc=%d\n", vt->ochan, rc ) ;
#     endif

      strval[0] = sizeof(obuf) ;
      strval[1] = (int) obuf ;
      rc=lib$getdvi( &DVI$_DEVNAM, &vt->ochan, 0, 0, strval, &olen) ;
      if (rc != SS$_NORMAL) complain( TSD, rc ) ;

#     ifdef VMS_DEBUG
         printf( "lib$getdvi() name=(%d) <%s>\n", olen, obuf ) ;
#     endif

      output.dsc$w_length = olen ;
      output.dsc$b_dtype = DSC$K_DTYPE_T ;
      output.dsc$b_class = DSC$K_CLASS_S ;
      output.dsc$a_pointer = obuf ;
   }

   sprintf( nbuf, "REXX-%d", getpid()) ;
   prc_name.dsc$w_length = strlen( nbuf ) ;
   prc_name.dsc$b_dtype = DSC$K_DTYPE_T ;
   prc_name.dsc$b_class = DSC$K_CLASS_S ;
   prc_name.dsc$a_pointer = nbuf ;

   if (io_flags == REDIR_OUTPUT || io_flags == REDIR_OUTFIFO) {
      rc = lib$get_ef( &vt->oflag ) ;
      if (rc != SS$_NORMAL) complain( TSD, rc ) ;

      rc = sys$clref( vt->oflag ) ;
/*      if (rc != SS$_NORMAL) complain( TSD, rc ) ; */
   }

   rc = lib$get_ef( &eflag ) ;
   if (rc != SS$_NORMAL) complain( TSD, rc ) ;

   rc = sys$clref( eflag ) ;
/*   if (rc != SS$_NORMAL) complain( TSD, rc ) ; */

   vt->comp_stat = 0 ;
   rc = lib$spawn( &name,
                   ((in) ? &input : NULL),
                   ((out || fout) ? &output : NULL),
                   &CLI$M_NOWAIT, &prc_name, &vt->pid, &vt->comp_stat,
                   &eflag, NULL, NULL, NULL, NULL ) ;

   if (rc != SS$_NORMAL) complain( TSD, rc ) ;

#  ifdef VMS_DEBUG
      printf( "lib$spawn() rc=%d\n", rc ) ;
#  endif

   if (in) write_out_ast() ;

   if (out || fout) read_in_ast( 0 ) ;

#  ifdef VMS_DEBUG
      printf( "Input and output asts started, synching on process\n" ) ;
#  endif

   rc = sys$synch( eflag, NULL ) ;

#  ifdef VMS_DEBUG
      printf( "sys$synch() rc=%d, ochan=%d\n", rc, vt->ochan ) ;
#  endif

   if (vt->ichan) {
      rc = sys$dassgn( (short)vt->ichan ) ;
      vt->ichan = 0 ;
      if (rc != SS$_NORMAL) complain( TSD, rc ) ;
   }

   if (io_flags == REDIR_OUTPUT || io_flags == REDIR_OUTFIFO) {
      rc = sys$synch( vt->oflag, NULL ) ;
      if (vt->ochan)
         printf( "Warning ... output channel still exists ochan=%d\n",
                 vt->ochan);

      if (rc != SS$_NORMAL)
         complain( TSD, rc ) ;

      rc = lib$free_ef( &vt->oflag ) ;
      if (rc != SS$_NORMAL) complain( TSD, rc ) ;
   }

   rc = lib$free_ef( &eflag ) ;
   if (rc != SS$_NORMAL) complain( TSD, rc ) ;

   /*
    * Warning, kludge ahead!!!   When a process under VMS exits, it
    * seems like there is a little delay until the PRCCNT (process
    * count) is decremented. So ... if we just continues without
    * sync'ing up against the PRCCNT, we might get a 'quota exceeded'
    * on the next command (if it is started very soon)
    */

   lib$getjpi( &JPI$_PRCLM, 0, 0, &max, 0, 0 ) ;
   for (lim=max; lim>=max; )
      lib$getjpi( &JPI$_PRCCNT, 0, 0, &lim, 0, 0 ) ;

   complain( TSD, 0 ) ;

#ifdef TODO
   if (out || fout)
      flush_stack( TSD, io_flags ) ;
#endif

/*
 * I have no idea _why_, but bit 28 is sometimes set in the comp_stat.
 * Manuals indicate that this is an internal field, but at least it
 * kills checking against the predefined symbols, so I strip it away.
 * This should most probably have been handled differently, can someone
 * educate me on this?   .... please???
 */
   if ((vt->comp_stat & 0x0fffffff) == CLI$_NORMAL) vt->comp_stat = SS$_NORMAL ;
   return (((vt->comp_stat & 0x0fffffff)==SS$_NORMAL) ? 0 : vt->comp_stat) ;
}
#endif

int vms_killproc( tsd_t *TSD )
{
   vms_tsd_t *vt;

   vt = TSD->vms_tsd;
   if (vt->pid)
      sys$delprc( &vt->pid, NULL ) ;

   vt->pid = 0 ;
   return 0;
}

streng *vms_resolv_symbol( tsd_t *TSD, streng *name, streng *new, streng *pool )
{
   struct dsc$descriptor_s sym_name, sym_val, new_val ;
   char buffer[MAX_SYM_LENGTH] ;
   unsigned int length=0 ;
   int rc ;
   streng *old ;
   vms_tsd_t *vt;

   vt = TSD->vms_tsd;
   sym_name.dsc$w_length = Str_len( name ) ;
   sym_name.dsc$b_dtype = DSC$K_DTYPE_T ;
   sym_name.dsc$b_class = DSC$K_CLASS_S ;
   sym_name.dsc$a_pointer = name->value ;

   if (new) {
      new_val.dsc$w_length = Str_len( new ) ;
      new_val.dsc$b_dtype = DSC$K_DTYPE_T ;
      new_val.dsc$b_class = DSC$K_CLASS_S ;
      new_val.dsc$a_pointer = new->value ;
   }

   sym_val.dsc$w_length = MAX_SYM_LENGTH ;
   sym_val.dsc$b_dtype = DSC$K_DTYPE_T ;
   sym_val.dsc$b_class = DSC$K_CLASS_S ;
   sym_val.dsc$a_pointer = buffer ;

   if (strncmp( pool->value, "SYMBOL", MAX(6,Str_len(pool))) ||
       strncmp( pool->value, "SYSTEM", MAX(6,Str_len(pool))))
   {
      rc = lib$get_symbol( &sym_name, &sym_val, &length ) ;
      if (new)
         lib$set_symbol( &sym_name, &new_val ) ;
   }
   else if (strncmp( pool->value, "LOGICAL", MAX(7, Str_len(pool))))
   {
/*    rc = lib$get_logical( ... ) */
      if (new)
         lib$set_symbol( &sym_name, &new_val ) ;
      else
         lib$delete_logical( &sym_name ) ;
   } else {
      return(NULL) ;
   }

   old = Str_makeTSD( length ) ;
   Str_ncatstrTSD( old, buffer, length ) ;
   return(old) ;
}
