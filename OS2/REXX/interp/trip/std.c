#include <stdio.h>
#include <signal.h>

int main( int argc, char **argv )
{
   int i, max = 0;
   int errerr = 0;
   int errout = 0;

#ifdef SIGPIPE
   signal( SIGPIPE, SIG_IGN );
#endif

   if ( argc == 2 )
   {
      char c;

      if ( sscanf( argv[1], "%d%c", &max, &c ) != 1 )
         max = 0;
   }

   if ( max == 0 )
   {
      fprintf( stderr, "Line to stderr\n" );
      fprintf( stdout, "Line to stdout\n" );
   }
   else
   {
      for ( i = 0; i < max; i++ )
      {
         errerr = ( errerr ) ? errerr : ferror( stderr );
         errout = ( errout ) ? errout : ferror( stdout );

         if ( !errerr )
            fprintf( stderr, "Line %d to stderr\n", i + 1 );
         if ( !errout )
            fprintf( stdout, "Line %d to stdout\n", i + 1 );
      }
   }

   return 0;
}
