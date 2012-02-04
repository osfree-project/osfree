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
  printf ("\n%s: missing arguments\n"
          "\n"
          "Synopsis:\n"
          "screenrs <inifile> [[<cx> <cy> <bpp>] [<test>]]\n"
          "\n"
          "Example:\n"
          "> screenrs d:\\os2\\os2.ini 1024 768 24 TEST\n\n"
          "will test setting the screen resolution at 1024 x 768 pixel with a\n"
          "color depth of 24 bit per pixel\n\n", argv[0]);

  exit(4);
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
  ULONG   cx = 0, cy = 0, bpp = 0;
  int     fd;
  int     len, rd;
  int     filesize;
  int     i;

  if (argc > 1 &&
      (argv[1][0] == '-' || argv[1][0] == '/') &&
      (argv[1][1] == '?' || argv[1][1] == 'h'))
    showhelp(argc, argv);

  if (argc == 2)
  {
    inipath = argv[1];
    test = (char *)1;
  }
  else
    if (argc == 1 || argc < 5 || argc > 6)
      showhelp(argc, argv);
    else
    {
      inipath = argv[1];

      if (argc == 6)
        test = argv[5];

      cx  = atol(argv[2]);
      cy  = atol(argv[3]);
      bpp = atol(argv[4]);
    }

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

    if (app.appname > filesize)
    {
      printf("Seek beyond the filesize!\n");
      exit(5);
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

    if (!strcasecmp(appname, "PM_DISPLAYDRIVERS"))
      break;

    if (!app.nextapp)
      break;

    if (app.nextapp > filesize)
    {
      printf("Seek beyond the filesize!\n");
      exit(5);
    }

    lseek(fd, app.nextapp, SEEK_SET);
  }

  if (app.key > filesize)
  {
    printf("Seek beyond the filesize!\n");
    exit(5);
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

    if (key.keyname > filesize)
    {
      printf("Seek beyond the filesize!\n");
      exit(5);
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

    if (!strcasecmp(keyname, "DEFAULTSYSTEMRESOLUTION"))
      break;

    if (!key.nextkey)
      break;

    if (key.nextkey > filesize)
    {
      printf("Seek beyond the filesize!\n");
      exit(5);
    }

    lseek(fd, key.nextkey, SEEK_SET);
  }

  if (key.val > filesize)
  {
    printf("Seek beyond the filesize!\n");
    exit(5);
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

  printf("\n  ...that means %lux%lux%lu colors\n",
         keyval[0], keyval[1], keyval[2]);

  /* set new values */
  keyval[0] = cx;        // screen width
  keyval[1] = cy;        // screen height
  keyval[2] = 1 << bpp;  // color depth
  keyval[3] = 1;         // unknown
  keyval[4] = 0x40;      // unknown

  if (argc > 2)
  {
    printf("new data:     ");

    for (i = 0; i < len / 4; i++)
      printf("%08lx ", BYTESWAP(keyval[i]));

    printf("\n  ...that means %lux%lux%lu colors\n",
           keyval[0], keyval[1], keyval[2]);
  }

  if (!test)
  {
    // not testing
    lseek(fd, key.val, SEEK_SET);
    len = write(fd, (void *)keyval, 20);

    if (len < rd)
    {
      printf("Error writing data to file: %s!\n", inipath);
      exit(3);
    }

    printf("  ...written.\n");
  }

  close(fd);

  return 0;
}
