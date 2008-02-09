#define INCL_DOSPROCESS       /* Process and thread values */
#define INCL_DOS
#define INCL_DOSERRORS        /* DOS error values          */
#define INCL_DOSFILEMGR
#define INCL_KBD
#define INCL_VIO
#include <osfree.h>

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
