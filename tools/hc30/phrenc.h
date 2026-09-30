/*!
 * @file phrenc.h
 * @brief Phrase-table encoder for WinHelp |Phrases and |TOPIC.
 *
 * @par Function naming
 * OS/2 CP API, OS/2 V2.0 Vol.4 section 12.2.
 *
 * @par References
 *  [1] Winterhoff, M., helpdeco (1997), PhraseLoad, PhraseReplace.
 *  [2] SAA CPI C Reference - Level 2, SC09-1308-02 (Sep 1991).
 *
 * @par Compression format (HC30, old style)
 * The |Phrases file stores a 16-bit phrase count, a 16-bit marker
 * 0x0100, an offset array of count+1 16-bit words (offsets include
 * the array size itself), and the phrase bodies.
 *
 * Text inside |TOPIC is encoded as follows:
 *   - Byte 0x00: literal NUL.
 *   - Byte in 0x01..0x0F: start of a phrase reference. The next byte
 *     completes the value; the phrase index is
 *     (256*(b1-1) + b2) / 2 and an odd value requests a trailing
 *     space after the phrase.
 *   - Any other byte: literal byte.
 *
 * The maximum number of phrases is 1920.
 */
#ifndef PHRENC_H
#define PHRENC_H

#include "os2types.h"
#include "os2err.h"
#include <stdio.h>

typedef HANDLE HPHRE;
typedef HPHRE  *PHPHRE;

/*!
 * @brief Create an empty phrase encoder.
 *
 * @param[out] phEnc Receiver. Not NULL. Set to NULLHANDLE on error.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a phEnc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrEncCreate(PHPHRE phEnc);

/*!
 * @brief Destroy a phrase encoder and release all its data.
 *
 * @param[in] hEnc Handle. May be NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR              Success.
 * @retval ERROR_INVALID_HANDLE  Handle is not recognized.
 */
APIRET APIENTRY PhrEncDestroy(HPHRE hEnc);

/*!
 * @brief Add a text to the corpus used for phrase table building.
 *
 * @param[in] hEnc    Handle. Not NULLHANDLE.
 * @param[in] pszText Text to scan. Not NULL.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrEncAddText(HPHRE hEnc, PCSZ pszText);

/*!
 * @brief Build the phrase table from the accumulated corpus.
 *
 * Must be called after all texts have been added and before any
 * calls to PhrEncEncode or PhrEncWrite.
 *
 * @param[in] hEnc Handle. Not NULLHANDLE.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  @a hEnc is NULL.
 * @retval ERROR_NOT_ENOUGH_MEMORY  Allocation failed.
 */
APIRET APIENTRY PhrEncBuildTable(HPHRE hEnc);

/*!
 * @brief Encode a string using the phrase table.
 *
 * @param[in]  hEnc     Handle. Not NULLHANDLE.
 * @param[in]  pszIn    Input string. Not NULL.
 * @param[out] pbOut    Output buffer. Not NULL.
 * @param[in]  cbOut    Size of the output buffer.
 * @param[out] pcbUsed  Receives the number of bytes written.
 *
 * @return APIRET
 * @retval NO_ERROR                 Success.
 * @retval ERROR_INVALID_PARAMETER  Any parameter is NULL.
 * @retval ERROR_BUFFER_OVERFLOW    Output buffer is too small.
 */
APIRET APIENTRY PhrEncEncode(HPHRE hEnc, PCSZ pszIn, PBYTE pbOut,
                             ULONG cbOut, PULONG pcbUsed);

/*!
 * @brief Query the number of phrases in the table.
 */
APIRET APIENTRY PhrEncQueryCount(HPHRE hEnc, PULONG pulCount);

/*!
 * @brief Write the |Phrases file to a stream.
 */
APIRET APIENTRY PhrEncWrite(HPHRE hEnc, FILE* f);

#endif /* PHRENC_H */
