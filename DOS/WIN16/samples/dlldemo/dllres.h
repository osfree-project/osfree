/*
 *      resource.h
 *      DLL for the Dynamic Linking Sample Application
 *
 *      exported functions:
 *      - GetResourceString()
 *      - WEP()
 */

#ifndef _resource_h_
#define _resource_h_

#include <stdio.h>
#include <string.h>
#include "windows.h"

int FAR PASCAL 
DLLres_LibMain(HINSTANCE, HINSTANCE, LPSTR, int);

int FAR PASCAL _export WEP();

void FAR PASCAL _export GetResourceString(LPSTR);

#endif

                                   
