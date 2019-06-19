/*
 *      dllres.c
 *      DLL for the Dynamic Linking Sample Application
 *
 *      exported functions:
 *      - GetResourceString()
 *      - WEP()
 */

#include "dllres.h"

static HINSTANCE hInstance;

int FAR PASCAL
DLLres_LibMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{                   
        hInstance = hInst;
        return 1;
}

void FAR PASCAL _export
GetResourceString(LPSTR theString)
{
        LoadString(hInstance, 1, theString, 100);
}

int FAR PASCAL _export WEP()
{
  return 1;
}

