/*  Drive letter assignment
 *  algorithms
 *
 */

#include <shared.h>

int toupper (int c);
int grub_strcmp (const char *s1, const char *s2);
int kprintf(const char *format, ...);
int rawread (int drive, int sector, int byte_offset, int byte_len, char *buf);

#define PART_TABLE_OFFSET 0x1be

/* multiboot structure pointer */
extern struct multiboot_info *m;

#pragma aux m            "*"

// danidasd supported partition types
char default_part_types[] = {
  0x01, 0x04, 0x06, 0x07, 0x0b, 0x0c, 0x0e, 0x35, 0x83, 0
};

// get the number of drive letters for supported partitions
// on the drive with BIOS drive number 'diskno'
int get_num_part(int diskno)
{
  char buf[0x40];
  char *pte, *p;
  int i;
  int parts = 0;
  unsigned long ext_start = 0;
  unsigned long part_start = 0;
  unsigned long part_length = 0;

  // read the Partition table
  rawread(diskno, 0, PART_TABLE_OFFSET, 0x40, buf);

  // dump PT
  for (i = 0; i < 0x40; i++) kprintf("0x%02x,", buf[i]);
  kprintf("\n");

  pte = buf;
  // a loop by the number of primary partitions
  for (i = 0; i < 4; i++)
  {
    // if part type is supported
    for (p = default_part_types; *p; p++) if (pte[4] == *p)
    {
      kprintf("found a supported partition of type: 0x%02x\n", pte[4]);
      parts++;
    }

    while (pte[4] == 0x5 || pte[4] == 0xf) // extended partition
    {
      part_start   = (pte[11] << 24) | (pte[10] << 16)  | (pte[9] << 8)  | pte[8];
      part_length  = (pte[15] << 24) | (pte[14] << 16)  | (pte[13] << 8) | pte[12];

      if (!ext_start)
        ext_start = part_start;
      else
        part_start = ext_start + part_start;

      kprintf("part_start=%lu, part_length=%lu\n", part_start, part_length);
      // read EBR
      rawread(diskno, part_start, PART_TABLE_OFFSET, 0x40, buf);

      // dump PT
      for (i = 0; i < 0x40; i++) kprintf("0x%02x,", buf[i]);
      kprintf("\n");

      pte = buf;
      // if part type is supported
      for (p = default_part_types; *p; p++) if (pte[4] == *p)
      {
        kprintf("found a supported partition of type: 0x%02x\n", pte[4]);
        parts++;
      }
      pte += 0x10;
    }
    pte += 0x10; // next partition
  }

  return parts;
}

// assign a drive letter using 'AUTO' algorithm
int assign_auto(void)
{
  struct drive_info *drv;
  int n;
  int len;
  int diskno;
  int num_letters = 0;

  if (m->flags & MB_INFO_DRIVE_INFO)
  {
    // the 1st drive structure
    drv = (struct drive_info *)m->drives_addr;
    len = m->drives_length;

    do {
      diskno = drv->drive_number;
      n = get_num_part(diskno);
      if (diskno != 0x80) num_letters += n;
      kprintf("drive: %x, num of partitions: %u\n", diskno, n);
      len -= drv->size;
      drv = (struct drive_info *)((char *)drv + drv->size);
    } while (len);
  }

  return ('C' + num_letters);
}

// assign a drive letter by DLA tables
int assign_dlat(void)
{
  return 'C';
}

int assign_drvletter (char *mode)
{
  int letter = 'C';

  kprintf("mode=%s\n", mode);

  if (!grub_strcmp(mode, "AUTO"))
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
