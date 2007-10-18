/*
 *
 *
 */

#include <lip.h>

lip_t *l;

/* Far pointer as a structure */
typedef _Packed struct fp {
  unsigned short off;
  unsigned short seg;
} fp_t;

extern void __cdecl real_test(void);
extern void __cdecl call_rm(fp_t);

int  stage0_mount (void)
{
  return l->lip_fs_mount();
}

int  stage0_read (char *buf, int len)
{
  return l->lip_fs_read(buf, len);
}

int  stage0_dir (char *dirname)
{
  return l->lip_fs_dir(dirname);
}

int init(void)
{
  fp_t func;

  /* Call real_test dummy function in real mode */
  func.seg = 0x3200;
  func.off = (unsigned short)(real_test);
  call_rm(func);

  return 0;
}

#pragma aux init     "_*"
#pragma aux real_test "*"
#pragma aux call_rm   "*"
