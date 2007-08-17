#include <string.h>
#include <malloc.h>
#include "regexp.h"

#define  MAX_SUBST_STRING  4095

typedef struct subst_string
{
  char    s[MAX_SUBST_STRING+1];
  int     len;

} subst_string;


static void
subst_string_init( subst_string*  string,
                   const char*    source )
{
  int  len = 0;

  if (source)
  {
    len = strlen(source);
    if (len > MAX_SUBST_STRING)
      len = MAX_SUBST_STRING;

    if (len > 0)
      memcpy( string->s, source, len );
  }
  string->s[len] = 0;
  string->len    = len;
}


static void
subst_string_add( subst_string*  string,
                  const char*    source,
                  int            src_len )
{
  int  delta;

  delta = string->len + src_len - MAX_SUBST_STRING;
  if (delta > 0)
    src_len -= delta;

  if ( src_len > 0 )
  {
    memcpy( string->s + string->len, source, src_len );
    string->len += src_len;
    string->s[ string->len ] = 0;
  }
}


static void
subst_string_add_char( subst_string*  string,
                       const char     c )
{
  int  len = string->len;
  if ( len < MAX_SUBST_STRING )
  {
    string->s[len++] = c;
    string->s[len]   = 0;
    string->len = len;
  }
}


static void
subst_string_setsub( subst_string*  string,
                     int            offset,
                     int            len,
                     const char*    replacement )
{
  subst_string  temp;
  int           end;

  subst_string_init( &temp, 0 );

  subst_string_add( &temp, string->s, offset );

  if (replacement)
    subst_string_add( &temp, replacement, strlen(replacement) );

  offset += len;
  subst_string_add( &temp, string->s + offset, string->len - offset );

  *string = temp;
}



void substitute( const char*  source,
                 const char*  pattern,
                 const char*  replacement,
                 char*        target )
{
  regexp*  repat;

  target[0] = '\0';

  repat = regcomp( (char*)pattern );
  if (repat)
  {
    if ( regexec( repat, (char*)source ) )
    {
      subst_string  targ;

      /* a match was found */
      if ( !strchr( replacement, '$' ) )
      {
        /* fast, simple replacement */
        subst_string_init( &targ, source );
        subst_string_setsub( &targ, repat->startp[0] - source,
                             repat->endp[0] - repat->startp[0],
                             replacement );
      }
      else
      {
        /* perform sub-expression replacement */
        subst_string  temp;
        const char*   src = replacement;
        char          c;

        subst_string_init( &targ, 0 );
        subst_string_init( &temp, 0 );

        for (;;)
        {
          c = *src++;
          if (!c) break;

          if ( c == '$' )
          {
            if ( *src == '&' || (*src - '0') <= 9 )
            {
              int  n = (*src++ - '0');

              if ( (unsigned int)n > 9)
                n = 0;

              subst_string_add( &temp, repat->startp[n],
                                repat->endp[n] - repat->startp[n] );

              continue;
            }
          }

          /* ordinary character */
          if ( c == '\\' && ( *src == '\\' || *src == '$' ) )
            c = *src++;

          subst_string_add_char( &temp, c );
        }

        subst_string_setsub( &targ, repat->startp[0] - source,
                             repat->endp[0] - repat->startp[0],
                             temp.s );
      }

      /* now copy the content of "targ" to the target */
      strcpy( target, targ.s );
    }
    else
    {
      /* no match, simply copy source into target */
      strcpy( target, source );
    }
    free( repat );
  }
}

