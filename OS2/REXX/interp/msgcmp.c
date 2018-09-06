/*
 * Compile text message files into binary files
 *
 * Expects any number of files specified on the command line
 * and outputs an equivalent number of binary files with the
 * same name (with .mtb extension) in the local directory.
 *
 * Format of input files:
 * xxx,yyy,ttttttttttttttttttttttttt
 * (type),pppppppppppppppppp
 *
 * where xxx is errno
 *       yyy is suberrno
 *       ttttt is error message text
 * and   (type) is error prefix type
 *       ppppp is error prefix string
 *
 * For each file:
 *  get file size
 *  allocate this much memory for text strings
 *  allocate 1000 structs for
 *  read all lines into memory counting number of rows and putting
 *   text lines into memory chunk, and mesage lengths into struct
 *   array, and offsets (0 being start of memory chunk)
 *  write out number of entries
 *  iterate through struct arrays, adding file offset of start of
 *   text messages based on n number of structs
 *  write out array of structs
 *  write out memory chunck of error texts
 */

/*
 * Bug in LCC complier wchar.h that incorrectly says it defines stat struct
 * but doesn't
 */
#if defined(__LCC__)
# include <sys/stat.h>
#endif

#include "rexx.h"

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include "rexxmsg.h"

/*
 * Static pointers to language-specific error messages
 * IF THIS EVER CHANGES, ALSO CHANGE THE SAME TABLE IN error.c
 */
static const char *errlang[] =
{
   "en", /* english */
   "de", /* german */
   "es", /* spanish */
   "no", /* norwegian */
   "pt", /* portuguese */
   "pl", /* polish */
   "sv", /* swedish */
   "tr", /* turkish */
#if 0
   "en","ca","cs","da","de","el","es","fi","fr","he","hu","is","it","ja","ko",
   "lt","nl","no","pl","pt","ru","sk","sl","sv","th","zh",
#endif
} ;

unsigned int getlanguage( char *lang )
{
   int i,size;

   size = sizeof(errlang) / sizeof(errlang[0]);
   for ( i = 0; i < size; i++)
   {
      if ( strcmp( errlang[i], lang ) == 0 )
         return i;
   }
   return 255;
}

int main( int argc, char *argv[] )
{
   struct stat statbuf;
   FILE *infp, *outfp;
   char line[512];
   int i,j,len,end=0,start=0,text_size;
   char *fn;
   char *text;
   struct textindex *pti,*ti;
   unsigned int count,prefix_count,total_count,offset,lang;
   int last_count=0,last_prefix_count=0;

   ti = (struct textindex *)malloc( 500*sizeof(struct textindex) );
   if ( ti == NULL )
   {
      fprintf( stderr, "Unable to allocate memory for 500 message structures.\n" );
      exit(1);
   }
   for ( i = 1; i < argc; i++ )
   {
      fn = argv[i];
      infp = fopen( fn, "r" );
      if ( infp == NULL )
      {
         fprintf( stderr, "Unable to open %s for reading.\n", fn );
         free(ti);
         exit(1);
      }
      stat( fn, &statbuf );
      text = (char *)malloc( statbuf.st_size );
      if ( text == NULL )
      {
         fprintf( stderr, "Unable to allocate %d bytes of memory.\n", (int) statbuf.st_size );
         free(ti);
         exit(1);
      }
      /*
       * Read each line from the file...
       */
      text_size = 0;
      pti = ti;
      for ( count = 0, prefix_count = 0; ; )
      {
         if ( fgets( line, 511, infp ) == NULL )
            break;
         if ( line[0] == '#' )
         ;
         else if ( line[0] == 'P' )
         {
            line[5] = '\0';
            pti->errorno = 999;
            pti->suberrorno = atol( line+2 );
            pti->fileoffset = text_size;
            pti->textlength = strlen( line+6)-1;
            memcpy( text+text_size, line+6, pti->textlength);
            text_size += pti->textlength;
            prefix_count++;
            pti++;
         }
         else
         {
            line[3] = '\0';
            line[7] = '\0';
            pti->errorno = atol( line );
            pti->suberrorno = atol( line+4 );
            pti->fileoffset = text_size;
            pti->textlength = strlen( line+8)-1;
            memcpy( text+text_size, line+8, pti->textlength);
            text_size += pti->textlength;
            count++;
            pti++;
         }
      }
      fclose( infp );
      if ( last_count != 0 && count != (unsigned) last_count )
      {
         fprintf( stderr, "Inconsistent numbers of error messages (%d) between this file %s and the previous one (%d).\n", count, fn, last_count );
         free(ti);
         exit(1);
      }
      if ( last_prefix_count != 0 && prefix_count != (unsigned) last_prefix_count )
      {
         fprintf( stderr, "Inconsistent numbers of prefix messages (%d) between this file %s and the previous one (%d).\n", prefix_count, fn, last_prefix_count );
         free(ti);
         exit(1);
      }
      last_count = count;
      last_prefix_count = prefix_count;
      total_count = count + prefix_count;
      /* generate output file name */
      len = strlen( argv[i] );
      for ( j = len-1; j >= 0; j-- )
      {
         if ( argv[i][j] == '.' )
            end = j;
         if ( argv[i][j] == '/' || argv[i][j] == '\\')
         {
            start = j;
            break;
         }
      }
      memcpy( line, fn+start+1, end-start );
      line[end-start-1] = '\0';
      if ( ( lang = getlanguage( line ) ) == 255 )
      {
         fprintf( stderr, "Unknown language file name %s.\n", line );
         free(ti);
         exit(1);
      }
      strcat( line, ".mtb" );
      outfp = fopen( line, "wb" );
      if ( outfp == NULL )
      {
         fprintf( stderr, "Unable to open %s for writing.\n", line );
         free(ti);
         exit(1);
      }
      if ( fwrite( (void *)&count, sizeof(unsigned int), 1, outfp ) != 1 )
      {
         fprintf( stderr, "Unable to write message count to %s.\n", line );
         free(ti);
         exit(1);
      }
      if ( fwrite( (void *)&lang, sizeof(unsigned int), 1, outfp ) != 1 )
      {
         fprintf( stderr, "Unable to write message lang to %s.\n", line );
         free(ti);
         exit(1);
      }
      if ( fwrite( (void *)&prefix_count, sizeof(unsigned int), 1, outfp ) != 1 )
      {
         fprintf( stderr, "Unable to prefix message count to %s.\n", line );
         free(ti);
         exit(1);
      }
      /* update the file offsets */
      offset = total_count * sizeof( struct textindex ) + ( 3 * sizeof( unsigned int ) );
      pti = ti;
      for ( j = 0; (unsigned) j < total_count; j++ )
      {
         pti->fileoffset += offset;
         pti++;
      }
      /* write the index structs */
      pti = ti;
      for ( j = 0; (unsigned) j < total_count; j++ )
      {
         if ( fwrite( (void *)pti, sizeof(struct textindex), 1, outfp ) != 1 )
         {
            fprintf( stderr, "Unable to write index struct %d to %s.\n", j, line );
            free(ti);
            exit(1);
         }
         pti++;
      }
      /* write the text messages */
      if ( (int) fwrite( text, sizeof(char), text_size, outfp ) != text_size )
      {
         fprintf( stderr, "Unable to write text messages to %s.\n", line );
         free(ti);
         exit(1);
      }
      fclose( outfp );
      free( text );
   }
   free(ti);
   printf( "%d error messages and %d prefixes compiled\n", last_count, prefix_count );
   return 0;
}
