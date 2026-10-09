/*!
 * @file def.c
 *
 * @brief Implementation of the .DEF parser.
 *
 * Reads the module-definition file understood by IBM LINK386.
 * See def.h for the public API.
 *
 * @par Parsing model
 * The file is read line by line. Comments start with a semicolon
 * and extend to the end of the line. Lines are trimmed and
 * dispatched on the first keyword. Multi-line sections (IMPORTS,
 * EXPORTS, SEGMENTS) collect the lines that follow their opening
 * keyword until the next top-level keyword appears.
 *
 * @par Statement order
 * LINK386 Reference, "Module Statement Rules": LIBRARY, NAME,
 * VIRTUAL DEVICE and PHYSICAL DEVICE must precede all other
 * statements. Violations are counted in the parse-error counter.
 *
 * @par Mutual exclusion
 * LINK386 Reference, "LIBRARY Statement" and "NAME Statement":
 * the four identifying statements are mutually exclusive.
 *
 * @par Numeric arguments
 * LINK386 Reference, "Entering Numeric Arguments": numbers are
 * C-style, decimal by default, octal with a leading 0, hexadecimal
 * with a leading 0x. Signs are not defined and are rejected.
 * Overflow beyond ULONG_MAX is detected via errno.
 *
 * @par References
 *   - IBM OS/2 Warp 4.5 Toolkit, Tools Reference, LINK386.
 *   - SAA CPI C Reference, Level 2 (SC09-1308-02, Sep 1991).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "os2types.h"
#include "os2err.h"
#include "vector.h"
#include "def.h"

/*!
 * @def DEF_MAGIC
 * @brief Validation word stored in every live DEFFILE handle.
 */
#define DEF_MAGIC        0x44454646UL   /* "DEFF" */

/*!
 * @def DEF_CURSOR_MAGIC
 * @brief Validation word stored in every live cursor.
 */
#define DEF_CURSOR_MAGIC 0x44454643UL   /* "DEFC" */

/*!
 * @def DEF_MAX_NAME
 * @brief Maximum length of a name string.
 */
#define DEF_MAX_NAME     255UL

/*!
 * @def DEF_MAX_DESC
 * @brief Maximum length of a DESCRIPTION text.
 */
#define DEF_MAX_DESC     512UL

/*!
 * @def DEF_MAX_LINE
 * @brief Maximum length of one physical input line.
 */
#define DEF_MAX_LINE     1024UL

/* ==================================================================
 * Private record types
 * ================================================================== */

/*!
 * @struct _DEF_IMP
 * @brief Private representation of one IMPORTS entry.
 */
typedef struct _DEF_IMP {
    PSZ  pszInternal;
    PSZ  pszModule;
    PSZ  pszExternal;
    BOOL fExplicit;
    BOOL fOrdinal;
} DEF_IMP;

/*! @brief Pointer to _DEF_IMP. */
typedef DEF_IMP *PDEF_IMP;

/*!
 * @struct _DEF_EXP
 * @brief Private representation of one EXPORTS entry.
 */
typedef struct _DEF_EXP {
    PSZ    pszName;
    PSZ    pszInternal;
    USHORT usOrdinal;
    BOOL   fHasOrdinal;
    BOOL   fResident;
    USHORT usPWords;
    BOOL   fHasPWords;
} DEF_EXP;

/*! @brief Pointer to _DEF_EXP. */
typedef DEF_EXP *PDEF_EXP;

/*!
 * @struct _DEF_SEGMENT
 * @brief Private representation of one SEGMENTS entry.
 */
typedef struct _DEF_SEGMENT {
    PSZ    pszName;
    PSZ    pszClass;
    ULONG  ulLoad;
    ULONG  ulRW;
    ULONG  ulRX;
    ULONG  ulIOPL;
    ULONG  ulConf;
    BOOL   fMixed;
    BOOL   fAlias;
    ULONG  ulShare;
} DEF_SEGMENT;

/*! @brief Pointer to _DEF_SEGMENT. */
typedef DEF_SEGMENT *PDEF_SEGMENT;

/*!
 * @struct _DEF_CODEDATA
 * @brief Default attributes shared by CODE and DATA statements.
 */
typedef struct _DEF_CODEDATA {
    ULONG ulLoad;
    ULONG ulRW;
    ULONG ulRX;
    ULONG ulIOPL;
    ULONG ulConf;
    ULONG ulSharing;
    ULONG ulShare;
    BOOL  fPresent;
} DEF_CODEDATA;

/*!
 * @struct _DEFFILE
 * @brief Internal representation behind HDEFFILE.
 *
 * Not exposed to callers. def.h declares the handle as HANDLE, so
 * the layout of this structure may change freely.
 */
typedef struct _DEFFILE {
    ULONG        ulMagic;

    ULONG        ulModuleType;
    PSZ          pszModuleName;

    ULONG        ulInit;
    ULONG        ulTerm;
    ULONG        ulAppType;

    PSZ          pszDescription;
    ULONG        ulExeType;

    ULONG        ulBase;
    BOOL         fHasBase;

    ULONG        ulHeapSize;
    BOOL         fHeapSizeMaxVal;
    BOOL         fHasHeapSize;

    ULONG        ulStackSize;
    BOOL         fHasStackSize;

    PSZ          pszOldName;
    PSZ          pszStubName;
    BOOL         fProtMode;

    ULONG        ulParseErrors;
    BOOL         fSeenAnyStatement;
    BOOL         fSeenIdentifier;

    DEF_CODEDATA stCode;
    DEF_CODEDATA stData;

    HVECTOR      hSegments;   /* vector of PDEF_SEGMENT */
    HVECTOR      hImports;    /* vector of PDEF_IMP     */
    HVECTOR      hExports;    /* vector of PDEF_EXP     */
} DEFFILE;

/*! @brief Pointer to _DEFFILE. */
typedef DEFFILE *PDEFFILE;

/*!
 * @struct _DEF_CURSOR
 * @brief Internal representation behind HDEFIMPORT, HDEFEXPORT and
 *        HDEFSEGMENT.
 *
 * A cursor is a thin index into the owning handle's vector,
 * together with a back-pointer used for lifetime validation.
 */
typedef struct _DEF_CURSOR {
    ULONG   ulMagic;
    PDEFFILE pOwner;
    ULONG   ulIndex;
} DEF_CURSOR;

/*! @brief Pointer to _DEF_CURSOR. */
typedef DEF_CURSOR *PDEF_CURSOR;

/* ==================================================================
 * Handle validation
 * ================================================================== */

/*!
 * @brief Return the handle if it carries DEF_MAGIC, else NULL.
 *
 * @param[in] hDef  Handle from DefOpen. May be NULLHANDLE.
 *
 * @return Pointer to DEFFILE, or NULL.
 *
 * @retval NULL  Handle is NULLHANDLE or magic does not match.
 * @retval pDef  Handle is valid.
 */
static PDEFFILE DefLive(HDEFFILE hDef)
{
    PDEFFILE pDef = (PDEFFILE)hDef;
    if (!pDef) return NULL;
    if (pDef->ulMagic != DEF_MAGIC) return NULL;
    return pDef;
}

/*!
 * @brief Return the cursor if it carries DEF_CURSOR_MAGIC, else NULL.
 *
 * @param[in] hCursor  Cursor. May be NULLHANDLE.
 *
 * @return Pointer to DEF_CURSOR, or NULL.
 *
 * @retval NULL     Cursor is NULLHANDLE or magic does not match.
 * @retval pCursor  Cursor is valid.
 */
static PDEF_CURSOR DefCursorLive(HANDLE hCursor)
{
    PDEF_CURSOR pCursor = (PDEF_CURSOR)hCursor;
    if (!pCursor) return NULL;
    if (pCursor->ulMagic != DEF_CURSOR_MAGIC) return NULL;
    return pCursor;
}

/* ==================================================================
 * Small helpers
 * ================================================================== */

/*!
 * @brief Duplicate a non-empty string onto the heap.
 *
 * Empty strings return NULL rather than a heap allocation, because
 * every caller treats an empty field as absent.
 *
 * @param[in] psz  Source string. May be NULL or empty.
 *
 * @return Heap copy, or NULL.
 *
 * @retval NULL  Source is NULL or empty, or allocation failed.
 * @retval p     Fresh heap copy of the source.
 */
static PSZ DefDup(PCSZ psz)
{
    if (!psz || !*psz) return NULL;
    return strdup(psz);
}

/*!
 * @brief Trim leading and trailing whitespace in place.
 *
 * @param[in,out] psz  Buffer to trim. Not NULL.
 */
static void DefTrim(char *psz)
{
    char *p = psz;
    char *e;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != psz) memmove(psz, p, strlen(p) + 1);
    e = psz + strlen(psz);
    while (e > psz && isspace((unsigned char)e[-1])) *--e = '\0';
}

/*!
 * @brief Extract a single-quoted token.
 *
 * @param[in,out] ppsz    Pointer to the parse position. Updated.
 * @param[out]    pszOut  Output buffer. Not NULL.
 * @param[in]     cbOut   Size of pszOut, including room for NUL.
 *
 * @return TRUE on success, FALSE on missing quotes.
 *
 * @retval TRUE   Token extracted; *ppsz advanced past closing
 *                quote.
 * @retval FALSE  No opening quote, no closing quote within cbOut,
 *                or token would overflow pszOut.
 */
static BOOL DefParseQuoted(PCSZ *ppsz, char *pszOut, size_t cbOut)
{
    PCSZ p = *ppsz;
    size_t n = 0;

    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != '\'') return FALSE;
    p++;
    while (*p && *p != '\'' && n + 1 < cbOut) {
        pszOut[n++] = *p++;
    }
    if (*p != '\'') return FALSE;
    pszOut[n] = '\0';
    *ppsz = p + 1;
    return TRUE;
}

/*!
 * @brief Test whether the rest of the string is empty or whitespace.
 *
 * @param[in] p  Position in the parse stream. Not NULL.
 *
 * @return TRUE if only whitespace remains, FALSE otherwise.
 *
 * @retval TRUE   Only whitespace or end-of-string at p.
 * @retval FALSE  Non-whitespace character found.
 */
static BOOL DefIsEndOfLine(PCSZ p)
{
    while (*p && isspace((unsigned char)*p)) p++;
    return (*p == '\0');
}

/*!
 * @brief Parse a non-negative integer using C-style radix.
 *
 * @par Reference
 *   LINK386 Reference, "Entering Numeric Arguments".
 *
 * @param[in,out] ppsz      Pointer to the parse position. Updated.
 * @param[out]    pulValue  Receiver. Not NULL.
 *
 * @return TRUE on success, FALSE on malformed input.
 *
 * @retval TRUE   Number parsed; *ppsz advanced past it.
 * @retval FALSE  Sign, non-digit, empty string, or overflow.
 */
static BOOL DefParseNumber(PCSZ *ppsz, PULONG pulValue)
{
    PCSZ p = *ppsz;
    char *pEnd = NULL;
    unsigned long v;

    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) return FALSE;
    if (*p == '+' || *p == '-') return FALSE;
    if (!isdigit((unsigned char)*p)) return FALSE;

    errno = 0;
    v = strtoul(p, &pEnd, 0);
    if (pEnd == p) return FALSE;
    if (errno == ERANGE) return FALSE;

    *pulValue = (ULONG)v;
    *ppsz = pEnd;
    return TRUE;
}

/*!
 * @brief Fetch the next whitespace-separated word.
 *
 * @param[in,out] ppsz    Pointer to the parse position. Updated.
 * @param[out]    pszOut  Output buffer. Not NULL.
 * @param[in]     cbOut   Size of pszOut, including room for NUL.
 *
 * @return TRUE if a word was copied, FALSE at end of line.
 *
 * @retval TRUE   Word copied; *ppsz advanced.
 * @retval FALSE  End of line reached.
 */
static BOOL DefNextWord(PCSZ *ppsz, char *pszOut, size_t cbOut)
{
    PCSZ p = *ppsz;
    size_t n = 0;

    while (*p && isspace((unsigned char)*p)) p++;
    if (!*p) return FALSE;
    while (*p && !isspace((unsigned char)*p) && n + 1 < cbOut) {
        pszOut[n++] = *p++;
    }
    pszOut[n] = '\0';
    *ppsz = p;
    return TRUE;
}

/* ==================================================================
 * Top-level statement recognition
 * ================================================================== */

/*!
 * @enum _DEFSTMT
 * @brief Recognized top-level statement kinds.
 *
 * @par Reference
 *   LINK386 Reference, "Module Statements".
 */
enum {
    STMT_NONE = 0,
    STMT_BASE,
    STMT_CODE,
    STMT_DATA,
    STMT_DESCRIPTION,
    STMT_EXETYPE,
    STMT_EXPORTS,
    STMT_HEAPSIZE,
    STMT_IMPORTS,
    STMT_LIBRARY,
    STMT_NAME,
    STMT_OLD,
    STMT_PHYSICAL_DEVICE,
    STMT_PROTMODE,
    STMT_SEGMENTS,
    STMT_STACKSIZE,
    STMT_STUB,
    STMT_VIRTUAL_DEVICE
};

/*!
 * @brief Test whether a statement kind is an identifying statement.
 *
 * @par Reference
 *   LINK386 Reference, "Module Statement Rules": "If you use a
 *   NAME, LIBRARY, VIRTUAL DEVICE, or PHYSICAL DEVICE statement,
 *   it must precede all other statements."
 *
 * @param[in] nKind  Statement kind.
 *
 * @return TRUE if identifying, FALSE otherwise.
 *
 * @retval TRUE   nKind is LIBRARY, NAME, PHYSICAL DEVICE or
 *                VIRTUAL DEVICE.
 * @retval FALSE  Any other kind.
 */
static BOOL DefIsIdentifying(int nKind)
{
    return (nKind == STMT_LIBRARY ||
            nKind == STMT_NAME ||
            nKind == STMT_PHYSICAL_DEVICE ||
            nKind == STMT_VIRTUAL_DEVICE);
}

/*!
 * @brief Determine the top-level statement a line begins with.
 *
 * @param[in]  pszLine   Trimmed line. Not NULL.
 * @param[out] ppszRest  Pointer past the keyword. Not NULL.
 *
 * @return STMT_* constant, or STMT_NONE.
 *
 * @retval STMT_*     Statement recognized.
 * @retval STMT_NONE  No keyword matches.
 */
static int DefRecognizeStatement(PCSZ pszLine, PCSZ *ppszRest)
{
    static const struct { PCSZ pszKw; int nKind; } aKw[] = {
        { "BASE",            STMT_BASE            },
        { "CODE",            STMT_CODE            },
        { "DATA",            STMT_DATA            },
        { "DESCRIPTION",     STMT_DESCRIPTION     },
        { "EXETYPE",         STMT_EXETYPE         },
        { "EXPORTS",         STMT_EXPORTS         },
        { "HEAPSIZE",        STMT_HEAPSIZE        },
        { "IMPORTS",         STMT_IMPORTS         },
        { "LIBRARY",         STMT_LIBRARY         },
        { "NAME",            STMT_NAME            },
        { "OLD",             STMT_OLD             },
        { "PHYSICAL DEVICE", STMT_PHYSICAL_DEVICE },
        { "PROTMODE",        STMT_PROTMODE        },
        { "SEGMENTS",        STMT_SEGMENTS        },
        { "STACKSIZE",       STMT_STACKSIZE       },
        { "STUB",            STMT_STUB            },
        { "VIRTUAL DEVICE",  STMT_VIRTUAL_DEVICE  },
        { NULL, 0 }
    };
    int i;
    for (i = 0; aKw[i].pszKw; i++) {
        size_t n = strlen(aKw[i].pszKw);
        if (strnicmp(pszLine, aKw[i].pszKw, n) == 0 &&
            (pszLine[n] == '\0' || isspace((unsigned char)pszLine[n]))) {
            *ppszRest = pszLine + n;
            return aKw[i].nKind;
        }
    }
    return STMT_NONE;
}

/* ==================================================================
 * Attribute parsers
 * ================================================================== */

/*!
 * @brief Parse a load-timing attribute.
 *
 * @par Reference
 *   LINK386 Reference, "Load Code Attributes", "Load Data
 *   Attributes", "Load Segments Attributes".
 *
 * @param[in]  pszWord  Attribute word to test.
 * @param[out] pulLoad  Updated in place; left unchanged when the
 *                      word is not recognized.
 */
static void DefParseLoad(PCSZ pszWord, PULONG pulLoad)
{
    if (stricmp(pszWord, "PRELOAD") == 0)
        *pulLoad = DEF_LOAD_PRELOAD;
    else if (stricmp(pszWord, "LOADONCALL") == 0)
        *pulLoad = DEF_LOAD_LOADONCALL;
}

/*!
 * @brief Parse a Read/Write access attribute.
 *
 * @par Reference
 *   LINK386 Reference, "Read/Write Data Attributes", "Read/Write
 *   Segments Attributes".
 *
 * @param[in]  pszWord  Attribute word to test.
 * @param[out] pulRW    Updated in place; left unchanged when the
 *                      word is not recognized.
 */
static void DefParseRW(PCSZ pszWord, PULONG pulRW)
{
    if (stricmp(pszWord, "READONLY") == 0)
        *pulRW = DEF_RW_READONLY;
    else if (stricmp(pszWord, "READWRITE") == 0)
        *pulRW = DEF_RW_READWRITE;
}

/*!
 * @brief Parse a Read/Execute access attribute.
 *
 * @par Reference
 *   LINK386 Reference, "Read/Execute Code Attributes",
 *   "Read/Execute Segments Attributes".
 *
 * @param[in]  pszWord  Attribute word to test.
 * @param[out] pulRX    Updated in place; left unchanged when the
 *                      word is not recognized.
 */
static void DefParseRX(PCSZ pszWord, PULONG pulRX)
{
    if (stricmp(pszWord, "EXECUTEONLY") == 0)
        *pulRX = DEF_RX_EXECUTEONLY;
    else if (stricmp(pszWord, "EXECUTEREAD") == 0)
        *pulRX = DEF_RX_EXECUTEREAD;
}

/*!
 * @brief Parse an I/O privilege attribute.
 *
 * @par Reference
 *   LINK386 Reference, "I/O Privilege Code Attributes",
 *   "LINK386 -I/O Privilege Data Attributes",
 *   "I/O Privilege Segments Attributes".
 *
 * @param[in]  pszWord  Attribute word to test.
 * @param[out] pulIOPL  Updated in place; left unchanged when the
 *                      word is not recognized.
 */
static void DefParseIOPL(PCSZ pszWord, PULONG pulIOPL)
{
    if (stricmp(pszWord, "IOPL") == 0)
        *pulIOPL = DEF_IOPL_YES;
    else if (stricmp(pszWord, "NOIOPL") == 0)
        *pulIOPL = DEF_IOPL_NO;
}

/*!
 * @brief Parse a conforming attribute.
 *
 * @par Reference
 *   LINK386 Reference, "Conforming Code Attributes",
 *   "Conforming Segments Attributes".
 *
 * @param[in]  pszWord  Attribute word to test.
 * @param[out] pulConf  Updated in place; left unchanged when the
 *                      word is not recognized.
 */
static void DefParseConf(PCSZ pszWord, PULONG pulConf)
{
    if (stricmp(pszWord, "CONFORMING") == 0)
        *pulConf = DEF_CONF_CONFORMING;
    else if (stricmp(pszWord, "NONCONFORMING") == 0)
        *pulConf = DEF_CONF_NONCONFORMING;
}

/*!
 * @brief Parse an automatic data segment sharing attribute.
 *
 * @par Reference
 *   LINK386 Reference, "Sharing Data Attributes".
 *
 * @param[in]  pszWord     Attribute word to test.
 * @param[out] pulSharing  Updated in place; left unchanged when the
 *                         word is not recognized.
 */
static void DefParseSharing(PCSZ pszWord, PULONG pulSharing)
{
    if (stricmp(pszWord, "NONE") == 0)
        *pulSharing = DEF_SHARING_NONE;
    else if (stricmp(pszWord, "SINGLE") == 0)
        *pulSharing = DEF_SHARING_SINGLE;
    else if (stricmp(pszWord, "MULTIPLE") == 0)
        *pulSharing = DEF_SHARING_MULTIPLE;
}

/*!
 * @brief Parse a shareable data segment attribute.
 *
 * @par Reference
 *   LINK386 Reference, "Shareable Data Attributes",
 *   "Specify that Segment is Shared".
 *
 * @param[in]  pszWord   Attribute word to test.
 * @param[out] pulShare  Updated in place; left unchanged when the
 *                       word is not recognized.
 */
static void DefParseShare(PCSZ pszWord, PULONG pulShare)
{
    if (stricmp(pszWord, "SHARED") == 0)
        *pulShare = DEF_SHARE_SHARED;
    else if (stricmp(pszWord, "NONSHARED") == 0)
        *pulShare = DEF_SHARE_NONSHARED;
}

/* ==================================================================
 * CODE / DATA
 * ================================================================== */

/*!
 * @brief Parse a CODE statement's attributes.
 *
 * @par Reference
 *   LINK386 Reference, "CODE Statement" and its four attribute
 *   pages.
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the CODE keyword. Not NULL.
 */
static void DefParseCodeLine(PDEFFILE pDef, PCSZ pszRest)
{
    char  achWord[64];
    PCSZ  p = pszRest;

    pDef->stCode.fPresent = TRUE;
    while (DefNextWord(&p, achWord, sizeof(achWord))) {
        DefParseLoad(achWord,    &pDef->stCode.ulLoad);
        DefParseRX(achWord,      &pDef->stCode.ulRX);
        DefParseIOPL(achWord,    &pDef->stCode.ulIOPL);
        DefParseConf(achWord,    &pDef->stCode.ulConf);
    }
}

/*!
 * @brief Parse a DATA statement's attributes.
 *
 * @par Reference
 *   LINK386 Reference, "DATA Statement" and its five attribute
 *   pages.
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the DATA keyword. Not NULL.
 */
static void DefParseDataLine(PDEFFILE pDef, PCSZ pszRest)
{
    char  achWord[64];
    PCSZ  p = pszRest;

    pDef->stData.fPresent = TRUE;
    while (DefNextWord(&p, achWord, sizeof(achWord))) {
        DefParseLoad(achWord,    &pDef->stData.ulLoad);
        DefParseRW(achWord,      &pDef->stData.ulRW);
        DefParseSharing(achWord, &pDef->stData.ulSharing);
        DefParseShare(achWord,   &pDef->stData.ulShare);
        DefParseIOPL(achWord,    &pDef->stData.ulIOPL);
    }
}

/* ==================================================================
 * SEGMENTS line parser
 * ================================================================== */

/*!
 * @brief Test whether a word is a top-level keyword.
 *
 * @par Reference
 *   LINK386 Reference, "SEGMENTS Statement": "The quotation marks
 *   are required if <segmentname> conflicts with a module
 *   definition keyword, such as CODE or DATA."
 *
 * @param[in] pszWord  Word to test.
 *
 * @return TRUE if reserved, FALSE otherwise.
 *
 * @retval TRUE   Word is a recognized top-level keyword.
 * @retval FALSE  Word is not a keyword.
 */
static BOOL DefIsReservedKeyword(PCSZ pszWord)
{
    static const PCSZ aReserved[] = {
        "BASE", "CODE", "DATA", "DESCRIPTION", "EXETYPE", "EXPORTS",
        "HEAPSIZE", "IMPORTS", "LIBRARY", "NAME", "OLD",
        "PROTMODE", "SEGMENTS", "STACKSIZE", "STUB", NULL
    };
    int i;
    for (i = 0; aReserved[i]; i++) {
        if (stricmp(pszWord, aReserved[i]) == 0) return TRUE;
    }
    return FALSE;
}

/*!
 * @brief Parse one SEGMENTS entry.
 *
 * @par Reference
 *   LINK386 Reference, "SEGMENTS Statement" and its attribute
 *   pages:
 *     [']segmentname['] [CLASS 'classname'][attribute...]
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszLine  Trimmed line. Not NULL.
 *
 * @return Pointer to a freshly allocated DEF_SEGMENT, or NULL.
 *
 * @retval pSeg  Parsed entry; caller owns it.
 * @retval NULL  Malformed line or allocation failure.
 */
static PDEF_SEGMENT DefParseSegmentLine(PDEFFILE pDef, PCSZ pszLine)
{
    PDEF_SEGMENT pSeg;
    PCSZ p = pszLine;
    char achName[256];
    char achClass[256];
    char achWord[64];

    pSeg = (PDEF_SEGMENT)calloc(1, sizeof(*pSeg));
    if (!pSeg) return NULL;

    while (*p && isspace((unsigned char)*p)) p++;

    if (*p == '\'') {
        if (!DefParseQuoted(&p, achName, sizeof(achName))) goto fail;
        if (!DefIsEndOfLine(p) && !isspace((unsigned char)*p)) {
            pDef->ulParseErrors++;
            goto fail;
        }
    } else {
        if (!DefNextWord(&p, achName, sizeof(achName))) goto fail;
        if (DefIsReservedKeyword(achName)) {
            pDef->ulParseErrors++;
            goto fail;
        }
    }
    if (strlen(achName) > DEF_MAX_NAME) { pDef->ulParseErrors++; goto fail; }

    pSeg->pszName = DefDup(achName);
    if (!pSeg->pszName) goto fail;

    /* Optional CLASS 'name' */
    {
        PCSZ pSave = p;
        while (*p && isspace((unsigned char)*p)) p++;
        if (DefNextWord(&p, achWord, sizeof(achWord)) &&
            stricmp(achWord, "CLASS") == 0) {
            if (!DefParseQuoted(&p, achClass, sizeof(achClass))) {
                pDef->ulParseErrors++;
                p = pSave;
            } else {
                if (!DefIsEndOfLine(p) && !isspace((unsigned char)*p)) {
                    pDef->ulParseErrors++;
                }
                if (strlen(achClass) > DEF_MAX_NAME) {
                    pDef->ulParseErrors++;
                } else {
                    pSeg->pszClass = DefDup(achClass);
                    if (!pSeg->pszClass) goto fail;
                }
            }
        } else {
            p = pSave;
        }
    }

    /* Attributes */
    while (DefNextWord(&p, achWord, sizeof(achWord))) {
        if (stricmp(achWord, "MIXED1632") == 0) {
            pSeg->fMixed = TRUE;
        } else if (stricmp(achWord, "ALIAS") == 0) {
            pSeg->fAlias = TRUE;
        } else {
            DefParseLoad(achWord,  &pSeg->ulLoad);
            DefParseRW(achWord,    &pSeg->ulRW);
            DefParseRX(achWord,    &pSeg->ulRX);
            DefParseIOPL(achWord,  &pSeg->ulIOPL);
            DefParseConf(achWord,  &pSeg->ulConf);
            DefParseShare(achWord, &pSeg->ulShare);
        }
    }

    return pSeg;

fail:
    if (pSeg->pszName)  free(pSeg->pszName);
    if (pSeg->pszClass) free(pSeg->pszClass);
    free(pSeg);
    return NULL;
}

/* ==================================================================
 * IMPORTS line parser
 * ================================================================== */

/*!
 * @brief Parse one IMPORTS entry.
 *
 * @par Reference
 *   LINK386 Reference, "IMPORTS Statement":
 *     [internalname=]modulename.entry
 *   "If an ordinal value is given, then <internalname> is
 *   required."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszLine  Trimmed line. Not NULL.
 *
 * @return Pointer to a freshly allocated DEF_IMP, or NULL.
 *
 * @retval pRec  Parsed entry; caller owns it.
 * @retval NULL  Malformed line, missing internalname for an
 *               ordinal entry, or allocation failure.
 */
static PDEF_IMP DefParseImportLine(PDEFFILE pDef, PCSZ pszLine)
{
    PDEF_IMP pRec;
    PCSZ     p = pszLine;
    char     achInternal[256];
    char     achModule[256];
    char     achExternal[256];
    PCSZ     pEq;
    PCSZ     pDot;
    size_t   n;

    pRec = (PDEF_IMP)calloc(1, sizeof(*pRec));
    if (!pRec) return NULL;

    while (*p && isspace((unsigned char)*p)) p++;

    pEq = strchr(p, '=');

    if (pEq) {
        n = (size_t)(pEq - p);
        while (n && isspace((unsigned char)p[n-1])) n--;
        if (n == 0 || n >= sizeof(achInternal)) goto fail;
        memcpy(achInternal, p, n);
        achInternal[n] = '\0';
        if (strlen(achInternal) > DEF_MAX_NAME) goto fail;
        pRec->pszInternal = DefDup(achInternal);
        if (!pRec->pszInternal) goto fail;
        pRec->fExplicit = TRUE;
        p = pEq + 1;
    } else {
        pRec->fExplicit = FALSE;
    }

    while (*p && isspace((unsigned char)*p)) p++;

    pDot = strchr(p, '.');
    if (!pDot) goto fail;
    n = (size_t)(pDot - p);
    while (n && isspace((unsigned char)p[n-1])) n--;
    if (n == 0 || n >= sizeof(achModule)) goto fail;
    memcpy(achModule, p, n);
    achModule[n] = '\0';
    if (strlen(achModule) > DEF_MAX_NAME) goto fail;
    pRec->pszModule = DefDup(achModule);
    if (!pRec->pszModule) goto fail;

    p = pDot + 1;
    while (*p && isspace((unsigned char)*p)) p++;
    n = 0;
    while (p[n] && !isspace((unsigned char)p[n]) &&
           n + 1 < sizeof(achExternal)) {
        achExternal[n] = p[n];
        n++;
    }
    achExternal[n] = '\0';
    if (n == 0) goto fail;
    if (strlen(achExternal) > DEF_MAX_NAME) goto fail;
    pRec->pszExternal = DefDup(achExternal);
    if (!pRec->pszExternal) goto fail;

    if (!pRec->fExplicit) {
        pRec->pszInternal = DefDup(achExternal);
        if (!pRec->pszInternal) goto fail;
    }

    {
        size_t i;
        BOOL   fDigits = TRUE;
        for (i = 0; achExternal[i]; i++) {
            if (!isdigit((unsigned char)achExternal[i])) {
                fDigits = FALSE;
                break;
            }
        }
        pRec->fOrdinal = fDigits && achExternal[0] != '\0';
    }

    if (pRec->fOrdinal && !pRec->fExplicit) {
        pDef->ulParseErrors++;
        goto fail;
    }

    return pRec;

fail:
    if (pRec->pszInternal) free(pRec->pszInternal);
    if (pRec->pszModule)   free(pRec->pszModule);
    if (pRec->pszExternal) free(pRec->pszExternal);
    free(pRec);
    return NULL;
}

/* ==================================================================
 * EXPORTS line parser
 * ================================================================== */

/*!
 * @brief Parse one EXPORTS entry.
 *
 * @par Reference
 *   LINK386 Reference, "EXPORTS Statement":
 *     entryname [=internalname] [@ord[RESIDENTNAME]] [pwords]
 *   "RESIDENTNAME... is applicable only if <ord> is used."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszLine  Trimmed line. Not NULL.
 *
 * @return Pointer to a freshly allocated DEF_EXP, or NULL.
 *
 * @retval pRec  Parsed entry; caller owns it.
 * @retval NULL  Malformed line or allocation failure.
 */
static PDEF_EXP DefParseExportLine(PDEFFILE pDef, PCSZ pszLine)
{
    PDEF_EXP pRec;
    PCSZ     p = pszLine;
    char     achName[256];
    char     achInternal[256];
    size_t   n;

    pRec = (PDEF_EXP)calloc(1, sizeof(*pRec));
    if (!pRec) return NULL;

    while (*p && isspace((unsigned char)*p)) p++;

    n = 0;
    while (p[n] && !isspace((unsigned char)p[n]) &&
           p[n] != '=' && p[n] != '@' && n + 1 < sizeof(achName)) {
        achName[n] = p[n];
        n++;
    }
    achName[n] = '\0';
    if (n == 0) goto fail;
    if (strlen(achName) > DEF_MAX_NAME) goto fail;
    pRec->pszName = DefDup(achName);
    if (!pRec->pszName) goto fail;

    p += n;

    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '=') {
        p++;
        while (*p && isspace((unsigned char)*p)) p++;
        n = 0;
        while (p[n] && !isspace((unsigned char)p[n]) &&
               p[n] != '@' && n + 1 < sizeof(achInternal)) {
            achInternal[n] = p[n];
            n++;
        }
        achInternal[n] = '\0';
        if (n > 0) {
            if (strlen(achInternal) > DEF_MAX_NAME) goto fail;
            pRec->pszInternal = DefDup(achInternal);
            if (!pRec->pszInternal) goto fail;
        }
        p += n;
    }

    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '@') {
        ULONG ulOrd;
        p++;
        if (!DefParseNumber(&p, &ulOrd)) { pDef->ulParseErrors++; goto fail; }
        if (ulOrd == 0 || ulOrd > 0xFFFFUL) { pDef->ulParseErrors++; goto fail; }
        pRec->usOrdinal = (USHORT)ulOrd;
        pRec->fHasOrdinal = TRUE;

        {
            PCSZ pSave = p;
            char achWord[64];
            while (*p && isspace((unsigned char)*p)) p++;
            if (DefNextWord(&p, achWord, sizeof(achWord)) &&
                stricmp(achWord, "RESIDENTNAME") == 0) {
                pRec->fResident = TRUE;
            } else {
                p = pSave;
            }
        }
    } else {
        PCSZ pSave = p;
        char achWord[64];
        while (*p && isspace((unsigned char)*p)) p++;
        if (DefNextWord(&p, achWord, sizeof(achWord)) &&
            stricmp(achWord, "RESIDENTNAME") == 0) {
            pDef->ulParseErrors++;
        } else {
            p = pSave;
        }
    }

    while (*p && isspace((unsigned char)*p)) p++;
    if (*p) {
        ULONG ulPWords;
        PCSZ  pSave = p;
        if (DefParseNumber(&p, &ulPWords)) {
            if (ulPWords > 0xFFFFUL) { pDef->ulParseErrors++; p = pSave; }
            else {
                pRec->usPWords = (USHORT)ulPWords;
                pRec->fHasPWords = TRUE;
            }
        }
    }

    if (!DefIsEndOfLine(p)) pDef->ulParseErrors++;

    return pRec;

fail:
    if (pRec->pszName)     free(pRec->pszName);
    if (pRec->pszInternal) free(pRec->pszInternal);
    free(pRec);
    return NULL;
}

/* ==================================================================
 * Top-level statement handlers
 * ================================================================== */

/*!
 * @brief Parse a LIBRARY statement.
 *
 * @par Reference
 *   LINK386 Reference, "LIBRARY Statement":
 *     LIBRARY [libraryname][initialization] [termination]
 *   "If omitted, <initialization> defaults to INITGLOBAL."
 *   "If omitted, <initialization> defaults to TERMGLOBAL."
 *   "Using this keyword without a termination flag implies
 *   TERMGLOBAL for DLLs with 32-bit entry points." Same for
 *   INITINSTANCE implying TERMINSTANCE.
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the LIBRARY keyword. Not NULL.
 */
static void DefParseLibrary(PDEFFILE pDef, PCSZ pszRest)
{
    char  achWord[64];
    PCSZ  p = pszRest;
    char  achName[256];
    size_t n = 0;
    BOOL fSawInit = FALSE;
    BOOL fSawTerm = FALSE;

    pDef->ulModuleType = DEF_MODTYPE_LIBRARY;

    while (*p && isspace((unsigned char)*p)) p++;
    while (p[n] && !isspace((unsigned char)p[n]) && n + 1 < sizeof(achName)) {
        achName[n] = p[n];
        n++;
    }
    if (n > 0) {
        achName[n] = '\0';
        if (strlen(achName) > DEF_MAX_NAME) {
            pDef->ulParseErrors++;
        } else {
            if (pDef->pszModuleName) free(pDef->pszModuleName);
            pDef->pszModuleName = DefDup(achName);
        }
    } else if (*p && !isspace((unsigned char)*p) && *p != '\0') {
        pDef->ulParseErrors++;
    }
    p += n;

    while (DefNextWord(&p, achWord, sizeof(achWord))) {
        if (stricmp(achWord, "INITGLOBAL") == 0) {
            if (fSawInit) { pDef->ulParseErrors++; }
            else { pDef->ulInit = DEF_INIT_GLOBAL; fSawInit = TRUE; }
        } else if (stricmp(achWord, "INITINSTANCE") == 0) {
            if (fSawInit) { pDef->ulParseErrors++; }
            else { pDef->ulInit = DEF_INIT_INSTANCE; fSawInit = TRUE; }
        } else if (stricmp(achWord, "TERMGLOBAL") == 0) {
            if (fSawTerm) { pDef->ulParseErrors++; }
            else { pDef->ulTerm = DEF_TERM_GLOBAL; fSawTerm = TRUE; }
        } else if (stricmp(achWord, "TERMINSTANCE") == 0) {
            if (fSawTerm) { pDef->ulParseErrors++; }
            else { pDef->ulTerm = DEF_TERM_INSTANCE; fSawTerm = TRUE; }
        } else {
            pDef->ulParseErrors++;
        }
    }

    if (!fSawTerm) {
        if (pDef->ulInit == DEF_INIT_INSTANCE)
            pDef->ulTerm = DEF_TERM_INSTANCE;
        else if (pDef->ulInit == DEF_INIT_GLOBAL)
            pDef->ulTerm = DEF_TERM_GLOBAL;
    }
}

/*!
 * @brief Parse a NAME statement.
 *
 * @par Reference
 *   LINK386 Reference, "NAME Statement":
 *     NAME [appname][apptype]
 *   "<apptype>... defines the type of application: WINDOWAPI,
 *   WINDOWCOMPAT, NOTWINDOWCOMPAT."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the NAME keyword. Not NULL.
 */
static void DefParseName(PDEFFILE pDef, PCSZ pszRest)
{
    char  achWord[64];
    PCSZ  p = pszRest;
    char  achName[256];
    size_t n = 0;
    BOOL fSawType = FALSE;

    pDef->ulModuleType = DEF_MODTYPE_APPLICATION;

    while (*p && isspace((unsigned char)*p)) p++;
    while (p[n] && !isspace((unsigned char)p[n]) && n + 1 < sizeof(achName)) {
        achName[n] = p[n];
        n++;
    }
    if (n > 0) {
        achName[n] = '\0';
        if (strlen(achName) > DEF_MAX_NAME) {
            pDef->ulParseErrors++;
        } else {
            if (pDef->pszModuleName) free(pDef->pszModuleName);
            pDef->pszModuleName = DefDup(achName);
        }
    }
    p += n;

    while (DefNextWord(&p, achWord, sizeof(achWord))) {
        if (stricmp(achWord, "WINDOWAPI") == 0) {
            if (fSawType) { pDef->ulParseErrors++; }
            else { pDef->ulAppType = DEF_APPTYPE_WINDOWAPI; fSawType = TRUE; }
        } else if (stricmp(achWord, "WINDOWCOMPAT") == 0) {
            if (fSawType) { pDef->ulParseErrors++; }
            else { pDef->ulAppType = DEF_APPTYPE_WINDOWCOMPAT; fSawType = TRUE; }
        } else if (stricmp(achWord, "NOTWINDOWCOMPAT") == 0) {
            if (fSawType) { pDef->ulParseErrors++; }
            else { pDef->ulAppType = DEF_APPTYPE_NOTWINDOWCOMPAT; fSawType = TRUE; }
        } else {
            pDef->ulParseErrors++;
        }
    }
}

/*!
 * @brief Parse a PHYSICAL DEVICE or VIRTUAL DEVICE statement.
 *
 * @par Reference
 *   LINK386 Reference, "PHYSICAL DEVICE Statement":
 *     PHYSICAL DEVICE [drivername]
 *   "VIRTUAL DEVICE Statement":
 *     VIRTUAL DEVICE  [drivername]
 *   "If <drivername> is given, it becomes the name of the driver
 *   as it is known by OS/2."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the keyword. Not NULL.
 * @param[in] ulType   DEF_MODTYPE_PHYSICAL_DEVICE or
 *                     DEF_MODTYPE_VIRTUAL_DEVICE.
 */
static void DefParseDriver(PDEFFILE pDef, PCSZ pszRest, ULONG ulType)
{
    char  achName[256];
    PCSZ  p = pszRest;
    size_t n = 0;

    pDef->ulModuleType = ulType;
    while (*p && isspace((unsigned char)*p)) p++;
    while (p[n] && !isspace((unsigned char)p[n]) && n + 1 < sizeof(achName)) {
        achName[n] = p[n];
        n++;
    }
    if (n > 0) {
        achName[n] = '\0';
        if (strlen(achName) > DEF_MAX_NAME) {
            pDef->ulParseErrors++;
        } else {
            if (pDef->pszModuleName) free(pDef->pszModuleName);
            pDef->pszModuleName = DefDup(achName);
        }
    }
    p += n;
    if (!DefIsEndOfLine(p)) pDef->ulParseErrors++;
}

/*!
 * @brief Parse a DESCRIPTION statement.
 *
 * @par Reference
 *   LINK386 Reference, "DESCRIPTION Statement":
 *     DESCRIPTION 'text'
 *   "The <text> field is a one line string enclosed in single
 *   quotation marks."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the DESCRIPTION keyword.
 *                     Not NULL.
 */
static void DefParseDescription(PDEFFILE pDef, PCSZ pszRest)
{
    char achText[DEF_MAX_DESC];
    PCSZ p = pszRest;
    if (!DefParseQuoted(&p, achText, sizeof(achText))) {
        pDef->ulParseErrors++;
        return;
    }
    if (!DefIsEndOfLine(p)) {
        pDef->ulParseErrors++;
        return;
    }
    if (pDef->pszDescription) free(pDef->pszDescription);
    pDef->pszDescription = DefDup(achText);
}

/*!
 * @brief Parse an EXETYPE statement.
 *
 * @par Reference
 *   LINK386 Reference, "EXETYPE Statement":
 *     EXETYPE [OS2 | WINDOWS | UNKNOWN]
 *   "OS2: OS/2 applications and dynamic-link libraries (default)".
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the EXETYPE keyword. Not NULL.
 */
static void DefParseExeType(PDEFFILE pDef, PCSZ pszRest)
{
    char achWord[64];
    PCSZ p = pszRest;
    if (!DefNextWord(&p, achWord, sizeof(achWord))) {
        pDef->ulParseErrors++;
        return;
    }
    if (stricmp(achWord, "OS2") == 0)
        pDef->ulExeType = DEF_EXETYPE_OS2;
    else if (stricmp(achWord, "WINDOWS") == 0)
        pDef->ulExeType = DEF_EXETYPE_WINDOWS;
    else if (stricmp(achWord, "UNKNOWN") == 0)
        pDef->ulExeType = DEF_EXETYPE_UNKNOWN;
    else
        pDef->ulParseErrors++;
    if (!DefIsEndOfLine(p)) pDef->ulParseErrors++;
}

/*!
 * @brief Parse a BASE statement.
 *
 * @par Reference
 *   LINK386 Reference, "BASE Statement": syntax BASE=n. "Where n is
 *   a value rounded up to the nearest multiple of 64K."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the BASE keyword. Not NULL.
 */
static void DefParseBase(PDEFFILE pDef, PCSZ pszRest)
{
    PCSZ p = pszRest;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != '=') { pDef->ulParseErrors++; return; }
    p++;
    if (!DefParseNumber(&p, &pDef->ulBase)) { pDef->ulParseErrors++; return; }
    if (!DefIsEndOfLine(p)) { pDef->ulParseErrors++; return; }
    pDef->fHasBase = TRUE;
}

/*!
 * @brief Parse a HEAPSIZE statement.
 *
 * @par Reference
 *   LINK386 Reference, "HEAPSIZE Statement":
 *     HEAPSIZE bytes | MAXVAL
 *   "<bytes> contains any positive integer... Instead of entering
 *   a number for <bytes>, you can enter the keyword MAXVAL."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the HEAPSIZE keyword. Not NULL.
 */
static void DefParseHeapSize(PDEFFILE pDef, PCSZ pszRest)
{
    char achWord[64];
    PCSZ p = pszRest;
    if (!DefNextWord(&p, achWord, sizeof(achWord))) {
        pDef->ulParseErrors++;
        return;
    }
    if (stricmp(achWord, "MAXVAL") == 0) {
        if (!DefIsEndOfLine(p)) { pDef->ulParseErrors++; return; }
        pDef->fHeapSizeMaxVal = TRUE;
        pDef->fHasHeapSize = TRUE;
    } else {
        PCSZ pNum = achWord;
        if (!DefParseNumber(&pNum, &pDef->ulHeapSize)) {
            pDef->ulParseErrors++;
            return;
        }
        if (!DefIsEndOfLine(pNum) || !DefIsEndOfLine(p)) {
            pDef->ulParseErrors++;
            return;
        }
        pDef->fHasHeapSize = TRUE;
    }
}

/*!
 * @brief Parse a STACKSIZE statement.
 *
 * @par Reference
 *   LINK386 Reference, "STACKSIZE Statement":
 *     STACKSIZE number
 *   "<number> contains a positive integer."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the STACKSIZE keyword.
 *                     Not NULL.
 */
static void DefParseStackSize(PDEFFILE pDef, PCSZ pszRest)
{
    PCSZ p = pszRest;
    if (!DefParseNumber(&p, &pDef->ulStackSize)) {
        pDef->ulParseErrors++;
        return;
    }
    if (!DefIsEndOfLine(p)) { pDef->ulParseErrors++; return; }
    pDef->fHasStackSize = TRUE;
}

/*!
 * @brief Parse an OLD statement.
 *
 * @par Reference
 *   LINK386 Reference, "OLD Statement":
 *     OLD 'filename'
 *   "This statement directs LINK386 to search another dynamic-link
 *   module for export ordinals."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the OLD keyword. Not NULL.
 */
static void DefParseOld(PDEFFILE pDef, PCSZ pszRest)
{
    char achName[256];
    PCSZ p = pszRest;
    if (!DefParseQuoted(&p, achName, sizeof(achName))) {
        pDef->ulParseErrors++;
        return;
    }
    if (!DefIsEndOfLine(p)) { pDef->ulParseErrors++; return; }
    if (strlen(achName) > DEF_MAX_NAME) { pDef->ulParseErrors++; return; }
    if (pDef->pszOldName) free(pDef->pszOldName);
    pDef->pszOldName = DefDup(achName);
}

/*!
 * @brief Parse a STUB statement.
 *
 * @par Reference
 *   LINK386 Reference, "STUB Statement":
 *     STUB 'filename'
 *   "This statement adds a DOS executable file to the beginning of
 *   the application or library being created."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the STUB keyword. Not NULL.
 */
static void DefParseStub(PDEFFILE pDef, PCSZ pszRest)
{
    char achName[256];
    PCSZ p = pszRest;
    if (!DefParseQuoted(&p, achName, sizeof(achName))) {
        pDef->ulParseErrors++;
        return;
    }
    if (!DefIsEndOfLine(p)) { pDef->ulParseErrors++; return; }
    if (strlen(achName) > DEF_MAX_NAME) { pDef->ulParseErrors++; return; }
    if (pDef->pszStubName) free(pDef->pszStubName);
    pDef->pszStubName = DefDup(achName);
}

/*!
 * @brief Parse a PROTMODE statement.
 *
 * @par Reference
 *   LINK386 Reference, "PROTMODE Statement": syntax PROTMODE.
 *   "This statement specifies that the module runs only in
 *   protected mode and not in Windows or dual mode."
 *
 * @param[in] pDef     Handle internals. Not NULL.
 * @param[in] pszRest  Text following the PROTMODE keyword.
 *                     Must be empty.
 */
static void DefParseProtMode(PDEFFILE pDef, PCSZ pszRest)
{
    if (!DefIsEndOfLine(pszRest)) { pDef->ulParseErrors++; return; }
    pDef->fProtMode = TRUE;
}

/* ==================================================================
 * Line driver
 * ================================================================== */

/*!
 * @brief Read a line, detecting buffer overflow.
 *
 * If fgets fills the buffer without a terminating newline, the rest
 * of the physical line is discarded and *pfOverflow is set.
 *
 * @param[in]  fp          Open stream. Not NULL.
 * @param[out] pszBuf      Line buffer. Not NULL.
 * @param[in]  cbBuf       Size of pszBuf in bytes.
 * @param[out] pfOverflow  Receiver. Not NULL.
 *
 * @return TRUE if a line was read, FALSE at end of file.
 *
 * @retval TRUE   Line read; buffer possibly truncated.
 * @retval FALSE  End of file reached.
 */
static BOOL DefReadLine(FILE *fp, char *pszBuf, size_t cbBuf,
                        PBOOL pfOverflow)
{
    size_t len;
    int    ch;

    *pfOverflow = FALSE;
    if (!fgets(pszBuf, (int)cbBuf, fp)) return FALSE;

    len = strlen(pszBuf);
    if (len == 0) return TRUE;

    if (pszBuf[len - 1] != '\n' && len == cbBuf - 1) {
        *pfOverflow = TRUE;
        while ((ch = fgetc(fp)) != EOF && ch != '\n') {
            /* discard remainder of the oversized line */
        }
    }
    return TRUE;
}

/*!
 * @brief Parse the whole file.
 *
 * @par Reference
 *   LINK386 Reference, "Module Statement Rules".
 *
 * @param[in] pDef  Handle internals. Not NULL.
 * @param[in] fp    Open stream. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  VectorAdd failed.
 */
static APIRET DefParseLines(PDEFFILE pDef, FILE *fp)
{
    char   line[DEF_MAX_LINE];
    int    section = STMT_NONE;
    APIRET rc = NO_ERROR;

    for (;;) {
        BOOL fOverflow = FALSE;
        char *semi;
        int   nKind;
        PCSZ  pszRest = NULL;

        if (!DefReadLine(fp, line, sizeof(line), &fOverflow)) break;
        if (fOverflow) { pDef->ulParseErrors++; continue; }

        semi = strchr(line, ';');
        if (semi) *semi = '\0';
        DefTrim(line);
        if (line[0] == '\0') continue;

        nKind = DefRecognizeStatement(line, &pszRest);

        if (nKind == STMT_IMPORTS || nKind == STMT_EXPORTS ||
            nKind == STMT_SEGMENTS) {
            pDef->fSeenAnyStatement = TRUE;
            section = nKind;
            continue;
        }

        if (nKind != STMT_NONE) {
            if (DefIsIdentifying(nKind)) {
                if (pDef->fSeenAnyStatement) {
                    pDef->ulParseErrors++;
                    section = STMT_NONE;
                    continue;
                }
                if (pDef->fSeenIdentifier) {
                    pDef->ulParseErrors++;
                    section = STMT_NONE;
                    continue;
                }
                pDef->fSeenIdentifier = TRUE;
            }
            pDef->fSeenAnyStatement = TRUE;
            section = STMT_NONE;

            switch (nKind) {
            case STMT_LIBRARY:
                DefParseLibrary(pDef, pszRest);
                break;
            case STMT_NAME:
                DefParseName(pDef, pszRest);
                break;
            case STMT_PHYSICAL_DEVICE:
                DefParseDriver(pDef, pszRest,
                               DEF_MODTYPE_PHYSICAL_DEVICE);
                break;
            case STMT_VIRTUAL_DEVICE:
                DefParseDriver(pDef, pszRest,
                               DEF_MODTYPE_VIRTUAL_DEVICE);
                break;
            case STMT_DESCRIPTION:
                DefParseDescription(pDef, pszRest);
                break;
            case STMT_EXETYPE:
                DefParseExeType(pDef, pszRest);
                break;
            case STMT_BASE:
                DefParseBase(pDef, pszRest);
                break;
            case STMT_HEAPSIZE:
                DefParseHeapSize(pDef, pszRest);
                break;
            case STMT_STACKSIZE:
                DefParseStackSize(pDef, pszRest);
                break;
            case STMT_OLD:
                DefParseOld(pDef, pszRest);
                break;
            case STMT_STUB:
                DefParseStub(pDef, pszRest);
                break;
            case STMT_PROTMODE:
                DefParseProtMode(pDef, pszRest);
                break;
            case STMT_CODE:
                DefParseCodeLine(pDef, pszRest);
                break;
            case STMT_DATA:
                DefParseDataLine(pDef, pszRest);
                break;
            default:
                break;
            }
            continue;
        }

        switch (section) {
        case STMT_IMPORTS: {
            PDEF_IMP pRec = DefParseImportLine(pDef, line);
            if (!pRec) { pDef->ulParseErrors++; break; }
            rc = VectorAdd(pDef->hImports, &pRec);
            if (rc != NO_ERROR) {
                free(pRec->pszInternal);
                free(pRec->pszModule);
                free(pRec->pszExternal);
                free(pRec);
                return rc;
            }
            break;
        }
        case STMT_EXPORTS: {
            PDEF_EXP pRec = DefParseExportLine(pDef, line);
            if (!pRec) { pDef->ulParseErrors++; break; }
            rc = VectorAdd(pDef->hExports, &pRec);
            if (rc != NO_ERROR) {
                free(pRec->pszName);
                if (pRec->pszInternal) free(pRec->pszInternal);
                free(pRec);
                return rc;
            }
            break;
        }
        case STMT_SEGMENTS: {
            PDEF_SEGMENT pSeg = DefParseSegmentLine(pDef, line);
            if (!pSeg) { pDef->ulParseErrors++; break; }
            rc = VectorAdd(pDef->hSegments, &pSeg);
            if (rc != NO_ERROR) {
                if (pSeg->pszName)  free(pSeg->pszName);
                if (pSeg->pszClass) free(pSeg->pszClass);
                free(pSeg);
                return rc;
            }
            break;
        }
        default:
            pDef->ulParseErrors++;
            break;
        }
    }
    return NO_ERROR;
}

/* ==================================================================
 * String field accessor helper
 * ================================================================== */

/*!
 * @brief Copy a string field out of a private record.
 *
 * Size-query convention as described in vector.h.
 *
 * @param[in]  pszSrc   Source string. May be NULL.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Buffer is NULL but size is not 0.
 * @retval ERROR_FILE_NOT_FOUND     Source is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
static APIRET DefCopyString(PCSZ pszSrc,
                            PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    ULONG ulLen;
    if (!pszBuf && ulSize != 0) return ERROR_INVALID_PARAMETER;
    if (!pszSrc) {
        if (pulUsed) *pulUsed = 0;
        return ERROR_FILE_NOT_FOUND;
    }
    ulLen = (ULONG)strlen(pszSrc);
    if (!pszBuf) {
        if (pulUsed) *pulUsed = ulLen + 1;
        return NO_ERROR;
    }
    if (ulSize < ulLen + 1) {
        if (pulUsed) *pulUsed = ulLen + 1;
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pszSrc, ulLen + 1);
    if (pulUsed) *pulUsed = ulLen;
    return NO_ERROR;
}

/* ==================================================================
 * Public API: lifecycle
 * ================================================================== */

/*!
 * @brief Open and parse a .DEF file.
 *
 * Reads the file, strips comments, dispatches on the first keyword
 * of each line and collects the IMPORTS, EXPORTS and SEGMENTS
 * entries into vectors inside the handle.
 *
 * @par Reference
 * LINK386 Reference, "Module Statement Rules".
 *
 * @param[in]  pszPath  Path to the .DEF file. Not NULL.
 * @param[out] phDef    Receives the handle. Not NULL. Set to
 *                      NULLHANDLE on failure.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  pszPath or phDef is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see DefClose
 */
APIRET APIENTRY DefOpen(PCSZ pszPath, PHDEFFILE phDef)
{
    PDEFFILE pDef;
    FILE    *fp;
    APIRET   rc;

    if (!pszPath || !phDef) return ERROR_INVALID_PARAMETER;
    *phDef = NULLHANDLE;

    fp = fopen(pszPath, "r");
    if (!fp) return ERROR_OPEN_FAILED;

    pDef = (PDEFFILE)calloc(1, sizeof(*pDef));
    if (!pDef) { fclose(fp); return ERROR_NOT_ENOUGH_MEMORY; }

    rc = VectorCreate(sizeof(PDEF_SEGMENT), &pDef->hSegments);
    if (rc != NO_ERROR) { free(pDef); fclose(fp); return rc; }
    rc = VectorCreate(sizeof(PDEF_IMP), &pDef->hImports);
    if (rc != NO_ERROR) {
        VectorDestroy(pDef->hSegments);
        free(pDef); fclose(fp); return rc;
    }
    rc = VectorCreate(sizeof(PDEF_EXP), &pDef->hExports);
    if (rc != NO_ERROR) {
        VectorDestroy(pDef->hSegments);
        VectorDestroy(pDef->hImports);
        free(pDef); fclose(fp); return rc;
    }

    pDef->ulMagic = DEF_MAGIC;
    rc = DefParseLines(pDef, fp);
    fclose(fp);

    if (rc != NO_ERROR) { DefClose((HDEFFILE)pDef); return rc; }

    *phDef = (HDEFFILE)pDef;
    return NO_ERROR;
}

/*!
 * @brief Close a parsed .DEF file and release all records.
 *
 * Releases the import, export and segment vectors together with
 * every string owned by the handle. Passing NULLHANDLE is a no-op.
 *
 * @param[in] hDef  Handle. May be NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 *
 * @see DefOpen
 */
APIRET APIENTRY DefClose(HDEFFILE hDef)
{
    PDEFFILE pDef = (PDEFFILE)hDef;
    ULONG    ulCount, i;

    if (!pDef) return NO_ERROR;
    if (pDef->ulMagic != DEF_MAGIC) return ERROR_INVALID_HANDLE;

    if (pDef->hSegments) {
        VectorGetCount(pDef->hSegments, &ulCount);
        for (i = 0; i < ulCount; i++) {
            PDEF_SEGMENT p = NULL;
            if (VectorGetItem(pDef->hSegments, i, &p,
                              sizeof(p), NULL) == NO_ERROR && p) {
                if (p->pszName)  free(p->pszName);
                if (p->pszClass) free(p->pszClass);
                free(p);
            }
        }
        VectorDestroy(pDef->hSegments);
    }
    if (pDef->hImports) {
        VectorGetCount(pDef->hImports, &ulCount);
        for (i = 0; i < ulCount; i++) {
            PDEF_IMP p = NULL;
            if (VectorGetItem(pDef->hImports, i, &p,
                              sizeof(p), NULL) == NO_ERROR && p) {
                if (p->pszInternal) free(p->pszInternal);
                if (p->pszModule)   free(p->pszModule);
                if (p->pszExternal) free(p->pszExternal);
                free(p);
            }
        }
        VectorDestroy(pDef->hImports);
    }
    if (pDef->hExports) {
        VectorGetCount(pDef->hExports, &ulCount);
        for (i = 0; i < ulCount; i++) {
            PDEF_EXP p = NULL;
            if (VectorGetItem(pDef->hExports, i, &p,
                              sizeof(p), NULL) == NO_ERROR && p) {
                if (p->pszName)     free(p->pszName);
                if (p->pszInternal) free(p->pszInternal);
                free(p);
            }
        }
        VectorDestroy(pDef->hExports);
    }

    if (pDef->pszModuleName)  free(pDef->pszModuleName);
    if (pDef->pszDescription) free(pDef->pszDescription);
    if (pDef->pszOldName)     free(pDef->pszOldName);
    if (pDef->pszStubName)    free(pDef->pszStubName);

    pDef->ulMagic = 0;
    free(pDef);
    return NO_ERROR;
}

/* ==================================================================
 * Public API: module identification
 * ================================================================== */

/*!
 * @brief Return the module type.
 *
 * @par Reference
 * LINK386 Reference, "LIBRARY Statement", "NAME Statement",
 * "PHYSICAL DEVICE Statement", "VIRTUAL DEVICE Statement".
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pulType  Receives one of DEF_MODTYPE_*. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryModuleType(HDEFFILE hDef, PULONG pulType)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulType) return ERROR_INVALID_PARAMETER;
    *pulType = pDef->ulModuleType;
    return NO_ERROR;
}

/*!
 * @brief Return the module name.
 *
 * The name comes from LIBRARY, NAME, PHYSICAL DEVICE or VIRTUAL
 * DEVICE, whichever was present. If the statement carried no name,
 * ERROR_FILE_NOT_FOUND is returned; the linker's fallback to the
 * executable file name is not performed by this parser.
 *
 * @par Reference
 * LINK386 Reference, "LIBRARY Statement", "NAME Statement",
 * "PHYSICAL DEVICE Statement", "VIRTUAL DEVICE Statement".
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No identifying statement carried
 *                                  a name.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryModuleName(HDEFFILE hDef,
                                   PSZ pszBuf, ULONG ulSize,
                                   PULONG pulUsed)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef) return ERROR_INVALID_HANDLE;
    return DefCopyString(pDef->pszModuleName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return the LIBRARY initialization mode.
 *
 * @par Reference
 * LINK386 Reference, "LIBRARY Statement": INITGLOBAL (default) or
 * INITINSTANCE.
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pulInit  Receives one of DEF_INIT_*. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Module is not a LIBRARY.
 */
APIRET APIENTRY DefQueryLibraryInit(HDEFFILE hDef, PULONG pulInit)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulInit) return ERROR_INVALID_PARAMETER;
    if (pDef->ulModuleType != DEF_MODTYPE_LIBRARY)
        return ERROR_FILE_NOT_FOUND;
    *pulInit = pDef->ulInit;
    return NO_ERROR;
}

/*!
 * @brief Return the LIBRARY termination mode.
 *
 * @par Reference
 * LINK386 Reference, "LIBRARY Statement": TERMGLOBAL (default) or
 * TERMINSTANCE.
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pulTerm  Receives one of DEF_TERM_*. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Module is not a LIBRARY.
 */
APIRET APIENTRY DefQueryLibraryTerm(HDEFFILE hDef, PULONG pulTerm)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulTerm) return ERROR_INVALID_PARAMETER;
    if (pDef->ulModuleType != DEF_MODTYPE_LIBRARY)
        return ERROR_FILE_NOT_FOUND;
    *pulTerm = pDef->ulTerm;
    return NO_ERROR;
}

/*!
 * @brief Return the NAME application type.
 *
 * @par Reference
 * LINK386 Reference, "NAME Statement": WINDOWAPI, WINDOWCOMPAT or
 * NOTWINDOWCOMPAT.
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pulType  Receives one of DEF_APPTYPE_*. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     Module is not an application.
 */
APIRET APIENTRY DefQueryApplicationType(HDEFFILE hDef, PULONG pulType)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulType) return ERROR_INVALID_PARAMETER;
    if (pDef->ulModuleType != DEF_MODTYPE_APPLICATION)
        return ERROR_FILE_NOT_FOUND;
    *pulType = pDef->ulAppType;
    return NO_ERROR;
}

/*!
 * @brief Return the DESCRIPTION text.
 *
 * @par Reference
 * LINK386 Reference, "DESCRIPTION Statement":
 *   DESCRIPTION 'text'
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No DESCRIPTION statement present.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryDescription(HDEFFILE hDef,
                                    PSZ pszBuf, ULONG ulSize,
                                    PULONG pulUsed)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef) return ERROR_INVALID_HANDLE;
    return DefCopyString(pDef->pszDescription, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return the EXETYPE value.
 *
 * @par Reference
 * LINK386 Reference, "EXETYPE Statement": OS2 (default), WINDOWS or
 * UNKNOWN.
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pulType  Receives one of DEF_EXETYPE_*. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryExeType(HDEFFILE hDef, PULONG pulType)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulType) return ERROR_INVALID_PARAMETER;
    *pulType = pDef->ulExeType;
    return NO_ERROR;
}

/*!
 * @brief Return the BASE address.
 *
 * @par Reference
 * LINK386 Reference, "BASE Statement": syntax BASE=n.
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pulBase  Receives the base address. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No BASE statement present.
 */
APIRET APIENTRY DefQueryBase(HDEFFILE hDef, PULONG pulBase)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulBase) return ERROR_INVALID_PARAMETER;
    if (!pDef->fHasBase) return ERROR_FILE_NOT_FOUND;
    *pulBase = pDef->ulBase;
    return NO_ERROR;
}

/*!
 * @brief Return the HEAPSIZE value.
 *
 * @par Reference
 * LINK386 Reference, "HEAPSIZE Statement":
 *   HEAPSIZE bytes | MAXVAL
 *
 * @param[in]  hDef      Handle. Not NULLHANDLE.
 * @param[out] pulBytes  Receives the heap size. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No HEAPSIZE statement present.
 */
APIRET APIENTRY DefQueryHeapSize(HDEFFILE hDef, PULONG pulBytes)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulBytes) return ERROR_INVALID_PARAMETER;
    if (!pDef->fHasHeapSize) return ERROR_FILE_NOT_FOUND;
    *pulBytes = pDef->ulHeapSize;
    return NO_ERROR;
}

/*!
 * @brief Test whether HEAPSIZE was given as MAXVAL.
 *
 * @par Reference
 * LINK386 Reference, "HEAPSIZE Statement".
 *
 * @param[in]  hDef      Handle. Not NULLHANDLE.
 * @param[out] pfMaxVal  Receives TRUE if MAXVAL was given. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryHeapSizeMaxVal(HDEFFILE hDef, PBOOL pfMaxVal)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pfMaxVal) return ERROR_INVALID_PARAMETER;
    *pfMaxVal = pDef->fHeapSizeMaxVal;
    return NO_ERROR;
}

/*!
 * @brief Return the STACKSIZE value.
 *
 * @par Reference
 * LINK386 Reference, "STACKSIZE Statement":
 *   STACKSIZE number
 *
 * @param[in]  hDef      Handle. Not NULLHANDLE.
 * @param[out] pulBytes  Receives the stack size. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No STACKSIZE statement present.
 */
APIRET APIENTRY DefQueryStackSize(HDEFFILE hDef, PULONG pulBytes)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulBytes) return ERROR_INVALID_PARAMETER;
    if (!pDef->fHasStackSize) return ERROR_FILE_NOT_FOUND;
    *pulBytes = pDef->ulStackSize;
    return NO_ERROR;
}

/*!
 * @brief Return the OLD module file name.
 *
 * @par Reference
 * LINK386 Reference, "OLD Statement":
 *   OLD 'filename'
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No OLD statement present.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryOldName(HDEFFILE hDef,
                                PSZ pszBuf, ULONG ulSize,
                                PULONG pulUsed)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef) return ERROR_INVALID_HANDLE;
    return DefCopyString(pDef->pszOldName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return the STUB file name.
 *
 * @par Reference
 * LINK386 Reference, "STUB Statement":
 *   STUB 'filename'
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No STUB statement present.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryStubName(HDEFFILE hDef,
                                 PSZ pszBuf, ULONG ulSize,
                                 PULONG pulUsed)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef) return ERROR_INVALID_HANDLE;
    return DefCopyString(pDef->pszStubName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Test whether PROTMODE was present.
 *
 * @par Reference
 * LINK386 Reference, "PROTMODE Statement": syntax PROTMODE.
 *
 * @param[in]  hDef        Handle. Not NULLHANDLE.
 * @param[out] pfProtMode  Receives TRUE if PROTMODE was present.
 *                         Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryProtMode(HDEFFILE hDef, PBOOL pfProtMode)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pfProtMode) return ERROR_INVALID_PARAMETER;
    *pfProtMode = pDef->fProtMode;
    return NO_ERROR;
}

/*!
 * @brief Return the number of parse errors encountered.
 *
 * Counts lines that failed to parse: malformed entries, oversized
 * lines, statements out of order, duplicate identifying statements.
 * This counter is an extension; it is not part of the LINK386
 * Reference.
 *
 * @param[in]  hDef      Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receives the error count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryParseErrorCount(HDEFFILE hDef, PULONG pulCount)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulCount) return ERROR_INVALID_PARAMETER;
    *pulCount = pDef->ulParseErrors;
    return NO_ERROR;
}

/* ==================================================================
 * Public API: CODE and DATA default attributes
 * ================================================================== */

/*!
 * @brief Return a default CODE attribute.
 *
 * @par Reference
 * LINK386 Reference, "CODE Statement" and its attribute pages:
 * "Load Code Attributes", "Read/Execute Code Attributes",
 * "I/O Privilege Code Attributes", "Conforming Code Attributes".
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[in]  ulAttr   One of DEF_ATTR_LOAD_CODE, DEF_ATTR_RX,
 *                      DEF_ATTR_IOPL_CODE, DEF_ATTR_CONF.
 * @param[out] pulValue Receives the attribute value. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad attribute selector or NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryCodeAttr(HDEFFILE hDef, ULONG ulAttr,
                                 PULONG pulValue)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulValue) return ERROR_INVALID_PARAMETER;
    switch (ulAttr) {
    case DEF_ATTR_LOAD_CODE: *pulValue = pDef->stCode.ulLoad;  break;
    case DEF_ATTR_RX:        *pulValue = pDef->stCode.ulRX;    break;
    case DEF_ATTR_IOPL_CODE: *pulValue = pDef->stCode.ulIOPL;  break;
    case DEF_ATTR_CONF:      *pulValue = pDef->stCode.ulConf;  break;
    default: return ERROR_INVALID_PARAMETER;
    }
    return NO_ERROR;
}

/*!
 * @brief Return a default DATA attribute.
 *
 * @par Reference
 * LINK386 Reference, "DATA Statement" and its attribute pages:
 * "Load Data Attributes", "Read/Write Data Attributes",
 * "Sharing Data Attributes", "Shareable Data Attributes",
 * "LINK386 -I/O Privilege Data Attributes".
 *
 * @param[in]  hDef     Handle. Not NULLHANDLE.
 * @param[in]  ulAttr   One of DEF_ATTR_LOAD_DATA, DEF_ATTR_RW,
 *                      DEF_ATTR_SHARING, DEF_ATTR_SHARE,
 *                      DEF_ATTR_IOPL_DATA.
 * @param[out] pulValue Receives the attribute value. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad attribute selector or NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryDataAttr(HDEFFILE hDef, ULONG ulAttr,
                                 PULONG pulValue)
{
    PDEFFILE pDef = DefLive(hDef);
    if (!pDef || !pulValue) return ERROR_INVALID_PARAMETER;
    switch (ulAttr) {
    case DEF_ATTR_LOAD_DATA: *pulValue = pDef->stData.ulLoad;    break;
    case DEF_ATTR_RW:        *pulValue = pDef->stData.ulRW;      break;
    case DEF_ATTR_SHARING:   *pulValue = pDef->stData.ulSharing; break;
    case DEF_ATTR_SHARE:     *pulValue = pDef->stData.ulShare;   break;
    case DEF_ATTR_IOPL_DATA: *pulValue = pDef->stData.ulIOPL;    break;
    default: return ERROR_INVALID_PARAMETER;
    }
    return NO_ERROR;
}

/* ==================================================================
 * Cursor helpers
 * ================================================================== */

/*!
 * @brief Allocate a fresh cursor.
 *
 * @param[in]  pOwner   Owning handle. Not NULL.
 * @param[in]  ulIndex  Initial index.
 * @param[out] phCursor Receives the cursor. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET DefCursorCreate(PDEFFILE pOwner, ULONG ulIndex,
                              HANDLE *phCursor)
{
    PDEF_CURSOR pCursor;
    pCursor = (PDEF_CURSOR)calloc(1, sizeof(*pCursor));
    if (!pCursor) return ERROR_NOT_ENOUGH_MEMORY;
    pCursor->ulMagic = DEF_CURSOR_MAGIC;
    pCursor->pOwner  = pOwner;
    pCursor->ulIndex = ulIndex;
    *phCursor = (HANDLE)pCursor;
    return NO_ERROR;
}

/*!
 * @brief Release a cursor.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hCursor  Cursor. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle is not recognized.
 */
static APIRET DefCursorDestroy(HANDLE hCursor)
{
    PDEF_CURSOR pCursor = (PDEF_CURSOR)hCursor;
    if (!pCursor) return NO_ERROR;
    if (pCursor->ulMagic != DEF_CURSOR_MAGIC) return ERROR_INVALID_HANDLE;
    pCursor->ulMagic = 0;
    free(pCursor);
    return NO_ERROR;
}

/* ==================================================================
 * Public API: SEGMENTS enumeration
 * ================================================================== */

/*!
 * @brief Open a cursor on the first SEGMENTS entry.
 *
 * @par Reference
 * LINK386 Reference, "SEGMENTS Statement".
 *
 * @param[in]  hDef       Handle. Not NULLHANDLE.
 * @param[out] phSegment  Receives the cursor. Not NULL. Set to
 *                        NULLHANDLE on error or when there are no
 *                        entries.
 * @param[out] pulCount   Optional. May be NULL. On success receives
 *                        the total number of entries.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDef or phSegment is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No entries. *phSegment is
 *                                  NULLHANDLE.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see DefFindNextSegment
 * @see DefFindCloseSegment
 */
APIRET APIENTRY DefFindFirstSegment(HDEFFILE hDef, HDEFSEGMENT *phSegment,
                                    PULONG pulCount)
{
    PDEFFILE pDef = DefLive(hDef);
    ULONG    ulCount = 0;
    APIRET   rc;

    if (!pDef || !phSegment) return ERROR_INVALID_PARAMETER;
    *phSegment = NULLHANDLE;

    rc = VectorGetCount(pDef->hSegments, &ulCount);
    if (rc != NO_ERROR) return rc;
    if (pulCount) *pulCount = ulCount;
    if (ulCount == 0) return ERROR_NO_MORE_ITEMS;

    return DefCursorCreate(pDef, 0, phSegment);
}

/*!
 * @brief Advance a SEGMENTS cursor to the next entry.
 *
 * @param[in] hSegment  Cursor. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more entries.
 *
 * @see DefFindFirstSegment
 * @see DefFindCloseSegment
 */
APIRET APIENTRY DefFindNextSegment(HDEFSEGMENT hSegment)
{
    PDEF_CURSOR pCursor = DefCursorLive(hSegment);
    ULONG ulCount = 0;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    VectorGetCount(pCursor->pOwner->hSegments, &ulCount);
    if (pCursor->ulIndex + 1 >= ulCount) return ERROR_NO_MORE_ITEMS;
    pCursor->ulIndex++;
    return NO_ERROR;
}

/*!
 * @brief Close a SEGMENTS cursor.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hSegment  Cursor. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @see DefFindFirstSegment
 */
APIRET APIENTRY DefFindCloseSegment(HDEFSEGMENT hSegment)
{
    return DefCursorDestroy(hSegment);
}

/*!
 * @brief Return the name of the current segment.
 *
 * @par Reference
 * LINK386 Reference, "SEGMENTS Statement".
 *
 * @param[in]  hSegment  Cursor. Not NULLHANDLE.
 * @param[out] pszBuf    Output buffer, or NULL for a size query.
 * @param[in]  ulSize    Size of pszBuf in bytes.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQuerySegmentName(HDEFSEGMENT hSegment,
                                    PSZ pszBuf, ULONG ulSize,
                                    PULONG pulUsed)
{
    PDEF_CURSOR pCursor = DefCursorLive(hSegment);
    PDEF_SEGMENT pRec = NULL;
    APIRET rc;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    rc = VectorGetItem(pCursor->pOwner->hSegments, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    return DefCopyString(pRec->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return the class name of the current segment.
 *
 * When the source line had no CLASS argument, the accessor returns
 * the string "CODE", which is the documented default.
 *
 * @par Reference
 * LINK386 Reference, "SEGMENTS Statement": "If you do not use the
 * CLASS argument, LINK386 assumes that the class is CODE."
 *
 * @param[in]  hSegment  Cursor. Not NULLHANDLE.
 * @param[out] pszBuf    Output buffer, or NULL for a size query.
 * @param[in]  ulSize    Size of pszBuf in bytes.
 * @param[out] pulUsed   Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQuerySegmentClass(HDEFSEGMENT hSegment,
                                     PSZ pszBuf, ULONG ulSize,
                                     PULONG pulUsed)
{
    PDEF_CURSOR pCursor = DefCursorLive(hSegment);
    PDEF_SEGMENT pRec = NULL;
    APIRET rc;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    rc = VectorGetItem(pCursor->pOwner->hSegments, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    if (!pRec->pszClass)
        return DefCopyString("CODE", pszBuf, ulSize, pulUsed);
    return DefCopyString(pRec->pszClass, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return an attribute of the current segment.
 *
 * @par Reference
 * LINK386 Reference, "SEGMENTS Statement" and its attribute pages:
 * "Load Segments Attributes", "Read/Write Segments Attributes",
 * "Read/Execute Segments Attributes",
 * "I/O Privilege Segments Attributes",
 * "Conforming Segments Attributes",
 * "Specify Mixed 16 and 32-Bit Segments",
 * "Specify that Segment is Aliased",
 * "Specify that Segment is Shared".
 *
 * @param[in]  hSegment  Cursor. Not NULLHANDLE.
 * @param[in]  ulAttr    One of DEF_SEGATTR_*.
 * @param[out] pulValue  Receives the attribute value. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad attribute selector or NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQuerySegmentAttr(HDEFSEGMENT hSegment, ULONG ulAttr,
                                    PULONG pulValue)
{
    PDEF_CURSOR pCursor = DefCursorLive(hSegment);
    PDEF_SEGMENT pRec = NULL;
    APIRET rc;

    if (!pCursor || !pulValue) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(pCursor->pOwner->hSegments, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    switch (ulAttr) {
    case DEF_SEGATTR_LOAD:  *pulValue = pRec->ulLoad;  break;
    case DEF_SEGATTR_RW:    *pulValue = pRec->ulRW;    break;
    case DEF_SEGATTR_RX:    *pulValue = pRec->ulRX;    break;
    case DEF_SEGATTR_IOPL:  *pulValue = pRec->ulIOPL;  break;
    case DEF_SEGATTR_CONF:  *pulValue = pRec->ulConf;  break;
    case DEF_SEGATTR_MIXED: *pulValue = pRec->fMixed ? 1 : 0; break;
    case DEF_SEGATTR_ALIAS: *pulValue = pRec->fAlias ? 1 : 0; break;
    case DEF_SEGATTR_SHARE: *pulValue = pRec->ulShare; break;
    default: return ERROR_INVALID_PARAMETER;
    }
    return NO_ERROR;
}

/* ==================================================================
 * Public API: IMPORTS enumeration
 * ================================================================== */

/*!
 * @brief Open a cursor on the first IMPORTS entry.
 *
 * @param[in]  hDef      Handle. Not NULLHANDLE.
 * @param[out] phImport  Receives the cursor. Not NULL. Set to
 *                       NULLHANDLE on error or when there are no
 *                       entries.
 * @param[out] pulCount  Optional. May be NULL. On success receives
 *                       the total number of entries.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDef or phImport is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No entries. *phImport is
 *                                  NULLHANDLE.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see DefFindNextImport
 * @see DefFindCloseImport
 */
APIRET APIENTRY DefFindFirstImport(HDEFFILE hDef, HDEFIMPORT *phImport,
                                   PULONG pulCount)
{
    PDEFFILE pDef = DefLive(hDef);
    ULONG    ulCount = 0;
    APIRET   rc;

    if (!pDef || !phImport) return ERROR_INVALID_PARAMETER;
    *phImport = NULLHANDLE;

    rc = VectorGetCount(pDef->hImports, &ulCount);
    if (rc != NO_ERROR) return rc;
    if (pulCount) *pulCount = ulCount;
    if (ulCount == 0) return ERROR_NO_MORE_ITEMS;

    return DefCursorCreate(pDef, 0, phImport);
}

/*!
 * @brief Advance an IMPORTS cursor to the next entry.
 *
 * @param[in] hImport  Cursor. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more entries.
 *
 * @see DefFindFirstImport
 * @see DefFindCloseImport
 */
APIRET APIENTRY DefFindNextImport(HDEFIMPORT hImport)
{
    PDEF_CURSOR pCursor = DefCursorLive(hImport);
    ULONG ulCount = 0;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    VectorGetCount(pCursor->pOwner->hImports, &ulCount);
    if (pCursor->ulIndex + 1 >= ulCount) return ERROR_NO_MORE_ITEMS;
    pCursor->ulIndex++;
    return NO_ERROR;
}

/*!
 * @brief Close an IMPORTS cursor.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hImport  Cursor. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @see DefFindFirstImport
 */
APIRET APIENTRY DefFindCloseImport(HDEFIMPORT hImport)
{
    return DefCursorDestroy(hImport);
}

/*!
 * @brief Return the internal name of the current IMPORTS entry.
 *
 * The internal name is the left-hand side of "NAME = MOD.EXT". If
 * the source line omitted the "internalname=" prefix, the accessor
 * returns the external name (the documented default).
 *
 * @par Reference
 * LINK386 Reference, "IMPORTS Statement":
 *   [internalname=]modulename.entry
 *
 * @param[in]  hImport  Cursor. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryImportInternal(HDEFIMPORT hImport,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed)
{
    PDEF_CURSOR pCursor = DefCursorLive(hImport);
    PDEF_IMP pRec = NULL;
    APIRET rc;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    rc = VectorGetItem(pCursor->pOwner->hImports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    return DefCopyString(pRec->pszInternal, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return the module name of the current IMPORTS entry.
 *
 * @par Reference
 * LINK386 Reference, "IMPORTS Statement": modulename.
 *
 * @param[in]  hImport  Cursor. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryImportModule(HDEFIMPORT hImport,
                                     PSZ pszBuf, ULONG ulSize,
                                     PULONG pulUsed)
{
    PDEF_CURSOR pCursor = DefCursorLive(hImport);
    PDEF_IMP pRec = NULL;
    APIRET rc;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    rc = VectorGetItem(pCursor->pOwner->hImports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    return DefCopyString(pRec->pszModule, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return the entry of the current IMPORTS entry.
 *
 * The entry is the right-hand side of "NAME = MOD.EXT". It may be
 * a name or an ordinal number.
 *
 * @par Reference
 * LINK386 Reference, "IMPORTS Statement": entry.
 *
 * @param[in]  hImport  Cursor. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryImportExternal(HDEFIMPORT hImport,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed)
{
    PDEF_CURSOR pCursor = DefCursorLive(hImport);
    PDEF_IMP pRec = NULL;
    APIRET rc;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    rc = VectorGetItem(pCursor->pOwner->hImports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    return DefCopyString(pRec->pszExternal, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Test whether the current IMPORTS entry used an explicit
 *        internalname.
 *
 * @par Reference
 * LINK386 Reference, "IMPORTS Statement": "If an ordinal value is
 * given, then <internalname> is required."
 *
 * @param[in]  hImport      Cursor. Not NULLHANDLE.
 * @param[out] pfExplicit   Receives TRUE if the source line carried
 *                          an explicit assignment. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryImportInternalExplicit(HDEFIMPORT hImport,
                                               PBOOL pfExplicit)
{
    PDEF_CURSOR pCursor = DefCursorLive(hImport);
    PDEF_IMP pRec = NULL;
    APIRET rc;

    if (!pCursor || !pfExplicit) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(pCursor->pOwner->hImports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    *pfExplicit = pRec->fExplicit;
    return NO_ERROR;
}

/*!
 * @brief Test whether the current IMPORTS entry's external reference
 *        is an ordinal value.
 *
 * @par Reference
 * LINK386 Reference, "IMPORTS Statement": "If an ordinal value is
 * given, then <internalname> is required."
 *
 * @param[in]  hImport     Cursor. Not NULLHANDLE.
 * @param[out] pfOrdinal   Receives TRUE if entry is a numeric
 *                         ordinal. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryImportOrdinal(HDEFIMPORT hImport,
                                      PBOOL pfOrdinal)
{
    PDEF_CURSOR pCursor = DefCursorLive(hImport);
    PDEF_IMP pRec = NULL;
    APIRET rc;

    if (!pCursor || !pfOrdinal) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(pCursor->pOwner->hImports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    *pfOrdinal = pRec->fOrdinal;
    return NO_ERROR;
}

/* ==================================================================
 * Public API: EXPORTS enumeration
 * ================================================================== */

/*!
 * @brief Open a cursor on the first EXPORTS entry.
 *
 * @param[in]  hDef      Handle. Not NULLHANDLE.
 * @param[out] phExport  Receives the cursor. Not NULL. Set to
 *                       NULLHANDLE on error or when there are no
 *                       entries.
 * @param[out] pulCount  Optional. May be NULL. On success receives
 *                       the total number of entries.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  hDef or phExport is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No entries. *phExport is
 *                                  NULLHANDLE.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see DefFindNextExport
 * @see DefFindCloseExport
 */
APIRET APIENTRY DefFindFirstExport(HDEFFILE hDef, HDEFEXPORT *phExport,
                                   PULONG pulCount)
{
    PDEFFILE pDef = DefLive(hDef);
    ULONG    ulCount = 0;
    APIRET   rc;

    if (!pDef || !phExport) return ERROR_INVALID_PARAMETER;
    *phExport = NULLHANDLE;

    rc = VectorGetCount(pDef->hExports, &ulCount);
    if (rc != NO_ERROR) return rc;
    if (pulCount) *pulCount = ulCount;
    if (ulCount == 0) return ERROR_NO_MORE_ITEMS;

    return DefCursorCreate(pDef, 0, phExport);
}

/*!
 * @brief Advance an EXPORTS cursor to the next entry.
 *
 * @param[in] hExport  Cursor. Not NULLHANDLE.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_NO_MORE_ITEMS      No more entries.
 *
 * @see DefFindFirstExport
 * @see DefFindCloseExport
 */
APIRET APIENTRY DefFindNextExport(HDEFEXPORT hExport)
{
    PDEF_CURSOR pCursor = DefCursorLive(hExport);
    ULONG ulCount = 0;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    VectorGetCount(pCursor->pOwner->hExports, &ulCount);
    if (pCursor->ulIndex + 1 >= ulCount) return ERROR_NO_MORE_ITEMS;
    pCursor->ulIndex++;
    return NO_ERROR;
}

/*!
 * @brief Close an EXPORTS cursor.
 *
 * Passing NULLHANDLE is a no-op.
 *
 * @param[in] hExport  Cursor. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE    Handle is not recognized.
 *
 * @see DefFindFirstExport
 */
APIRET APIENTRY DefFindCloseExport(HDEFEXPORT hExport)
{
    return DefCursorDestroy(hExport);
}

/*!
 * @brief Return the entryname of the current EXPORTS entry.
 *
 * @par Reference
 * LINK386 Reference, "EXPORTS Statement":
 *   entryname [=internalname] [@ord[RESIDENTNAME]] [pwords]
 *
 * @param[in]  hExport  Cursor. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryExportName(HDEFEXPORT hExport,
                                   PSZ pszBuf, ULONG ulSize,
                                   PULONG pulUsed)
{
    PDEF_CURSOR pCursor = DefCursorLive(hExport);
    PDEF_EXP pRec = NULL;
    APIRET rc;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    rc = VectorGetItem(pCursor->pOwner->hExports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    return DefCopyString(pRec->pszName, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return the internalname of the current EXPORTS entry.
 *
 * When the source line carried no "=internalname", the accessor
 * returns ERROR_FILE_NOT_FOUND. The caller may then fall back to
 * the entryname, which is the documented default.
 *
 * @par Reference
 * LINK386 Reference, "EXPORTS Statement": "By default, this name
 * is the same as <entryname>."
 *
 * @param[in]  hExport  Cursor. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for a size query.
 * @param[in]  ulSize   Size of pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No internalname was specified.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY DefQueryExportInternal(HDEFEXPORT hExport,
                                       PSZ pszBuf, ULONG ulSize,
                                       PULONG pulUsed)
{
    PDEF_CURSOR pCursor = DefCursorLive(hExport);
    PDEF_EXP pRec = NULL;
    APIRET rc;

    if (!pCursor) return ERROR_INVALID_HANDLE;
    rc = VectorGetItem(pCursor->pOwner->hExports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    return DefCopyString(pRec->pszInternal, pszBuf, ulSize, pulUsed);
}

/*!
 * @brief Return the ordinal of the current EXPORTS entry.
 *
 * @par Reference
 * LINK386 Reference, "EXPORTS Statement": the optional @ord gives
 * the ordinal position of the function within the module definition
 * table.
 *
 * @param[in]  hExport     Cursor. Not NULLHANDLE.
 * @param[out] pusOrdinal  Receives the ordinal. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No @ord was specified.
 */
APIRET APIENTRY DefQueryExportOrdinal(HDEFEXPORT hExport,
                                      PUSHORT pusOrdinal)
{
    PDEF_CURSOR pCursor = DefCursorLive(hExport);
    PDEF_EXP pRec = NULL;
    APIRET rc;

    if (!pCursor || !pusOrdinal) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(pCursor->pOwner->hExports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    if (!pRec->fHasOrdinal) return ERROR_FILE_NOT_FOUND;
    *pusOrdinal = pRec->usOrdinal;
    return NO_ERROR;
}

/*!
 * @brief Return the parameter word count of the current EXPORTS
 *        entry.
 *
 * @par Reference
 * LINK386 Reference, "EXPORTS Statement": pwords is the total size
 * of the function's parameters in words. Required only for I/O
 * privileged functions.
 *
 * @param[in]  hExport    Cursor. Not NULLHANDLE.
 * @param[out] pusPWords  Receives the word count. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No pwords was specified.
 */
APIRET APIENTRY DefQueryExportPWords(HDEFEXPORT hExport,
                                     PUSHORT pusPWords)
{
    PDEF_CURSOR pCursor = DefCursorLive(hExport);
    PDEF_EXP pRec = NULL;
    APIRET rc;

    if (!pCursor || !pusPWords) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(pCursor->pOwner->hExports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    if (!pRec->fHasPWords) return ERROR_FILE_NOT_FOUND;
    *pusPWords = pRec->usPWords;
    return NO_ERROR;
}

/*!
 * @brief Test whether the current EXPORTS entry had the
 *        RESIDENTNAME keyword.
 *
 * @par Reference
 * LINK386 Reference, "EXPORTS Statement": RESIDENTNAME is
 * applicable only if @ord is used.
 *
 * @param[in]  hExport     Cursor. Not NULLHANDLE.
 * @param[out] pfResident  Receives TRUE if RESIDENTNAME was
 *                         present. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle is not recognized.
 */
APIRET APIENTRY DefQueryExportResident(HDEFEXPORT hExport,
                                       PBOOL pfResident)
{
    PDEF_CURSOR pCursor = DefCursorLive(hExport);
    PDEF_EXP pRec = NULL;
    APIRET rc;

    if (!pCursor || !pfResident) return ERROR_INVALID_PARAMETER;
    rc = VectorGetItem(pCursor->pOwner->hExports, pCursor->ulIndex,
                       &pRec, sizeof(pRec), NULL);
    if (rc != NO_ERROR) return rc;
    *pfResident = pRec->fResident;
    return NO_ERROR;
}
