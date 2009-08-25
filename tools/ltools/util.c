/* LTOOLS
 * Programs to read, write, delete and change Linux extended 2 filesystems under DOS
 *
 * Mainly byteorder related stuff here, used for running LREAD on big endian machines (SPARC, 68000, ...)
 * 
 * This stuff is maintained by <Richard.Zidlicky@stud.informatik.uni-erlangen.de> .


 */

#include <stdio.h>
#include "ext2.h"
#include "proto.h"
#include "ldir.h"

#ifdef __WATCOM__
#include <stdlib.h>
#endif

#ifdef UNIX
#define STDOUT stdout
#define STDERR stdout
#else
extern FILE *STDERR, *STDOUT;
#endif

int is_bigendian = 0;

void test_endianness()
{
    int err=0;
    static char *c,xx[] =
    {1, 2, 3, 4};
    unsigned long u;

    u = *(unsigned long *) xx;
    if (u == 0x01020304)
	is_bigendian = 1;
    else if (u == 0x04030201)
	is_bigendian = 0;
    else
    {
	fprintf(STDERR,"endianness test failed: much fun implementing it for PDP ;-) \n");
	exit(1);
    }
#ifdef SANITY_CHECKS
    if (sizeof(c)>4)
      {
	fprintf(STDERR,"WARNING: not tested for 64 bit systems\n");
      }
    if (sizeof(err)<4)
      {
	fprintf(STDERR,"WARNING: not tested with int<32\n");
      }

    if ( sizeof(_u8)!=1)
      {
	fprintf(STDERR,"wrong sizeof _u8, fix ext.h or your compiler\n");
	err=1;
      }
    if ( sizeof(_u16)!=2)
      {
	fprintf(STDERR,"wrong sizeof _u16, fix ext.h or your compiler\n");
	err=1;
      }
    if ( sizeof(_u32)!=4)
      {
	fprintf(STDERR,"wrong sizeof _u32, fix ext.h or your compiler\n");
	err=1;
      }

    if (err) exit(1);
#endif
}

#define __swab16(x) \
	((_u16)( \
		(((_u16)(x) & (_u16)0x00ffU) << 8) | \
		(((_u16)(x) & (_u16)0xff00U) >> 8) ))

#define __swab32(x) \
	((_u32)( \
		(((_u32)(x) & (_u32)0x000000ffUL) << 24) | \
		(((_u32)(x) & (_u32)0x0000ff00UL) <<  8) | \
		(((_u32)(x) & (_u32)0x00ff0000UL) >>  8) | \
		(((_u32)(x) & (_u32)0xff000000UL) >> 24) ))


_u32 cpu_to_le32(_u32 i)
{
    return is_bigendian ? __swab32(i) : i;
}
_u16 cpu_to_le16(_u16 i)
{
    return is_bigendian ? __swab16(i) : i;
}

_u32 le32_to_cpu(_u32 i)
{
    return is_bigendian ? __swab32(i) : i;
}
_u16 le16_to_cpu(_u16 i)
{
    return is_bigendian ? __swab16(i) : i;
}
