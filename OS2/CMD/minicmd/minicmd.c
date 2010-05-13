// This is minimal CMD.EXE. It is goal is to provide minimal testing base
// for CPI API implementation. Note only OS/2 API allowed here.

#include "minicmd.h"

// here minimal set of commands. We need to have possibility to change
// current directory, get some help, change disk, execute external commands
// and exit from the shell. And nothing more. Don't add new commands here
// expect command or feature REALLY required for minimal functionality.

// returns 1 if the specified drive is ready, 0 if it isn't
int QueryDriveReady( int drive )
{
    char curdir[255];
    unsigned int maxpath = 254;
    int rval = 254;

    // turn off VIOPOPUP error window
    (void)DosError( 2 );

    rval = ( DosQueryCurrentDir( (ULONG)drive, curdir, (PULONG)&maxpath ) == 0 );

    // turn VIOPOPUP error window back on
    (void)DosError( 1 );

    return rval;
}


void execute_external(int argc, char **argv)
{
  UCHAR       LoadError[255];
  PSZ         Args;
  PSZ         Envs;
  RESULTCODES ChildRC;
  APIRET      rc;  /* Return code */

  rc = DosExecPgm(LoadError,           /* Object name buffer           */
                  sizeof(LoadError),   /* Length of object name buffer */
                  EXEC_SYNC,           /* Asynchronous/Trace flags     */
                  argv[1], //Args,                /* Argument string              */
                  NULL, //Envs,                /* Environment string           */
                  &ChildRC,            /* Termination codes            */
                  argv[0]);                /* Program file name            */

  if (rc != NO_ERROR) {
//     printf("DosExecPgm error: return code = %u\n",rc);
     VioWrtTTY("DosExecPgm error\n\r", 18, 0); //: return code = %u\n",rc);
     return ;
  } else {
//     printf("DosExecPgm complete.  Termination Code: %u  Return Code: %u\n",
//             ChildRC.codeTerminate,
//             ChildRC.codeResult);  /* This is explicitly set by other pgm */
  } /* endif */
}

int isdelim( char c )
{
        return (( c == '\0' ) || ( c == ' ' ) || ( c == '\t' ) || ( c == ',' ));
}

BOOL parse_cmd(char * cmd)
{
  unsigned int i, rc;
  int Argc;                // argument count
  char *Argv[10];      // argument pointers
  char *pszTemp;

  if (strlen(cmd)==0)
  {
    return FALSE;
  }
  
  if (strlen(cmd)==2)
    if (cmd[1]==':')
    {
      if (QueryDriveReady(cmd[0]-'a'+1))
        DosSetDefaultDisk(cmd[0]-'a'+1);
      return FALSE;
    }

  while (( *cmd == ' ' ) || ( *cmd == '\t' ))
          cmd++;

  // loop through arguments
  for ( Argc = 0; (( Argc < 10 ) && ( *cmd )); Argc++ )  {

          pszTemp = cmd;              // save start of argument

          while ( isdelim( *cmd ) == 0 ) {
                  if ( *cmd )
                          cmd++;
          }

          // check flag for terminators
          if (( *cmd != '\0' ))
                  *cmd++ = '\0';

          // update the Argv pointer
          Argv[ Argc ] = pszTemp;

          // skip delimiters for Argv[2] onwards
          while (( *cmd ) && ( isdelim( *cmd )))
                  cmd++;
  }

  Argv[Argc] = NULL;


  if (!strcmp(Argv[0], "exit"))
  {
    return TRUE;
  }

  for (i=0;i<5;i++)
  {
    if (!strcmp(Argv[0], commands[i].cmdname))
    {
      (*(commands[i].func))(Argc, Argv);
      return FALSE;
    }
  }

  execute_external(Argc, Argv);

  return FALSE;
}

void read_cmd(char * cmd)
{
  STRINGINBUF sin;
  sin.cb=255;
  KbdStringIn(cmd, &sin, 0, 0);
  cmd[sin.cchIn]='\0';
  VioWrtTTY("\n\r", 2, 0);
}

void hello(void)
{
  VioWrtTTY("MiniCMD. (C) osFree project.\n\r\n\r", 32, 0);
}

void showpath(void)
{
  UCHAR   chDisk;
  UCHAR   achDirName[256]     = "";          /* Directory name for queries */
  ULONG   cbDirPathLen    = 0;               /* Length of directory path   */
  APIRET  rc           = NO_ERROR;      /* Return code                     */
  ULONG   ulDriveNum   = 0;      /* Drive number (A=1, B=2, C=3, ...)    */
  ULONG   ulDriveMap   = 0;      /* Mapping of valid drives              */
  ULONG   i            = 0;      /* A loop index                         */

  rc = DosQueryCurrentDisk (&ulDriveNum, &ulDriveMap);
  chDisk=(UCHAR)(ulDriveNum+'A'-1);

  cbDirPathLen = 0;
  rc = DosQueryCurrentDir(0, achDirName, &cbDirPathLen);
  rc = DosQueryCurrentDir(0, achDirName, &cbDirPathLen);

  VioWrtTTY(&chDisk, 1, 0);
  VioWrtTTY(":\\", 2, 0);
  VioWrtTTY(achDirName, cbDirPathLen-1, 0);
  VioWrtTTY(">", 1, 0);
}

void main(void)
{
  UCHAR cmd[255];
  BOOL exitflag;

  hello();

  exitflag=FALSE;
  // Ok. Here we just in endless loop. Except for EXIT command.
  while (!exitflag)
  {
    showpath();
    read_cmd(&cmd);
    exitflag=parse_cmd(cmd);
  }
}
