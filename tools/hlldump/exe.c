/* lx-related functions */

#include <os2.h>

/* to make some headers happy */
#define FOR_EXEHDR          1           /* exe386.h flag */
#define DWORD               ULONG       /* Used by exe386.h / newexe.h */
#define WORD                USHORT      /* Used by exe386.h / newexe.h */

#include <newexe.h>
#include <exe386.h>
#include <stdio.h>
#include "hllfuncs.h"

#pragma pack(push, 1)
#include "hll.h"
#pragma pack(pop)

extern void *pvFile;
extern BYTE bDebugFormat; /* debug format */
extern ULONG ulDebugOff;

typedef struct exe_hdr EXEHDR;
typedef struct e32_exe EXE32HDR;

BOOL ProcessLx()
{
  EXEHDR * fileHdr = (EXEHDR *) pvFile;
  EXE32HDR * exe32Hdr;
  ULONG ulHdrOff;

  printf("+++ Processing Lx module...\n");

    if (fileHdr->e_magic == EMAGIC)
        ulHdrOff = fileHdr->e_lfanew;
    else
        ulHdrOff = 0;

    exe32Hdr = (EXE32HDR *)((unsigned)pvFile + ulHdrOff);

    if (exe32Hdr->e32_magic[0] != E32MAGIC1 || 
        exe32Hdr->e32_magic[1] != E32MAGIC2)
    {
        printf("*** Error: not valid LX module\n");
        return FALSE;
    };

    printf("    - debuginfo offset  0x%08x (%d)\n"
           "    - debuginfo length  0x%08x (%d)\n",
           exe32Hdr->e32_debuginfo, exe32Hdr->e32_debuglen);

   if ((exe32Hdr->e32_debuginfo!=0)&&(exe32Hdr->e32_debuglen))
   {
     PBYTE pbDbg = (PBYTE)pvFile + exe32Hdr->e32_debuginfo;
     ulDebugOff=exe32Hdr->e32_debuginfo;

     printf("    - debug signature: %c%c%c%c\n",
                   pbDbg[0], pbDbg[1], pbDbg[2], pbDbg[3]);

     if (pbDbg[0] == 'N' && pbDbg[1] == 'B' && pbDbg[2] == '0')
     {
       bDebugFormat=pbDbg[3];

       switch (pbDbg[3])
       {
                case DBGTYPE_32CV:
                    printf("    - debug format: 32-bit Codeview\n");
                    break;

                case DBGTYPE_AIX:
                    printf("    - debug format: AIX Debugger\n");
                    break;

                case DBGTYPE_16CV:
                    printf("    - debug format: 16-bit Codeview\n");
                    break;

                case DBGTYPE_32IBM:
                    printf("    - debug format: 32-bit OS/2 PM Debugger (HLL)\n");
                    break;

                default:
                    printf("*** Error: Invalid debug type, %d (%c)\n", pbDbg[3], pbDbg[3]);
                    return FALSE;
            }
           
     } else
     {
       printf("*** Error unknown debug signature\n");
       return FALSE;
     }; //END: if (pbDbg[0] == 'N' && pbDbg[1] == 'B' && pbDbg[2] == '0')

   } else
   {
    printf("*** Error: debuginfo offset or lenght is zero!\n");
    return FALSE;
   }; //END: if ((exe32Hdr->e32_debuginfo!=0)&&(exe32Hdr->e32_debuglen))

   printf("--- Processed\n");

  return TRUE;
};