#include "minicmd.h"

BUILTIN commands[] =
{
  "?", cmds_cmd,
  "exit", exit_cmd,
  "help", cmds_cmd,
  "cd", chdir_cmd,
  "chdir", chdir_cmd,
  "dir", dir_cmd
};

int cmds_cmd(int argc, char **argv)
{
  unsigned int i;

  VioWrtTTY("Commands:\r\n", 11, 0);
  for (i=0;i<6;i++)
  {
    VioWrtTTY(commands[i].cmdname, strlen(commands[i].cmdname), 0);
    VioWrtTTY("\x9", 1, 0);
  }
  VioWrtTTY("\r\n", 2, 0);
  return 0;
}

int exit_cmd(int argc, char **argv)
{
  return 0;
}


int chdir_cmd(int argc, char **argv)
{
  UCHAR   achDirName[256]     = "";          /* Directory name for queries */
  ULONG   cbDirPathLen    = 0;               /* Length of directory path   */
  APIRET  rc           = NO_ERROR;      /* Return code                     */
  UCHAR   chDisk;
  ULONG   ulDriveNum   = 0;      /* Drive number (A=1, B=2, C=3, ...)    */
  ULONG   ulDriveMap   = 0;      /* Mapping of valid drives              */
  ULONG   i            = 0;      /* A loop index                         */

  if (argc==1)
  {
  rc = DosQueryCurrentDisk (&ulDriveNum, &ulDriveMap);
  chDisk=(UCHAR)(ulDriveNum+'A'-1);

  cbDirPathLen = 0;
  rc = DosQueryCurrentDir(0, achDirName, &cbDirPathLen);
  rc = DosQueryCurrentDir(0, achDirName, &cbDirPathLen);

  VioWrtTTY(&chDisk, 1, 0);
  VioWrtTTY(":\\", 2, 0);
  VioWrtTTY(achDirName, cbDirPathLen-1, 0);
  VioWrtTTY("\n\r", 2, 0);
  } else {
    DosSetCurrentDir(argv[1]);
  };


  return 0;
}
