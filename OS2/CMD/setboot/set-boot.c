/* setboot - tell OS/2 Boot Manager what system to load at next boot */

/* 
 * Copyright 1994 Chris Smith
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and that
 * both that copyright notice and this permission notice appears in
 * supporting documentation.  I make no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

#ifndef __linux__
#define __dos__ 1
#endif

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if __linux__
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#if __dos__
#include <bios.h>
#include <dos.h>
#endif

#include "bootmgr.h"

#if __linux__

/* On Linux, i/o is to /dev/hda and friends, a drive handle is an fd,
   and disk addresses are sector numbers, 0-origin. */

typedef unsigned drive;
typedef unsigned disk_addr;

disk_addr disk_addr_0 = 0;
disk_addr bm_0, bm_1, bm_3;

#endif

#if __dos__

/* On DOS, i/o is via bios calls, a drive handle is a bios drive number
   (80h, 81h, etc), and disk addresses are pointers to a cyl/head/sec
   struct in int13 format. */

typedef unsigned char drive;
typedef struct int13 *disk_addr;

struct int13 sector_1 = { 0, 0, 1 };
struct int13 bm_sector_0, bm_sector_1, bm_sector_3;

disk_addr disk_addr_0 = &sector_1;
disk_addr bm_0 = &bm_sector_0;
disk_addr bm_1 = &bm_sector_1;
disk_addr bm_3 = &bm_sector_3;

#endif

/* Partition data, one entry per partition, primary or logical.
   The boot manager menu entries are the ones marked 'bootable'.
   'Startable' is boot-manager-ese for active, it is set on
   boot manager itself. */

struct partition_info
{
  char tag[8];				/* name used in menu */
  struct int13 addr;			/* disk address in int13 format */
  unsigned long secno;			/* first sector number */
  unsigned char bootable, startable;	/* boot mgr flags */
  unsigned char system;			/* type  (1 FAT12, 5 ext, etc) */
  unsigned char devno;			/* devno 0 = 80h, 1 = 81h, ... */
  unsigned char partno;			/* partno 0-3 primary, 4 up logical */
  unsigned char pad[8];  	/* remove this when gcc 2.4.5 mul bug fixed */
};

struct partition_info info[256];
unsigned n_partitions;

/* A global to number the drives, first the /dev/hdx's then the /dev/sdx's.
   On dos, this is in step with the bios drive handle 80h, 81h, ... . */

unsigned devno;

/* A global used for error messages */

char current_dev[16];

/* The boot drive, 80h or the first /dev we find that exists */

drive bm_dev;

/* forwards */

void read_partition_info (void);
void read_partitions (drive dev);
void read_boot_manager (void);
void list_partitions (void);
void setboot (char *tag);
void setboot_1 (char *tag, char drive);
void list_menu (void);

void *get1 (drive dev, disk_addr addr);
void put1 (drive dev, disk_addr addr, void *buf);
void no_boot_manager (void);
void fatal (char *fmt, ...);

#if __linux__
drive open_drive (char *dev);
drive open_boot_drive (void);
void fatal_perror (char *fmt, ...);
char boot_dev[16];
#endif

#if __dos__
drive open_drive (int n);
int strncasecmp (char *a, char *b, unsigned n);
void reboot (void);
#endif


/* main. */

int main (int argc, char *argv[])
{
  char arg[256];
  int n;
  int bflag = 0;
#ifndef NDEBUG
  int lflag = 0;
#endif

  /* Concat args into one string, so e.g. { "OS/2", "2.1" } is "OS/2 2.1" */

  *arg = 0;
  for (n = 1; n < argc; n++) {
    char *argn = *++argv;
    if (! strcmp (argn, "/b") || ! strcmp (argn, "/B"))
      bflag = 1;
#ifndef NDEBUG
    else if (! strcmp (argn, "-l"))
      lflag = 1;
#endif
    else {
      if (*arg)
	strcat (arg, " ");
      strcat (arg, argn);
    }
  }

  /* Read all partition tags, so we can check if the arg is among them */
  read_partition_info ();
  read_boot_manager ();

#ifndef NDEBUG
  /* Dump the table if requested */
  if (lflag)
    list_partitions ();
#endif

  /* Do it. */
  setboot (arg);

#if __dos__
  /* Reboot. */
  fflush (stdout);
  if (bflag)
    reboot ();
#endif

  return 0;
}

/* Fetch partition tables from all disks into info[] */

void read_partition_info ()
{
  int n;
  drive d;

#if __linux__
  char dev[16];

  for (n = 0; ; n++) {
    sprintf (dev, "/dev/hd%c", n + 'a');
    if (d = open_drive (dev))
      read_partitions (d);
    else
      break;
  }

  for (n = 0; ; n++) {
    sprintf (dev, "/dev/sd%c", n + 'a');
    if (d = open_drive (dev))
      read_partitions (d);
    else
      break;
  }
#endif

#if __dos__
  for (n = 0x80; ; n++) {
    if (d = open_drive (n))
      read_partitions (d);
    else
      break;
  }
#endif
}

/* Fetch partition tables from one disk into info[] */

void read_partitions (drive dev)
{
  struct partition_boot_sector *p;
  struct partition_info *ext, *log;
  int n;
  int logical_partno;
  unsigned char t;

  assert (sizeof *p == 512);
  ext = 0;

  /* Get master boot record, with primary partition table */

  p = get1 (dev, disk_addr_0);
  if (p->magic != 0xaa55)
    fatal ("MBR of %s has bad magic number", current_dev);

  /* Stash info.
     Partition type 05 heads a linked list of logical partitions. */

  for (n = 0; n < 4; n++) {
    if (t = p->partition[n].end.indicator) {
      struct partition_info *i = &info[n_partitions];
      i->system = t;
      i->devno = devno;
      i->partno = n;
      i->addr = p->partition[n].start;
      i->secno = p->partition[n].sector_number;
      i->startable = p->partition[n].start.indicator & 0x80;
      if (t == 5)
	ext = i;
      n_partitions++;
    }
  }

  /* If we saw a type-5 partition, it points to the first logical
     partition.  The first sector of that partition is its partition
     boot record, which may have a type-5 entry pointing to the next
     logical partition.  The sector numbers in logical partition boot
     records are relative to the start of the extended partition, the
     type-5 entry in the primary partition table. */

    for (log = ext, logical_partno = 4; log != 0; logical_partno++) {
      
#if __linux__
      if (log == ext)
	p = get1 (dev, ext->secno);
      else
	p = get1 (dev, ext->secno + log->secno);
#endif

#if __dos__
      p = get1 (dev, &log->addr);
#endif

      if (p->magic != 0xaa55)
	fatal ("%s partition boot record %d has bad magic number",
	       current_dev, logical_partno);

      log = 0;
      for (n = 0; n < 4; n++) {
	if (t = p->partition[n].end.indicator) {
	  struct partition_info *i = &info[n_partitions];
	  i->system = t;
	  i->devno = devno;
	  i->partno = logical_partno;
	  i->addr = p->partition[n].start;
	  i->secno = p->partition[n].sector_number;
	  i->startable = 0;
	  i->bootable = p->bootable;
	  memcpy (i->tag, p->tag, sizeof i->tag);
	  if (t == 5)
	    log = i;
	  else
	    n_partitions++;
	}
      }
    }

  /* done with this disk, increment devno for next disk */

  devno++;
}

/* Locate boot manager, and fetch its menu info for the primary partitions.
   Boot manager is in the partition that will get booted -- active primary
   partition on the first disk. */

void read_boot_manager ()
{
  int n;
  struct boot_block *bootblock;
  struct boot_manager_menu *menublock;

  assert (sizeof *bootblock == 512);
  assert (sizeof *menublock == 512);  

  /* Find the active primary partition on the first disk.
     Check that it has system indicator 0A, OPUS. ( <- joke) */

  for (n = 0; n < n_partitions; n++)
    if (info[n].startable)
      break;

  if (n == n_partitions || info[n].devno != 0
      || info[n].partno > 3 || info[n].system != 10)
    no_boot_manager ();

  /* Record the addresses of boot manager sectors 0, 1, 3. */

#if __linux__
  bm_dev = open_boot_drive ();
  bm_0 = info[n].secno;
  bm_1 = bm_0 + 1;
  bm_3 = bm_0 + 3;
#endif

#if __dos__
  bm_dev = 0x80;
  *bm_0 = *bm_1 = *bm_3 = info[n].addr;
  bm_1->cyln_secn += 1;
  bm_3->cyln_secn += 3;
#endif

  /* Fetch boot mgr's boot sector and see if it has magic oem-id */

  bootblock = get1 (bm_dev, bm_0);

  if (bootblock->magic != 0xaa55
      || memcmp (bootblock->oem_id, "APJ&WN", 5))
    no_boot_manager ();

  /* Sector 3 of boot manager has the menu info for primary partitions
     on all disks */

  menublock = get1 (bm_dev, bm_3);

  for (n = 0; n < n_partitions; n++) {
    if (info[n].partno < 4) {
      struct partition_data *p =
	&menublock->partition_data[info[n].devno * 4 + info[n].partno];
      info[n].bootable = p->bootable;
      memcpy (info[n].tag, p->tag, sizeof p->tag);
    }
  }

}

#ifndef NDEBUG
/* Dump the assembled partition data */

void list_partitions ()
{
  int n;

  for (n = 0; n < n_partitions; n++) {
    struct partition_info *p = &info[n];
    printf ("%d %d %8.8s %02x %s %s %08lx %02x %04x\n",
	    p->devno, p->partno,
	    p->tag,
	    p->system,
	    p->bootable ? "bootable" : "        ",
	    p->startable ? "startable" : "         ",
	    p->secno,
	    p->addr.headn,
	    p->addr.cyln_secn);
  }

  exit (1);
}
#endif

/* Compare the user's menu selection with the boot manager menu and see if
   it's there.  All this hassle is because giving an invalid name causes
   boot manager to emit the paralyzing message

       Operating system missing, system halted

   so do a validity check.  Since we're at it, allow unique abbrevs and
   case insensitivity */

void setboot (char *tag)
{
  int n;
  char *iba;
  unsigned ibd;
  int taglen = strlen(tag);
  char extag[8];

  /* Pad tag with trailing spaces for exact-match check */

  if (taglen > 8) taglen = 8;
  memcpy (extag, "        ", 8);
  memcpy (extag, tag, taglen);

  /* Just 'setboot' lists the menu */

  if (taglen == 0)
    list_menu ();

  /* setboot X: stores that letter to be booted from.
     I don't know how to validity-check this one */

  else if (taglen == 2 && tag[1] == ':') {
    ibd = tag[0];
    if (ibd - 'a' < 26) ibd -= 040;
    if (ibd - 'A' < 26)
      setboot_1 (0, ibd);
    else
      list_menu ();
  }

  /* setboot TAG selects that tag from the menu */
  
  else {
    iba = 0;
    for (n = 0; n < n_partitions; n++) {
      struct partition_info *p = &info[n];
      if (p->bootable)
	if (! memcmp (extag, p->tag, 8)) {
	  iba = p->tag;
	  break;
	} else if (! strncasecmp (tag, p->tag, taglen)) {
	  if (iba) {
	    fprintf (stderr, "Ambiguous.\n");
	    list_menu ();
	    return;
	  }
	  iba = p->tag;
	}
    }

    /* if no match, give help, otherwise store the full tag for next boot */
    if (! iba)
      list_menu ();
    else
      setboot_1 (iba, 0);
  }
}

/* Actually write the chosen tag or drive letter into sector 1 of boot
   manager's partition.  */

void setboot_1 (char *tag, char drive)
{
  struct boot_manager_transient *p;
  assert (sizeof *p == 512);

  /* get sector 1 */

  p = get1 (bm_dev, bm_1);

  /* insert /IBD:X or /IBA:TAG into its place, and clear the other one */

  if (tag) {
    p->boot_device = 0;
    memcpy (p->boot_tag, tag, 8);
  } else {
    p->boot_device = drive;
    memset (p->boot_tag, 0, 8);
  }
    
  /* write sector 1 back */

  put1 (bm_dev, bm_1, p);

  /* tell what we did */

  if (tag)
    printf ("Next boot loads %.8s\n", tag);
  else
    printf ("Next boot from drive %c:\n", drive);
}

/* print the boot manager tags of the partitions marked Bootable */

void list_menu ()
{
  int n;

  printf ("Bootable partitions:\n");
  for (n = 0; n < n_partitions; n++) {
    struct partition_info *p = &info[n];
    if (p->bootable)
      printf ("    %.8s\n", p->tag);
  }

  exit (1);
}

/* explain regretfully */

void no_boot_manager ()
{
  fatal ("Can't find boot manager");
}

/* fall over dead */

void fatal (char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\n");
  va_end (ap);
  exit (1);
}

#if __linux__

drive open_drive (char *dev)
{
  int fd;
  char buf[512];

  fd = open (dev, 0);
  if (fd < 0) {
    if (errno == ENODEV || errno == ENOENT)
      return 0;
    fatal_perror ("Can't read %s", dev);
  }

  /* read the first block to make sure the dev really is there.
     This is due to a quirk: if there is (e.g.) no second IDE disk
     but /dev/hdb is present, it can be opened but is size 0. */
  switch (read (fd, buf, 512)) {
  case 0:
    return 0;
  case 512:
    break;
  default:
    fatal_perror ("Can't read %s sector 0", dev);
  }

  /* save name for error messages in get1 and put1 */
  strcpy (current_dev, dev);

  /* The first one we find is the boot drive */
  if (*boot_dev == 0)
    strcpy (boot_dev, dev);

  return fd;
}

drive open_boot_drive ()
{
  int fd;

  if (*boot_dev == 0)
    fatal ("No boot drive");
  strcpy (current_dev, boot_dev);

  fd = open (boot_dev, 2);
  if (fd < 0)
    fatal_perror ("can't write %s", boot_dev);

  return fd;
}

void *get1 (drive fd, disk_addr secno)
{
  static char buf[512];
  
  if (lseek (fd, secno * 512, 0) < 0)
    fatal_perror ("lseek %s to sector %d", current_dev, secno);
  if (read (fd, buf, 512) != 512)
    fatal_perror ("read %s sector %d", current_dev, secno);

  return buf;
}

void put1 (drive fd, disk_addr secno, void *buf)
{
  if (lseek (fd, secno * 512, 0) < 0)
    fatal_perror ("lseek %s to sector %d", current_dev, secno);
  if (write (fd, buf, 512) != 512)
    fatal_perror ("write %s sector %d", current_dev, secno);
}

void fatal_perror (char *fmt, ...)
{
  char buf[128];
  va_list ap;
  va_start (ap, fmt);
  vsprintf (buf, fmt, ap);
  va_end (ap);
  perror (buf);
  exit (1);
}

#endif /* __linux__ */

#if __dos__

drive open_drive (int n)
{
  union REGS regs;

  sprintf (current_dev, "%xh", n);

  regs.h.ah = 8;		/* int13 code, get drive parms */
  regs.h.dl = n;		/* drive */

  int86 (0x13, &regs, &regs);

  /* Check against # of drives returned in DL */
  if (regs.x.cflag || (n & 0x7f) >= regs.h.dl)
    return 0;
  else
    return n;
}

void *get1 (drive dev, disk_addr addr)
{
  static char buf[512];
  union REGS regs;
  struct SREGS sregs;

  regs.h.ah = 2;		/* int13 code, read */
  regs.h.al = 1;		/* length, 1 sector */
  {				/* address (ES:BX) is DS:buf */
    void far *t = buf;
    sregs.es = FP_SEG (t);
    regs.x.bx = FP_OFF (t);
  }
  regs.x.cx = addr->cyln_secn;	/* disk address */
  regs.h.dh = addr->headn;
  regs.h.dl = dev;		/* drive */

  int86x (0x13, &regs, &regs, &sregs);

  if (regs.x.cflag)
    fatal ("bios read disk %#x error %#x", dev, regs.h.ah);

  return buf;
}

void put1 (drive dev, disk_addr addr, void *buf)
{
  union REGS regs;
  struct SREGS sregs;

  regs.h.ah = 3;		/* int13 code, write */
  regs.h.al = 1;		/* length, 1 sector */
  {				/* address (ES:BX) is DS:buf */
    void far *t = buf;
    sregs.es = FP_SEG (t);
    regs.x.bx = FP_OFF (t);
  }
  regs.x.cx = addr->cyln_secn;	/* disk address */
  regs.h.dh = addr->headn;
  regs.h.dl = dev;		/* drive */

  int86x (0x13, &regs, &regs, &sregs);

  if (regs.x.cflag)
    fatal ("bios write disk %#x error %#x", dev, regs.h.ah);
}

/* I can't for the life of me conjure a far call (let alone jump) out
   of this compiler so let's just do it in hex. */

void reboot ()
{
  char ick[] = {
    /* Disk reset flushes caches, I do hope */
    0xb4, 0x0D, 0xcd, 0x21,

    /* mov [40:72],1234 for warm boot */
    0x6a, 0x40, 0x07, 0x68, 0x34, 0x12, 0x26, 0x8f, 0x06, 0x72, 0x00,

    /* jmp -1:0 to restart machine */
    0xea, 0x00, 0x00, 0xff, 0xff,
  };

  asm (ick, ick);
}

int strncasecmp (unsigned char *a, unsigned char *b, unsigned n)
{
  unsigned c1, c2;
  int t;
  while (n--) {
    c1 = *a++;
    c2 = *b++;
    if (c1 - 'a' < 26) c1 -= 040;
    if (c2 - 'a' < 26) c2 -= 040;
    if (t = c1 - c2)
      return t;
  }
  return 0;
}
#endif /* __dos__ */

/* The code above is correct but not real clear about exactly what
   is going on with sector numbers and other values in extended and
   logical partition table entries.  The code doesn't need to be 
   smarter than it is; if it does someday, here is a useful description
   of what is actually going on there.

Newsgroups: comp.os.msdos.programmer
From: wbass@netcom.com (Wally Bass)
Subject: Re: Hidden sectors in the BPB - how do these things work?
Date: Sun, 20 Mar 1994 21:57:29 GMT

In article <2mhfgo$hkn@ccu2.auckland.ac.nz>,
	Peter Gutmann <pgut1@cs.aukuni.ac.nz> wrote:
>Does anyone know how the hidden sectors field in the BPB works?
     (stuff deleted)
The following is from notes that I made awhile back. I agree with
you that the usage is illogical.

Some definitions:
  MPR:    the master partition record at cyl/head/sector 0/0/1.
  EP1:    the 'first' extended partition, defined by the MPR.
  EPn:    any 'extended partition' contained within EP1.
  EPR:    'partition record' of an EP1 or EPn, the first sector in an
          EP1 or EPn.
  SCHS:   start address entry in a partition table entry (Cyl-Hd-Sect).
  ECHS:   end address entry in a partition table entry (Cyl-Hd-Sect).
  OFFSET: entry at offset 8 in a partition entry, normally a count of
          'skipped' sectors (from somewhere) prior to the space defined
          by the entry.
  SIZE:   size entry (at offset 0Ch in a partition entry) (sectors).
  HIDDEN: the entry at offset 1Ch in the 'boot' record of a volume. The
          boot record is the record which contains the BPB. This was
          once a two byte integer, but in later DOS versions it is a
          four byte integer, still at offset 1Ch.

Conventions. The entries marked with an '*' are ones that I consider
illogical.

MPR type 01/04/06 entry (describes a 'primary partition' volume)
  SCHS:   first sector of described partition.
  ECHS:   last sector of described partition.
  OFFSET: from beginning of physical volume to SCHS.
  SIZE:   count of sectors from SCHS to ECHS, inclusive. Often, there
          is a fraction of a cluster at the end of a partition/volume
          which cannot be used, but these sectors are included in SIZE
          as it appears here (and also as it appears at offset 13h in
          the boot record for the partition).
  HIDDEN: the HIDDEN field in the boot record for a primary partition
          gives the offset from the beginning of the physical volume.

MPR type 05 entry:
  SCHS:   first sector of described partition. According to the
          'architecture,' this must be the first sector in a cylinder.
  ECHS:   last sector of described partition.
  OFFSET: from beginning of physical volume to SCHS.
  SIZE:   count of sectors from SCHS to ECHS, inclusive.

EPR type 01/04/06 entry (describes a 'logical volume')
  SCHS:   first sector of described volume.
  ECHS:   last sector of described volume.
* OFFSET: from the beginning of EPn, or, for the first 'logical volume'
          in an EP1, from the beginning of the EP1. To me, offset from
          beginning of physical volume would have been a more logical
          choice.
  SIZE:   count of sectors from SCHS to ECHS, inclusive.
* HIDDEN: the HIDDEN field in the boot record of a logical volume gives
          the offset from the beginning of containing EPn, or, for the
          first 'logical volume' in an EP1, from the beginning of the
          EP1. This definition seems pretty dumb, because it means that
          a boot record of an extended volume, by itself, doesn't
          describe it's own location (i.e., you need to also know the
          origin of the containing EP1 or EPn). This makes it harder to
          boot from a logical volume in an extended partition, because
          the boot record, after being loaded, can't easily find the
          containing partition. (The boot sector would describe its own
          location if it contained the offset from beginning of physical
          volume.)

EPR type 05 entry:
  SCHS:   first sector of the described EPn.
* ECHS:   last sector of the described EPn. Note, however, that unlike
          an EP1, an EPn is only big enough to contain one associated
          'logical volume', and has the same ending sector as the the
          logical volume that it contains, rather than an ending sector
          which matches EP1. This seems illogical, as it prevents the
          definition (and code which deals with these definitions) from
          being recursive.
* OFFSET: from beginning of EP1 to SCHS. This also inhibits recursion,
          and doesn't match either of the previous definitions of
          OFFSET.
  SIZE:   count of sectors from SCHS to ECHS, inclusive.

Note that the 'architecture' manages to have no less that *three*
different conventions for OFFSETs:
  from beginning of volume: all MPR entries,
  from beginning of EP1:    EPR type 05 entries, and
  from beginning of an EPn: EPR entries other than type 05.

In mapping a logical volume in an EP1 or EPn, DOS 5 uses the OFFSET
field and uses the cylinder value from the SCHS field, both from the EPR
type 01/04/06 entry describing the volume. In handling a request, DOS
takes a given logical sector offset in the volume, adds the OFFSET
value, then does a radix convertion to convert that to
cylinder/head/sector, and then adds the cylinder value from the SCHS
field. This procedure breaks down with the above definitions, unless
extended partitions start on cylinder boundaries.

Wally Bass

*/
