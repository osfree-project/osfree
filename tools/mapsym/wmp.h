/*!
 * @file  wmp.h
 * @brief Open Watcom WLINK map file reader.
 */
#ifndef WMP_H
#define WMP_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Handle to an open Watcom map file. */
typedef HANDLE HWMP;

/*! @brief Iteration cursor over symbols. */
typedef HANDLE HWMPFIND;

/*! @brief Segment flag: 32-bit symbol values. */
#define WMP_SEG_32BIT  0x01

/*! @brief Segment flag: alphabetic symbol table present. */
#define WMP_SEG_ALPHA  0x02

APIRET APIENTRY WmpOpen(PCSZ pszPath, HWMP *phWmp);
APIRET APIENTRY WmpClose(HWMP hWmp);

APIRET APIENTRY WmpQueryModule(HWMP hWmp, PSZ pszBuf,
                               ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY WmpQuerySymbolCount(HWMP hWmp, PULONG pulCount);

APIRET APIENTRY WmpQuerySegmentCount(HWMP hWmp, PULONG pulCount);
APIRET APIENTRY WmpQuerySegmentName(HWMP hWmp, ULONG ulSegIdx,
                                    PSZ pszBuf, ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY WmpQuerySegmentClass(HWMP hWmp, ULONG ulSegIdx,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY WmpQuerySegmentGroup(HWMP hWmp, ULONG ulSegIdx,
                                     PSZ pszBuf, ULONG ulSize, PULONG pulUsed);
APIRET APIENTRY WmpQuerySegmentFlags(HWMP hWmp, ULONG ulSegIdx,
                                     PULONG pulFlags);
APIRET APIENTRY WmpQuerySegmentSize(HWMP hWmp, ULONG ulSegIdx,
                                    PULONG pulSize);
APIRET APIENTRY WmpQuerySegmentBaseOff(HWMP hWmp, ULONG ulSegIdx,
                                       PULONG pulBaseOff);
APIRET APIENTRY WmpQuerySegmentSegNum(HWMP hWmp, ULONG ulSegIdx,
                                      PULONG pulSegNum);
APIRET APIENTRY WmpQuerySegmentSymbolCount(HWMP hWmp, ULONG ulSegIdx,
                                           PULONG pulCount);

APIRET APIENTRY WmpQuerySymbolValue(HWMP hWmp, ULONG ulSegIdx,
                                    ULONG ulSymIdx, PULONG pulValue);
APIRET APIENTRY WmpQuerySymbolName(HWMP hWmp, ULONG ulSegIdx,
                                   ULONG ulSymIdx, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed);

APIRET APIENTRY WmpFindSymbolByValue(HWMP hWmp, ULONG ulSegIdx,
                                     ULONG ulValue, PULONG pulSymIdx);
APIRET APIENTRY WmpFindSymbolByName(HWMP hWmp, PCSZ pszName,
                                    PULONG pulSegIdx, PULONG pulSymIdx);

APIRET APIENTRY WmpFindFirstSymbol(HWMP hWmp, HWMPFIND *phFind,
                                   PULONG pulSegIdx, PULONG pulSymIdx,
                                   PSZ pszName, ULONG ulNameSize,
                                   PULONG pulNameUsed, PULONG pulValue);
APIRET APIENTRY WmpFindNextSymbol(HWMPFIND hFind,
                                  PULONG pulSegIdx, PULONG pulSymIdx,
                                  PSZ pszName, ULONG ulNameSize,
                                  PULONG pulNameUsed, PULONG pulValue);
APIRET APIENTRY WmpFindClose(HWMPFIND hFind);

#ifdef __cplusplus
}
#endif
#endif /* WMP_H */
