/*!
 * @file  sym.h
 * @brief MAPSYM (.SYM) file reader and writer.
 */
#ifndef SYM_H
#define SYM_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief File is not a valid .SYM. */
#define SYM_ERROR_INVALID_SYNTAX     0xFF01
/*! @brief Unknown format version. */
#define SYM_ERROR_BAD_VERSION        0xFF02
/*! @brief Segment count exceeds the format limit (1024). */
#define SYM_ERROR_TOO_MANY_SEGMENTS  0xFF03
/*! @brief Segment index out of range. */
#define SYM_ERROR_SEGMENT_NOT_FOUND  0xFF04
/*! @brief Symbol count exceeds the format limit (10000/segment). */
#define SYM_ERROR_TOO_MANY_SYMBOLS   0xFF05

#define SYM_MAX_SEGMENTS             1024
#define SYM_MAX_SYMBOLS_PER_SEG      10000
#define SYM_MAX_SYM_NAME             255
#define SYM_MAX_SEG_NAME             255
#define SYM_MAX_MOD_NAME             128

#define SYM_OPEN_READ        0x0001
#define SYM_OPEN_WRITE       0x0002
#define SYM_OPEN_EXISTING    0x0000
#define SYM_OPEN_CREATE      0x0010
#define SYM_OPEN_TRUNCATE    0x0020

#define SYM_SEGDEF_32BIT     0x01
#define SYM_SEGDEF_ALPHA     0x02

typedef HANDLE HSYMFILE;
typedef HANDLE HSYMFIND;

APIRET APIENTRY SymOpen(PCSZ pszPath, HSYMFILE *phSym, ULONG flOpen);
APIRET APIENTRY SymClose(HSYMFILE hSym);

APIRET APIENTRY SymQueryModule(HSYMFILE hSym, PSZ pszBuf,
                               ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY SymQuerySegmentCount(HSYMFILE hSym, PULONG pulCount);
APIRET APIENTRY SymQuerySymbolCount(HSYMFILE hSym, PULONG pulCount);

APIRET APIENTRY SymQuerySegmentName(HSYMFILE hSym, ULONG ulSegIdx,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY SymQuerySegmentFlags(HSYMFILE hSym, ULONG ulSegIdx,
                                     PULONG pulFlags);
APIRET APIENTRY SymQuerySegmentSymbolCount(HSYMFILE hSym, ULONG ulSegIdx,
                                           PULONG pulCount);

APIRET APIENTRY SymQuerySymbolValue(HSYMFILE hSym, ULONG ulSegIdx,
                                    ULONG ulSymIdx, PULONG pulValue);
APIRET APIENTRY SymQuerySymbolName(HSYMFILE hSym, ULONG ulSegIdx,
                                   ULONG ulSymIdx, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed);

APIRET APIENTRY SymFindSymbolByValue(HSYMFILE hSym, ULONG ulSegIdx,
                                     ULONG ulValue, PULONG pulSymIdx);
APIRET APIENTRY SymFindSymbolByName(HSYMFILE hSym, PCSZ pszName,
                                    PULONG pulSegIdx, PULONG pulSymIdx);

APIRET APIENTRY SymEnumFirst(HSYMFILE hSym, HSYMFIND *phEnum);
APIRET APIENTRY SymEnumNext(HSYMFIND hFind);
APIRET APIENTRY SymEnumGetSegIdx(HSYMFIND hFind, PULONG pulSegIdx);
APIRET APIENTRY SymEnumGetValue(HSYMFIND hFind, PULONG pulValue);
APIRET APIENTRY SymEnumGetName(HSYMFIND hFind, PSZ pszBuf,
                               ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY SymEnumClose(HSYMFIND hFind);

APIRET APIENTRY SymSetModule(HSYMFILE hSym, PCSZ pszModule);
APIRET APIENTRY SymAddSegment(HSYMFILE hSym, PCSZ pszName,
                              ULONG ulFlags, PULONG pulSegIdx);
APIRET APIENTRY SymAddSymbol(HSYMFILE hSym, ULONG ulSegIdx,
                             ULONG ulValue, PCSZ pszName);
APIRET APIENTRY SymFlush(HSYMFILE hSym);

#ifdef __cplusplus
}
#endif
#endif /* SYM_H */
