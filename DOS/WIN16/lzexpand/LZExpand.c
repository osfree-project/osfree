/*
        LZExpand.c      2.8
        Copyright 1997 Willows Software, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

        http://www.willows.com

To send email to the maintainer of the Willows Twin Libraries.

        mailto:twin@willows.com

 */

#include <windows.h>
#include <LZExpand.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>


BOOL
DeleteFile(/*LPTSTR*/char FAR * lpszName)
{
   return FALSE;
// return ( MFS_DELETE(lpszName) != MFS_FAILURE );
}

void
NewTrap(char *name)
{
        int     rc;
        char    buf[128];

        sprintf(buf, "Unimplemented API: %s\n\nPress OK to continue execution,\npress Cancel to quit application", name);
        rc = MessageBox((HWND)NULL, buf, "Trap", MB_ICONEXCLAMATION | MB_OKCANCEL);
        if ( rc == IDCANCEL )
                FatalExit(0);
}

LPVOID
WinMalloc(unsigned int size)
{
    DWORD dwSize = (size+3) & ~3;
    LPVOID lpCore = malloc(dwSize+32);
    return lpCore;
}

LPVOID
WinRealloc(LPVOID ptr, unsigned int size)
{
   LPVOID lpCore;
   DWORD dwSize = (size+3) & ~3;

   if (!ptr)
      return(WinMalloc(size));

    lpCore = realloc(ptr,dwSize);
    return lpCore;
}


static int              nLZFileTableSize;
static LPLZFILE         lpLZFileTable;

static HFILE
twin_AllocLZFileTable(void)
{
        HFILE hLZFile, h;
        int nNewLZFileTableSize;
        LPLZFILE lpNewLZFileTable;

        for (hLZFile = 0; hLZFile < nLZFileTableSize; hLZFile++)
                if (lpLZFileTable[hLZFile].bOpen == FALSE)
                        return (hLZFile);

        nNewLZFileTableSize = nLZFileTableSize + 16;
        lpNewLZFileTable = (LPLZFILE) WinRealloc((LPSTR) lpLZFileTable,
                nNewLZFileTableSize * sizeof(LZFILE));
        if (lpNewLZFileTable == NULL)
                return (LZERROR_GLOBALLOC);
        hLZFile = nLZFileTableSize;
        nLZFileTableSize = nNewLZFileTableSize;
        lpLZFileTable = lpNewLZFileTable;
        for (h = hLZFile; h < nLZFileTableSize; h++)
                lpLZFileTable[h].bOpen = FALSE;

        return (hLZFile);

}

void NewTrap(char *);

int     LZAPI
LZStart(void)
{
        NewTrap("LZStart() is obsolete");
        return (LZERROR_GLOBALLOC);
}

void    LZAPI
LZDone(void)
{
        NewTrap("LZDone() is obsolete");
}

LONG    LZAPI
CopyLZFile(HFILE hFile1, HFILE hFile2)
{
        NewTrap("CopyLZFile() is obsolete");
        return (-1);
}

LONG    LZAPI
LZCopy(HFILE hSrcLZFile, HFILE hDestFile)
{
        LONG i, n;
        char buffer[1024];

        if ((hSrcLZFile < 0)
         || (hSrcLZFile >= nLZFileTableSize)
         || (lpLZFileTable == NULL)
         || (lpLZFileTable[hSrcLZFile].bOpen == FALSE)
        ) return (LZERROR_BADINHANDLE);

        if (hDestFile < 0)
                return (LZERROR_BADOUTHANDLE);

        for (i = n = 0; (i = LZRead(hSrcLZFile, buffer, sizeof(buffer))); n += i)
        {
                if (i < 0)
                        return (LZERROR_READ);
                if (_lwrite(hDestFile, buffer, i) != i)
                        return (LZERROR_WRITE);
        }

        return (n);

}

static void Decode(HFILE infile, HFILE outfile);

HFILE   LZAPI
LZInit(HFILE hSrcFile)
{
        char header[14];
        HFILE hSrcLZFile;
        OFSTRUCT ofs;

        if ((hSrcLZFile = twin_AllocLZFileTable()) < 0)
        {
                _lclose(hSrcFile);
                return (LZERROR_GLOBALLOC);
        }

        if ((_lread(hSrcFile, header, sizeof(header)) != sizeof(header))
         || memcmp(header, "SZDD", 4))
        {
                _llseek(hSrcFile, 0L, 0);
                lpLZFileTable[hSrcLZFile].bOpen = TRUE;
                lpLZFileTable[hSrcLZFile].hFile = hSrcFile;
                lpLZFileTable[hSrcLZFile].szDeleteFile[0] = '\0';
        }
        else
        {
                lpLZFileTable[hSrcLZFile].bOpen = TRUE;
                sprintf(lpLZFileTable[hSrcLZFile].szDeleteFile,
                        "lzf%x.tmp", hSrcLZFile);
                lpLZFileTable[hSrcLZFile].hFile =
                        OpenFile(lpLZFileTable[hSrcLZFile].szDeleteFile, &ofs,
                        OF_CREATE|OF_WRITE);
                Decode(hSrcFile, lpLZFileTable[hSrcLZFile].hFile);
                _lclose(lpLZFileTable[hSrcLZFile].hFile);
                _lclose(hSrcFile);
                lpLZFileTable[hSrcLZFile].hFile =
                        OpenFile(lpLZFileTable[hSrcLZFile].szDeleteFile, &ofs,
                        OF_READ);
        }

        return (hSrcLZFile);

}

int     LZAPI
GetExpandedName(LPCSTR lpSrcFileName, LPSTR lpDestFileName)
{
        OFSTRUCT ofs;
        HFILE hSrcFile;
        char header[14];

        if ((hSrcFile = OpenFile(lpSrcFileName, &ofs, OF_READ)) == (HFILE)NULL)
                return (LZERROR_BADVALUE);

        lstrcpy(lpDestFileName, lpSrcFileName);

        if ((_lread(hSrcFile, header, sizeof(header)) == sizeof(header))
         && !memcmp(header, "SZDD", 4)
         && header[9] != '\0')
                lpDestFileName[lstrlen(lpDestFileName)-1] = header[9];

        _lclose(hSrcFile);

        return (1);

}

HFILE   LZAPI
LZOpenFile(LPCSTR lpFileName, OFSTRUCT FAR *lpReOpenBuf, UINT wStyle)
{
        HFILE hSrcFile = OpenFile(lpFileName, lpReOpenBuf, wStyle);
        if (hSrcFile == HFILE_ERROR)
                return (LZERROR_BADINHANDLE);
        return ((wStyle & ~OF_READ) != OF_READ) ? hSrcFile : LZInit(hSrcFile);
}

LONG    LZAPI
LZSeek(HFILE hLZFile, LONG lOffset, int nOrigin)
{
        if ((hLZFile < 0)
         || (hLZFile >= nLZFileTableSize)
         || (lpLZFileTable == NULL)
         || (lpLZFileTable[hLZFile].bOpen == FALSE)
        ) return (LZERROR_BADINHANDLE);

        return _llseek(lpLZFileTable[hLZFile].hFile, lOffset, nOrigin);

}

int     LZAPI
LZRead(HFILE hLZFile, void FAR *lpBuffer, int nBufSiz)
{
        if ((hLZFile < 0)
         || (hLZFile >= nLZFileTableSize)
         || (lpLZFileTable == NULL)
         || (lpLZFileTable[hLZFile].bOpen == FALSE)
        ) return (LZERROR_BADINHANDLE);

        return _lread(lpLZFileTable[hLZFile].hFile, lpBuffer, nBufSiz);

}

void    LZAPI
LZClose(HFILE hLZFile)
{
        if ((hLZFile < 0)
         || (hLZFile >= nLZFileTableSize)
         || (lpLZFileTable == NULL)
         || (lpLZFileTable[hLZFile].bOpen == FALSE)
        ) return;

        lpLZFileTable[hLZFile].bOpen = FALSE;
        _lclose(lpLZFileTable[hLZFile].hFile);
        if (lpLZFileTable[hLZFile].szDeleteFile[0])
                DeleteFile(lpLZFileTable[hLZFile].szDeleteFile);

}

/* copied from lzc.c ******************************************************* */

#define N               4096    /* size of ring buffer */
#define F               16      /* 18 upper limit for match_length */
#define THRESHOLD       2       /* minimum character length to compress */
#define NIL             N       /* index for root of binary search trees */

/*********************************************************************
   ring buffer of size N, with extra F-1 bytes to facilitate string
   comparison of longest match.  These are set by InsertNode()
**********************************************************************/
BYTE    text_buf[N + F - 1];

static int GETC(HFILE infile)
{
        unsigned char c;
        if (_lread(infile, &c, sizeof(c)) != sizeof(c))
                return (EOF);
        return (c);
}

static void PUTC(int c, HFILE outfile)
{
        unsigned char buf[1];
        buf[0] = c;
        _lwrite(outfile, buf, sizeof(buf));
}

static void Decode(HFILE infile, HFILE outfile)
{
        int  i, j, k, r, c;
        int  ii,jj;
        unsigned int  flags;
        int     phase;

        for (i = 0; i < N - F; i++) text_buf[i] = ' ';
        r = N - F;  flags = 0;

        for ( ; ; ) {
                phase = 0;
                if (((flags >>= 1) & 0x100) == 0) {
                        if ((c = GETC(infile)) == EOF)
                                break;

                        /* uses higher byte cleverly to count eight */
                        flags = c | 0xff00;
                }

                if (flags & 1) {
                        phase = 1;
                        if ((c = GETC(infile)) == EOF)
                                break;

                        PUTC(c, outfile);
                        text_buf[r++] = c;  r &= (N - 1);
                } else {
                        phase = 2;
                        if ((i = GETC(infile)) == EOF) break;
                        phase = 3;
                        if ((j = GETC(infile)) == EOF) break;

                        ii = i;
                        jj = j;

                        i |= ((j & 0xf0) << 4);  j = (j & 0x0f) + THRESHOLD;

                        for (k = 0; k <= j; k++) {
                                c = text_buf[(i + k) & (N - 1)];

                                PUTC(c, outfile);
                                text_buf[r++] = c;  r &= (N - 1);
                        }
                }
        }
}

#pragma off (unreferenced);
BOOL FAR PASCAL LibMain( HINSTANCE hInstance, WORD wDataSegment,
                         WORD wHeapSize, LPSTR lpszCmdLine )
#pragma on (unreferenced);
{
  short i;
  char buf[128];

  //BreakPoint();
//  i = GetSS();

  sprintf( buf, "DLL16 Started");
  MessageBox( NULL, buf, "LZEXPAND", MB_OK | MB_TASKMODAL );

  return( 1 );
}
