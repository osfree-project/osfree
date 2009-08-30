/*  Drive letter assignment
 *  algorithms
 *
 */

#include <shared.h>

int toupper (int c);
int grub_strcmp (const char *s1, const char *s2);
int grub_memcmp (const char *s1, const char *s2, int n);
int kprintf(const char *format, ...);
int rawread (int drive, int sector, int byte_offset, int byte_len, char *buf);
int get_disk_type(int driveno, int *status);

#define PART_TABLE_OFFSET 0x1be

/* multiboot structure pointer */
extern struct multiboot_info *m;

#pragma aux m            "*"

// danidasd supported partition types
char default_part_types[] = {
  0x01, 0x04, 0x06, 0x07, 0x0b, 0x0c, 0x0e, 0
};

char part_types[0x20];

// get the number of drive letters for supported partitions
// on the drive with BIOS drive number 'diskno'
int get_num_parts(int diskno)
{
  char buf[0x40];
  char *pte, *p, *ext;
  int i;
  int parts = 0;
  unsigned long ext_start = 0;
  unsigned long part_start = 0;

  // read the Partition table
  rawread(diskno, 0, PART_TABLE_OFFSET, 0x40, buf);

  // dump PT
  for (i = 0; i < 0x40; i++) kprintf("0x%02x,", buf[i]);
  kprintf("\n");

  pte = buf;
  ext = 0;
  // a loop by the number of primary partitions
  for (i = 0; i < 4; i++, pte += 0x10)
  {
    if (pte[4] == 0x5 || pte[4] == 0xf)
      ext = pte;
    else // if part type is supported
      for (p = part_types; *p; p++) if (pte[4] == *p)
      {
        kprintf("found a supported partition of type: 0x%02x\n", pte[4]);
        parts++;
      }
  }

  pte = ext;
  while (pte) // while partition does exist
  {
    part_start   = (pte[11] << 24) | (pte[10] << 16) | (pte[9] << 8) | pte[8];

    if (!ext_start)
      ext_start = part_start;
    else
      part_start += ext_start;

    kprintf("part_start=%lu\n", part_start);
    // read EBR
    rawread(diskno, part_start, PART_TABLE_OFFSET, 0x40, buf);

    // dump PT
    for (i = 0; i < 0x40; i++) kprintf("0x%02x,", buf[i]);
    kprintf("\n");

    pte = buf;
    ext = 0;
    for (i = 0; i < 2; i++, pte += 0x10)
    {
      if (pte[4] == 0x5 || pte[4] == 0xf)
        ext = pte;
      else // if part type is supported
        for (p = part_types; *p; p++) if (pte[4] == *p)
        {
          kprintf("found a supported partition of type: 0x%02x\n", pte[4]);
          parts++;
        }
    }
    pte = ext;
  }

  return parts;
}

// assign a drive letter using 'AUTO' algorithm
int assign_auto(void)
{
  struct drive_info *drv;
  int n, i = 0, l;
  int len;
  int diskno;
  int num_letters = -1;
  int status;
  int rc;
  struct drive_info *drives[16];
  unsigned short *pport;
  unsigned char fixed_disks;

  // get fixed disks number from BIOS data area at 0x40:0x75
  __asm {
    push bx
    mov  bx, 0x475
    mov  bl, byte ptr ds:[bx]
    mov  fixed_disks, bl
    pop  bx
  }
  kprintf("fixed_disks: %u\n", fixed_disks);

  if (m->flags & MB_INFO_DRIVE_INFO)
  {
    // the 1st drive structure
    drv = (struct drive_info *)m->drives_addr;
    len = m->drives_length;

    for (; len > 0; len -= drv->size,
         drv = (struct drive_info *)((char *)drv + drv->size))
    {
      diskno = drv->drive_number;
      if ((diskno & 0x7f) + 1 > fixed_disks) continue;
      n = get_num_parts(diskno);
      rc = get_disk_type(diskno, &status);
      if (!rc && (status == 0x3)) num_letters += n; // if it is a harddisk
      // when booting from removable, add its drv. letter
      if (!rc && (status == 0x2) && !(diskno & 0x7f)) num_letters++;
      kprintf("drive: %x, num of partitions: %u\n", diskno, n);
      kprintf("rc = %lu, status=0x%lx\n", rc, status);
    }
  }

  return ('C' + num_letters);
}

// assign a drive letter by DLA tables
int assign_dlat(void)
{
  return 'C';
}

// assign a drive letter by using defaults
int assign_default(void)
{
  return 'C';
}

int assign_drvletter (char *mode)
{
  int letter = 'C';

  kprintf("mode=%s\n", mode);

  if (!*mode) // if mode is not set
  {
    // set defaults
    kprintf("mode: defaults\n");
    letter = assign_default();
  }
  else if (!grub_strcmp(mode, "AUTO"))
  {
    kprintf("mode: auto\n");
    letter = assign_auto();
  }
  else if (!grub_strcmp(mode, "DLAT"))
  {
    kprintf("mode: dlat\n");
    letter = assign_dlat();
  }
  else
  {
    kprintf("drv letter is explicitly specified\n");
    letter = toupper(mode[0]);
  }

  return letter;
}
