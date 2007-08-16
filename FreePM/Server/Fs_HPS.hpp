/*
 $Id: Fs_HPS.hpp,v 1.1 2003/08/14 20:48:04 evgen2 Exp $
*/
/* Fs_HPS.hpp */
/* functions for HPS management */
/* ver 0.00 22.09.2002       */

#ifndef FREEPMS_HPS
  #define FREEPMS_HPS

BOOL F_PS_GpiSetColor(struct  F_PS *ps, LONG lColor);
BOOL F_PS_GpiLine(struct  F_PS *ps, PPOINTL pptlPoint);
BOOL F_PS_GpiMove(struct  F_PS *ps, PPOINTL pptlPoint);

#endif
   /* FREEPMS_HPS */
