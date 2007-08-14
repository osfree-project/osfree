// VDM2OS2 - Stub to start 4OS2 & TCMD/OS2 from DOS VDM
// Copyright (c) 1996 Rex C. Conn

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

typedef struct _STARTDATA {
        unsigned int uLength;
        unsigned int uRelated;
        unsigned int uForeground;
        unsigned int uTrace;
        char _far *lpTitle;
        char _far *lpPgmName;
        char _far *lpArgs;
        unsigned long ulTermQ;
        char _far *lpEnvironment;
        unsigned int uInherit;
        unsigned int uSession;
        char _far *lpIcon;
        unsigned long ulPgmHandle;
        unsigned int uPgmControl;
        unsigned int uColumn;
        unsigned int uRow;
        unsigned int uWidth;
        unsigned int uHeight;
        unsigned int uReserved;
        unsigned long ulObjectBuffer;
        unsigned long ulObjectBufferLen;
} STARTDATA;

extern char _far *_LpPgmName;
extern char _far *_LpCmdLine;


int main( void )
{
        STARTDATA StartData;
        union REGS aRegs;

        if ( _osmajor < 20 ) {
                aRegs.h.ah = 0x09;
                aRegs.w.dx = (unsigned int)"This app requires OS/2.\r\n$";
                intdos( &aRegs, &aRegs );
                return 2;
        }

        StartData.uLength = sizeof( STARTDATA );

        StartData.uRelated = 0;
        StartData.uForeground = 0;
        StartData.uTrace = 0;

        // set title, name, & args
        StartData.lpTitle = 0L;
        StartData.lpPgmName = _LpPgmName;
        StartData.lpArgs = _LpCmdLine;

        StartData.ulTermQ = 0L;
        StartData.lpEnvironment = 0L;
        StartData.uInherit = 0;
        StartData.uSession = 0;
        StartData.lpIcon = 0;
        StartData.ulPgmHandle = 0L;
        StartData.uPgmControl = 0;
        StartData.uColumn = StartData.uRow = 0;
        StartData.uWidth = StartData.uHeight = StartData.uReserved = 0;
        StartData.ulObjectBuffer = 0L;
        StartData.ulObjectBufferLen = 0L;

        aRegs.h.ah = 0x64;
        aRegs.w.bx = 0x25;
        aRegs.w.cx = 0x636C;
        aRegs.w.si = (unsigned int)&StartData;
        intdos( &aRegs, &aRegs );

        return 0;
}

