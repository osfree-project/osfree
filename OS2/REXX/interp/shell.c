/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
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

#include "regina_c.h"

#if defined(WIN32)
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#   pragma warning(disable: 4115 4201 4214 4514)
#  endif
# endif
# include <windows.h>
# ifdef _MSC_VER
#  if _MSC_VER >= 1100
#   pragma warning(default: 4115 4201 4214)
#  endif
# endif
#endif

#include "rexx.h"
#include <stdio.h>

#include <string.h>
#include <signal.h>
#include <errno.h>
#ifdef HAVE_ASSERT_H
# include <assert.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#if defined(VMS)
# define fork() vfork()

#if 0
/* lets try and see if VMS is smart enough now to do this posixly */
# ifdef  posix_do_command
#  undef posix_do_command
# endif
# define posix_do_command __regina_vms_do_command
#endif
#endif

#define STD_IO     0x00
#define QUEUE      0x01
#define LIFO       0x02
#define FIFO       0x04
#define STREAM     0x08
#define STEM       0x10
#define STRING     0x20
#define LIFOappend 0x80
#define FIFOappend 0x100

#if defined(_POSIX_PIPE_BUF) && !defined(PIPE_BUF)
# define PIPE_BUF _POSIX_PIPE_BUF
#endif

typedef struct { /* shl_tsd: static variables of this module (thread-safe) */
   char         *cbuff ;
   int           child ;
   int           status ;
   int           running ;
   void         *AsyncInfo ;
   unsigned char IObuf[REGINA_MAX_BUFFER_SIZE]; /* write cache */
   unsigned      IOBused;
} shl_tsd_t; /* thread-specific but only needed by this module. see
              * init_shell
              */

/* init_shell initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_shell( tsd_t *TSD )
{
   shl_tsd_t *st;

   if ( TSD->shl_tsd != NULL )
      return(1);

   if ( ( TSD->shl_tsd = MallocTSD( sizeof(shl_tsd_t) ) ) == NULL )
      return(0);
   st = (shl_tsd_t *)TSD->shl_tsd;
   memset( st, 0, sizeof(shl_tsd_t) );  /* correct for all values */
   return(1);
}

static const streng *stem_access( tsd_t *TSD, environpart *e, int pos,
                                  streng *value )
/* appends "."+itoa(pos) to e->currname and accesses this variable.
 * value is NULL to access the current value or non-NULL to set the new value.
 * The return value is NULL if a new value is set or the old one.
 */
{
   int leaflen ;

   leaflen = sprintf( e->currname->value + e->currnamelen, "%d", pos ) ;

   e->currname->len = e->currnamelen + leaflen ;

   /*
    * FGC: Changed back from get_it_anyway_compound to getvalue.
    *      This will raise a NOVALUE condition if we try to read
    *      unexpected data.
    *      Furthermore we allow the user to use extended stems
    *      like "A.B." if we ever want to allow it. We do so by
    *      changing from [sg]etdirvalue_compound to [sg]etvalue.
    */
   if (value == NULL)
      return( getvalue( TSD, e->currname, -1 ) ) ;

   setvalue( TSD, e->currname, value, -1 ) ;
   return( NULL ) ;
}

static void set_currname( tsd_t *TSD, environpart *e )
/* Sets the initial currname of the environpart from its name. e->currname
 * will be freed at first if not set to NULL.
 */
{
   const streng *source = e->name;

   if (e->flags.ant == antSIMSYMBOL)
   {
      /*
       * Just in case of a stem we access the variable directly. Otherwise
       * we have to resolve the symbol.
       */
      if ( (source->len > 0) && ( e->flags.awt != awtSTEM ) )
      {
         source = getvalue( TSD, source, -1 ) ;
      }
   }
   else
   {
      assert( ( e->flags.ant == antSTRING ) || ( source == NULL ) );
   }

   if (e->currname != NULL)
      Free_stringTSD( e->currname ) ;

   /*
    * We need space for "." and the maximal number, but first check if we
    * have a "default" value.
    */
   if ( source == NULL )
   {
      e->currname = NULL ;
      return ;
   }
   e->currnamelen = Str_len( source ) ;
   e->currname = Str_makeTSD( e->currnamelen + 3*sizeof(int) ) ;
   memcpy( e->currname->value, source->value, e->currnamelen ) ;
   e->currname->len = e->currnamelen ; /* pro forma, will be recomputed */
}

static void prepare_env_io( environpart *e )
/*
 * Prepares the WITH-IO-redirection from envpart.
 */
{
   e->SameAsOutput = 0;
   e->FileRedirected = 0;
   e->tempname = NULL; /* none as default, might become char* RedirTempFile */
   e->queue = NULL;
   e->tmp_queue = NULL;
   e->type = STD_IO;
   e->hdls[0] = e->hdls[1] = e->hdls[2] = -1;
}

static void open_env_io( tsd_t *TSD, environpart *e, unsigned overwrite, int isString )
/* Opens the WITH-IO-redirection from envpart and sets *flag to either
 * STREAM or STEM. Nothing happens if there isn't a redirection.
 * The protect-fields are used by the
 */
{
   const streng *h ;
   int error ;
   unsigned awt ;
   char code ;

   if ( ( e->name == NULL ) && !overwrite && ( e->flags.awt == awtUNKNOWN ) )
      return ;

   set_currname(TSD, e);
   if ( overwrite == awtUNKNOWN )
      awt = e->flags.awt ;
   else
      awt = overwrite ;
   switch (awt)
   {
      case awtSTREAM:
         /*
          * For a STREAM input/output redirection, set the file reopen
          * flag, and reopen the file.
          */
         e->type = STREAM;
         if ( e->flags.isinput )
            code = 'r';
         else if ( e->flags.append )
            code = 'A';
         else /* REPLACE */
            code = 'R';
         if ( e->flags.isinput || !e->SameAsOutput )
            e->file = addr_reopen_file( TSD, e->currname, code,
                                        e->flags.iserror );
         break;

      case awtSTEM:
         /*
          * For a STEM input/output redirection, check that existing state of
          * the stem if appropriate and initialise the stem
          */
         e->type = STEM ;

         if (e->flags.isinput || e->flags.append)
         {
            /*
             * For a STEM input redirection, the stem must already exist and be
             * a valid Rexx "array". This happens to an existing output stem in
             * the append mode, too.
             */
            h = stem_access( TSD, e, 0, NULL ) ;
            /* h must be a whole positive number */
            e->maxnum = streng_to_int( TSD, h, &error ) ;
            if (error || (e->maxnum < 0))
               exiterror( ERR_INVALID_STEM_OPTION, /* needs stem.0 and      */
                          1,                       /* (stem.0) as arguments */
                          tmpstr_of( TSD, e->currname ),
                          tmpstr_of( TSD, h ) )  ;
            e->currnum = (e->flags.isinput) ? 1 : e->maxnum + 1;
         }
         else /* must be REPLACE */
         {
            e->maxnum = 0 ;
            e->currnum = 1 ;
            e->base->value[0] = '0' ;
            e->base->len = 1 ;
            stem_access( TSD, e, 0, Str_dupTSD( e->base ) ) ;
         }
         break;

      case awtLIFO:
         if ( overwrite != awtUNKNOWN )
         {
            if ( e->flags.isinput ) /* "LIFO> cmd ... " */
            {
               e->type = QUEUE;
               e->queue = addr_reopen_queue( TSD, NULL, 'r' ) ; /* current */
            }
            else
            {
               if ( isString )
                  e->type = STRING ;
               else
               {
                  e->type = LIFOappend ;
                  e->queue = addr_reopen_queue( TSD, NULL, 'A' ) ; /* current */
               }
            }
         }
         else /* "normal" use of WITH ??? LIFO ??? */
         {
            if ( e->flags.isinput ) /* "LIFO> cmd ... " */
            {
               e->type = QUEUE;
               e->queue = addr_reopen_queue( TSD, e->currname, 'r' ) ;
               if ( e->queue == NULL )
                  exiterror( ERR_EXTERNAL_QUEUE, 109, tmpstr_of( TSD, e->currname ) ) ;
            }
            else
            {
               e->type = LIFO;
               if (e->flags.append)
               {
                  e->type = LIFOappend;
                  code = 'A' ;
               }
               else /* REPLACE */
               {
                  e->type = LIFO;
                  code = 'R' ;
               }
               e->queue = addr_reopen_queue( TSD, e->currname, code ) ;
               if ( e->queue == NULL )
                  exiterror( ERR_EXTERNAL_QUEUE, 109, tmpstr_of( TSD, e->currname ) ) ;
            }
         }
         break;

      case awtFIFO:
         if ( overwrite != awtUNKNOWN )
         {
            if ( e->flags.isinput ) /* "FIFO> cmd ... " */
            {
               e->type = QUEUE;
               e->queue = addr_reopen_queue( TSD, NULL, 'r' ) ; /* current */
            }
            else
            {
               if ( isString )
                  e->type = STRING ;
               else
               {
                  e->type = FIFOappend ;
                  e->queue = addr_reopen_queue( TSD, NULL, 'A' ) ; /* current */
               }
            }
         }
         else /* "normal" use of WITH ??? FIFO ??? */
         {
            if ( e->flags.isinput ) /* "FIFO> cmd ... " */
            {
               e->type = QUEUE;
               e->queue = addr_reopen_queue( TSD, e->currname, 'r' ) ;
               if ( e->queue == NULL )
                  exiterror( ERR_EXTERNAL_QUEUE, 109, tmpstr_of( TSD, e->currname ) ) ;
            }
            else
            {
               if (e->flags.append)
               {
                  e->type = FIFOappend;
                  code = 'A' ;
               }
               else /* REPLACE */
               {
                  e->type = FIFO;
                  code = 'R' ;
               }
               e->queue = addr_reopen_queue( TSD, e->currname, code ) ;
               if ( e->queue == NULL )
                  exiterror( ERR_EXTERNAL_QUEUE, 109, tmpstr_of( TSD, e->currname ) ) ;
            }
         }
         break;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "Illegal address code in open_env_io()" )  ;
   }
}

void put_stem( tsd_t *TSD, environpart *e, streng *str )
/* Adds one line to the stem specified in e. */
{
   e->maxnum = e->currnum ;
   e->currnum++ ;
   e->base->len = sprintf( e->base->value, "%d", e->maxnum ) ;
   stem_access( TSD, e, 0, Str_dupTSD( e->base ) ) ;
   stem_access( TSD, e, e->maxnum, str ) ;
}

static int write_buffered(const tsd_t *TSD, int hdl, const void *buf,
                                               unsigned size, void *async_info)
{
   int rc, done;
   unsigned todo;
   shl_tsd_t *st = (shl_tsd_t *)TSD->shl_tsd;

   if ((buf == NULL) || (size == 0)) /* force flush buffers */
   {
      do
      {
         if (st->IOBused)
         {
            /* the following call will return -EAGAIN if not completed */
            rc = TSD->OS->write_exec(hdl, st->IObuf, st->IOBused, async_info);
         }
         else
            rc = 0;
         if (rc >= 0)
         {
            if (rc == (int) st->IOBused)
            {
               st->IOBused = 0;
               rc = TSD->OS->write_exec(hdl, NULL, 0, async_info);
            }
            else
            {
               memmove(st->IObuf, st->IObuf + rc, st->IOBused - rc);
               st->IOBused -= rc;
            }
         }
         else
            TSD->OS->write_exec(hdl, NULL, 0, async_info);
         /*
          * continue until everything written or an error like -EAGAIN
          * is available. -EAGAIN will let the caller come back to this
          * function when writing is possible again, so everything
          * will be written after a while.
          */
      } while (st->IOBused && (rc >= 0));

      return(rc);
   }

   done = 0;
   while (size) {
      /* step 1: fill buffer up to the maximum */
      todo = size;
      if (todo > sizeof(st->IObuf) - st->IOBused)
         todo = sizeof(st->IObuf) - st->IOBused;
      if (todo > 0)
      {
         memcpy(st->IObuf + st->IOBused, buf, todo);
         st->IOBused += todo;
      }
      done += todo; /* dropped to the buffer --> done for upper layer */

      /* step 2: flush buffer, if buffer filled */
      if (st->IOBused < sizeof(st->IObuf))
         return(done);

      /* step 3: buffer full, giving optimal performance (I hope!) */
      rc = TSD->OS->write_exec(hdl, st->IObuf, st->IOBused, async_info);
      if (rc <= 0)
      {
         if (done)
            break; /* something done sucessfully */
         return(rc);
      }
      if (rc == (int) st->IOBused)
         st->IOBused = 0;
      else
      {
         memmove(st->IObuf, st->IObuf + rc, st->IOBused - rc);
         st->IOBused -= rc;
      }

      /* just try another chunk of the input buffer */
      buf = (const char *) buf + todo;
      size -= todo;
   }

   return(done); /* something done sucessfully */
}

static int feed( const tsd_t *TSD, streng **string, int hdl, void *async_info )
/* Writes the content of *string into the file associated with hdl. The
 * string is shortened and, after the final write, deleted and *string set
 * to NULL.
 * async_info is both a structure and a flag. If set, asynchronous IO shall
 * be used, otherwise blocked IO has to be used.
 * feed returns 0 on success or an errno value on error.
 * A return value of EAGAIN is set if we have to wait.
 */
{
   unsigned total ;
   int done ;

   if ((string == NULL) || (*string == NULL))
      return( 0 ) ;

   total = Str_len( *string ) ;
   if (total == 0)
      return( 0 ) ;

   done = write_buffered(TSD, hdl, (*string)->value, total, async_info);
   if (done <= 0)
   {
      if (done == 0)      /* no error set? */
         done = ENOSPC ;  /* good assumption */
      else
         done = -done;
      if ((done != EAGAIN) && (done != EPIPE))
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, strerror(done) )  ;
      return( done ) ; /* error */
   }

   if ((unsigned) done < total)
   {
      (*string)->len -= done ;
      memmove((*string)->value, (*string)->value + done, (*string)->len);
   }
   else
   {
      assert((unsigned)done==total);
      Free_stringTSD(*string);
      *string = NULL;
   }
   return(0);
}

static int reap( const tsd_t *TSD, streng **string, int hdl, void *async_info )
/* Reads data from the file associated with hdl and returns it in *string.
 * *string may be NULL or valid, in which case it is expanded.
 * reap returns 0 on success or an errno value on error. The value -1 indicates
 * EOF.
 * async_info is both a structure and a flag. If set, asynchronous IO shall
 * be used, otherwise blocked IO has to be used.
 * A maximum chunk of REGINA_MAX_BUFFER_SIZE bytes is read in one operation.
 * A return value of EAGAIN is set if we have to wait.
 */
{
   char buf[REGINA_MAX_BUFFER_SIZE] ;
   unsigned len, total ;
   streng *s ;
   int done ;

   if (string == NULL)
      return( 0 ) ;

   done = TSD->OS->read_exec( hdl, buf, sizeof(buf), async_info ) ;
   if (done <= 0)
   {
      if (done == 0)
         return( -1 ); /* EOF */
      else
         done = -done;
      /* FGC, FIXME: Not sure, this is the right processing. Setting RS, etc? */
      if ( done != EAGAIN )
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, strerror(done) )  ;
      return( done ) ; /* error */
   }

   if (( s = *string ) == NULL)
   {
      len = 0 ;
      s = Str_makeTSD( done ) ;
   }
   else
   {
      len = Str_len( s ) ;
      total = Str_max( s ) ;
      if (len + done > total)
      {
         s = Str_makeTSD( len + done ) ;
         s->len = len ;
         memcpy( s->value, (*string)->value, len ) ;
         Free_stringTSD( *string ) ;
      }
   }
   memcpy( s->value + len, buf, done ) ;
   s->len += done ;
   *string = s ;
   return( 0 ) ;
}

void cleanup_envirpart(const tsd_t *TSD, environpart *ep)
/* Closes the associated file handles of ep and deletes a temporary file
 * if necessary.
 */
{
   shl_tsd_t *st = (shl_tsd_t *)TSD->shl_tsd;

   if (ep->hdls[0] != -1)
   {
      TSD->OS->close_exec(ep->hdls[0], (ep->FileRedirected) ? NULL : st->AsyncInfo);
      ep->hdls[0] = -1;
   }
   if (ep->hdls[1] != -1)
   {
      TSD->OS->close_exec(ep->hdls[1], (ep->FileRedirected) ? NULL : st->AsyncInfo);
      ep->hdls[1] = -1;
   }
   if (ep->hdls[2] != -1)
   {
      TSD->OS->close_special(ep->hdls[2]);
      ep->hdls[2] = -1;
   }
   if (ep->tempname)
   {
      unlink(ep->tempname);
      FreeTSD(ep->tempname);
      ep->tempname = NULL;
   }
   if ( ep->tmp_queue != NULL )
   {
      addr_close_queue( TSD, ep->tmp_queue ) ;
      ep->tmp_queue = NULL ;
   }
   if ( ep->queue != NULL )
   {
      addr_close_queue( TSD, ep->queue ) ;
      ep->queue = NULL ;
   }
}

static void cleanup( tsd_t *TSD, environment *env )
/* Closes all open handles in env and deletes temporary files. Already closed
 * handles are marked by a value of -1.
 * -1 is set to each handle after closing.
 */
{
   shl_tsd_t *st = (shl_tsd_t *)TSD->shl_tsd;

   cleanup_envirpart(TSD, &env->input);
   cleanup_envirpart(TSD, &env->output);
   cleanup_envirpart(TSD, &env->error);

   if (st->AsyncInfo)
      TSD->OS->delete_async_info(st->AsyncInfo);
   st->AsyncInfo = NULL;
   st->IOBused = 0;
}

/*
 * CheckAndDealWithSameStems shall be used after opening all IO channels.
 * It checks whether some of the channels address the same stem. This function
 * takes care of this circumstances and prevents race conditions. In fact,
 * in may produce a copy of the input to prevent overwriting.
 */
static void CheckAndDealWithSameStems( tsd_t *TSD, environment *env )
{
   if ( ( env->output.type == STEM ) && ( env->error.type == STEM ) )
   {
      /* env->output.name and env->error.name must exist here
       *
       * We have to take special care if output and error are
       * redirected to the same stem. We neither want to overwrite
       * stem values twice nor want to read "stem.0" for every
       * stem on every access to prevent it.
       */
      if ( Str_ccmp( env->output.currname, env->error.currname ) == 0 )
      {
         env->error.SameAsOutput = 1;
         if ( env->error.maxnum == 0 )
         {
            /* error may has the REPLACE option while output has not.
             * Force a silent replace in this case.
             */
            env->output.maxnum = 0;
            env->output.currnum = 1;
         }
      }
   }

   if ( env->input.type == STEM )
   {
      /* Same procedure. To prevent overwriting variables while
       * outputting to a stem wherefrom we have to read, buffer
       * the input stem if the names do overlap.
       */

      if ( ( env->output.type == STEM ) &&
           ( Str_ccmp( env->input.currname, env->output.currname ) == 0 ) )
         env->input.SameAsOutput |= 1;

      if ( ( env->error.type == STEM ) &&
           ( Str_ccmp( env->input.currname, env->error.currname ) == 0 ) )
         env->input.SameAsOutput |= 2;

      if ( env->input.SameAsOutput )
      {
         /*
          * Fixes bug 609017
          */
         env->input.currname->len = env->input.currnamelen;
         env->input.tmp_queue =
           fill_input_queue_stem( TSD, env->input.currname, env->input.maxnum);
      }
   }
}

/*
 * CheckSameStreams shall be used before opening all IO channels.
 * It checks whether some of the channels address the same file. This function
 * takes detects it and prepares the system to reduce file opening.
 */
static void CheckSameStreams( tsd_t *TSD, int io_flags, environment *env )
{
   environpart *e;
   int i, isFile[3], mask;
   const streng *name[3];
   streng *full[3];

   memset( isFile, 0, sizeof( isFile ) );
   memset( (void *)name, 0, sizeof( name ) );

   for ( i = 0; i < 3; i++ )
   {
      switch ( i )
      {
         case 0:
            mask = REDIR_INPUT;
            e = &env->input;
            break;

         case 1:
            mask = REDIR_OUTLIFO | REDIR_OUTFIFO | REDIR_OUTSTRING;
            e = &env->output;
            break;

         default:
            mask = 0;
            e = &env->error;
            break;

      }

      e->SameAsOutput = 0;
      full[i] = NULL;
      if ( ( io_flags & mask ) ||
           ( e->flags.awt != awtSTREAM ) )
         continue;

      name[i] = e->name;
      if ( ( name[i] != NULL ) && ( name[i]->len == 0 ) )
         name[i] = NULL;

      if ( ( e->flags.ant == antSIMSYMBOL ) && ( name[i] != NULL ) )
      {
         name[i] = getvalue( TSD, name[i], -1 ) ;
      }
      else
      {
         assert( ( e->flags.ant == antSTRING ) || ( name[i] == NULL ) );
      }

      isFile[i] = 1;
      /*
       * Delay the computation of the fully qualified filename until we
       * know that it is useful.
       */
   }
   if ( isFile[0] + isFile[1] + isFile[2] > 1 )
   {
      for ( i = 0; i < 3; i++ )
      {
         full[i] = addr_file_info( TSD, name[i], i );
      }
   }
   if ( ( full[0] != NULL ) && ( full[1] != NULL ) )
      env->input.SameAsOutput |= filename_cmp( full[0], full[1] ) ? 0 : 1;
   if ( ( full[0] != NULL ) && ( full[2] != NULL ) )
      env->input.SameAsOutput |= filename_cmp( full[0], full[2] ) ? 0 : 2;
   if ( ( full[1] != NULL ) && ( full[2] != NULL ) )
      env->error.SameAsOutput |= filename_cmp( full[1], full[2] ) ? 0 : 1;

   if ( env->error.SameAsOutput && ( env->error.flags.append == 0 ) )
      env->output.flags.append = 0;
   for ( i = 0; i < 3; i++ )
   {
      if ( full[i] )
      {
         Free_stringTSD( full[i] );
      }
   }
}

/*
 * CheckAndDealWithSameQueues shall be used after opening all IO channels.
 * It checks whether some of the channels address the same queue. This function
 * takes care of this circumstances and prevents race conditions. In fact,
 * in may produce a copy of the input to prevent overwriting.
 */
static void CheckAndDealWithSameQueues( tsd_t *TSD, environment *env )
{
   if ( ( env->output.type & ( LIFO | FIFO | LIFOappend | FIFOappend ) )
     && ( env->error.type  & ( LIFO | FIFO | LIFOappend | FIFOappend ) ) )
   {
      /*
       * We have to take special care if output and error are
       * redirected to the same queue. We neither want to push
       * values twice nor want to read our own output for every
       * stem on every access to prevent it.
       */
      if ( addr_same_queue( TSD, env->output.queue, env->error.queue ) )
      {
         env->error.SameAsOutput = 1;
         if ( env->error.type  & ( LIFO | FIFO ) )
         {
            /* error may has the REPLACE option while output has not.
             * Force a silent replace in this case.
             */
            if ( env->output.type == LIFOappend )
               env->output.type = LIFO;
            if ( env->output.type == FIFOappend )
               env->output.type = FIFO;
         }
      }
   }

   if ( env->input.type == QUEUE )
   {
      /* Same procedure. To prevent overwriting values while
       * outputting to a queue wherefrom we have to read, buffer
       * the input queue if the queue are same.
       */

      if ( get_options_flag( TSD->currlevel, EXT_FLUSHSTACK ) == 0 )
      {
         if ( ( env->output.type & ( LIFO | FIFO | LIFOappend | FIFOappend ) )
           && addr_same_queue( TSD, env->input.queue, env->output.queue ) )
            env->input.SameAsOutput |= 1;

         if ( ( env->error.type & ( LIFO | FIFO | LIFOappend | FIFOappend ) )
           && addr_same_queue( TSD, env->input.queue, env->error.queue ) )
            env->input.SameAsOutput |= 2;

         if (env->input.SameAsOutput)
            env->input.tmp_queue = addr_redir_queue( TSD, env->input.queue );
      }
   }

   /* Final stages for queues: if not "append", do a replace by purging */
   if ( ( env->output.type == FIFO )
     || ( env->output.type == LIFO ) )
      addr_purge_queue( TSD, env->output.queue );
   if ( ( ( env->error.type == FIFO )
       || ( env->error.type == LIFO ) )
     && !env->error.SameAsOutput )
      addr_purge_queue( TSD, env->error.queue );
   /* reduce used names */
   if ( env->output.type == FIFOappend )
      env->output.type = FIFO;
   if ( env->output.type == LIFOappend )
      env->output.type = LIFO;
   if ( env->error.type == FIFOappend )
      env->error.type = FIFO;
   if ( env->error.type == LIFOappend )
      env->error.type = LIFO;
}

static int setup_io( tsd_t *TSD, int io_flags, environment *env )
/* Sets up the IO-redirections based on the values in io_flags and env.
 * Each environpart (env->input, env->output, env->error) is set up as follows:
 * a) The enviroment-based streams and stems are set up if used or not.
 *    env->input.type (or output or error) is set to STREAM, STEM or STD_IO.
 * b) The io_flags overwrite the different settings and may have
 *    values QUEUE, simLIFO, simFIFO, STRING.
 * c) If a redirection takes place (type != STD_IO) a pipe() or temporary
 *    file is opened and used.
 * This function returns 1 on success, 0 on error, in which case an error is
 * already reported..
 */
{
   shl_tsd_t *st = (shl_tsd_t *)TSD->shl_tsd;
   int overwrite;

   cleanup( TSD, env ); /* Useful in case of an undetected previous error */

   prepare_env_io( &env->input );
   prepare_env_io( &env->output );
   prepare_env_io( &env->error );

   CheckSameStreams( TSD, io_flags, env );
   /*
    * Determine which ANSI redirections are in effect
    * Use the special io_flags for redirection to overwrite the standard
    * rules of the environment.
    */
   overwrite = ( io_flags & REDIR_INPUT ) ? awtFIFO : awtUNKNOWN;
   open_env_io( TSD, &env->input, overwrite, 0 );
   if ( env->input.SameAsOutput )
   {
      /*
       * It must be a file since we don't have checked for stems and queues.
       * We read the input into a temporary queue, then we can proceed in the
       * usual way.
       */
      env->input.tmp_queue = fill_input_queue_stream( TSD, env->input.file );
      addr_reset_file( TSD, env->input.file );
      env->input.file = NULL;
   }

   if ( io_flags & REDIR_OUTLIFO )
      overwrite = awtLIFO;
   else if ( io_flags & REDIR_OUTFIFO )
      overwrite = awtFIFO;
   else if ( io_flags & REDIR_OUTSTRING )
      overwrite = awtFIFO;
   else
      overwrite = awtUNKNOWN;
   open_env_io( TSD, &env->output, overwrite, io_flags & REDIR_OUTSTRING );

   if ( env->error.SameAsOutput )
   {
      /*
       * It must be a file since we don't have checked for stems and queues.
       * We read the input into a temporary queue or a temporary file.
       * Then we can proceed in the usual way.
       */
      env->error.type = STREAM;
   }
   else
      open_env_io( TSD, &env->error, awtUNKNOWN, 0 );

   CheckAndDealWithSameStems( TSD, env );
   CheckAndDealWithSameQueues( TSD, env );

   if ( env->input.type != STD_IO )
   {
      if ( TSD->OS->open_subprocess_connection( TSD, &env->input ) != 0 )
      {
         cleanup( TSD, env );
         exiterror( ERR_SYSTEM_FAILURE, 920, "creating redirection", "for input", strerror(errno) );
         return 0;
      }
   }
   if ( env->output.type != STD_IO )
   {
      if ( TSD->OS->open_subprocess_connection( TSD, &env->output ) != 0 )
      {
         cleanup( TSD, env );
         exiterror( ERR_SYSTEM_FAILURE, 920, "creating redirection", "for output", strerror(errno) );
         return 0;
      }
   }
   else
      fflush( stdout );
   if ( env->error.type != STD_IO )
   {
      if ( TSD->OS->open_subprocess_connection( TSD, &env->error ) != 0 )
      {
         cleanup( TSD, env );
         exiterror( ERR_SYSTEM_FAILURE, 920, "creating redirection", "for error", strerror(errno) );
         return 0;
      }
   }
   else
      fflush( stderr );
   st->AsyncInfo = TSD->OS->create_async_info( TSD );
   return 1;
}

static streng *fetch_food( tsd_t *TSD, environment *env )
/* returns one streng fetched either from a queue (env->input.type == QUEUE) or
 * from a stem or stream.
 * Returns NULL if there is no more input to feed the child process.
 */
{
   const streng *c ;
   streng *retval ;
   int delflag = 0 ;

   switch (env->input.type)
   {
      case QUEUE:
         delflag = 1 ;
         if ( env->input.tmp_queue )
            c = addr_io_queue( TSD, env->input.tmp_queue, NULL, 0 ) ;
         else
            c = addr_io_queue( TSD, env->input.queue, NULL, 0 ) ;
         if ( c == NULL )
            return NULL ;
         break;

      case STREAM:
         delflag = 1;

         if ( env->input.tmp_queue )
         {
            c = addr_io_queue( TSD, env->input.tmp_queue, NULL, 0 );
            if ( c == NULL )
               return NULL;
            break;
         }
         if (env->input.file == NULL)
            return NULL;
         c = addr_io_file( TSD, env->input.file, NULL );
         if ( !c )
            return NULL;
         if ( c->len == 0 )
         {
            Free_stringTSD( (streng *) c );
            return NULL;
         }
         break;

      case STEM:
         if (!env->input.SameAsOutput)
         {
            if (env->input.currnum > env->input.maxnum)
               return( NULL ) ;
            c = stem_access( TSD, &env->input, env->input.currnum++, NULL ) ;
         }
         else
         {
            delflag = 1 ;
            c = addr_io_queue( TSD, env->input.tmp_queue, NULL, 0 ) ;
         }
         if (!c)
            return( NULL ) ;
         break;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "Illegal feeder in fetch_food()" )  ;
         return( NULL ) ;
         break ;
   }

   if ( env->input.type != STREAM )
   {
      if ( env->input.flags.noeol )
      {
         /* ADDRESS SYSTEM ... WITH INPUT NOEOL ... */
         assert( delflag ) ;
         retval = (streng *) c ; /* will be destroyed */
      }
      else
      {
         /* Append a newline to the end of the line before returning */
#if defined(DOS) || defined(OS2) || defined(WIN32)
         retval = Str_makeTSD( c->len + 2 ) ;
         memcpy(retval->value, c->value, c->len);
         retval->value[c->len] = REGINA_CR;
         retval->value[c->len + 1] = REGINA_EOL;
         retval->len = c->len + 2;
#else
         retval = Str_makeTSD( c->len + 1 ) ;
         memcpy(retval->value, c->value, c->len);
         retval->value[c->len] = REGINA_EOL;
         retval->len = c->len + 1;
#endif
         if (delflag)
            Free_stringTSD( (streng *) c ) ;
      }
   }
   else
   {
      /* STREAM mode reads blocks of buffers, usually 4KB ignoring any
       * line structure
       */
      assert( delflag ) ;
      retval = (streng *) c ; /* will be destroyed */
   }
   return( retval ) ;
}

static void drop_crop_line( tsd_t *TSD, environment *env, const char *data,
                                                unsigned length, int is_error )
/* Called while reading the output of the child. The output is in data and
 * contains length bytes without the line terminator.
 * which may be empty or not yet completed. The exact destination is determined
 * by env->x.type, where x is either output or error depending on is_error.
 * type may have one of the values simLIFO, simFIFO, STRING, STREAM or STEM.
 * is_error is set if the error redirection should happen.
 */
{
   streng *string ;
   int type;

   string = Str_makeTSD( length + 1 ) ; /* We need a terminating 0 in some */
                                        /* cases                           */
   memcpy( string->value, data, length ) ;
   string->len = length ;
   string->value[length] = '\0' ;

   if (is_error)
      type = env->error.type;
   else
      type = env->output.type;

   switch (type)
   {
      case LIFO:
         if ( is_error && !env->error.SameAsOutput )
         {
            if ( env->error.tmp_queue != NULL )
               addr_io_queue( TSD, env->error.tmp_queue, string, 0 ) ;
            else
               addr_io_queue( TSD, env->error.queue, string, 0 ) ;
         }
         else
         {
            if ( env->output.tmp_queue != NULL )
               addr_io_queue( TSD, env->output.tmp_queue, string, 0 ) ;
            else
               addr_io_queue( TSD, env->output.queue, string, 0 ) ;
         }
         return;  /* consumes the new string */

      case FIFO:
      case STRING:
         if ( is_error && !env->error.SameAsOutput )
         {
            if ( env->error.tmp_queue != NULL )
               addr_io_queue( TSD, env->error.tmp_queue, string, 1 ) ;
            else
               addr_io_queue( TSD, env->error.queue, string, 1 ) ;
         }
         else
         {
            if ( env->output.tmp_queue != NULL )
               addr_io_queue( TSD, env->output.tmp_queue, string, 1 ) ;
            else
               addr_io_queue( TSD, env->output.queue, string, 1 ) ;
         }
         return;  /* consumes the new string */

      case STREAM:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "Illegal STREAM in drop_crop_line()" )  ;
         break;

      case STEM:
         if (is_error && !env->error.SameAsOutput)
            put_stem( TSD, &env->error, string ) ;
         else
            put_stem( TSD, &env->output, string ) ;
         return;  /* consumes the new string */

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "Illegal crop in drop_crop_line()" )  ;
         break ;
   }

   Free_stringTSD( string ) ;
}

/* line_length tries to find ANY line terminator. This is either \r,
 * \n, \r\n or \n\r.
 * The lookup happens in line (not 0-terminated) with a length of len.
 * The return value is either -1 (not found) or the length of the line
 * content. *termlen is set to the number of characters of the line
 * terminator, either 1 or 2.
 * EOFreached is boolean and indicated a final line if set.
 */
static int line_length(const char *line, int len, int *termlen, int EOFreached)
{
   char *ccr;
   char *clf;
   int h;

   ccr = (char *)memchr( line, '\r', len ) ;
   clf = (char *)memchr( line, '\n', len ) ;
   if ( ccr != NULL )
   {
      if ( clf != NULL )
      {
         if ( clf < ccr )
         {
            *termlen = ( ccr == clf + 1 ) ? 2 : 1 ;
            return (int) ( clf - line ) ;
         }
         else
         {
            *termlen = ( clf == ccr + 1 ) ? 2 : 1 ;
            return (int) ( ccr - line ) ;
         }
      }
      /* else '\r' found, but we must know if it terminates */
      h = (int) ( ccr - line ) ;
      if ( ( h + 1 < len ) || EOFreached )
      {
         *termlen = 1 ;
         return h ;
      }
   }
   else if ( clf != NULL ) /* simple line feed */
   {
      h = (int) ( clf - line ) ;
      if ( ( h + 1 < len ) || EOFreached )
      {
         *termlen = 1 ;
         return h ;
      }
   }
   return -1 ;
}

static void drop_crop( tsd_t *TSD, environment *env, streng **string,
                                                  int EOFreached, int is_error)
/* Called while reading the output of the child. The output is in *string,
 * which may be empty or not yet completed. The exact destination is determined
 * by env->x.type, where x is either output or error depending on is_error.
 * type may have one of the values simLIFO, simFIFO, STRING, STREAM or STEM.
 * If EOFreached is set and some data is in *string, this data is interpreted
 * as a completed line.
 * Completed lines are cut of the string. The string itself isn't deleted.
 * is_error is set if the error redirection should happen.
 */
{
   streng *s ;
   char *ptr ;
   void *fptr = NULL ;
   int max, found, termlen, isStream ;


   s = *string;
   if (s == NULL) /* might happen on a first call */
      return;

   if (is_error)
   {
      isStream = ( env->error.type == STREAM ) ;
      fptr = ( env->error.SameAsOutput ) ? env->output.file : env->error.file;
   }
   else
   {
      isStream = ( env->output.type == STREAM ) ;
      fptr = env->output.file ;
   }

   if ( isStream )
   {
      /* very fast bypass, we can simply drop the whole thing */
      if ( fptr )
         addr_io_file( TSD, fptr, s ) ;

      s->len = 0 ;
   }
   else
   {
      ptr = s->value ;
      max = Str_len( s ) ;

      while ( max > 0 )
      {
         found = line_length( ptr, max, &termlen, EOFreached ) ;

         if ( ( found == -1 ) && EOFreached )
         {
            found = max ;
            termlen = 0 ;
         }
         if ( found < 0 )
            break;

         drop_crop_line( TSD, env, ptr, (unsigned) found, is_error ) ;
         found += termlen ;
         max -= found ;
         ptr += found ;
      }
      if (max > 0)
      {
         if (ptr == s->value)
         {
            if (max != Str_len(s))
            {
               exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "Illegal memory access");
            }
         }
         else
            memmove( s->value, ptr, max ) ;
      }
      s->len = max ;
   }
   *string = s ;
}

int posix_do_command( tsd_t *TSD, const streng *command, int io_flags, environment *env, Queue *redir )
{
   int child, rc ;
   int in, out, err;
   streng *istring = NULL, *ostring = NULL, *estring = NULL ;
   char *cmdline ;
   shl_tsd_t *st = (shl_tsd_t *)TSD->shl_tsd;
#ifdef SIGPIPE
   signal_handler prev_sig;
#endif

   fflush( stdout ) ;
   fflush( stderr ) ;

   CloseOpenFiles( TSD, fpdRETAIN );

   /*
    * Fixes bug 615822
    */
   if (!setup_io(TSD, io_flags, env))
      exiterror( ERR_SYSTEM_FAILURE, 0 )  ;

   if (env->input.FileRedirected)
   {
      /* fill up the input file without closing the stream. */

      while ((istring = fetch_food(TSD, env)) != NULL)
      {
         if (feed(TSD, &istring, env->input.hdls[1], NULL) != 0)
            break; /* shall not happen! */
      }
      rc = write_buffered(TSD, env->input.hdls[1], NULL, 0, NULL);
      if (rc < 0)
      {
         errno = -rc;
         exiterror( ERR_SYSTEM_FAILURE, 920, "feeding redirection file", "for input", strerror(errno) );
      }
      /* seek positions of both fdin may have been destroyed */
      TSD->OS->restart_file(env->input.hdls[0]);
      TSD->OS->close_exec(env->input.hdls[1], NULL);
      env->input.hdls[1] = -1;
   }

   if ( env->output.type == STRING )
      env->output.tmp_queue = redir ;

   cmdline = str_ofTSD( command ) ;
   child = TSD->OS->fork_exec( TSD, env, cmdline, &rc );
   FreeTSD( cmdline ) ;
   if ( ( child == -1 ) || ( child == 0 ) )
   {
      err = errno;
      cleanup( TSD, env ) ;
      if ( child == -1 )
         exiterror( ERR_SYSTEM_FAILURE, 1, strerror( err ) );
      return ( rc > 0 ) ? -rc : ( rc == 0 ) ? -1000 : rc;
   }

   /* Close the child part of the handles */
   if (env->input.hdls[0]  != -1) TSD->OS->close_exec(env->input.hdls[0], NULL) ;
   if (env->output.hdls[1] != -1) TSD->OS->close_exec(env->output.hdls[1], NULL) ;
   if (env->error.hdls[1]  != -1) TSD->OS->close_exec(env->error.hdls[1], NULL) ;
   env->input.hdls[0] = env->output.hdls[1] = env->error.hdls[1] = -1;

   /* Force our own handles to become nonblocked */
   if (!env->input.FileRedirected && ((in = env->input.hdls[1]) != -1))
   {
      TSD->OS->unblock_handle( &in, st->AsyncInfo ) ;
   }
   else
      in = -1;
   if (!env->output.FileRedirected && ((out = env->output.hdls[0]) != -1))
   {
      TSD->OS->unblock_handle( &out, st->AsyncInfo ) ;
   }
   else
      out = -1;
   if (!env->error.FileRedirected && ((err = env->error.hdls[0]) != -1))
   {
      TSD->OS->unblock_handle( &err, st->AsyncInfo ) ;
   }
   else
      err = -1;

#ifdef SIGPIPE
    prev_sig = regina_signal( SIGPIPE, SIG_IGN ) ;
#endif

   while ((in != -1) || (out != -1) || (err != -1))
   {
      TSD->OS->reset_async_info(st->AsyncInfo);
      if (in != -1)
      {
         do {
            if (!istring)
               istring = fetch_food( TSD, env ) ;
            if (!istring)
            {
               rc = write_buffered(TSD, in, NULL, 0, st->AsyncInfo);
               if (rc == -EAGAIN)
                  TSD->OS->add_async_waiter(st->AsyncInfo, in, 0);
               else
               {
                  if ((rc < 0) && (-rc != EPIPE)) /* fixes bug 945218 */
                  {
                     errno = -rc;
                     exiterror( ERR_SYSTEM_FAILURE, 920, "writing to", "input redirection", strerror(errno) );
                  }
                  if (TSD->OS->close_exec(in, st->AsyncInfo))
                     exiterror( ERR_SYSTEM_FAILURE, 920, "closing redirection", "for input", strerror(errno) );
                  env->input.hdls[1] = in = -1 ;
                  rc = -1 ; /* indicate a closed stream */
               }
            }
            else  /* nothing left in string, but more in the stack */
            {
               rc = feed( TSD, &istring, in, st->AsyncInfo ) ;
               if (rc)
               {
                  if (rc == EAGAIN)
                     TSD->OS->add_async_waiter(st->AsyncInfo, in, 0);
                  else
                  {
                     TSD->OS->close_exec(in, st->AsyncInfo) ;
                     env->input.hdls[1] = in = -1 ;
                  }
               }
               else if (istring != NULL)
               {
                  /* hasn't written all at once, therefore is blocked.
                   * do a little performance boost and don't try to write
                   * once more, perform the wait instead.
                   */
                  rc = -1;
                  TSD->OS->add_async_waiter(st->AsyncInfo, in, 0);
               }
            }
         } while (rc == 0); /* It is best for performance and no penalty for */
                            /* security to write as much as possible         */

      } /* if (in != -1) */

      if (out != -1)
      {
         do {
            rc = reap( TSD, &ostring, out, st->AsyncInfo );
            if (rc)
            {
               if (rc == EAGAIN)
                  TSD->OS->add_async_waiter(st->AsyncInfo, out, 1);
               else
               {
                  TSD->OS->close_exec(out, st->AsyncInfo) ;
                  env->output.hdls[0] = out = -1 ;
               }
            }
            else if (ostring != NULL)
               drop_crop( TSD, env, &ostring, 0, 0 ) ;
         } while (rc == 0); /* It is best for performance and no penalty for */
                            /* security to write as much as possible         */
      } /* if (out != -1) */

      if (err != -1)
      {
         do {
            rc = reap( TSD, &estring, err, st->AsyncInfo );
            if (rc)
            {
               if (rc == EAGAIN)
                  TSD->OS->add_async_waiter(st->AsyncInfo, err, 1);
               else
               {
                  TSD->OS->close_exec(err, st->AsyncInfo) ;
                  env->error.hdls[0] = err = -1 ;
               }
            }
            else if (estring != NULL)
               drop_crop( TSD, env, &estring, 0, 1 ) ;
         } while (rc == 0); /* It is best for performance and no penalty for */
                            /* security to write as much as possible         */
      } /* if (err != -1) */

      TSD->OS->wait_async_info(st->AsyncInfo); /* wait for any more IO */
   } /* end of IO */

   if (istring)
      Free_stringTSD( istring );

   if (ostring)
   {
      if ( Str_len( ostring ) )
         drop_crop( TSD, env, &ostring, 1, 0 );
      Free_stringTSD( ostring );
   }

   if (estring)
   {
      if ( Str_len( estring ) )
         drop_crop( TSD, env, &estring, 1, 1 );
      Free_stringTSD( estring );
   }

   if ( ( env->input.type == QUEUE ) && ( env->input.tmp_queue == NULL ) )
      addr_purge_queue( TSD, env->input.queue ) ;

   rc = TSD->OS->wait_exec(child);

#ifdef SIGPIPE
   regina_signal( SIGPIPE, prev_sig ) ;
//   regina_signal( SIGPIPE, SIG_DFL ) ;
#endif

   if (env->output.FileRedirected)
   {
      /* The file position is usually at the end: */
      TSD->OS->restart_file(env->output.hdls[0]);
      while (reap( TSD, &ostring, env->output.hdls[0], NULL ) == 0)
      {
         if (ostring != NULL)
            drop_crop( TSD, env, &ostring, 0, 0 ) ;
      }
      if (ostring != NULL)
         drop_crop( TSD, env, &ostring, 1, 0 ) ;

      /* use the automatted closing feature of cleanup */
   }
   if (env->error.FileRedirected)
   {
      /* The file position is usually at the end: */
      TSD->OS->restart_file(env->error.hdls[0]);
      while (reap( TSD, &estring, env->error.hdls[0], NULL ) == 0) {
         if (estring != NULL)
            drop_crop( TSD, env, &estring, 0, 1 ) ;
      }
      if (estring != NULL)
         drop_crop( TSD, env, &estring, 1, 1 ) ;
      /* use the automatted closing feature of cleanup */
   }

   if ( env->output.type & ( LIFO | FIFO ) ) /* never use STRING here */
      flush_stack( TSD, env->output.tmp_queue, env->output.queue, env->output.type == FIFO ) ;
   if ( env->error.type & ( LIFO | FIFO ) )
      flush_stack( TSD, env->error.tmp_queue, env->error.queue, env->output.type == FIFO ) ;

   if ( ( env->input.type == STREAM ) && ( env->input.file != NULL ) )
      addr_reset_file( TSD, env->input.file );
   if ( env->output.type == STREAM )
      addr_reset_file( TSD, env->output.file );
   if ( ( env->error.type == STREAM ) && !env->error.SameAsOutput )
      addr_reset_file( TSD, env->error.file );
   if ( env->output.type == STRING )
      env->output.tmp_queue = NULL ;

   cleanup( TSD, env ) ;

   return rc ;
}
