/* display.h */
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define IDD_TEXT                    101
#define IDD_TEXT2                   102
#define IDD_TEXT4                   104

   /* default color table indices */

   #define CLR_FALSE                     (-5)
   #define CLR_TRUE                      (-4)

   #define CLR_ERROR                   (-255)
   #define CLR_DEFAULT                   (-3)
   #define CLR_BLACK                     (-1)
   #define CLR_BACKGROUND                  0
   #define D_BLUE                        1
   #define D_RED                         2
   #define D_PINK                        3
   #define D_GREEN                       4
   #define D_CYAN                        5
   #define D_YELLOW                      6
   #define D_NEUTRAL                     7

   #define D_DARKGRAY                    8
   #define D_DARKBLUE                    9
   #define D_DARKRED                    10
   #define D_DARKPINK                   11
   #define D_DARKGREEN                  12
   #define D_DARKCYAN                   13
   #define D_BROWN                      14
   #define D_WHITE                        15


#define BLACK_BG    0
#define BLACK_FG    10
#define LT_WHITE_BG 1
#define LT_WHITE_FG 11
#define BLUE_BG     2
#define LT_CYAN_FG  3
#define HIGHLIGHTED_BG 1
#define HIGHLIGHTED_FG 4
#define GRAY_FG 5

#define WHITE_FG             11 //11
#define PANEL_ATTRIBUTE         (D_NEUTRAL<<4)|D_BLUE //CLR_NEUTRAL // 6
#define MESSAGE_BAR_ATTRIBUTE                  D_WHITE               
#define PANEL_BORDER_ATTRIBUTE  (D_NEUTRAL<<4)|D_BLUE
#define MENU_ATTRIBUTE          (D_BLUE<<4)|D_WHITE
#define MENU_BORDER_ATTRIBUTE   (D_BLUE<<4)|11
#define ENTRY_ATTRIBUTE         (D_BLUE<<4)|D_WHITE 
#define ENTRY_BORDER_ATTRIBUTE  (D_BLUE<<4)|12
#define CHOICE_ATTRIBUTE        (D_BLUE<<4)|D_WHITE 
#define CHOICE_BORDER_ATTRIBUTE (D_BLUE<<4)|12

#define INFO_ATTRIBUTE 9
#define INFO_BORDER_ATTRIBUTE 80
#define WARNING_ATTRIBUTE     0xf
#define WARNING_BORDER_ATTRIBUTE 0xc0
#define ERROR_ATTRIBUTE          0xf
#define ERROR_BORDER_ATTRIBUTE   0xc0
#define HELP_ATTRIBUTE           (D_WHITE<<4)
#define HELP_BORDER_ATTRIBUTE    (D_NEUTRAL<<4)|12


#endif //__DISPLAY_H__
