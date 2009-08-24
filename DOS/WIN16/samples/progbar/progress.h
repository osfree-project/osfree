/********************************************************************
*    PROGRAM: Progress.h
*
*    @(#)progress.h	1.2:
*
*    Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
*
********************************************************************/

#ifndef progress__h
#define progress__h


// menu commands

// Find menu
#define IDM_SETRANGE    100
#define IDM_START       101
#define IDM_EXIT        102
#define IDM_STOP        103

// Help menu
#define IDM_ABOUT       200

// icons
#define PROGRESS_ICON     300

//dialog box defines
// set range box
#define IDE_MIN         500
#define IDE_MAX         501

#define ID_TIMER        1000

// Function prototypes

// procs
long APIENTRY MainWndProc(HWND, UINT, UINT, LONG);
BOOL APIENTRY About(HWND, UINT, UINT, LONG);
BOOL APIENTRY Range(HWND, UINT, UINT, LONG);

//functions
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);

//string constants
char *hello = "Choose Start from the Options menu to start the progress bar.";



#endif /* progress__h */
