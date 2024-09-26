// Emulation of standard windows function via DOS ones
// for sharing of this code with WIN16 KERNEL.EXE
// List of emulated functions
//   _lopen
//   _llseek
//   _lread
//   _lclose

#include "winemu.h"


#pragma disable_message(107);
HFILE _lopen(LPCSTR lpPathName, int iReadWrite)
{
  __asm {
    push ds
    mov ax, iReadWrite
    mov dx, word ptr lpPathName
    mov ds, word ptr lpPathName+2
    mov ah, 3dh
  }
  Dos3Call;
  __asm {
    pop ds
    jnc lopenexit
    mov ax,-1
lopenexit:
  }
}
#pragma enable_message(107);

#pragma disable_message(107);
UINT _lread(HFILE  hFile, LPVOID lpBuffer, UINT uBytes)
{
  __asm {
    push ds
    mov bx, word ptr hFile
    mov dx, word ptr lpBuffer
    mov ds, word ptr lpBuffer+2
    mov cx, uBytes
    mov ah, 3fh
  }
  Dos3Call;
  __asm {
    pop ds
    jnc lreadexit
    mov ax,-1
lreadexit:
  }
}
#pragma enable_message(107);

#pragma disable_message(107);
HFILE _lclose(HFILE hFile)
{
  __asm {
    mov bx, hFile
    mov ax, 3e00h
  }
  Dos3Call;
  __asm {
    jnc lcloseexit
    mov ax,-1
lcloseexit:
  }
}
#pragma enable_message(107);

#pragma disable_message(107);
LONG _llseek( HFILE hFile, LONG lOffset, int nOrigin )
{
  __asm {
    mov bx, word ptr hFile
    mov dx, word ptr lOffset
    mov cx, word ptr lOffset+2
    mov ax, nOrigin
    mov ah, 42h
  }
  Dos3Call;
  __asm {
  }
}
#pragma enable_message(107);
