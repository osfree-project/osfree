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

#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#include <os2.h>

#include <string.h>

#if 0

// Safe functions
#include <strlcpy.c>
#include <strlcat.c>

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
        strlcpy( path, drive, CCHMAXPATH );
    } else ( *path ) = 0;

    if( *dir ) {
        strlcat( path, dir, CCHMAXPATH );
        if( *( dir + strlen( dir ) - 1 ) != '\\' )
            strlcat( path, "\\", CCHMAXPATH );
    }

    if( *fname ) {
        strlcat( path, fname, CCHMAXPATH );
        if( *ext ) {
            strlcat( path, ext, CCHMAXPATH );
        }
    }
}

/*
char * const dst
 , const char *pattern
 , const char *src
 , int len)
*/

#endif

APIRET APIENTRY DosEditName(ULONG metalevel,
                            PCSZ pszSource,
                            PCSZ pszEdit,
                            PSZ pszTarget,
                            ULONG cbTarget)
{
  char *s;

  // only metalevel == 1 is supported
  if (metalevel != 1)
    return ERROR_INVALID_PARAMETER;

  if (!cbTarget)
    return ERROR_INVALID_PARAMETER;

  if (!pszTarget)
    return ERROR_INVALID_PARAMETER;

  if (!pszEdit)
    pszEdit = "";

  if (!pszSource)
    pszSource = "";

  s = pszTarget;
  while (--cbTarget)
  {
    switch (*s = *pszEdit++)
    {
      case '?':
        // '.' does not copy a character when the end of
        // filename or extension is reached
        if (*pszSource == '.' || !*pszSource)
          continue;

        *s++ = *pszSource;
        if (*pszSource)
          ++pszSource;
        break;
      case '*':
        for (;;)
        {
          *s = *pszSource;
          cbTarget--;
          if (*pszSource)
            pszSource++;
          else
            break;
          if (*s == *pszEdit)
            break;
          if (*s == '\0')
            break;
          s++;
        }
        *s = *pszEdit;
        cbTarget++;
        break;
      case '\0':
        goto ende;
      case '.':
        s++;
        break;
      default:
        if (*pszSource)
          ++pszSource;
        s++;
        break;
    }
  }

ende:
  *s = '\0';

  if (!strchr(pszTarget, '?'))
    return ERROR_INVALID_NAME;

  if (!strchr(pszTarget, '*'))
    return ERROR_INVALID_NAME;

  return NO_ERROR;
}

/*

void fillFnam(char *dest, const char * const pattern
 , const char * const fnam)
{
  char dr[MAXDRIVE], pa[MAXDIR], fn[MAXFILE], ex[MAXEXT], pfn[MAXFILE],
       pex[MAXEXT];
  char dfn[MAXFILE], dex[MAXEXT];

  assert(fnam);
  assert(pattern);

  if(strchr(pattern, '?') == 0 && strchr(pattern, '*') == 0)
    strlcpy(dest, pattern, CCHMAXPATH);

  myfnsplit(pattern, dr, pa, fn, ex);
  myfnsplit(fnam, 0, 0, pfn, pex);

  fillComp(dfn, fn, pfn, MAXFILE);
  fillComp(dex, ex, pex, MAXEXT);

  myfnmerge(dest, dr, pa, dfn, dex);
}

*/
