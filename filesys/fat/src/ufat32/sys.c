#define  INCL_BASE
#define  INCL_LONGLONG
#define  INCL_DOSPROCESS
#define  INCL_DOSDEVIOCTL
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <process.h>
#include <malloc.h>

#include "portable.h"
#include "fat32def.h"
#include "sys.h"

void open_drive (char *path, HFILE *hDevice);
void close_drive(HFILE hDevice);
void lock_drive(HFILE hDevice);
void unlock_drive(HFILE hDevice);
APIRET read_drive(HFILE hDevice, char *pBuf, ULONG *cbSize);
APIRET write_drive(HFILE hDevice, LONGLONG off, char *pBuf, ULONG *cbSize);
void begin_format (HFILE hDevice);
void remount_media (HFILE hDevice);

ULONG ReadSect(HFILE hf, ULONG ulSector, USHORT nSectors, PBYTE pbSector);
ULONG WriteSect(HFILE hf, ULONG ulSector, USHORT nSectors, PBYTE pbSector);

INT cdecl iShowMessage(PCDINFO pCD, USHORT usNr, USHORT usNumFields, ...);
PSZ GetOS2Error(USHORT rc);

#pragma pack(1)

struct _fat32buf
{
    // Bootsector (sector 0 = 512 bytes)
    CHAR jmp1[3];
    CHAR Oem_Id[8];
    CHAR Bpb[79];
    CHAR Boot_Code[411];
    USHORT FSD_LoadSeg;
    USHORT FSD_Entry;
    UCHAR FSD_Len;
    ULONG FSD_Addr;
    USHORT Boot_End;
    CHAR Sector1[512];
    // ldr starts from sector 2
    USHORT jmp2;
    USHORT FS_Len;
    USHORT Preldr_Len;
    UCHAR Force_Lba;
    UCHAR Bundle;
    CHAR data2[4];
    CHAR data3[30];
    UCHAR PartitionNr;
    UCHAR zero1;
    CHAR FS[16];
    CHAR Data4[8192-(60+1024)];
} fat32buf;

// preldr0 header
struct _ldr0hdr
{
    USHORT jmp1;
    USHORT FSD_size;
    USHORT LDR_size;
    UCHAR  force_lba;
    UCHAR  bundle;
    ULONG  head2;
    CHAR   head3[30];
    UCHAR  PartNr;    // not used anymore
    UCHAR  zero1;
    CHAR   FS[16];
} ldr0hdr;

#pragma pack()

#define STACKSIZE 0x10000

void _System sysinstx_thread(ULONG args)
{
  ULONG cbSize, ulAction, cbActual, cbOffActual;
  ULONG  ulDeadFace = 0xDEADFACE;
  ULONG  ulParmSize;
  char   file[20];
  char   *drive = (char *)args;
  HFILE  hf;
  APIRET rc;

  rc = DosOpenL(drive,
                &hf,
                &ulAction,                         /* action taken */
                0LL,                               /* new size     */
                0,                                 /* attributes   */
                OPEN_ACTION_OPEN_IF_EXISTS,        /* open flags   */
                OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_DASD |
                OPEN_FLAGS_WRITE_THROUGH,         /* OPEN_FLAGS_NO_CACHE , */
                0L);

  if (rc)
  {
    printf("Error %lu opening %s disk.\n", rc, drive);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  lock_drive(hf);

  rc = ReadSect(hf, 0, sizeof(fat32buf) / 512, (char *)&fat32buf);

  if (rc)
  {
    printf("Error %lu reading %s disk.\n", rc, drive);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  // copy bootsector to buffer (skipping JMP, OEM ID and BPB)
  memcpy(&fat32buf.Boot_Code, (char *)bootsec + 11 + 79, sizeof(bootsec) - 11 - 79);
  // copy OEM ID
  strncpy(&fat32buf.Oem_Id, "[osFree]", 8);
  // FSD load segment
  fat32buf.FSD_LoadSeg = 0x0800;
  // FSD entry point
  fat32buf.FSD_Entry = 0;
  // FSD length in sectors
  fat32buf.FSD_Len = (8192 - 1024) / 512;
  // FSD offset in sectors
  fat32buf.FSD_Addr = 2; 
  // copy the mini pre-loader
  memcpy((char *)&fat32buf.jmp2, preldr_mini, sizeof(preldr_mini));
  // copy FSD
  memcpy((char *)&fat32buf.jmp2 + sizeof(preldr_mini), fat_mdl, sizeof(fat_mdl));
  // FSD length
  fat32buf.FS_Len = sizeof(fat_mdl);
  // pre-loader length
  fat32buf.Preldr_Len = sizeof(preldr_mini);
  // FSD and pre-loaded are bundled
  fat32buf.Bundle = 0x80;
  // partition number (not used ATM)
  fat32buf.PartitionNr = 0;
  // FS name
  strncpy(&fat32buf.FS, "fat", 3);
  fat32buf.FS[3] = 0;

  ulParmSize = sizeof(ulDeadFace);
  rc = DosFSCtl(NULL, 0, 0,
                (PBYTE)&ulDeadFace, ulParmSize, &ulParmSize,
                FAT32_SECTORIO,
                NULL,
                hf,
                FSCTL_HANDLE);

  if (rc)
  {
    printf("Error %lu doing FAT32_SECTORIO to %s disk.\n", rc, drive);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  rc = WriteSect(hf, 0, sizeof(fat32buf) / 512, (char *)&fat32buf);

  if (rc)
  {
    printf("Error %lu writing to %s disk.\n", rc, drive);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  unlock_drive(hf);
  close_drive(hf);

  // create subdirs
  memset(file, 0, sizeof(file));
  file[0] = drive[0];
  strcat(file, ":\\boot");
  DosCreateDir(file, NULL);
  strcat(file, "\\loader");
  DosCreateDir(file, NULL);
  strcat(file, "\\fsd");
  DosCreateDir(file, NULL);

  memset(file, 0, sizeof(file));
  file[0] = drive[0];
  strcat(file, ":\\boot\\loader\\preldr0.mdl");

  rc = DosOpen(file,
               &hf,
               &ulAction,                         /* action taken */
               0L,                                /* new size     */
               0,                                 /* attributes   */
               OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS, /* open flags   */
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_WRITE_THROUGH,
               0L);

  if (rc)
  {
    printf("Error %lu creating %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  rc = DosWrite(hf, preldr0_mdl, sizeof(preldr0_mdl), &cbActual);

  if (rc)
  {
    printf("Error %lu writing to %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  DosSetFilePtr(hf, 0, FILE_BEGIN, &cbOffActual);

  rc = DosRead(hf, &ldr0hdr, sizeof(ldr0hdr), &cbActual);

  if (rc)
  {
    printf("Error %lu reading from %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  ldr0hdr.FSD_size = 0;
  ldr0hdr.LDR_size = sizeof(preldr0_mdl);
  strcpy(&ldr0hdr.FS, "fat");

  DosSetFilePtr(hf, 0, FILE_BEGIN, &cbOffActual);

  rc = DosWrite(hf, &ldr0hdr, sizeof(ldr0hdr), &cbActual);

  if (rc)
  {
    printf("Error %lu writing to %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  DosClose(hf);

  strncpy(file + 22, ".rel", 4);

  rc = DosOpen(file,
               &hf,
               &ulAction,                         /* action taken */
               0L,                                /* new size     */
               0,                                 /* attributes   */
               OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS, /* open flags   */
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_WRITE_THROUGH,
               0L);

  if (rc)
  {
    printf("Error %lu creating %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  rc = DosWrite(hf, preldr0_rel, sizeof(preldr0_rel), &cbSize);

  if (rc)
  {
    printf("Error %lu writing to %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  DosClose(hf);

  memset(file, 0, sizeof(file));
  file[0] = drive[0];
  strcat(file, ":\\boot\\loader\\fsd\\fat.mdl");

  rc = DosOpen(file,
               &hf,
               &ulAction,                         /* action taken */
               0L,                                /* new size     */
               0,                                 /* attributes   */
               OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS, /* open flags   */
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_WRITE_THROUGH,
               0L);

  if (rc)
  {
    printf("Error %lu creating %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  rc = DosWrite(hf, fat_mdl, sizeof(fat_mdl), &cbActual);

  if (rc)
  {
    printf("Error %lu writing to %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  DosClose(hf);

  strncpy(file + 22, ".rel", 4);

  rc = DosOpen(file,
               &hf,
               &ulAction,                         /* action taken */
               0L,                                /* new size     */
               0,                                 /* attributes   */
               OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS, /* open flags   */
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_WRITE_THROUGH,
               0L);

  if (rc)
  {
    printf("Error %lu creating %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  rc = DosWrite(hf, fat_rel, sizeof(fat_rel), &cbSize);

  if (rc)
  {
    printf("Error %lu writing to %s file.\n", rc, file);
    printf("%s\n", GetOS2Error(rc));
    return;
  }

  DosClose(hf);

  // The system files have been transferred.
  iShowMessage(NULL, 1272, 0);

  return;
}

int sys(int argc, char *argv[], char *envp[])
{
  char *stack;
  APIRET rc;

  // Here we're switching stack, because the original
  // sysinstx.com stack is too small.

  // allocate stack
  rc = DosAllocMem((void **)&stack, 
                   STACKSIZE, 
                   PAG_READ | PAG_WRITE | 
                   PAG_COMMIT | OBJ_TILE);

  if (rc)
    return rc;

  // call sysinstx_thread on new stack
  _asm {
    mov eax, esp
    mov edx, stack
    mov ecx, argv
    add edx, STACKSIZE - 4
    mov esp, edx
    push eax
    mov ecx, [ecx + 4]
    push ecx
    call sysinstx_thread
    add esp, 4
    pop esp
  }

  // deallocate new stack
  DosFreeMem(stack);

  return 0;
}
