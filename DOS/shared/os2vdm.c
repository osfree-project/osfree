#include <os2vdm.h>
#include <dos.h>

/*
INT 21 U - OS/2 v1.x FAPI - OS/2 FILE SYSTEM JOIN/SUBST
        AH = 61h
        BP = 6467h ("dg")
        AL = function
            00h list (i.e. get)
            01h add
            02h delete
        BX = drive number
        CX = size of buffer
        SI = type (0002h JOIN, 0003h SUBST)
        ES:DI -> buffer
Return: CF clear if successful
            AX = 0000h
            ES:DI buffer filled, if appropriate
        CF set on error
            AX = error code
Notes:  used by JOIN and SUBST to communicate with the OS/2 file system
        for function 00h (list), the ES:DI buffer is filled with the ASCIZ
          JOIN/SUBST path or an empty string if the drive is not JOINed/SUBSTed
        also supported by OS/2 v2.0+ Virtual DOS Machines

For error codes refer to OS/2 <bseerr.h>
*/

APIRET APIENTRY VdmSubstSet(CHAR cDrive, PSZ pszPath)
{
  union REGPACK rp;

  rp.h.ah=0x61;
  rp.w.bp=0x6467;// ("dg")
  rp.h.al=0x01;        // add
  rp.w.bx=cDrive - 'A';
  rp.w.cx=strlen(pszPath)+1;
  rp.w.si=0x0003;
  rp.w.es=FP_SEG(pszPath);
  rp.w.di=FP_OFF(pszPath);

  intr(0x21, &rp);

  return rp.x.ax; // error
}


APIRET APIENTRY VdmSubstQuery(CHAR cDrive, USHORT usBufferSize, PVOID Buffer)
{
  union REGPACK rp;

  rp.h.ah=0x61;
  rp.w.bp=0x6467;// ("dg")
  rp.h.al=0x00; // list
  rp.w.bx=cDrive - 'A';
  rp.w.cx=usBufferSize;
  rp.w.si=0x0003;
  rp.w.es=FP_SEG(Buffer);
  rp.w.di=FP_OFF(Buffer);

  intr(0x21, &rp);

  return rp.x.ax; // error

}

APIRET APIENTRY VdmSubstDelete(CHAR cDrive)
{
  union REGPACK rp;

  rp.h.ah=0x61;
  rp.w.bp=0x6467;// ("dg")
  rp.h.al=0x02;     //delete
  rp.w.bx=cDrive - 'A';
  rp.w.cx=0x00;
  rp.w.si=0x0003;
  rp.w.es=FP_SEG(NULL);
  rp.w.di=FP_OFF(NULL);

  intr(0x21, &rp);

  return rp.x.ax; // error
}
