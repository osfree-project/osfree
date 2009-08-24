#ifndef __KAL__
  #define INCL_DOSPROCESS       /* Process and thread values */
  #define INCL_DOS
  #define INCL_DOSERRORS        /* DOS error values          */
  #define INCL_DOSMISC          /* DOS error values          */
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_BSESUB
  #define INCL_KBD
  #define INCL_VIO
  #include <os2.h>
#else

  #include <l4/os2/kal.h>

  #define DosError kal_DosError
  #define DosSetDefaultDisk kal_DosSetDefaultDisk
  #define DosQueryCurrentDir kal_DosQueryCurrentDir
  #define DosExecPgm kal_DosExecPgm
  #define VioWrtTTY kal_VioWrtTTY
  #define DosSetDefaultDisk kal_DosSetDefaultDisk
  #define KbdStringIn kal_KbdStringIn
  #define DosQueryCurrentDisk kal_DosQueryCurrentDisk
  #define DosSetCurrentDir kal_DosSetCurrentDir
#endif

#include <string.h>

// array used for internal commands (indirect function calls)
typedef struct
{
        char *cmdname;                  // command name
        int (* func)(int, char **);     // pointer to function
} BUILTIN;

int cmds_cmd(int argc, char **argv);
int exit_cmd(int argc, char **argv);
int chdir_cmd(int argc, char **argv);

BUILTIN commands[] =
{
  "?", cmds_cmd,
  "exit", exit_cmd,
  "help", cmds_cmd,
  "cd", chdir_cmd,
  "chdir", chdir_cmd
};
