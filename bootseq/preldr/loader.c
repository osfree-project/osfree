/*
 *
 *
 */

#include <lip.h>
#include <types.h>
#include <loader.h>
#include <bpb.h>

#include <shared.h>

#include "fsys.h"
#include "fsd.h"

extern lip_t *l;

void init (void)
{
  int rc;
  char *f = "/boot/eltorito.bin";
  char *buf;

  l->lip_printmsg("Hello from loader!\r\n");
  rc = l->lip_open(f);
  l->lip_printmsg("\r\nfile ");
  l->lip_printmsg(f);
  l->lip_printmsg(" opened, rc = ");
  l->lip_printd(rc);

  buf = (char *)(EXT2BUF_BASE);

  if (rc) {
    rc = l->lip_read(buf, -1);
    l->lip_printmsg("\r\nfile read, size: ");
    l->lip_printd(rc);
  }
}
