/*  Drive letter assignment
 *  algorithms
 *
 */

#include <shared.h>

#include <stdarg.h>
#include <serial.h>

#include <lvm_data.h>

extern unsigned short serial_hw_port;
void comwait(unsigned short port);

int vsprintf(char *buf, const char *fmt, va_list args);
int toupper (int c);
int grub_strcmp (const char *s1, const char *s2);
int grub_memcmp (const char *s1, const char *s2, int n);
int kprintf(const char *format, ...);
int rawread (int drive, int sector, int byte_offset, int byte_len, char *buf);
int get_disk_type(int driveno, int *status);

#define PART_TABLE_OFFSET 0x1be

/* multiboot structure pointer */
extern struct multiboot_info *m;
extern unsigned long boot_drive;
extern char debug;

extern grub_error_t errnum;

#pragma aux m            "*"
#pragma aux printmsg     "*"
#pragma aux boot_drive   "*"

extern char buf[];

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
  int primaries = 0;

  memset(buf, 0, sizeof(buf));

  // read the Partition table
  rawread(diskno, 0, PART_TABLE_OFFSET, 0x40, buf);

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
        primaries++;
        if (primaries == 1) parts++;
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
  int diskno, diskno_sav;
  int num_letters = 0; // -1;
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
  kprintf("boot_drive=0x%x\n", boot_drive);

  if (m->flags & MB_INFO_DRIVE_INFO)
  {
    // the 1st drive structure
    drv = (struct drive_info *)m->drives_addr;
    len = m->drives_length;

    for (i = 0; len > 0; len -= drv->size, i++,
         drv = (struct drive_info *)((char *)drv + drv->size))
    {
      diskno = drv->drive_number;
      kprintf("diskno=%u\n", diskno);
      if (diskno == boot_drive) continue;
      if ((diskno & 0x7f) + 1 > fixed_disks) continue;
      num_letters += get_num_parts(diskno);
    }
  }

  return ('C' + num_letters);
}

/*
  Name  : CRC-32
  Poly  : 0x04C11DB7    x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11
                       + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
  Init  : 0xFFFFFFFF
  Revert: true
  XorOut: 0xFFFFFFFF
  Check : 0xCBF43926 ("123456789")
  MaxLen: 268 435 455 байт (2 147 483 647 бит) - обнаружение
   одинарных, двойных, пакетных и всех нечетных ошибок

  (Got from russian wikipedia, http://ru.wikipedia.org/wiki/CRC32)
  and corrected ;)
*/
unsigned long crc32(unsigned char *buf, unsigned long len)
{
    unsigned long crc_table[256];
    unsigned long crc;
    int i, j;

    for (i = 0; i < 256; i++)
    {
        crc = i;
        for (j = 0; j < 8; j++)
            crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

        crc_table[i] = crc;
    };

    crc = 0xFFFFFFFFUL;

    while (len--)
      crc = crc_table[(crc ^ *buf++) & 0xff] ^ ((crc >> 8) & 0x00ffffffL);

    return crc;
}

int
offset (int part_no, int flag)
{
  char buf[0x40];
  unsigned long sec;
  char *pte, *ext;
  int i;
  int parts = 1;
  unsigned long ext_start = 0;
  unsigned long part_start = 0;

  memset(buf, 0, sizeof(buf));
  rawread(boot_drive, 0, PART_TABLE_OFFSET, 0x40, buf);
  kprintf("part_no=%u\n", part_no);

  ext = 0;
  pte = buf;

  if (part_no < 4) // primary partition
  {
    pte += 0x10 * part_no;
    part_start  = (pte[11] << 24) | (pte[10] << 16) | (pte[9] << 8) | pte[8];
  }
  else // logical partition
  {
    parts = 4;
    ext = 0;
    for (i = 0; i < 4; i++, pte += 0x10)
    {
      if (pte[4] == 0x5 || pte[4] == 0xf)
      {
        ext = pte;
        break;
      }
    }
    pte = ext;
    while (pte) // while partition does exist
    {
      part_start = (pte[11] << 24) | (pte[10] << 16) | (pte[9] << 8) | pte[8];

      if (!ext_start)
        ext_start = part_start;
      else
        part_start += ext_start;

      if (parts == part_no) break;

      kprintf("part_start=%u\n", part_start);
      // read EBR
      rawread(boot_drive, part_start, PART_TABLE_OFFSET, 0x40, buf);

      pte = buf;
      ext = 0;
      for (i = 0; i < 2; i++, pte += 0x10)
      {
        if (pte[4] == 0x5 || pte[4] == 0xf)
          ext = pte;
        else // if part type is supported
          parts++;
      }
      pte = ext;
    }
  }

  if (flag)
  {
    // read the last EBR
    rawread(boot_drive, part_start, PART_TABLE_OFFSET, 0x40, buf);

    // find needed pte
    pte = buf;
    for (i = 0; i < 2; i++, pte += 0x10)
      if (pte[4] != 0x5 && pte[4] != 0xf)
        break;

    // get partition offset
    part_start += (pte[11] << 24) | (pte[10] << 16) | (pte[9] << 8) | pte[8];
  }

  return part_start;
}

/*  Determine a drive letter through DLA tables
 *  (DLA stands for Drive Letter Assignment)
 */
int dla(char *driveletter)
{
  char buff[0x220];
  unsigned long CRC32, crc;
  DLA_Table_Sector *dlat;
  DLA_Entry *dlae;
  char      *p;
  char      part_no;
  unsigned long sec;
  int       add;
  struct geometry geom;

  part_no = (m->boot_device >> 16) & 0xff;

  if (get_diskinfo(boot_drive, &geom))
    return 0;

  add = geom.sectors - 1;

  // DLAT info sector
  if (part_no < 4) // primary partition
    sec = add;
  else // logical partition
    sec = offset(part_no, 0) + add;

  p = buff;

  /* make a pointer 16-byte aligned */
  p = (char *)(((int)p >> 4) << 4);
  if (p < buff) p += 0x10;

  memset(p, 0, 0x200);

  // read DLAT sector
  rawread(boot_drive, sec, 0, 0x200, p);

  dlat = (DLA_Table_Sector *)p;

  if (dlat->DLA_Signature1 == DLA_TABLE_SIGNATURE1 &&
      dlat->DLA_Signature2 == DLA_TABLE_SIGNATURE2)
  {
    /* Calculate DLAT CRC */
    crc = dlat->DLA_CRC;
    /* zero-out CRC field and unused sector space */
    dlat->DLA_CRC = 0;
    memset(p + sizeof(DLA_Table_Sector), 0, 0x200 - sizeof(DLA_Table_Sector));
    CRC32 = crc32(p, 0x200);
    dlat->DLA_CRC = crc; // return back
    if (crc == CRC32)    // crc ok
    {
      /* Get and parse partition DLAT entry */
      dlae = (DLA_Entry *)dlat->DLA_Array;
      if (part_no < 4) dlae += part_no; // for primary partitions
      *driveletter = grub_toupper(dlae->Drive_Letter);

      return 1;
    }
  }

  return 0;
}

// assign a drive letter by DLA tables
int assign_dlat(void)
{
  char letter;

  if (!dla(&letter))
     return 'C';

  return letter;
}

// assign a drive letter by using defaults
int assign_default(void)
{
  return 'C';
}

int assign_drvletter (char *__mode)
{
  int letter = 'C';

  kprintf("mode=%s\n", __mode);

  if (!*__mode) // if mode is not set
  {
    // set defaults
    kprintf("mode: defaults\n");
    letter = assign_default();
  }
  else if (!grub_strcmp(__mode, "AUTO"))
  {
    kprintf("mode: auto\n");
    letter = assign_auto();
  }
  else if (!grub_strcmp(__mode, "DLAT"))
  {
    kprintf("mode: dlat\n");
    letter = assign_dlat();
  }
  else
  {
    kprintf("drv letter is explicitly specified\n");
    letter = toupper(__mode[0]);
  }

  kprintf("letter = %c:\r\n", letter);

  return letter;
}
