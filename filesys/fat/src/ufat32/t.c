#include <stdio.h>
#include <string.h>

#include "fat32c.h"
#include "fat32def.h"
#include "portable.h"


int main(int argc, char **argv)
{
  //FSINFO     fsi = {0};
  VOLUMELABEL vl = {0};
  APIRET rc;
  int diskno;
  int l;
  char path[256];
  char vol[12];

  if (argc < 3)
  {
    printf("Incorrect args!\n");
    return 1;
  }

  strcpy(&path, argv[1]);
  strcpy(&vol,  argv[2]);

  l = strlen(vol);

  diskno = toupper(*path) - 'A' + 1;

  memset(&vl, 0, sizeof(VOLUMELABEL));
  strcpy(vl.szVolLabel, vol);
  vl.cch = strlen(vl.szVolLabel);
  
  rc = DosSetFSInfo(diskno, FSIL_VOLSER,
                    (PVOID)&vl, sizeof(VOLUMELABEL));

  if ( rc )
    printf ("Warning: failed to set the volume label, rc=%lu\n", rc);

  return 0;
}
