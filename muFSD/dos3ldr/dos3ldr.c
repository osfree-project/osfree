/*
 *   A DOS program loading a blackbox from a DOS file
 *   and executing it. Must reside on the DOS partition
 *   along with the blackbox for corresponding partition
 *   (ext2, jfs, fat or other).
 *   (c) osFree project,
 *   valerius, 2007, Jan 13
 *
 *   must be executed as:
 *
 *   dos3ldr -d <HDD BIOS number> -p <partition number> -b <blackbox file> \
 *           -l <blackbox load segment> -s <segment to create BPB at>
 *
 *
 *   The task for this DOS loader is to load a blackbox at the specified
 *   segment address, load a bootsector with BPB in it at specified address,
 *   give control to blackbox and service a disk read function to the blackbox.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <i86.h>
 #include <dos.h>
 #include <bios.h>
 #include <string.h>
 #include <fcntl.h>
 #include <io.h>

 #include <os3/types.h>
 #include <os3/bpb.h>

 #include "dos3ldr.h"

 u8     buf[512];
 u16    cyl  = 0,
        head = 0,
        sec  = 0;

 void main(int argc, char **argv, char **envp)
 {
    u8     blackbox[] = "bb_ext2";
    u8     disk        = 0x80;
    u8     part       = 0x1;
    BPB    bpb;
    u16    i;
    u8     far *s;
    u16    rc         = 0;
    u16    nsec;

    int   fd;
    u32   size;

    struct diskinfo_t di;
    struct SREGS sregs;

    segread(&sregs);

    s = MK_FP(BOOTSECT_LOAD_SEG, 0);

    memset((void *)s, 0, 0x10000);

    rc = disk_geo(disk, &cyl, &head, &sec);

    if (rc)
      printf("Error getting disk geometry: %u\n", rc);
    else {
      printf("Geometry:\n"
             "Cyl: %u, Heads: %u, Sec: %u\n",
             cyl, head, sec);
    }

    // Read MBR
    nsec = 1;
    rc = disk_read(disk, (u32)0, &nsec, s);

    if (rc)
      printf("Disk read error!\n");
    else {
      printf("%u sectors read\n", nsec);

      for (i = 0; i < 512; i++)
        putchar(s[i]);
    }

    putchar('\n');
    s = MK_FP(BLACKBOX_LOAD_SEG,  0);

    if( _dos_open(blackbox, O_RDONLY, &fd) != 0)
    {
       printf("Can't open file %s!\n", blackbox);
       exit(-1);
    }

    printf("File %s opened,\n", blackbox);
    size = filelength(fd);
    printf("it's size is %u bytes\n", size);
    _dos_read(fd, (void far *)s, size, (unsigned int *)&rc);
    printf("_dos_read() called,\n");
    printf("%u bytes read.\n", rc);

    _dos_close(fd);

    __asm {
      .286
      push BLACKBOX_LOAD_SEG
    ;  push BOOTSECT_LOAD_SEG
      push 0
      retf
    }

    //exit(0);
 }

 // Reads <= 65535 sectors at once
 u16 disk_read(u8 device,
               u32 lba,
               u16 *nsec,
               u8  far *buf)
 {
    u16 rc;
    u8  n;

    while (*nsec > 128)
    {
      n = 128;
      rc = disk_read8(device, lba, &n, buf);
      if (rc)
        return 1;
      *nsec -= 0x80;
      buf   = MK_FP(FP_SEG(buf) + (n << 5), FP_OFF(buf));
      lba   += n;
    }

    n = (u8)(*nsec);
    rc = disk_read8(device, lba, &n, buf);

    if (rc)
      return 1;

    return 0;
 }

 // Reads <= 128 sectors at once
 u16 disk_read8(u8 device,
               u32 lba,
               u8 *nsec,
               u8 far *buf)
 {
    u16 c, h, s;
    u16 rc;

    lba2chs(lba, &c, &h, &s);
    //c = 0; h = 0; s = 1;

    printf("c = %u, h = %u, s = %u, for lba = %u\n",
           c, h, s, lba);

    rc = disk_read_lba(device,
                       lba, nsec, buf);

    if (!rc)
      return 0;

    rc = 0;

    rc = disk_read_chs(device,
                       c, h, s,
                       nsec, buf);
    if (rc)
      return 1;

    return 0;
 }

 // Reads sectors by CHS
 u8  disk_read_chs(u8 device,
                   u16 cyl, u8 head, u8 sec,
                   u8 *nsec, u8 far *buf)
 {
    u8 err = 0;
    u8 read = 0;

    read = *nsec;

    __asm {
       .286
       mov  ah, 2
       mov  al, read
       mov  cx, cyl
       xchg cl, ch
       shl  cl, 6
       or   cl, sec
       mov  dh, head
       mov  dl, device
       les  bx, buf
       int  13h
       jc   __err
       mov  read, al
       jmp  __noerr
    __err:
       mov  err, 1
    __noerr:
    }

    if (err)
      return 1;

    *nsec = read;
    return 0;
 }

 // Reads sectors by LBA
 u16 disk_read_lba(u8 device,
                   u32 lba,
                   u8 *nsec,
                   u8 far *buf)
 {
    u8 err = 0;
    u16 read = 0;
    struct disk_addr_packet dap;
    struct SREGS sregs;
    void far *p;
    int  reg;

    segread(&sregs);

    p = MK_FP(sregs.ss, &dap);
    read = *nsec;

    dap.size = 0x10;
    dap.nsec = read;
    dap.buffer = (u32) buf;
    dap.lba = lba;

    __asm {
       .286
       lds  si, p
       mov  dl, device
       mov  ah, 42h
       int  13h
       jnc   __noerr
       mov  err, 1
    __noerr:
    }

    if (err)
      return 1;

    *nsec = (u8)dap.nsec;
    return 0;
 }

 // Gets disk geometry
 u16 disk_geo(u8 device, u16 *cyl, u16 *head, u16 *sec)
 {
    u8  err = 0;
    u8  type = 0;
    u16 c;
    u8  h, s;
    u16 rds = 0,
        rsi = 0;
    u8  far *p;
    u32 parm;
    u16 i;

    struct SREGS sregs;
    segread(&sregs);

    parm = (u32)MK_FP(sregs.ds, &buf);

    __asm {
       .286
       mov  ah, 48h
       mov  dl, device
       lds  si, parm
       mov  word ptr [si], 1Ah
       int  13h
       jnc   __noerr
       mov  err, 1
    __noerr:
    }

    if (!err) {
      p = (u8 far *)parm;
      *cyl  =  *((u32 far *)((u8 far *)p + 4));
      *head =  *((u32 far *)((u8 far *)p + 8));
      *sec  =  *((u32 far *)((u8 far *)p + 12));
      return 0;
    } else
        err = 0;

    __asm {
       .286
       mov  ah, 8
       mov  dl, device
       xor  di, di
       mov  es, di
       int  13h
       jc   __err
       mov  al, cl
       shl  al, 2
       shr  al, 2
       shr  cl, 6
       xchg cl, ch
       mov  c, cx
       mov  s, al
       mov  h, dh
       jmp  __noerr
    __err:
       mov  err, 1
    __noerr:
    }

    if (err)
      return 1;

    *cyl  = c;
    *head = h;
    *sec  = s;

    return 0;
 }

 void lba2chs(u32 lba, u16 *c, u16 *h, u16 *s)
 {
    u16 cylsize;
    u16 r;

    cylsize = head * sec;
    *c = lba / cylsize;
    *h = (lba - *c * cylsize) % sec;
    *s = lba - *c * cylsize - *h * sec + 1;

 }

 void give_help(void)
 {
    printf("A blackbox DOS loader,\n"
           "(c) osFree project,\n"
           "author Valery V. Sedletski\n"
           "\n"
           "dos3ldr -d <HDD BIOS number> -p <partition number> -b <blackbox file>\n"
           "        -l <blackbox load segment> -s <segment to create BPB at>     \n");
 }
