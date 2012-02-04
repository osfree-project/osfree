/*  Set/query video mode in os2.ini
 *  screenres.exe clone by valerius (_valerius (dog) mail (dot) ru)
 *  original version was written for eComStation (closed source)
 *  2012, Feb 4
 */

/* libc          */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

/* private defs  */
#include <os2ini.h>

void showhelp (int argc, char **argv)
{
  printf ("%s: missing arguments\n"
          "\n"
          "Synopsis:\n"
          "%s <IniFile> <cxScreen> <cyScreen> <ColorBits> <TEST>\n"
          "\n"
          "Example:\n"
          "%s d:\\os2\\os2.ini 1024 768 24 TEST\n"
          "will test setting the screen resolution at 1024 x 768 pixel with a\n"
          "color depth of 24 bit per pixel\n", argv[0], argv[0], argv[0]);
}

int main (int argc, char **argv)
{
  INI_hdr hdr;
  INI_app app;
  INI_key key;
  ULONG   keyval[5];
  char    appname[CCHMAXPATH];
  char    keyname[CCHMAXPATH];
  char    *inipath;
  char    *test = 0;
  ULONG   cx, cy, bpp;
  int     fd;
  int     len, rd;
  int     filesize;
  int     i;

  if (argc == 1 || argc < 5 || argc > 6)
    showhelp(argc, argv);

  inipath = argv[1];

  if (argc == 6)
    test = argv[5];

  cx  = atol(argv[2]); cy = atol(argv[3]);
  bpp = atol(argv[4]);

  //printf("Set video mode: %ux%ux%u\n", cx, cy, bpp);

  fd = open(inipath, O_RDWR | O_BINARY);

  if (fd == -1)
  {
    printf("Cannot open file: %s\n", inipath);
    exit(1);
  }

  /* Set file position to the beginning */
  lseek(fd, 0, SEEK_SET);
  rd = sizeof(INI_hdr);
  len = read(fd, (void *)&hdr, rd);

  if (len < rd)
  {
    printf("Error reading file: %s!\n", inipath);
    exit(1);
  }

  //printf("Ini fmt version: 0x%lx\n", hdr.version);
  //printf("Ini size: %lu\n", hdr.filesize);

  filesize = hdr.filesize;

  /* read applications until PM_DISPLAYDRIVERS is encountered */
  for (;;)
  {
    rd = sizeof(INI_app);
    len = read(fd, (void *)&app, rd);

    if (len < rd)
    {
      printf("Error reading file: %s!\n", inipath);
      exit(1);
    }

    /* seek to appname */
    lseek(fd, app.appname, SEEK_SET);
    rd = app.applen[0];
    len = read(fd, (void *)appname, rd);
    appname[len] = '\0';

    if (len < rd)
    {
      printf("Error reading file: %s!\n", inipath);
      exit(1);
    }

    //printf("app: %s\n", appname);

    if (!strcasecmp(appname, "PM_DISPLAYDRIVERS"))
      break;

    if (!app.nextapp)
      break;

    lseek(fd, app.nextapp, SEEK_SET);
  }

  /* PM_DISPLAYDRIVERS found, now search for DEFAULTSYSTEMRESOLUTION key */
  /* now seek to 1st key    */
  lseek(fd, app.key, SEEK_SET);

  for (;;)
  {
    rd = sizeof(INI_key);
    len = read(fd, (void *)&key, rd);

    if (len < rd)
    {
      printf("Error reading file: %s!\n", inipath);
      exit(1);
    }

    /* seek to keyname */
    lseek(fd, key.keyname, SEEK_SET);
    rd = key.keylen[0];
    len = read(fd, (void *)keyname, rd);
    keyname[len] = '\0';

    if (len < rd)
    {
      printf("Error reading file: %s!\n", inipath);
      exit(1);
    }

    //printf("key: %s\n", keyname);

    if (!strcasecmp(keyname, "DEFAULTSYSTEMRESOLUTION"))
      break;

    if (!key.nextkey)
      break;

    lseek(fd, key.nextkey, SEEK_SET);
  }

  /* DEFAULTSYSTEMRESOLUTION key found */

  /* now seek to key value             */
  lseek(fd, key.val, SEEK_SET);
  rd = key.vallen[0];

  if (rd != 20)
  {
    printf("Key value length incorrect!\n");
    exit(2);
  }

  len = read(fd, (void *)keyval, rd);

  if (len < rd)
  {
    printf("Error reading file: %s!\n", inipath);
    exit(1);
  }

  printf("current data: ");

  for (i = 0; i < len / 4; i++)
    printf("%08lx ", BYTESWAP(keyval[i]));

  printf("\n\n");

  keyval[0] = cx; keyval[1] = cy;
  keyval[2] = 1 << bpp;
  keyval[3] = keyval[4] = 0;

  printf("new data: ");

  for (i = 0; i < len / 4; i++)
    printf("%08lx ", BYTESWAP(keyval[i]));

  printf("\n");

  if (!test)
  {
    // not testing
    lseek(fd, key.val, SEEK_SET);
    len = write(fd, (void *)keyval, 20);

    //printf("len=%lu\n", len);

    if (len < rd)
    {
      printf("Error writing data to file: %s!\n", inipath);
      exit(3);
    }

    printf("written\n");
  }

  close(fd);

  return 0;
}
