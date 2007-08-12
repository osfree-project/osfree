/* ldir.h
 * Werner Zimmermann, FHTE Esslingen, Germany
 *
   Copyright information and copying policy see file README.TXT

   History see file MAIN.C

 * This file contains user settings and global defines
 */

/*------------------------------------------------------------------------*/
/*User Options and Settings */

#ifndef UNIX
  #define LINUX_DISK    "/dev/hda"				/* Name of your Linux partition
								   Specify partition number, eg. /dev/hda6
								   if you do not want ldir/lread to use
								   the first LINUX partition on your drive.
								   May also be a floppy like /dev/fd0.
								   Can be overwritten by -s/dev/...
								   command line switch */
#else
  #define LINUX_DISK    "/dev/hda"
#endif

/*-------------------------------------------------------------------------*/
/*Debugging switches, normally no need to change */

/*
   #define DEBUG_MAIN		1				//debug level 	1
   #define DEBUG_READDISK	2				//		2
   #define DEBUG_SUPER		4				//		...
   #define DEBUG_DIR		8				//(these values are logically or'ed)
   #define DEBUG_GROUP		16
   #define DEBUG_INODE		32
   #define DEBUG_RFS		64
   #define DEBUG_ALLOC
*/

/*------------------------------------------------------------------------*/
/*Global Defines, but nothing to edit for users */
#define READ_CMD  2						/*Disk Read  Command, do *NOT* change!!! */
#define PARA_CMD  8                                             /*Disk parameter Command, do *NOT* change!!! */
#define WRITE_CMD 3						/*Disk Write Command, do *NOT* change!!! */
#define EXT_CHK   0x41						/*Bios extension installation check*/
#define EXT_PARA  0x48						/*Extended disk parameter command, do *NOT* change!!! */
#define EXT_READ  0x42						/*Extended disk read  command, do *NOT* change!!! */
#define EXT_WRITE 0x43						/*Extended disk write command, do *NOT* change!!! */

#define NONE      0						/*used in readdisk.c during linux partition search */
#define LINUX     1
#define EXTENDED  2
#define END_TABLE 3

#define INIT         0
#define KEEP_BUF     1						/*used to control free(buf) in read_inode() */
#define RELEASE_BUF  2
#define KILL_BUF     3

#define NORM 0
#define SELF 1

#define DISK_BLOCK_SIZE  512UL					/*size of harddisk sectors in byte */
#define BUFSIZE          32768                         		/*size of read/write buffer */

#define EXT2PART    0x83					/*signature of Extended2 Linux partitions */
#define EXT2PARTNEW 0x8E					/*LVM partitions use this*/

#ifdef __BORLANDC__
  #define TIMEZONE _timezone
#else
  #define TIMEZONE timezone
#endif

#define MAX_PATH_LENGTH  512					/*length of pathnames*/

//Definitions stolen from linux/hdreg.h
#ifdef UNIX
/*
struct hd_geometry {
      unsigned char heads;
      unsigned char sectors;
      unsigned short cylinders;
      unsigned long start;
};

#define HDIO_GETGEO		0x0301				// get device geometry
*/
#endif
