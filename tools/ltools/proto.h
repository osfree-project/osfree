/* proto.h

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C
 *
 * This header file contains the function prototypes
 */

/* readdisk.c */
void convert(unsigned long x, unsigned short *head, unsigned short *track, \
	     unsigned short *sector, unsigned short *offset);
int examine_drive(void);
unsigned long readdisk (unsigned char *buf, unsigned long lba, unsigned short offset, unsigned long size);
unsigned long writedisk(unsigned char *buf, unsigned long lba, unsigned short offset, unsigned long size);
#if !defined(__WIN32__) && !defined(__OS2__)
int dos_long_creat(char *filename);
int lockDrive(char mode);
#endif

/* super.c */
int write_super(void);
int load_super(void);
void print_super(void);

/* group.c */
int load_groups(void);
void store_groups(void);
void print_groups(void);

/* inode.c */
inode *load_inode(unsigned long inode_no);
int store_inode(unsigned long inode_no, inode * i);
void print_inode(inode * i, unsigned long inode_no);
unsigned long block_list(inode * i);
int read_inode(inode * i, char * buffer, unsigned long size, char ctrl);
int write_inode(inode * i, char * buffer, unsigned long size);
int modifyBlockBitmap(unsigned long block_num, int mode);
int modifyInodeBitmap(unsigned long inode_num, int mode);
void toLba(unsigned long g, unsigned long h, unsigned long *lba, unsigned short *offset);
#ifdef UNIX
void inode_fix_byteorder(inode *);
#endif

/* dir.c */
unsigned long eatpath(char *path, char *inode_name, char ctrl);
void list_dir(inode * i, char *path);
char *build_mode(inode * i);
unsigned long search_dir(inode * i, char *str, char *inode_name, char ctrl);
unsigned long modify_dir(inode * i, char *str, struct dir *newDirEntry, int mode, int *i_mode);

/* main.c */
#if !defined(__WIN32__) && !defined(__OS2__)
int lockDrive(char mode);
#endif
int DebugOut(int level, const char *fmt,...);

/* util.c */
//#ifdef UNIX
_u32 cpu_to_le32(_u32);
_u16 cpu_to_le16(_u16);
_u32 le32_to_cpu(_u32);
_u16 le16_to_cpu(_u16);
//#else
/* ot translation needed for DOS/WIN/NT */
//#define cpu_to_le32
//#define cpu_to_le16
//#define le32_to_cpu
//#define le16_to_cpu
//#endif

/* rfs.c */
int rfsMain(int argc, char* argv[]);
int rfsVersion(void);
int rfsPart(void);
