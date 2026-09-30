/*!
 * @file topic.h
 * @brief |TOPIC internal file generator for WinHelp 3.0 (HC30).
 *
 * @par References
 *  [1] Winterhoff, M., helpdeco (1997), TOPICLINK, TOPICHEADER30,
 *      TOPICBLOCKHEADER, TopicRead.
 *  [2] MSDN Q75010, WinHelp 3.0 RTF subset.
 *  [3] ccl container library: vector.h.
 *  [4] phrenc.h.
 */
#ifndef TOP_H
#define TOP_H

#include "os2types.h"
#include "os2err.h"
#include "ccl.h"
#include "phrenc.h"
#include <stdio.h>

typedef HANDLE HTOP;
typedef HTOP  *PHTOP;

#define TOP_BLOCK_SIZE      0x0800
#define TOP_UNRESOLVED_LINK 0xFFFFFFFF

APIRET APIENTRY TopCreate(PHTOP phTop);
APIRET APIENTRY TopDestroy(HTOP hTop);
APIRET APIENTRY TopAddTopic(HTOP hTop, PCSZ pszTitle, PCSZ pszContext);
APIRET APIENTRY TopAddText(HTOP hTop, PCSZ pszText, ULONG ulFontIndex);
APIRET APIENTRY TopAddLink(HTOP hTop, ULONG ulTargetTopic, BOOL fPopup);
APIRET APIENTRY TopQueryTopicCount(HTOP hTop, PULONG pulCount);
APIRET APIENTRY TopWrite(HTOP hTop, FILE* f, PHVECTOR phvOffsets, HPHRE hEnc);

#endif /* TOP_H */
