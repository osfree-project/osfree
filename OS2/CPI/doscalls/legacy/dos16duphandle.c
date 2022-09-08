#include "kal.h"

APIRET16 APIENTRY16 DOS16DUPHANDLE(HFILE hf, HFILE * _Seg16 phf2)
{
  HFILE Hfile;
  APIRET16 rc;
  if (*phf2==0xffff)
  {
    Hfile=0xffffffff;
  } else {
    Hfile=*phf2;
  }
  rc=DosDupHandle(hf, &Hfile);
  *phf2=Hfile;
  return rc;
}
