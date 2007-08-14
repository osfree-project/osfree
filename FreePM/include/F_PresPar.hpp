/* 
 $Id: F_PresPar.hpp,v 1.1 2002/09/13 11:47:11 evgen2 Exp $ 
*/
#ifndef FREEPM_PRESPARAM
  #define FREEPM_PRESPARAM

struct WinPresParam
{
   int ForegroundColor; /*  PP_FOREGROUNDCOLOR  Foreground color (in RGB) attribute. */
   int BackgroungColor; /*  PP_BACKGROUNDCOLOR  Background color (in RGB) attribute. */
   int HiliteForegroundColor;/* PP_HILITEFOREGROUNDCOLOR Highlighted foreground color (in RGB) attribute, for example for selected menu items.  */
   int HiliteBackgroungColor;/* PP_HILITEBACKGROUNDCOLOR Highlighted background color (in RGB) attribute. */
   int DisabledForegroundColor; /* PP_DISABLEDFOREGROUNDCOLOR Disabled foreground color (in RGB) attribute. */
   int DisabledBackgroungColor; /* PP_DISABLEDBACKGROUNDCOLOR Disabled background color (in RGB) attribute. */
   int BorderColor;      /* PP_BORDERCOLOR  Border color (in RGB) attribute.   */
   char *pFontNameSize;  /* PP_FONTNAMESIZE  Font name and size attribute. */
   int  ActiveColor;     /* Active color value of data type RGB. */
   int  InactiveColor;   /* Inactive color value of data type RGB. */
   int  ActiveTextFGNDColor; /* PP_ACTIVETEXTFGNDCOLOR Active text foreground color value of data type RGB. */
   int  ActiveTextBGNDColor; /* PP_ACTIVETEXTBGNDCOLOR Active text background color value of data type RGB. */
   int  InactiveTextFGNDColor; /* PP_INACTIVETEXTFGNDCOLOR Inactive text foreground color value of data type RGB. */
   int  InactiveTextBGNDColor; /* PP_INACTIVETEXTBGNDCOLOR Inactive text background color value of data type RGB. */
   int  ShadowColor;           /* PP_SHADOW  Changes the color used for drop shadows on certain controls. */
   int  user0;                 /* PP_USER This is a user-defined presentation parameter. */
};  

#endif
  /* FREEPM_PRESPARAM */
