#include "minicmd.h"

BUILTIN cmds_desc  = {"?", cmds_cmd};
BUILTIN help_desc  = {"help", cmds_cmd};
BUILTIN exit_desc  = {"exit", exit_cmd};
BUILTIN cd_desc    = {"cd", chdir_cmd};
BUILTIN chdir_desc = {"chdir", chdir_cmd};
BUILTIN dir_desc   = {"dir", dir_cmd};
BUILTIN set_desc   = {"set", set_cmd};
BUILTIN env_desc   = {"env", env_cmd};

BUILTIN *commands[] =
{
  &cmds_desc,
  &help_desc,
  &exit_desc,
  &cd_desc,
  &chdir_desc,
  &dir_desc,
  &set_desc,
  &env_desc,
  NULL
};

int cmds_cmd(int argc, char **argv)
{
  unsigned int i;

  VioWrtTTY("Commands:\r\n", 11, 0);
  for (i = 0; commands[i]; i ++)
  {
    VioWrtTTY(commands[i]->cmdname, strlen(commands[i]->cmdname), 0);
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
