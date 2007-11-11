/*
 *   A BPB structure
 *
 *   (c) osFree project
 *   valerius, 2007, Jan 13
 */

#include <types.h>


typedef _Packed struct _bios_parameters_block {
  // Standard BPB                                        //
  unsigned short sect_size;                              // Sector size in bytes
  unsigned char  clus_size;                              // Cluster size in sectors
  unsigned short res_sectors;                            // Reserved sectors
  unsigned char  n_fats;                                 // The number of FAT copies
  unsigned short root_dir_size;                          // Max. no of root directory entries
  unsigned short n_sect;                                 // No. of sectors of disk, if its size
                                                         // is less then 32M
  unsigned char  media_desc;                             // Media descriptor byte
  unsigned short fat_size;                               // FAT size in sectors
  unsigned short track_size;                             // Number of sectors in track
  unsigned short heads_cnt;                              // Number of surfaces
  unsigned long  hidden_secs;                            // Number of hidden sectors
  unsigned long  n_sect_ext;                             // Number of sectors on disk, if its
                                                         // size exceeds 32M
  // Extended BPB

  unsigned char  disk_num;                               // 00 -- floppy; 80 -- hard drive
  unsigned char  log_drive;                              // if (logDrive & 80h) = true, (logDrive & 7Fh) is
                                                         // the number of logical disk (c: is 00)
  unsigned char  marker;                                 //
  unsigned long  vol_ser_no;                             // Volume serial number
  unsigned char  vol_label[11];                          // Volume label
  unsigned char  fs_name[8];                             // File system name
                                                         //
} bios_parameters_block;

