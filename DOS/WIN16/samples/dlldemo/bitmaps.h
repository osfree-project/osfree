/*
 *      bitmaps.h
 *      DLL for the Dynamic Linking Sample Application
 *      
 *      exported functions:
 *
 *      - GetLibraryBitmap
 *      - WEP
 *      
 */

#ifndef _bitmaps_h_
#define _bitmaps_h_
               
#include <stdio.h>
#include <string.h>
#include "windows.h"               
               
int FAR PASCAL 
Bitmaps_LibMain(HINSTANCE, HINSTANCE, LPSTR, int);

HBITMAP FAR PASCAL _export GetLibraryBitmap();

int FAR PASCAL _export WEP();
                   
#endif
                   
