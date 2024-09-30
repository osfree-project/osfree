// Emulation of standard windows function via DOS ones
// for sharing of this code with WIN16 KERNEL.EXE
// List of emulated functions
//   _lopen
//   _llseek
//   _lread
//   _lclose

#include "winemu.h"

HFILE WINAPI _lopen(LPCSTR lpPathName, int iReadWrite)
{
  HFILE f=fopen(lpPathName, "rb");
  return (f?f:(HFILE)-1);
}

UINT WINAPI _lread(HFILE  hFile, LPVOID lpBuffer, UINT uBytes)
{
	return fread(lpBuffer, 1, uBytes, hFile);
}

HFILE WINAPI _lclose(HFILE hFile)
{
	return (HFILE)fclose(hFile);
}

LONG WINAPI _llseek( HFILE hFile, LONG lOffset, int nOrigin )
{
	return fseek(hFile, lOffset, nOrigin);
}
