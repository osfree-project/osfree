/*
 *      BinLib.h
 *
 *      @(#)binlib2.h	1.1 5/15/96 16:37:14 /users/sccs/src/samples/bindemo2/s.binlib2.h
 *
 *      Dynamic Linking Sample Application
 *
 */

/* library-functions we will use */
BOOL FAR PASCAL cdtInit(int FAR *pdxCard, int FAR *pdyCard);
BOOL FAR PASCAL cdtDrawExt(HDC hdc, int x, int y, int carddx, int carddy, int cd, int mode, DWORD rgbBgnd);
BOOL FAR PASCAL cdtDraw(HDC hdc, int x, int y, int cd, int mode, DWORD rgbBgnd); 
void FAR PASCAL cdtTerm();
