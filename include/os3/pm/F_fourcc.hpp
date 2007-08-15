/* F_fourcc.hpp */
/*****************************************************************************\
* Module Name: F_FOURCC.hpp
*
* Color space definitions, used in FREEPM & HW drivers
* analog of fourcc.h
\*****************************************************************************/

#ifndef _HWFOURCC_H_
#define _HWFOURCC_H_

/* Notes:
 *  F_dive.hpp/dive.h contain #define FOURCC ULONG, so don't use FOURCC again!  
 *  mmio.h contain mmioFOURCC
 *  we will use independend macro FOUR_CH
 *  todo: what about FOUR_CH and MMX ?
 */
  #define FOUR_CH( ch0, ch1, ch2, ch3 )                      \
    ( (ULONG)(BYTE)(ch0) | ( (ULONG)(BYTE)(ch1) << 8 ) |    \
    ( (ULONG)(BYTE)(ch2) << 16 ) | ((ULONG)(BYTE)(ch3) << 24 ) )

  #define FOURCC_SCRN  0
  #define FOURCC_LUT4 FOUR_CH('L','U','T','4')  /* 8-bit palettized, high 4 bits unused */
  #define FOURCC_LUT8 FOUR_CH('L','U','T','8')  /* 8-bit palettized */
  #define FOURCC_R565 FOUR_CH('R','5','6','5')  /* RGB 565 */
  #define FOURCC_R555 FOUR_CH('R','5','5','5')  /* RGB 555 */
  #define FOURCC_R666 FOUR_CH('R','6','6','6')  /* RGB 666 */
  #define FOURCC_R664 FOUR_CH('R','6','6','4')  /* RGB 664 */
  #define FOURCC_RGB3 FOUR_CH('R','G','B','3')  /* RGB 24 in 3 bytes */
  #define FOURCC_BGR3 FOUR_CH('B','G','R','3')  /* BGR 24 in 3 bytes */
  #define FOURCC_RGB4 FOUR_CH('R','G','B','4')  /* RGB 24 in 4 bytes */
  #define FOURCC_BGR4 FOUR_CH('B','G','R','4')  /* BGR 24 in 4 bytes */
  #define FOURCC_Y888 FOUR_CH('Y','8','8','8')  /* YUV 24 */
  #define FOURCC_Y411 FOUR_CH('Y','4','1','1')  /* YUV 411 interleaved 4 by 1 subsampled*/
  #define FOURCC_Y422 FOUR_CH('Y','4','2','2')  /* YUV 422 (CCIR601) */
  #define FOURCC_YUV9 FOUR_CH('Y','U','V','9')  /* YUV9 */
  #define FOURCC_Y2X2 FOUR_CH('Y','2','X','2')  /* YUV 2 by 2 subsampled multi-plane */
  #define FOURCC_Y4X4 FOUR_CH('Y','4','X','4')  /* YUV 4 by 4 subsampled multi-plane */
  #define FOURCC_Y644 FOUR_CH('Y','6','4','4')  /* two-plane format*/
  #define FOURCC_MONO FOUR_CH('M','O','N','O')
  #define FOURCC_GY16 FOUR_CH('G','Y','1','6')
  #define FOURCC_LT12 FOUR_CH('L','T','1','2')
  #define FOURCC_GREY FOUR_CH('G','R','E','Y')  /* 8bpp greyscale */

// Generic colour definitions for use in Radeon_GetColourCode ().

#define BLACK            0
#define DARKBLUE         1
#define DARKGREEN        2
#define DARKCYAN         3
#define DARKRED          4
#define DARKMAGENTA      5
#define BROWN            6
#define LIGHTGRAY        7
#define DARKGRAY         8
#define LIGHTBLUE        9
#define LIGHTGREEN      10
#define LIGHTCYAN       11
#define LIGHTRED        12
#define LIGHTMAGENTA    13
#define YELLOW          14
#define WHITE           15
#define NUM_COLOURS     16


#endif /* _HWFOURCC_H_ */
