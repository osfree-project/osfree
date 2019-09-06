/*
 * Reads stdin and writes it to stdout or stderr depending on parameters.
 * 1) "line" or "char"
 *    "line" based, stdin is split into lines
 *    "char" based, treat as binary blob - default
 * 2) "stderr" or not supplied
 *    "stderr", if "line" based every second line written to stderr (others written to stdout)
 *              if "char" based all chars written to stderr
 *    if not supplied everthing written to stdout - default
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_BUF 20000000

void writeline( FILE *fp, char *buf, int line_based, int use_stderr, int *to_stdout )
{
   FILE *fp1;
   char *where;
   if ( use_stderr )
   {
      if ( *to_stdout )
      {
         fp1 = stdout;
         where = "stdout";
         *to_stdout = 0;
      }
      else
      {
         fp1 = stderr;
         where = "stderr";
         *to_stdout = 1;
      }
   }
   else
   {
      fp1 = stdout;
      where = "stdout";
      *to_stdout = 0;
   }
   fprintf( fp1, "%s\n", buf );
// FGC: Speedup:   fprintf(fp,"write to %s use_stderr %d to_stdout %d buf[%s]\n",where, use_stderr, *to_stdout,buf);
}

int main( int argc, char *argv[] )
{
   char ch[2];
   char *buf;
   char *start;
   size_t rc;
   int count = 0, i=0;
   int use_stderr = 0;
   int to_stdout = 1, skip;
   int line_based = 0;
   long max_buf = 1000;
   FILE *fp;

#ifdef SIGPIPE
   signal( SIGPIPE, SIG_IGN );
#endif

   fp = fopen( "filter.log", "a" );
   if ( argc > 1 )
   {
      max_buf = atol( argv[1] );
      if ( max_buf == -1 )
      {
         fprintf( fp,"Invalid argument 1: %s. Should be a valid number\n", argv[1] );
         fclose(fp);
         exit( 1 );
      }
   }
   if ( argc > 2 )
   {
      if ( strcmp( argv[2], "line" ) == 0 )
         line_based = 1;
      else if ( strcmp( argv[2], "char" ) == 0 )
         line_based = 0;
      else
      {
         fprintf( fp,"Invalid argument 2: %s. Should be \"line\" or \"char\"\n", argv[2] );
         fclose(fp);
         exit( 1 );
      }
   }
   if ( argc > 3 )
   {
      if ( strcmp( argv[3], "stderr" ) == 0 )
         use_stderr = 1;
      else
      {
         fprintf( fp,"Invalid argument 3: %s. Should be \"stderr\" or not supplied\n", argv[3] );
         fclose(fp);
         exit( 1);
      }
   }
   fprintf( fp,"\nfilter starting. Args:");
   for ( i = 1; i < argc; i++ )
   {
      fprintf( fp," %s", argv[i]);
   }
   fprintf( fp,"\n");
#ifdef _MSC_VER
   _setmode(_fileno(stdin), _O_BINARY);
   _setmode(_fileno(stdout), _O_BINARY);
   _setmode(_fileno(stderr), _O_BINARY);
#endif
   buf = malloc( max_buf+1 );
   if ( buf == NULL )
   {
      fprintf( fp,"Unable to allocate: %d bytes. Aborting\n", max_buf );
      fclose(fp);
      exit( 1 );
   }
   skip = 0;
   for( i = 0;;)
   {
      rc = fread( &ch, sizeof(char), 1, stdin );
      if ( rc == EOF)
      {
         fprintf( fp,"filter: End read with EOF char.\n");
         break;
      }
      if ( feof( stdin ) )
      {
         fprintf( fp,"filter: End read with feof().\n");
         break;
      }
      count++;
      if ( count == max_buf )
      {
         fprintf( fp,"Buffer size of %d exceeded. Aborting\n", max_buf );
         fclose(fp);
         exit( 1 );
      }
      if ( skip == 0 )
      {
         if ( line_based )
         {

#ifdef _MSC_VER
            if ( ch[0] == 0x0d )
            {
               skip = 1;
#else
            if ( ch[0] == 0x0a )
            {
               skip = 0;
#endif
               buf[i] = '\0';
               writeline( fp, buf, line_based, use_stderr, &to_stdout );
               i = 0;
            }
            else
            {
               buf[i] = ch[0];
               i++;
            }
         }
         else
         {
            buf[i] = ch[0];
            i++;
         }
      }
      else
         skip = 0;
   }
   if ( line_based == 0 )
   {
      if ( use_stderr )
         fwrite( buf, sizeof(char), i, stderr );
      else
         fwrite( buf, sizeof(char), i, stdout );
   }
   else
   {
      buf[i] = '\0';
      if ( strlen( buf ) != 0 )
         writeline( fp, buf, line_based, use_stderr, &to_stdout );
   }
   fprintf( fp,"filter: Read %d chars. Could have read %d chars.\n",count,max_buf);
   fclose(fp);
   exit(0);
}
