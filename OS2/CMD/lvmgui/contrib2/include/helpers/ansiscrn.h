
/*
 *@@sourcefile ansiscrn.h:
 *      quick ANSI screen code macros for use with printf().
 *      This file does not correspond to any .C code file.
 *
 *      Usage: Any C code on platforms which support ANSI
 *             escape sequences.
 *
 *      Based on ANSISCRN.H, which was
 *      contributed to the public domain 12-26-91 by
 *      Matthew J. Glass.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 */

#ifndef   ANSISCRN_INCLUDED
#define   ANSISCRN_INCLUDED

#include   <stdio.h>

#define   ESC                  27
#define   ANSI_cup(a,b)        printf("%c[%d;%dH",ESC,a,b)
#define   ANSI_up(a)           printf("%c[%dA",ESC,a)
#define   ANSI_down(a)         printf("%c[%dB",ESC,a)
#define   ANSI_right(a)        printf("%c[%dC",ESC,a)
#define   ANSI_left(a)         printf("%c[%dD",ESC,a)
#define   ANSI_locate(a,b)     printf("%c[%d;%df",ESC,a,b)
#define   ANSI_savecurs()      printf("%c[S",ESC)
#define   ANSI_restcurs()      printf("%c[U",ESC)
#define   ANSI_cls()           printf("%c[2J",ESC)
#define   ANSI_cleol()         printf("%c[K",ESC)
#define   ANSI_margins(a,b)    printf("%c[%d;%dr",ESC,a,b)

#define   NORMAL         0     /* attributes for ANSI_attrib() */
#define   BOLD           1
#define   USCORE         2
#define   BLINK          3
#define   REVERSE        4
#define   INVIS          5

#define   BLACK          0     /* colors for ANSI_bg_color() and */
#define   RED            1     /* ANSI_fg_color.                 */
#define   GREEN          2
#define   YELLOW         3
#define   BLUE           4
#define   MAGENTA        5
#define   CYAN           6
#define   WHITE          7
#define   B_BLACK        8     /* bright colors for ANSI_fg_color() */
#define   B_RED          9
#define   B_GREEN        10
#define   B_YELLOW       11
#define   B_BLUE         12
#define   B_MAGENTA      13
#define   B_CYAN         14
#define   B_WHITE        15

static char *_atrb_plt[] = {
   "0","1","4","5","7","8"
   };

static char *_fg_plt[] = {
   "0;30","0;31","0;32","0;33",
   "0;34","0;35","0;36","0;37",
   "1;30","1;31","1;32","1;33",
   "1;34","1;35","1;36","1;37"
   };

static char *_bg_plt[] = {
   "40","41","42","43",
   "44","45","46","47"
    };

#define   ANSI_attrib(a)     printf("%c[%sm",ESC,_atrb_plt[a])
#define   ANSI_fg_color(a)   printf("%c[%sm",ESC, _fg_plt[a] )
#define   ANSI_bg_color(a)   printf("%c[%sm",ESC, _bg_plt[a] )

#endif /* ANSISCRN */
