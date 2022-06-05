/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1993-1994  Anders Christensen <anders@pvv.unit.no>
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

#include "rexx.h"
#include <string.h>

#if defined(VMS)
# define fork() vfork()
/*
 *  At least with OpenVMS 7.3-1 on Alpha, the Posix way seems to work.
 *  So there is no need to redirect on (now) bogus code.
 */

#ifdef VMS_DO_COMMAND
# ifdef posix_do_command
#  undef posix_do_command
# endif
# define posix_do_command __regina_vms_do_command
#endif
#endif

/*
 * The following strings must match AddressWithType enums in regina_t.h
 */
static char *env_type[] = { "NORMAL", "STREAM", "STEM", "LIFO", "FIFO" };

struct envir
{
   environment e ;
   int type ;
   struct envir *next, *prev ;
};

/* clear_environpart sets initial values of an environpart.
 * attention: cleanup_environpart() closes open file handles and removes
 *            temporary files, this function not.
 */
static void clear_environpart(environpart *ep)
{
   ep->currnum = -1 ;
   ep->maxnum = -1 ;
   ep->SameAsOutput = 0;
   ep->FileRedirected = 0;
   ep->tempname = NULL ;
   ep->tmp_queue = NULL ;
   ep->queue = NULL ;
   ep->type = 0 ;
   ep->hdls[2] = ep->hdls[1] = ep->hdls[0] = -1;
}

void add_envir( tsd_t *TSD, const streng *name, int type, int subtype )
/* an environment with the same name can be added more than once! */
{
   struct envir *ptr;

   ptr = (struct envir *)MallocTSD( sizeof( struct envir ) );
   memset( &ptr->e, 0, sizeof(ptr->e) );
   clear_environpart( &ptr->e.input );
   clear_environpart( &ptr->e.output );
   clear_environpart( &ptr->e.error );
   ptr->e.input.flags.isinput = 1;
   ptr->e.error.flags.iserror = 1;

   ptr->e.name = Str_dupTSD( name );
   ptr->e.subtype = subtype;
   ptr->e.subcomed = 0;
   ptr->type = type;
   ptr->prev = (struct envir *)TSD->firstenvir;
   ptr->next = NULL;
   TSD->firstenvir = ptr;
   if ( ptr->prev )
      ptr->prev->next = ptr;
}

#ifdef TRACEMEM
static void markenvir( const tsd_t *TSD )
{
   struct envir *eptr=NULL ;

   eptr = (struct envir *) TSD->firstenvir ;
   for (; eptr; eptr=eptr->prev )
   {
      markmemory( eptr, TRC_ENVIRBOX ) ;
      markmemory( eptr->e.name, TRC_ENVIRNAME ) ;
      if (eptr->e.input.name)
         markmemory( eptr->e.input.name, TRC_ENVIRNAME ) ;
      if (eptr->e.output.name)
         markmemory( eptr->e.output.name, TRC_ENVIRNAME ) ;
      if (eptr->e.error.name)
         markmemory( eptr->e.error.name, TRC_ENVIRNAME ) ;
   }
}
#endif /* TRACEMEM */


static struct envir *find_envir( const tsd_t *TSD, const streng *name )
{
   struct envir *ptr=NULL ;

   for ( ptr = (struct envir *)TSD->firstenvir; ptr; ptr = ptr->prev )
   {
      if ( !Str_cmp( ptr->e.name, name ) )
         return ptr ;
   }
   return NULL ;
}

/*
 * This function is used to determine if an environment exists. It is the
 * external equivalent of find_envir()
 */
int envir_exists( const tsd_t *TSD, const streng *name )
{
   if ( find_envir( TSD, name ) == NULL )
      return 0;
   else
      return 1;
}

/*
 * This function returns textual information about the current environment.
 * Suitable output for ADDRESS BIF
 */
streng *get_envir_details( const tsd_t *TSD, char opt, const streng *name )
{
   struct envir *ptr=NULL ;
   streng *result, *env_resource, *raw_resource;
   char *env_position;
   int awt, ant, len;

   ptr = find_envir( TSD, name );
   switch( opt )
   {
      case 'I':
         env_position = (char *)"INPUT";
         awt = ptr->e.input.flags.awt;
         ant = ptr->e.input.flags.ant;
         raw_resource = ptr->e.input.name;
         break;
      case 'O':
         env_position = (char *)( (ptr->e.output.flags.append) ? "APPEND" : "REPLACE" );
         awt = ptr->e.output.flags.awt;
         ant = ptr->e.output.flags.ant;
         raw_resource = ptr->e.output.name;
         break;
      case 'E':
         env_position = (char *)( (ptr->e.error.flags.append) ? "APPEND" : "REPLACE" );
         awt = ptr->e.error.flags.awt;
         ant = ptr->e.error.flags.ant;
         raw_resource = ptr->e.error.name;
         break;
      default:
         /*
          * Should not get here!!
          */
         awt = ant = 0;
         env_position = NULL;
         raw_resource = NULL;
         break;
   }
   /*
    * Determine the resource string...
    */
   if ( raw_resource == NULL )
   {
      env_resource = nullstringptr();
   }
   else if ( awt == awtSTEM )
   {
      env_resource = raw_resource;
   }
   else if ( ant == antSTRING )
   {
      if ( raw_resource == NULL )
      {
         env_resource = nullstringptr();
      }
      else
      {
         env_resource = raw_resource;
      }
   }
   else
   {
      /*
       * Get the value of the symbol...
       */
      env_resource = (streng *)getdirvalue( (tsd_t *)TSD, raw_resource );
   }
   len = 3 + strlen( env_position) + strlen( env_type[awt] ) + env_resource->len;
   result = Str_makeTSD( len );
   result = Str_catstrTSD( result, env_position );
   result = Str_catstrTSD( result, " " );
   result = Str_catstrTSD( result, env_type[awt] );
   if ( env_resource->len )
   {
      result = Str_catstrTSD( result, " " );
      result = Str_catTSD( result, env_resource );
   }
   return result;
}

/*
 * This function is used to set the subcomed flag in an environment exists.
 */
int set_subcomed_envir( const tsd_t *TSD, const streng *name, int subcomed )
{
   struct envir *ptr=NULL ;
   if ( ( ptr = find_envir( TSD, name ) ) == NULL )
      return 0;
   ptr->e.subcomed = subcomed;
   return 1;
}
/*
 * This function is used to get the subcomed flag from an environment.
 */
int get_subcomed_envir( const tsd_t *TSD, const streng *name )
{
   struct envir *ptr=NULL ;
   if ( ( ptr = find_envir( TSD, name ) ) == NULL )
      return 0;
   return ptr->e.subcomed;
}

static void del_envirpart( const tsd_t *TSD, environpart *e )
/* Deletes all allocated parts of the environpart e.
 * The names are set to NULL afterwards, thus allowing multiple calls.
 */
{
   if ( e->name )
      Free_stringTSD( e->name ) ;
   if ( e->base )
      Free_stringTSD( e->base ) ;
   if ( e->currname )
      Free_stringTSD( e->currname ) ;
   e->name =
   e->base =
   e->currname = NULL;
   /* Temporary files must be deleted under all circumstances! */
   cleanup_envirpart(TSD, e);
}

void del_envir( tsd_t *TSD, const streng *name )
/* Deletes all allocated parts of the environment with the given name and
 * removes it from the linked list.
 */
{
   struct envir *ptr=NULL ;

   ptr = find_envir( TSD, name ) ;
   if (!ptr)
      return;

   if (ptr->prev)
      ptr->prev->next = ptr->next ;
   if (ptr->next)
      ptr->next->prev = ptr->prev ;
   if (TSD->firstenvir==ptr)
      TSD->firstenvir = ptr->prev ;

   /* Delete the names in the environment */
   if ( ptr->e.name )
      Free_stringTSD( ptr->e.name ) ;
   del_envirpart( TSD, &ptr->e.input ) ;
   del_envirpart( TSD, &ptr->e.output ) ;
   del_envirpart( TSD, &ptr->e.error ) ;
   FreeTSD( ptr ) ;
}

static void update_environpart( const tsd_t *TSD, environpart *e, const nodeptr newptr)
/* e is the environpart which has to be reset. new is the new part and has
 * to been valid. new->name may be NULL (for NORMAL behaviour).
 */
{
   del_envirpart( TSD, e ) ;

   if (newptr->name)
   {
      e->name = Str_dupTSD( newptr->name ) ;
      e->base = Str_makeTSD( 3*sizeof(int) ) ;
   }

   e->flags = newptr->u.of ;
   clear_environpart(e);
}

int set_envir( const tsd_t *TSD, const streng *envirname, const nodeptr ios )
/* This function sets all three IO-redirections INPUT, OUTPUT and ERROR.
 * ios->p[0] must be the redirection of INPUT,
 * ios->p[1] must be the redirection of OUTPUT,
 * ios->p[2] must be the redirection of ERROR
 * ios==NULL || envirname==NULL --> don't do anything.
 * ios->p[x]==NULL --> don't touch this redirection.
 * ios->p[x].name may be NULL (for NORMAL behaviour).
 *
 * The types of all four nodes may or may not be X_ADDR_WITH.
 *
 * returns 1 on success, 0 if the envirname doesn't exist.
 */
{
   struct envir *e;

   if ( ( envirname == NULL ) || ( ios == NULL ) )
      return 1;

   if ( ( e = find_envir( TSD, envirname ) ) == NULL )
      return 0;
   if (ios->p[0]) update_environpart( TSD, &e->e.input,  ios->p[0] );
   if (ios->p[1]) update_environpart( TSD, &e->e.output, ios->p[1] );
   if (ios->p[2]) update_environpart( TSD, &e->e.error,  ios->p[2] );
   e->e.input.flags.isinput = 1;
   e->e.error.flags.iserror = 1;

   return 1;
}

static void dup_environpart( const tsd_t *TSD, environpart *dest,
                                 const nodeptr prefer, const environpart *src )
/* Copies src to dest if prefer isn't set. In this case prefer is used.
 */
{
   if (prefer)
      update_environpart( TSD, dest, prefer ) ;
   else
   {
      if (src->name)
      {
         dest->name = Str_dupTSD( src->name ) ;
         dest->base = Str_makeTSD( 3*sizeof(int) ) ;
      }
      dest->flags = src->flags ;
   }
   clear_environpart(dest);
}

static struct envir *dup_envir( tsd_t *TSD, const streng *name, cnodeptr ios )
/* This functions returns a new instance of the environment with the given
 * name or NULL if there is not such an environment name.
 * The current IO-redirection settings are overwritten with the
 * environment parts given in ios->p[0..2] for INPUT, OUTPUT and ERROR.
 */
{
   struct envir *prev, *newptr;

   if ( ( prev = find_envir( TSD, name ) ) == NULL )
      return NULL;

   add_envir( TSD, name, prev->type, prev->e.subtype );
   newptr = (struct envir *)TSD->firstenvir;

   dup_environpart( TSD, &newptr->e.input,  ios->p[0], &prev->e.input  );
   dup_environpart( TSD, &newptr->e.output, ios->p[1], &prev->e.output );
   dup_environpart( TSD, &newptr->e.error,  ios->p[2], &prev->e.error  );
   newptr->e.input.flags.isinput = 1;
   newptr->e.error.flags.iserror = 1;

   return newptr;
}

int init_envir( tsd_t *TSD )
{
   static const struct {
      const char *name ;
      int         subtype ;
   } locals[] = {
      { "COMMAND",        SUBENVIR_PATH    } , /* was SUBENVIR_COMMAND */
      { "SYSTEM",         SUBENVIR_SYSTEM  } ,
      { "OS2ENVIRONMENT", SUBENVIR_SYSTEM  } ,
      { "ENVIRONMENT",    SUBENVIR_SYSTEM  } ,
      { "CMD",            SUBENVIR_PATH    } , /* was SUBENVIR_COMMAND */
      { "PATH",           SUBENVIR_PATH    } ,
      { "REGINA",         SUBENVIR_REXX    } ,
      { "REXX",           SUBENVIR_REXX    }
   };
   unsigned i;
   streng *tmp;

   for ( i = 0; i < sizeof(locals) / sizeof(locals[0]); i++ )
   {
      tmp = Str_creTSD( locals[i].name ) ;
      add_envir( TSD, tmp, ENVIR_SHELL, locals[i].subtype ) ;
      Free_stringTSD( tmp ) ;
   }
#ifdef TRACEMEM
   regmarker( TSD, markenvir ) ;
#endif
   return(1);
}


static int get_io_flag( tsd_t *TSD, streng *command, streng **rxqueue )
{
   int length=0, i=0, pos=0 ;
   int flag=0,have_space=0 ;
   int qname_start,qname_end;
   streng *tmpq=NULL;

   flag = REDIR_NONE ;
   /*
    * All I/O redirection valid with INTERNAL queues only
    */
   if ((length=Str_len(command)) > 5
   &&  (get_options_flag( TSD->currlevel, EXT_INTERNAL_QUEUES )
      ||   external_queues_used( TSD ) == 0 ) )
   {
      if ((!memcmp(command->value,"lifo>",5)) ||
          (!memcmp(command->value,"LIFO>",5)))
               flag |= REDIR_INPUT ;

      if ((!memcmp(command->value+length-5,">lifo",5))
      ||  (!memcmp(command->value+length-5,">LIFO",5)))
      {
         flag |= REDIR_OUTLIFO ;
         command->len -= 5;
      }
      else
      {
         if ( ( !memcmp( command->value+length-5, ">fifo" ,5 ) )
         ||   ( !memcmp( command->value+length-5, ">FIFO", 5 ) ) )
         {
            flag |= REDIR_OUTFIFO ;
            command->len -= 5;
         }
         else
         {
            if ( length >= 8 )
            {

               for ( i = 0, pos = -1; i < length; i++ )
               {
                  if ( *(command->value + i ) == '|' )
                  {
                     pos = i;
                     /* don't break here, as we want the last '|' */
                  }
               }
               if ( pos != -1 )
               {
                  /* allow "|" [whitespace] "rxqueue" [whitespace[args]] */
                  /* "|" already checked */
                  for ( i = pos + 1; i < length; i++ )
                  {
                     if ( !rx_isspace(command->value[i] ) )
                        break;
                  }
                  if ( i+7 <= length )
                  {
                     if (mem_cmpic( command->value + i, "RXQUEUE", 7 ) == 0 )
                     {
                        i += 7;
                        for ( ; i < length; i++ )
                        {
                           if ( !rx_isspace( command->value[i] ) )
                              break;
                           have_space = 1;
                        }
                        if ( i == length )
                        {
                            flag |= REDIR_OUTFIFO ;
                            command->len = pos;
                        }
                        else if ( have_space )
                        {
                            if ( *(command->value + i) == '/' )
                            {
                               /*
                                * Only a switch, not a queuename
                                */
                               if ( i+6 <= length
                               &&   mem_cmpic( command->value + i, "/CLEAR", 6 ) == 0 )
                               {
                                  flag |= REDIR_CLEAR;
                                  flag |= REDIR_OUTFIFO ;
                               }
                               else if ( i+5 <= length )
                               {
                                  if (mem_cmpic( command->value + i, "/FIFO", 5 ) == 0 )
                                     flag |= REDIR_OUTFIFO ;
                                  else if (mem_cmpic( command->value + i, "/LIFO", 5 ) == 0 )
                                     flag |= REDIR_OUTLIFO ;
                                  else
                                     flag |= REDIR_OUTFIFO ;
                                  /*
                                   * Let the queue name be determined by the caller
                                   */
                               }
                            }
                            else
                            {
                               /*
                                * First word must be a queue name, optionally
                                * followed by a switch
                                */
                               have_space = 0;
                               qname_start = i;
                               for ( ; i < length; i++ )
                               {
                                  if ( rx_isspace( command->value[i] ) )
                                  {
                                     have_space = 1;
                                     qname_end = i;
                                     /*
                                      * Get queuename from RXQUEUE env
                                      * variable, or if not set, use SESSION
                                      */
                                     tmpq = Str_make_TSD( TSD, qname_end - qname_start );
                                     tmpq->len = qname_end - qname_start;
                                     memcpy( tmpq->value, command->value + qname_start, qname_end - qname_start );
                                     break;
                                  }
                               }
                               if ( have_space )
                               {
                                  /*
                                   * Eat up all spaces. If we have
                                   * any non-spaces left, it should be
                                   * a switch
                                   */
                                  for ( ; i < length; i++ )
                                  {
                                     if ( !rx_isspace( command->value[i] ) )
                                        break;
                                  }
                                  if ( i+6 <= length
                                  &&   mem_cmpic( command->value + i, "/CLEAR", 6 ) == 0 )
                                  {
                                     flag |= REDIR_CLEAR;
                                     flag |= REDIR_OUTFIFO ;
                                  }
                                  else if ( i+5 <= length )
                                  {
                                     if (mem_cmpic( command->value + i, "/FIFO", 5 ) == 0 )
                                        flag |= REDIR_OUTFIFO ;
                                     else if (mem_cmpic( command->value + i, "/LIFO", 5 ) == 0 )
                                        flag |= REDIR_OUTLIFO ;
                                     else
                                        flag |= REDIR_OUTFIFO ;
                                  }
                               }
                               else
                               {
                                  /*
                                   * Only have a queue name
                                   */
                                  tmpq = Str_make_TSD( TSD, length - qname_start );
                                  tmpq->len = length - qname_start;
                                  memcpy( tmpq->value, command->value + qname_start, length - qname_start );
                                  flag |= REDIR_OUTFIFO ;
                               }
                            }
                            command->len = pos;
                        }
                     }
                  }
               }
            }
         }
      }
   }

   if (flag & REDIR_INPUT)
   {
      for(i=5; i<Str_len(command); i++ ) /* avoid buffer overrun */
         command->value[i-5]=command->value[i] ;
      command->len -= 5 ;
   }
   *rxqueue = tmpq;
   return flag ;
}

/*
 * ANSI 8.2.4 and 8.3.5 and others forces us to set some variables showing
 * the result and to raise some conditions in case of errors after
 * processing an external command.
 */
void post_process_system_call( tsd_t *TSD, const streng *cmd,
                               int rc_code, const streng *rc_value,
                               cnodeptr thisptr )
{
   int rs;
   trap *traps;
   int type;

   /*
    * ANSI 8.2.4 etc. forces us to do the following.
    *
    * 10-08-2004 MH interpreted 8.2.4 as only being applicable to
    * clauses entered at the interactive prompt.
    */
#if 0
   if ( !TSD->systeminfo->interactive )
#endif
   {
      if ( rc_value != NULL )
         set_reserved_value( TSD, POOL0_RC, Str_dupTSD( rc_value ), 0,
                             VFLAG_STR );
      else
         set_reserved_value( TSD, POOL0_RC, NULL, rc_code, VFLAG_NUM );
   }

   /* set .RS: -1==Failure, 0=OK, 1=Error */
   if ( rc_code == 0 )
      rs = 0;
   else if ( rc_code < 0 )
      rs = -1;
   else
      rs = 1;
   set_reserved_value( TSD, POOL0_RS, NULL, rs, VFLAG_NUM );

   if ( rc_code )
   {
      traceerror( TSD, thisptr, rc_code );
      traps = gettraps( TSD, TSD->currlevel );
      type = ( rc_code > 0 ) ? SIGNAL_ERROR : SIGNAL_FAILURE;

      if ( traps[type].on_off )
         condition_hook( TSD, type, rc_code, 0, thisptr->lineno, Str_dupTSD( cmd ), NULL );
   }
}

streng *perform( tsd_t *TSD, const streng *command, const streng *envir, cnodeptr thisptr, cnodeptr overwrite )
/* If and only if overwrite is set, a new instance of the environment is
 * temporarily created and reset to the new IO-redirections.
 */
{
   int rc=0, io_flag=0, clearq=0, tempenvir=0;
   struct envir *eptr;
   streng *retstr=NULL ;
   streng *rxqueue=NULL;
   streng *cmd=Str_dupTSD(command);
   streng *saved_queue=NULL;
   char *rxq=NULL;

   if ( overwrite )
   {
      /*
       * fixes bug 653214, overwrite was this->p[1] in former versions, which
       * was wrong is this wasn't an ADDRESS statement.
       */
      if (( eptr = dup_envir( TSD, envir, overwrite ) ) != NULL)
         tempenvir = 1;
   }
   else
   {
      eptr = find_envir( TSD, envir ) ;
   }

   if (eptr)
   {
      if ( TSD->restricted
      &&   eptr->e.subtype )
         exiterror( ERR_RESTRICTED, 5 )  ;
      switch (eptr->type)
      {
         case ENVIR_PIPE:
            retstr = SubCom( TSD, cmd, envir, &rc ) ;
            break ;

         case ENVIR_SHELL:
            io_flag = get_io_flag( TSD, cmd, &rxqueue ) ;
            /*
             * Save the current queue name
             * Then change it to the value of rxqueue
             * Only for internal queues
             */
            if ( get_options_flag( TSD->currlevel, EXT_INTERNAL_QUEUES ) )
            {
               /*
                * If no queuename specified,
                * get queuename from RXQUEUE env
                * variable, or if not set, use SESSION
                */
               if ( rxqueue == NULL )
               {
                  if ( ( rxq = getenv("RXQUEUE") ) == NULL )
                  {
                     rxqueue = Str_cre_TSD( TSD, "SESSION" );
                  }
                  else
                  {
                     rxqueue = Str_cre_TSD( TSD, rxq );
                  }
               }
               saved_queue = set_queue( TSD, rxqueue );
               if ( io_flag & REDIR_CLEAR )
               {
                  clearq = 1;
                  io_flag -= REDIR_CLEAR;
               }
            }
            rc = posix_do_command( TSD, cmd, io_flag, &eptr->e, NULL ) ;
            /*
             * Change the current queue name back
             * to the saved name
             */
            if ( get_options_flag( TSD->currlevel, EXT_INTERNAL_QUEUES ) )
            {
               if ( clearq )
                  drop_buffer( TSD, 0 ) ;
               set_queue( TSD, saved_queue );
               if ( rxqueue != NULL )
                  Free_stringTSD( rxqueue );
            }
            retstr = int_to_streng( TSD, rc ) ;
            break ;

         default:
             exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
      }
   }
   else
   {
      retstr = SubCom( TSD, cmd, envir, &rc ) ;
   }

   if (tempenvir)
      del_envir( TSD, envir ) ;

   post_process_system_call( TSD, cmd, rc, retstr, thisptr );

   Free_stringTSD( cmd ) ;
   return retstr ;
}

/* run_popen is a special implementation of a direct call to posix_do_command.
 * It redirects input and error to "normal" and fetches the output.
 * This output has blanks as line-delimiters. All lines are returned
 * concatenated on success; NULL is returned if an errors occurs.
 * The global variable "RC" contains the return code of the called process.
 *
 * The command is executed in the current environment. If the environment
 * isn't a standard environment, SYSTEM is used.
 */
streng *run_popen( tsd_t *TSD, const streng *command, const streng *envir )
{
   int rc;
   streng *retval;
   struct envir *ptr=NULL ;
   Queue *q ;

   for ( ptr = (struct envir *)TSD->firstenvir; ptr; ptr = ptr->prev )
   {
      if ( ( ptr->type == ENVIR_SHELL ) && ( Str_cmp( ptr->e.name, envir ) == 0 ) )
         break;
   }
   if ( ptr == NULL )
   {
      retval = Str_creTSD( "SYSTEM" ); /* temporary misuse */
      ptr = find_envir( TSD, retval );
      Free_stringTSD( retval ) ;
   }

   /* Create a new environment with no redirections. */
   add_envir(TSD, ptr->e.name, ENVIR_SHELL, ptr->e.subtype);

   q = find_free_slot( TSD ) ;
   q->type = QisTemp;
   rc = posix_do_command(TSD, command, REDIR_OUTSTRING, (environment *)TSD->firstenvir, q ) ;
   retval = stack_to_line( TSD, q ) ;

   /* restore the previous environment and delete the temporary one */
   del_envir(TSD, ptr->e.name);

   set_reserved_value( TSD, POOL0_RC, NULL, rc, VFLAG_NUM );

   if (rc >= 0)
      return(retval);

   /* rc shows an error while calling: */
   Free_stringTSD(retval);
   return(NULL);
}
