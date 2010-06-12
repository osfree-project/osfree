/*
 *
 */

#include <shared.h>

extern unsigned long current_drive;
extern unsigned long current_partition;
extern char at_drive[16];

#ifndef STAGE1_5

#pragma aux m     "*"
#pragma aux l     "*"
#pragma aux stop  "*"
#pragma aux errnum               "*"
#pragma aux start_kernel         "*"
#pragma aux boot_drive           "*"
//#pragma aux sane_partition       "*"

/* XX used for device completion in 'set_device' and 'print_completions' */
static int incomplete, disk_choice;
static enum
{
  PART_UNSPECIFIED = 0,
  PART_DISK,
  PART_CHOSEN,
}
part_choice;

int
sane_partition (void)
{
  /* network drive */
  if (current_drive == NETWORK_DRIVE)
    return 1;

  if (!(current_partition & 0xFF000000uL)
      && ((current_drive & 0xFFFFFF7F) < 8
          || current_drive == cdrom_drive
          || (current_drive & 0xC0) == 0xC0)
      && (current_partition & 0xFF) == 0xFF
      && ((current_partition & 0xFF00) == 0xFF00
          || (current_partition & 0xFF00) < 0x800)
      && ((current_partition >> 16) == 0xFF
          || (current_drive & 0x80)))
    return 1;

  errnum = ERR_DEV_VALUES;
  return 0;
}
#endif /* ! STAGE1_5 */

char *
set_device (char *device)
{
#ifdef STAGE1_5
    /* In Stage 1.5, the first 4 bytes of FILENAME has a device number.  */
  unsigned long dev = *((unsigned long *) device);
  int drive = (dev >> 24) & 0xFF;
  int partition = dev & 0xFFFFFF;

  /* If DRIVE is disabled, use SAVED_DRIVE instead.  */
  if (drive == GRUB_INVALID_DRIVE)
    current_drive = saved_drive;
  else
    current_drive = drive;

  /* The `partition' part must always have a valid number.  */
  current_partition = partition;

  return device + sizeof (unsigned long);

#else /* ! STAGE1_5 */

  int result = 0;
  int p;

  incomplete = 0;
  disk_choice = 1;
  part_choice = PART_UNSPECIFIED;
  current_drive = saved_drive;
  current_partition = 0xFFFFFF;

  if (*device == '(' && !*(device + 1))
    /* user has given '(' only, let disk_choice handle what disks we have */
    return device + 1;

  if (*device == '(' && *(++device))
    {
      if (*device != ',' && *device != ')')
        {
          char ch = *device;
#ifdef SUPPORT_NETBOOT
          if (*device == 'n' && !network_ready)
            bootp();

          if (*device == 'f' || *device == 'h'
              || (*device == 'n' && network_ready)
              || (*device == 'c' && cdrom_drive != GRUB_INVALID_DRIVE)
              || (*device == 'l'))
#else
          if (*device == 'f' || *device == 'h'
              || (*device == 'c' && cdrom_drive != GRUB_INVALID_DRIVE)
              || (*device == 'l'))
#endif /* SUPPORT_NETBOOT */
            {
              /* user has given '([fhn]', check for resp. add 'd' and
                 let disk_choice handle what disks we have */
              if (!*(device + 1))
                {
                  device++;
                  *device++ = 'd';
                  *device = '\0';
                  return device;
                }
              else if (*(device + 1) == 'd' && !*(device + 2))
                return device + 2;
            }

          if ((*device == 'f'
               || *device == 'h'
#ifdef SUPPORT_NETBOOT
               || (*device == 'n' && network_ready)
#endif
               || (*device == 'c' && cdrom_drive != GRUB_INVALID_DRIVE)
               || (*device == 'l'))
              && (device += 2, (*(device - 1) != 'd')))
            errnum = ERR_NUMBER_PARSING;

#ifdef SUPPORT_NETBOOT
          if (ch == 'n' && network_ready)
            current_drive = NETWORK_DRIVE;
          else
#endif /* SUPPORT_NETBOOT */
            {
              if (ch == 'c' && cdrom_drive != GRUB_INVALID_DRIVE)
                current_drive = cdrom_drive;
              else
                {
                  safe_parse_maxint (&device, (long *) &current_drive);

                  disk_choice = 0;
                  if (ch == 'h')
                    current_drive += 0x80;
                  else if (ch == 'l')
                    current_drive += 0xC0;
                }
            }
        }

      if (errnum)
        return 0;

      if (*device == ')')
        {
          part_choice = PART_CHOSEN;
          result = 1;
        }
      else if (*device == ',')
        {
          /* Either an absolute PC or BSD partition. */
          disk_choice = 0;
          part_choice ++;
          device++;

          if (*device >= '0' && *device <= '9')
            {
              part_choice ++;
              current_partition = 0;

              if (!(current_drive & 0x80)
                  || !safe_parse_maxint (&device, (long *) &current_partition)
                  || current_partition > 254)
                {
                  errnum = ERR_DEV_FORMAT;
                  return 0;
                }

              current_partition = (current_partition << 16) + 0xFFFF;

              if (*device == ',')
                device++;

              if (*device >= 'a' && *device <= 'h')
                {
                  current_partition = (((*(device++) - 'a') << 8)
                                       | (current_partition & 0xFF00FF));
                }
            }
          else if (*device >= 'a' && *device <= 'h')
            {
              part_choice ++;
              current_partition = ((*(device++) - 'a') << 8) | 0xFF00FF;
            }

          if (*device == ')')
            {
              if (part_choice == PART_DISK)
                {
                  current_partition = saved_partition;
                  part_choice ++;
                }

              result = 1;
            }
        }
    }

  if (! sane_partition ())
    return 0;

  if (result)
    return device + 1;
  else
    {
      if (!*device)
        incomplete = 1;
      errnum = ERR_DEV_FORMAT;
    }

  return 0;

#endif /* ! STAGE1_5 */
}

