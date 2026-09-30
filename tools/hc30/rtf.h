/*!
 * @file rtf.h
 * @brief WinHelp RTF parser library.
 *
 * @par Function naming
 * Function names follow OS/2 Control Program API conventions as
 * defined in OS/2 Version 2.0 - Volume 4: Writing Applications,
 * section 12.2 "Function Name Changes".
 *
 * @par References
 *  - [1] SAA CPI C Reference - Level 2, SC09-1308-02 (Sep 1991).
 *  - [2] OS/2 Version 2.0 - Volume 4: Writing Applications, IBM, 1993.
 *  - [3] RTF Specification 1.0, Microsoft PSS Application Note GC0165.
 *  - [4] MSDN Q75010, WinHelp 3.0 RTF subset.
 *  - [5] Microsoft Help Workshop, "Building a Help File with Build Tags".
 *  - [6] ccl container library: vector.h, strset.h.
 *
 * @par Supported RTF subset (WinHelp 3.0 / HC30)
 *  - \\page, {\\fonttbl}, {\\colortbl}, \\fN, \\fsN, \\cfN, \\cbN
 *  - \\b \\i \\ul, \\strike, \\uldb \\v, \\plain, \\'xx
 *  - {\\footnote ...} with $, # and build tags after ';'
 *  - {\\*\\bkmkstart TAG} ... {\\*\\bkmkend TAG} for build tags
 *
 * Tables (\\trowd \\cell \\row \\intbl) are not supported.
 *
 * @par Thread safety
 * The library is single threaded.
 */
#ifndef RTF_H
#define RTF_H

#include "os2types.h"
#include "os2err.h"
/* Local fallback: os2types.h does not provide VOID on host builds. */
#ifndef VOID
#define VOID void
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef HANDLE HRTFDOC;
typedef HANDLE HRTFTOPIC;
typedef HANDLE HRTFFRAG;
typedef HANDLE HRTFENUM;

typedef HRTFDOC   *PHRTFDOC;
typedef HRTFTOPIC *PHRTFTOPIC;
typedef HRTFFRAG  *PHRTFFRAG;
typedef HRTFENUM  *PHRTFENUM;

#define MAX_FONTTBL  32
#define MAX_COLORTBL 32

#define RTF_FRAG_TEXT  0
#define RTF_FRAG_LINK  1

/* ==================================================================
 * Document lifecycle
 * ================================================================== */

APIRET APIENTRY RtfCreateDoc(PHRTFDOC phDoc);
APIRET APIENTRY RtfDestroyDoc(HRTFDOC hDoc);

/* ==================================================================
 * Build tags
 * ================================================================== */

/*!
 * @brief Set the active build tag list for a document.
 *
 * Must be called before RtfReadFile. If never called, no build tag
 * filtering is performed and all build-tagged content is included.
 *
 * @param[in] hDoc      Document handle. Not NULLHANDLE.
 * @param[in] cTags     Number of tags. May be 0.
 * @param[in] apszTags  Array of NUL-terminated tag names.
 */
APIRET APIENTRY RtfSetBuildTags(HRTFDOC hDoc, ULONG cTags, PCSZ* apszTags);

/*!
 * @brief Query the number of build tags attached to a topic.
 */
APIRET APIENTRY RtfQueryTopicBuildTagCount(HRTFTOPIC hTopic, PULONG pulCount);

/*!
 * @brief Query a topic build tag by index.
 *
 * Uses the size-query convention.
 */
APIRET APIENTRY RtfQueryTopicBuildTag(HRTFTOPIC hTopic, ULONG ulIndex,
                                      PSZ pszBuf, ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Parsing
 * ================================================================== */

APIRET APIENTRY RtfReadFile(HRTFDOC hDoc, PCSZ pszFileName);

/* ==================================================================
 * Query: document
 * ================================================================== */

APIRET APIENTRY RtfQueryDocTopicCount(HRTFDOC hDoc, PULONG pulCount);
APIRET APIENTRY RtfQueryDocFontCount(HRTFDOC hDoc, PULONG pulCount);
APIRET APIENTRY RtfQueryDocFontName(HRTFDOC hDoc, ULONG ulIndex,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY RtfQueryDocFontDescriptorCount(HRTFDOC hDoc, PULONG pulCount);
APIRET APIENTRY RtfQueryDocFontDescriptor(HRTFDOC hDoc, ULONG ulIndex,
                                          PULONG pulFaceIndex,
                                          PULONG pulHalfPoints,
                                          PULONG pulAttributes,
                                          PBYTE  pbFGRGB,
                                          PBYTE  pbBGRGB,
                                          PULONG pulFamily);
APIRET APIENTRY RtfQueryDocColorCount(HRTFDOC hDoc, PULONG pulCount);
APIRET APIENTRY RtfQueryDocColor(HRTFDOC hDoc, ULONG ulIndex, PBYTE pbRGB);

/* ==================================================================
 * Find: topic enumeration
 * ================================================================== */

APIRET APIENTRY RtfFindFirstTopic(HRTFDOC hDoc, PHRTFENUM phEnum);
APIRET APIENTRY RtfFindNextTopic(HRTFENUM hEnum);
APIRET APIENTRY RtfQueryEnumTopic(HRTFENUM hEnum, PHRTFTOPIC phTopic);
APIRET APIENTRY RtfFindClose(HRTFENUM hEnum);

/* ==================================================================
 * Query: topic fields
 * ================================================================== */

APIRET APIENTRY RtfQueryTopicTitle(HRTFTOPIC hTopic, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY RtfQueryTopicContext(HRTFTOPIC hTopic, PSZ pszBuf,
                                     ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY RtfQueryTopicFragmentCount(HRTFTOPIC hTopic, PULONG pulCount);

/* ==================================================================
 * Find: fragment enumeration
 * ================================================================== */

APIRET APIENTRY RtfFindFirstFragment(HRTFTOPIC hTopic, PHRTFENUM phEnum);
APIRET APIENTRY RtfFindNextFragment(HRTFENUM hEnum);
APIRET APIENTRY RtfQueryEnumFragment(HRTFENUM hEnum, PHRTFFRAG phFrag);

/* ==================================================================
 * Query: fragment fields
 * ================================================================== */

APIRET APIENTRY RtfQueryFragType(HRTFFRAG hFrag, PULONG pulType);
APIRET APIENTRY RtfQueryFragText(HRTFFRAG hFrag, PSZ pszBuf,
                                 ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY RtfQueryFragContext(HRTFFRAG hFrag, PSZ pszBuf,
                                    ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY RtfQueryFragPopup(HRTFFRAG hFrag, PBOOL pfPopup);
APIRET APIENTRY RtfQueryFragFont(HRTFFRAG hFrag, PULONG pulFontIndex);

#ifdef __cplusplus
}
#endif

#endif /* RTF_H */
