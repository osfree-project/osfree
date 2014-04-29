#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#define INCL_DOSNLS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"

typedef struct _UniPage
{
USHORT usCode[256];
} UNIPAGE, *PUNIPAGE;

#define ARRAY_COUNT_PAGE    4
#define MAX_ARRAY_PAGE      ( 0x100 / ARRAY_COUNT_PAGE )

#define ARRAY_COUNT_UNICODE 256
#define MAX_ARRAY_UNICODE   (( USHORT )( 0x10000L / ARRAY_COUNT_UNICODE ))

static PUNIPAGE rgPage[ ARRAY_COUNT_PAGE ] = { NULL, };
static PUSHORT  rgUnicode[ ARRAY_COUNT_UNICODE ] = { NULL, };

static UCHAR rgFirstInfo[ 256 ] = { 0 , };

static UCHAR rgLCase[ 256 ] =
{   0,
    1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
   11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
   21,  22,  23,  24,  25,  26,  27,  28,  29,  30,
   31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
   41,  42,  43,  44,  45,  46,  47,  48,  49,  50,
   51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
   61,  62,  63,  64,
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
   91,  92,  93,  94,  95,  96,  97,  98,  99, 100,
  101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
  111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
  121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
  131, 132, 133, 134, 135, 136, 137, 138, 139, 140,
  141, 142, 143, 144, 145, 146, 147, 148, 149, 150,
  151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
  161, 162, 163, 164, 165, 166, 167, 168, 169, 170,
  171, 172, 173, 174, 175, 176, 177, 178, 179, 180,
  181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
  191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
  201, 202, 203, 204, 205, 206, 207, 208, 209, 210,
  211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
  221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
  231, 232, 233, 234, 235, 236, 237, 238, 239, 240,
  241, 242, 243, 244, 245, 246, 247, 248, 249, 250,
  251, 252, 253, 254, 255
};

PRIVATE USHORT QueryUni2NLS( USHORT usPage, USHORT usChar );
PRIVATE VOID   SetUni2NLS( USHORT usPage, USHORT usChar, USHORT usCode );
PRIVATE USHORT QueryNLS2Uni( USHORT usCode );
PRIVATE USHORT GetCurrentCodePage(VOID);

BOOL IsDBCSLead( UCHAR uch )
{
   return ( rgFirstInfo[ uch ] == 2 );
}

VOID GetFirstInfo( PBOOL pFirstInfo )
{
    memcpy( pFirstInfo, rgFirstInfo, sizeof( rgFirstInfo ));
}

VOID GetCaseConversion( PUCHAR pCase )
{
    memcpy( pCase, rgLCase, sizeof( rgLCase ));
}

#if 1  /* by OAX */
/* Get the last-character. (sbcs/dbcs) */
int lastchar(const char *string)
{
    UCHAR *s;
    USHORT c = 0;
    int i, len = strlen(string);
    s = (UCHAR *)string;
    for(i = 0; i < len; i++)
    {
        c = *(s + i);
        if(IsDBCSLead(( UCHAR )c))
        {
            c = (c << 8) + *(s + i + 1);
            i++;
        }
    }
    return c;
}

/* byte step DBCS type strchr() (but. different wstrchr()) */
#ifdef __WATCOM
_WCRTLINK char _FAR_ * _FAR_ strchr(const char _FAR_ *string, int c)
#else
char _FAR_ * _FAR_ _cdecl strchr(const char _FAR_ *string, int c)
#endif
{
    UCHAR *s;
    int  i, len = strlen(string);
    unsigned int ch;
    s = (UCHAR *)string;
    for(i = 0; i < len; i++)
    {
        ch = *(s + i);
        if(IsDBCSLead(( UCHAR )ch))
            ch = (ch << 8) + *(s + i + 1);
        if(( USHORT )c == ch)
            return (s + i);
        if(ch & 0xFF00)
            i++;
    }
    return NULL;
}
/* byte step DBCS type strrchr() (but. different wstrrchr()) */
#ifdef __WATCOM
_WCRTLINK char _FAR_ * _FAR_ strrchr(const char _FAR_ *string, int c)
#else
char _FAR_ * _FAR_ _cdecl strrchr(const char _FAR_ *string, int c)
#endif
{
    char *s, *lastpos;
    s = (char *)string;
    lastpos = strchr(s, c);
    if(!lastpos)
        return NULL;
    for(;;)
    {
        s = lastpos + 1;
        s = strchr(s, c);
        if(!s)
            break;
        lastpos = s;
    }
    return lastpos;
}
#endif /* by OAX */

#ifdef __WATCOM
_WCRTLINK int stricmp( const char *s1, const char *s2 )
#else
int cdecl stricmp( const char *s1, const char *s2 )
#endif
{
    static char szS1Upper[ FAT32MAXPATH ] = "";
    static char szS2Upper[ FAT32MAXPATH ] = "";

    FSH_UPPERCASE(( char * )s1, FAT32MAXPATH, szS1Upper );
    FSH_UPPERCASE(( char * )s2, FAT32MAXPATH, szS2Upper );

    return strcmp( szS1Upper, szS2Upper );
}

#ifdef __WATCOM
_WCRTLINK char * strupr( char *s )
#else
char * cdecl strupr( char *s )
#endif
{
    FSH_UPPERCASE( s, strlen( s ), s );

    return s;
}

#ifdef __WATCOM
_WCRTLINK char * strlwr( char *s )
#else
char * cdecl strlwr( char *s )
#endif
{
    char *t = s;

    while( *t )
    {
        if( IsDBCSLead( *t ))
        {
            t++;
            if( *t )            /* correct tail ? */
                t++;
            else                /* broken DBCS ? */
                break;
        }
        else
            *t++ = rgLCase[ *t ];
    }

    return s;
}

VOID TranslateAllocBuffer( VOID )
{
   INT iIndex;

   for( iIndex = 0; iIndex < ARRAY_COUNT_PAGE; iIndex++ )
      rgPage[ iIndex ] = malloc( sizeof( UNIPAGE ) * MAX_ARRAY_PAGE );

   for( iIndex = 0; iIndex < ARRAY_COUNT_UNICODE; iIndex++ )
      rgUnicode[ iIndex ] = malloc( sizeof( USHORT ) * MAX_ARRAY_UNICODE );
}

VOID TranslateFreeBuffer( VOID )
{
   INT iIndex;

   for( iIndex = 0; iIndex < ARRAY_COUNT_PAGE; iIndex++ )
      free( rgPage[ iIndex ]);

   for( iIndex = 0; iIndex < ARRAY_COUNT_UNICODE; iIndex++ )
      free( rgUnicode[ iIndex ]);
}

USHORT Translate2Win(PSZ pszName, PUSHORT pusUni, USHORT usLen)
{
USHORT usCode;
USHORT usProcessedLen;

   usProcessedLen = 0;

   if (!f32Parms.fTranslateNames)
      {
      while (*pszName && usLen)
         {
         *pusUni++ = (USHORT)*pszName++;
         usLen--;
         usProcessedLen++;
         }
      return usProcessedLen;
      }

/*
   GetCurrentCodePage();
*/

   while (*pszName && usLen)
      {
      usCode = *pszName++;
      if( IsDBCSLead(( UCHAR )usCode ))
         {
         usCode |= (( USHORT )*pszName++ << 8 ) & 0xFF00;
         usProcessedLen++;
         }

      *pusUni++ = QueryNLS2Uni( usCode );
      usLen--;
      usProcessedLen++;
      }

   return usProcessedLen;
}

VOID Translate2OS2(PUSHORT pusUni, PSZ pszName, USHORT usLen)
{
USHORT usPage;
USHORT usChar;
USHORT usCode;

   if (!f32Parms.fTranslateNames)
      {
      while (*pusUni && usLen)
         {
         *pszName++ = (BYTE)*pusUni++;
         usLen--;
         }

      return;
      }

/*
   GetCurrentCodePage();
*/

   while (*pusUni && usLen)
      {
      usPage = ((*pusUni) >> 8) & 0x00FF;
      usChar = (*pusUni) & 0x00FF;

      usCode = QueryUni2NLS( usPage, usChar );
      *pszName++ = ( BYTE )( usCode & 0x00FF );
      if( usCode & 0xFF00 )
         {
         *pszName++ = ( BYTE )(( usCode >> 8 ) & 0x00FF );
         usLen--;
         }

      pusUni++;
      usLen--;
      }
}

BOOL TranslateInit(BYTE rgTrans[], USHORT usSize)
{
ULONG  ulCode;
USHORT usPage;
USHORT usChar;
INT    iIndex;

PVOID *prgTrans = ( PVOID * )rgTrans;

   if( rgPage[ 0 ] == NULL )
      TranslateAllocBuffer();

   if (usSize != sizeof( PVOID ) * ( ARRAY_COUNT_UNICODE + 2 ) )
      return FALSE;

   for( iIndex = 0; iIndex < ARRAY_COUNT_UNICODE; iIndex++ )
      memcpy( rgUnicode[ iIndex ], prgTrans[ iIndex ], sizeof( USHORT ) * MAX_ARRAY_UNICODE );

   for( iIndex = 0; iIndex < MAX_ARRAY_UNICODE; iIndex++ )
      rgFirstInfo[ iIndex ] = ( UCHAR )((( PUSHORT )( prgTrans[ ARRAY_COUNT_UNICODE ] ))[ iIndex ]);

   for( iIndex = 0; iIndex < MAX_ARRAY_UNICODE; iIndex++ )
      rgLCase[ iIndex ] = ( UCHAR )((( PUSHORT )( prgTrans[ ARRAY_COUNT_UNICODE + 1 ] ))[ iIndex ]);

   for( iIndex = 0; iIndex < ARRAY_COUNT_PAGE; iIndex++ )
      memset( rgPage[ iIndex ], '_', sizeof( UNIPAGE ) * MAX_ARRAY_PAGE );

   for (ulCode = 0; ulCode < 0x10000; ulCode++)
      {
      usPage = (QueryNLS2Uni(( USHORT )ulCode ) >> 8) & 0x00FF;
      usChar = QueryNLS2Uni(( USHORT )ulCode ) & 0x00FF;

      SetUni2NLS( usPage, usChar, ( USHORT )ulCode );
      }

   f32Parms.fTranslateNames = TRUE;

   return TRUE;
}

USHORT QueryUni2NLS( USHORT usPage, USHORT usChar )
{
    return rgPage[ usPage / MAX_ARRAY_PAGE ][ usPage % MAX_ARRAY_PAGE ].usCode[ usChar ];
}

VOID SetUni2NLS( USHORT usPage, USHORT usChar, USHORT usCode )
{
    rgPage[ usPage / MAX_ARRAY_PAGE ][ usPage % MAX_ARRAY_PAGE ].usCode[ usChar ] = usCode;
}

USHORT QueryNLS2Uni( USHORT usCode )
{
    return rgUnicode[ usCode / MAX_ARRAY_UNICODE ][ usCode % MAX_ARRAY_UNICODE ];
}

USHORT GetCurrentCodePage(VOID)
{
PULONG pulCP;
USHORT rc;

   rc = DevHelp_GetDOSVar(12,  0, (PPVOID)&pulCP);
   if (rc)
      {
      if (f32Parms.fMessageActive)
         Message("GetDosVar returned %u", rc);
      return 0;
      }
   if (f32Parms.fMessageActive)
      Message("Current codepage tag at %lX = %lu", pulCP, *pulCP);

   rc = DevHelp_GetDOSVar(DHGETDOSV_DOSCODEPAGE,  0, (PPVOID)&pulCP);
   if (rc)
      {
      if (f32Parms.fMessageActive)
         Message("GetDosVar returned %u", rc);
      return 0;
      }
   if (f32Parms.fMessageActive)
      Message("Current codepage tag at %lX = %lu", pulCP, *pulCP);
   return (USHORT)(*pulCP);
}


