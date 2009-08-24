/******************************************************************************
 * REXX Universal Language Support library (RXULS.DLL)                        *
 * (C) 2008 Alex Taylor.                                                      *
 *                                                                            *
 * LICENSE:                                                                   *
 *                                                                            *
 *   Redistribution and use in source and binary forms, with or without       *
 *   modification, are permitted provided that the following conditions are   *
 *   met:                                                                     *
 *                                                                            *
 *   1. Redistributions of source code must retain the above copyright        *
 *      notice, this list of conditions and the following disclaimer.         *
 *                                                                            *
 *   2. Redistributions in binary form must reproduce the above copyright     *
 *      notice, this list of conditions and the following disclaimer in the   *
 *      documentation and/or other materials provided with the distribution.  *
 *                                                                            *
 *   3. The name of the author may not be used to endorse or promote products *
 *      derived from this software without specific prior written permission. *
 *                                                                            *
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR     *
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED           *
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE   *
 *   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,       *
 *   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES       *
 *   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR       *
 *   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       *
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,      *
 *   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN *
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE          *
 *   POSSIBILITY OF SUCH DAMAGE.                                              *
 *                                                                            *
 ******************************************************************************/

#define INCL_WINATOM
#define INCL_WINCLIPBOARD
#define INCL_WINERRORS
#define INCL_DOSPROCESS
#define INCL_REXXSAA
#ifndef OS2_INCLUDED
    #include <os2.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <uconv.h>

//#define INCL_RXSHV
//#define INCL_RXFUNC
#include <rexxsaa.h>


// MACROS

// Convert UniChars to and from separate byte values
#define UNICHARFROM2BYTES( bFirst, bSecond )    (( bFirst << 8 ) | bSecond )
#define BYTE1FROMUNICHAR( uniChar )             ( uniChar >> 8 )
#define BYTE2FROMUNICHAR( uniChar )             ( uniChar & 0x00FF )


// CONSTANTS

#define SZ_LIBRARY_NAME         "RXULS"     // Name of this library
#define SZ_ERROR_NAME           "ULSERR"    // REXX variable used to store error codes

#define US_VERSION_MAJOR        0           // Major version number of this library
#define US_VERSION_MINOR        5           // Minor version number of this library
#define US_VERSION_REFRESH      2           // Refresh level of this library

#define US_CPSPEC_MAXZ          72          // Maximum length of a conversion specifier
#define LOCALE_BUF_MAX          4096        // Maximum length of the locale list

#define US_COMPOUND_MAXZ        250         // Maximum length of a compound variable
#define US_INTEGER_MAXZ         12          // Maximum length of an integer string
#define US_STEM_MAXZ          ( US_COMPOUND_MAXZ - US_INTEGER_MAXZ )  // Maximum length of a stem
#define US_ERRSTR_MAXZ          250         // Maximum length of an error string

// Values for conversion modifiers (0 values are defaults)
#define CONTROLS_MAP_DATA       0           // Use "@map=data"
#define CONTROLS_MAP_DISPLAY    1           // Use "@map=display"
#define CONTROLS_MAP_CDRA       2           // Use "@map=cdra"
#define CONTROLS_MAP_CRLF       3           // Use "@map=crlf"

#define UCS_ENDIAN_SYSTEM       0           // Use "@endian=system"
#define UCS_ENDIAN_BIG          1           // Use "@endian=big"
#define UCS_ENDIAN_LITTLE       UCS_ENDIAN_SYSTEM

#define DBCS_PATH_YES           0           // Use "@path=yes"
#define DBCS_PATH_NO            1           // Use "@path=no"

#define CONV_FLAG_INVALID       0xFFFF      // An invalid value was specified

// List of functions to be registered by ULSLoadFuncs
static PSZ RxFunctionTbl[] = {
    "ULSConvertCodepage",
    "ULSCountryLocale",
    "ULSDropFuncs",
    "ULSFindAttr",
    "ULSGetLocales",
    "ULSGetUnicodeClipboard",
    "ULSPutUnicodeClipboard",
    "ULSQueryAttr",
    "ULSQueryLocaleItem",
    "ULSTransform",
    "ULSVersion"
};


// FUNCTION DECLARATIONS

// Exported REXX functions
RexxFunctionHandler ULSConvertCodepage;
RexxFunctionHandler ULSCountryLocale;
RexxFunctionHandler ULSDropFuncs;
RexxFunctionHandler ULSFindAttr;
RexxFunctionHandler ULSGetLocales;
RexxFunctionHandler ULSGetUnicodeClipboard;
RexxFunctionHandler ULSLoadFuncs;
RexxFunctionHandler ULSPutUnicodeClipboard;
RexxFunctionHandler ULSQueryAttr;
RexxFunctionHandler ULSQueryLocaleItem;
RexxFunctionHandler ULSTransform;
RexxFunctionHandler ULSVersion;


// Private internal functions
USHORT ParseUconvMapFlag( RXSTRING rxstring );
USHORT ParseUconvPathFlag( RXSTRING rxstring );
ULONG  CreateUconvObject( UconvObject *uco, ULONG ulCP, USHORT fEndian, USHORT fMap, USHORT fPath, USHORT ucSubChar );
BOOL   SaveResultString( PRXSTRING prsResult, PCH pchBytes, ULONG ulBytes );
void   WriteStemElement( PSZ pszStem, ULONG ulIndex, PSZ pszValue );
void   WriteErrorCode( ULONG ulError, PSZ pszContext );


/* ------------------------------------------------------------------------- *
 * ULSLoadFuncs                                                              *
 *                                                                           *
 * Should be self-explanatory...                                             *
 *                                                                           *
 * REXX ARGUMENTS:    None                                                   *
 * REXX RETURN VALUE: ""                                                     *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSLoadFuncs( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    int entries,
        i;

    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    if ( argc > 0 ) return ( 40 );
    entries = sizeof(RxFunctionTbl) / sizeof(PSZ);
    for ( i = 0; i < entries; i++ )
        RexxRegisterFunctionDll( RxFunctionTbl[i], SZ_LIBRARY_NAME, RxFunctionTbl[i] );

    MAKERXSTRING( *prsResult, "", 0 );
    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSDropFuncs                                                              *
 *                                                                           *
 * Ditto.                                                                    *
 *                                                                           *
 * REXX ARGUMENTS:    None                                                   *
 * REXX RETURN VALUE: ""                                                     *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSDropFuncs( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    int entries,
        i;

    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    if ( argc > 0 ) return ( 40 );
    entries = sizeof(RxFunctionTbl) / sizeof(PSZ);
    for ( i = 0; i < entries; i++ )
        RexxDeregisterFunction( RxFunctionTbl[i] );

    MAKERXSTRING( *prsResult, "", 0 );
    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSVersion                                                                *
 *                                                                           *
 * Returns the current RXULS library version.                                *
 *                                                                           *
 * REXX ARGUMENTS:    None                                                   *
 * REXX RETURN VALUE: Current version in the form "major.minor.refresh"      *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSVersion( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    CHAR szVersion[ 12 ];

    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    if ( argc > 0 ) return ( 40 );
    sprintf( szVersion, "%d.%d.%d", US_VERSION_MAJOR, US_VERSION_MINOR, US_VERSION_REFRESH );

    MAKERXSTRING( *prsResult, szVersion, strlen(szVersion) );
    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSConvertCodepage                                                        *
 *                                                                           *
 * Convert a string from one codepage to another.                            *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. String to be converted  (REQUIRED)                                   *
 *   2. Source codepage number  (DEFAULT: current)                           *
 *   3. Target codepage number  (DEFAULT: current)                           *
 *   4. Substitution character (hex)  (DEFAULT: varies by codepage)          *
 *   5. Control flag (how to treat byte values 0x00-0x19,0x7F), one of:      *
 *        D = data, treat as control bytes and do not convert (DEFAULT)      *
 *        G = glyphs, treat as glyphs and convert according to codepage      *
 *        C = cdra, treat as control bytes and convert using IBM mapping     *
 *        L = linebreak, treat CR and LF as controls, all others as glyphs   *
 *   6. Path conversion flag (only applies to DBCS codepages), one of:       *
 *        Y = yes, assume string contains a path specifier (DEFAULT)         *
 *        N = no, assume string doesn't contain a path specifier             *
 *                                                                           *
 * REXX RETURN VALUE: Converted string                                       *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSConvertCodepage( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    UconvObject uconvSource = NULL,     // conversion object for source codepage
                uconvTarget = NULL;     // conversion object for target codepage
    UniChar     *psuConverted,          // converted (intermediate) UCS-2 string
                *psuOffset;             // pointer to current UniChar
    PSZ         pszFinal,               // converted final string
                pszOffset;              // pointer to current character
    ULONG       ulSourceCP = 0,         // requested source codepage
                ulTargetCP = 0,         // requested target codepage
                ulRC       = 0,         // return code
                ulBytes    = 0,         // size in bytes of output string
                ulChars    = 0,         // size in UniChars of input string
                i, j;                   // loop indices
    USHORT      fMap  = 0,              // control-mapping flag
                fPath = 0,              // path-conversion flag
                usSub = 0;              // substitution character value
    UCHAR       ucFirst,                // first byte of a UCS-2 pair
                ucSecond;               // second byte of a UCS-2 pair
    size_t      stInLength      = 0,    // length of input buffer (UniUconv*Ucs)
                stOutLength     = 0,    // length of output buffer (UniUconv*Ucs)
                stSubstitutions = 0;    // substitution count (UniUconv*Ucs)


    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Make sure we have at least one valid argument (the input string)
    if ( argc < 1  || ( !RXVALIDSTRING(argv[0]) )) return ( 40 );

    // Other arguments: these are optional, but must be correct if specified
    if ( argc >= 2 ) {
        // second argument: source codepage
        if ( RXVALIDSTRING(argv[1]) && sscanf(argv[1].strptr, "%u", &ulSourceCP) < 1 )
            return ( 40 );
    }
    if ( argc >= 3 ) {
        // third argument: target codepage
        if ( RXVALIDSTRING(argv[2]) && sscanf(argv[2].strptr, "%u", &ulTargetCP) < 1 )
            return ( 40 );
    }
    if ( argc >= 4 ) {
        // fourth argument: substitution character
        if ( RXVALIDSTRING(argv[3]) && sscanf(argv[3].strptr, "%4X", &usSub ) < 1 )
            return ( 40 );
    }
    if ( argc >= 5 ) {
        // fifth argument: control flag
        if (( fMap = ParseUconvMapFlag( argv[4] )) == CONV_FLAG_INVALID )
            return ( 40 );
    }
    if ( argc >= 6 ) {
        // sixth argument: path flag
        if (( fPath = ParseUconvPathFlag( argv[5] )) == CONV_FLAG_INVALID )
            return ( 40 );
    }

    // If source and target codepages are the same, just return the input string
    if ( ulSourceCP == ulTargetCP ) {
        MAKERXSTRING( *prsResult, argv[0].strptr, argv[0].strlength );
        return ( 0 );
    }

    // Create the source-to-UCS conversion object
    if ( ulSourceCP != 1200 )
        ulRC = CreateUconvObject( &uconvSource, ulSourceCP, UCS_ENDIAN_SYSTEM, fMap, fPath, usSub );

    if ( ulRC == ULS_SUCCESS ) {

        if ( ulSourceCP == 1200 ) {
            // Input string is already UCS-2; read it directly into a UniChar array
            psuConverted = (UniChar *) malloc( argv[0].strlength + sizeof(UniChar) );
            ulChars = argv[0].strlength / 2;
            j = 0;
            for ( i = 0; i < ulChars; i++ ) {
                ucFirst  = argv[0].strptr[ j++ ];
                ucSecond = argv[0].strptr[ j++ ];
                psuConverted[ i ] = UNICHARFROM2BYTES( ucFirst, ucSecond );
            }
            psuConverted[ i ] = 0x0000;
        } else {
            // Convert the input string to UCS-2
            stInLength   = argv[0].strlength;
            stOutLength  = stInLength;
            psuConverted = (UniChar *) calloc( stOutLength + 1, sizeof(UniChar) );
            pszOffset    = argv[0].strptr;
            psuOffset    = psuConverted;
            memset( psuConverted, 0, (stOutLength+1) * sizeof(UniChar) );
            ulRC = UniUconvToUcs( uconvSource, (PPVOID) &pszOffset, &stInLength,
                                  &psuOffset, &stOutLength, &stSubstitutions     );
        }

        if ( ulRC == ULS_SUCCESS ) {

            if ( ulTargetCP == 1200 ) {
                // Converting to UCS-2; simply return the UniChar array as bytes
                ulBytes  = UniStrlen( psuConverted ) * sizeof( UniChar );
                pszFinal = (PSZ) malloc( ulBytes );
                j = 0;
                for ( i = 0; i < UniStrlen(psuConverted); i++ ) {
                    pszFinal[j++] = BYTE1FROMUNICHAR( psuConverted[i] );
                    pszFinal[j++] = BYTE2FROMUNICHAR( psuConverted[i] );
                }
                if ( ! SaveResultString( prsResult, pszFinal, ulBytes )) {
                    MAKERXSTRING( *prsResult, "", 0 );
                }
                free( pszFinal );
            } else {
                // Create the UCS-to-target conversion object
                ulRC = CreateUconvObject( &uconvTarget, ulTargetCP, UCS_ENDIAN_SYSTEM, fMap, fPath, usSub  );
                if ( ulRC == ULS_SUCCESS ) {

                    // Convert the string to the target codepage
                    // (Allow up to 4x the length of the original string)
                    stInLength      = UniStrlen( psuConverted );
                    stOutLength     = ( argv[0].strlength * 4 );
                    pszFinal        = (PSZ) malloc( stOutLength + 1 );
                    psuOffset       = psuConverted;
                    pszOffset       = pszFinal;
                    stSubstitutions = 0;
                    memset( pszFinal, 0, stOutLength + 1 );
                    ulRC = UniUconvFromUcs( uconvTarget, &psuOffset, &stInLength,
                                            (PVOID *) &pszOffset, &stOutLength, &stSubstitutions );
                    if ( ulRC == ULS_SUCCESS ) {
                        // Return the final converted string
                        if ( ! SaveResultString( prsResult, pszFinal, strlen(pszFinal) )) {
                            MAKERXSTRING( *prsResult, "", 0 );
                        }
                    } else {
                        // UniUconvFromUcs failed
                        WriteErrorCode( ulRC, "UniUconvFromUcs");
                        MAKERXSTRING( *prsResult, "", 0 );
                    }
                    free( pszFinal );
                    UniFreeUconvObject( uconvTarget );

                } else {
                    // Failed to create target UconvObject
                    WriteErrorCode( ulRC, "UniCreateUconvObject");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
            }
            free( psuConverted );

        } else {
            // UniUconvToUcs failed
            WriteErrorCode( ulRC, "UniUconvFromUcs");
            MAKERXSTRING( *prsResult, "", 0 );
        }

        if ( ulSourceCP != 1200 )
            UniFreeUconvObject( uconvSource );

    } else {
        // Failed to create source UconvObject
        WriteErrorCode( ulRC, "UniCreateUconvObject");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSCountryLocale                                                          *
 *                                                                           *
 * Returns the locale name associated with the specified integer identifier. *
 * This integer is almost, but not quite, the same as the standard OS/2      *
 * "country code" as used by DosQueryDBCSEnv().  Since it ISN'T quite the    *
 * same, this function is probably of rather limited usefulness; for this    *
 * reason, and to help prevent confusion on the issue, I may end up leaving  *
 * it out of the public RXULS documentation.                                 *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. The numeric locale code being queried.                               *
 *                                                                           *
 * REXX RETURN VALUE: Locale name                                            *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSCountryLocale( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    UconvObject uconv;
    UniChar     suLocale[ ULS_LNAMEMAX ];
    CHAR        szLocale[ ULS_LNAMEMAX ];
    ULONG       ulCountry,
                ulRC;

    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Make sure we have exactly one valid argument
    if ( argc != 1 || ( !RXVALIDSTRING(argv[0]) )) return ( 40 );
    if ( RXVALIDSTRING(argv[0]) && sscanf(argv[0].strptr, "%d", &ulCountry) < 1 )
        return ( 40 );

    ulRC = UniMapCtryToLocale( ulCountry, suLocale, ULS_LNAMEMAX );
    if ( ulRC == ULS_SUCCESS ) {
        // The above returns a UniChar string; convert it to a codepage string
        ulRC = UniCreateUconvObject( (UniChar *)L"@map=display", &uconv );
        if ( ulRC == ULS_SUCCESS ) {
            ulRC = UniStrFromUcs( uconv, szLocale, suLocale, ULS_LNAMEMAX );
            if ( ulRC != ULS_SUCCESS ) sprintf( szLocale, "%ls", suLocale );
            UniFreeUconvObject( uconv );
        } else
            sprintf( szLocale, "%ls", suLocale );
        MAKERXSTRING( *prsResult, szLocale, strlen(szLocale) );
    } else {
        WriteErrorCode( ulRC, "UniMapCtryToLocale");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSGetLocales                                                             *
 *                                                                           *
 * Returns a list of locales defined on the system.                          *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. Type of locales to list.  One of the following:                      *
 *        B = Both system and user locales  (DEFAULT)                        *
 *        S = System locales                                                 *
 *        U = User locales                                                   *
 *   2. Name of the REXX stem variable to return the results (REQUIRED)      *
 *                                                                           *
 * REXX RETURN VALUE: Number of locales returned                             *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSGetLocales( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    UconvObject uconv;
    UniChar     suLocales[ LOCALE_BUF_MAX ], // list of null-separated locales
                *psuLocale;                  // a single UCS-2 locale name
    CHAR        szLocale[ ULS_LNAMEMAX ],    // a converted locale name
                szStem[ US_STEM_MAXZ ],      // the name of our stem variable
                szNumber[ US_INTEGER_MAXZ ]; // the total number of locales
    ULONG       fLocaleType,                 // locale type flag
                ulOffset,                    // current array position
                ulCount,                     // locale count
                ulRC;                        // return code

    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Check the arguments
    if ( argc != 2 || !RXVALIDSTRING(argv[1]) )
        return ( 40 );

    if ( RXVALIDSTRING(argv[0]) ) {
        strupr( argv[0].strptr );
        if (strcspn(argv[0].strptr, "BSU") > 0 ) return ( 40 );
        switch ( argv[0].strptr[0] ) {
            case 'S': fLocaleType = UNI_SYSTEM_LOCALES;                    break;
            case 'U': fLocaleType = UNI_USER_LOCALES;                      break;
            default : fLocaleType = UNI_SYSTEM_LOCALES | UNI_USER_LOCALES; break;
        }
    } else fLocaleType = UNI_SYSTEM_LOCALES | UNI_USER_LOCALES;

    // Generate the stem variable name from the argument (stripping any final dot)
    if ( argv[1].strptr[ argv[1].strlength-1 ] == '.') argv[1].strlength--;
    strncpy( szStem, argv[1].strptr, RXSTRLEN(argv[1]) );
    szStem[ RXSTRLEN(argv[1]) ] = '\0';

    // Get the requested list of locales
    ulRC = UniQueryLocaleList( fLocaleType, suLocales, LOCALE_BUF_MAX );
    if ( ulRC == ULS_SUCCESS ) {

        // Create a conversion object for the current codepage
        ulRC = UniCreateUconvObject( (UniChar *)L"@map=display", &uconv );
        if ( ulRC != ULS_SUCCESS ) uconv = NULL;

        // Now parse each null-delimited locale name from the list
        ulOffset = 0;
        ulCount  = 0;
        while (( ulOffset < LOCALE_BUF_MAX ) && ( suLocales[ulOffset] != 0 )) {
            psuLocale = suLocales + ulOffset;
            // Convert the locale name to a codepage string and add to the stem
            if (( uconv == NULL ) || ( UniStrFromUcs( uconv, szLocale, psuLocale, ULS_LNAMEMAX ) == ULS_SUCCESS ))
                sprintf( szLocale, "%ls", psuLocale );
            WriteStemElement( szStem, ++ulCount, szLocale );
            ulOffset += UniStrlen( psuLocale ) + 1;
        }

        // Write the stem.0 element (the number of locales), and also return it
        sprintf( szNumber, "%d", ulCount );
        WriteStemElement( szStem, 0, szNumber );
        MAKERXSTRING( *prsResult, szNumber, strlen(szNumber) );

        if ( uconv != NULL ) UniFreeUconvObject( uconv );

    } else {
        WriteErrorCode( ulRC, "UniQueryLocaleList");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSFindAttr                                                               *
 *                                                                           *
 * Searches a string for the first character with the specified attribute.   *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. String being searched  (REQUIRED)                                    *
 *   2. Name of attribute to check for  (REQUIRED)                           *
 *   3. First character to search  (DEFAULT: start of string, i.e. 1)        *
 *   4. Maximum number of characters to search  (DEFAULT: entire string)     *
 *   5. Type of search:                                                      *
 *        T = True, locate first matching character (DEFAULT)                *
 *        F = False, locate first non-matching character                     *
 *   6. Source codepage number  (DEFAULT: current)                           *
 *   7. Name of locale  (DEFAULT: as per current environment settings)       *
 *                                                                           *
 * REXX RETURN VALUE: Position of the first match, or 0 if not found         *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSFindAttr( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    UconvObject  uconvCP;               // conversion object
    AttrObject   attrib;                // attribute object
    LocaleObject locale;                // handle to the requested locale
    UniChar      *psuString,            // input UniChar string
                 *psuOffset,
                 *psuSearch,            // pointer to start of searched text
                 *psuAttrib;            // name of the requested attribute
    PSZ          pszOffset;
    UCHAR        ucFirst,               // first byte of a UCS-2 pair
                 ucSecond;              // second byte of a UCS-2 pair
    ULONG        ulChars,               // number of UniChars in string
                 ulSourceCP = 0,        // source codepage number
                 ulStart    = 0,        // starting position of search
                 ulRC       = 0,        // return code
                 i, j;                  // counters
    BOOL         fInverse = FALSE;      // inverse (negative) search flag
    size_t       stLimit  = 0,          // length limit of search
                 stPos    = 0,          // character position
                 stInLen,
                 stOutLen,
                 stSubs;

    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Make sure we have at least two valid arguments (and no more than seven)
    if ( argc < 2 || argc > 7 || !RXVALIDSTRING(argv[0]) || !RXVALIDSTRING(argv[1]) )
        return ( 40 );
    if ( argv[0].strlength < 2 ) return ( 40 );

    // UniCreateAttrObject() requires the attribute name in lowercase
    strlwr( argv[1].strptr );

    // Other arguments: these are optional, but must be correct if specified
    if ( argc >= 3 && RXVALIDSTRING(argv[2]) ) {
        if ( sscanf(argv[2].strptr, "%d", &ulStart ) < 1 )
            return ( 40 );
        if ( ulStart > (argv[0].strlength / 2) || ulStart < 1 )
            return ( 40 );
        ulStart--;
    }
    if ( argc >= 4 ) {
        if ( RXVALIDSTRING(argv[3]) && sscanf(argv[3].strptr, "%d", &stLimit ) < 1 )
            return ( 40 );
    }
    if ( argc >= 5 && RXVALIDSTRING(argv[4]) ) {
        strupr( argv[4].strptr );
        if (strcspn(argv[4].strptr, "TF") > 0 ) return ( 40 );
        switch ( argv[4].strptr[0] ) {
            case 'F': fInverse = TRUE;  break;
            default : fInverse = FALSE; break;
        }
    }
    if ( argc >= 6 ) {
        // Parse the codepage number
        if ( RXVALIDSTRING(argv[5]) && sscanf(argv[5].strptr, "%d", &ulSourceCP) < 1 )
            return ( 40 );
        // For UCS-2 input, make sure we have at least one valid character...
        if (( ulSourceCP == 1200 ) && ( argv[0].strlength < 2 ))
            return ( 40 );
    }
    // Parse the optional locale argument and create a handle for that locale
    if ( argc >= 7 && RXVALIDSTRING(argv[6]) )
        ulRC = UniCreateLocaleObject( UNI_MBS_STRING_POINTER, argv[6].strptr, &locale );
    else
        ulRC = UniCreateLocaleObject( UNI_MBS_STRING_POINTER, "", &locale );

    if ( ulRC == ULS_SUCCESS ) {

        ulRC = CreateUconvObject( &uconvCP, ulSourceCP,
                                  UCS_ENDIAN_SYSTEM, CONTROLS_MAP_DATA, DBCS_PATH_NO, 0 );
        if ( ulRC == ULS_SUCCESS ) {

            if ( ulSourceCP == 1200 ) {
                // Input string is already UCS-2; read it directly into a UniChar array
                psuString = (UniChar *) malloc( argv[0].strlength + sizeof(UniChar) );
                ulChars = ( argv[0].strlength + 1 ) / 2;
                j = 0;
                for ( i = 0; i < ulChars; i++ ) {
                    ucFirst  = argv[0].strptr[ j++ ];
                    ucSecond = argv[0].strptr[ j++ ];
                    psuString[ i ] = UNICHARFROM2BYTES( ucFirst, ucSecond );
                }
                psuString[ i ] = 0x0000;
            } else {
                // Convert the input text to UCS-2
                stInLen   = argv[0].strlength;
                stOutLen  = stInLen;
                stSubs    = 0;
                psuString = (UniChar *) calloc( stOutLen + 1, sizeof(UniChar) );
                pszOffset = argv[0].strptr;
                psuOffset = psuString;
                ulRC = UniUconvToUcs( uconvCP, (PPVOID) &pszOffset, &stInLen, &psuOffset, &stOutLen, &stSubs );
                if ( ulRC != ULS_SUCCESS ) {
                    WriteErrorCode( ulRC, "UniUconvToUcs");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
            }

            if ( ulRC == ULS_SUCCESS ) {
                // Convert the attribute to UCS-2
                psuAttrib = (UniChar *) calloc( argv[1].strlength + 1, sizeof(UniChar) );
                ulRC = UniStrToUcs( uconvCP, psuAttrib, argv[1].strptr, argv[1].strlength );
                if ( ulRC != ULS_SUCCESS )
                    mbstowcs( psuAttrib, argv[1].strptr, argv[1].strlength );

                // Create an AttrObject for the requested attribute
                ulRC = UniCreateAttrObject( locale, psuAttrib, &attrib );
                if ( ulRC == ULS_SUCCESS ) {

                    // Set the search boundaries
                    psuSearch = psuString + ulStart;
                    if ( stLimit < 1 ) stLimit = UniStrlen( psuSearch );

                    // Now scan the string for the requested attribute
                    ulRC = UniScanForAttr( attrib, psuSearch, stLimit, fInverse, &stPos );
                    if ( ulRC == ULS_NOMATCH ) {
                        sprintf( prsResult->strptr, "0");
                        prsResult->strlength = strlen( prsResult->strptr );
                    } else if ( ulRC == ULS_SUCCESS ) {
                        sprintf( prsResult->strptr, "%d", stPos + 1 );
                        prsResult->strlength = strlen( prsResult->strptr );
                    } else {
                        WriteErrorCode( ulRC, "UniScanForAttr");
                        MAKERXSTRING( *prsResult, "", 0 );
                    }

                    UniFreeAttrObject( attrib );
                } else {
                    WriteErrorCode( ulRC, "UniCreateAttrObject");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
                free( psuAttrib );
            }
            UniFreeUconvObject( uconvCP );

        } else {
            WriteErrorCode( ulRC, "UniCreateUconvObject");
            MAKERXSTRING( *prsResult, "", 0 );
        }

        UniFreeLocaleObject( locale );
        free( psuString );

    } else {
        WriteErrorCode( ulRC, "UniCreateLocaleObject");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSQueryAttr                                                              *
 *                                                                           *
 * Queries whether the specified multi-byte character has the specified      *
 * character attribute.                                                      *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. 1-4 byte codepage character being queried  (REQUIRED)                *
 *   2. Name of attribute to check for  (REQUIRED)                           *
 *   3. Source codepage number  (DEFAULT: current)                           *
 *   4. Name of locale  (DEFAULT: as per current environment settings)       *
 *                                                                           *
 * REXX RETURN VALUE:                                                        *
 *   '0' : Character does not have specified attribute                       *
 *   '1' : Character has specified attribute                                 *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSQueryAttr( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    UconvObject  uconvCP;               // conversion object
    AttrObject   attrib;                // attribute object
    LocaleObject locale;                // handle to the requested locale
    UniChar      suChar[ 2 ],           // input UniChar (as string)
                 *psuOffset,
                 *psuAttrib;            // name of the requested attribute
    PSZ          pszOffset;
    ULONG        ulSourceCP = 0,        // source codepage number
                 ulRC       = 0;        // return code
    size_t       stInLen,
                 stOutLen,
                 stSubs;


    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Make sure we have at least two valid arguments (and no more than four)
    if ( argc < 2 || argc > 4 || !RXVALIDSTRING(argv[0]) || !RXVALIDSTRING(argv[1]) )
        return ( 40 );

    // UniCreateAttrObject() requires the attribute name in lowercase
    strlwr( argv[1].strptr );

    // Parse the codepage number
    if ( argc >= 3 ) {
        if ( RXVALIDSTRING(argv[2]) && sscanf(argv[2].strptr, "%d", &ulSourceCP) < 1 )
            return ( 40 );
        // For UCS-2 input, make sure we have a valid character...
        if (( ulSourceCP == 1200 ) && ( argv[0].strlength < 2 ))
            return ( 40 );
    }
    // Parse the optional locale argument and create a handle for that locale
    if ( argc >= 4 && RXVALIDSTRING(argv[3]))
        ulRC = UniCreateLocaleObject( UNI_MBS_STRING_POINTER, argv[3].strptr, &locale );
    else
        ulRC = UniCreateLocaleObject( UNI_MBS_STRING_POINTER, "", &locale );

    if ( ulRC == ULS_SUCCESS ) {

        ulRC = CreateUconvObject( &uconvCP, ulSourceCP,
                                  UCS_ENDIAN_SYSTEM, CONTROLS_MAP_DATA, DBCS_PATH_NO, 0 );
        if ( ulRC == ULS_SUCCESS ) {
            memset( suChar, 0, sizeof(suChar) );
            if ( ulSourceCP == 1200 ) {
                // Input text is already UCS-2; parse it directly as such
                suChar[ 0 ] = UNICHARFROM2BYTES( argv[0].strptr[0], argv[0].strptr[1] );
            } else {
                // Convert the input text to UCS-2
                stInLen   = argv[0].strlength;
                stOutLen  = 2;
                stSubs    = 0;
                pszOffset = argv[0].strptr;
                psuOffset = suChar;
                ulRC = UniUconvToUcs( uconvCP, (PPVOID) &pszOffset, &stInLen, &psuOffset, &stOutLen, &stSubs );
                if ( ulRC != ULS_SUCCESS ) {
                    WriteErrorCode( ulRC, "UniUconvToUcs");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
            }

            if ( ulRC == ULS_SUCCESS ) {
                psuAttrib = (UniChar *) calloc( argv[1].strlength + 1, sizeof(UniChar) );
                ulRC = UniStrToUcs( uconvCP, psuAttrib, argv[1].strptr, argv[1].strlength );
                if ( ulRC != ULS_SUCCESS )
                    mbstowcs( psuAttrib, argv[1].strptr, argv[1].strlength );

                // Create an AttrObject for the requested attribute
                ulRC = UniCreateAttrObject( locale, psuAttrib, &attrib );
                if ( ulRC == ULS_SUCCESS ) {

                    // Now do the query and return the result
                    if ( UniQueryCharAttr( attrib, suChar[0] ) == 0 ) {
                        MAKERXSTRING( *prsResult, "0", 1 );
                    } else {
                        MAKERXSTRING( *prsResult, "1", 1 );
                    }
                    UniFreeAttrObject( attrib );

                } else {
                    WriteErrorCode( ulRC, "UniCreateAttrObject");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
                free( psuAttrib );
            }
            UniFreeUconvObject( uconvCP );

        } else {
            WriteErrorCode( ulRC, "UniCreateUconvObject");
            MAKERXSTRING( *prsResult, "", 0 );
        }
        UniFreeLocaleObject( locale );

    } else {
        WriteErrorCode( ulRC, "UniCreateLocaleObject");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSQueryLocaleItem                                                        *
 *                                                                           *
 * Queries the value of the specified locale item.                           *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. Name of locale item  (REQUIRED)                                      *
 *   2. Name of locale  (DEFAULT: as per current environment settings)       *
 *   3. Target codepage number  (DEFAULT: current)                           *
 *   4. Substitution character (hex)  (DEFAULT: varies by codepage)          *
 *                                                                           *
 * REXX RETURN VALUE: The requested item value                               *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSQueryLocaleItem( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    UconvObject  uconvCP;               // conversion object
    LocaleObject locale;                // handle to the requested locale
    LocaleItem   item;                  // the locale item being queried
    UniChar      *psuValue,             // the returned item value
                 *psuOffset;            // pointer into psuValue
    PSZ          pszFinal,              // converted item value
                 pszOffset;             // pointer into pszFinal
    ULONG        ulCP    = 0,           // target codepage number
                 ulBytes = 0,           // length of pszFinal
                 ulRC    = 0,           // return code
                 i, j;                  // indices
    USHORT       usSub = 0;             // substitution character value
    size_t       stInLength  = 0,       // input buffer length (UniUconvFromUcs)
                 stOutLength = 0,       // output buffer length (UniUconvFromUcs)
                 stSubs      = 0;       // substitution count (UniUconvFromUcs)


    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Make sure we have at least one valid argument (and no more than four)
    if ( argc < 1 || argc > 4 || !RXVALIDSTRING(argv[0]) )
        return ( 40 );

    // Parse the optional locale argument and create a handle for that locale
    if ( argc >= 2 && RXVALIDSTRING(argv[1]) )
        ulRC = UniCreateLocaleObject( UNI_MBS_STRING_POINTER, argv[1].strptr, &locale );
    else
        ulRC = UniCreateLocaleObject( UNI_MBS_STRING_POINTER, "", &locale );

    // Parse the target codepage number
    if ( argc >= 3 ) {
        if ( RXVALIDSTRING(argv[2]) && sscanf(argv[2].strptr, "%d", &ulCP) < 1 )
            return ( 40 );
    }

    // Parse the substitution character option
    if ( argc >= 4 ) {
        if ( RXVALIDSTRING(argv[3]) && sscanf(argv[3].strptr, "%4X", &usSub ) < 1 )
            return ( 40 );
    }

    if ( ulRC == ULS_SUCCESS ) {

        // Parse the name or number of the locale item requested
        if ( sscanf( argv[0].strptr, "%d", &item ) < 1 ) {
            // name string specified
            strupr( argv[0].strptr );
            if ( strcmp( argv[0].strptr, "SDATETIME") == 0 )                 item = LOCI_sDateTime;
            else if ( strcmp( argv[0].strptr, "SSHORTDATE") == 0 )           item = LOCI_sShortDate;
            else if ( strcmp( argv[0].strptr, "STIMEFORMAT") == 0 )          item = LOCI_sTimeFormat;
            else if ( strcmp( argv[0].strptr, "S1159") == 0 )                item = LOCI_s1159;
            else if ( strcmp( argv[0].strptr, "S2359") == 0 )                item = LOCI_s2359;
            else if ( strcmp( argv[0].strptr, "SABBREVDAYNAME7") == 0 )      item = LOCI_sAbbrevDayName7;
            else if ( strcmp( argv[0].strptr, "SABBREVDAYNAME1") == 0 )      item = LOCI_sAbbrevDayName1;
            else if ( strcmp( argv[0].strptr, "SABBREVDAYNAME2") == 0 )      item = LOCI_sAbbrevDayName2;
            else if ( strcmp( argv[0].strptr, "SABBREVDAYNAME3") == 0 )      item = LOCI_sAbbrevDayName3;
            else if ( strcmp( argv[0].strptr, "SABBREVDAYNAME4") == 0 )      item = LOCI_sAbbrevDayName4;
            else if ( strcmp( argv[0].strptr, "SABBREVDAYNAME5") == 0 )      item = LOCI_sAbbrevDayName5;
            else if ( strcmp( argv[0].strptr, "SABBREVDAYNAME6") == 0 )      item = LOCI_sAbbrevDayName6;
            else if ( strcmp( argv[0].strptr, "SDAYNAME7") == 0 )            item = LOCI_sDayName7;
            else if ( strcmp( argv[0].strptr, "SDAYNAME1") == 0 )            item = LOCI_sDayName1;
            else if ( strcmp( argv[0].strptr, "SDAYNAME2") == 0 )            item = LOCI_sDayName2;
            else if ( strcmp( argv[0].strptr, "SDAYNAME3") == 0 )            item = LOCI_sDayName3;
            else if ( strcmp( argv[0].strptr, "SDAYNAME4") == 0 )            item = LOCI_sDayName4;
            else if ( strcmp( argv[0].strptr, "SDAYNAME5") == 0 )            item = LOCI_sDayName5;
            else if ( strcmp( argv[0].strptr, "SDAYNAME6") == 0 )            item = LOCI_sDayName6;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME1") == 0 )    item = LOCI_sAbbrevMonthName1;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME2") == 0 )    item = LOCI_sAbbrevMonthName2;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME3") == 0 )    item = LOCI_sAbbrevMonthName3;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME4") == 0 )    item = LOCI_sAbbrevMonthName4;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME5") == 0 )    item = LOCI_sAbbrevMonthName5;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME6") == 0 )    item = LOCI_sAbbrevMonthName6;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME7") == 0 )    item = LOCI_sAbbrevMonthName7;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME8") == 0 )    item = LOCI_sAbbrevMonthName8;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME9") == 0 )    item = LOCI_sAbbrevMonthName9;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME10") == 0 )   item = LOCI_sAbbrevMonthName10;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME11") == 0 )   item = LOCI_sAbbrevMonthName11;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME12") == 0 )   item = LOCI_sAbbrevMonthName12;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME1") == 0 )          item = LOCI_sMonthName1;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME2") == 0 )          item = LOCI_sMonthName2;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME3") == 0 )          item = LOCI_sMonthName3;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME4") == 0 )          item = LOCI_sMonthName4;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME5") == 0 )          item = LOCI_sMonthName5;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME6") == 0 )          item = LOCI_sMonthName6;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME7") == 0 )          item = LOCI_sMonthName7;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME8") == 0 )          item = LOCI_sMonthName8;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME9") == 0 )          item = LOCI_sMonthName9;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME10") == 0 )         item = LOCI_sMonthName10;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME11") == 0 )         item = LOCI_sMonthName11;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME12") == 0 )         item = LOCI_sMonthName12;
            else if ( strcmp( argv[0].strptr, "SDECIMAL") == 0 )             item = LOCI_sDecimal;
            else if ( strcmp( argv[0].strptr, "STHOUSAND") == 0 )            item = LOCI_sThousand;
            else if ( strcmp( argv[0].strptr, "SYESSTRING") == 0 )           item = LOCI_sYesString;
            else if ( strcmp( argv[0].strptr, "SNOSTRING") == 0 )            item = LOCI_sNoString;
            else if ( strcmp( argv[0].strptr, "SCURRENCY") == 0 )            item = LOCI_sCurrency;
            else if ( strcmp( argv[0].strptr, "SCODESET") == 0 )             item = LOCI_sCodeSet;
            else if ( strcmp( argv[0].strptr, "XLOCALETOKEN") == 0 )         item = LOCI_xLocaleToken;
            else if ( strcmp( argv[0].strptr, "XWINLOCALE") == 0 )           item = LOCI_xWinLocale;
            else if ( strcmp( argv[0].strptr, "ILOCALERESNUM") == 0 )        item = LOCI_iLocaleResnum;
            else if ( strcmp( argv[0].strptr, "SNATIVEDIGITS") == 0 )        item = LOCI_sNativeDigits;
            else if ( strcmp( argv[0].strptr, "IMAXITEM") == 0 )             item = LOCI_iMaxItem;
            else if ( strcmp( argv[0].strptr, "STIMEMARK") == 0 )            item = LOCI_sTimeMark;
            else if ( strcmp( argv[0].strptr, "SERA") == 0 )                 item = LOCI_sEra;
            else if ( strcmp( argv[0].strptr, "SALTSHORTDATE") == 0 )        item = LOCI_sAltShortDate;
            else if ( strcmp( argv[0].strptr, "SALTDATETIME") == 0 )         item = LOCI_sAltDateTime;
            else if ( strcmp( argv[0].strptr, "SALTTIMEFORMAT") == 0 )       item = LOCI_sAltTimeFormat;
            else if ( strcmp( argv[0].strptr, "SALTDIGITS") == 0 )           item = LOCI_sAltDigits;
            else if ( strcmp( argv[0].strptr, "SYESEXPR") == 0 )             item = LOCI_sYesExpr;
            else if ( strcmp( argv[0].strptr, "SNOEXPR") == 0 )              item = LOCI_sNoExpr;
            else if ( strcmp( argv[0].strptr, "SDATE") == 0 )                item = LOCI_sDate;
            else if ( strcmp( argv[0].strptr, "STIME") == 0 )                item = LOCI_sTime;
            else if ( strcmp( argv[0].strptr, "SLIST") == 0 )                item = LOCI_sList;
            else if ( strcmp( argv[0].strptr, "SMONDECIMALSEP") == 0 )       item = LOCI_sMonDecimalSep;
            else if ( strcmp( argv[0].strptr, "SMONTHOUSANDSEP") == 0 )      item = LOCI_sMonThousandSep;
            else if ( strcmp( argv[0].strptr, "SGROUPING") == 0 )            item = LOCI_sGrouping;
            else if ( strcmp( argv[0].strptr, "SMONGROUPING") == 0 )         item = LOCI_sMonGrouping;
            else if ( strcmp( argv[0].strptr, "IMEASURE") == 0 )             item = LOCI_iMeasure;
            else if ( strcmp( argv[0].strptr, "IPAPER") == 0 )               item = LOCI_iPaper;
            else if ( strcmp( argv[0].strptr, "IDIGITS") == 0 )              item = LOCI_iDigits;
            else if ( strcmp( argv[0].strptr, "ITIME") == 0 )                item = LOCI_iTime;
            else if ( strcmp( argv[0].strptr, "IDATE") == 0 )                item = LOCI_iDate;
            else if ( strcmp( argv[0].strptr, "ICURRENCY") == 0 )            item = LOCI_iCurrency;
            else if ( strcmp( argv[0].strptr, "ICURRDIGITS") == 0 )          item = LOCI_iCurrDigits;
            else if ( strcmp( argv[0].strptr, "ILZERO") == 0 )               item = LOCI_iLzero;
            else if ( strcmp( argv[0].strptr, "INEGNUMBER") == 0 )           item = LOCI_iNegNumber;
            else if ( strcmp( argv[0].strptr, "ILDATE") == 0 )               item = LOCI_iLDate;
            else if ( strcmp( argv[0].strptr, "ICALENDARTYPE") == 0 )        item = LOCI_iCalendarType;
            else if ( strcmp( argv[0].strptr, "IFIRSTDAYOFWEEK") == 0 )      item = LOCI_iFirstDayOfWeek;
            else if ( strcmp( argv[0].strptr, "IFIRSTWEEKOFYEAR") == 0 )     item = LOCI_iFirstWeekOfYear;
            else if ( strcmp( argv[0].strptr, "INEGCURR") == 0 )             item = LOCI_iNegCurr;
            else if ( strcmp( argv[0].strptr, "ITLZERO") == 0 )              item = LOCI_iTLzero;
            else if ( strcmp( argv[0].strptr, "ITIMEPREFIX") == 0 )          item = LOCI_iTimePrefix;
            else if ( strcmp( argv[0].strptr, "IOPTIONALCALENDAR") == 0 )    item = LOCI_iOptionalCalendar;
            else if ( strcmp( argv[0].strptr, "SINTLSYMBOL") == 0 )          item = LOCI_sIntlSymbol;
            else if ( strcmp( argv[0].strptr, "SABBREVLANGNAME") == 0 )      item = LOCI_sAbbrevLangName;
            else if ( strcmp( argv[0].strptr, "SCOLLATE") == 0 )             item = LOCI_sCollate;
            else if ( strcmp( argv[0].strptr, "IUPPERTYPE") == 0 )           item = LOCI_iUpperType;
            else if ( strcmp( argv[0].strptr, "IUPPERMISSING") == 0 )        item = LOCI_iUpperMissing;
            else if ( strcmp( argv[0].strptr, "SPOSITIVESIGN") == 0 )        item = LOCI_sPositiveSign;
            else if ( strcmp( argv[0].strptr, "SNEGATIVESIGN") == 0 )        item = LOCI_sNegativeSign;
            else if ( strcmp( argv[0].strptr, "SLEFTNEGATIVE") == 0 )        item = LOCI_sLeftNegative;
            else if ( strcmp( argv[0].strptr, "SRIGHTNEGATIVE") == 0 )       item = LOCI_sRightNegative;
            else if ( strcmp( argv[0].strptr, "SLONGDATE") == 0 )            item = LOCI_sLongDate;
            else if ( strcmp( argv[0].strptr, "SALTLONGDATE") == 0 )         item = LOCI_sAltLongDate;
            else if ( strcmp( argv[0].strptr, "SMONTHNAME13") == 0 )         item = LOCI_sMonthName13;
            else if ( strcmp( argv[0].strptr, "SABBREVMONTHNAME13") == 0 )   item = LOCI_sAbbrevMonthName13;
            else if ( strcmp( argv[0].strptr, "SNAME") == 0 )                item = LOCI_sName;
            else if ( strcmp( argv[0].strptr, "SLANGUAGEID") == 0 )          item = LOCI_sLanguageID;
            else if ( strcmp( argv[0].strptr, "SCOUNTRYID") == 0 )           item = LOCI_sCountryID;
            else if ( strcmp( argv[0].strptr, "SENGLANGUAGE") == 0 )         item = LOCI_sEngLanguage;
            else if ( strcmp( argv[0].strptr, "SLANGUAGE") == 0 )            item = LOCI_sLanguage;
            else if ( strcmp( argv[0].strptr, "SENGCOUNTRY") == 0 )          item = LOCI_sEngCountry;
            else if ( strcmp( argv[0].strptr, "SCOUNTRY") == 0 )             item = LOCI_sCountry;
            else if ( strcmp( argv[0].strptr, "SNATIVECTRYNAME") == 0 )      item = LOCI_sNativeCtryName;
            else if ( strcmp( argv[0].strptr, "ICOUNTRY") == 0 )             item = LOCI_iCountry;
            else if ( strcmp( argv[0].strptr, "SISOCODEPAGE") == 0 )         item = LOCI_sISOCodepage;
            else if ( strcmp( argv[0].strptr, "IANSICODEPAGE") == 0 )        item = LOCI_iAnsiCodepage;
            else if ( strcmp( argv[0].strptr, "ICODEPAGE") == 0 )            item = LOCI_iCodepage;
            else if ( strcmp( argv[0].strptr, "IALTCODEPAGE") == 0 )         item = LOCI_iAltCodepage;
            else if ( strcmp( argv[0].strptr, "IMACCODEPAGE") == 0 )         item = LOCI_iMacCodepage;
            //else if ( strcmp( argv[0].strptr, "IEBCDICCODEPAGE") == 0 )      item = LOCI_iEbcdicCodepage;
            else if ( strcmp( argv[0].strptr, "SOTHERCODEPAGES") == 0 )      item = LOCI_sOtherCodepages;
            else if ( strcmp( argv[0].strptr, "SSETCODEPAGE") == 0 )         item = LOCI_sSetCodepage;
            else if ( strcmp( argv[0].strptr, "SKEYBOARD") == 0 )            item = LOCI_sKeyboard;
            else if ( strcmp( argv[0].strptr, "SALTKEYBOARD") == 0 )         item = LOCI_sAltKeyboard;
            else if ( strcmp( argv[0].strptr, "SSETKEYBOARD") == 0 )         item = LOCI_sSetKeyboard;
            else if ( strcmp( argv[0].strptr, "SDEBIT") == 0 )               item = LOCI_sDebit;
            else if ( strcmp( argv[0].strptr, "SCREDIT") == 0 )              item = LOCI_sCredit;
            else if ( strcmp( argv[0].strptr, "SLATIN1LOCALE") == 0 )        item = LOCI_sLatin1Locale;
            else if ( strcmp( argv[0].strptr, "WTIMEFORMAT") == 0 )          item = LOCI_wTimeFormat;
            else if ( strcmp( argv[0].strptr, "WSHORTDATE") == 0 )           item = LOCI_wShortDate;
            else if ( strcmp( argv[0].strptr, "WLONGDATE") == 0 )            item = LOCI_wLongDate;
            else if ( strcmp( argv[0].strptr, "JISO3COUNTRYNAME") == 0 )     item = LOCI_jISO3CountryName;
            else if ( strcmp( argv[0].strptr, "JPERCENTPATTERN") == 0 )      item = LOCI_jPercentPattern;
            else if ( strcmp( argv[0].strptr, "JPERCENTSIGN") == 0 )         item = LOCI_jPercentSign;
            else if ( strcmp( argv[0].strptr, "JEXPONENT") == 0 )            item = LOCI_jExponent;
            else if ( strcmp( argv[0].strptr, "JFULLTIMEFORMAT") == 0 )      item = LOCI_jFullTimeFormat;
            else if ( strcmp( argv[0].strptr, "JLONGTIMEFORMAT") == 0 )      item = LOCI_jLongTimeFormat;
            else if ( strcmp( argv[0].strptr, "JSHORTTIMEFORMAT") == 0 )     item = LOCI_jShortTimeFormat;
            else if ( strcmp( argv[0].strptr, "JFULLDATEFORMAT") == 0 )      item = LOCI_jFullDateFormat;
            else if ( strcmp( argv[0].strptr, "JMEDIUMDATEFORMAT") == 0 )    item = LOCI_jMediumDateFormat;
            else if ( strcmp( argv[0].strptr, "JDATETIMEPATTERN") == 0 )     item = LOCI_jDateTimePattern;
            else if ( strcmp( argv[0].strptr, "JERASTRINGS") == 0 )          item = LOCI_jEraStrings;
            else return ( 40 );
        }

        // Now query the item's value
        ulRC = UniQueryLocaleItem( locale, item, &psuValue );
        if ( ulRC == ULS_SUCCESS ) {
            // Query succeeded, now convert the result into the requested codepage
            if ( ulCP == 1200 ) {
                // UCS-2 requested, so simply return the UniChar array as bytes
                ulBytes  = UniStrlen( psuValue ) * sizeof( UniChar );
                pszFinal = (PSZ) malloc( ulBytes );
                j = 0;
                for ( i = 0; i < UniStrlen(psuValue); i++ ) {
                    pszFinal[j++] = BYTE1FROMUNICHAR( psuValue[i] );
                    pszFinal[j++] = BYTE2FROMUNICHAR( psuValue[i] );
                }
                if ( ! SaveResultString( prsResult, pszFinal, ulBytes )) {
                    MAKERXSTRING( *prsResult, "", 0 );
                }
                free( pszFinal );
            } else {
                // Create the UCS-to-target conversion object
                ulRC = CreateUconvObject( &uconvCP, ulCP,
                                          UCS_ENDIAN_SYSTEM, CONTROLS_MAP_DATA, DBCS_PATH_NO, usSub );
                if ( ulRC == ULS_SUCCESS ) {

                    // Convert the string to the target codepage
                    // (Allow up to 4x the length of the original string)
                    stInLength  = UniStrlen( psuValue );
                    stOutLength = stInLength * 4;
                    pszFinal    = (PSZ) malloc( stOutLength + 1 );
                    psuOffset   = psuValue;
                    pszOffset   = pszFinal;
                    stSubs      = 0;
                    memset( pszFinal, 0, stOutLength + 1 );
                    ulRC = UniUconvFromUcs( uconvCP, &psuOffset, &stInLength,
                                            (PVOID *) &pszOffset, &stOutLength, &stSubs );
                    if ( ulRC == ULS_SUCCESS ) {
                        // Return the final converted string
                        if ( ! SaveResultString( prsResult, pszFinal, strlen(pszFinal) )) {
                            MAKERXSTRING( *prsResult, "", 0 );
                        }
                        UniFreeUconvObject( uconvCP );
                    } else {
                        // UniUconvFromUcs failed
                        WriteErrorCode( ulRC, "UniUconvFromUcs");
                        MAKERXSTRING( *prsResult, "", 0 );
                    }
                    free( pszFinal );

                } else {
                    // Failed to create UconvObject
                    WriteErrorCode( ulRC, "UniCreateUconvObject");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
            }
            UniFreeMem( psuValue );

        } else
            WriteErrorCode( ulRC, "UniQueryLocaleItem");

        UniFreeLocaleObject( locale );
    } else {
        WriteErrorCode( ulRC, "UniCreateLocaleObject");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSTransform                                                              *
 *                                                                           *
 * Transform a string according to one of the transformation types defined   *
 * for all locales.  The actual effect is locale-dependent.                  *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. String to be transformed  (REQUIRED)                                 *
 *   2. Name of transformation  (REQUIRED)                                   *
 *   3. Source codepage number  (DEFAULT: current)                           *
 *   4. Name of locale  (DEFAULT: as per current environment settings)       *
 *                                                                           *
 * REXX RETURN VALUE: Transformed string                                     *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSTransform( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    XformObject  xformation;            // transformation object
    LocaleObject locale;                // handle to the requested locale
    UconvObject  uconvCP;               // conversion object
    UniChar      *psuString,            // input UniChar string
                 *psuOffset,
                 *psuFinal,             // transformed output UniChar string
                 *psuXform;             // name of the requested transformation
    PSZ          pszOffset,
                 pszFinal;
    ULONG        ulSourceCP = 0,        // source codepage number
                 ulChars,               // length of input UniChar string
                 ulBytes,               // length of returned string in bytes
                 ulRC = 0,              // return code
                 i, j;                  // counters
    UCHAR        ucFirst,               // first byte of a UCS-2 pair
                 ucSecond;              // second byte of a UCS-2 pair
    size_t       stInLen,
                 stOutLen,
                 stSubs;
    int          iInLength,             // input buffer length (UniTransformStr)
                 iOutLength;            // output buffer length (UniTransformStr)


    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Make sure we have at least two valid arguments
    if ( argc < 2 || !RXVALIDSTRING(argv[0]) || !RXVALIDSTRING(argv[1]) )
        return ( 40 );
    if ( argv[0].strlength < 2 ) return ( 40 );

    // UniCreateTransformObject() requires a lowercase transformation name
    strlwr( argv[1].strptr );

    // Parse the codepage number
    if ( argc >= 3 ) {
        if ( RXVALIDSTRING(argv[2]) && sscanf(argv[2].strptr, "%d", &ulSourceCP) < 1 )
            return ( 40 );
        // For UCS-2 input, make sure we have a valid character...
        if (( ulSourceCP == 1200 ) && ( argv[0].strlength < 2 ))
            return ( 40 );
    }
    // Parse the optional locale argument and create a handle for that locale
    if ( argc >= 4 && RXVALIDSTRING(argv[3]))
        ulRC = UniCreateLocaleObject( UNI_MBS_STRING_POINTER, argv[3].strptr, &locale );
    else
        ulRC = UniCreateLocaleObject( UNI_MBS_STRING_POINTER, "", &locale );

    if ( ulRC == ULS_SUCCESS ) {

        ulRC = CreateUconvObject( &uconvCP, ulSourceCP,
                                  UCS_ENDIAN_SYSTEM, CONTROLS_MAP_DATA, DBCS_PATH_NO, 0 );
        if ( ulRC == ULS_SUCCESS ) {

            if ( ulSourceCP == 1200 ) {
                // Input text is already UCS-2; parse it directly as a UniChar array
                psuString = (UniChar *) malloc( argv[0].strlength + sizeof(UniChar) );
                ulChars = (argv[0].strlength + 1) / 2;
                j = 0;
                for ( i = 0; i < ulChars; i++ ) {
                    ucFirst  = argv[0].strptr[ j++ ];
                    ucSecond = argv[0].strptr[ j++ ];
                    psuString[ i ] = UNICHARFROM2BYTES( ucFirst, ucSecond );
                }
                psuString[ i ] = 0x0000;
            } else {
                // Convert the input text to UCS-2
                stInLen   = argv[0].strlength;
                stOutLen  = stInLen;
                stSubs    = 0;
                psuString = (UniChar *) calloc( stOutLen + 1, sizeof(UniChar) );
                pszOffset = argv[0].strptr;
                psuOffset = psuString;
                ulRC = UniUconvToUcs( uconvCP, (PPVOID) &pszOffset, &stInLen, &psuOffset, &stOutLen, &stSubs );
                if ( ulRC != ULS_SUCCESS ) {
                    WriteErrorCode( ulRC, "UniUconvToUcs");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
            }

            if ( ulRC == ULS_SUCCESS ) {
                psuXform = (UniChar *) calloc( argv[1].strlength + 1, sizeof(UniChar) );
                ulRC = UniStrToUcs( uconvCP, psuXform, argv[1].strptr, argv[1].strlength );
                if ( ulRC != ULS_SUCCESS )
                    mbstowcs( psuXform, argv[1].strptr, argv[1].strlength );

                // Create a transformation object for the requested transformation type
                ulRC = UniCreateTransformObject( locale, psuXform, &xformation );
                if ( ulRC == ULS_SUCCESS ) {

                    // Perform the transformation
                    iInLength  = argv[0].strlength;
                    // (allow 4x the input string length for decomposition)
                    iOutLength = ( iInLength * 4 ) + 1;
                    psuFinal   = (UniChar *) calloc( iOutLength, sizeof(UniChar) );
                    ulRC = UniTransformStr( xformation, psuString, &iInLength, psuFinal, &iOutLength );

                    // Now convert the string back to the original codepage
                    if ( ulRC == ULS_SUCCESS ) {

                        if ( ulSourceCP == 1200 ) {
                            // Converting to UCS-2; simply return the UniChar array as bytes
                            ulBytes  = UniStrlen( psuFinal ) * sizeof( UniChar );
                            pszFinal = (PSZ) malloc( ulBytes );
                            j = 0;
                            for ( i = 0; i < UniStrlen(psuFinal); i++ ) {
                                pszFinal[j++] = BYTE1FROMUNICHAR( psuFinal[i] );
                                pszFinal[j++] = BYTE2FROMUNICHAR( psuFinal[i] );
                            }
                            if ( ! SaveResultString( prsResult, pszFinal, ulBytes )) {
                                MAKERXSTRING( *prsResult, "", 0 );
                            }
                        } else {
                            // Convert the string back to the source codepage
                            // (Allow up to 4x the length of the original string)
                            stInLen   = UniStrlen( psuFinal );
                            stOutLen  = stInLen * 4;
                            pszFinal  = (PSZ) malloc( stOutLen + 1 );
                            psuOffset = psuFinal;
                            pszOffset = pszFinal;
                            stSubs    = 0;
                            memset( pszFinal, 0, stOutLen + 1 );
                            ulRC = UniUconvFromUcs( uconvCP, &psuOffset, &stInLen,
                                                    (PPVOID) &pszOffset, &stOutLen, &stSubs );
                            if ( ulRC == ULS_SUCCESS ) {
                                if ( ! SaveResultString( prsResult, pszFinal, strlen(pszFinal) )) {
                                    MAKERXSTRING( *prsResult, "", 0 );
                                }
                            } else {
                                WriteErrorCode( ulRC, "UniUconvFromUcs");
                                MAKERXSTRING( *prsResult, "", 0 );
                            }
                        }
                        free ( pszFinal );
                    } else {
                        WriteErrorCode( ulRC, "UniTransformStr");
                        MAKERXSTRING( *prsResult, "", 0 );
                    }
                    free( psuFinal );
                    UniFreeTransformObject( xformation );

                } else {
                    WriteErrorCode( ulRC, "UniCreateTransformObject");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
                free( psuXform );

            }
            free( psuString );
            UniFreeUconvObject( uconvCP );

        } else {
            WriteErrorCode( ulRC, "UniCreateUconvObject");
            MAKERXSTRING( *prsResult, "", 0 );
        }
        UniFreeLocaleObject( locale );

    } else {
        WriteErrorCode( ulRC, "UniCreateLocaleObject");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSPutUnicodeClipboard                                                    *
 *                                                                           *
 * Write a string to the clipboard (converted to UCS-2) in "text/unicode"    *
 * format.  (This is the format used by Mozilla, and is also compatible with *
 * various other applications.)                                              *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. String to be written to the clipboard  (REQUIRED)                    *
 *   2. Source codepage number  (DEFAULT: current)                           *
 *   3. Control flag (how to treat byte values 0x00-0x19,0x7F), one of:      *
 *        D = data, treat as control bytes and do not convert (DEFAULT)      *
 *        G = glyphs, treat as glyphs and convert according to codepage      *
 *        C = cdra, treat as control bytes and convert using IBM mapping     *
 *        L = linebreak, treat CR and LF as controls, all others as glyphs   *
 *   4. Path conversion flag (only applies to DBCS codepages), one of:       *
 *        Y = yes, assume string contains a path specifier (DEFAULT)         *
 *        N = no, assume string doesn't contain a path specifier             *
 *                                                                           *
 * REXX RETURN VALUE:  ""                                                    *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSPutUnicodeClipboard( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    UconvObject uconvSource;            // conversion object for source codepage
    UniChar     *psuConverted,          // converted UCS-2 string
                *psuOffset,             // pointer to current UniChar
                *psuShareMem;           // Unicode text in clipboard
    PSZ         pszOffset;              // pointer to current char
    ULONG       ulSourceCP = 0,         // requested source codepage
                ulRC       = 0,         // return code
                ulChars    = 0,         // size in UniChars of input string
                ulPType    = 0,         // process-type flag
                i, j;                   // loop indices
    USHORT      fMap  = 0,              // control-mapping flag
                fPath = 0;              // path-conversion flag
    UCHAR       ucFirst,                // first byte of a UCS-2 pair
                ucSecond;               // second byte of a UCS-2 pair
    size_t      stInLength      = 0,    // length of input buffer (UniUconv*Ucs)
                stOutLength     = 0,    // length of output buffer (UniUconv*Ucs)
                stSubstitutions = 0;    // substitution count (UniUconv*Ucs)
    BOOL        fHabTerm = TRUE;        // terminate HAB ourselves?
    HAB         hab;                    // anchor-block handle (for Win*)
    HMQ         hmq;                    // message-queue handle
    HATOMTBL    hSATbl;                 // handle to system atom table
    ATOM        cf_TextUnicode;         // "text/unicode" clipboard format atom
    PPIB        ppib;                   // process information block
    PTIB        ptib;                   // thread information block


    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Make sure we have at least one valid argument (the input string)
    if ( argc < 1  || ( !RXVALIDSTRING(argv[0]) )) return ( 40 );

    // Other arguments: these are optional, but must be correct if specified
    if ( argc >= 2 ) {
        // second argument: source codepage
        if ( RXVALIDSTRING(argv[1]) && sscanf(argv[1].strptr, "%d", &ulSourceCP) < 1 )
            return ( 40 );
    }
    if ( argc >= 3 ) {
        // third argument: control flag
        if ( RXVALIDSTRING(argv[2]) ) {
            strupr( argv[2].strptr );
            if ( strcspn(argv[2].strptr, "DGCL") > 0 ) return ( 40 );
            switch ( argv[2].strptr[0] ) {
                case 'G': fMap = CONTROLS_MAP_DISPLAY; break;
                case 'C': fMap = CONTROLS_MAP_CDRA;    break;
                case 'L': fMap = CONTROLS_MAP_CRLF;    break;
                default : fMap = CONTROLS_MAP_DATA;    break;
            }
        } else fMap = CONTROLS_MAP_DATA;
    }
    if ( argc >= 4 ) {
        // fourth argument: path flag
        if ( RXVALIDSTRING(argv[3]) ) {
            strupr( argv[3].strptr );
            if ( strcspn(argv[3].strptr, "YN") > 0 ) return ( 40 );
            switch ( argv[3].strptr[0] ) {
                case 'N': fPath = DBCS_PATH_NO;  break;
                default : fPath = DBCS_PATH_YES; break;
            }
        } else fPath = DBCS_PATH_YES;
    }

    // Create the source-to-UCS conversion object
    if ( ulSourceCP != 1200 )
        ulRC = CreateUconvObject( &uconvSource, ulSourceCP, 0, fMap, fPath, 0 );

    if ( ulRC == ULS_SUCCESS ) {

        if ( ulSourceCP == 1200 ) {
            // Input string is already UCS-2; read it directly into a UniChar array
            psuConverted = (UniChar *) malloc( argv[0].strlength + sizeof(UniChar) );
            ulChars = argv[0].strlength / 2;
            j = 0;
            for ( i = 0; i < ulChars; i++ ) {
                ucFirst  = argv[0].strptr[ j++ ];
                ucSecond = argv[0].strptr[ j++ ];
                psuConverted[ i ] = UNICHARFROM2BYTES( ucFirst, ucSecond );
            }
            psuConverted[ i ] = 0x0000;
        } else {
            // Convert the input string to UCS-2
            stInLength   = argv[0].strlength;
            stOutLength  = stInLength;
            psuConverted = (UniChar *) calloc( stOutLength + 1, sizeof(UniChar) );
            pszOffset    = argv[0].strptr;
            psuOffset    = psuConverted;
            memset( psuConverted, 0, (stOutLength+1) * sizeof(UniChar) );
            ulRC = UniUconvToUcs( uconvSource, (PPVOID) &pszOffset, &stInLength,
                                  &psuOffset, &stOutLength, &stSubstitutions     );
        }

        if ( ulRC == ULS_SUCCESS ) {

            // Initialize the PM API
            DosGetInfoBlocks( &ptib, &ppib );
            ulPType = ppib->pib_ultype;
            ppib->pib_ultype = 3;
            hab = WinInitialize( 0 );
            if ( !hab ) {
                fHabTerm = FALSE;
                hab = 1;
            }

            /* Try to create a message-queue if one doesn't exist.  We don't
             * need to check the result, because it could fail if a message
             * queue already exists (in the calling process), which is also OK.
             */
            hmq = WinCreateMsgQueue( hab, 0);

            // Make sure the Unicode clipboard format is registered
            hSATbl = WinQuerySystemAtomTable();
            cf_TextUnicode = WinAddAtom( hSATbl, "text/unicode");

            // Place the UCS-2 string on the clipboard as "text/unicode"
            ulRC = WinOpenClipbrd( hab );
            if ( ulRC ) {
                stOutLength = argv[0].strlength;
                ulRC = DosAllocSharedMem( (PVOID) &psuShareMem, NULL, (stOutLength + 1) * sizeof(UniChar),
                                          PAG_READ | PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE );
                if ( ulRC == 0 ) {
                    memset( psuShareMem, 0, (stOutLength+1) * sizeof(UniChar) );
                    UniStrncpy( psuShareMem, psuConverted, stOutLength );
                    if ( ! WinSetClipbrdData( hab, (ULONG) psuShareMem, cf_TextUnicode, CFI_POINTER ))
                        WriteErrorCode( ERRORIDERROR(WinGetLastError(hab)), "WinSetClipbrdData");
                    else
                        MAKERXSTRING( *prsResult, "", 0 );
                } else {
                    WriteErrorCode( ulRC, "DosAllocSharedMem");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
                WinCloseClipbrd( hab );
            } else {
                WriteErrorCode( ulRC, "WinOpenClipbrd");
                MAKERXSTRING( *prsResult, "", 0 );
            }

            WinDeleteAtom( hSATbl, cf_TextUnicode );
            if ( hmq != NULLHANDLE ) WinDestroyMsgQueue( hmq );
            if ( fHabTerm ) WinTerminate( hab );

            ppib->pib_ultype = ulPType;
            free( psuConverted );
        } else {
            // UniUconvToUcs failed
            WriteErrorCode( ulRC, "UniUconvFromUcs");
            MAKERXSTRING( *prsResult, "", 0 );
        }

        if ( ulSourceCP != 1200 )
            UniFreeUconvObject( uconvSource );

    } else {
        // Failed to create source UconvObject
        WriteErrorCode( ulRC, "UniCreateUconvObject");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ULSGetUnicodeClipboard                                                    *
 *                                                                           *
 * Retrieve a "text/unicode" string from the clipboard if one exists         *
 * (converting it into the target codepage, if applicable).                  *
 *                                                                           *
 * REXX ARGUMENTS:                                                           *
 *   1. Target codepage number  (DEFAULT: current)                           *
 *   2. Substitution character (hex)  (DEFAULT: varies by codepage)          *
 *   3. Control flag (how to treat byte values 0x00-0x19,0x7F), one of:      *
 *        D = data, treat as control bytes and do not convert (DEFAULT)      *
 *        G = glyphs, treat as glyphs and convert according to codepage      *
 *        C = cdra, treat as control bytes and convert using IBM mapping     *
 *        L = linebreak, treat CR and LF as controls, all others as glyphs   *
 *   4. Path conversion flag (only applies to DBCS codepages), one of:       *
 *        Y = yes, assume string contains a path specifier (DEFAULT)         *
 *        N = no, assume string doesn't contain a path specifier             *
 *                                                                           *
 * REXX RETURN VALUE: The converted clipboard string                         *
 * ------------------------------------------------------------------------- */
ULONG APIENTRY ULSGetUnicodeClipboard( PSZ pszName, ULONG argc, RXSTRING argv[], PSZ pszQueue, PRXSTRING prsResult )
{
    UconvObject uconvTarget;            // conversion object for target codepage
    UniChar     *psuClipText,           // UCS-2 string from the clipboard
                *psuOffset;             // pointer to current UniChar
    PSZ         pszFinal,               // converted final string
                pszOffset;              // pointer to current character
    ULONG       ulTargetCP = 0,         // requested target codepage
                ulRC       = 0,         // return code
                ulBytes    = 0,         // size in bytes of output string
                ulChars    = 0,         // size in UniChars of clipboard string
                ulPType    = 0,         // process-type flag
                i, j;                   // loop indices
    USHORT      fMap  = 0,              // control-mapping flag
                fPath = 0,              // path-conversion flag
                usSub = 0;              // substitution character
    size_t      stInLength      = 0,    // length of input buffer (UniUconv*Ucs)
                stOutLength     = 0,    // length of output buffer (UniUconv*Ucs)
                stSubstitutions = 0;    // substitution count (UniUconv*Ucs)
    BOOL        fHabTerm = TRUE;        // terminate HAB ourselves?
    HAB         hab;                    // anchor-block handle (for Win*)
    HMQ         hmq;                    // message-queue handle
    HATOMTBL    hSATbl;                 // handle to system atom table
    ATOM        cf_TextUnicode;         // "text/unicode" clipboard format atom
    PPIB        ppib;                   // process information block
    PTIB        ptib;                   // thread information block


    // Reset the error indicator
    WriteErrorCode( 0, NULL );

    // Arguments: these are all optional, but must be correct if specified
    if ( argc >= 1 ) {
        // first argument: target codepage
        if ( RXVALIDSTRING(argv[0]) && sscanf(argv[0].strptr, "%d", &ulTargetCP) < 1 )
            return ( 40 );
    }
    if ( argc >= 2 ) {
        // second argument: substitution character
        if ( RXVALIDSTRING(argv[1]) && sscanf(argv[1].strptr, "%4X", &usSub ) < 1 )
            return ( 40 );
    }
    if ( argc >= 3 ) {
        // third argument: control flag
        if ( RXVALIDSTRING(argv[2]) ) {
            strupr( argv[2].strptr );
            if ( strcspn(argv[2].strptr, "DGCL") > 0 ) return ( 40 );
            switch ( argv[2].strptr[0] ) {
                case 'G': fMap = CONTROLS_MAP_DISPLAY; break;
                case 'C': fMap = CONTROLS_MAP_CDRA;    break;
                case 'L': fMap = CONTROLS_MAP_CRLF;    break;
                default : fMap = CONTROLS_MAP_DATA;    break;
            }
        } else fMap = CONTROLS_MAP_DATA;
    }
    if ( argc >= 4 ) {
        // fourth argument: path flag
        if ( RXVALIDSTRING(argv[3]) ) {
            strupr( argv[3].strptr );
            if ( strcspn(argv[3].strptr, "YN") > 0 ) return ( 40 );
            switch ( argv[3].strptr[0] ) {
                case 'N': fPath = DBCS_PATH_NO;  break;
                default : fPath = DBCS_PATH_YES; break;
            }
        } else fPath = DBCS_PATH_YES;
    }

    // Initialize the PM API
    DosGetInfoBlocks( &ptib, &ppib );
    ulPType = ppib->pib_ultype;
    ppib->pib_ultype = 3;
    hab = WinInitialize( 0 );
    if ( !hab ) {
        fHabTerm = FALSE;
        hab = 1;
    }

    /* Note: A message-queue must exist before we can access the clipboard.  We
     *       don't actually use the returned value.  In fact, we don't even
     *       verify it, because it could be NULLHANDLE if this function was
     *       called from a PM process (e.g. VX-REXX) - in which case, a message
     *       queue should already exist, and we can proceed anyway.
     */
    hmq = WinCreateMsgQueue( hab, 0 );

    // Make sure the Unicode clipboard format is registered
    hSATbl = WinQuerySystemAtomTable();
    cf_TextUnicode = WinAddAtom( hSATbl, "text/unicode");

    // Open the clipboard
    ulRC = WinOpenClipbrd( hab );
    if ( ulRC ) {
        // Read Unicode text from the clipboard, if available
        if (( psuClipText = (UniChar *) WinQueryClipbrdData( hab, cf_TextUnicode )) != NULL ) {

            ulChars = UniStrlen( psuClipText );
            // Convert it to the target codepage
            if ( ulTargetCP == 1200 ) {
                // "Converting" to UCS-2; simply return the UniChar array as bytes
                ulBytes  = ulChars * sizeof( UniChar );
                pszFinal = (PSZ) malloc( ulBytes );
                j = 0;
                for ( i = 0; i < ulChars; i++ ) {
                    pszFinal[j++] = BYTE1FROMUNICHAR( psuClipText[i] );
                    pszFinal[j++] = BYTE2FROMUNICHAR( psuClipText[i] );
                }
                if ( ! SaveResultString( prsResult, pszFinal, ulBytes )) {
                    MAKERXSTRING( *prsResult, "", 0 );
                }
                free( pszFinal );
            } else {
                // Create the UCS-to-target conversion object
                ulRC = CreateUconvObject( &uconvTarget, ulTargetCP, 0, fMap, fPath, usSub );
                if ( ulRC == ULS_SUCCESS ) {

                    // Convert the string to the target codepage
                    // (Allow up to 4x the length of the Unicode string)
                    stInLength      = ulChars;
                    stOutLength     = stInLength * 4;
                    pszFinal        = (PSZ) malloc( stOutLength + 1 );
                    psuOffset       = psuClipText;
                    pszOffset       = pszFinal;
                    stSubstitutions = 0;
                    memset( pszFinal, 0, stOutLength + 1 );
                    ulRC = UniUconvFromUcs( uconvTarget, &psuOffset, &stInLength,
                                            (PVOID *) &pszOffset, &stOutLength, &stSubstitutions );
                    if ( ulRC == ULS_SUCCESS ) {
                        // Return the final converted string
                        if ( ! SaveResultString( prsResult, pszFinal, strlen(pszFinal) )) {
                            MAKERXSTRING( *prsResult, "", 0 );
                        }
                    } else {
                        // UniUconvFromUcs failed
                        WriteErrorCode( ulRC, "UniUconvFromUcs");
                        MAKERXSTRING( *prsResult, "", 0 );
                    }
                    free( pszFinal );
                    UniFreeUconvObject( uconvTarget );

                } else {
                    // Failed to create target UconvObject
                    WriteErrorCode( ulRC, "UniCreateUconvObject");
                    MAKERXSTRING( *prsResult, "", 0 );
                }
            }
        } else {
            // Either no text exists, or clipboard is not readable
            MAKERXSTRING( *prsResult, "", 0 );
        }

        WinCloseClipbrd( hab );
    } else {
        WriteErrorCode( ulRC, "WinOpenClipbrd");
        MAKERXSTRING( *prsResult, "", 0 );
    }

    WinDeleteAtom( hSATbl, cf_TextUnicode );
    if ( hmq != NULLHANDLE ) WinDestroyMsgQueue( hmq );
    if ( fHabTerm ) WinTerminate( hab );

    ppib->pib_ultype = ulPType;

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ParseUconvMapFlag                                                         *
 *                                                                           *
 * Parses the 'map' flag used by various functions that do conversions.      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   RXSTRING rxstring: Input RXSTRING containing the specified flag. (I)    *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   The requested flag value.                                               *
 * ------------------------------------------------------------------------- */
USHORT ParseUconvMapFlag( RXSTRING rxstring )
{
    USHORT fMap;

    if ( RXVALIDSTRING(rxstring) ) {
        switch ( toupper(rxstring.strptr[0]) ) {
            case 'G': fMap = CONTROLS_MAP_DISPLAY; break;
            case 'C': fMap = CONTROLS_MAP_CDRA;    break;
            case 'L': fMap = CONTROLS_MAP_CRLF;    break;
            case 'D': fMap = CONTROLS_MAP_DATA;    break;
            default : fMap = CONV_FLAG_INVALID;    break;
        }
    } else fMap = CONTROLS_MAP_DATA;
    return ( fMap );
}


/* ------------------------------------------------------------------------- *
 * ParseUconvPathFlag                                                        *
 *                                                                           *
 * Parses the 'path' flag used by various functions that do conversions.     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   RXSTRING rxstring: Input RXSTRING containing the specified flag (I).    *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   The requested flag value.                                               *
 * ------------------------------------------------------------------------- */
USHORT ParseUconvPathFlag( RXSTRING rxstring )
{
    USHORT fPath;

    if ( RXVALIDSTRING(rxstring) ) {
        switch ( toupper(rxstring.strptr[0]) ) {
            case 'N': fPath = DBCS_PATH_NO;      break;
            case 'Y': fPath = DBCS_PATH_YES;     break;
            default : fPath = CONV_FLAG_INVALID; break;
        }
    } else fPath = DBCS_PATH_YES;
    return ( fPath );
}


/* ------------------------------------------------------------------------- *
 * CreateUconvObject                                                         *
 *                                                                           *
 * Creates a conversion object (UconvObject) for the specified codepage with *
 * the specified modifiers.  (This object should be freed when no longer     *
 * needed using UniFreeUconvObject.)                                         *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   UconvObject *uco     : Pointer to the UconvObject to be created (O)     *
 *   ULONG       ulCP     : The requested codepage (I)                       *
 *   USHORT      fEndian  : The endian type to use (I)                       *
 *   USHORT      fMap     : The control-character mapping to use (I)         *
 *   USHORT      fPath    : The DBCS path flag to use (I)                    *
 *   UCHAR       ucSubChar: The codepage substitution character to use (I)   *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   Return code from UniCreateUconvObject.                                  *
 * ------------------------------------------------------------------------- */
ULONG CreateUconvObject( UconvObject *uco, ULONG ulCP, USHORT fEndian, USHORT fMap, USHORT fPath, USHORT ucSubChar )
{
    UniChar suCP[ US_CPSPEC_MAXZ ],      // conversion specifier
            suSub[ 3 ];                  // used to store subchar modifier
    UCHAR   szSub[ 3 ];                  // used to generate subchar modifier
    ULONG   ulRC;                        // return code
//  uconv_attribute_t ucattr;
//  int i;

    memset( suCP, 0, sizeof(suCP) );

    // Generate the codepage name string
    if ( ulCP == 0 ) suCP[ 0 ] = L'@';
    else {
        ulRC = UniMapCpToUcsCp( ulCP, suCP, 12 );
        if ( ulRC != ULS_SUCCESS ) return ( ulRC );
        UniStrcat( suCP, (UniChar *) L"@");
    }

    // Generate the conversion modifier string
    switch ( fEndian ) {
        case UCS_ENDIAN_BIG:    UniStrcat( suCP, (UniChar *) L"endian=big,");    break;
        case UCS_ENDIAN_SYSTEM: UniStrcat( suCP, (UniChar *) L"endian=system,"); break;
        default:                                                                 break;
    }
    switch ( fMap ) {
        case CONTROLS_MAP_DISPLAY: UniStrcat( suCP, (UniChar *) L"map=display,"); break;
        case CONTROLS_MAP_CDRA:    UniStrcat( suCP, (UniChar *) L"map=cdra,");    break;
        case CONTROLS_MAP_CRLF:    UniStrcat( suCP, (UniChar *) L"map=crlf,");    break;
        case CONTROLS_MAP_DATA:    UniStrcat( suCP, (UniChar *) L"map=data,");    break;
        default:                                                                  break;
    }
    switch ( fPath ) {
        case DBCS_PATH_NO:  UniStrcat( suCP, (UniChar *) L"path=no");  break;
        case DBCS_PATH_YES: UniStrcat( suCP, (UniChar *) L"path=yes"); break;
        default:                                                       break;
    }
    if ( ucSubChar > 0 ) {
        sprintf( szSub, "%02X", ucSubChar );
        suSub[0] = (UniChar) szSub[0];
        suSub[1] = (UniChar) szSub[1];
        suSub[2] = 0x0000;
        UniStrcat( suCP, (UniChar *) L",subchar=\\x");
        UniStrcat( suCP, suSub );
    }

    // Create the conversion object
//    printf("\n[Conversion: \"%ls\"]\n", suCP );
    ulRC = UniCreateUconvObject( suCP, uco );
    return ( ulRC );
}


/* ------------------------------------------------------------------------- *
 * SaveResultString                                                          *
 *                                                                           *
 * Writes new string contents to the specified RXSTRING, allocating any      *
 * additional memory that may be required.  If the string to be written has  *
 * zero length, nothing is done.                                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PRXSTRING prsResult: Pointer to an existing RXSTRING for writing.       *
 *   PCH       pchBytes : The string contents to write to prsResult.         *
 *   ULONG     ulBytes  : The number of bytes in pchBytes to write.          *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if prsResult was successfully updated.  FALSE otherwise.           *
 * ------------------------------------------------------------------------- */
BOOL SaveResultString( PRXSTRING prsResult, PCH pchBytes, ULONG ulBytes )
{
    ULONG ulRC;
    PCH   pchNew;

    if ( ulBytes == 0 ) return ( FALSE );
    if ( ulBytes > 256 ) {
        // REXX provides 256 bytes by default; allocate more if necessary
        ulRC = DosAllocMem( (PVOID) &pchNew, ulBytes, PAG_WRITE | PAG_COMMIT );
        if ( ulRC != 0 ) {
            WriteErrorCode( ulRC, "DosAllocMem");
            return ( FALSE );
        }
        DosFreeMem( prsResult->strptr );
        prsResult->strptr = pchNew;
    }
    memcpy( prsResult->strptr, pchBytes, ulBytes );
    prsResult->strlength = ulBytes;

    return ( TRUE );
}


/* ------------------------------------------------------------------------- *
 * WriteStemElement                                                          *
 *                                                                           *
 * Creates a stem element (compound variable) in the calling REXX program    *
 * using the REXX shared variable pool interface.                            *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ   pszStem  : The name of the stem (before the '.')                  *
 *   ULONG ulIndex  : The number of the stem element (after the '.')         *
 *   PSZ   pszValue : The value to write to the compound variable.           *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void WriteStemElement( PSZ pszStem, ULONG ulIndex, PSZ pszValue )
{
    SHVBLOCK shvVar;                   // REXX shared variable pool block
    ULONG    ulRc;
    CHAR     szCompoundName[ US_COMPOUND_MAXZ ],
             szValue[ ULS_LNAMEMAX ];

    sprintf( szCompoundName, "%s.%d", pszStem, ulIndex );
    strncpy( szValue, pszValue, ULS_LNAMEMAX );
    MAKERXSTRING( shvVar.shvname,  szCompoundName, strlen(szCompoundName) );
    MAKERXSTRING( shvVar.shvvalue, szValue,        strlen(szValue) );
    shvVar.shvnamelen  = RXSTRLEN( shvVar.shvname );
    shvVar.shvvaluelen = RXSTRLEN( shvVar.shvvalue );
    shvVar.shvcode     = RXSHV_SYSET;
    shvVar.shvnext     = NULL;
    ulRc = RexxVariablePool( &shvVar );
    if ( ulRc > 1 )
        printf("Unable to set %s: rc = %d\n", shvVar.shvname.strptr, shvVar.shvret );

}


/* ------------------------------------------------------------------------- *
 * WriteErrorCode                                                            *
 *                                                                           *
 * Writes an error code to a special variable in the calling REXX program    *
 * using the REXX shared variable pool interface.  This is used to return    *
 * API error codes to the REXX program, since the REXX functions themselves  *
 * normally return string values.                                            *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   ULONG ulError   : The error code returned by the failing API call.      *
 *   PSZ   pszContext: A string describing the API call that failed.         *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void WriteErrorCode( ULONG ulError, PSZ pszContext )
{
    SHVBLOCK shvVar;                   // REXX shared variable pool block
    ULONG    ulRc;
    CHAR     szErrorText[ US_ERRSTR_MAXZ ];

    if ( pszContext == NULL )
        sprintf( szErrorText, "%X", ulError );
    else
        sprintf( szErrorText, "%X: %s", ulError, pszContext );
    MAKERXSTRING( shvVar.shvname,  SZ_ERROR_NAME, strlen(SZ_ERROR_NAME) );
    MAKERXSTRING( shvVar.shvvalue, szErrorText,   strlen(szErrorText) );
    shvVar.shvnamelen  = RXSTRLEN( shvVar.shvname );
    shvVar.shvvaluelen = RXSTRLEN( shvVar.shvvalue );
    shvVar.shvcode     = RXSHV_SYSET;
    shvVar.shvnext     = NULL;
    ulRc = RexxVariablePool( &shvVar );
    if ( ulRc > 1 )
        printf("Unable to set %s: rc = %d\n", shvVar.shvname.strptr, shvVar.shvret );
}


