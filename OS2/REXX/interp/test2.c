

#define INCL_RXSHV      /* Shared variable support */
#define INCL_RXFUNC     /* External functions support */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rexxsaa.h"

#ifdef _MSC_VER
/* This picky compiler claims about unused formal parameters.
 * This is correct but hides (for human eyes) other errors since they
 * are many and we can't reduce them all.
 * Error 4100 is "unused formal parameter".
 */
# pragma warning(disable:4100)
#endif

#define DLLNAME "rxtest2"


#define FUNCTION1 Test2Function1
#define FUNCTION2 Test2Function2
#define LOADFUNCS Test2LoadFuncs
#define DROPFUNCS Test2DropFuncs

#define NAME_FUNCTION1 "Test2Function1"
#define NAME_FUNCTION2 "Test2Function2"
#define NAME_LOADFUNCS "Test2LoadFuncs"
#define NAME_DROPFUNCS "Test2DropFuncs"

#ifdef __cplusplus
extern "C" {
#endif
RexxFunctionHandler Test2Function1;
RexxFunctionHandler Test2Function2;
RexxFunctionHandler Test2LoadFuncs;
RexxFunctionHandler Test2DropFuncs;
#ifdef __cplusplus
}
#endif

/*-----------------------------------------------------------------------------
 * Table entry for a REXX function.
 *----------------------------------------------------------------------------*/
typedef struct {
   PSZ   function_name;
   PFN   EntryPoint;
} RexxTestFunction;

/*-----------------------------------------------------------------------------
 * Table of REXX Functions. Used to install/de-install functions.
 *----------------------------------------------------------------------------*/
static const RexxTestFunction RexxTestFunctions[] = {
   {(PSZ)NAME_FUNCTION1,   (PFN)Test2Function1  },
   {(PSZ)NAME_FUNCTION2,   (PFN)Test2Function2  },
   {(PSZ)NAME_DROPFUNCS,   (PFN)Test2DropFuncs  },
   {(PSZ)NAME_LOADFUNCS,   (PFN)Test2LoadFuncs  },
   {NULL,NULL}
};

static char *make_upper( char *in )
{
   int len = strlen( in );
   int i;

   for ( i = 0; i < len; i++ )
   {
      if ( islower( in[i] ) )
         in[i] = (char)toupper( in[i] );
   }
   return in;
}

static int set_rexx_variable( char *name, int suffix, char *value, int value_length )
{
   SHVBLOCK shv;
   char variable_name[250];
   int rc=0;

   shv.shvnext=NULL;                                   /* only one block */
   shv.shvcode=RXSHV_SET;                              /* use direct set */
   sprintf( variable_name, "%s.%-d", name, suffix );
   (void)make_upper( variable_name );/* make variable name uppercase */
   /*
    * Now (attempt to) set the REXX variable
    * Add name/value to SHVBLOCK
    */
   MAKERXSTRING( shv.shvname, variable_name, strlen( variable_name) );
   MAKERXSTRING( shv.shvvalue, value, value_length );
   /*
    * One or both of these is needed, too <sigh>
    */
   shv.shvnamelen = strlen( variable_name );
   shv.shvvaluelen = value_length;

   rc = RexxVariablePool( &shv );              /* Set the REXX variable */
   if ( rc != RXSHV_OK
   &&   rc != RXSHV_NEWV)
   {
      rc = 1;
   }
   else
      rc = 0;
   return rc;
}

static void static_show_parameter(ULONG argc, RXSTRING argv[], PSZ func_name)
{
   char buf[100];
   if (argc == 0)
   {
      printf("   %s(static): *** No parameters passed ***\n",DLLNAME);
      return;
   }
   memcpy(buf,argv[0].strptr,argv[0].strlength);
   buf[argv[0].strlength] = '\0';
   if (strcmp(func_name,buf) != 0)
      printf("   %s(static): *** Mismatch of parameters: %s is NOT expected: %s ***\n",
             DLLNAME,buf,func_name);
 return;
}

#if defined(DYNAMIC_STATIC)
static void global_show_parameter(ULONG argc, RXSTRING argv[], PSZ func_name)
#else
void global_show_parameter(ULONG argc, RXSTRING argv[], PSZ func_name)
#endif
{
   char buf[100];
   if (argc == 0)
   {
      printf("   %s(global): *** No parameters passed ***\n",DLLNAME);
      return;
   }
   memcpy(buf,argv[0].strptr,argv[0].strlength);
   buf[argv[0].strlength] = '\0';
   if (strcmp(func_name,buf) != 0)
      printf("   %s(global): *** Mismatch of parameters: %s is NOT expected: %s ***\n",
             DLLNAME,buf,func_name);
   return;
}

APIRET APIENTRY FUNCTION1(PCSZ name,ULONG argc,PRXSTRING argv,PCSZ stck,PRXSTRING retstr)
{
   int i=0;
   char tmp[50];

   for ( i = 0; i < (int)argc; i++ )
   {
      printf("   %s(Test2Function1): Arg: %d <%s>\n", DLLNAME, i, argv[i].strptr );
      /*
       * Set Rexx variables for each argument...
       */
      if ( set_rexx_variable( (char *)name, i+1, argv[i].strptr, argv[i].strlength ) == 1 )
         printf( "%s(Test2Function1): Error setting variable for Arg: %d <%s.%d>\n", DLLNAME, i+1, argv[i].strptr, i+1 );
   }
   sprintf( tmp, "%ld", argc );
   if ( set_rexx_variable( (char *)name, 0, tmp, strlen( tmp ) ) == 1 )
      printf( "%s(Test2Function1): Error setting stem index.\n", DLLNAME );
   static_show_parameter(argc,argv,NAME_FUNCTION1);
   global_show_parameter(argc,argv,NAME_FUNCTION1);
   /*
    * Set return code...
    */
   strcpy(retstr->strptr,"0");
   retstr->strlength = 1;
   return 0L;
}

APIRET APIENTRY FUNCTION2(PCSZ name,ULONG argc,PRXSTRING argv,PCSZ stck,PRXSTRING retstr)
{
   int i=0;
   char tmp[50];

   for ( i = 0; i < (int)argc; i++ )
   {
      printf( "   %s(Test2Function2): Arg: %d <%s>\n", DLLNAME, i, argv[i].strptr );
      /*
       * Set Rexx variables for each argument...
       */
      if ( set_rexx_variable( (char *)name, i+1, argv[i].strptr, argv[i].strlength ) == 1 )
         printf( "%s(Test2Function2): Error setting variable for Arg: %d <%s.%d>\n", DLLNAME, i+1, argv[i].strptr, i+1 );
   }
   sprintf( tmp, "%ld", argc );
   if ( set_rexx_variable( (char *)name, 0, tmp, strlen( tmp ) ) == 1 )
      printf( "%s(Test12unction2): Error setting stem index.\n", DLLNAME );
   static_show_parameter(argc,argv,NAME_FUNCTION2);
   global_show_parameter(argc,argv,NAME_FUNCTION2);
   /*
    * Set return code...
    */
   strcpy(retstr->strptr,"0");
   retstr->strlength = 1;
   return 0L;
}


APIRET APIENTRY DROPFUNCS(PCSZ name,ULONG argc,PRXSTRING argv,PCSZ stck,PRXSTRING retstr)
{
   int rc=0;
   const RexxTestFunction  *func=NULL;

   /* DeRegister all REXX functions */
   for (func = RexxTestFunctions; func->function_name; func++)
      rc = RexxDeregisterFunction(func->function_name);
   sprintf(retstr->strptr,"%d",rc);
   retstr->strlength = strlen(retstr->strptr);
   return 0L;
}


/*-----------------------------------------------------------------------------
 * This function is called to initiate REXX interface.
 *----------------------------------------------------------------------------*/
static int InitTestRexx(PSZ progname)
{
   const RexxTestFunction  *func=NULL;
   ULONG rc=0L;

   /* Register all REXX functions */
   for (func = RexxTestFunctions; func->function_name; func++)
      rc = RexxRegisterFunctionDll(func->function_name,DLLNAME,func->function_name);

   return 0;
}

APIRET APIENTRY LOADFUNCS(PCSZ name,ULONG argc,PRXSTRING argv,PCSZ stck,PRXSTRING retstr)
{
   int rc=0;

   rc = InitTestRexx(DLLNAME);
   printf("   %s built %s %s\n",DLLNAME,__DATE__,__TIME__);
   sprintf(retstr->strptr,"%d",rc);
   retstr->strlength = strlen(retstr->strptr);
   return 0L;
}

#if defined( DYNAMIC_STATIC )
void *getTest2FunctionAddress( char *name )
{
   const RexxTestFunction  *func=NULL;
   for (func = RexxTestFunctions; func->function_name; func++)
   {
      if ( strcmp( func->function_name, name) == 0 )
         return func->EntryPoint;
   }
   return NULL;
}
#endif

#if !defined( DYNAMIC_STATIC )
# ifdef SKYOS
/*
 * Required as entry point for DLL under SkyOS
 */
int DllMain( void )
{
   return 0;
}
# endif
#endif
