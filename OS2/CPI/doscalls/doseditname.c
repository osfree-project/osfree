/*!

     @file doseditname.c

     @brief Implements DosEditName API. Based on FreeCOM wildcard functions.

        Substitute wildcards in filenames

        fillFnam():
        Substitutes the wildcards of the last filename component.

        fillComp():
        replaces the wildcards ? and * of a filename pattern with characters
        from a source filename (useful in COPY a*.* b?1.*).
        If a question mark appears bejond the end of the file name, it
        is silently ignored, e.g. in above example if copy'ing A.TXT)

 */

#define INCL_DOSFILEMGR
#include <os2.h>

void myfnsplit( const char *path,
                      char *drv,
                      char *dir,
                      char *name,
                      char *ext )
{
    const char* end;
    const char* p;
    const char* s;

    if( path[ 0 ] && path[ 1 ] == ':' ) {
        if( drv ) {
            *drv++ = *path++;
            *drv++ = *path++;
            *drv = '\0';
        }
    } else if( drv ) *drv = '\0';

    for( end = path; *end && *end != ':'; ) end++;

    for( p = end; p > path && *--p != '\\'/* && *p != '/'*/; )
        if( *p == '.' ) {
            end = p;
            break;
        }
    if ( ext ) for( s = end; ( *ext = *s++ ) != '\0'; ) ext++;

    for( p = end; p > path; )
        if( *--p == '\\'/* || *p == '/'*/) {
        /*
         * '/' can't happen as a path seperator in FreeCOM because it's treated
         * as a switch character no matter where it's found
         */
            p++;
            break;
        }

    if( name ) {
        for( s = p; s < end; ) *name++ = *s++;
        *name = '\0';
    }

    if ( dir ) {
        for( s = path; s < p; ) *dir++ = *s++;
        *dir = '\0';
    }
}

void myfnmerge( char *path, const char *drive, const char *dir,
                            const char *fname, const char *ext )
{
    if( *drive ) {
        strcpy( path, drive );
    } else ( *path ) = 0;

    if( *dir ) {
        strcat( path, dir );
        if( *( dir + strlen( dir ) - 1 ) != '\\' )
            strcat( path, "\\" );
    }

    if( *fname ) {
        strcat( path, fname );
        if( *ext ) {
            strcat( path, ext );
        }
    }
}

/*
char * const dst
 , const char *pattern
 , const char *src
 , int len)
*/

APIRET APIENTRY DosEditName(ULONG metalevel,
                            PSZ pszSource,
                            PSZ pszEdit,
                            PSZ pszTarget,
                            ULONG cbTarget);
{ char *s;

  assert(len);
  assert(dst);

  if(!pattern)
    pattern = "";
  if(!src)
    src = "";

  s = dst;
  while(--len) {
    switch(*s = *pattern++) {
    case '\0':
      goto ende;
    case '?':
        if(!*src)
                continue;
/*      if(*src) do not keep ? bejond end-of-filename */
        *s = *src;
    default:
      ++s;
      break;
    case '*':
      memcpy(s, src, len);
      s[len] = '\0';
      return;
    }
    if(*src)
      ++src;
  }
ende:
  *s = '\0';
  assert(strchr(dst, '?') == 0);
}

void fillFnam(char *dest, const char * const pattern
 , const char * const fnam)
{
  char dr[MAXDRIVE], pa[MAXDIR], fn[MAXFILE], ex[MAXEXT], pfn[MAXFILE],
       pex[MAXEXT];
  char dfn[MAXFILE], dex[MAXEXT];

  assert(fnam);
  assert(pattern);

  if(strchr(pattern, '?') == 0 && strchr(pattern, '*') == 0)
    strcpy(dest, pattern);

  myfnsplit(pattern, dr, pa, fn, ex);
  myfnsplit(fnam, 0, 0, pfn, pex);

  fillComp(dfn, fn, pfn, MAXFILE);
  fillComp(dex, ex, pex, MAXEXT);

  myfnmerge(dest, dr, pa, dfn, dex);
}
