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

#define NO_CTYPE_REPLACEMENT
#include "rexx.h"
#include <locale.h>
#include <ctype.h>
/* Do _not_ use stddef, anders! _not_ stddef! Remember that!!!
   (it breaks on suns running gcc without fixincludes */
#include <stdio.h>
#include <limits.h>
#if defined(HAVE_ASSERT_H)
# include <assert.h>
#endif

#if defined(TIME_WITH_SYS_TIME)
# include <sys/time.h>
# include <time.h>
#else
# if defined(HAVE_SYS_TIME_H)
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if defined(_AMIGA)
const char *Version="$VER: "PARSE_VERSION_STRING" "__AMIGADATE__" $";
# if defined(__SASC)
static const long __stack = 0x6000;
# endif
#endif

/*
 * The idea is to initialize the character operation table only once.
 * Everything is cached until the end of the process.
 * We need the uppercase/lowercase conversion table and we need a table
 * for the state information of each character.
 *
 * One great benefit is a homogeneous world of characters where an
 * attribute "is an uppercase letter" persists between different library
 * calls. If one has ever tried these stupid HP printer drivers for NT,
 * he/she will like this feature.
 */

PROTECTION_VAR( locale_info )
static char *locale_lc_ctype = NULL;
unsigned char_info[257] = {0, }; /*
                                  * Last char indicates what attributes
                                  * have been loaded.
                                  */


unsigned char u_to_l[256] = { /* initially the identity */
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
   0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
   0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
   0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
   0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
   0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
   0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
   0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
   0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
   0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
   0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
   0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
   0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
   0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
   0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
   0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
   0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
   0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
   0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
   0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
   0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
   0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
   0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
   0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
   0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
   0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
   0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
   0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

unsigned char l_to_u[256] = { /* initially the identity */
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
   0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
   0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
   0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
   0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
   0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
   0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
   0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
   0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
   0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
   0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
   0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
   0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
   0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
   0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
   0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
   0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
   0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
   0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
   0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
   0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
   0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
   0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
   0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
   0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
   0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
   0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
   0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

/*
 * This function must be called at the very start of the program. It sets
 * the used character locale. Valid values are the empty string or an OS
 * known value.
 */
void set_locale_info( const char *info )
{
   setlocale( LC_CTYPE, info );
#ifdef LC_MESSAGES
   setlocale( LC_MESSAGES, info );
#endif
}

/*
 * This function loads one piece of character information. The information
 * is stored system wide, and the information will persist over a fork() call.
 */
static void load_info( unsigned infobit )
{
#define AUTO_LOAD( name, rxbit ) { for ( i = 0; i < 256; i++ ) { \
                                      if ( name( i ) )           \
                                         char_info[i] |= rxbit;  \
                                   }                             \
                                 }
   char *current_locale;
   int i;

   THREAD_PROTECT( locale_info )

   /*
    * The desired information may have been collected during the
    * THREAD_PROTECT step by another thread. Test it.
    */
   if ( ( char_info[256] & infobit ) == 0 )
   {
      if ( locale_lc_ctype == NULL )
      {
         /*
          * Get the current locale for every following usage.
          */
         if ( ( current_locale = setlocale( LC_CTYPE, NULL ) ) == NULL )
            current_locale = "C";
         if ( ( current_locale = strdup( current_locale ) ) == NULL )
            current_locale = "C";

         locale_lc_ctype = current_locale;
         setlocale( LC_CTYPE, current_locale );
      }
      current_locale = setlocale( LC_CTYPE, NULL );

      /*
       * The standard locale is loaded. Now collect the infos.
       */
      switch ( infobit )
      {
         case RX_ISLOWER:
            AUTO_LOAD( islower, infobit );
            for ( i = 0; i < 256; i++ )
            {
               if ( char_info[i] & RX_ISLOWER )
                  u_to_l[(unsigned char) toupper( i )] = (unsigned char) i;
            }
            break;

         case RX_ISUPPER:
            AUTO_LOAD( isupper, infobit );
            for ( i = 0; i < 256; i++ )
            {
               if ( char_info[i] & RX_ISUPPER )
                  l_to_u[(unsigned char) tolower( i )] = (unsigned char) i;
            }
            break;

         case RX_ISALPHA:
            AUTO_LOAD( isalpha, infobit );
            break;

         case RX_ISALNUM:
            AUTO_LOAD( isalnum, infobit );
            break;

         case RX_ISDIGIT:
            AUTO_LOAD( isdigit, infobit );
            break;

         case RX_ISXDIGIT:
            AUTO_LOAD( isxdigit, infobit );
            break;

         case RX_ISPUNCT:
            AUTO_LOAD( ispunct, infobit );
            break;

         case RX_ISSPACE:
            AUTO_LOAD( isspace, infobit );
            break;

         case RX_ISPRINT:
            AUTO_LOAD( isprint, infobit );
            break;

         case RX_ISGRAPH:
            AUTO_LOAD( isgraph, infobit );
            break;

         case RX_ISCNTRL:
            AUTO_LOAD( iscntrl, infobit );
            break;

      }

      setlocale( LC_CTYPE, current_locale );
      char_info[256] |= infobit;
   }

   THREAD_UNPROTECT( locale_info )
#undef AUTO_LOAD
}

#define Is_expand( c, bit ) if ( ! ( char_info[256] & bit ) )               \
                               load_info( bit );                            \
                            return char_info[(unsigned char) c] & bit
int Islower( int c )
{
   Is_expand( c, RX_ISLOWER );
}

int Isupper( int c )
{
   Is_expand( c, RX_ISUPPER );
}

int Isalpha( int c )
{
   Is_expand( c, RX_ISALPHA );
}

int Isalnum( int c )
{
   Is_expand( c, RX_ISALNUM );
}

int Isdigit( int c )
{
   Is_expand( c, RX_ISDIGIT );
}

int Isxdigit( int c )
{
   Is_expand( c, RX_ISXDIGIT );
}

int Ispunct( int c )
{
   Is_expand( c, RX_ISPUNCT );
}

int Isspace( int c )
{
   Is_expand( c, RX_ISSPACE );
}

int Isprint( int c )
{
   Is_expand( c, RX_ISPRINT );
}

int Isgraph( int c )
{
   Is_expand( c, RX_ISGRAPH );
}

int Iscntrl( int c )
{
   Is_expand( c, RX_ISCNTRL );
}
#undef Is_expand

int Toupper( int c )
{
   if ( ! ( char_info[256] & RX_ISUPPER ) )
      load_info( RX_ISUPPER );
   return l_to_u[ (unsigned char) c ];
}

int Tolower( int c )
{
   if ( ! ( char_info[256] & RX_ISLOWER ) )
      load_info( RX_ISLOWER );
   return u_to_l[ (unsigned char) c ];
}

void mem_upper( void *m, int length )
{
   unsigned char *c = (unsigned char *) m;

   if ( ! ( char_info[256] & RX_ISUPPER ) )
      load_info( RX_ISUPPER );

   while ( length-- > 0 )
   {
      *c = l_to_u[ *c ];
      c++;
   }
}

void mem_lower( void *m, int length )
{
   unsigned char *c = (unsigned char *) m;

   if ( ! ( char_info[256] & RX_ISLOWER ) )
      load_info( RX_ISLOWER );

   while ( length-- > 0 )
   {
      *c = u_to_l[ *c ];
      c++;
   }
}

int mem_cmpic( const void *buf1, const void *buf2, int len )
/*
 * Function  : Compares two memory buffers for equality;
 *             case insensitive. Same as memicmp() Microsoft C.
 * Parameters: buf1     - first buffer
 *             buf2     - second buffer
 *             len      - number of characters to compare.
 * Return    : <0 if buf1 < buf2
 *             =0 if buf1 = buf2
 *             >0 if buf1 > buf2
 */
{
   const unsigned char *b1 = (const unsigned char *) buf1;
   const unsigned char *b2 = (const unsigned char *) buf2;
   unsigned char c1,c2;

   if ( ! ( char_info[256] & RX_ISLOWER ) )
      load_info( RX_ISLOWER );

   while ( len-- > 0 )
   {
      c1 = u_to_l[ *b1 ];
      c2 = u_to_l[ *b2 ];
      if ( c1 != c2 )
         return( (int) c1 ) - ( (int) c2 );
      b1++;
      b2++;
   }
   return 0;
}

void getsecs( time_t *secs, time_t *usecs )
{
#if defined(HAVE_GETTIMEOFDAY)
   struct timeval times ;

   gettimeofday(&times, NULL) ;
   *secs = times.tv_sec ;
   *usecs = times.tv_usec ;

   if (times.tv_usec < 0)
   {
      *usecs = (times.tv_usec + 1000000) ;
      *secs = times.tv_sec - 1 ;
   }
   assert( *secs>=0 && *usecs>=0 ) ;

#elif defined(HAVE_FTIME)
   struct timeb timebuffer;

   ftime(&timebuffer);
   *secs = timebuffer.time;
   *usecs = timebuffer.millitm * 1000;
   assert( *secs>=0 && *usecs>=0 ) ;
#else
   *secs = time(NULL) ;
   *usecs = 0 ;
   assert( *secs>=0 && *usecs>=0 ) ;
#endif
}


const char *system_type( void )
{
#if defined(VMS)
   return "VMS" ;
#elif defined(OS2)
   return "OS/2" ;
#elif defined(DOS)
# if defined(__EMX__)
   if (_osmode == DOS_MODE)
      return("DOS");
   else
      return("OS/2");
#else
   return("DOS");
# endif
#elif defined(SKYOS)
   return "SKYOS" ;
#elif defined(__CYGWIN__)
   return "UNIX" ;
#elif defined(WIN64) || defined(_WIN64)
   return "WIN64" ;
#elif defined(WIN32) || defined(_WIN32)
   return "WIN32" ;
#elif defined(_AMIGA) || defined(AMIGA)
   return "AMIGA" ;
#elif defined(__QNX__)
   return "QNX" ;
#elif defined(__BEOS__)
   return "BEOS" ;
#elif defined(__HAIKU__)
   return "HAIKU" ;
#elif defined(__WINS__)
   return "EPOC32-WINS" ;
#elif defined(__EPOC32__)
   return "EPOC32-MARM" ;
#elif defined(__APPLE__) && defined(__MACH__)
# include <TargetConditionals.h>
# if TARGET_IPHONE_SIMULATOR == 1
   return "iOS (Simulator)" ;
# elif TARGET_OS_IPHONE == 1
   return "iOS" ;
# elif TARGET_OS_MAC == 1
   return "UNIX" ;
# endif
#else
   return "UNIX" ;
#endif
}


#if !defined(HAVE_STRERROR)
/*
 * Sigh! This must probably be done this way, although it's incredibly
 * backwards. Some versions of gcc comes with a complete set of ANSI C
 * include files, which contains the definition of strerror(). However,
 * that function does not exist in the default libraries of SunOS.
 * To circumvent that problem, strerror() is #define'd to get_sys_errlist()
 * in config.h, and here follows the definition of that function.
 * Originally, strerror() was #defined to sys_errlist[x], but that does
 * not work if string.h contains a declaration of the (non-existing)
 * function strerror().
 *
 * So, this is a mismatch between the include files and the library, and
 * it should not create problems for Regina. However, the _user_ will not
 * encounter any problems until he compiles Regina, so we'll have to
 * clean up after a buggy installation of the C compiler!
 */
const char *get_sys_errlist( int num )
{
   extern char *sys_errlist[] ;
   return sys_errlist[num] ;
}
#endif


double cpu_time( void )
{
#ifndef CLOCKS_PER_SEC
/*
 * Lots of systems don't seem to get this ANSI C piece of code correctly
 * but most of them seems to use one million ...  Using a million for
 * those systems that haven't defined CLOCKS_PER_SEC may give an incorrect
 * value if clock() does not return microseconds!
 */
# define CLOCKS_PER_SEC 1000000
#endif
   return ((double)(clock()))/((double)(CLOCKS_PER_SEC)) ;
}

/* HIGHBIT is an unsigned with the highest bit set */
#define HIGHBIT (((unsigned) 1) << ((sizeof(unsigned) * CHAR_BIT) - 1))

/* hashvalue computes a value for hashing from a string content. Use
 * hashvalue_ic for a case insensitive version.
 * length may less than 0. The string length is computed by strlen() in this
 * case.
 * The return value might be modified by the %-operator for real hash
 * values.
 */
unsigned hashvalue(const char *string, int length)
{
   unsigned retval = 0, wrap ;
   const unsigned char *ptr = (const unsigned char *) string; /* unsigned char makes it fast */
   unsigned char c;

   if (length < 0)
      length = strlen(string);

   while (length--) {
      c = *ptr++; /* Yes this is slower but believe in your optimizer! */
      retval ^= c;
      wrap = (retval & HIGHBIT) ? 1 : 0;
      retval <<= 1;
      retval |= wrap;
   }

   return(retval);
}

/* hashvalue_ic computes a value for hashing from a string content. Use
 * hashvalue for a case significant version. This is case insensitive.
 * length may less than 0. The string length is computed by strlen() in this
 * case.
 * The return value might be modified by the %-operator for real hash
 * values.
 */
unsigned hashvalue_ic(const char *string, int length)
{
   unsigned retval = 0, wrap ;
   const unsigned char *ptr = (const unsigned char *) string; /* unsigned char makes it fast */
   unsigned char c;

   if ( length < 0 )
      length = strlen( string );

   if ( ! ( char_info[256] & RX_ISLOWER ) )
      load_info( RX_ISLOWER );

   while ( length-- ) {
      c = u_to_l[ *ptr ];
      ptr++;
      retval ^= c;
      wrap = (retval & HIGHBIT) ? 1 : 0;
      retval <<= 1;
      retval |= wrap;
   }

   return(retval);
}

/*
 * hashvalue_var computes a value for hashing from a variable name.
 * The computing starts at the start'th character in name and stops either
 * at the end of the string or at the next dot. The later one only happens
 * if stop != NULL. In this case, the dot's position is returned in *stop.
 * Note: This is one of the most time-consuming routines. Be careful.
 */
unsigned hashvalue_var( const streng *name, int start, int *stop )
{
   unsigned sum, idx;
   const char *ch1, *ech0;

   if ( ( char_info[256] & ( RX_ISLOWER | RX_ISDIGIT ) ) !=
                                                 ( RX_ISLOWER | RX_ISDIGIT ) )
   {
      /*
       * The above conditional is the fastest check. Now do the slow
       * individual check. It speeds up all things.
       */
      if ( ! ( char_info[256] & RX_ISLOWER ) )
         load_info( RX_ISLOWER );
      if ( ! ( char_info[256] & RX_ISDIGIT ) )
         load_info( RX_ISDIGIT );
   }

   ch1 = name->value;
   ech0 = Str_end( name );

   ch1 += start;
   sum = idx = 0;
   for (; ch1 < ech0; ch1++ )
   {
      if ( *ch1 == '.' )
      {
         if ( stop )
            break;
         else
            continue;
      }
      if ( char_info[(unsigned char) *ch1] & RX_ISDIGIT )
         idx = idx * 10 + (unsigned) ( *ch1 - '0' );
      else
      {
         if ( idx )
         {
            sum += u_to_l[ (unsigned char) *ch1 ] + idx;
            idx = 0;
         }
         else
            sum += u_to_l[ (unsigned char) *ch1 ];
      }
   }

   if ( stop )
      *stop = ch1 - name->value;

   return sum + idx;
}

/*
 * Because this modules defines the helper functions for rx_isspace we
 * don't have access to this defined macro. Just re-define it locally.
 */
#define is_expand( c, bit, func ) ( ( char_info[256] & bit ) ?               \
                        ( char_info[(unsigned char) c] & bit ) :  func( c ) )
#define rx_isspace( c )  is_expand( c, RX_ISSPACE , Isspace  )

/*
 * nextarg parses source for the next argument in unix shell terms. If target
 * is given, it must consist of enough free characters to hold the result +
 * one byte for the terminator. If len != NULL it will become the length of
 * the string (which might not been return if target == NULL). The return value
 * is either NULL or a new start value for nextarg.
 * escape is the current escape value which should be used, must be set.
 */
static const char *nextarg(const char *source, unsigned *len, char *target,
                                                                  char escape)
{
   unsigned l;
   char c, term;

   if (len != NULL)
      *len = 0;
   if (target != NULL)
      *target = '\0';
   l = 0;  /* cached length */

   if (source == NULL)
      return NULL;

   while (rx_isspace(*source)) /* jump over initial spaces */
      source++;
   if (*source == '\0')
      return NULL;

   do {
      /* There's something to return. Check for delimiters */
      term = *source++;

      if ((term == '\'') || (term == '\"'))
      {
         while ((c = *source++) != term) {
            if (c == escape)
               c = *source++;
            if (c == '\0')  /* stray \ at EOS is equiv to normal EOS */
            {
               /* empty string is valid! */
               if (len != NULL)
                  *len = l;
               if (target != NULL)
                  *target = '\0';
               return source - 1; /* next try returns NULL */
            }
            l++;
            if (target != NULL)
               *target++ = c;
         }
      }
      else /* whitespace delimiters */
      {
         c = term;
         while (!rx_isspace(c) && (c != '\'') && (c != '\"')) {
            if (c == escape)
               c = *source++;
            if (c == '\0')  /* stray \ at EOS is equiv to normal EOS */
            {
               /* at least a stray \ was found, empty string checked in
                * the very beginning.
                */
               if (len != NULL)
                  *len = l;
               if (target != NULL)
                  *target = '\0';
               return source - 1; /* next try returns NULL */
            }
            l++;
            if (target != NULL)
               *target++ = c;
            c = *source++;
         }
         source--; /* undo the "wrong" character */
      }
   } while (!rx_isspace(*source));

   if (len != NULL)
      *len = l;
   if (target != NULL)
      *target = '\0';
   return source;
}

/*
 * makeargs chops string into arguments and returns an array of x+1 strings if
 * string contains x args. The last argument is NULL. This function usually is
 * called from the subprocess if fork/exec is used.
 * Example: "xx y" -> { "xx", "y", NULL }
 * escape must be the escape character of the command line and is usually ^
 * or \
 */
char **makeargs(const char *string, char escape)
{
   char **retval;
   const char *p;
   int i, argc = 0;
   unsigned size;

   p = string; /* count the number of strings */
   while ((p = nextarg(p, NULL, NULL, escape)) != NULL)
      argc++;
   if ((retval = (char **)malloc((argc + 1) * sizeof(char *))) == NULL)
      return(NULL);

   p = string; /* count each string length */
   for (i = 0; i < argc; i++)
   {
      p = nextarg(p, &size, NULL, escape);
      if ((retval[i] = (char *)malloc(size + 1)) == NULL)
      {
         i--;
         while (i >= 0)
            free(retval[i--]);
         free(retval);
         return(NULL);
      }
   }

   p = string; /* assign each string */
   for (i = 0; i < argc; i++)
      p = nextarg(p, NULL, retval[i], escape);
   retval[argc] = NULL;

   return(retval);
}

/* splitoffarg chops string into two different pieces: The first argument and
 * all other (uninterpreted) arguments. The first argument is returned in a
 * freshly allocated string. The rest is a pointer somewhere within string
 * and returned in *trailer. The return value is allocated by malloc().
 * Example: "xx y" -> returns "xx", *trailer == "xx y"+2
 * escape must be the escape character of the command line and is usually ^
 * or \
 */
char *splitoffarg(const char *string, const char **trailer, char escape)
{
   unsigned size;
   char *retval;
   const char *t;

   if (trailer != NULL)
      *trailer = ""; /* just a default */
   nextarg(string, &size, NULL, escape);
   if ((retval = malloc(size + 1)) == NULL) /* don't change to internal allocation routine */
      return(NULL);

   t = nextarg(string, NULL, retval, escape);
   if (trailer != NULL)
      *trailer = t;
   return(retval);
}

/*
 * nextarg parses source for the next argument as a simple word. If target
 * is given, it must consist of enough free characters to hold the result +
 * one byte for the terminator. If len != NULL it will become the length of
 * the string (which might not been return if target == NULL). The return value
 * is either NULL or a new start value for nextarg.
 */
static const char *nextsimplearg(const char *source, unsigned *len,
                                                                  char *target)
{
   unsigned l;
   char c;

   if (len != NULL)
      *len = 0;
   if (target != NULL)
      *target = '\0';
   l = 0;  /* cached length */

   if (source == NULL)
      return(NULL);

   while (rx_isspace(*source)) /* jump over initial spaces */
      source++;
   if (*source == '\0')
      return(NULL);

   c = *source++;

   while (!rx_isspace(c))
   {
      if (c == '\0')  /* stray \ at EOS is equiv to normal EOS */
      {
         /* something's found, therefore we don't have to return NULL */
         if (len != NULL)
            *len = l;
         if (target != NULL)
            *target = '\0';
         return(source - 1); /* next try returns NULL */
      }
      l++;
      if (target != NULL)
         *target++ = c;
      c = *source++;
   }
   source--; /* undo the "wrong" character */

   if (len != NULL)
      *len = l;
   if (target != NULL)
      *target = '\0';
   return(source);
}

/*
 * makesimpleargs chops string into arguments and returns an array of x+1
 * strings if string contains x args. The last argument is NULL. This function
 * usually is called from the subprocess if fork/exec is used.
 * Example: "xx y" -> { "xx", "y", NULL }
 */
char **makesimpleargs(const char *string)
{
   char **retval;
   const char *p;
   int i, argc = 0;
   unsigned size;

   p = string; /* count the number of strings */
   while ((p = nextsimplearg(p, NULL, NULL)) != NULL)
      argc++;
   if ((retval = malloc((argc + 1) * sizeof(char *))) == NULL)
      return(NULL);

   p = string; /* count each string length */
   for (i = 0; i < argc; i++)
   {
      p = nextsimplearg(p, &size, NULL);
      if ((retval[i] = malloc(size + 1)) == NULL)
      {
         i--;
         while (i >= 0)
            free(retval[i--]);
         free(retval);
         return(NULL);
      }
   }

   p = string; /* assign each string */
   for (i = 0; i < argc; i++)
      p = nextsimplearg(p, NULL, retval[i]);

   return(retval);
}

/*
 * destroyargs destroys the array created by makeargs
 */
void destroyargs(char **args)
{
   char **run = args;

   while (*run) {
      free(*run);
      run++;
   }
   free(args);
}
