/*!
 * @file rspinst.c
 *
 * @brief Parser implementation for OS/2 response files.
 *
 * @par References
 *   - IBM Redbook SG24-2010-00, section 7.1.5, "Include Keywords
 *     Processing Sequence".
 *   - IBM Redbook SG24-2010-00, section 7.2, "Feature Installer
 *     Response File" (nested block syntax).
 *   - IBM OS/2 Toolkit OS2DEF.H (LHANDLE, APIRET, ULONG, PCSZ,
 *     BOOL, APIENTRY).
 */

#include "rspinst_private.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

/* ==================================================================
 * Internal helpers - strings
 * ================================================================== */

/*!
 * @brief Duplicate a NUL-terminated string.
 *
 * @param[in] pszSrc  Source. May be NULL.
 *
 * @return A newly allocated copy, or NULL on failure or when
 *         @p pszSrc is NULL.
 *
 * @retval NULL     @p pszSrc is NULL or allocation failed.
 * @retval other    A newly allocated copy.
 */
static PSZ rsp_strdup(PCSZ pszSrc)
{
    size_t cbLen;
    PSZ    pszDst;

    if (pszSrc == NULL) {
        return NULL;
    }
    cbLen = strlen(pszSrc) + 1;
    pszDst = (PSZ)malloc(cbLen);
    if (pszDst != NULL) {
        memcpy(pszDst, pszSrc, cbLen);
    }
    return pszDst;
}

/*!
 * @brief Case-insensitive comparison of two NUL-terminated strings.
 *
 * @param[in] pszA  First string.
 * @param[in] pszB  Second string.
 *
 * @return Ordering of the two strings.
 *
 * @retval 0   The two strings are equal (case-insensitive).
 * @retval <0  @p pszA sorts before @p pszB.
 * @retval >0  @p pszA sorts after @p pszB.
 */
static int rsp_ci_cmp(PCSZ pszA, PCSZ pszB)
{
    int cA;
    int cB;

    for (;;) {
        cA = tolower((unsigned char)*pszA);
        cB = tolower((unsigned char)*pszB);
        if (cA != cB) {
            return cA - cB;
        }
        if (cA == 0) {
            return 0;
        }
        pszA++;
        pszB++;
    }
}

/*!
 * @brief Trim leading and trailing whitespace in place.
 *
 * @param[in,out] pszStr  String to trim.
 *
 * @return Pointer to the first non-whitespace character.
 *
 * @retval NULL     @p pszStr is NULL.
 * @retval other    A pointer into @p pszStr.
 */
static PSZ rsp_trim(PSZ pszStr)
{
    PSZ pszEnd;

    if (pszStr == NULL) {
        return NULL;
    }
    while (*pszStr != '\0' && isspace((unsigned char)*pszStr)) {
        pszStr++;
    }
    if (*pszStr == '\0') {
        return pszStr;
    }
    pszEnd = pszStr + strlen(pszStr) - 1;
    while (pszEnd > pszStr && isspace((unsigned char)*pszEnd)) {
        *pszEnd = '\0';
        pszEnd--;
    }
    return pszStr;
}

/*!
 * @brief Query whether a line begins a comment.
 *
 * @param[in] pszLine  Line, already left-trimmed. Not NULL.
 *
 * @return Boolean.
 *
 * @retval TRUE   The first non-whitespace character is '#', ';' or '*'.
 * @retval FALSE  Otherwise.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.2, comment rules.
 */
static BOOL rsp_is_comment(PCSZ pszLine)
{
    while (*pszLine != '\0' && isspace((unsigned char)*pszLine)) {
        pszLine++;
    }
    if (*pszLine == '#' || *pszLine == ';' || *pszLine == '*') {
        return TRUE;
    }
    return FALSE;
}

/* ==================================================================
 * Internal helpers - block storage
 * ================================================================== */

/*!
 * @brief Append a new entry to a block.
 *
 * @param[in] pBlock  Block. Not NULL.
 *
 * @return Pointer to the new entry, or NULL on allocation failure.
 *
 * @retval NULL     Allocation failure.
 * @retval other    A pointer to the newly appended entry.
 */
static RSP_PAIR *rsp_block_push(RSP_BLOCK *pBlock)
{
    RSP_PAIR *pNew;
    ULONG     ulNewCap;

    if (pBlock->ulCount == pBlock->ulCapacity) {
        ulNewCap = (pBlock->ulCapacity == 0)
                 ? 32
                 : pBlock->ulCapacity * 2;
        pNew = (RSP_PAIR *)realloc((void *)pBlock->paItems,
                                   (size_t)(ulNewCap * sizeof(RSP_PAIR)));
        if (pNew == NULL) {
            return NULL;
        }
        pBlock->paItems = pNew;
        pBlock->ulCapacity = ulNewCap;
    }
    pNew = &pBlock->paItems[pBlock->ulCount];
    memset(pNew, 0, sizeof(*pNew));
    pBlock->ulCount++;
    return pNew;
}

/*!
 * @brief Release the memory owned by a block and everything below
 *        it.
 *
 * @param[in] pBlock  Block. May be NULL.
 */
static void rsp_block_free(RSP_BLOCK *pBlock)
{
    ULONG i;

    if (pBlock == NULL) {
        return;
    }
    for (i = 0; i < pBlock->ulCount; i++) {
        free(pBlock->paItems[i].pszName);
        free(pBlock->paItems[i].pszValue);
        free(pBlock->paItems[i].pszFile);
        if (pBlock->paItems[i].pBlock != NULL) {
            rsp_block_free(pBlock->paItems[i].pBlock);
            free(pBlock->paItems[i].pBlock);
        }
    }
    free(pBlock->paItems);
    memset(pBlock, 0, sizeof(*pBlock));
}

/* ==================================================================
 * Internal helpers - include queue
 * ================================================================== */

/*!
 * @brief Append a deferred Include directive to the queue.
 *
 * @param[in] pQueue    Queue. Not NULL.
 * @param[in] pszPath   Path. Not NULL.
 * @param[in] ulLine    Line number of the Include directive.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET rsp_incqueue_push(RSP_INCQUEUE *pQueue, PCSZ pszPath,
                                ULONG ulLine)
{
    ULONG ulNewCap;
    PSZ  *papszNew;
    ULONG *paulNew;
    PSZ   pszCopy;

    if (pQueue->ulTail == pQueue->ulCapacity) {
        ulNewCap = (pQueue->ulCapacity == 0)
                 ? 16
                 : pQueue->ulCapacity * 2;
        papszNew = (PSZ *)realloc((void *)pQueue->papszPaths,
                                  (size_t)(ulNewCap * sizeof(PSZ)));
        if (papszNew == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        pQueue->papszPaths = papszNew;
        paulNew = (ULONG *)realloc((void *)pQueue->paulLines,
                                   (size_t)(ulNewCap * sizeof(ULONG)));
        if (paulNew == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        pQueue->paulLines = paulNew;
        pQueue->ulCapacity = ulNewCap;
    }
    pszCopy = rsp_strdup(pszPath);
    if (pszCopy == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pQueue->papszPaths[pQueue->ulTail] = pszCopy;
    pQueue->paulLines[pQueue->ulTail] = ulLine;
    pQueue->ulTail++;
    return NO_ERROR;
}

/*!
 * @brief Pop one path from the front of the queue.
 *
 * @param[in]  pQueue  Queue. Not NULL.
 * @param[out] pulLine Receiver for the source line of the directive,
 *                     or NULL.
 *
 * @return The popped path, or NULL when the queue is empty.
 *
 * @retval NULL     Queue is empty.
 * @retval other    A newly owned path string.
 */
static PSZ rsp_incqueue_pop(RSP_INCQUEUE *pQueue, PULONG pulLine)
{
    PSZ pszPath;

    if (pQueue->ulHead >= pQueue->ulTail) {
        return NULL;
    }
    pszPath = pQueue->papszPaths[pQueue->ulHead];
    if (pulLine != NULL) {
        *pulLine = pQueue->paulLines[pQueue->ulHead];
    }
    pQueue->papszPaths[pQueue->ulHead] = NULL;
    pQueue->ulHead++;
    return pszPath;
}

/*!
 * @brief Release every resource held by the queue.
 *
 * @param[in] pQueue  Queue. Not NULL.
 */
static void rsp_incqueue_free(RSP_INCQUEUE *pQueue)
{
    ULONG i;

    for (i = pQueue->ulHead; i < pQueue->ulTail; i++) {
        free(pQueue->papszPaths[i]);
    }
    free(pQueue->papszPaths);
    free(pQueue->paulLines);
    memset(pQueue, 0, sizeof(*pQueue));
}

/* ==================================================================
 * Internal helpers - path handling
 * ================================================================== */

/*!
 * @brief Query whether a path is absolute.
 *
 * Recognises "/...", "\\...", "X:..." and "\\\\server".
 *
 * @param[in] pszPath  Path.
 *
 * @return Boolean.
 *
 * @retval TRUE   The path is absolute.
 * @retval FALSE  The path is relative or @p pszPath is NULL/empty.
 *
 * @par Reference
 *   IBM OS/2 Command Reference, "File Names".
 */
static BOOL rsp_path_is_absolute(PCSZ pszPath)
{
    if (pszPath == NULL || *pszPath == '\0') {
        return FALSE;
    }
    if (pszPath[0] == '/' || pszPath[0] == '\\') {
        return TRUE;
    }
    if (pszPath[0] != '\0' && pszPath[1] == ':') {
        return TRUE;
    }
    return FALSE;
}

/*!
 * @brief Return the directory part of a path.
 *
 * @param[in] pszPath  Path.
 *
 * @return A newly allocated string holding the directory, or NULL on
 *         allocation failure. A path without a separator returns
 *         ".".
 *
 * @retval NULL     Allocation failure or @p pszPath is NULL.
 * @retval other    A newly allocated directory string.
 */
static PSZ rsp_path_dirname(PCSZ pszPath)
{
    PCSZ pszLastSep = NULL;
    PCSZ pszScan;
    size_t cbLen;
    PSZ  pszResult;

    if (pszPath == NULL) {
        return NULL;
    }
    for (pszScan = pszPath; *pszScan != '\0'; pszScan++) {
        if (*pszScan == '/' || *pszScan == '\\') {
            pszLastSep = pszScan;
        }
    }
    if (pszLastSep == NULL) {
        return rsp_strdup(".");
    }
    cbLen = (size_t)(pszLastSep - pszPath) + 1;
    pszResult = (PSZ)malloc(cbLen + 1);
    if (pszResult == NULL) {
        return NULL;
    }
    memcpy(pszResult, pszPath, cbLen);
    pszResult[cbLen] = '\0';
    return pszResult;
}

/*!
 * @brief Join a directory and a relative path.
 *
 * @param[in] pszDir  Directory. May be empty.
 * @param[in] pszRel  Relative path. Not NULL.
 *
 * @return A newly allocated string, or NULL on failure.
 *
 * @retval NULL     Allocation failure.
 * @retval other    A newly allocated joined path.
 */
static PSZ rsp_path_join(PCSZ pszDir, PCSZ pszRel)
{
    size_t cbDir;
    size_t cbRel;
    BOOL   fAddSep;
    PSZ    pszResult;

    if (pszDir == NULL || *pszDir == '\0') {
        return rsp_strdup(pszRel);
    }
    if (pszRel == NULL || *pszRel == '\0') {
        return rsp_strdup(pszDir);
    }
    if (rsp_path_is_absolute(pszRel)) {
        return rsp_strdup(pszRel);
    }
    cbDir = strlen(pszDir);
    cbRel = strlen(pszRel);
    fAddSep = (pszDir[cbDir - 1] != '/' && pszDir[cbDir - 1] != '\\')
            ? TRUE : FALSE;
    pszResult = (PSZ)malloc(cbDir + (fAddSep ? 1 : 0) + cbRel + 1);
    if (pszResult == NULL) {
        return NULL;
    }
    memcpy(pszResult, pszDir, cbDir);
    if (fAddSep) {
        pszResult[cbDir] = '/';
        cbDir++;
    }
    memcpy(pszResult + cbDir, pszRel, cbRel);
    pszResult[cbDir + cbRel] = '\0';
    return pszResult;
}

/*!
 * @brief Resolve a relative include path against the including file.
 *
 * @param[in] pszIncludingFile  Path of the including file.
 * @param[in] pszInclude        Path from the Include/IncludeInLine
 *                              directive.
 *
 * @return A newly allocated string, or NULL on failure.
 *
 * @retval NULL     Allocation failure.
 * @retval other    A newly allocated resolved path.
 */
static PSZ rsp_path_resolve(PCSZ pszIncludingFile, PCSZ pszInclude)
{
    PSZ pszDir;
    PSZ pszResult;

    if (pszInclude == NULL || *pszInclude == '\0') {
        return rsp_strdup(pszInclude);
    }
    if (rsp_path_is_absolute(pszInclude)) {
        return rsp_strdup(pszInclude);
    }
    pszDir = rsp_path_dirname(pszIncludingFile);
    if (pszDir == NULL) {
        return NULL;
    }
    pszResult = rsp_path_join(pszDir, pszInclude);
    free(pszDir);
    return pszResult;
}

/* ==================================================================
 * Internal helpers - visited files
 * ================================================================== */

/*!
 * @brief Test whether a file was already visited.
 *
 * @param[in] pDoc     Document. Not NULL.
 * @param[in] pszPath  Effective path.
 *
 * @return Boolean.
 *
 * @retval TRUE   The path is already present.
 * @retval FALSE  Otherwise.
 */
static BOOL rsp_visited_contains(RSP_DOC *pDoc, PCSZ pszPath)
{
    ULONG i;

    for (i = 0; i < pDoc->ulVisitedCount; i++) {
        if (rsp_ci_cmp(pDoc->papszVisited[i], pszPath) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

/*!
 * @brief Register a visited file.
 *
 * @param[in] pDoc     Document. Not NULL.
 * @param[in] pszPath  Effective path.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 Success.
 * @retval RSP_ERROR_TOO_MANY_FILES Too many distinct files.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 */
static APIRET rsp_visited_add(RSP_DOC *pDoc, PCSZ pszPath)
{
    ULONG ulNewCap;
    PSZ  *papszNew;
    PSZ   pszCopy;

    if (pDoc->ulVisitedCount >= RSP_MAX_FILES) {
        return RSP_ERROR_TOO_MANY_FILES;
    }
    if (pDoc->ulVisitedCount == pDoc->ulVisitedCapacity) {
        ulNewCap = (pDoc->ulVisitedCapacity == 0)
                 ? 16
                 : pDoc->ulVisitedCapacity * 2;
        papszNew = (PSZ *)realloc((void *)pDoc->papszVisited,
                                  (size_t)(ulNewCap * sizeof(PSZ)));
        if (papszNew == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        pDoc->papszVisited = papszNew;
        pDoc->ulVisitedCapacity = ulNewCap;
    }
    pszCopy = rsp_strdup(pszPath);
    if (pszCopy == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pDoc->papszVisited[pDoc->ulVisitedCount] = pszCopy;
    pDoc->ulVisitedCount++;
    return NO_ERROR;
}

/*!
 * @brief Release every entry of the visited list.
 *
 * @param[in] pDoc  Document. Not NULL.
 */
static void rsp_visited_free(RSP_DOC *pDoc)
{
    ULONG i;

    for (i = 0; i < pDoc->ulVisitedCount; i++) {
        free(pDoc->papszVisited[i]);
    }
    free(pDoc->papszVisited);
    pDoc->papszVisited = NULL;
    pDoc->ulVisitedCount = 0;
    pDoc->ulVisitedCapacity = 0;
}

/* ==================================================================
 * Internal helpers - line reader
 * ================================================================== */

/*!
 * @brief Read one physical line from a stream.
 *
 * Handles LF, CRLF and lone CR. Lines longer than @p cbBuf are
 * consumed up to the newline and reported as
 * RSP_ERROR_LINE_TOO_LONG.
 *
 * @param[in]  pFile     Open stream. Not NULL.
 * @param[out] pszBuf    Line buffer. Not NULL.
 * @param[in]  cbBuf     Buffer size in bytes.
 * @param[in]  pulLine   Current line number. Incremented on return.
 * @param[in]  pDoc      Document, for error counting. Not NULL.
 * @param[out] pfGotLine Receives TRUE when a line was read, FALSE
 *                       on end of file.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 A line was read or end of file
 *                                  was reached (see @p pfGotLine).
 * @retval RSP_ERROR_LINE_TOO_LONG  The physical line exceeds
 *                                  @p cbBuf.
 */
static APIRET rsp_read_line(FILE *pFile, PSZ pszBuf, size_t cbBuf,
                            PULONG pulLine, RSP_DOC *pDoc,
                            PBOOL pfGotLine)
{
    size_t cbUsed = 0;
    int    cCurrent;
    BOOL   fAnyByte = FALSE;

    *pfGotLine = FALSE;
    while ((cCurrent = fgetc(pFile)) != EOF) {
        fAnyByte = TRUE;
        if (cCurrent == '\n') {
            break;
        }
        if (cCurrent == '\r') {
            int cNext = fgetc(pFile);
            if (cNext != '\n' && cNext != EOF) {
                ungetc(cNext, pFile);
            }
            break;
        }
        if (cbUsed + 1 >= cbBuf) {
            while ((cCurrent = fgetc(pFile)) != EOF &&
                   cCurrent != '\n' &&
                   cCurrent != '\r') {
                /* discard */
            }
            if (cCurrent == '\r') {
                int cNext = fgetc(pFile);
                if (cNext != '\n' && cNext != EOF) {
                    ungetc(cNext, pFile);
                }
            }
            (*pulLine)++;
            pDoc->ulErrorCount++;
            return RSP_ERROR_LINE_TOO_LONG;
        }
        pszBuf[cbUsed++] = (char)cCurrent;
    }
    if (!fAnyByte && cCurrent == EOF) {
        return NO_ERROR;
    }
    pszBuf[cbUsed] = '\0';
    (*pulLine)++;
    *pfGotLine = TRUE;
    return NO_ERROR;
}

/* ==================================================================
 * Internal helpers - block body parser
 * ================================================================== */

/*!
 * @brief Read the body of a "KEY=( ... )" block until ")".
 *
 * @param[in]  pFile    Open stream. Not NULL.
 * @param[in]  pBlock   Destination block. Not NULL.
 * @param[in]  pszFile  Effective path of the file. Not NULL.
 * @param[in]  pulLine  Current line number. Updated on return.
 * @param[in]  ulDepth  Current recursion depth.
 * @param[in]  pDoc     Document. Not NULL.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 Success.
 * @retval RSP_ERROR_INCLUDE_DEPTH  Recursion too deep.
 * @retval RSP_ERROR_UNEXPECTED_EOF End of file before ")".
 * @retval RSP_ERROR_LINE_TOO_LONG  A physical line is too long.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @par Reference
 *   Nested block syntax, IBM Redbook SG24-2010-00, section 7.2.
 */
static APIRET rsp_read_block_body(FILE *pFile, RSP_BLOCK *pBlock,
                                  PCSZ pszFile, PULONG pulLine,
                                  ULONG ulDepth, RSP_DOC *pDoc)
{
    char   achLine[RSP_LINE_MAX];
    BOOL   fGotLine;
    APIRET rc;
    PSZ    pszTrimmed;
    PSZ    pszEq;
    PSZ    pszKey;
    PSZ    pszVal;
    RSP_PAIR *pEntry;

    if (ulDepth > RSP_MAX_INCLUDE_DEPTH) {
        return RSP_ERROR_INCLUDE_DEPTH;
    }
    for (;;) {
        fGotLine = FALSE;
        rc = rsp_read_line(pFile, achLine, sizeof(achLine),
                           pulLine, pDoc, &fGotLine);
        if (rc != NO_ERROR) {
            return rc;
        }
        if (!fGotLine) {
            return RSP_ERROR_UNEXPECTED_EOF;
        }
        pszTrimmed = rsp_trim(achLine);
        if (*pszTrimmed == '\0' || rsp_is_comment(pszTrimmed)) {
            continue;
        }
        if (strcmp(pszTrimmed, ")") == 0) {
            return NO_ERROR;
        }
        pszEq = strchr(pszTrimmed, '=');
        if (pszEq == NULL) {
            pDoc->ulWarningCount++;
            continue;
        }
        *pszEq = '\0';
        pszKey = rsp_trim(pszTrimmed);
        pszVal = rsp_trim(pszEq + 1);
        if (*pszKey == '\0') {
            continue;
        }
        pEntry = rsp_block_push(pBlock);
        if (pEntry == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        pEntry->pszName = rsp_strdup(pszKey);
        pEntry->pszFile = rsp_strdup(pszFile);
        pEntry->ulLine = *pulLine;
        if (pEntry->pszName == NULL || pEntry->pszFile == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        if (pszVal[0] == '(' && pszVal[1] == '\0') {
            pEntry->pBlock = (RSP_BLOCK *)calloc(1, sizeof(RSP_BLOCK));
            if (pEntry->pBlock == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            rc = rsp_read_block_body(pFile, pEntry->pBlock, pszFile,
                                     pulLine, ulDepth + 1, pDoc);
            if (rc != NO_ERROR) {
                return rc;
            }
        } else {
            pEntry->pszValue = rsp_strdup(pszVal);
            if (pEntry->pszValue == NULL) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }
}

/* ==================================================================
 * Internal helpers - file reader
 * ================================================================== */

/*!
 * @brief Read one file into the document.
 *
 * Processes IncludeInLine recursively, queues Include for later and
 * stores every other keyword in @p pRoot.
 *
 * @param[in] pDoc      Document. Not NULL.
 * @param[in] pRoot     Destination block. Not NULL.
 * @param[in] pszPath   Effective path of the file. Not NULL.
 * @param[in] ulDepth   Current recursion depth.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 Success.
 * @retval RSP_ERROR_INCLUDE_DEPTH  Recursion too deep.
 * @retval RSP_ERROR_CYCLIC_INCLUDE The file has already been read.
 * @retval RSP_ERROR_LINE_TOO_LONG  A physical line is too long.
 * @retval RSP_ERROR_UNEXPECTED_EOF End of file inside a block.
 * @retval RSP_ERROR_TOO_MANY_FILES Too many distinct files.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.5.
 */
static APIRET rsp_read_file(RSP_DOC *pDoc, RSP_BLOCK *pRoot,
                            PCSZ pszPath, ULONG ulDepth)
{
    FILE      *pFile;
    char       achLine[RSP_LINE_MAX];
    ULONG      ulLine = 0;
    BOOL       fGotLine;
    APIRET     rc;
    PSZ        pszTrimmed;
    PSZ        pszEq;
    PSZ        pszKey;
    PSZ        pszVal;
    RSP_PAIR  *pEntry;

    if (ulDepth > RSP_MAX_INCLUDE_DEPTH) {
        return RSP_ERROR_INCLUDE_DEPTH;
    }
    if (rsp_visited_contains(pDoc, pszPath)) {
        return RSP_ERROR_CYCLIC_INCLUDE;
    }
    pFile = fopen(pszPath, "rb");
    if (pFile == NULL) {
        return ERROR_OPEN_FAILED;
    }
    rc = rsp_visited_add(pDoc, pszPath);
    if (rc != NO_ERROR) {
        fclose(pFile);
        return rc;
    }
    for (;;) {
        fGotLine = FALSE;
        rc = rsp_read_line(pFile, achLine, sizeof(achLine),
                           &ulLine, pDoc, &fGotLine);
        if (rc != NO_ERROR) {
            fclose(pFile);
            return rc;
        }
        if (!fGotLine) {
            break;
        }
        pszTrimmed = rsp_trim(achLine);
        if (*pszTrimmed == '\0' || rsp_is_comment(pszTrimmed)) {
            continue;
        }
        pszEq = strchr(pszTrimmed, '=');
        if (pszEq == NULL) {
            pDoc->ulWarningCount++;
            continue;
        }
        *pszEq = '\0';
        pszKey = rsp_trim(pszTrimmed);
        pszVal = rsp_trim(pszEq + 1);
        if (*pszKey == '\0') {
            continue;
        }
        if (rsp_ci_cmp(pszKey, "IncludeInLine") == 0) {
            PSZ pszResolved = rsp_path_resolve(pszPath, pszVal);
            if (pszResolved == NULL) {
                fclose(pFile);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            rc = rsp_read_file(pDoc, pRoot, pszResolved, ulDepth + 1);
            free(pszResolved);
            if (rc != NO_ERROR) {
                fclose(pFile);
                return rc;
            }
            continue;
        }
        if (rsp_ci_cmp(pszKey, "Include") == 0) {
            PSZ pszResolved = rsp_path_resolve(pszPath, pszVal);
            if (pszResolved == NULL) {
                fclose(pFile);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            rc = rsp_incqueue_push(&pDoc->stIncludes, pszResolved,
                                   ulLine);
            free(pszResolved);
            if (rc != NO_ERROR) {
                fclose(pFile);
                return rc;
            }
            continue;
        }
        pEntry = rsp_block_push(pRoot);
        if (pEntry == NULL) {
            fclose(pFile);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        pEntry->pszName = rsp_strdup(pszKey);
        pEntry->pszFile = rsp_strdup(pszPath);
        pEntry->ulLine = ulLine;
        if (pEntry->pszName == NULL || pEntry->pszFile == NULL) {
            fclose(pFile);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        if (pszVal[0] == '(' && pszVal[1] == '\0') {
            pEntry->pBlock = (RSP_BLOCK *)calloc(1, sizeof(RSP_BLOCK));
            if (pEntry->pBlock == NULL) {
                fclose(pFile);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            rc = rsp_read_block_body(pFile, pEntry->pBlock, pszPath,
                                     &ulLine, ulDepth + 1, pDoc);
            if (rc != NO_ERROR) {
                fclose(pFile);
                return rc;
            }
        } else {
            pEntry->pszValue = rsp_strdup(pszVal);
            if (pEntry->pszValue == NULL) {
                fclose(pFile);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }
    fclose(pFile);
    return NO_ERROR;
}

/* ==================================================================
 * Internal helpers - document
 * ================================================================== */

/*!
 * @brief Release every resource owned by a document.
 *
 * @param[in] pDoc  Document. May be NULL.
 */
static void rsp_doc_free(RSP_DOC *pDoc)
{
    if (pDoc == NULL) {
        return;
    }
    pDoc->ulMagic = 0;
    free(pDoc->pszSourcePath);
    rsp_block_free(&pDoc->stRoot);
    rsp_incqueue_free(&pDoc->stIncludes);
    rsp_visited_free(pDoc);
    free(pDoc);
}

/*!
 * @brief Obtain the RSP_DOC structure from a handle, or NULL.
 *
 * @param[in] hDoc  Handle.
 *
 * @return The internal structure, or NULL on invalid handle.
 *
 * @retval NULL     @p hDoc is NULL or its magic is wrong.
 * @retval other    A pointer to the live document.
 */
RSP_DOC * APIENTRY rsp_doc_from_handle(HRSPINST hDoc)
{
    RSP_DOC *pDoc = (RSP_DOC *)hDoc;

    if (pDoc == NULL || pDoc->ulMagic != RSP_DOC_MAGIC) {
        return NULL;
    }
    return pDoc;
}

/*!
 * @brief Obtain the RSP_REPORT structure from a handle, or NULL.
 *
 * @param[in] hReport  Handle.
 *
 * @return The internal structure, or NULL on invalid handle.
 *
 * @retval NULL     @p hReport is NULL or its magic is wrong.
 * @retval other    A pointer to the live report.
 */
RSP_REPORT * APIENTRY rsp_report_from_handle(HRSPREPORT hReport)
{
    RSP_REPORT *pReport = (RSP_REPORT *)hReport;

    if (pReport == NULL || pReport->ulMagic != RSP_REPORT_MAGIC) {
        return NULL;
    }
    return pReport;
}

/* ==================================================================
 * Public API - document lifecycle
 * ================================================================== */

/*!
 * @brief Open and parse a response file.
 *
 * Reads @p pszPath, resolves Include and IncludeInLine references
 * recursively, and stores every entry in document order. All
 * buffers are owned by the resulting handle.
 *
 * @par Reference
 *   IBM Redbook SG24-2010-00, section 7.1.5.
 *
 * @param[in]  pszPath  Path to the response file. Not NULL.
 * @param[out] phDoc    Handle receiver. Not NULL. Set to NULLHANDLE
 *                      on error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszPath or @p phDoc is NULL.
 * @retval ERROR_OPEN_FAILED        File cannot be opened.
 * @retval ERROR_READ_FAULT         Read error.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 * @retval RSP_ERROR_SYNTAX         Syntax error.
 * @retval RSP_ERROR_LINE_TOO_LONG  Line exceeds RSP_LINE_MAX.
 * @retval RSP_ERROR_UNEXPECTED_EOF End of file inside an open block.
 * @retval RSP_ERROR_INCLUDE_DEPTH  Include nesting too deep.
 * @retval RSP_ERROR_CYCLIC_INCLUDE Include cycle detected.
 * @retval RSP_ERROR_TOO_MANY_FILES Too many distinct files.
 *
 * @see RspClose
 */
APIRET APIENTRY RspOpen(PCSZ pszPath, HRSPINST *phDoc)
{
    RSP_DOC *pDoc;
    APIRET   rc;

    if (pszPath == NULL || phDoc == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *phDoc = NULLHANDLE;

    pDoc = (RSP_DOC *)calloc(1, sizeof(RSP_DOC));
    if (pDoc == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pDoc->ulMagic = RSP_DOC_MAGIC;
    pDoc->pszSourcePath = rsp_strdup(pszPath);
    if (pDoc->pszSourcePath == NULL) {
        rsp_doc_free(pDoc);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    rc = rsp_read_file(pDoc, &pDoc->stRoot, pszPath, 0);
    if (rc != NO_ERROR) {
        rsp_doc_free(pDoc);
        return rc;
    }

    for (;;) {
        PSZ   pszNext;
        ULONG ulLine = 0;

        pszNext = rsp_incqueue_pop(&pDoc->stIncludes, &ulLine);
        if (pszNext == NULL) {
            break;
        }
        rc = rsp_read_file(pDoc, &pDoc->stRoot, pszNext, 1);
        free(pszNext);
        if (rc != NO_ERROR) {
            rsp_doc_free(pDoc);
            return rc;
        }
    }

    *phDoc = (HRSPINST)pDoc;
    return NO_ERROR;
}

/*!
 * @brief Release a parsed response file.
 *
 * Every entry, every include list and every buffer owned by the
 * handle is released. Passing NULLHANDLE is a no-op.
 *
 * @param[in] hDoc  Handle. NULLHANDLE is accepted.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 *
 * @see RspOpen
 */
APIRET APIENTRY RspClose(HRSPINST hDoc)
{
    RSP_DOC *pDoc;

    if (hDoc == NULLHANDLE) {
        return NO_ERROR;
    }
    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL) {
        return ERROR_INVALID_HANDLE;
    }
    rsp_doc_free(pDoc);
    return NO_ERROR;
}

/* ==================================================================
 * Public API - document metadata
 * ================================================================== */

/*!
 * @brief Query the original path passed to RspOpen.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_INVALID_DATA       Stored source path is missing.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspQuerySourcePath(HRSPINST hDoc, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed)
{
    RSP_DOC *pDoc;
    size_t   cbNeed;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if (pDoc->pszSourcePath == NULL) {
        return ERROR_INVALID_DATA;
    }
    cbNeed = strlen(pDoc->pszSourcePath);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return NO_ERROR;
    }
    if (pszBuf == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if ((size_t)ulSize < cbNeed + 1) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pDoc->pszSourcePath, cbNeed + 1);
    if (pulUsed != NULL) {
        *pulUsed = (ULONG)cbNeed;
    }
    return NO_ERROR;
}

/*!
 * @brief Query the number of entries at the top level.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspQueryEntryCount(HRSPINST hDoc, PULONG pulCount)
{
    RSP_DOC *pDoc;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || pulCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *pulCount = pDoc->stRoot.ulCount;
    return NO_ERROR;
}

/*!
 * @brief Query the number of parse errors recorded while opening.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspQueryErrorCount(HRSPINST hDoc, PULONG pulCount)
{
    RSP_DOC *pDoc;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || pulCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *pulCount = pDoc->ulErrorCount;
    return NO_ERROR;
}

/*!
 * @brief Query the number of parse warnings recorded while opening.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspQueryWarningCount(HRSPINST hDoc, PULONG pulCount)
{
    RSP_DOC *pDoc;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || pulCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *pulCount = pDoc->ulWarningCount;
    return NO_ERROR;
}

/*!
 * @brief Query the version string.
 *
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p pszBuf is NULL without size
 *                                  query.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspQueryVersion(PSZ pszBuf, ULONG ulSize, PULONG pulUsed)
{
    size_t cbNeed;

    cbNeed = strlen(RSP_VERSION_STRING);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return NO_ERROR;
    }
    if (pszBuf == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if ((size_t)ulSize < cbNeed + 1) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, RSP_VERSION_STRING, cbNeed + 1);
    if (pulUsed != NULL) {
        *pulUsed = (ULONG)cbNeed;
    }
    return NO_ERROR;
}

/* ==================================================================
 * Public API - entry access
 * ================================================================== */

/*!
 * @brief Obtain a borrowed handle to the entry at the top level.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] phAnn    Receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY RspQueryEntry(HRSPINST hDoc, ULONG ulIndex,
                              HRSPANN *phAnn)
{
    RSP_DOC *pDoc;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || phAnn == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *phAnn = NULLHANDLE;
    if (ulIndex >= pDoc->stRoot.ulCount) {
        return ERROR_NO_MORE_ITEMS;
    }
    *phAnn = (HRSPANN)&pDoc->stRoot.paItems[ulIndex];
    return NO_ERROR;
}

/*!
 * @brief Query the type of an entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pulType  Receiver of RSP_ENTRYTYPE_*. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 */
APIRET APIENTRY RspEntryQueryType(HRSPANN hAnn, PULONG pulType)
{
    RSP_PAIR *pPair = (RSP_PAIR *)hAnn;

    if (pPair == NULL || pulType == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *pulType = (pPair->pBlock != NULL)
             ? RSP_ENTRYTYPE_BLOCK
             : RSP_ENTRYTYPE_SCALAR;
    return NO_ERROR;
}

/*!
 * @brief Query the keyword name of an entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspEntryQueryName(HRSPANN hAnn, PSZ pszBuf,
                                  ULONG ulSize, PULONG pulUsed)
{
    RSP_PAIR *pPair = (RSP_PAIR *)hAnn;
    size_t    cbNeed;

    if (pPair == NULL || pPair->pszName == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    cbNeed = strlen(pPair->pszName);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return NO_ERROR;
    }
    if (pszBuf == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if ((size_t)ulSize < cbNeed + 1) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pPair->pszName, cbNeed + 1);
    if (pulUsed != NULL) {
        *pulUsed = (ULONG)cbNeed;
    }
    return NO_ERROR;
}

/*!
 * @brief Query the value of a scalar entry.
 *
 * Blocks have no scalar value; the function returns
 * ERROR_INVALID_DATA for a block handle.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_INVALID_DATA       Entry is a block.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspEntryQueryValue(HRSPANN hAnn, PSZ pszBuf,
                                   ULONG ulSize, PULONG pulUsed)
{
    RSP_PAIR *pPair = (RSP_PAIR *)hAnn;
    size_t    cbNeed;

    if (pPair == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if (pPair->pBlock != NULL) {
        return ERROR_INVALID_DATA;
    }
    if (pPair->pszValue == NULL) {
        return ERROR_INVALID_DATA;
    }
    cbNeed = strlen(pPair->pszValue);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return NO_ERROR;
    }
    if (pszBuf == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if ((size_t)ulSize < cbNeed + 1) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pPair->pszValue, cbNeed + 1);
    if (pulUsed != NULL) {
        *pulUsed = (ULONG)cbNeed;
    }
    return NO_ERROR;
}

/*!
 * @brief Query the source file path of an entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pszBuf   Output buffer, or NULL for size query.
 * @param[in]  ulSize   Size of @p pszBuf in bytes.
 * @param[out] pulUsed  Optional. May be NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_BUFFER_OVERFLOW    Buffer too small.
 */
APIRET APIENTRY RspEntryQuerySourceFile(HRSPANN hAnn, PSZ pszBuf,
                                        ULONG ulSize, PULONG pulUsed)
{
    RSP_PAIR *pPair = (RSP_PAIR *)hAnn;
    size_t    cbNeed;

    if (pPair == NULL || pPair->pszFile == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    cbNeed = strlen(pPair->pszFile);
    if (pszBuf == NULL && ulSize == 0) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return NO_ERROR;
    }
    if (pszBuf == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if ((size_t)ulSize < cbNeed + 1) {
        if (pulUsed != NULL) {
            *pulUsed = (ULONG)cbNeed + 1;
        }
        return ERROR_BUFFER_OVERFLOW;
    }
    memcpy(pszBuf, pPair->pszFile, cbNeed + 1);
    if (pulUsed != NULL) {
        *pulUsed = (ULONG)cbNeed;
    }
    return NO_ERROR;
}

/*!
 * @brief Query the 1-based source line of an entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pulLine  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 */
APIRET APIENTRY RspEntryQueryLineNumber(HRSPANN hAnn, PULONG pulLine)
{
    RSP_PAIR *pPair = (RSP_PAIR *)hAnn;

    if (pPair == NULL || pulLine == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *pulLine = pPair->ulLine;
    return NO_ERROR;
}

/*!
 * @brief Query the number of children of a block entry.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_INVALID_DATA       Entry is a scalar.
 */
APIRET APIENTRY RspEntryQueryChildCount(HRSPANN hAnn, PULONG pulCount)
{
    RSP_PAIR *pPair = (RSP_PAIR *)hAnn;

    if (pPair == NULL || pulCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if (pPair->pBlock == NULL) {
        return ERROR_INVALID_DATA;
    }
    *pulCount = pPair->pBlock->ulCount;
    return NO_ERROR;
}

/*!
 * @brief Obtain a borrowed handle to one child of a block.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based child index.
 * @param[out] phChild  Receiver. Not NULL. Set to NULLHANDLE on
 *                      error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_INVALID_DATA       Entry is a scalar.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY RspEntryQueryChild(HRSPANN hAnn, ULONG ulIndex,
                                   HRSPANN *phChild)
{
    RSP_PAIR *pPair = (RSP_PAIR *)hAnn;

    if (pPair == NULL || phChild == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *phChild = NULLHANDLE;
    if (pPair->pBlock == NULL) {
        return ERROR_INVALID_DATA;
    }
    if (ulIndex >= pPair->pBlock->ulCount) {
        return ERROR_NO_MORE_ITEMS;
    }
    *phChild = (HRSPANN)&pPair->pBlock->paItems[ulIndex];
    return NO_ERROR;
}

/*!
 * @brief Find a child of a block by name.
 *
 * The comparison is case-insensitive. If several children share
 * the name, the last one wins.
 *
 * @param[in]  hAnn     Entry handle. Not NULLHANDLE.
 * @param[in]  pszName  Child name. Not NULL.
 * @param[out] phChild  Receiver. Not NULL. Set to NULLHANDLE when
 *                      the child is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Entry handle not recognized.
 * @retval ERROR_INVALID_DATA       Entry is a scalar.
 * @retval ERROR_FILE_NOT_FOUND     No child with that name.
 */
APIRET APIENTRY RspEntryFindChild(HRSPANN hAnn, PCSZ pszName,
                                  HRSPANN *phChild)
{
    RSP_PAIR *pPair = (RSP_PAIR *)hAnn;
    ULONG     i;

    if (pPair == NULL || pszName == NULL || phChild == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *phChild = NULLHANDLE;
    if (pPair->pBlock == NULL) {
        return ERROR_INVALID_DATA;
    }
    i = pPair->pBlock->ulCount;
    while (i > 0) {
        i--;
        if (rsp_ci_cmp(pPair->pBlock->paItems[i].pszName, pszName) == 0) {
            *phChild = (HRSPANN)&pPair->pBlock->paItems[i];
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/* ==================================================================
 * Public API - top-level lookup
 * ================================================================== */

/*!
 * @brief Find the last top-level entry with the given name.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  pszName  Keyword name. Not NULL.
 * @param[out] phAnn    Receiver. Not NULL. Set to NULLHANDLE when
 *                      the keyword is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
APIRET APIENTRY RspFindLast(HRSPINST hDoc, PCSZ pszName, HRSPANN *phAnn)
{
    RSP_DOC *pDoc;
    ULONG    i;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || pszName == NULL || phAnn == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *phAnn = NULLHANDLE;
    i = pDoc->stRoot.ulCount;
    while (i > 0) {
        i--;
        if (rsp_ci_cmp(pDoc->stRoot.paItems[i].pszName, pszName) == 0) {
            *phAnn = (HRSPANN)&pDoc->stRoot.paItems[i];
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Find the first top-level entry with the given name.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  pszName  Keyword name. Not NULL.
 * @param[out] phAnn    Receiver. Not NULL. Set to NULLHANDLE when
 *                      the keyword is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
APIRET APIENTRY RspFindFirst(HRSPINST hDoc, PCSZ pszName, HRSPANN *phAnn)
{
    RSP_DOC *pDoc;
    ULONG    i;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || pszName == NULL || phAnn == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *phAnn = NULLHANDLE;
    for (i = 0; i < pDoc->stRoot.ulCount; i++) {
        if (rsp_ci_cmp(pDoc->stRoot.paItems[i].pszName, pszName) == 0) {
            *phAnn = (HRSPANN)&pDoc->stRoot.paItems[i];
            return NO_ERROR;
        }
    }
    return ERROR_FILE_NOT_FOUND;
}

/*!
 * @brief Count top-level entries with the given name.
 *
 * @param[in]  hDoc     Handle. Not NULLHANDLE.
 * @param[in]  pszName  Keyword name. Not NULL.
 * @param[out] pulCount Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspCount(HRSPINST hDoc, PCSZ pszName, PULONG pulCount)
{
    RSP_DOC *pDoc;
    ULONG    i;
    ULONG    ulFound = 0;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || pszName == NULL || pulCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    for (i = 0; i < pDoc->stRoot.ulCount; i++) {
        if (rsp_ci_cmp(pDoc->stRoot.paItems[i].pszName, pszName) == 0) {
            ulFound++;
        }
    }
    *pulCount = ulFound;
    return NO_ERROR;
}

/*!
 * @brief Traverse a dotted path in the document.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[in]  pszDotted Dotted path. Not NULL.
 * @param[in]  fFirst    TRUE to prefer the first match.
 * @param[out] phAnn     Receiver. Not NULL.
 *
 * @return APIRET.
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
static APIRET rsp_find_dotted_common(HRSPINST hDoc, PCSZ pszDotted,
                                     BOOL fFirst, HRSPANN *phAnn)
{
    RSP_PAIR  *pPair = NULL;
    RSP_BLOCK *pBlock;
    PCSZ       pszRemaining;
    RSP_DOC   *pDoc;
    APIRET     rc;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || pszDotted == NULL || phAnn == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *phAnn = NULLHANDLE;

    rc = fFirst ? RspFindFirst(hDoc, pszDotted, phAnn)
                : RspFindLast(hDoc, pszDotted, phAnn);
    if (rc == NO_ERROR) {
        return NO_ERROR;
    }

    pBlock = &pDoc->stRoot;
    pszRemaining = pszDotted;
    for (;;) {
        const char *pszDot;
        char       achName[256];
        size_t     cbName;
        ULONG      i;
        BOOL       fFoundThisSegment = FALSE;

        pszDot = strchr(pszRemaining, '.');
        if (pszDot == NULL) {
            cbName = strlen(pszRemaining);
            if (cbName >= sizeof(achName)) {
                return ERROR_FILE_NOT_FOUND;
            }
            memcpy(achName, pszRemaining, cbName);
            achName[cbName] = '\0';
            if (fFirst) {
                for (i = 0; i < pBlock->ulCount; i++) {
                    if (pBlock->paItems[i].pBlock == NULL &&
                        rsp_ci_cmp(pBlock->paItems[i].pszName,
                                   achName) == 0) {
                        *phAnn = (HRSPANN)&pBlock->paItems[i];
                        return NO_ERROR;
                    }
                }
            } else {
                i = pBlock->ulCount;
                while (i > 0) {
                    i--;
                    if (pBlock->paItems[i].pBlock == NULL &&
                        rsp_ci_cmp(pBlock->paItems[i].pszName,
                                   achName) == 0) {
                        *phAnn = (HRSPANN)&pBlock->paItems[i];
                        return NO_ERROR;
                    }
                }
            }
            return ERROR_FILE_NOT_FOUND;
        }
        cbName = (size_t)(pszDot - pszRemaining);
        if (cbName == 0 || cbName >= sizeof(achName)) {
            return ERROR_FILE_NOT_FOUND;
        }
        memcpy(achName, pszRemaining, cbName);
        achName[cbName] = '\0';
        for (i = 0; i < pBlock->ulCount; i++) {
            if (pBlock->paItems[i].pBlock != NULL &&
                rsp_ci_cmp(pBlock->paItems[i].pszName, achName) == 0) {
                pPair = &pBlock->paItems[i];
                fFoundThisSegment = TRUE;
                break;
            }
        }
        if (!fFoundThisSegment) {
            return ERROR_FILE_NOT_FOUND;
        }
        pBlock = pPair->pBlock;
        pszRemaining = pszDot + 1;
    }
}

/*!
 * @brief Find the last top-level entry using a dotted path.
 *
 * The path "A.B.C" first tries the flat keyword "A.B.C" (for
 * keywords such as "ART.Selection"), then walks the nested blocks
 * A -> B -> C.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[in]  pszDotted Dotted keyword path. Not NULL.
 * @param[out] phAnn     Receiver. Not NULL. Set to NULLHANDLE when
 *                       the keyword is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
APIRET APIENTRY RspFindLastDotted(HRSPINST hDoc, PCSZ pszDotted,
                                  HRSPANN *phAnn)
{
    return rsp_find_dotted_common(hDoc, pszDotted, FALSE, phAnn);
}

/*!
 * @brief Find the first top-level entry using a dotted path.
 *
 * @param[in]  hDoc      Handle. Not NULLHANDLE.
 * @param[in]  pszDotted Dotted keyword path. Not NULL.
 * @param[out] phAnn     Receiver. Not NULL. Set to NULLHANDLE when
 *                       the keyword is not present.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_FILE_NOT_FOUND     No matching entry.
 */
APIRET APIENTRY RspFindFirstDotted(HRSPINST hDoc, PCSZ pszDotted,
                                   HRSPANN *phAnn)
{
    return rsp_find_dotted_common(hDoc, pszDotted, TRUE, phAnn);
}

/* ==================================================================
 * Public API - dump
 * ================================================================== */

/*!
 * @brief Emit a string through the callback of RspDump.
 *
 * @param[in] pfnWrite  Callback. Not NULL.
 * @param[in] pvUser    Context.
 * @param[in] pszText   Text to emit. Not NULL.
 *
 * @return APIRET from the callback.
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_WRITE_FAULT Callback rejected the chunk.
 */
static APIRET rsp_emit(RSPWRITEFN pfnWrite, PVOID pvUser, PCSZ pszText)
{
    return pfnWrite(pszText, (ULONG)strlen(pszText), pvUser);
}

/*!
 * @brief Recursive worker for RspDump in nested form.
 *
 * @param[in] pBlock    Block. Not NULL.
 * @param[in] pfnWrite  Callback. Not NULL.
 * @param[in] pvUser    Context.
 * @param[in] ulDepth   Current indentation depth.
 *
 * @return APIRET from the callback chain.
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_WRITE_FAULT Callback rejected a chunk.
 */
static APIRET rsp_dump_nested(const RSP_BLOCK *pBlock,
                              RSPWRITEFN pfnWrite, PVOID pvUser,
                              ULONG ulDepth)
{
    ULONG  i;
    ULONG  j;
    APIRET rc;
    char   achBuf[RSP_LINE_MAX];

    for (i = 0; i < pBlock->ulCount; i++) {
        for (j = 0; j < ulDepth; j++) {
            rc = rsp_emit(pfnWrite, pvUser, "  ");
            if (rc != NO_ERROR) {
                return rc;
            }
        }
        if (pBlock->paItems[i].pBlock != NULL) {
            sprintf(achBuf, "%s=(\n", pBlock->paItems[i].pszName);
            rc = rsp_emit(pfnWrite, pvUser, achBuf);
            if (rc != NO_ERROR) {
                return rc;
            }
            rc = rsp_dump_nested(pBlock->paItems[i].pBlock, pfnWrite,
                                 pvUser, ulDepth + 1);
            if (rc != NO_ERROR) {
                return rc;
            }
            for (j = 0; j < ulDepth; j++) {
                rc = rsp_emit(pfnWrite, pvUser, "  ");
                if (rc != NO_ERROR) {
                    return rc;
                }
            }
            rc = rsp_emit(pfnWrite, pvUser, ")\n");
            if (rc != NO_ERROR) {
                return rc;
            }
        } else {
            sprintf(achBuf, "%s=%s\n",
                    pBlock->paItems[i].pszName,
                    pBlock->paItems[i].pszValue != NULL
                        ? pBlock->paItems[i].pszValue : "");
            rc = rsp_emit(pfnWrite, pvUser, achBuf);
            if (rc != NO_ERROR) {
                return rc;
            }
        }
    }
    return NO_ERROR;
}

/*!
 * @brief Recursive worker for RspDump in flat INI form.
 *
 * @param[in] pBlock    Block. Not NULL.
 * @param[in] pfnWrite  Callback. Not NULL.
 * @param[in] pvUser    Context.
 * @param[in] pszPrefix Dotted prefix, or "" for the root.
 *
 * @return APIRET from the callback chain.
 *
 * @retval NO_ERROR          Success.
 * @retval ERROR_WRITE_FAULT Callback rejected a chunk.
 */
static APIRET rsp_dump_flat(const RSP_BLOCK *pBlock,
                            RSPWRITEFN pfnWrite, PVOID pvUser,
                            PCSZ pszPrefix)
{
    ULONG  i;
    APIRET rc;
    char   achBuf[RSP_LINE_MAX];

    for (i = 0; i < pBlock->ulCount; i++) {
        if (pBlock->paItems[i].pBlock != NULL) {
            char achChild[RSP_LINE_MAX];
            if (*pszPrefix == '\0') {
                sprintf(achChild, "%s", pBlock->paItems[i].pszName);
            } else {
                sprintf(achChild, "%s.%s", pszPrefix,
                        pBlock->paItems[i].pszName);
            }
            rc = rsp_dump_flat(pBlock->paItems[i].pBlock, pfnWrite,
                               pvUser, achChild);
            if (rc != NO_ERROR) {
                return rc;
            }
        } else {
            if (*pszPrefix == '\0') {
                sprintf(achBuf, "%s=%s\n",
                        pBlock->paItems[i].pszName,
                        pBlock->paItems[i].pszValue != NULL
                            ? pBlock->paItems[i].pszValue : "");
            } else {
                sprintf(achBuf, "%s.%s=%s\n", pszPrefix,
                        pBlock->paItems[i].pszName,
                        pBlock->paItems[i].pszValue != NULL
                            ? pBlock->paItems[i].pszValue : "");
            }
            rc = rsp_emit(pfnWrite, pvUser, achBuf);
            if (rc != NO_ERROR) {
                return rc;
            }
        }
    }
    return NO_ERROR;
}

/*!
 * @brief Serialise the whole document through a callback.
 *
 * When @p fIniStyle is FALSE, the output is the original nested
 * form: each entry as "KEY=value", each block as "KEY=(" followed
 * by indented children and ")". When @p fIniStyle is TRUE, every
 * scalar is flattened to "A.B.C=value" and blocks are not echoed
 * separately.
 *
 * @param[in] hDoc       Handle. Not NULLHANDLE.
 * @param[in] fIniStyle  TRUE for flat INI form, FALSE for nested.
 * @param[in] pfnWrite   Callback. Not NULL.
 * @param[in] pvUser     Context passed to @p pfnWrite.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p hDoc or @p pfnWrite is NULL.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_WRITE_FAULT        @p pfnWrite rejected a chunk.
 */
APIRET APIENTRY RspDump(HRSPINST hDoc, BOOL fIniStyle,
                        RSPWRITEFN pfnWrite, PVOID pvUser)
{
    RSP_DOC *pDoc;

    pDoc = rsp_doc_from_handle(hDoc);
    if (pDoc == NULL || pfnWrite == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if (fIniStyle) {
        return rsp_dump_flat(&pDoc->stRoot, pfnWrite, pvUser, "");
    }
    return rsp_dump_nested(&pDoc->stRoot, pfnWrite, pvUser, 0);
}

/* ==================================================================
 * Public API - validation report
 * ================================================================== */

/*!
 * @brief Create an empty validation report.
 *
 * @param[out] phReport  Receiver. Not NULL. Set to NULLHANDLE on
 *                       error.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @p phReport is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failure.
 *
 * @see RspReportFree
 */
APIRET APIENTRY RspReportCreate(HRSPREPORT *phReport)
{
    RSP_REPORT *pReport;

    if (phReport == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *phReport = NULLHANDLE;
    pReport = (RSP_REPORT *)calloc(1, sizeof(RSP_REPORT));
    if (pReport == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pReport->ulMagic = RSP_REPORT_MAGIC;
    *phReport = (HRSPREPORT)pReport;
    return NO_ERROR;
}

/*!
 * @brief Release a validation report.
 *
 * @param[in] hReport  Handle. NULLHANDLE is a no-op.
 *
 * @return APIRET
 *
 * @retval NO_ERROR               Success. Also for NULLHANDLE.
 * @retval ERROR_INVALID_HANDLE   Handle not recognized.
 */
APIRET APIENTRY RspReportFree(HRSPREPORT hReport)
{
    RSP_REPORT *pReport;

    if (hReport == NULLHANDLE) {
        return NO_ERROR;
    }
    pReport = rsp_report_from_handle(hReport);
    if (pReport == NULL) {
        return ERROR_INVALID_HANDLE;
    }
    pReport->ulMagic = 0;
    free(pReport->paItems);
    free(pReport);
    return NO_ERROR;
}

/*!
 * @brief Query the number of records in a report.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspReportQueryCount(HRSPREPORT hReport, PULONG pulCount)
{
    RSP_REPORT *pReport;

    pReport = rsp_report_from_handle(hReport);
    if (pReport == NULL || pulCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *pulCount = pReport->ulCount;
    return NO_ERROR;
}

/*!
 * @brief Query the number of records with severity RSP_SEV_ERROR.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspReportQueryErrorCount(HRSPREPORT hReport,
                                         PULONG pulCount)
{
    RSP_REPORT *pReport;

    pReport = rsp_report_from_handle(hReport);
    if (pReport == NULL || pulCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *pulCount = pReport->ulErrors;
    return NO_ERROR;
}

/*!
 * @brief Query the number of records with severity RSP_SEV_WARNING.
 *
 * @param[in]  hReport   Handle. Not NULLHANDLE.
 * @param[out] pulCount  Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 */
APIRET APIENTRY RspReportQueryWarningCount(HRSPREPORT hReport,
                                           PULONG pulCount)
{
    RSP_REPORT *pReport;

    pReport = rsp_report_from_handle(hReport);
    if (pReport == NULL || pulCount == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *pulCount = pReport->ulWarnings;
    return NO_ERROR;
}

/*!
 * @brief Retrieve one diagnostic record by index.
 *
 * The returned structure is a snapshot; string fields point to
 * memory owned by the report and remain valid until RspReportFree.
 *
 * @param[in]  hReport  Handle. Not NULLHANDLE.
 * @param[in]  ulIndex  Zero-based index.
 * @param[out] pErr     Receiver. Not NULL.
 *
 * @return APIRET
 *
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Bad parameter.
 * @retval ERROR_INVALID_HANDLE     Handle not recognized.
 * @retval ERROR_NO_MORE_ITEMS      Index out of range.
 */
APIRET APIENTRY RspReportQueryEntry(HRSPREPORT hReport, ULONG ulIndex,
                                    PRSPERR pErr)
{
    RSP_REPORT *pReport;

    pReport = rsp_report_from_handle(hReport);
    if (pReport == NULL || pErr == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    if (ulIndex >= pReport->ulCount) {
        return ERROR_NO_MORE_ITEMS;
    }
    *pErr = pReport->paItems[ulIndex];
    return NO_ERROR;
}
