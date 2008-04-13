/***********************************************************************/
/* REXX.C - REXX interface routines.                                   */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

static char RCSid[] = "$Id: rexx.c,v 1.18 2002/07/11 10:24:27 mark Exp $";

#if defined(__EMX__) && defined(USE_REGINA)
# define INCL_VIO
# define INCL_KBD
# define INCL_DOSPROCESS     /* Process/Thread Info */
# include <os2.h>
#endif

# define INCL_RXFUNC         /* External function handler values */
# define INCL_RXSUBCOM       /* Subcommand handler values */
# define INCL_RXSHV          /* Shared variable support */
# define INCL_RXSYSEXIT      /* System exit routines */

#include <the.h>
#include <proto.h>

_LINE *rexxout_first_line=NULL;
_LINE *rexxout_last_line=NULL;
_LINE *rexxout_curr=NULL;
LINETYPE rexxout_number_lines=0L;

#if (defined(HAVE_PROTO) && !defined(NOREXX)) || defined(USE_REXXIMC)

/*
 * Because Rexx interpreters include <windows.h> on Windows platforms
 * and it defines MOUSE_MOVED, we have to undef it here to avoid
 * conflict.
 */
#undef MOUSE_MOVED
#include <query.h>
#include <therexx.h>

#if defined(HAVE_SYS_WAIT_H)
# include <sys/wait.h>
#endif

#define BUF_SIZE 512

#if defined(USE_REXX6000)
LONG THE_Commands( RSH_ARG0_TYPE, RSH_ARG1_TYPE, RSH_ARG2_TYPE );
LONG THE_Exit_Handler( REH_ARG0_TYPE, REH_ARG1_TYPE, REH_ARG2_TYPE );
LONG ver_handler( REH_ARG0_TYPE, REH_ARG1_TYPE, REH_ARG2_TYPE );
LONG rexx_interpreter_version_exit( REH_ARG0_TYPE, REH_ARG1_TYPE, REH_ARG2_TYPE );
USHORT THE_Function_Handler( RFH_ARG0_TYPE, RFH_ARG1_TYPE, RFH_ARG2_TYPE, RFH_ARG3_TYPE, RFH_ARG4_TYPE );
#else
RexxSubcomHandler THE_Commands;
RexxExitHandler THE_Exit_Handler;
RexxExitHandler rexx_interpreter_version_exit;
RexxFunctionHandler THE_Function_Handler;
#endif

static RXSTRING *get_compound_rexx_variable Args((CHARTYPE *,RXSTRING *,short));
static short valid_target_function Args((ULONG, RXSTRING []));
static short run_os_function Args((ULONG, RXSTRING []));
static int run_os_command Args((CHARTYPE *,CHARTYPE *,CHARTYPE *,CHARTYPE *));
static CHARTYPE *MakeAscii Args((RXSTRING *));
static char *get_a_line Args((FILE *, char *, int *, int *));
static short set_rexx_variables_from_file Args((char *,CHARTYPE *));
static void *THEAllocateMemory Args(( ULONG size ));
static void THEFreeMemory Args(( void *ptr ));

static LINETYPE captured_lines;
static bool rexx_halted;
static CHARTYPE version_buffer[MAX_FILE_NAME+1];


/***********************************************************************
 * This function allocates memory for the Rexx interpreter
 ***********************************************************************/
void *THEAllocateMemory
#if defined(HAVE_PROTO)
   (
   ULONG size)
#else
   (size)
   ULONG size;
#endif
/***********************************************************************/
{
   void *ptr;

   TRACE_FUNCTION("rexx.c:    THEAllocateMemory");

#if defined(REXXALLOCATEMEMORY)
   ptr = (RXSTRING_STRPTR_TYPE)RexxAllocateMemory( size );
#elif defined(WIN32) && (defined(USE_OREXX) || defined(USE_WINREXX) || defined(USE_QUERCUS))
   ptr = (RXSTRING_STRPTR_TYPE)GlobalAlloc( GMEM_FIXED, size );
#elif defined(USE_OS2REXX)
   DosAllocMem( (void *)ptr, size, PAG_READ|PAG_WRITE|PAG_COMMIT );
#else
   ptr = (RXSTRING_STRPTR_TYPE)malloc( size );
#endif
   TRACE_RETURN();
   return ptr;
}

/***********************************************************************
 * This function frees memory allocated by the Rexx interpreter
 ***********************************************************************/
void THEFreeMemory
#if defined(HAVE_PROTO)
   (
   void *ptr)
#else
   (ptr)
   void *ptr;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("rexx.c:    THEFreeMemory");
#if defined(REXXFREEMEMORY)
   RexxFreeMemory( ptr );
#elif defined(WIN32) && (defined(USE_OREXX) || defined(USE_WINREXX) || defined(USE_QUERCUS))
   GlobalFree( ptr );
#elif defined(USE_OS2REXX)
   DosFreeMem( ptr );
#else
   free( ptr );
#endif
   TRACE_RETURN();
   return;
}

/***********************************************************************
 * This function is made available for functions to deregister implied
 * functions.
 ***********************************************************************/
unsigned long MyRexxDeregisterFunction
#if defined(HAVE_PROTO)
   (
   CHARTYPE *Name)       /* Name of function to be deregistered  */
#else
   (Name)
   CHARTYPE *Name;       /* Name of function to be deregistered  */
#endif
/***********************************************************************/
{
   CHARTYPE newname[80];
   ULONG rc;

   TRACE_FUNCTION("rexx.c:    MyRexxDeregisterFunction");

   rc = RexxDeregisterFunction((RDF_ARG0_TYPE)Name);
   strcpy((DEFCHAR *)newname, (DEFCHAR *)Name);
   make_upper(newname);
   rc = RexxDeregisterFunction((RDF_ARG0_TYPE)newname);
   TRACE_RETURN();
   return rc;
}

/***********************************************************************/
/* This hack is by imc (sorry) - when registering external functions,  */
/* register both the original name and the name in upper case, so as   */
/* to satisfy case sensitive interpreters.                             */
/***********************************************************************/
unsigned long MyRexxRegisterFunctionExe
#if defined(HAVE_PROTO)
   (
   CHARTYPE *Name)         /* Name of function to be registered  */
#else
   (Name)
   CHARTYPE *Name;         /* Name of function to be registered  */
#endif
/***********************************************************************/
{
   CHARTYPE _THE_FAR newname[256];
   ULONG rc;

   TRACE_FUNCTION("rexx.c:    MyRexxRegisterFunctionExe");

#if defined(USE_REXX6000)
   rc = RexxRegisterFunction( (RRFE_ARG0_TYPE)Name,
                              (RRFE_ARG1_TYPE)THE_Function_Handler,
                              NULL);
#else
   rc = RexxRegisterFunctionExe( (RRFE_ARG0_TYPE)Name, (RRFE_ARG1_TYPE)THE_Function_Handler) ;
   /*
    * If the lowercase name was registered successfully, then there is no point in
    * trying the uppercase version, so return.
    */
   if ( rc == RXFUNC_OK )
   {
      TRACE_RETURN();
      return rc;
   }
   if ( strlen( (DEFCHAR *)Name) + 1 > (sizeof newname)/(sizeof (CHARTYPE) ) )
   {
      TRACE_RETURN();
      return rc;
   }
   strcpy( (DEFCHAR *)newname, (DEFCHAR *)Name );
   make_upper( newname );
   rc = RexxRegisterFunctionExe( (RRFE_ARG0_TYPE)newname, (RRFE_ARG1_TYPE)THE_Function_Handler );
   /*
    * Regina will return with RXFUNC_DEFINED as it ignores the case
    * of the external function name.
    * Object Rexx returns RXFUNC_NOTREG.
    */
   if ( rc == RXFUNC_DEFINED
   ||   rc == RXFUNC_NOTREG )
      rc = RXFUNC_OK;
#endif
   TRACE_RETURN();
   return rc;
}

/***********************************************************************/
RSH_RETURN_TYPE THE_Commands
#if defined(HAVE_PROTO)
   (
   RSH_ARG0_TYPE Command,    /* Command string passed from the caller    */
   RSH_ARG1_TYPE Flags,      /* pointer to short for return of flags     */
   RSH_ARG2_TYPE Retstr)     /* pointer to RXSTRING for RC return        */
#else
   ( Command, Flags, Retstr )
   RSH_ARG0_TYPE Command;    /* Command string passed from the caller    */
   RSH_ARG1_TYPE Flags;      /* pointer to short for return of flags     */
   RSH_ARG2_TYPE Retstr;     /* pointer to RXSTRING for RC return        */
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("rexx.c:    THE_Commands");

   if (allocate_temp_space(Command->strlength,TEMP_TEMP_CMD) != RC_OK)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      *Flags = RXSUBCOM_ERROR;             /* raise an error condition   */
      sprintf((DEFCHAR *)Retstr->strptr, "%d", rc);   /* format return code string  */
                                           /* and set the correct length */
      Retstr->strlength = strlen((DEFCHAR *)Retstr->strptr);
      TRACE_RETURN();
      return 0L;                           /* processing completed       */
   }
   memcpy(temp_cmd,Command->strptr,Command->strlength);
   temp_cmd[Command->strlength] = '\0';
   rc = command_line(temp_cmd,COMMAND_ONLY_FALSE);
   if (rc < 0)
      *Flags = RXSUBCOM_ERROR;             /* raise an error condition   */
   else
      *Flags = RXSUBCOM_OK;                /* not found is not an error  */

   sprintf((DEFCHAR *)Retstr->strptr, "%d", rc); /* format return code string  */
                                           /* and set the correct length */
   Retstr->strlength = strlen((DEFCHAR *)Retstr->strptr);
   TRACE_RETURN();
   return 0L;                              /* processing completed       */
}
#ifdef USE_REXX6000
/***********************************************************************/
/* This is a nop function for REXX6000 for getting the version */
/***********************************************************************/
REH_RETURN_TYPE ver_exit
#if defined(HAVE_PROTO)
   (
   REH_ARG0_TYPE ExitNumber,    /* code defining the exit function    */
   REH_ARG1_TYPE Subfunction,   /* code defining the exit subfunction */
   REH_ARG2_TYPE ParmBlock      /* function dependent control block   */
   )
#else
   ( ExitNumber, Subfunction, ParmBlock )
   REH_ARG0_TYPE ExitNumber;    /* code defining the exit function    */
   REH_ARG1_TYPE Subfunction;   /* code defining the exit subfunction */
   REH_ARG2_TYPE ParmBlock;     /* function dependent control block   */
#endif
/***********************************************************************/
{
 return(0L);
}
#endif

/***********************************************************************/
REH_RETURN_TYPE THE_Exit_Handler
#if defined(HAVE_PROTO)
   (
   REH_ARG0_TYPE ExitNumber,    /* code defining the exit function    */
   REH_ARG1_TYPE Subfunction,   /* code defining the exit subfunction */
   REH_ARG2_TYPE ParmBlock      /* function dependent control block   */
   )
#else
   ( ExitNumber, Subfunction, ParmBlock )
   REH_ARG0_TYPE ExitNumber;    /* code defining the exit function    */
   REH_ARG1_TYPE Subfunction;   /* code defining the exit subfunction */
   REH_ARG2_TYPE ParmBlock;     /* function dependent control block   */
#endif
/***********************************************************************/
{
   RXSIOTRC_PARM *trc_parm;
   RXFNCCAL_PARM *fnc_parm;
   LONG rc=0L;
   short errnum=0;
   short macrorc;
   char rexxout_temp[60];
   CHARTYPE *macroname=NULL;
   VIEW_DETAILS *found_view=NULL;
   static bool first=TRUE;
   RXSYSEXIT exit_list[3];                /* system exit list           */
   RXSTRING retstr;

   switch( ExitNumber )
   {
      case RXSIO:
         if ( Subfunction != RXSIOSAY
         &&   Subfunction != RXSIOTRC )
         {
            rc = RXEXIT_NOT_HANDLED;
            break;
         }
         trc_parm = (RXSIOTRC_PARM *)ParmBlock;
         /*
          * If this is the first time this exit handler is called, set up the
          * handling of the result; either open the capture file, or set the
          * terminal out of curses mode so scrolling etc. work.
          */
         if (!rexx_output)
         {
            rexx_output = TRUE;
            if (CAPREXXOUTx)
            {
               /*
                * Initialise the temporary file...
                */
               if (first)
               {
                  first = FALSE;
                  strcpy((DEFCHAR *)rexx_pathname,(DEFCHAR *)dir_pathname);
                  strcat((DEFCHAR *)rexx_pathname,(DEFCHAR *)rexxoutname);
                  if (splitpath(rexx_pathname) != RC_OK)
                  {
                     rc = RXEXIT_RAISE_ERROR;
                  }
                  strcpy((DEFCHAR *)rexx_pathname,(DEFCHAR *)sp_path);
                  strcpy((DEFCHAR *)rexx_filename,(DEFCHAR *)sp_fname);
               }
               /*
                * Free up the existing linked list (if any)
                */
#if !defined(MULTIPLE_PSEUDO_FILES)
               rexxout_first_line = rexxout_last_line = lll_free(rexxout_first_line);
               rexxout_number_lines = 0L;
               if ((found_view = find_file(rexx_pathname,rexx_filename)) != (VIEW_DETAILS *)NULL)
               {
                  found_view->file_for_view->first_line = found_view->file_for_view->last_line = NULL;
                  found_view->file_for_view->number_lines = 0L;
               }
#endif
               /*
                * first_line is set to "Top of File"
                */
               if ((rexxout_first_line = add_LINE(rexxout_first_line,NULL,TOP_OF_FILE,
                  strlen((DEFCHAR *)TOP_OF_FILE),0,FALSE)) == NULL)
               rc = RXEXIT_RAISE_ERROR;
               /*
                * last line is set to "Bottom of File"
                */
               if ((rexxout_last_line = add_LINE(rexxout_first_line,rexxout_first_line,BOTTOM_OF_FILE,
                  strlen((DEFCHAR *)BOTTOM_OF_FILE),0,FALSE)) == NULL)
               rc = RXEXIT_RAISE_ERROR;
               rexxout_curr = rexxout_first_line;
               if (found_view != (VIEW_DETAILS *)NULL)
               {
                  found_view->file_for_view->first_line = rexxout_first_line;
                  found_view->file_for_view->last_line = rexxout_last_line;
               }
            }
#ifndef XCURSES
            else
            {
               if (!batch_only)
               {
                  wmove(statarea,0,COLS-1);
                  wrefresh(statarea);
                  suspend_curses();
               }
               printf("\n");                       /* scroll the screen 1 line */
               fflush(stdout);
            }
#endif
         }
         /*
          * If the REXX interpreter has been halted by line limit exceeded, just
          * return to the interpreter indicating that THE is hadnling the output
          * of messages. This is done to stop the "clutter" that comes back as
          * the interpreter tries to tell us that it is stopping.
          */
         if (rexx_halted)
            return(RXEXIT_HANDLED);
         /*
          * If we are capturing the rexx output, print the string to the file.
          */
         if (CAPREXXOUTx)
         {
            rexxout_number_lines++;
            if ((rexxout_curr = add_LINE(rexxout_first_line,rexxout_curr,
                                 (CHARTYPE *)trc_parm->rxsio_string.strptr,
                                 (trc_parm->rxsio_string.strlength==(-1))?0:trc_parm->rxsio_string.strlength,0,FALSE)) == NULL)
               rc = RXEXIT_RAISE_ERROR;
            else
               rc = RXEXIT_HANDLED;
         }
         else
            rc = RXEXIT_NOT_HANDLED;
         /*
          * If the number of lines processed exceeds the line limit, display our
          * own message telling what has happened and exit with
          * RXEXIT_RAISE_ERROR. This tells the interpreter that it is to stop.
          */
         if (++captured_lines > CAPREXXMAXx)
         {
            if (CAPREXXOUTx)
            {
               rexxout_number_lines++;
               sprintf(rexxout_temp,"THE: REXX macro halted - line limit (%ld) exceeded",CAPREXXMAXx);
               rexxout_curr = add_LINE(rexxout_first_line,rexxout_curr,
                               (CHARTYPE *)rexxout_temp,
                               strlen((DEFCHAR *)rexxout_temp),0,FALSE);
            }
            else
               printf("THE: REXX macro halted - line limit (%ld) exceeded\n",CAPREXXMAXx);
            rc = RXEXIT_RAISE_ERROR;
            rexx_halted = TRUE;
         }
         break;
      case RXFNC:
         fnc_parm = (RXFNCCAL_PARM *)ParmBlock;
/*       fprintf(stderr,"Got a RXFNC: %s %d\n",fnc_parm->rxfnc_name,fnc_parm->rxfnc_namel); */
         /*
          * Allocate some space for macroname...
          */
         if ((macroname = (CHARTYPE *)(*the_malloc)((MAX_FILE_NAME+1)*sizeof(CHARTYPE))) == NULL)
         {
            display_error(30,(CHARTYPE *)"",FALSE);
            return(RC_OUT_OF_MEMORY);
         }
         /*
          * Find the fully qualified file name for the supplied macro name.
          */
         rc = get_valid_macro_file_name( fnc_parm->rxfnc_name, macroname, macro_suffix, &errnum );
         /*
          * Validate the return code...
          */
         if ( rc != RC_OK )
         {
            rc = RXEXIT_NOT_HANDLED;
            (*the_free)(macroname);
            break;
         }
         /*
          * Found the macro file, we will process it instead of the Rexx interpreter
          */
         /*
          * Set up pointer to REXX Exit Handler.
          */
#if defined(USE_REXX6000)
         exit_list[0].sysexit_func = THE_Exit_Handler;
#else
         exit_list[0].sysexit_name = "THE_EXIT";
         exit_list[1].sysexit_name = "THE_EXIT";
#endif
         exit_list[0].sysexit_code = RXSIO;
         exit_list[1].sysexit_code = RXFNC;
         exit_list[2].sysexit_code = RXENDLST;
         MAKERXSTRING( retstr, NULL, 0 );
         /*
          * Call the REXX interpreter.
          */
         rc = RexxStart((RS_ARG0_TYPE)fnc_parm->rxfnc_argc,
                (RS_ARG1_TYPE)fnc_parm->rxfnc_argv,
                (RS_ARG2_TYPE)macroname,
                (RS_ARG3_TYPE)NULL,
                (RS_ARG4_TYPE)"THE",
                (RS_ARG5_TYPE)RXSUBROUTINE,
                (RS_ARG6_TYPE)exit_list,
                (RS_ARG7_TYPE)&macrorc,
                (RS_ARG8_TYPE)&retstr);
         if (rc != RC_OK)
         {
            display_error(54,(CHARTYPE *)"",FALSE);
            rc = RXEXIT_NOT_HANDLED;
         }
         else
         {
            if ( retstr.strlength > RXAUTOBUFLEN )
            {
               fnc_parm->rxfnc_retc.strptr = retstr.strptr;
               fnc_parm->rxfnc_retc.strlength = retstr.strlength;
               /* The Rexx interpreter needs to free the memory in retstr.strptr */
            }
            else
            {
               memcpy( fnc_parm->rxfnc_retc.strptr, retstr.strptr, retstr.strlength );
               fnc_parm->rxfnc_retc.strlength = retstr.strlength;
               THEFreeMemory( retstr.strptr );
            }
            rc = RXEXIT_HANDLED;
         }
         (*the_free)(macroname);
         break;
      default:
         rc = RXEXIT_NOT_HANDLED;
   }
   return(rc);
}
/***********************************************************************/
RFH_RETURN_TYPE THE_Function_Handler
#if defined(HAVE_PROTO)
   (
   RFH_ARG0_TYPE FunctionName,  /* name of function */
   RFH_ARG1_TYPE Argc,          /* number of arguments    */
   RFH_ARG2_TYPE Argv,          /* array of arguments in RXSTRINGs */
   RFH_ARG3_TYPE QueueName,     /* name of queue */
   RFH_ARG4_TYPE Retstr         /* return string   */
   )
#else
   ( FunctionName, Argc, Argv, QueueName, Retstr )
   RFH_ARG0_TYPE FunctionName;  /* name of function */
   RFH_ARG1_TYPE Argc;          /* number of arguments    */
   RFH_ARG2_TYPE Argv,          /* array of arguments in RXSTRINGs */
   RFH_ARG3_TYPE QueueName;     /* name of queue */
   RFH_ARG4_TYPE Retstr;        /* return string   */
#endif

/***********************************************************************/
{
   int functionname_length=0;
   int itemno=0,item_index=0,num_vars=0,rc=0;

   itemno = split_function_name( (CHARTYPE *)FunctionName, &functionname_length );
   /*
    * If the tail is > maximum number of variables that we can
    * handle, exit with error.
    */
   if ( itemno > MAX_VARIABLES_RETURNED )
      return(1);
   /*
    * Find the external function name in the array. Error if not found.
    */
   set_compare_exact( TRUE );
   if ( itemno == (-1) )
   {
      rc = search_query_item_array( function_item,
                                    number_function_item(),
                                    sizeof( QUERY_ITEM ),
                                    (DEFCHAR *)FunctionName,
                                    functionname_length );
      if (rc == (-1))
         return(1);
      item_index = function_item[rc].item_values;
      switch(function_item[rc].item_number)
      {
         case ITEM_VALID_TARGET_FUNCTION:
            if ( number_of_files == 0 )
            {
               display_error( 83, (CHARTYPE *)"", FALSE );
               return(1);
            }
            valid_target_function( Argc, Argv );
            break;
         case ITEM_RUN_OS_FUNCTION:
            run_os_function( Argc, Argv );
            break;
         default:
            if ( number_of_files == 0
            &&   function_item[rc].level != LVL_GLOB )
            {
               display_error( 83, (CHARTYPE *)"", FALSE );
               return(1);
            }
            if ( (Argv == NULL) || (Argc == 0) ) /* FGC */
            {
               num_vars = get_item_values( 0,
                                           function_item[rc].item_number,
                                           (CHARTYPE *)"",
                                           QUERY_FUNCTION,
                                           (LINETYPE)Argc,
                                           NULL,
                                           0L );
            }
            else
            {
               num_vars = get_item_values( 0,
                                           function_item[rc].item_number,
                                           (CHARTYPE *)"",
                                           QUERY_FUNCTION,
                                           (LINETYPE)Argc,
                                           (CHARTYPE *)Argv[0].strptr,
                                           (LINETYPE)Argv[0].strlength );
            }
            break;
      }
   }
   else
   {
      rc = search_query_item_array( query_item,
                                    number_query_item(),
                                    sizeof( QUERY_ITEM ),
                                    (DEFCHAR *)FunctionName,
                                    functionname_length );
      if (rc == (-1))
         return(1);
      item_index = itemno;
      if ( number_of_files == 0
      &&   query_item[rc].level != LVL_GLOB )
      {
         display_error( 83, (CHARTYPE *)"", FALSE );
         return(1);
      }
      if ( (Argv == NULL) || (Argc == 0) ) /* FGC */
      {
         num_vars = get_item_values( 1,
                                     query_item[rc].item_number,
                                     (CHARTYPE *)"",
                                     QUERY_FUNCTION,
                                     (LINETYPE)Argc,
                                     NULL,
                                     0L );
      }
      else
      {
         num_vars = get_item_values( 1,
                                     query_item[rc].item_number,
                                     (CHARTYPE *)"",
                                     QUERY_FUNCTION,
                                     (LINETYPE)Argc,
                                     (CHARTYPE *)Argv[0].strptr,
                                     (LINETYPE)Argv[0].strlength );
      }
   }

   if ( item_values[item_index].len > 256 )
   {
      /* this MUST use the appropriate memory allocation routine that the */
      /* Rexx interpreter expects.                                        */
      Retstr->strptr = (RXSTRING_STRPTR_TYPE)THEAllocateMemory( item_values[item_index].len );
      if (Retstr->strptr == NULL)
      {
         display_error( 30, (CHARTYPE *)"", FALSE );
         return(1);
      }
   }
   memcpy( Retstr->strptr, item_values[item_index].value, item_values[item_index].len );
   Retstr->strlength = item_values[item_index].len;
   return(0);
}
/***********************************************************************/
short initialise_rexx
#if defined(HAVE_PROTO)
      (void)
#else
      ()
#endif
/***********************************************************************/
{
   ULONG rc;
   register int i=0,j=0;
   int num_values=0;
   char _THE_FAR buf[50];

   TRACE_FUNCTION("rexx.c:    initialise_rexx");

#if defined(MSWIN)
   if (RexxThread(GetCurrentTask(),THREAD_ATTACH) != THREAD_ATTACH_AOK)
   {
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
#endif

#if defined(USE_REXX6000)
   rc = RexxRegisterSubcom((RRSE_ARG0_TYPE)"THE",
                           (RRSE_ARG1_TYPE)THE_Commands,
                           (RRSE_ARG2_TYPE)NULL);
#else
   rc = RexxRegisterSubcomExe((RRSE_ARG0_TYPE)"THE",
                              (RRSE_ARG1_TYPE)THE_Commands,
                              (RRSE_ARG2_TYPE)NULL);
#endif
   if (rc != RXSUBCOM_OK)
   {
      TRACE_RETURN();
      return((short)rc);
   }

#if !defined(USE_REXX6000)
   rc = RexxRegisterExitExe((RREE_ARG0_TYPE)"THE_EXIT",
                            (RREE_ARG1_TYPE)THE_Exit_Handler,
                            (RREE_ARG2_TYPE)NULL);
   if (rc != RXEXIT_OK)
   {
      TRACE_RETURN();
      return((short)rc);
   }
#endif

   for (i=0; i<number_function_item(); i++)
   {
      rc = MyRexxRegisterFunctionExe(function_item[i].name);
      if (rc != RXFUNC_OK)
         break;
   }
   if (rc != RXFUNC_OK)
   {
      TRACE_RETURN();
      return((short)rc);
   }

   for (i=0; i<number_query_item(); i++)
   {
      num_values = query_item[i].item_values + 1;
      for (j=0;j<num_values;j++)
      {
         sprintf(buf,"%s.%d",query_item[i].name,j);
         rc = MyRexxRegisterFunctionExe((CHARTYPE *)buf);
         if (rc != RXFUNC_OK)
            break;
      }
      if (rc != RXFUNC_OK)
         break;
   }
   TRACE_RETURN();
   return((short)rc);
}
/***********************************************************************/
short finalise_rexx
#if defined(HAVE_PROTO)
    (void)
#else
    ()
#endif
/***********************************************************************/
{
   ULONG rc;
#ifdef NO_NEED_TO_DEREGISTER_FUNCTIONS_ANYMORE
   register short i=0,j=0;
   int num_values=0;
   char _THE_FAR buf[50];
#endif

   TRACE_FUNCTION("rexx.c:    finalise_rexx");

#if defined(USE_REXX6000)
   rc = RexxDeregisterSubcom((RDS_ARG0_TYPE)"THE");
#else
   rc = RexxDeregisterSubcom((RDS_ARG0_TYPE)"THE",
                           (RDS_ARG1_TYPE)NULL);
   rc = RexxDeregisterExit((RDE_ARG0_TYPE)"THE_EXIT",
                         (RDE_ARG1_TYPE)NULL);
#endif

#ifdef NO_NEED_TO_DEREGISTER_FUNCTIONS_ANYMORE
   /*
    * MHES - 7-4-2001
    * Don't bother deregistering the external functions. As this is an EXE
    * the function registrations will disappear when the process dies. The
    * main reason for not deregistering is now that Object Rexxx for Linux
    * v 2.2 now works with THE, it takes way too long to shut down, and
    * Reiner Micke suggested it not be done.
    */
   /*
    * Deregister all the builtin functions
    */
   for (i=0; i<number_function_item(); i++)
   {
      rc = MyRexxDeregisterFunction(function_item[i].name);
   }
   /*
    * Deregister all the implied functions
    */
   for (i=0; i<number_query_item(); i++)
   {
      num_values = query_item[i].item_values + 1;
      if ( num_values == 1 )
      {
         /*
          * These are dynamic functions, so the number of values
          * to deregister is based on the number of registered functions.
          */
         num_values = 1+get_number_dynamic_items( query_item[i].item_number );
      }
      for (j=0;j<num_values;j++)
      {
         sprintf(buf,"%s.%d",query_item[i].name,j);
         rc = MyRexxDeregisterFunction(buf);
      }
   }
#endif

#if defined(MSWIN)
   (void)RexxThread(GetCurrentTask(),THREAD_DETACH);
#endif

   TRACE_RETURN();
   return((short)rc);
}
/***********************************************************************/
short execute_macro_file
#if defined(HAVE_PROTO)
      (CHARTYPE *filename, CHARTYPE *params, short *macrorc, bool interactive )
#else
      (filename,params,macrorc,interactive)
       CHARTYPE *filename;
       CHARTYPE *params;
       short *macrorc;
       bool interactive;
#endif
/***********************************************************************/
{
#if defined(USE_REXX6000)
   LONG rexxrc=0L;
#else
   SHORT rexxrc=0L;
#endif
   RXSTRING retstr;
   RXSTRING argstr;
   ULONG rc=0;
/*   CHAR retbuf[260]; */
   LONG num_params=0L;
   CHARTYPE *rexx_args=NULL;
   RXSYSEXIT exit_list[3];                /* system exit list           */

   TRACE_FUNCTION("rexx.c:    execute_macro_file");
   /*
    * Determine how many parameters are to be passed to the interpreter.
    * Only 0 or 1 are valid values.
    */
   if (params == NULL
   || strcmp((DEFCHAR *)params,"") == 0)
   {
      num_params = 0;
      MAKERXSTRING(argstr,"",0);
      strcpy((DEFCHAR *)rexx_macro_parameters,"");
   }
   else
   {
      num_params = 1;
      if ((rexx_args = (CHARTYPE *)(*the_malloc)(strlen((DEFCHAR *)params)+1)) == (CHARTYPE *)NULL)
      {
         display_error(30,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY);
      }
      strcpy((DEFCHAR *)rexx_macro_parameters,(DEFCHAR *)params);
      strcpy((DEFCHAR *)rexx_args,(DEFCHAR *)params);
      MAKERXSTRING(argstr,(DEFCHAR *)rexx_args,strlen((DEFCHAR *)rexx_args));
   }

/*   MAKERXSTRING(retstr,retbuf,sizeof(retbuf)); */
   MAKERXSTRING( retstr, NULL, 0 );

   strcpy((DEFCHAR *)rexx_macro_name,(DEFCHAR *)filename);
   /*
    * Set up pointer to REXX Exit Handler.
    */
#if defined(USE_REXX6000)
   exit_list[0].sysexit_func = THE_Exit_Handler;
#else
   exit_list[0].sysexit_name = "THE_EXIT";
   exit_list[1].sysexit_name = "THE_EXIT";
#endif
   exit_list[0].sysexit_code = RXSIO;
   exit_list[1].sysexit_code = RXFNC;
   exit_list[2].sysexit_code = RXENDLST;
#if (defined(WIN32) || defined(OS2)) && defined(HAVE_RESET_PROG_MODE)
   /*
    * Save the current "program" settings
    */
   if ( curses_started )
      def_prog_mode();
#endif
   /*
    * Under OS/2 use of interactive trace in a macro only works if an OS
    * command has been run before executing the macro, so we run a REM
    */
   if (interactive)
   {
#if defined(OS2) || defined(WIN32)
      execute_os_command((CHARTYPE*)"REM",TRUE,FALSE);
      reset_shell_mode();
#endif
#ifdef UNIX
      execute_os_command((CHARTYPE*)"echo",TRUE,FALSE);
#endif
   }

   captured_lines = 0L;
   rexx_output = FALSE;
   rexx_halted = FALSE;
   /*
    * Call the REXX interpreter.
    */
   rc = RexxStart((RS_ARG0_TYPE)num_params,
                (RS_ARG1_TYPE)&argstr,
                (RS_ARG2_TYPE)filename,
                (RS_ARG3_TYPE)NULL,
                (RS_ARG4_TYPE)"THE",
                (RS_ARG5_TYPE)RXCOMMAND,
                (RS_ARG6_TYPE)exit_list,
                (RS_ARG7_TYPE)&rexxrc,
                (RS_ARG8_TYPE)&retstr);
   /*
    * If we get a return value in retstr (ie retstr.strptr != NULL)
    * then the Rexx interpreter has allocated some memory for us.
    * We don't want it, free it up.
    */
   THEFreeMemory( retstr.strptr );
   /*
    * Edit the captured file or clean up after REXX output displays.
    */
   if (rexx_output)
   {
      rexx_output = FALSE;
      if (CAPREXXOUTx)
      {
#if defined(MULTIPLE_PSEUDO_FILES)
         Xedit((CHARTYPE *)"***REXXOUT***");
#else
         strcpy((DEFCHAR *)temp_cmd,(DEFCHAR *)rexx_pathname);
         strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)rexx_filename);
         Xedit(temp_cmd);
#endif
      }
      else
      {
         if (batch_only)
            error_on_screen = TRUE;
#ifndef XCURSES
         else
         {
            /*
             * Pause for operator intervention and restore the screen to the
             * current screen if there are still file(s) in the ring.
             */
            printf("\n%s",HIT_ANY_KEY);
            fflush(stdout);
            resume_curses();
            (void)my_getch(stdscr);
            if (number_of_files > 0)
            {
#if defined(HAVE_BROKEN_SYSVR4_CURSES)
               short x=0,y=0;
               getyx(CURRENT_WINDOW,y,x);
               force_curses_background();
               wmove(CURRENT_WINDOW,y,x);
               refresh();
#endif
               restore_THE();
            }
         }
#endif
      }
   }
#if (defined(WIN32) || defined(OS2)) && defined(HAVE_RESET_PROG_MODE)
   else
   {
      if ( curses_started )
         reset_prog_mode();
   }
#endif

   if (rexx_args != NULL)
      (*the_free)(rexx_args);
   *macrorc = (short)rexxrc;
   TRACE_RETURN();
   return((short)rc);
}

/***********************************************************************/
short execute_macro_instore
#if defined(HAVE_PROTO)
      (CHARTYPE *commands,short *macrorc,CHARTYPE **pcode,int *pcode_len,int *tokenised, int macro_ident)
#else
      (commands, macrorc,pcode,pcode_len, tokenised)
       CHARTYPE *commands;
       short *macrorc;
       CHARTYPE **pcode;
       int *pcode_len;
       int *tokenised;
       int macro_ident;
#endif
/***********************************************************************/
{
#if defined(__EMX__) || defined(USE_REGINA)
   SHORT rexxrc=0;
#elif defined(USE_REXX6000)
   LONG rexxrc=0L;
#else
   USHORT rexxrc=0L;
#endif
   RXSTRING instore[2];
   RXSTRING retstr;
   CHAR _THE_FAR retbuf[260];
   CHARTYPE _THE_FAR macro_name[20];
   ULONG rc=0;
   LONG num_params=0L;
   RXSYSEXIT exit_list[2];                /* system exit list           */
   bool save_in_macro=in_macro;

   TRACE_FUNCTION("rexx.c:    execute_macro_instore");
   in_macro = TRUE;
   sprintf((DEFCHAR *)macro_name,"INSTORE%d",macro_ident);
   /*
    * Set up pointer to REXX Exit Handler.
    */
#if defined(USE_REXX6000)
   exit_list[0].sysexit_func = THE_Exit_Handler;
#else
   exit_list[0].sysexit_name = "THE_EXIT";
#endif
   exit_list[0].sysexit_code = RXSIO;
   exit_list[1].sysexit_code = RXENDLST;
#if (defined(WIN32) || defined(OS2)) && defined(HAVE_RESET_PROG_MODE)
   /*
    * Save the current "program" settings
    */
   if ( curses_started )
      def_prog_mode();
#endif

   captured_lines = 0L;
   rexx_output = FALSE;
   rexx_halted = FALSE;
   instore[0].strptr = (RXSTRING_STRPTR_TYPE)commands;
   instore[0].strlength = strlen((DEFCHAR *)commands);
   if (pcode)
      instore[1].strptr = (RXSTRING_STRPTR_TYPE)*pcode;
   else
      instore[1].strptr = (RXSTRING_STRPTR_TYPE)NULL;
   instore[1].strlength = (pcode_len) ? *pcode_len : 0;
   MAKERXSTRING(retstr,retbuf,sizeof(retbuf));
   /*
    * Call the REXX interpreter.
    */
   rc = RexxStart((RS_ARG0_TYPE)num_params,
                (RS_ARG1_TYPE)NULL,
                (RS_ARG2_TYPE)macro_name,
                (RS_ARG3_TYPE)instore,
                (RS_ARG4_TYPE)"THE",
                (RS_ARG5_TYPE)RXCOMMAND,
                (RS_ARG6_TYPE)exit_list,
                (RS_ARG7_TYPE)&rexxrc,
                (RS_ARG8_TYPE)&retstr);
   if (instore[1].strptr)
   {
      if (pcode_len
      &&  pcode
      &&  *pcode == NULL) /* first time an instore defined key is hit */
      {
         *pcode = (CHARTYPE*)(*the_malloc)(instore[1].strlength);
         if (*pcode != NULL)
         {
            *pcode_len = instore[1].strlength;
            memcpy((DEFCHAR *)*pcode,(DEFCHAR *)instore[1].strptr,*pcode_len);
            *tokenised = 1;
         }
      }
      /*
       * If the pointer to the pcode returned by RexxStart() is NOT the
       * same as the pointer we already have, then free it.
       */
      if (pcode_len
      &&  pcode
      &&  (CHARTYPE*)*pcode != (CHARTYPE*)instore[1].strptr)
      {
         THEFreeMemory( instore[1].strptr );
      }
   }
   /*
    * Edit the captured file or clean up after REXX output displays.
    */
   if (rexx_output)
   {
      rexx_output = FALSE;
      if (CAPREXXOUTx)
      {
#if defined(MULTIPLE_PSEUDO_FILES)
         Xedit((CHARTYPE *)"***REXXOUT***");
#else
         strcpy((DEFCHAR *)temp_cmd,(DEFCHAR *)rexx_pathname);
         strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)rexx_filename);
         Xedit(temp_cmd);
#endif
      }
      else
      {
         if (batch_only)
            error_on_screen = TRUE;
#ifndef XCURSES
         else
         {
            /*
             * Pause for operator intervention and restore the screen to the
             * current screen if there are still file(s) in the ring.
             */
            printf("\n%s",HIT_ANY_KEY);
            fflush(stdout);
            resume_curses();
            (void)my_getch(stdscr);
            if (number_of_files > 0)
            {
#if defined(HAVE_BROKEN_SYSVR4_CURSES)
               short x=0,y=0;
               getyx(CURRENT_WINDOW,y,x);
               force_curses_background();
               wmove(CURRENT_WINDOW,y,x);
               refresh();
#endif
               restore_THE();
            }
         }
#endif
      }
   }
#if (defined(WIN32) || defined(OS2)) && defined(HAVE_RESET_PROG_MODE)
   else
   {
      if ( curses_started )
         reset_prog_mode();
   }
#endif
   in_macro = save_in_macro;
   *macrorc = (short)rexxrc;
   TRACE_RETURN();
   return((short)rc);
}

/***********************************************************************/
REH_RETURN_TYPE rexx_interpreter_version_exit
#if defined(HAVE_PROTO)
   (
   REH_ARG0_TYPE ExitNumber,    /* code defining the exit function    */
   REH_ARG1_TYPE Subfunction,   /* code defining the exit subfunction */
   REH_ARG2_TYPE ParmBlock      /* function dependent control block   */
   )
#else
   ( ExitNumber, Subfunction, ParmBlock )
   REH_ARG0_TYPE ExitNumber;    /* code defining the exit function    */
   REH_ARG1_TYPE Subfunction;   /* code defining the exit subfunction */
   REH_ARG2_TYPE ParmBlock;     /* function dependent control block   */
#endif
/***********************************************************************/
{
   SHVBLOCK shv;
   int rc=0;

   shv.shvnext=NULL;
   shv.shvcode=RXSHV_PRIV;
   shv.shvname.strptr = "VERSION" ;
   shv.shvname.strlength = 7;
   shv.shvnamelen = shv.shvname.strlength ;
   shv.shvvalue.strptr = (DEFCHAR *)version_buffer ;
   shv.shvvalue.strlength=MAX_FILE_NAME;
   shv.shvvaluelen = shv.shvvalue.strlength ;
   rc = RexxVariablePool(&shv);            /* get the next variable */
   if (rc == 0)
      version_buffer[shv.shvvaluelen] = '\0';
   return(RXEXIT_HANDLED);
}

/***********************************************************************/
CHARTYPE *get_rexx_interpreter_version
#if defined(HAVE_PROTO)
      (CHARTYPE *buf)
#else
      (buf)
       CHARTYPE *buf;
#endif
/***********************************************************************/
{
 RXSYSEXIT Exits[2] ;
 RXSTRING Instore[2] ;
 RS_ARG7_TYPE rexxrc=0;
 int rc ;

 if (rexx_support)
 {
    strcpy((DEFCHAR *)version_buffer,"Unable to obtain Rexx version from interpreter");
#if defined(USE_REXX6000)
    Exits[0].sysexit_func = ver_exit;
#else
    RexxRegisterExitExe((RREE_ARG0_TYPE)"ver_exit",
                        (RREE_ARG1_TYPE)rexx_interpreter_version_exit,
                        (RREE_ARG2_TYPE)NULL);

    Exits[0].sysexit_name = "ver_exit" ;
#endif
    Exits[0].sysexit_code = RXTER ;
    Exits[1].sysexit_code = RXENDLST ;

    Instore[0].strptr = "/**/;nop;" ;
    Instore[0].strlength = strlen( Instore[0].strptr ) ;
    Instore[1].strptr = NULL ;
    Instore[1].strlength = 0 ;

    rc = RexxStart((RS_ARG0_TYPE)0,
                (RS_ARG1_TYPE)NULL,
                (RS_ARG2_TYPE)"instore",
                (RS_ARG3_TYPE)Instore,
                (RS_ARG4_TYPE)"ver",
                (RS_ARG5_TYPE)RXCOMMAND,
                (RS_ARG6_TYPE)Exits,
                (RS_ARG7_TYPE)&rexxrc,
                (RS_ARG8_TYPE)NULL);

#if defined(USE_REXX6000)
    RexxDeregisterExit( "ver_exit", NULL ) ;
#endif
    if (rc == 0)
       strcpy((DEFCHAR *)buf,(DEFCHAR *)version_buffer);
 }
 else
 {
    strcpy((DEFCHAR *)buf,"NONE");
 }
 return(buf);
}

/***********************************************************************/
short get_rexx_variable
#if defined(HAVE_PROTO)
      (CHARTYPE *name,CHARTYPE **value,int *value_length)
#else
      (name,value,value_length)
       CHARTYPE *name;
       CHARTYPE **value;
       int *value_length;
#endif
/***********************************************************************/
{
   RXSTRING retstr;
   short rc=RC_OK;

   TRACE_FUNCTION("rexx.c:    get_rexx_variable");
   MAKERXSTRING(retstr,"",0);
   (void)get_compound_rexx_variable(name,&retstr,(-1));
   if (retstr.strptr == NULL)
      rc = RC_INVALID_ENVIRON;
   *value = (CHARTYPE*)retstr.strptr;
   *value_length = retstr.strlength;
   TRACE_RETURN();
   return(rc);
}

/***********************************************************************/
short set_rexx_variable
#if defined(HAVE_PROTO)
      (CHARTYPE *name,CHARTYPE *value,short value_length,short suffix)
#else
      (name,value,value_length,suffix)
       CHARTYPE *name;
       CHARTYPE *value;
       short value_length;
       short suffix;
#endif
/***********************************************************************/
{
   SHVBLOCK shv;
   CHAR _THE_FAR variable_name[250];
   short rc=0;

   TRACE_FUNCTION("rexx.c:    set_rexx_variable");

   shv.shvnext=NULL;                                   /* only one block */
   shv.shvcode=RXSHV_SET;                              /* use direct set */
   /*
    * This calls the RexxVariablePool() function for each value. This is
    * not the most efficient way of doing this.
    */
   if (suffix == (-1))
      strcpy(variable_name,(DEFCHAR*)name);
   else
      sprintf(variable_name,"%s.%-d",name,suffix);
   (void)make_upper((CHARTYPE *)variable_name);/* make variable name uppercase */
   /*
    * Now (attempt to) set the REXX variable
    * Add name/value to SHVBLOCK
    */
   MAKERXSTRING(shv.shvname, variable_name, strlen(variable_name));
   MAKERXSTRING(shv.shvvalue,(DEFCHAR *)value,value_length);
   /*
    * One or both of these is needed, too <sigh>
    */
   shv.shvnamelen=strlen(variable_name);
   shv.shvvaluelen=value_length;

#if defined(USE_OS2REXX) || defined(USE_REXX6000)
   rc=(short)RexxVariablePool(&shv);              /* Set the REXX variable */
#else
   rc = RexxVariablePool(&shv);                 /* Set the REXX variable */
/* rc = RXSHV_OK;*/
#endif
   if (rc != RXSHV_OK
   &&  rc != RXSHV_NEWV)
   {
      display_error(25,(CHARTYPE *)"",FALSE);
      rc = RC_SYSTEM_ERROR;
   }
   else
      rc = RC_OK;
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
static RXSTRING *get_compound_rexx_variable
#if defined(HAVE_PROTO)
      (CHARTYPE *name,RXSTRING *value,short suffix)
#else
      (name,value,suffix)
       CHARTYPE *name;
       RXSTRING *value;
       short suffix;
#endif
/***********************************************************************/
{
   static SHVBLOCK shv;
   CHAR _THE_FAR variable_name[250];
   short rc=0;

   TRACE_FUNCTION("rexx.c:    get_compound_rexx_variable");
   shv.shvnext=NULL;                                   /* only one block */
   shv.shvcode=RXSHV_FETCH;                            /* use direct set */
   /*
    * This calls the RexxVariablePool() function for each value. This is
    * not the most efficient way of doing this.
    */
   if (suffix == (-1))
      strcpy(variable_name,(DEFCHAR*)name);
   else
      sprintf(variable_name,"%s.%-d",name,suffix);
#if 0
 (void)make_upper((CHARTYPE *)variable_name);/* make variable name uppercase */
#endif
   /*
    * Now (attempt to) get the REXX variable
    * Set shv.shvvalue to NULL to force interpreter to allocate memory.
    */
   MAKERXSTRING(shv.shvname, variable_name, strlen(variable_name));
   shv.shvvalue.strptr = NULL;
   shv.shvvalue.strlength = 0;
   /*
    * One or both of these is needed, too <sigh>
    */
   shv.shvnamelen=strlen(variable_name);
   shv.shvvaluelen=0;
#if defined(USE_OS2REXX) || defined(USE_REXX6000)
   rc=(short)RexxVariablePool(&shv);              /* Set the REXX variable */
#else
   rc = RexxVariablePool(&shv);                 /* Set the REXX variable */
#endif
   switch(rc)
   {
      case RXSHV_OK:
#ifdef USE_REXX6000
         value->strptr = (PUCHAR)(*the_malloc)((sizeof(char)*shv.shvvalue.strlength)+1);
#else
         value->strptr = (char *)(*the_malloc)((sizeof(char)*shv.shvvalue.strlength)+1);
#endif
         if (value->strptr != NULL)
         {
            value->strlength = shv.shvvalue.strlength;
            memcpy(value->strptr,shv.shvvalue.strptr,value->strlength);
            *(value->strptr+value->strlength) = '\0';
         }
#if defined(REXXFREEMEMORY)
         RexxFreeMemory( shv.shvvalue.strptr );
#elif defined(WIN32) && (defined(USE_OREXX) || defined(USE_WINREXX) || defined(USE_QUERCUS))
         GlobalFree( shv.shvvalue.strptr );
#elif defined(USE_OS2REXX)
         DosFreeMem( shv.shvvalue.strptr );
#else
         free( shv.shvvalue.strptr );
#endif
         break;
      case RXSHV_NEWV:
#ifdef USE_REXX6000
         value->strptr = (PUCHAR)(*the_malloc)((sizeof(char)*shv.shvname.strlength)+1);
#else
         value->strptr = (char *)(*the_malloc)((sizeof(char)*shv.shvname.strlength)+1);
#endif
         if (value->strptr != NULL)
         {
            value->strlength = shv.shvname.strlength;
            memcpy(value->strptr,shv.shvname.strptr,value->strlength);
            *(value->strptr+value->strlength) = '\0';
         }
         break;
      default:
         value->strptr = NULL;
         value->strlength = 0;
         break;
   }
   TRACE_RETURN();
   return(value);
}
/***********************************************************************/
static short valid_target_function
#if defined(HAVE_PROTO)
      (ULONG Argc,RXSTRING Argv[])
#else
     (Argc,Argv)
      ULONG Argc;
      RXSTRING Argv[];
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 static TARGET target={NULL,0L,0L,0L,NULL,0,0,FALSE};
 short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
 LINETYPE true_line=0L;
 short rc=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("rexx.c:    valid_target_function");
 switch(1)
   {
    case 1:
         if (Argc < 1 || Argc > 2)  /* incorrect no of arguments - error */
           {
            item_values[1].value = (CHARTYPE *)"ERROR";
            item_values[1].len = 5;
            break;
           }
         if (Argc == 2)
            target_type = target_type | TARGET_SPARE;
/* allocate sufficient space for the spare string and 2 longs */
         if (target_buffer == NULL)
           {
            target_buffer = (CHARTYPE *)(*the_malloc)(sizeof(CHARTYPE)*(Argv[0].strlength+30));
            target_buffer_len = Argv[0].strlength+30;
           }
         else
           {
            if (target_buffer_len < Argv[0].strlength+30)
              {
               target_buffer = (CHARTYPE *)(*the_realloc)(target_buffer,sizeof(CHARTYPE)*(Argv[0].strlength+30));
               target_buffer_len = Argv[0].strlength+30;
              }
           }
         if (target_buffer == (CHARTYPE *)NULL)
           {
            item_values[1].value = (CHARTYPE *)"ERROR";
            item_values[1].len = 5;
            free_target(&target);
            break;
           }
         memcpy(target_buffer,Argv[0].strptr,Argv[0].strlength);
         *(target_buffer+Argv[0].strlength) = '\0';
         if (in_prefix_macro)
            true_line = prefix_current_line;
         else
            true_line = get_true_line(TRUE);

         initialise_target(&target);
         rc = validate_target(target_buffer,&target,target_type,true_line,FALSE,FALSE);
         if (rc == RC_TARGET_NOT_FOUND)
           {
            item_values[1].value = (CHARTYPE *)"NOTFOUND";
            item_values[1].len = 8;
            free_target(&target);
            break;
           }
         if (rc != RC_OK)
           {
            item_values[1].value = (CHARTYPE *)"ERROR";
            item_values[1].len = 5;
            free_target(&target);
            break;
           }
         if (Argc == 2
         &&  target.spare != (-1))
            sprintf((DEFCHAR *)target_buffer,"%ld %ld %s",target.true_line,target.num_lines,
                                        target.rt[target.spare].string);
         else
             sprintf((DEFCHAR *)target_buffer,"%ld %ld",target.true_line,target.num_lines);
         item_values[1].value = target_buffer;
         item_values[1].len = strlen((DEFCHAR *)target_buffer);
         free_target(&target);
         break;
   }
 item_values[0].value = (CHARTYPE *)"1";
 item_values[0].len = 1;
 TRACE_RETURN();
 return(rc);
}
/***********************************************************************/
static short run_os_function
#if defined(HAVE_PROTO)
      (ULONG Argc,RXSTRING Argv[])
#else
     (Argc,Argv)
      ULONG Argc;
      RXSTRING Argv[];
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 int rc=0;
 static CHARTYPE num0[5];                  /* large enough for 1000+rc */
 CHARTYPE *cmd=NULL,*instem=NULL,*outstem=NULL,*errstem=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("rexx.c:    run_os_function");
 switch(Argc)
   {
    case 0:
         sprintf((DEFCHAR *)num0,"%d",RC_INVALID_OPERAND+1000);
         break;
    case 4:
         if ((errstem = (CHARTYPE *)MakeAscii(&Argv[3])) == NULL)
           {
            sprintf((DEFCHAR *)num0,"%d",RC_OUT_OF_MEMORY+1000);
            break;
           }
    case 3:
         if ((outstem = (CHARTYPE *)MakeAscii(&Argv[2])) == NULL)
           {
            sprintf((DEFCHAR *)num0,"%d",RC_OUT_OF_MEMORY+1000);
            break;
           }
    case 2:
         if ((instem = (CHARTYPE *)MakeAscii(&Argv[1])) == NULL)
           {
            sprintf((DEFCHAR *)num0,"%d",RC_OUT_OF_MEMORY+1000);
            break;
           }
    case 1:
         if ((cmd = (CHARTYPE *)MakeAscii(&Argv[0])) == NULL)
           {
            sprintf((DEFCHAR *)num0,"%d",RC_OUT_OF_MEMORY+1000);
            break;
           }
         rc = run_os_command(cmd,instem,outstem,errstem);
         sprintf((DEFCHAR *)num0,"%d",rc);
         break;
    default:
         sprintf((DEFCHAR *)num0,"%d",RC_INVALID_OPERAND+1000);
         break;
   }
 item_values[1].value = num0;
 item_values[1].len = strlen((DEFCHAR *)num0);
 item_values[0].value = (CHARTYPE *)"1";
 item_values[0].len = 1;
 if (cmd) (*the_free)(cmd);
 if (instem) (*the_free)(instem);
 if (outstem) (*the_free)(outstem);
 if (errstem) (*the_free)(errstem);
 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
static int run_os_command
#if defined(HAVE_PROTO)
      (CHARTYPE *cmd,CHARTYPE *instem,CHARTYPE *outstem,CHARTYPE *errstem)
#else
      (cmd,instem,outstem,errstem)
       CHARTYPE *cmd;
       CHARTYPE *instem;
       CHARTYPE *outstem;
       CHARTYPE *errstem;
#endif
/***********************************************************************/
{
   RXSTRING tmpstr;
   bool in=TRUE,out=TRUE,err=TRUE;
   bool out_and_err_same=FALSE;
   int inlen=0,outlen=0,errlen=0;
   int i=0;
   FILE *infp=NULL;
   char *infile="",*outfile="",*errfile="";
   long innum=0L;
   int rc=0,rcode=0;
   int save_stdin=0,save_stdout=0,save_stderr=0;
   int infd=0,outfd=0,errfd=0;

   TRACE_FUNCTION("rexx.c:    run_os_command");
   /*
    * Determine if we are redirecting stdin, stdout or both and if the
    * values passed as stem variables end in '.'.
    */
   if (instem == NULL
   ||  strcmp((DEFCHAR *)instem,"") == 0)
      in = FALSE;
   else
   {
      inlen = strlen((DEFCHAR *)instem);
      if (*(instem+inlen-1) == '.')
         *(instem+inlen-1) = '\0';
      else
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND+1000);
      }
   }
   if (outstem == NULL
   ||  strcmp((DEFCHAR *)outstem,"") == 0)
      out = FALSE;
   else
   {
      outlen = strlen((DEFCHAR *)outstem);
      if (*(outstem+outlen-1) == '.')
         *(outstem+outlen-1) = '\0';
      else
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND+1000);
      }
   }
   if (errstem == NULL
   ||  strcmp((DEFCHAR *)errstem,"") == 0)
      err = FALSE;
   else
   {
      errlen = strlen((DEFCHAR *)errstem);
      if (*(errstem+errlen-1) == '.')
         *(errstem+errlen-1) = '\0';
      else
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND+1000);
      }
   }
   /*
    * Ensure that stdin stem is different to both stdout and stderr stems.
    */
   if (in)
   {
      if (out
      &&  strcmp((DEFCHAR *)instem,(DEFCHAR *)outstem) == 0)
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND+1000);
      }
      if (err
      &&  strcmp((DEFCHAR *)instem,(DEFCHAR *)errstem) == 0)
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND+1000);
      }
   }
   /*
    * An extra check here to determine if stdout and stderr are to be
    * redirected to the same place.
    */
   if (out && err)
   {
      if (strcmp((DEFCHAR *)outstem,(DEFCHAR *)errstem)==0)
         out_and_err_same = TRUE;
   }
   /*
    * If redirecting stdin, get the value of instem.0 to determine how
    * many variables to get...
    */
   if (in)
   {
      tmpstr.strptr = NULL;
      (void)get_compound_rexx_variable(instem,&tmpstr,0);
      if (tmpstr.strptr == NULL)
      {
         TRACE_RETURN();
         return(RC_SYSTEM_ERROR+1000);
      }
      if (!valid_positive_integer((CHARTYPE *)tmpstr.strptr))
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND+1000);
      }
      innum = atol((DEFCHAR *)tmpstr.strptr);
      (*the_free)(tmpstr.strptr);
      /*
       * Write the contents of the stdin stem to a temporary file...
       */
      infile = (char *)(*the_malloc)(L_tmpnam);
      if (infile == NULL)
      {
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY+1000);
      }
      if ((infile = tmpnam(infile)) == NULL)
      {
         TRACE_RETURN();
         return(RC_ACCESS_DENIED+1000);
      }
#if defined(GO32) || defined(__EMX__)
      /*
       * For djgpp and emx convert all \ to / for internal file handling
       * functions.
       */
      strrmdup(strtrans(infile,'\\','/'),'/',TRUE);
#endif
      if ((infp = fopen(infile,"w")) == NULL)
      {
         TRACE_RETURN();
         return(RC_ACCESS_DENIED+1000);
      }
      for (i=0;i<innum;i++)
      {
         tmpstr.strptr = NULL;
         (void)get_compound_rexx_variable(instem,&tmpstr,i+1);
         if (tmpstr.strptr == NULL)
         {
            TRACE_RETURN();
            return(RC_SYSTEM_ERROR+1000);
         }
         fputs((DEFCHAR *)tmpstr.strptr,infp);
         fputs("\n",infp);
         (*the_free)(tmpstr.strptr);
      }
      if (fclose(infp))
      {
         TRACE_RETURN();
         return(RC_ACCESS_DENIED+1000);
      }
   }
   /*
    * If redirecting stdout, create the name of a temporary file for the
    * output.
    */
   if (out)
   {
      outfile = (char *)(*the_malloc)(L_tmpnam);
      if (outfile == NULL)
      {
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY+1000);
      }
      if ((outfile = tmpnam(outfile)) == NULL)
      {
         TRACE_RETURN();
         return(RC_ACCESS_DENIED+1000);
      }
   }
   /*
    * If redirecting stderr, create the name of a temporary file for the
    * output.
    */
   if (err)
   {
      if (out_and_err_same)
      {
         errfile = outfile;
      }
      else
      {
         errfile = (char *)(*the_malloc)(L_tmpnam);
         if (errfile == NULL)
         {
            TRACE_RETURN();
            return(RC_OUT_OF_MEMORY+1000);
         }
         if ((errfile = tmpnam(errfile)) == NULL)
         {
            TRACE_RETURN();
            return(RC_ACCESS_DENIED+1000);
         }
      }
   }
   /*
    * Save file ids for stdin, stdout and stderr, then reassign them to
    * the appropriate temporary files.
    */
   if (in) save_stdin = dup(fileno(stdin));
   if (out) save_stdout = dup(fileno(stdout));
   if (err) save_stderr = dup(fileno(stderr));
   if (in)
   {
      if ((infd = open(infile,O_RDONLY)) == (-1))
      {
         TRACE_RETURN();
         return(RC_ACCESS_DENIED+1000);
      }
   }
   if (out)
   {
#if defined(S_IWRITE) && defined(S_IREAD)
      if ((outfd = open(outfile, O_RDWR|O_CREAT|O_TRUNC, S_IWRITE|S_IREAD)) == (-1))
#else
      if ((outfd = open(outfile, O_RDWR|O_CREAT|O_TRUNC)) == (-1))
#endif
      {
         TRACE_RETURN();
         return(RC_ACCESS_DENIED+1000);
      }
   }
   if (out_and_err_same)
      errfd = outfd;
   else
   {
      if (err)
      {
#if defined(UNIX)
         if ((errfd = open(errfile,O_RDWR|O_CREAT|O_TRUNC)) == (-1))
#else
         if ((errfd = open(errfile,O_RDWR|O_CREAT|O_TRUNC,S_IWRITE|S_IREAD)) == (-1))
#endif
         {
            TRACE_RETURN();
            return(RC_ACCESS_DENIED+1000);
         }
      }
   }
#if defined(UNIX)
   if (out)
      chmod(outfile,S_IWUSR|S_IRUSR);
   if (!out_and_err_same)
   {
      if (err)
         chmod(errfile,S_IWUSR|S_IRUSR);
   }
#endif
   if (in)  dup2(infd,fileno(stdin));
   if (out) dup2(outfd,fileno(stdout));
   if (err) dup2(errfd,fileno(stderr));
   if (in)  close(infd);
   if (out) close(outfd);
   if (!out_and_err_same)
   {
      if (err) close(errfd);
   }
   /*
    * Execute the OS command supplied.
    */
   rcode = system((DEFCHAR *)cmd);
#if defined(HAVE_SYS_WAIT_H)
   if (rcode) rcode = WEXITSTATUS(rcode);
#endif
   /*
    * Put all file ids back the way they were...
    */
   if (in)  dup2(save_stdin,fileno(stdin));
   if (out) dup2(save_stdout,fileno(stdout));
   if (err) dup2(save_stderr,fileno(stderr));
   if (in)  close(save_stdin);
   if (out) close(save_stdout);
   if (err) close(save_stderr);
   /*
    * If redirecting stdout, we now have to read the file and set a REXX
    * variable for each line read.
    */
   if (out)
      rc = set_rexx_variables_from_file(outfile,outstem);
   if (err)
   {
      if (!out_and_err_same)
         rc = set_rexx_variables_from_file(errfile,errstem);
   }
   /*
    * Delete the temporary file(s) and free up any memory.
    */
   if (in)
   {
      unlink(infile);
      (*the_free)(infile);
   }
   if (out)
   {
      unlink(outfile);
      (*the_free)(outfile);
   }
   if (err)
   {
      if (!out_and_err_same)
      {
         unlink(errfile);
         (*the_free)(errfile);
      }
   }
   /*
    * Return with, hopefully, return code from system() command.
    */
   TRACE_RETURN();
   if (rc)
      return(rc+1000);
   else
      return(rcode);
}
/***********************************************************************/
static CHARTYPE *MakeAscii
#if defined(HAVE_PROTO)
      (RXSTRING *rxstring)
#else
     (rxstring)
      RXSTRING *rxstring;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE *string=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("rexx.c:    MakeAscii");

 string = (CHARTYPE *)(*the_malloc)((sizeof(CHARTYPE)*rxstring->strlength)+1);
 if (string != NULL)
   {
    memcpy(string,rxstring->strptr,rxstring->strlength);
    *(string+(rxstring->strlength)) = '\0';
   }
 TRACE_RETURN();
 return(string);
}
/***********************************************************************/
static char *get_a_line
#if defined(HAVE_PROTO)
      (FILE *fp,char *string,int *length,int *rcode)
#else
      (fp,string,length,rcode)
      FILE *fp;
      char *string;
      int *length;
      int *rcode;
#endif
/***********************************************************************/
{
 int ch;
 static int bufs = 1;
 register int i=0;
/*---------------------------------------------------------------------*/
/* Allocate the first block of memory.                                 */
/*---------------------------------------------------------------------*/
 if ((string = (char *)(*the_malloc)(BUF_SIZE+1)) == NULL)
   {
    *rcode = RC_OUT_OF_MEMORY;
    return(NULL);
   }
 while(1)
   {
/*---------------------------------------------------------------------*/
/* Read a character from the stream...                                 */
/*---------------------------------------------------------------------*/
    if ((ch = fgetc(fp)) == EOF)
      {
/*---------------------------------------------------------------------*/
/* If EOF is reached, check that it really is end of file.             */
/*---------------------------------------------------------------------*/
       if (feof(fp))
         {
          *length = i;
          *rcode = RC_TOF_EOF_REACHED;
          return(string);
         }
      }
/*---------------------------------------------------------------------*/
/* If end of line is reached, nul terminate string and return.         */
/*---------------------------------------------------------------------*/
    if ((char)ch == '\n')
      {
       *(string+i) = '\0';
       break;
      }
/*---------------------------------------------------------------------*/
/* All other characters, copy to string.                               */
/*---------------------------------------------------------------------*/
    *(string+i++) = (char)ch;
/*---------------------------------------------------------------------*/
/* If we have got to the end of the allocated memory, realloc some more*/
/*---------------------------------------------------------------------*/
    if (i == BUF_SIZE*bufs)
      {
       if ((string = (char *)realloc(string,(BUF_SIZE*(++bufs))+1)) == NULL)
         {
          *rcode = RC_OUT_OF_MEMORY;
          return(NULL);
         }
      }
   }
/*---------------------------------------------------------------------*/
/* Return a line read from the temporary file.                         */
/*---------------------------------------------------------------------*/
 *length = i;
 *rcode = 0;
 return(string);
}
/***********************************************************************/
static short set_rexx_variables_from_file
#if defined(HAVE_PROTO)
      (char *filename,CHARTYPE *stem)
#else
      (filename,stem)
       char *filename;
       CHARTYPE *stem;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 FILE *fp=NULL;
 register int i=0;
 char *string=NULL;
 int length=0,rcode=0,rc=0;
 char tmpnum[10];
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("rexx.c:    set_rexx_variables_from_file");
 if ((fp = fopen(filename,"r")) == NULL)
   {
    TRACE_RETURN();
    return(RC_ACCESS_DENIED);
   }
 for (i=0;;i++)
   {
    string = get_a_line(fp,string,&length,&rcode);
    if (rcode == RC_OUT_OF_MEMORY)
      {
       TRACE_RETURN();
       return(rcode);
      }
    if (rcode == RC_TOF_EOF_REACHED
    && length == 0)
      {
       (*the_free)(string);
       break;
      }
    rc = set_rexx_variable(stem,(CHARTYPE *)string,strlen(string),i+1);
    (*the_free)(string);
    if (rcode == RC_TOF_EOF_REACHED)
       break;
   }
 sprintf(tmpnum,"%d",i);
 rc = set_rexx_variable(stem,(CHARTYPE *)tmpnum,strlen(tmpnum),0);
 if (fclose(fp))
    rc = RC_ACCESS_DENIED;
 TRACE_RETURN();
 return(rc);
}
#else
/*---------------------------------------------------------------------*/
/* The following are dummy routines to enable the compliation of THE   */
/* with a nonANSI compiler.                                            */
/*---------------------------------------------------------------------*/
/***********************************************************************/
short initialise_rexx
#if defined(HAVE_PROTO)
      (void)
#else
      ()
#endif
/***********************************************************************/
{
 return(RC_INVALID_ENVIRON);                        /* force an error */
}
/***********************************************************************/
short finalise_rexx
#if defined(HAVE_PROTO)
      (void)
#else
      ()
#endif
/***********************************************************************/
{
 return(RC_OK);
}
/***********************************************************************/
short execute_macro_file
#if defined(HAVE_PROTO)
      (CHARTYPE *filename,CHARTYPE *params,short *macrorc,bool interactive)
#else
      (filename,params,macrorc,interactive)
      CHARTYPE *filename;
      CHARTYPE *params;
      short *macrorc;
      bool interactive;
#endif
/***********************************************************************/
{
 return(RC_OK);
}
/***********************************************************************/
short execute_macro_instore
#if defined(HAVE_PROTO)
      (CHARTYPE *commands,short *macrorc,CHARTYPE **pcode,int *pcode_len,int *tokenised,int macro_ident)
#else
      (commands,macrorc,pcode,pcode_len,tokenised,macro_ident)
      CHARTYPE *commands;
      short *macrorc;
      CHARTYPE **pcode;
      int *pcode_len;
      int tokenised;
      int macro_ident;
#endif
/***********************************************************************/
{
 return(RC_OK);
}
/***********************************************************************/
short set_rexx_variable
#if defined(HAVE_PROTO)
      (CHARTYPE *name,CHARTYPE *value,short value_length,short suffix)
#else
      (name,value,value_length,suffix)
      CHARTYPE *name;
      CHARTYPE *value;
      short value_length;
      short suffix;
#endif
/***********************************************************************/
{
 return(RC_OK);
}

/***********************************************************************/
short get_rexx_variable
#if defined(HAVE_PROTO)
      (CHARTYPE *name,CHARTYPE **value,int *value_length)
#else
      (name,value,value_length)
       CHARTYPE *name;
       CHARTYPE **value;
       int *value_length;
#endif
/***********************************************************************/
{
 return(RC_OK);
}

/***********************************************************************/
CHARTYPE *get_rexx_interpreter_version
#if defined(HAVE_PROTO)
      (CHARTYPE *buf)
#else
      (buf)
       CHARTYPE *buf;
#endif
/***********************************************************************/
{
 strcpy((DEFCHAR *)buf,"NONE");
 return(buf);
}

/***********************************************************************/
unsigned long MyRexxDeregisterFunction
#if defined(HAVE_PROTO)
   (
   CHARTYPE *Name)       /* Name of function to be deregistered        */
#else
   (Name)
   CHARTYPE *Name;       /* Name of function to be deregistered        */
#endif
/***********************************************************************/
{
   return 0L;
}

/***********************************************************************/
unsigned long MyRexxRegisterFunctionExe
#if defined(HAVE_PROTO)
   (
   CHARTYPE *Name)         /* Name of function to be registered        */
#else
   (Name)
   CHARTYPE *Name;         /* Name of function to be registered        */
#endif
/***********************************************************************/
{
   return 0L;
}
#endif
