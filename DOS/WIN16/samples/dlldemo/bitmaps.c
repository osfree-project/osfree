/*
 *      bitmaps.c
 *      DLL for the Dynamic Linking Sample Application
 *      
 *      exported functions:
 *
 *      - GetLibraryBitmap
 *      - WEP
 *      
 */

#include "bitmaps.h"

HINSTANCE hInstance;
static HBITMAP  hBitmap;

int FAR PASCAL
Bitmaps_LibMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{   
        hInstance = hInst;
        hBitmap = LoadBitmap(hInst,"Bitmap1");
        return (1);
}

HBITMAP FAR PASCAL _export
GetLibraryBitmap()
{
        return hBitmap;
}

int FAR PASCAL _export
WEP()
{
        if(hBitmap)
                DeleteObject(hBitmap);
        hBitmap = 0;
        return (1);
}

