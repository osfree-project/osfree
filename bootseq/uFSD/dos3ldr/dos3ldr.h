/*
 *   A dosldr defines
 *
 *   (c) osFree project,
 *   valerius, 2007, Jan 13
 */

#define BLACKBOX_LOAD_SEG 0x8C20
//#define BOOTSECT_LOAD_SEG 0x8C00
#define BOOTSECT_LOAD_SEG 0x07C0

struct disk_addr_packet {
   u8     size;
   u8     reserved;
   u16    nsec;
   u32    buffer;
   u64    lba;
};

u8  disk_read_chs(u8 device,
                  u16 cyl, u8 head, u8 sec,
                  u8 *nsec, u8 far *buf);

u16 disk_read_lba(u8 device,
                  u32 lba,
                  u8 *nsec,
                  u8  far *buf);

u16 disk_read8(u8 device,
               u32 lba,
               u8 *nsec,
               u8 far *buf);

u16 disk_read(u8 device,
              u32 lba,
              u16 *nsec,
              u8  far *buf);

u16 disk_geo(u8 device, u16 *cyl, u16 *head, u16 *sec);

void lba2chs(u32 lba, u16 *c, u16 *h, u16 *s);
