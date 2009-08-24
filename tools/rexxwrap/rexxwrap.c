/*
 * Copyright (C) 1999-2000  Mark Hessling <M.Hessling@qut.edu.au>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Mark Hessling  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 *
 */
#if defined(HAVE_CONFIG_H)
# include "config.h"
#else
# include "defines.h"
#endif

#include <stdio.h>
#if defined(HAVE_STDLIB_H)
# include <stdlib.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
# include <sys/types.h>
#endif
#if defined(HAVE_SYS_STAT_H)
# include <sys/stat.h>
#endif
#if defined(HAVE_MALLOC_H)
# include <malloc.h>
#endif
#if defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif
#if defined(HAVE_STRING_H)
# include <string.h>
#endif
#if defined(HAVE_PROCESS_H)
# include <process.h>
#endif

#if defined(_MSC_VER)
# include <fcntl.h>
# include <io.h>
#endif

#if defined(__MINGW32__) && defined(USE_REGINA)
# define __REGINA_DLL 1
#endif

#include "rxdefines.h"

#if defined(USE_ZLIB)
# include "zlib.h"
#endif
#if defined(USE_DES)
# include "des.h"
#endif

int gotOutput = 0;
/*
 * The program is first compressed, then encrypted, then uuencoded
 */
typedef struct
{
   int len;
   char *line;
} PROGLINE;

PROGLINE progline[] =
{
%%ENCODED_PROGRAM%%
};

%%ORIGINAL_LENGTH%%
%%COMPRESSED_LENGTH%%
%%ENCRYPTED_LENGTH%%
%%ENCODED_LENGTH%%
%%PROGRAM_NAME%%

extern char *RexxWrapper_GetArgv0( char * );

#if defined(RUN_AS_GUI) && defined(WITH_GUI_CONSOLE)
# include <windows.h>
# define STARTUPCONSOLE() StartupConsole()
# define CLOSEDOWNCONSOLE() ClosedownConsole()
void StartupConsole( void )
{
   if ( gotOutput == 0 )
   {
      AllocConsole();
      freopen( "conin$", "r", stdin );
      freopen( "conout$", "w", stdout );
      freopen( "conout$", "w", stderr );
      gotOutput = 1;
   }
}
void ClosedownConsole( void )
{
   if ( gotOutput )
   {
      fprintf( stderr, "\n==> Press ENTER key to close this window. <==" );
      getchar();
      fclose(stdin);
      fclose(stdout);
      fclose(stderr);
      gotOutput = 0;
   }
}

REH_RETURN_TYPE RxExitHandlerForSayTraceRedirection

#if defined(HAVE_PROTO)
   ( REH_ARG0_TYPE ExitNumber, REH_ARG1_TYPE Subfunction, REH_ARG2_TYPE ParmBlock )
#else
   ( ExitNumber, Subfunction, ParmBlock )
   REH_ARG0_TYPE ExitNumber;    /* code defining the exit function    */
   REH_ARG1_TYPE Subfunction;   /* code defining the exit subfunction */
   REH_ARG2_TYPE ParmBlock;     /* function dependent control block   */
#endif
{
   long i = 0;
   int rc = 0;

   StartupConsole();

   switch( Subfunction )
   {
      case RXSIOSAY:
      {
         RXSIOSAY_PARM *say_parm = (RXSIOSAY_PARM *)ParmBlock;
         if ( say_parm->rxsio_string.strptr != NULL )
         {
            for( i = 0; i < (long)say_parm->rxsio_string.strlength; i++ )
            {
               fputc( ( char )say_parm->rxsio_string.strptr[i], stdout );
            }
         }
         fputc( '\n', stdout );
         rc = RXEXIT_HANDLED;
         break;
      }
      case RXSIOTRC:
      {
         RXSIOTRC_PARM *trc_parm = (RXSIOTRC_PARM *)ParmBlock;
         if ( trc_parm->rxsio_string.strptr != NULL )
         {
            for( i = 0; i < (long)trc_parm->rxsio_string.strlength; i++ )
            {
               fputc( ( char )trc_parm->rxsio_string.strptr[i], stderr );
            }
         }
         fputc( '\n', stderr );
         rc = RXEXIT_HANDLED;
         break;
      }
      case RXSIOTRD:
      {
         RXSIOTRD_PARM *trd_parm = (RXSIOTRD_PARM *)ParmBlock;
         int i = 0, ch = 0;
         do
         {
            if ( i < 256 )
               trd_parm->rxsiotrd_retc.strptr[i++] = ch = getc( stdin ) ;
         } while( ch != '\012' && (ch != EOF ) ) ;
         trd_parm->rxsiotrd_retc.strlength = i - 1;
         rc = RXEXIT_HANDLED;
         break;
      }
      case RXSIODTR:
      {
         RXSIODTR_PARM *dtr_parm = (RXSIODTR_PARM *)ParmBlock;
         int i = 0, ch = 0;
         do
         {
            if ( i < 256 )
               dtr_parm->rxsiodtr_retc.strptr[i++] = ch = getc( stdin ) ;
         } while( ch != '\012' && (ch != EOF ) ) ;
         dtr_parm->rxsiodtr_retc.strlength = i - 1;
         rc = RXEXIT_HANDLED;
         break;
      }
      default:
         rc = RXEXIT_NOT_HANDLED;
         break;
   }
   return rc;
}
#else
# define STARTUPCONSOLE()
# define CLOSEDOWNCONSOLE()
#endif

int decode(unsigned char *out, unsigned char *in, int len)
{
   unsigned int four[4];
   unsigned char tmp1,tmp2;
   int i,j;

   for (i=0,j=0;i<len;i+=4,j+=3)
   {
      four[0] = (int)in[i]-32;
      four[1] = (int)in[i+1]-32;
      four[2] = (int)in[i+2]-32;
      four[3] = (int)in[i+3]-32;

      tmp1 = (unsigned char)((four[0]<<2) & 0xfc);
      tmp2 = (unsigned char)((four[1]>>4) & 0x03);
      out[j] = (tmp1 + tmp2);

      tmp1 = (unsigned char)(((four[1] & 0x0f)<<4) & 0xf0);
      tmp2 = (unsigned char)((four[2] & 0xfc)>>2);
      out[j+1] = (tmp1 + tmp2);
      tmp1 = (unsigned char)(((four[2] & 0x03)<<6) & 0xf0);
      out[j+2] = (unsigned char)((int)tmp1 + ((int)four[3]  & (int)0x3f));
   }
   return j;
}

/*
 * Expand the program.
 * UUdecode, then decrypt, then uncompress
 */
unsigned char *expand_program( char *str, int *len )
{
   unsigned char *encoded_code=NULL;
   unsigned char *encrypted_code=NULL;
   unsigned char *compressed_code=NULL;
   unsigned char *original_code=NULL;
   int i,tot;
#if defined(USE_ZLIB)
   int rc;
   long new_original_length=0;
#endif
#if defined(USE_DES)
   des_cblock deskey;
   des_cblock in_cblock;
   des_cblock out_cblock;
   des_key_schedule ks;
   int num_chunks;
#endif
   /*
    * decoded to encrypted
    */
   encoded_code = (unsigned char *)malloc(encoded_length+10);
   if (encoded_code == NULL)
   {
      STARTUPCONSOLE();
      fprintf( stderr, "No memory at line %d\n", __LINE__ );
      return NULL;
   }
   encrypted_code = (unsigned char *)malloc(encrypted_length+10);
   if (encrypted_code == NULL)
   {
      STARTUPCONSOLE();
      fprintf( stderr, "No memory at line %d\n", __LINE__ );
      return NULL;
   }
   for (i=0,tot=0;progline[i].len != 0;i++)
   {
      memcpy(encoded_code+tot,progline[i].line,progline[i].len);
      tot += progline[i].len;
   }
   decode(encrypted_code,encoded_code,encoded_length);
   free(encoded_code);
#if defined(USE_DES)
   /*
    * Convert the DES key into a DES key schedule
    */
   des_string_to_key(str,&deskey);
   des_set_key((des_cblock *)&deskey,ks);
   /*
    * Decrypt encrypted_code to produce compressed_code
    */
   compressed_code = (unsigned char *)malloc(compressed_length+10);
   if (compressed_code == NULL)
   {
      STARTUPCONSOLE();
      fprintf( stderr, "No memory at line %d\n", __LINE__ );
      free(encrypted_code);
      return NULL;
   }
   /*
    * Now decrypt encrypted_code in 8byte chunks
    */
   num_chunks = encrypted_length / 8; /* this should ALWAYS be a multiple of 8 */
   for (i=0;i<num_chunks;i++)
   {
      memcpy(in_cblock,encrypted_code+(i*8),8);
      des_ecb_encrypt(&in_cblock,&out_cblock,ks,DES_DECRYPT);
      memcpy(compressed_code+(i*8),out_cblock,8);
   }
   free(encrypted_code);
#else
   compressed_code = encrypted_code;
   compressed_length = encrypted_length;
   str = str; /* keep compiler happy */
#endif

#if defined(USE_ZLIB)
   /*
    * compressed to original
    */
   original_code = (unsigned char *)malloc(original_length+10);
   if (original_code == NULL)
   {
      STARTUPCONSOLE();
      fprintf( stderr, "No memory at line %d\n", __LINE__ );
      return NULL;
   }
   new_original_length = original_length;
   rc = uncompress(original_code, (uLongf *)&new_original_length,
                 compressed_code, compressed_length);
   if (rc != Z_OK)
   {
      STARTUPCONSOLE();
      fprintf( stderr, "Error (%d) decompressing program code.\n", rc );
      free(original_code);
      free(compressed_code);
      return NULL;
   }
   if ( new_original_length != original_length)
   {
      STARTUPCONSOLE();
      fprintf( stderr, "Error decompressing program code. Original length: %ld Decompressed length: %ld\n", original_length, new_original_length );
      free(original_code);
      free(compressed_code);
      return NULL;
   }
   free(compressed_code);
#else
   original_code = compressed_code;
   original_length = compressed_length;
#endif
   original_code[original_length] = '\0';
   *len = original_length;
   return original_code;
}

/*-----------------------------------------------------------------------------
 * Processing starts here
 *----------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
{
   long i=0, ArgCount=0;
   RXSTRING retstr;
   CHAR retbuf[RXAUTOBUFLEN];

#if defined(REXXWRAPPER_SUBROUTINE)
   RXSTRING *Arguments;
#else
   RXSTRING ArgList;
#endif
   RXSTRING instore[2];
#if defined(RUN_AS_GUI) && defined(WITH_GUI_CONSOLE)
   RXSYSEXIT ExitList[2];
#endif
%%ORIGINAL_KEY%%
%%DEFAULT_ENVIRONMENT%%
   unsigned char *prog;
   int prog_len;
   short rc;
   int rcode=0;
   FILE *dumpfp;
   char *argv0;

   /*
    * First check if we are trying to display the embedded Rexx program
    */
   if (argc == 2
   &&  strlen( argv[1] ) > 6
   &&  memcmp(argv[1],"--key=",6) == 0)
   {
      if (strlen(argv[1]) != 6+original_key_length)
      {
         STARTUPCONSOLE();
         fprintf( stderr, "Key supplied is not the correct length - must be %d characters\n", original_key_length );
         CLOSEDOWNCONSOLE();
         exit(1);
      }
      if (memcmp(original_key,argv[1]+6,original_key_length) != 0)
      {
         STARTUPCONSOLE();
         fprintf( stderr, "Key supplied is not the correct key\n" );
         CLOSEDOWNCONSOLE();
         exit(1);
      }

      prog = expand_program(original_key,&prog_len);
      if (prog)
      {
#if defined(RUN_AS_GUI) && defined(WITH_GUI_CONSOLE)
         sprintf( retbuf, "%s.out", ProgramName );
         dumpfp = fopen( retbuf, "wb" );
         if ( dumpfp == NULL )
         {
            STARTUPCONSOLE();
            fprintf( stderr, "Unable to open file: %s to dump program contents\n", ProgramName );
            CLOSEDOWNCONSOLE();
            exit(1);
         }
#else
         dumpfp = stdout;
#endif
         fprintf( dumpfp, "--------------- Original program follows ---------------\n" );
#if defined(_MSC_VER)
         _setmode( fileno( dumpfp ), _O_BINARY );
#endif
         for ( i=0; i<original_length; i++ )
         {
            fputc( prog[i], dumpfp );
#if defined(_MSC_VER)
            if ( i % 1000 == 999 )
            {
               fflush( dumpfp );
               _setmode( fileno( dumpfp ), _O_BINARY );
            }
#endif
         }
#if defined(_MSC_VER)
         fflush( dumpfp );
         _setmode( fileno( dumpfp ), _O_TEXT );
#endif
         fprintf( dumpfp, "--------------- End of Original program ----------------\n" );
         fflush( dumpfp );
#if defined(RUN_AS_GUI) && defined(WITH_GUI_CONSOLE)
         fclose( dumpfp );
         STARTUPCONSOLE();
         fprintf( stderr, "Original program dumped to file: %s\n", retbuf );
         CLOSEDOWNCONSOLE();
#endif
         rc = 0;
      }
      else
      {
         rc = 1;
      }
      CLOSEDOWNCONSOLE();
      exit(rc);
   }
#if defined(RUN_AS_GUI) && defined(WITH_GUI_CONSOLE)
   /*
    * Register the system exit
    */
   RexxRegisterExitExe( ( RREE_ARG0_TYPE )ProgramName,
                          ( RREE_ARG1_TYPE )RxExitHandlerForSayTraceRedirection,
                          ( RREE_ARG2_TYPE )NULL);
   ExitList[0].sysexit_name = ProgramName;
   ExitList[0].sysexit_code = RXSIO;
   ExitList[1].sysexit_code = RXENDLST;
#endif

   /*
    * Get number of arguments to the Rexx program
    */
   ArgCount = argc - 1;

   /*
    * Build an array of arguments if any.
    */
#if defined(REXXWRAPPER_SUBROUTINE)
   if ( ArgCount > 0 )
   {
      int len=0;

      if ( ( Arguments = (RXSTRING *)malloc( ArgCount * sizeof( RXSTRING ) ) ) == (RXSTRING *)NULL )
      {
         STARTUPCONSOLE();
         (void)fprintf( stderr, "%s: out of memory\n", argv[0] );
         CLOSEDOWNCONSOLE();
         exit( 1 );
      }
      for ( i = 1; i < argc; i++ )
      {
         MAKERXSTRING( Arguments[i-1], argv[i], strlen( argv[i] ) );
      }
   }
   else
   {
      Arguments = NULL;
   }
#else
   if ( ArgCount > 0 )
   {
      int len=0;

      for ( i = 1; i < argc; i++ )
      {
         len += strlen( (char *)argv[i] );
      }
      if ( ( ArgList.strptr = (RXSTRING_STRPTR_TYPE)malloc( len + 1 + ArgCount) ) == (RXSTRING_STRPTR_TYPE)NULL )
      {
         STARTUPCONSOLE();
         (void)fprintf( stderr, "%s: out of memory\n", argv[0] );
         CLOSEDOWNCONSOLE();
         exit( 1 );
      }
      strcpy( ArgList.strptr, "" );
      for ( i = 1; i < argc; i++ )
      {
         strcat( ArgList.strptr, (RXSTRING_STRPTR_TYPE)argv[i] );
         if ( i != argc )
            strcat( ArgList.strptr, (RXSTRING_STRPTR_TYPE)" " );
      }
      ArgList.strlength = ArgCount + len - 1;
   }
   else
   {
      ArgList.strptr = NULL;
      ArgList.strlength = 0;
   }
#endif
   /*
    * Find the full path to ourselves if possible
    */
   argv0 = RexxWrapper_GetArgv0( argv[0] );
   if ( argv0 == NULL )
      argv0 = ProgramName;

   prog = expand_program(original_key,&prog_len);
   if ( prog )
   {
#if defined(USE_TOKENIZED)
      instore[0].strptr = (RXSTRING_STRPTR_TYPE)NULL;
      instore[1].strptr = (RXSTRING_STRPTR_TYPE)prog;
      instore[1].strlength = prog_len;
#else
      instore[0].strptr = (RXSTRING_STRPTR_TYPE)prog;
      instore[0].strlength = prog_len;
      instore[1].strptr = (RXSTRING_STRPTR_TYPE)NULL;
#endif

      MAKERXSTRING( retstr, retbuf, sizeof( retbuf ) );
      /*
       * Execute the Rexx script. Use RXCOMMAND mode so that the Rexx program
       * expects the same parameter list if called directly via the Rexx
       * interpreter, or override this to run as a subroutine if required.
       */
#if defined(REXXWRAPPER_SUBROUTINE)
      rcode = RexxStart( ( RS_ARG0_TYPE )(ArgCount),
                         ( RS_ARG1_TYPE )Arguments,
                         ( RS_ARG2_TYPE )argv0,
                         ( RS_ARG3_TYPE )instore,
                         ( RS_ARG4_TYPE )DefaultEnvironment,
                         ( RS_ARG5_TYPE )RXSUBROUTINE,
#if defined(RUN_AS_GUI) && defined(WITH_GUI_CONSOLE)
                         ( RS_ARG6_TYPE )ExitList,
#else
                         ( RS_ARG6_TYPE )NULL,
#endif
                         ( RS_ARG7_TYPE )&rc,
                         ( RS_ARG8_TYPE )&retstr);
#else
      rcode = RexxStart( ( RS_ARG0_TYPE )(ArgCount) ? 1 : 0,
                         ( RS_ARG1_TYPE )&ArgList,
                         ( RS_ARG2_TYPE )argv0,
                         ( RS_ARG3_TYPE )instore,
                         ( RS_ARG4_TYPE )DefaultEnvironment,
                         ( RS_ARG5_TYPE )RXCOMMAND,
#if defined(RUN_AS_GUI) && defined(WITH_GUI_CONSOLE)
                         ( RS_ARG6_TYPE )ExitList,
#else
                         ( RS_ARG6_TYPE )NULL,
#endif
                         ( RS_ARG7_TYPE )&rc,
                         ( RS_ARG8_TYPE )&retstr);
#endif

      free ( prog );
   }
   else
   {
      rc = 1;
   }
#if defined(REXXWRAPPER_SUBROUTINE)
   if ( Arguments )
      free( Arguments );
#else
   if ( ArgList.strptr )
      free( ArgList.strptr );
#endif
   /*
    * Return the exit value from the program. This is useful for UNIX/DOS etc.
    * if the value is kept to 0-success, small positive numbers (say < 100)
    * to indicate errors!
    */
   if ( rcode != 0 )
      fprintf( stderr, "Error from Rexx interpreter: %d\n", rcode );
   CLOSEDOWNCONSOLE();
   return (int)rc;
}
/*
 * Following code allows the program to be built as a Win32 Console app
 * or as a Win32 Windows app.  The idea and code is based on similar code
 * in FLTK.
 */
#if defined(WIN32) && !defined(__GNUC__)
# if defined( BORLAND5 )
#  define __argc _argc
#  define __argv _argv
# endif

# if defined( __LCC__)
extern int __argc;
extern char *__argv[];
# endif

# if defined(RUN_AS_GUI)
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
   int rc;
   rc = main( __argc, __argv );
   return rc;
}
# endif
#endif
