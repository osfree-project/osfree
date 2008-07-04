enum boolean isdriveready(unsigned short driveno);
void readdriveinfo(unsigned char driveno, struct mydiskinfo *diskinfo);
void drivecheck(short *drive);
void checksumdrive(short driveno, md5_byte_t *digest);
void biosreaddriveinfo(unsigned short driveno);
void bioscheckside(short driveno, short sideno, md5_byte_t *digest);

/*
	this is the modified version of fatinfo - getfat gives all I need
	but not the sectors count. So I changet the fatinfo declaration
	for my needs. di_nsecs is filed in readdriveinfo function (or something
	like that ;)
*/

struct mydiskinfo {
  char  fi_sclus;  /* sectors per cluster */
  char  fi_fatid;  /* the FAT id byte */
  int   fi_nclus;  /* number of clusters from fatinfo*/
  int   di_bysec;  /* bytes per sector */
  int   di_nsecs;  /* number of sectors */
};


