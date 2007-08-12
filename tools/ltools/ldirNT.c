// Driver program for ldirNT.dll
//
// Work in progress

#include <windows.h>
#include <stdio.h>

__declspec(dllimport) void WINAPI ldirNT(int argc, char* argv[]);

void main(int argc, char* argv[])
{   ldirNT(argc, argv);

    //Output still goes into stdout
}