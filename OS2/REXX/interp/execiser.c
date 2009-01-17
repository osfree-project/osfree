#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INCL_RXSHV
#define INCL_RXFUNC
#define INCL_RXSYSEXIT
#define INCL_RXSUBCOM
#define INCL_RXQUEUE

#include "rexxsaa.h"

#define STARTUPMSG "       +++ Interactive trace.  \"Trace Off\" to end debug. ENTER to continue."

#ifdef _MSC_VER
/* This picky compiler claims about unused formal parameters.
 * This is correct but hides (for human eyes) other errors since they
 * are many and we can't reduce them all.
 * Error 4100 is "unused formal parameter".
 */
# pragma warning(disable:4100)
#endif

static void message( char *msg )
{
   printf( "  %s", msg ) ;
   fflush(stdout) ;
}


static void mycomplain( char *msg )
{
   fprintf( stderr, "Complaint: %s\n", msg ) ;
}

static void iverify( char *msg, int first, int second )
{
   if (first != second)
      printf( "\nInt verify: %s: %d ^= %d\n", msg, first, second ) ;
}

static void sverify( char *msg, PRXSTRING first, char *second )
{
   if (!second)
   {
      if (first->strptr)
         printf( "\nSecond is null, first: %d<%s>\n", (int) first->strlength,
                 first->strptr  ) ;
   }
   else if (strlen(second) != first->strlength)
   {
      printf( "\nString length: %s: %d ^=%d\n", msg,
              (int) first->strlength, (int) strlen(second)) ;
      printf( "   first=%4d<%s>\n", (int) first->strlength, first->strptr ) ;
      printf( "   secnd=%4d<%s>\n", (int) strlen(second), second ) ;
   }
   else
      if (memcmp(second, first->strptr, first->strlength))
         printf( "\nString contents: %s: <%s> ^== <%s>\n", msg,
                 first->strptr, second ) ;
}




LONG APIENTRY instore_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   RXSIOSAY_PARM *psiosay;
   iverify( "Exitfunction", ExNum, RXSIO ) ;
   iverify( "Subfunction", Subfun, RXSIOSAY ) ;
   psiosay = (RXSIOSAY_PARM *)PBlock;
   sverify( "Exit string", &(psiosay->rxsio_string), "foobar" ) ;

   return RXEXIT_HANDLED ;
}

static void instore( void )
{
   RXSTRING Instore[2] ;
   RXSYSEXIT Exits[2] ;
   int rc ;

   message( "instore" ) ;

   RexxRegisterExitExe( "Foo", instore_exit, NULL ) ;

   Exits[0].sysexit_name = "Foo" ;
   Exits[0].sysexit_code = RXSIO ;
   Exits[1].sysexit_code = RXENDLST ;

   Instore[0].strptr = "say 'foobar'" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;
   rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   rc = RexxStart( 0, NULL, "Testing", Instore, "", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   Instore[0].strptr = NULL ;
   rc = RexxStart( 0, NULL, "Testing", Instore, "", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   rc = RexxStart( 0, NULL, "Testing", Instore, "", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxFreeMemory( Instore[1].strptr ) ;
   RexxDeregisterExit( "Foo", NULL ) ;
}


LONG APIENTRY trace_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   RXSIOTRC_PARM *psiotrc;
   iverify( "Exitfunction", ExNum, RXSIO ) ;
   if (Subfun==RXSIOTRC)
   {
      psiotrc = (RXSIOTRC_PARM *)PBlock;
      sverify( "Exit string", &(psiotrc->rxsio_string),
               "     1 *-* trace off" ) ;
   }
   else
      mycomplain( "Unexpected subfunction\n" ) ;

   return RXEXIT_HANDLED ;
}

static void trace( void )
{
   RXSTRING Instore[2] ;
   RXSYSEXIT Exits[2] ;
   int rc ;

   message( "trace" ) ;

   RexxRegisterExitExe( "Foo", trace_exit, NULL ) ;

   Exits[0].sysexit_name = "Foo" ;
   Exits[0].sysexit_code = RXSIO ;
   Exits[1].sysexit_code = RXENDLST ;

   Instore[0].strptr = "trace all;trace off" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;

   rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxDeregisterExit( "Foo", NULL ) ;
}

static void assign_new_rxstring( PRXSTRING dest, const char *source )
{
   int len;
   char *buf;

   len = strlen( source );
   if ( ( buf = (char *)RexxAllocateMemory( len + 1 ) ) == NULL )
   {
      fprintf( stderr, "RexxAllocateMemory returns NULL.\n" );
      exit( 1 );
   }
   strcpy( buf, source );
   MAKERXSTRING( *dest, buf, len );

}

LONG APIENTRY intertrc_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   static int cnt=0 ;
   static char *data[] = {
      "",
      "     2 *-* say 'hallo'",
      "       >L>   \"hallo\"",
      "hallo",
      STARTUPMSG,
      "=",
      "       *-* say 'hallo'",
      "       >L>   \"hallo\"",
      "hallo",
      "",
      "       *-* trace off",
      "",
   };
   RXSIOTRC_PARM *psiotrc;
   RXSIOSAY_PARM *psiosay;
   RXSIODTR_PARM *psiodtr;

   iverify( "Exitfunction", ExNum, RXSIO ) ;
   switch (++cnt)
   {
       case 1:
       case 2:
       case 4:
       case 6:
       case 7:
       case 10:
       case 11:
          iverify( "Subfunction", Subfun, RXSIOTRC ) ;
          psiotrc = (RXSIOTRC_PARM *)PBlock;
          sverify( "Trace output", &(psiotrc->rxsio_string),data[cnt]);
          break ;

       case 3:
       case 8:
          iverify( "Subfunction", Subfun, RXSIOSAY ) ;
          psiosay = (RXSIOSAY_PARM *)PBlock;
          sverify( "Say output", &(psiosay->rxsio_string), data[cnt]);
          break ;

       case 5:
       case 9:
          iverify( "Subfunction", Subfun, RXSIODTR ) ;
          psiodtr = (RXSIODTR_PARM *)PBlock;
          assign_new_rxstring( &psiodtr->rxsiodtr_retc, data[cnt] );
          break ;

       default:
          fprintf( stderr, "Out of order msg subfunc=%ld\n", Subfun) ;
          return RXEXIT_NOT_HANDLED ;
   }
   return RXEXIT_HANDLED ;
}

static void intertrc( void )
{
   RXSTRING Instore[2] ;
   RXSYSEXIT Exits[2] ;
   int rc ;

   message( "intertrc" ) ;

   RexxRegisterExitExe( "Foo", intertrc_exit, NULL ) ;

   Exits[0].sysexit_name = "Foo" ;
   Exits[0].sysexit_code = RXSIO ;
   Exits[1].sysexit_code = RXENDLST ;

   Instore[0].strptr = "trace ?int\nsay 'hallo'; trace off" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;

   rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxDeregisterExit( "Foo", NULL ) ;
}





LONG APIENTRY pull_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   static int cnt=0 ;
   static char *data[] = {
      "",
      "alpha",
      "ALPHA",
      "FOO",
      "beta",
      "beta",
   };
   RXSIOSAY_PARM *psiosay;
   RXSIOTRD_PARM *psiotrd;

   iverify( "Exitfunction", ExNum, RXSIO ) ;
   switch (++cnt)
   {
       case 2:
       case 3:
       case 5:
          iverify( "Subfunction", Subfun, RXSIOSAY ) ;
          psiosay = (RXSIOSAY_PARM *)PBlock;
          sverify( "Trace output", &(psiosay->rxsio_string),data[cnt]);
          break ;

       case 1:
       case 4:
          iverify( "Subfunction", Subfun, RXSIOTRD ) ;
          psiotrd = (RXSIOTRD_PARM *)PBlock;
          assign_new_rxstring( &psiotrd->rxsiotrd_retc, data[cnt] );
          break ;

       default:
          fprintf( stderr, "Out of order msg subfunc=%ld\n", Subfun) ;
          return RXEXIT_NOT_HANDLED ;
   }
   return RXEXIT_HANDLED ;
}

static void pull( void )
{
   RXSTRING Instore[2] ;
   RXSYSEXIT Exits[2] ;
   int rc ;

   message( "pull" ) ;

   RexxRegisterExitExe( "Foo", pull_exit, NULL ) ;

   Exits[0].sysexit_name = "Foo" ;
   Exits[0].sysexit_code = RXSIO ;
   Exits[1].sysexit_code = RXENDLST ;

   Instore[0].strptr =
     "pull bar;say bar;push 'foo';pull bar;say bar;parse pull bar;say bar" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;

   rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxDeregisterExit( "Foo", NULL ) ;
}


LONG APIENTRY env_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   static int cnt=0 ;
   static char data[10];
   RXENVGET_PARM *penvget;
   RXENVSET_PARM *penvset;

   iverify( "Exitfunction", ExNum, RXENV ) ;
   switch (++cnt)
   {
       case 1:
          iverify( "Subfunction", Subfun, RXENVGET ) ;
          penvget = (RXENVGET_PARM *)PBlock;
          sverify( "Envget name", &(penvget->rxenv_name),"FRED");
          assign_new_rxstring( &penvget->rxenv_value, "initial" );
          break ;

       case 2:
          iverify( "Subfunction", Subfun, RXENVSET ) ;
          penvset = (RXENVSET_PARM *)PBlock;
          sverify( "Envset name", &(penvset->rxenv_name),"FRED");
          sverify( "Envset value", &(penvset->rxenv_value),"junk");
          strcpy( data, "mess" );
          break ;

       case 3:
          iverify( "Subfunction", Subfun, RXENVGET ) ;
          penvget = (RXENVGET_PARM *)PBlock;
          sverify( "Envget name", &(penvget->rxenv_name),"FRED");
          assign_new_rxstring( &penvget->rxenv_value, data );
          break ;

       default:
          fprintf( stderr, "Out of order msg subfunc=%ld\n", Subfun) ;
          return RXEXIT_NOT_HANDLED ;
   }
   return RXEXIT_HANDLED ;
}

static void env( void )
{
   RXSTRING Instore[2] ;
   RXSYSEXIT Exits[2] ;
   int rc ;

   message( "env") ;

   RexxRegisterExitExe( "Foo", env_exit, NULL ) ;

   Exits[0].sysexit_name = "Foo" ;
   Exits[0].sysexit_code = RXENV ;
   Exits[1].sysexit_code = RXENDLST ;

   Instore[0].strptr =
     "call value 'FRED', 'junk', 'ENVIRONMENT'; if value('FRED',,'ENVIRONMENT') <> 'mess' then say 'setenv/getenv failed'" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;

   rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxDeregisterExit( "Foo", NULL ) ;
}




LONG APIENTRY query_init_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   char *name="TEST1" ;
   int rc ;
   char result[100];
   ULONG dup;
   RXSTRING data;

   data.strptr = result;
   rc = RexxCreateQueue( result, sizeof(result), name, &dup ) ;
   iverify( "Query Init exit", rc, RXQUEUE_OK ) ;
   data.strlength = strlen(result);
   sverify( "Query Init exit", &data, name ) ;
   iverify( "Query Init dup", dup, 0L ) ;

   data.strptr = "line1";
   data.strlength = strlen(data.strptr);
   rc = RexxAddQueue( name, &data, RXQUEUE_FIFO ) ;
   iverify( "Query Init exit", rc, RXQUEUE_OK ) ;

   data.strptr = "line2";
   data.strlength = strlen(data.strptr);
   rc = RexxAddQueue( name, &data, RXQUEUE_FIFO ) ;
   iverify( "Query Init exit", rc, RXQUEUE_OK ) ;

   return RXEXIT_HANDLED ;
}

LONG APIENTRY query_term_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   int rc ;
   ULONG count;
   RXSTRING data;
   DATETIME timestamp;

   rc = RexxQueryQueue( "FRED", &count ) ;
   iverify( "Query Term exit", rc, RXQUEUE_OK ) ;
   iverify( "Query Term count", count, 2L ) ;

   data.strptr = NULL;
   rc = RexxPullQueue( "FRED", &data, &timestamp, RXQUEUE_NOWAIT );
   iverify( "Query Term exit", rc, RXQUEUE_OK ) ;
   sverify( "Query Term data", &data, "2line" ) ;
   RexxFreeMemory( data.strptr ) ;

   data.strptr = NULL;
   rc = RexxPullQueue( "FRED", &data, &timestamp, RXQUEUE_NOWAIT );
   iverify( "Query Term exit", rc, RXQUEUE_OK ) ;
   sverify( "Query Term data", &data, "1line" ) ;
   RexxFreeMemory( data.strptr ) ;

   rc = RexxDeleteQueue( "FRED" );
   iverify( "Query Term exit", rc, RXQUEUE_OK ) ;

   rc = RexxDeleteQueue( "TEST1" );
   iverify( "Query Term exit", rc, RXQUEUE_OK ) ;

   return RXEXIT_HANDLED ;
}

LONG APIENTRY it_init_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   SHVBLOCK shv ;
   char *name="FOO" ;
   int rc ;
   char *value="foz" ;

   shv.shvnext = NULL ;
   shv.shvname.strptr = name ;
   shv.shvnamelen = shv.shvname.strlength = strlen(name) ;
   shv.shvvalue.strptr = value ;
   shv.shvvaluelen = shv.shvvalue.strlength = strlen(value) ;
   shv.shvcode = RXSHV_SYSET ;

   rc = RexxVariablePool( &shv ) ;
   iverify( "Init exit", rc, RXSHV_NEWV ) ;
   iverify( "Init exit", shv.shvret, RXSHV_NEWV ) ;
   return RXEXIT_HANDLED ;
}


LONG APIENTRY it_term_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   SHVBLOCK shv ;
   char *name ;
   int rc ;
   char value[64] ;

   strcpy( value, "" ) ;
   shv.shvnext = NULL ;
   name = "MYVAR" ;
   shv.shvname.strptr = name ;
   shv.shvnamelen = shv.shvname.strlength = strlen(name) ;
   shv.shvvalue.strptr = value ;
   shv.shvvaluelen = 64 ;
   shv.shvcode = RXSHV_SYFET ;

   rc = RexxVariablePool( &shv ) ;
   iverify( "Term exit", rc, RXSHV_NEWV ) ;
   iverify( "Term exit", shv.shvret, RXSHV_NEWV ) ;
   sverify( "Term exit:", &(shv.shvvalue), "MYVAR" ) ;

   shv.shvnext = NULL ;
   name = "bar" ;
   shv.shvname.strptr = name ;
   shv.shvnamelen = shv.shvname.strlength = strlen(name) ;

   /* The value is the same as above (value) and therefore valid */
#if 0
    shv.shvvalue.strptr = value ;
    shv.shvvaluelen = 64 ;
   shv.shvvalue.strlength = strlen(value) ; /* not allowed! not 0-terminated */
#endif
   shv.shvcode = RXSHV_SYFET ;

   rc = RexxVariablePool( &shv ) ;
   iverify( "Term exit", rc, RXSHV_OK ) ;
   iverify( "Term exit", shv.shvret, RXSHV_OK ) ;
   sverify( "Term exit:", &(shv.shvvalue), "baz" ) ;

   return RXEXIT_HANDLED ;
}


LONG APIENTRY it_say_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   SHVBLOCK shv ;
   char *name ;
   int rc ;
   char value[64] ;
   RXSIOSAY_PARM *psiosay;

   iverify( "Say exit", ExNum, RXSIO ) ;
   iverify( "Say exit", Subfun, RXSIOSAY ) ;
   psiosay = (RXSIOSAY_PARM *)PBlock;
   sverify( "Say exit", &(psiosay->rxsio_string), "foz" ) ;

   shv.shvnext = NULL ;
   name = "myvar.1" ;
   shv.shvname.strptr = name ;
   shv.shvnamelen = shv.shvname.strlength = strlen(name) ;
   shv.shvvalue.strptr = value ;
   shv.shvvaluelen = 64 ;
   shv.shvcode = RXSHV_SYFET ;

   rc = RexxVariablePool( &shv ) ;
   iverify( "Say exit", rc, RXSHV_NEWV ) ;
   iverify( "Say exit", shv.shvret, RXSHV_NEWV ) ;
   sverify( "Say exit:", &(shv.shvvalue), "MYVAR.1" ) ;

   return RXEXIT_HANDLED ;
}

static void init_term(void)
{
   RXSTRING Instore[2] ;
   RXSYSEXIT Exits[4] ;
   int rc ;

   message( "init/term" ) ;

   RexxRegisterExitExe( "init", it_init_exit, NULL ) ;
   RexxRegisterExitExe( "term", it_term_exit, NULL ) ;
   RexxRegisterExitExe( "say", it_say_exit, NULL ) ;

   Exits[0].sysexit_name = "init" ;
   Exits[0].sysexit_code = RXINI ;
   Exits[1].sysexit_name = "term" ;
   Exits[1].sysexit_code = RXTER ;
   Exits[2].sysexit_name = "say" ;
   Exits[2].sysexit_code = RXSIO ;
   Exits[3].sysexit_code = RXENDLST ;

   Instore[0].strptr =
     "say foo\n\n\nbar='baz'\nexit" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;

   rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxDeregisterExit( "init", NULL ) ;
   RexxDeregisterExit( "term", NULL ) ;
   RexxDeregisterExit( "say", NULL ) ;
}



LONG APIENTRY vars_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   SHVBLOCK shv[10] ;
   int rc ;
   RXSIOSAY_PARM *psiosay;

   iverify( "Say exit", ExNum, RXSIO ) ;
   iverify( "Say exit", Subfun, RXSIOSAY ) ;

   psiosay = (RXSIOSAY_PARM *)PBlock;
   sverify( "Say exit", &(psiosay->rxsio_string), "Hello" ) ;

   shv[0].shvnext = &(shv[1]) ;
   shv[0].shvname.strptr = "BAR" ;
   shv[0].shvname.strlength = strlen( shv[0].shvname.strptr ) ;
   shv[0].shvnamelen = shv[0].shvname.strlength ;
   shv[0].shvvalue.strptr = "B-value-AR" ;
   shv[0].shvvalue.strlength = strlen( shv[0].shvvalue.strptr ) ;
   shv[0].shvvaluelen = shv[0].shvvalue.strlength ;
   shv[0].shvcode = RXSHV_SYSET ;

   /* Try to get it with lower case letters */
   shv[1].shvnext = &(shv[2]) ;
   shv[1].shvname.strptr = "bar " ;
   shv[1].shvname.strlength = strlen( shv[1].shvname.strptr ) ;
   shv[1].shvnamelen = shv[1].shvname.strlength ;
   shv[1].shvvalue.strptr = (char *)RexxAllocateMemory( 64 ) ;
   shv[1].shvvalue.strlength = 64 ;
   shv[1].shvvaluelen = shv[1].shvvalue.strlength ;
   shv[1].shvcode = RXSHV_SYFET ;

   /* then we use capital letters */
   shv[2].shvnext = &(shv[3]) ;
   shv[2].shvname.strptr = "BAR" ;
   shv[2].shvname.strlength = strlen( shv[2].shvname.strptr ) ;
   shv[2].shvnamelen = shv[2].shvname.strlength ;
   shv[2].shvvalue.strptr = (char *)RexxAllocateMemory( 64 ) ;
   shv[2].shvvalue.strlength = 64 ;
   shv[2].shvvaluelen = shv[2].shvvalue.strlength ;
   shv[2].shvcode = RXSHV_SYFET ;

   /* Then we set it to something else */
   shv[3].shvnext = &(shv[4]) ;
   shv[3].shvname.strptr = "BAR" ;
   shv[3].shvname.strlength = strlen( shv[3].shvname.strptr ) ;
   shv[3].shvnamelen = shv[3].shvname.strlength ;
   shv[3].shvvalue.strptr = "new value" ;
   shv[3].shvvalue.strlength = strlen( shv[3].shvvalue.strptr ) ;
   shv[3].shvvaluelen = shv[3].shvvalue.strlength ;
   shv[3].shvcode = RXSHV_SYSET ;

   /* And get it */
   shv[4].shvnext = &(shv[5]) ;
   shv[4].shvname.strptr = "BAR" ;
   shv[4].shvname.strlength = strlen( shv[4].shvname.strptr ) ;
   shv[4].shvnamelen = shv[4].shvname.strlength ;
   shv[4].shvvalue.strptr = (char *)RexxAllocateMemory( 64 ) ;
   shv[4].shvvalue.strlength =  64 ;
   shv[4].shvvaluelen = shv[4].shvvalue.strlength ;
   shv[4].shvcode = RXSHV_SYFET ;

   /* And drop it */
   shv[5].shvnext = &(shv[6]) ;
   shv[5].shvname.strptr = "BAR" ;
   shv[5].shvname.strlength = strlen( shv[5].shvname.strptr ) ;
   shv[5].shvnamelen = shv[5].shvname.strlength ;
   shv[5].shvvalue.strptr = NULL ;
   shv[5].shvvalue.strlength = 0 ;
   shv[5].shvvaluelen = shv[5].shvvalue.strlength ;
   shv[5].shvcode = RXSHV_SYDRO ;

   /* And then we try to get it again */
   shv[6].shvnext = &(shv[7]) ;
   shv[6].shvname.strptr = "BAR" ;
   shv[6].shvname.strlength = strlen( shv[6].shvname.strptr ) ;
   shv[6].shvnamelen = shv[6].shvname.strlength ;
   shv[6].shvvalue.strptr = (char *)RexxAllocateMemory( 64 ) ;
   shv[6].shvvalue.strlength = 64 ;
   shv[6].shvvaluelen = shv[6].shvvalue.strlength ;
   shv[6].shvcode = RXSHV_SYFET ;

   shv[7].shvnext = &(shv[8]) ;
   shv[7].shvname.strptr = "FOO" ;
   shv[7].shvname.strlength = strlen( shv[7].shvname.strptr ) ;
   shv[7].shvnamelen = shv[7].shvname.strlength ;
   shv[7].shvvalue.strptr = "OOPS" ;
   shv[7].shvvalue.strlength = strlen( shv[7].shvvalue.strptr ) ;
   shv[7].shvvaluelen = shv[7].shvvalue.strlength ;
   shv[7].shvcode = RXSHV_SYSET ;

   shv[8].shvnext = &(shv[9]) ;
   shv[8].shvname.strptr = "ABC.FOO" ;
   shv[8].shvname.strlength = strlen( shv[8].shvname.strptr ) ;
   shv[8].shvnamelen = shv[8].shvname.strlength ;
   shv[8].shvvalue.strptr = "hello, there! ... this is a long string" ;
   shv[8].shvvalue.strlength = strlen( shv[8].shvvalue.strptr ) ;
   shv[8].shvvaluelen = shv[8].shvvalue.strlength ;
   shv[8].shvcode = RXSHV_SYSET ;

   shv[9].shvnext = NULL ;
   shv[9].shvname.strptr = "ABC.OOPS" ;
   shv[9].shvname.strlength = strlen( shv[9].shvname.strptr ) ;
   shv[9].shvnamelen = shv[9].shvname.strlength ;
   shv[9].shvvalue.strptr = (char *)RexxAllocateMemory(16) ;
   shv[9].shvvalue.strlength = 16 ;
   shv[9].shvvaluelen = shv[9].shvvalue.strlength ;
   shv[9].shvcode = RXSHV_SYFET ;


   rc = RexxVariablePool( shv ) ;
   iverify( "Term exit", rc, RXSHV_TRUNC | RXSHV_BADN | RXSHV_NEWV ) ;

   iverify( "Term exit1 ", shv[0].shvret, RXSHV_NEWV ) ;
   iverify( "Term exit2 ", shv[1].shvret, RXSHV_BADN ) ;
   iverify( "Term exit3 ", shv[2].shvret, RXSHV_OK ) ;
   iverify( "Term exit4 ", shv[3].shvret, RXSHV_OK ) ;
   iverify( "Term exit5 ", shv[4].shvret, RXSHV_OK ) ;
   iverify( "Term exit6 ", shv[5].shvret, RXSHV_OK ) ;
   iverify( "Term exit7 ", shv[6].shvret, RXSHV_NEWV ) ;
   iverify( "Term exit8 ", shv[7].shvret, RXSHV_NEWV ) ;
   iverify( "Term exit9 ", shv[8].shvret, RXSHV_NEWV ) ;
   iverify( "Term exit10", shv[9].shvret, RXSHV_TRUNC ) ;


   sverify( "Term exit1 ", &(shv[2].shvvalue), "B-value-AR" ) ;
   sverify( "Term exit2 ", &(shv[4].shvvalue), "new value" ) ;
   sverify( "Term exit3 ", &(shv[6].shvvalue), "BAR" ) ;
   sverify( "Term exit4 ", &(shv[9].shvvalue), "hello, there! .." ) ;

   return RXEXIT_HANDLED ;
}

static void vars(void)
{
   RXSTRING Instore[2] ;
   RXSYSEXIT Exits[4] ;
   int rc ;

   message( "vars" ) ;

   RexxRegisterExitExe( "hepp", vars_exit, NULL ) ;

   Exits[0].sysexit_name = "hepp" ;
   Exits[0].sysexit_code = RXSIO ;
   Exits[1].sysexit_code = RXENDLST ;

   Instore[0].strptr =
     "say 'Hello'" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;

   rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxDeregisterExit( "hepp", NULL ) ;
}




LONG APIENTRY source_exit( LONG code, LONG subcode, PEXIT ptr )
{
   SHVBLOCK Req[6] ;

   Req[0].shvnext = &(Req[1]) ;
   Req[0].shvcode = RXSHV_PRIV ;
   Req[0].shvname.strptr = "SOURCE" ;
   Req[0].shvname.strlength = strlen( Req[0].shvname.strptr ) ;
   Req[0].shvnamelen = Req[0].shvname.strlength ;
   Req[0].shvvalue.strptr = NULL ;

   Req[1].shvnext = &(Req[2]) ;
   Req[1].shvcode = RXSHV_PRIV ;
   Req[1].shvname.strptr = "VERSION" ;
   Req[1].shvname.strlength = strlen( Req[1].shvname.strptr ) ;
   Req[1].shvnamelen = Req[1].shvname.strlength ;
   Req[1].shvvalue.strptr = NULL ;

   Req[2].shvnext = &(Req[3]) ;
   Req[2].shvcode = RXSHV_PRIV ;
   Req[2].shvname.strptr = "PARM" ;
   Req[2].shvname.strlength = strlen( Req[2].shvname.strptr ) ;
   Req[2].shvnamelen = Req[2].shvname.strlength ;
   Req[2].shvvalue.strptr = NULL ;

   Req[3].shvnext = &(Req[4]) ;
   Req[3].shvcode = RXSHV_PRIV ;
   Req[3].shvname.strptr = "PARM.1" ;
   Req[3].shvname.strlength = strlen( Req[3].shvname.strptr ) ;
   Req[3].shvnamelen = Req[3].shvname.strlength ;
   Req[3].shvvalue.strptr = NULL ;

   Req[4].shvnext = &(Req[5]) ;
   Req[4].shvcode = RXSHV_PRIV ;
   Req[4].shvname.strptr = "PARM.2" ;
   Req[4].shvname.strlength = strlen( Req[4].shvname.strptr ) ;
   Req[4].shvnamelen = Req[4].shvname.strlength ;
   Req[4].shvvalue.strptr = NULL ;

   Req[5].shvnext = NULL ;
   Req[5].shvcode = RXSHV_PRIV ;
   Req[5].shvname.strptr = "QUENAME" ;
   Req[5].shvname.strlength = strlen( Req[5].shvname.strptr ) ;
   Req[5].shvnamelen = Req[5].shvname.strlength ;
   Req[5].shvvalue.strptr = NULL ;

   iverify( "Termin exit", code, RXTER ) ;
   iverify( "Termin exit", subcode, RXTEREXT ) ;

   RexxVariablePool( Req ) ;

   iverify( "Source", Req[0].shvret, RXSHV_OK ) ;
   iverify( "Version", Req[1].shvret, RXSHV_OK ) ;
   if (memcmp(Req[1].shvvalue.strptr, "REXX", 4 ))
      mycomplain( "Invalid Rexx source string\n" ) ;

   iverify( "Parms", Req[2].shvret, RXSHV_OK ) ;
   sverify( "Parms", &(Req[2].shvvalue), "1" ) ;

   iverify( "Parm1a", Req[3].shvret, RXSHV_OK ) ;
   sverify( "Parm1b", &(Req[3].shvvalue), "one two three" ) ;

   iverify( "Parm2a", Req[4].shvret, RXSHV_OK ) ;
   sverify( "Parm2b", &(Req[4].shvvalue), NULL ) ;

   iverify( "QueName", Req[5].shvret, RXSHV_OK ) ;
   sverify( "QueName", &(Req[5].shvvalue), "SESSION" ) ;

   return RXEXIT_HANDLED ;
}



static void source( void )
{
   RXSYSEXIT Exits[2] ;
   RXSTRING Instore[2] ;
   RXSTRING Params[2] ;
   int rc ;

   message("private") ;

   RexxRegisterExitExe("hei", source_exit, NULL ) ;

   Exits[0].sysexit_name = "hei" ;
   Exits[0].sysexit_code = RXTER ;
   Exits[1].sysexit_code = RXENDLST ;

   Params[0].strptr = "one two three" ;
   Params[0].strlength = strlen( Params[0].strptr ) ;

   Instore[0].strptr = "nop" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;

   rc = RexxStart( 1, Params, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxDeregisterExit( "hei", NULL ) ;
}




LONG APIENTRY parms( LONG code, LONG subcode, PEXIT ptr )
{
   RXSYSEXIT Exits[2] ;
   RXSTRING Instore[2] ;
   RXSTRING Params[5] ;
   int rc ;
   static int done = 0 ;

   if (code == -1)
   {
      message("parms") ;

      RexxRegisterExitExe("hei", parms, NULL ) ;

      Exits[0].sysexit_name = "hei" ;
      Exits[0].sysexit_code = RXTER ;
      Exits[1].sysexit_code = RXENDLST ;

      Params[0].strptr = "one two three" ;
      Params[0].strlength = strlen( Params[0].strptr ) ;
      Params[1].strptr = NULL ;
      Params[1].strlength = 5 ;
      Params[2].strptr = "" ;
      Params[2].strlength = strlen( Params[2].strptr ) ;
      Params[3].strptr = "four five" ;
      Params[3].strlength = strlen( Params[3].strptr ) ;
      Params[4].strptr = NULL ;
      Params[4].strlength = 10 ;

      Instore[0].strptr = "nop" ;
      Instore[0].strlength = strlen( Instore[0].strptr ) ;
      Instore[1].strptr = NULL ;

      rc = RexxStart( 5, Params, "Testing", Instore, "Foo", RXFUNCTION,
                      Exits, NULL, NULL ) ;

      if (!done)
         printf("not done\n") ;

      RexxDeregisterExit( "hei", NULL ) ;
      return 0 ;
   }
   else
   {
      SHVBLOCK Req[7] ;

      done = 1 ;
      Req[0].shvnext = &(Req[1]) ;
      Req[0].shvcode = RXSHV_PRIV ;
      Req[0].shvname.strptr = "PARM" ;
      Req[0].shvname.strlength = strlen( Req[0].shvname.strptr ) ;
      Req[0].shvnamelen = Req[0].shvname.strlength ;
      Req[0].shvvalue.strptr = NULL ;

      Req[1].shvnext = &(Req[2]) ;
      Req[1].shvcode = RXSHV_PRIV ;
      Req[1].shvname.strptr = "PARM.1" ;
      Req[1].shvname.strlength = strlen( Req[1].shvname.strptr ) ;
      Req[1].shvnamelen = Req[1].shvname.strlength ;
      Req[1].shvvalue.strptr = NULL ;

      Req[2].shvnext = &(Req[3]) ;
      Req[2].shvcode = RXSHV_PRIV ;
      Req[2].shvname.strptr = "PARM.2" ;
      Req[2].shvname.strlength = strlen( Req[2].shvname.strptr ) ;
      Req[2].shvnamelen = Req[2].shvname.strlength ;
      Req[2].shvvalue.strptr = NULL ;

      Req[3].shvnext = &(Req[4]) ;
      Req[3].shvcode = RXSHV_PRIV ;
      Req[3].shvname.strptr = "PARM.3" ;
      Req[3].shvname.strlength = strlen( Req[3].shvname.strptr ) ;
      Req[3].shvnamelen = Req[3].shvname.strlength ;
      Req[3].shvvalue.strptr = NULL ;

      Req[4].shvnext = &(Req[5]) ;
      Req[4].shvcode = RXSHV_PRIV ;
      Req[4].shvname.strptr = "PARM.4" ;
      Req[4].shvname.strlength = strlen( Req[4].shvname.strptr ) ;
      Req[4].shvnamelen = Req[4].shvname.strlength ;
      Req[4].shvvalue.strptr = NULL ;

      Req[5].shvnext = &(Req[6]) ;
      Req[5].shvcode = RXSHV_PRIV ;
      Req[5].shvname.strptr = "PARM.5" ;
      Req[5].shvname.strlength = strlen( Req[5].shvname.strptr ) ;
      Req[5].shvnamelen = Req[5].shvname.strlength ;
      Req[5].shvvalue.strptr = NULL ;

      Req[6].shvnext = NULL ;
      Req[6].shvcode = RXSHV_PRIV ;
      Req[6].shvname.strptr = "PARM.6" ;
      Req[6].shvname.strlength = strlen( Req[6].shvname.strptr ) ;
      Req[6].shvnamelen = Req[6].shvname.strlength ;
      Req[6].shvvalue.strptr = NULL ;

      iverify( "Termin exit", code, RXTER ) ;
      iverify( "Termin exit", subcode, RXTEREXT ) ;

      RexxVariablePool( Req ) ;

      iverify( "Parm", Req[0].shvret, RXSHV_OK ) ;
      sverify( "Parm", &(Req[0].shvvalue), "5" ) ;

      iverify( "Parm1", Req[1].shvret, RXSHV_OK ) ;
      sverify( "Parm1", &(Req[1].shvvalue), "one two three" ) ;

      iverify( "Parm2", Req[2].shvret, RXSHV_OK ) ;
      sverify( "Parm2", &(Req[2].shvvalue), NULL ) ;

      iverify( "Parm3", Req[3].shvret, RXSHV_OK ) ;
      sverify( "Parm3", &(Req[3].shvvalue), "" ) ;

      iverify( "Parm4", Req[4].shvret, RXSHV_OK ) ;
      sverify( "Parm4", &(Req[4].shvvalue), "four five" ) ;

      iverify( "Parm5", Req[5].shvret, RXSHV_OK ) ;
      sverify( "Parm5", &(Req[5].shvvalue), NULL ) ;

      iverify( "Parm6", Req[6].shvret, RXSHV_OK ) ;
      sverify( "Parm6", &(Req[6].shvvalue), NULL ) ;

      return RXEXIT_HANDLED ;
   }
}





LONG APIENTRY allvars( LONG code, LONG subcode, PEXIT ptr )
{
   RXSYSEXIT Exits[2] ;
   RXSTRING Instore[2] ;
   RXSTRING Params[5] ;
   int rc ;
   static int done = 0 ;

   if (code == -1)
   {
      message("allvars") ;

      RexxRegisterExitExe("hei", allvars, NULL ) ;

      Exits[0].sysexit_name = "hei" ;
      Exits[0].sysexit_code = RXTER ;
      Exits[1].sysexit_code = RXENDLST ;

      Params[0].strptr = "one two three" ;
      Params[0].strlength = strlen( Params[0].strptr ) ;
      Params[1].strptr = NULL ;
      Params[1].strlength = 5 ;
      Params[2].strptr = "" ;
      Params[2].strlength = strlen( Params[2].strptr ) ;
      Params[3].strptr = "four five" ;
      Params[3].strlength = strlen( Params[3].strptr ) ;
      Params[4].strptr = NULL ;
      Params[4].strlength = 10 ;

      Instore[0].strptr = "foo='one';bar='';foo.='two';foo.bar='three';"
                          "foo.4='four';foo.5='five';drop foo.5 foo.6" ;

      Instore[0].strlength = strlen( Instore[0].strptr ) ;
      Instore[1].strptr = NULL ;

      rc = RexxStart( 5, Params, "Testing", Instore, "Foo", RXFUNCTION,
                      Exits, NULL, NULL ) ;

      if (!done)
         printf("not done\n") ;

      RexxDeregisterExit( "hei", NULL ) ;
      return 0 ;
   }
   else
   {
      SHVBLOCK Req[10] ;
      int i ;

      done = 1 ;
      for (i=0; i<10; i++)
      {
          Req[i].shvnext = (i<10-1) ? (&(Req[i+1])) : NULL ;
          Req[i].shvcode = RXSHV_NEXTV ;
          Req[i].shvname.strptr = Req[i].shvvalue.strptr = NULL ;
      }

      iverify( "Termin exit", code, RXTER ) ;
      iverify( "Termin exit", subcode, RXTEREXT ) ;

      RexxVariablePool( Req ) ;

      iverify( "Parm0", Req[0].shvret, RXSHV_OK ) ;
      sverify( "Parm0a", &(Req[0].shvname), "BAR" ) ;
      sverify( "Parm0b", &(Req[0].shvvalue), "" ) ;

      iverify( "Parm1", Req[1].shvret, RXSHV_OK ) ;
      sverify( "Parm1a", &(Req[1].shvname), "FOO." ) ;
      sverify( "Parm1b", &(Req[1].shvvalue), "three" ) ;

      iverify( "Parm2", Req[2].shvret, RXSHV_OK ) ;
      sverify( "Parm2a", &(Req[2].shvname), "FOO.4" ) ;
      sverify( "Parm2b", &(Req[2].shvvalue), "four" ) ;

      iverify( "Parm3", Req[3].shvret, RXSHV_OK ) ;
      sverify( "Parm3a", &(Req[3].shvname), "FOO.5" ) ;
      sverify( "Parm3b", &(Req[3].shvvalue), "FOO.5" ) ;

      iverify( "Parm4", Req[4].shvret, RXSHV_OK ) ;
      sverify( "Parm4a", &(Req[4].shvname), "FOO.6" ) ;
      sverify( "Parm4b", &(Req[4].shvvalue), "FOO.6" ) ;

      iverify( "Parm5", Req[5].shvret, RXSHV_OK ) ;
      sverify( "Parm5a", &(Req[5].shvname), "FOO." ) ;
      sverify( "Parm5b", &(Req[5].shvvalue), "two" ) ;

      iverify( "Parm6", Req[6].shvret, RXSHV_OK ) ;
      sverify( "Parm6a", &(Req[6].shvname), "FOO" ) ;
      sverify( "Parm6b", &(Req[6].shvvalue), "one" ) ;

      iverify( "Parm", Req[7].shvret, RXSHV_LVAR ) ;
      iverify( "Parm", Req[8].shvret, RXSHV_OK ) ;
      iverify( "Parm", Req[9].shvret, RXSHV_OK ) ;

      return RXEXIT_HANDLED ;
   }
}






ULONG APIENTRY extfunc( PCSZ name, ULONG params, RXSTRING *parm, PCSZ stck, RXSTRING *ptr )
{
   RXSTRING Instore[2] ;
   RXSTRING Result ;
   int rc ;
   short nret ;
   static int done = 0 ;

   if (name == NULL)
   {
      message("extfunc") ;

      RexxRegisterFunctionExe("EXTFUNC", extfunc ) ;

      Instore[0].strptr = "return extfunc('asdf','qwer',,'zxcv') + 'EXTFUNC'('asdf',,'wer','werrew')" ;
      Instore[0].strlength = strlen( Instore[0].strptr ) ;
      Instore[1].strptr = NULL ;

      Result.strlength = 10 ;
      Result.strptr = (char *)RexxAllocateMemory( 10 ) ;

      rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXFUNCTION,
                      NULL, &nret, &Result ) ;

      sverify( "extfunc", &Result, "7" ) ;
      iverify( "extfunc", nret, 7 ) ;

      if (!done)
         printf("not done\n") ;

      RexxDeregisterFunction( "EXTFUNC" ) ;
      return 0 ;
   }
   else
   {
      static int once=0 ;

      done = 1 ;
      if (!once)
      {
         ptr->strptr[0] = '3' ;
         ptr->strlength = 1 ;
         once = 1 ;
      }
      else
      {
         ptr->strptr[0] = '4' ;
         ptr->strlength = 1 ;
      }
      return 0 ;
   }
}

LONG APIENTRY subcom_exit( LONG ExNum, LONG Subfun, PEXIT PBlock )
{
   RXCMDHST_PARM *pcmdhst;

   iverify( "Subcom Funcion", ExNum, RXCMD ) ;
   iverify( "Subcom Subfunc", Subfun, RXCMDHST ) ;
   pcmdhst = (RXCMDHST_PARM *)PBlock;
   pcmdhst->rxcmd_retc.strlength = 2;
   strcpy(pcmdhst->rxcmd_retc.strptr,"16");

   return RXEXIT_HANDLED ;
}

ULONG APIENTRY run_subcom(PRXSTRING Command, PUSHORT Flags, PRXSTRING Retstr)
{
   RXSYSEXIT Exits[4] ;
   RXSTRING Instore[2] ;
   RXSTRING Result ;
   int rc ;
   short nret ;

   if (Command == NULL)
   {
      message("subcom") ;

      RexxRegisterExitExe( "slhepp", subcom_exit, NULL ) ;

      Exits[0].sysexit_name = "slhepp" ;
      Exits[0].sysexit_code = RXCMD ;
      Exits[1].sysexit_code = RXENDLST ;

      RexxRegisterSubcomExe("SUBCOM", run_subcom, NULL ) ;

      Instore[0].strptr = "'foobar asdf qwer'" ;
      Instore[0].strlength = strlen( Instore[0].strptr ) ;
      Instore[1].strptr = NULL ;

      Result.strlength = 20 ;
      Result.strptr = (char *)RexxAllocateMemory( 20 ) ;

      rc = RexxStart( 0, NULL, "Testing", Instore, "SUBCOM", RXCOMMAND,
                      Exits, &nret, &Result ) ;

/*      sverify( "subcom result", &Result, "16" ) ; */
/*      iverify( "subcom retc", nret, 16 ) ; */

      RexxDeregisterSubcom( "SUBCOM" , NULL) ;
      RexxDeregisterExit( "slhepp", NULL ) ;

      return 0 ;
   }
   else
   {
      sprintf(Retstr->strptr,"%ld",Command->strlength) ;
      Retstr->strlength = strlen(Retstr->strptr);
      *Flags = RXSUBCOM_OK;
      return 0 ;
   }

}

static void qtest( void )
{
   RXSTRING Instore[2] ;
   RXSYSEXIT Exits[3] ;
   int rc ;
   char result[100];
   ULONG dup;

   message( "queue" ) ;

   rc = RexxCreateQueue( result, sizeof(result), NULL, &dup ) ;
   iverify( "qtest-before", rc, RXQUEUE_OK ) ;
   iverify( "qtest-before", dup, 0L ) ;

   Exits[0].sysexit_name = "query_init" ;
   Exits[0].sysexit_code = RXINI ;
   Exits[1].sysexit_name = "query_term" ;
   Exits[1].sysexit_code = RXTER ;
   Exits[2].sysexit_code = RXENDLST ;
   RexxRegisterExitExe( "query_init", query_init_exit, NULL ) ;
   RexxRegisterExitExe( "query_term", query_term_exit, NULL ) ;

   Instore[0].strptr = "call rxqueue 'Set', 'test1';"
                       "if queued() \\= 2 then say 'error:line 2';"
                       "parse pull line;"
                       "if line \\= 'line1' then say 'error:line 4';"
                       "parse pull line;"
                       "if line \\= 'line2' then say 'error:line 5';"
                       "if rxqueue('Create','FRED') \\= 'FRED' then say 'error:line 6';"
                       "if rxqueue('Set','FRED') \\= 'TEST1' then say 'error:line 7';"
                       "push '1line';push '2line';" ;
   Instore[0].strlength = strlen( Instore[0].strptr ) ;
   Instore[1].strptr = NULL ;
   rc = RexxStart( 0, NULL, "Testing", Instore, "Foo", RXCOMMAND,
                   Exits, NULL, NULL ) ;

   RexxFreeMemory( Instore[1].strptr ) ;
   RexxDeregisterExit( "query_init", NULL ) ;
   RexxDeregisterExit( "query_term", NULL ) ;

   rc = RexxDeleteQueue( result );
   iverify( "qtest-after", rc, RXQUEUE_OK ) ;
}

static void (*(routines[]))(void) = {
    instore,
    trace,
    intertrc,
    pull,
    env,
    init_term,
    vars,
    source,
    NULL
} ;


static RexxExitHandler *(exits[]) = {
   parms,
   allvars,
   NULL
} ;


static RexxFunctionHandler *(efuncs[]) = {
   extfunc,
   NULL
} ;

static RexxSubcomHandler *(scfuncs[]) = {
   run_subcom,
   NULL
} ;

char char82( void )
{
   return (char) 0x82;
}

char char0( void )
{
   return '\0';
}

int main( int argc, char *argv[] )
{
   void (**fptr)( void ) ;
   RexxExitHandler **eptr ;
   RexxFunctionHandler **gptr ;
   RexxSubcomHandler **sptr;
   RXSTRING version;
   ULONG versioncode;

   /* We want to see surprisings at once: */
   setvbuf(stdout,NULL,_IONBF,0);
   setvbuf(stderr,NULL,_IONBF,0);
   printf( "Regina Rexx API Tester\n" );
   printf( "----------------------\n" );

   version.strlength = 0;
   version.strptr = NULL;
   /* This might not work if we check another Rexx: */
   versioncode = ReginaVersion(&version);
   printf("Regina's version is %lu.%lu",
          versioncode >> 8,
          versioncode & 0xFF);
   if (version.strptr)
   {
      printf(" (in complete \"%s\")",version.strptr);
      RexxFreeMemory(version.strptr);
   }
   printf("\n");

   if ( char82() < char0() )
      printf( "WARNING: Current compiler uses `signed char' as default!\n" );

   for( fptr=routines; *fptr; fptr++ )
      (*fptr)() ;

   for( eptr=exits; *eptr; eptr++ )
      (*eptr)( -1, -1, NULL ) ;

   for( gptr=efuncs; *gptr; gptr++ )
      (*gptr)( NULL, 0, NULL, NULL, NULL ) ;

   for( sptr=scfuncs; *sptr; sptr++ )
      (*sptr)( NULL, 0, NULL ) ;
#if !defined(NO_EXTERNAL_QUEUES)
   qtest();
#endif
   ReginaCleanup();
   printf( "\n" ) ;
   return 0 ;
}
