/*!
 * @file  map.h
 * @brief Microsoft LINK map file reader.
 *
 * Reads .map files produced by the Microsoft Segmented Executable
 * Linker and compatible tools. Produces an opaque HMAP handle.
 */
#ifndef MAP_H
#define MAP_H

#include "os2types.h"
#include "os2err.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief Handle to an open Microsoft map file. */
typedef HANDLE HMAP;

/*! @brief Iteration cursor over symbols. */
typedef HANDLE HMAPFIND;

/*! @brief Segment flag: 32-bit symbol values. */
#define MAP_SEG_32BIT  0x01

/*! @brief Segment flag: alphabetic symbol table present. */
#define MAP_SEG_ALPHA  0x02

/* ==================================================================
 * Lifecycle
 * ================================================================== */

/*!
 * @brief Open a Microsoft LINK map file.
 * @param pszPath  Path to the map file. Not NULL.
 * @param phMap    Receives the new handle. Not NULL.
 * @return APIRET.
 * @retval NO_ERROR                Success.
 * @retval ERROR_INVALID_PARAMETER pszPath or phMap is NULL.
 * @retval ERROR_OPEN_FAILED       File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY Allocation failure.
 * @retval ERROR_INVALID_DATA      File is not a Microsoft LINK map.
 */
APIRET APIENTRY MapOpen(PCSZ pszPath, HMAP *phMap);

/*!
 * @brief Close a map handle.
 * @param hMap Handle. NULLHANDLE is a no-op.
 * @return APIRET.
 */
APIRET APIENTRY MapClose(HMAP hMap);

/* ==================================================================
 * Document information
 * ================================================================== */

/*! @brief Set the module name. */
APIRET APIENTRY MapSetModule(HMAP hMap, PCSZ pszModule);

/*! @brief Query the module name. */
APIRET APIENTRY MapQueryModule(HMAP hMap, PSZ pszBuf,
                               ULONG ulSize, PULONG pulUsed);

/*! @brief Query the total number of symbols across all segments. */
APIRET APIENTRY MapQuerySymbolCount(HMAP hMap, PULONG pulCount);

/* ==================================================================
 * Segments
 * ================================================================== */

/*! @brief Query the number of segments. */
APIRET APIENTRY MapQuerySegmentCount(HMAP hMap, PULONG pulCount);

/*! @brief Query the name of a segment. */
APIRET APIENTRY MapQuerySegmentName(HMAP hMap, ULONG ulSegIdx,
                                    PSZ pszBuf, ULONG ulSize,
                                    PULONG pulUsed);

/*! @brief Query the class of a segment (Microsoft-specific). */
APIRET APIENTRY MapQuerySegmentClass(HMAP hMap, ULONG ulSegIdx,
                                     PSZ pszBuf, ULONG ulSize,
                                     PULONG pulUsed);

/*! @brief Query the flags of a segment (MAP_SEG_*). */
APIRET APIENTRY MapQuerySegmentFlags(HMAP hMap, ULONG ulSegIdx,
                                     PULONG pulFlags);

/*! @brief Query the size of a segment in bytes. */
APIRET APIENTRY MapQuerySegmentSize(HMAP hMap, ULONG ulSegIdx,
                                    PULONG pulSize);

/*! @brief Query the base offset of a segment. */
APIRET APIENTRY MapQuerySegmentBaseOff(HMAP hMap, ULONG ulSegIdx,
                                       PULONG pulBaseOff);

/*! @brief Query the segment number of a segment. */
APIRET APIENTRY MapQuerySegmentSegNum(HMAP hMap, ULONG ulSegIdx,
                                      PULONG pulSegNum);

/*! @brief Query the number of symbols in one segment. */
APIRET APIENTRY MapQuerySegmentSymbolCount(HMAP hMap, ULONG ulSegIdx,
                                           PULONG pulCount);

/* ==================================================================
 * Symbols
 * ================================================================== */

/*! @brief Query the value of a symbol. */
APIRET APIENTRY MapQuerySymbolValue(HMAP hMap, ULONG ulSegIdx,
                                    ULONG ulSymIdx, PULONG pulValue);

/*! @brief Query the name of a symbol. */
APIRET APIENTRY MapQuerySymbolName(HMAP hMap, ULONG ulSegIdx,
                                   ULONG ulSymIdx, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed);

/* ==================================================================
 * Search
 * ================================================================== */

/*! @brief Find a symbol by its exact value. */
APIRET APIENTRY MapFindSymbolByValue(HMAP hMap, ULONG ulSegIdx,
                                     ULONG ulValue, PULONG pulSymIdx);

/*! @brief Find a symbol by name across all segments. */
APIRET APIENTRY MapFindSymbolByName(HMAP hMap, PCSZ pszName,
                                    PULONG pulSegIdx, PULONG pulSymIdx);

/* ==================================================================
 * Iteration (DosFindFirst / DosFindNext / DosFindClose pattern)
 * ================================================================== */

/*! @brief Start iterating over all symbols. */
APIRET APIENTRY MapFindFirstSymbol(HMAP hMap, HMAPFIND *phFind,
                                   PULONG pulSegIdx, PULONG pulSymIdx,
                                   PSZ pszName, ULONG ulNameSize,
                                   PULONG pulNameUsed, PULONG pulValue);

/*! @brief Advance the iteration to the next symbol. */
APIRET APIENTRY MapFindNextSymbol(HMAPFIND hFind,
                                  PULONG pulSegIdx, PULONG pulSymIdx,
                                  PSZ pszName, ULONG ulNameSize,
                                  PULONG pulNameUsed, PULONG pulValue);

/*! @brief Close an iteration cursor. */
APIRET APIENTRY MapFindClose(HMAPFIND hFind);

#ifdef __cplusplus
}
#endif
#endif /* MAP_H */
