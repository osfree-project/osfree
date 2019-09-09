unsigned char * ensureDMA(unsigned int howbig);

void drivelettercheck(short *drive); /* translate A to 0 etc. */

void dosreaddriveinfo(unsigned short driveno, struct mydiskinfo *diskinfo);
void biosreaddriveinfo(unsigned short driveno, struct mydiskinfo *diskinfo);

void promptfordisk(unsigned short driveno);

void doschecksumdrive(unsigned short driveno, md5_byte_t *digest);
void bioschecksumdrive(unsigned short driveno, md5_byte_t *digest);

/*
  This holds data to describe a disk for comparison.
  Add fields as needed. DOS mode only uses nsecs, BIOS
  mode only uses the other fields.
*/

struct mydiskinfo {
  int   nsecs;  /* number of sectors */
  int   heads;  /* number of heads (sides) */
  int   secpertrack; /* sectors per track */
  int   tracks; /* number of tracks */
};


