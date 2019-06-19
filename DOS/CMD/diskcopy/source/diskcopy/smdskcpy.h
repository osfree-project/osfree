#ifndef SMQLL_DISKCOPY_H_
#define SMALL_DISKCOPY_H_

int WriteFileFromMemory (char *file, char *buffer, unsigned bsize,
			 unsigned long floppysize, int askdisk,
			 int fallthrough, int overwrite);
int ReadFileIntoMemory (char *file, char *buffer, unsigned bsize);

#endif
