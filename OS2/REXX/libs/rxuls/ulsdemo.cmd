/* ULSDEMO.C
 * Demonstrate various RxULS functions.
 */

/* ULSLoadFuncs(): register the RxULS library.
 */
CALL RxFuncAdd 'ULSLoadFuncs', 'RXULS', 'ULSLoadFuncs'
CALL ULSLoadFuncs


/* ULSVersion(): query the library version.
 */
SAY 'Using RXULS.DLL version:' ULSVersion()
SAY


/* Sample string containing a couple of non-ASCII characters.  This is encoded
 * in codepage 850.
 */
string850 = 'The train from Trois-RiviŠres to Montr‚al.'
SAY 'Input string (codepage 850): ' string850


/* ULSConvertCodepage(): convert the string to another codepage.
 * In this example, we convert it to UTF-8 (which is codepage 1208).
 */
stringutf = ULSConvertCodepage( string850, 850, 1208 )
IF ULSERR \= '0' THEN
    SAY ULSERR
ELSE
    SAY 'UTF-8 string (codepage 1208):' stringutf

/* We can also convert to UCS-2 (codepage 1200) - even though it isn't really
 * useful for direct output.
 */
stringucs = ULSConvertCodepage( string850, 850, 1200 )
IF ULSERR \= '0' THEN
    SAY ULSERR
ELSE
    SAY 'UCS-2 string (codepage 1200):' stringucs
SAY


/* ULSTransform(): transform a string.
 * Here we demonstrate how to transform the string to uppercase.  Notice that
 * the accented characters are translated correctly.
 */
stringupper = ULSTransform( string850, 'upper')
IF ULSERR \= '0' THEN
    SAY ULSERR
ELSE
    SAY 'Uppercase (using ULSTransform):' stringupper

/* By contrast, REXX's built-in TRANSLATE() function doesn't know how to
 * convert non-ASCII characters (so leaves them unchanged).
 */
SAY 'Uppercase (using TRANSLATE):   ' TRANSLATE( string850 )
SAY


/* ULSFindAttr(): search a string for a particular character attribute.
 * In this example, we search for the first non-ASCII character.  This requires
 * a negative search, i.e. return the position of the first character which does
 * NOT have the 'ascii' attribute.
 */
nacpos = ULSFindAttr( string850, 'ascii',,, 'F')
IF ULSERR \= '0' THEN
    SAY ULSERR
ELSE
    SAY 'The first non-ASCII character is at position' nacpos'.'
SAY


/* ULSQueryLocaleItem(): query the value of a locale property.
 * Here we query the current locale's name and default language (in English).
 */
locname = ULSQueryLocaleItem('sName')
IF ULSERR \= '0' THEN SAY ULSERR
ELSE DO
    lngname = ULSQueryLocaleItem('sEngLanguage')
    IF ULSERR \= '0' THEN SAY ULSERR
    ELSE
        SAY 'The current locale is' locname'.  The default language is' lngname'.'
END


/* ULSDropFuncs(): deregister all RxULS functions.
 */
CALL ULSDropFuncs

RETURN 0

