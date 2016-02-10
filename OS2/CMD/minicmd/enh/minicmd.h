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

#include <string.h>
#include <stdlib.h>

// array used for internal commands (indirect function calls)
typedef struct
{
        char *cmdname;                  // command name
        int (* func)(int, char **);     // pointer to function
} BUILTIN;

int cmds_cmd(int argc, char **argv);
int exit_cmd(int argc, char **argv);
int chdir_cmd(int argc, char **argv);
int dir_cmd(int argc, char **argv);
int set_cmd(int argc, char **argv);
int env_cmd(int argc, char **argv);

extern BUILTIN *commands[];

void log(const char *fmt, ...);
