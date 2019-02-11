/*!
  @file parser.c

  @brief config.sys parser
   Dedicated to JESUS CHRIST, my lord and savior
   Version: 0.12
   Copyright 2007 by Sascha Schmidt and the osFree Project
   This is free software under the terms of GPL v2 or later

   parses the systems config.sys file during bootup

  @author Sascha Schmidt <sascha.schmidt@asamnet.de>

*/

/* OS/2 API defines */
#define  INCL_BASE
#include <os2.h>

/* osFree defines */
#include <os3/io.h>
#include <os3/cfgparser.h>

/* LibC defines */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void error(const char *);
int warn(const char *);

char *strupr(char *s);

// Maximal line length
#define MAXLENGTH 1512

// This statement is ignored in comparison with original config.sys
//char *skiplist[] = { "DEVICE", "BASEDEV", "IFS", "REM " };
const char *skiplist[] = { "REM " };

// Arrays of pointers; used to store the content of config.sys
// @todo use dynamic allocation

struct name runserver[30];
struct name run[30];
struct name call[30];
struct name set[150];
struct name ixf[30];

struct types type[] =
{
   {"CALL", '=', call, 0},
   {"RUNSERVER", '=', runserver, 0},
   {"RUN", '=', run, 0},
   {"SET", '=', set, 0},
   {"IXF", '=', ixf, 0}
};

cfg_opts options;

const char *options_list[] =
{
  "AUTOFAIL", "BUFFERS", "CLOCKSCALE", "CLOSEFILES",
  "CODEPAGE", "COUNTRY", "DEVINFO", "DISKCACHE", "DLLBASING",
  "DUMPPROCESS", "EARLYMEMINIT", "FAKEISS", "I13PAGES", "IBM_JAVA_OPTIONS", "IOPL",
  "IROPT", "JAVANOFPK", "LASTDRIVE", "LDRSTACKOPT", "LIBPATH", "MAXWAIT", "MEMMAN",
  "MODE", "NDWTIMER", "PAUSEONERROR", "PRINTMONBUFSIZE", "PRIORITY",
  "PRIORITY_DISK_IO", "PROTECTONLY", "PROTSHELL", "RASKDATA", "REIPL",
  "REMOTE_INSTALL_STATE", "RESERVEDRIVELETTER", "SETBOOT", "SOURCEPATH", "STRACE",
  "SUPPRESSPOPUPS", "SWAPPATH", "SXFAKEHWFPU", "SYSDUMP", "THREADS", "TIMESLICE",
  "TRACE", "TRACEBUF", "TRAPDUMP", "TRAPLOG", "TRUEMODE", "VIRTUALADDRESSLIMIT",
  "VME", "WORKPLACE_NATIVE", "WORKPLACE_PRIMARY_CP", "WORKPLACE_PROCESS", "WP_OBJHANDLE",
  // Debugging options
  "DEBUGMODMGR", "DEBUGIXFMGR", "DEBUGPRCMGR"
};

/* char *
strupr(char *s)
{
  char *p;

  for (p = s; *p; p++)
    *p = toupper(*p);

  return s;
} */

/*! @brief Sets the default values in the options structure


    @return
      NO_ERROR                  Server finished successfully
      ERROR_INVALID_PARAMETER   Invalid CONFIG.SYS settings

*/
unsigned long CfgInitOptions()
{
  char *p, *str;

  options.autofail  = 0;
  options.dllbasing = 1;

  str = "YES";
  if ( !(p = (char *)malloc(strlen(str) + 1)) )
  {
    error("init_options: memory could not be allocated!");
    return ERROR_INVALID_PARAMETER;
  }

  strcpy(p, str);
  options.iopl = p;
  options.ldrstackopt = 1;
  options.maxwait = 3;
  options.mode_id = 0;
  options.pauseonerror = 1;
  options.priority = 1;
  options.priority_disk_io = 1;
  options.reipl = 0;
  options.protshell = NULL;
  options.debugmodmgr = 0;
  options.debugixfmgr = 0;
  options.debugprcmgr = 0;
  options.debugmemmgr = 0;

  str = "config.sys";
  //str = "/file/system/config.sys";
  if( !(p = (char *)malloc(strlen(str) + 1)) )
  {
    error("init_options: memory could not be allocated!");
    return ERROR_INVALID_PARAMETER;
  }

  strcpy(p, str);
  options.configfile = p;

  str = "C:";
  if ( !(p = (char *)malloc(strlen(str) + 1)) )
  {
    error("init_options: memory could not be allocated!");
    return ERROR_INVALID_PARAMETER;
  }

  strcpy(p, str);
  options.bootdrive = p;

  return NO_ERROR;
}

/**********************************************************************
 * The main parse function; gets a line and its length as arguments,  *
 * parses it, puts the content to the array it belongs to and returns.*
 * On error it returns NULL                                           *
 **********************************************************************/
unsigned long CfgParseLine(char line[], int len, int lineno)
{
  unsigned int count = 0, i = 0;
  int helper = 0;
  char *pc = NULL;

  for (i = 0; i < sizeof(type)/sizeof(struct types); i++)
  {
    /* Compares line and type[i].name and breaks
       on a difference */

    for (count = 0; count < strlen(type[i].name); count++)
    {
      /* Compares line and type[i].name and breaks
         on a difference */

      if (toupper(line[count]) != type[i].name[count])
      {
        break;
      }
    }

    if (count == strlen(type[i].name))
    {
      count++;
      strcpy(line,line + count);
      len -= count;

      if ( !(pc = (char *)malloc((size_t)len + 1)) )
        return(0);

      strcpy(pc,line);

      type[i].sp[type[i].ip].line = lineno;
      type[i].sp[type[i].ip].string = pc;

      type[i].ip++;

      return 1;
    }
  }

  for (i = 0; i < sizeof(options_list)/sizeof(char *); i++)
  {
    for (count = 0; count < strlen(options_list[i]); count++)
    {
      /* Compares line and type[i].name and breaks
         on a difference */

      if (toupper(line[count]) != options_list[i][count])
      {
        break;
      }
    }

    if (count == strlen(options_list[i]) && toupper(line[count]) == '=')
    {
      count++;
      strcpy(line, line + count);
      len -= count;

      switch (i)
      {
        case 0: // AUTOFAIL
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.autofail = 0;
          }
          else if (toupper(line[0]) == 'Y' &&
                   toupper(line[1]) == 'E' &&
                   toupper(line[2]) == 'S')
          {
            options.autofail = 1;
          }
          else
            warn("Wrong AUTOFAIL argument");
          break;

        case 1: // BUFFERS
          options.buffers = (short int)atoi(line);
          break;

        case 2: // CLOCKSCALE
          helper = atoi(line);
          if (helper == 1 || helper == 2 || helper == 4)
            options.clockscale=(short int)helper;
          else
            error("Syntax error in config.sys line.");
          break;

        case 3: // CLOSEFILES
          if (toupper(line[0]) == 'T' &&
              toupper(line[1]) == 'R' &&
              toupper(line[2]) == 'U' &&
              toupper(line[3]) == 'E')
          {
            options.closefiles = 1;
          }
          else
          {
            options.closefiles = 0;
          }
          break;

        case 4: // CODEPAGE
          for (helper = 0; helper < 7; helper++)
          {
            options.codepage[helper] = line[helper];
          }
          break;

        case 5: // COUNTRY
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, strupr(line));
          options.country = pc;
          break;

        case 6: // DEVINFO
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, strupr(line));

          if (pc[0] == 'K' &&
              pc[1] == 'B' &&
              pc[2] == 'D')
            options.devinfo_kbd = pc;
          else if (pc[0] == 'S' &&
                   pc[1] == 'C' &&
                   pc[2] == 'R')
            options.devinfo_vio = pc;
          break;

      /*  case 7: // DEVINFO_VIO
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, strupr(line));
          options.devinfo_vio = pc;
          break; */

        case 7: // DISKCACHE
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, strupr(line));
          options.diskcache = pc;
          break;

        case 8: // DLLBASING
          if (toupper(line[0]) == 'O' &&
              toupper(line[1]) == 'N')
          {
            options.dllbasing = 1;
          }
          else if (toupper(line[0]) == 'O' &&
                   toupper(line[1]) == 'F' &&
                   toupper(line[2]) == 'F')
          {
            options.dllbasing = 0;
          }
          else
            warn("Wrong DLLBASING argument");
          break;

        case 9: // DUMPPROCESS
          options.dumpprocess = line[0];
          break;

        case 10: // EARLYMEMINIT
          if (toupper(line[0]) == 'T' &&
              toupper(line[1]) == 'R' &&
              toupper(line[2]) == 'U' &&
              toupper(line[3]) == 'E')
          {
            options.earlymeminit = 1;
          }
          else
          {
            options.earlymeminit = 0;
          }
          break;

        case 11: // FAKEISS
          if (toupper(line[0]) == 'Y' &&
              toupper(line[1]) == 'E' &&
              toupper(line[2]) == 'S')
          {
            options.fakeiss = 1;
          }
          else
          {
            options.fakeiss = 0;
          }
          break;

        case 12: // I13PAGES
          if (line[0] == '1')
          {
            options.i13pages = 1;
          }
          else
          {
            options.i13pages = 0;
          }
          break;

        case 13: // IBM_JAVA_OPTIONS
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.ibm_java_options = pc;
          break;

        case 14: // IOPL
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.iopl = pc;
          break;

        case 15: // IROPT
          if( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.iropt = pc;
          break;

        case 16: // JAVANOFPK
          if (toupper(line[0]) == 'Y' &&
              toupper(line[1]) == 'E' &&
              toupper(line[2]) == 'S')
          {
            options.javanofpk = 1;
          }
          else
          {
            options.javanofpk = 0;
          }
          break;

        case 17: // LASTDRIVE
          options.lastdrive = line[0];
          break;

        case 18: // LDRSTACKOPT
          if (toupper(line[0]) == 'T' &&
              toupper(line[1]) == 'R' &&
              toupper(line[2]) == 'U' &&
              toupper(line[3]) == 'E')
          {
            options.ldrstackopt = 1;
          }
          else if (toupper(line[0]) == 'F' &&
                   toupper(line[1]) == 'A' &&
                   toupper(line[2]) == 'L' &&
                   toupper(line[3]) == 'S' &&
                   toupper(line[4]) == 'E')
          {
            options.ldrstackopt = 0;
          }
          else
            warn("Wrong LDRSTACKOPT argument");
          break;

        case 19: // LIBPATH
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.libpath = pc;
          break;

        case 20: // MAXWAIT
          helper = atoi(line);
          if (helper >= 1 && helper <= 255)
            options.maxwait = (short int)helper;
          else
            error("Syntax error in config.sys line.");
          break;

        case 21: // MEMMAN
          if ( !(pc = (char *)malloc((size_t)len + 1)))
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.memman = pc;
          break;

        case 22: // MODE
          if (options.mode_id < 20)
          {
            if ( !(pc = (char *)malloc((size_t)len + 1)) )
              error("Memory could not be allocated!");

            strcpy(pc, line);
            options.mode[options.mode_id++] = pc;
          }
          break;

        case 23: // NDWTIMER
          if (toupper(line[0]) == 'O' &&
              toupper(line[1]) == 'F' &&
              toupper(line[2]) == 'F')
          {
            options.ndwtimer = 0;
          }
          else
          {
            options.ndwtimer = 1;
          }
          break;

        case 24: // PAUSEONERROR
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.pauseonerror = 0;
          }
          else if (toupper(line[0]) == 'Y' &&
                   toupper(line[1]) == 'E' &&
                   toupper(line[2]) == 'S')
          {
            options.pauseonerror = 1;
          }
          else
            warn("Wrong PAUSEONERROR argument");
          break;

        case 25: // PRINTMONBUFSIZE
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.printmonbufsize = pc;
          break;

        case 26: // PRIORITY
          if (toupper(line[0]) == 'D' &&
              toupper(line[1]) == 'Y' &&
              toupper(line[2]) == 'N' &&
              toupper(line[3]) == 'A' &&
              toupper(line[4]) == 'M' &&
              toupper(line[5]) == 'I' &&
              toupper(line[6]) == 'C')
          {
            options.priority = 1;
          }
          else if (toupper(line[0]) == 'A' &&
                   toupper(line[1]) == 'B' &&
                   toupper(line[2]) == 'S' &&
                   toupper(line[3]) == 'O' &&
                   toupper(line[4]) == 'L' &&
                   toupper(line[5]) == 'U' &&
                   toupper(line[6]) == 'T' &&
                   toupper(line[7]) == 'E')
          {
            options.priority = 0;
          }
          else
            warn("Wrong PRIORITY argument");
          break;

        case 27: // PRIORITY_DISK_IO
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.priority_disk_io = 0;
          }
          else if (toupper(line[0]) == 'Y' &&
                   toupper(line[1]) == 'E' &&
                   toupper(line[2]) == 'S')
          {
            options.priority_disk_io=1;
          }
          else
            warn("Wrong PRIORITY_DISK_IO argument");
          break;

        case 28: // PROTECTONLY
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.protectonly = 0;
          }
          else if (toupper(line[0]) == 'Y' &&
                   toupper(line[1]) == 'E' &&
                   toupper(line[2]) == 'S')
          {
            options.protectonly = 1;
          }
          else
            warn("Wrong PROTECTONLY argument");
          break;

        case 29: // PROTSHELL
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.protshell = pc;
          break;

        case 30: // RASKDATA
          if (toupper(line[0]) == 'O' &&
              toupper(line[1]) == 'T' &&
              toupper(line[2]) == 'E')
          {
            options.raskdata = 1;
          }
          else
          {
            options.raskdata = 0;
          }
          break;

        case 31: // REIPL
          if (toupper(line[0]) == 'O' &&
              toupper(line[1]) == 'N')
          {
            options.reipl = 1;
          }
          else if (toupper(line[0]) == 'O' &&
                   toupper(line[1]) == 'F' &&
                   toupper(line[2]) == 'F')
          {
            options.reipl = 0;
          }
          else
            warn("Wrong REIPL argument");
          break;

        case 32: // REMOTE_INSTALL_STATE
          options.remote_install_state = line[0];
          break;

        case 33: // RESERVEDRIVELETTER
          options.reservedriveletter = line[0];
          break;

        case 34: // SETBOOT
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.setboot = pc;
          break;

        case 35: // SOURCEPATH
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.sourcepath = pc;
          break;

        case 36: // STRACE
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.strace = pc;
          break;

        case 37: // SUPPRESSPOPUPS
          options.suppresspopups = line[0];
          break;

        case 38: // SWAPPATH
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.swappath = pc;
          break;

        case 39: // SXFAKEHWFPU
          if (line[0] == '1')
          {
            options.sxfakehwfpu = 1;
          }
          else
          {
            options.sxfakehwfpu = 0;
          }
          break;

        case 40: // SYSDUMP
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.sysdump = pc;
          break;

        case 41: // THREADS
          options.threads = atoi(line);
          break;

        case 42: // TIMESLICE
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.timeslice = pc;
          break;

        case 43: // TRACE
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.trace = pc;
          break;

        case 44: // TRACEBUF
          options.tracebuf = atoi(line);
          break;

        case 45: // TRAPDUMP
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.trapdump = pc;
          break;

        case 46: // TRAPLOG
          if ( !(pc = (char *)malloc((size_t)len + 1)) )
            error("Memory could not be allocated!");
          strcpy(pc, line);
          options.traplog = pc;
          break;

        case 47: // TRUEMODE
          if (line[0] == '1')
          {
            options.truemode = 1;
          }
          else
          {
            options.truemode = 0;
          }
          break;

        case 48: // VIRTUALADDRESSLIMIT
          options.virtualaddresslimit = atoi(line);
          break;

        case 49: // VME
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.vme = 0;
          }
          else
          {
            options.vme = 1;
          }
          break;

        case 50: // WORKPLACE_NATIVE
          if (line[0] == '0')
          {
            options.workplace_native = 0;
          }
          else
          {
            options.workplace_native = 1;
          }
          break;

        case 51: // WORKPLACE_PRIMARY_CP
          if (line[0] == '1')
          {
            options.workplace_primary_cp = 1;
          }
          else
          {
            options.workplace_primary_cp = 0;
          }
          break;

        case 52: // WORKPLACE_PROCESS
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.workplace_process = 0;
          }
          else
          {
            options.workplace_process = 1;
          }
          break;

        case 53: // WP_OBJHANDLE
          options.wp_objhandle = atoi(line);
          break;

        case 54: // DEBUGMODMGR
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.debugmodmgr = 0;
          }
          else
          {
            options.debugmodmgr = 1;
          }
          break;

        case 55: // DEBUGIXFMGR
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.debugixfmgr = 0;
          }
          else
          {
            options.debugixfmgr = 1;
          }
          break;

        case 56: // DEBUGPRCMGR
          if (toupper(line[0]) == 'N' &&
              toupper(line[1]) == 'O')
          {
            options.debugprcmgr = 0;
          }
          else
          {
            options.debugprcmgr = 1;
          }
          break;

      default:
          io_log("Oh, well, this should not occur. Please note down the following to lines:\n%s\n%s\n", options_list[i], line);
          break;
      }

      return 1;
    }
  }

  return 2;
}

/**********************************************************************
 * cleanup() does what it name says: It cleans up ;-) It frees the    *
 * memory, the program used.                                          *
 **********************************************************************/
unsigned long CfgCleanup()
{
  unsigned int i = 0;
  int j;

  for (i = 0; i < sizeof(type)/sizeof(struct types); i++)
  {
    for (j = 0; j < type[i].ip; j++)
    {
      if (type[i].sp[j].string != NULL)
        free(type[i].sp[j].string);
    }
  }

  if (options.iopl != NULL)
    free(options.iopl);

  return 0;
}

/**********************************************************************
 * error() gets an error message as the only argument, prints it to   *
 * the screen and ends the programm.                                  *
   @todo remove this and return error codes instead
 **********************************************************************/
void error(const char *msg)
{
  io_log("failure-exit:\n%s",msg);
  exit(-1);
}

/**********************************************************************
 * warn() gets an warning message as the only argument, prints it to  *
 * the screen and returns -1.                                         *
   @todo remove this and return error codes instead
 **********************************************************************/
int warn(const char *msg)
{
  io_log("warning:\n%s\n",msg);
  return -1;
}

/*! @todo Add check for exceed MAXLENGTH */
unsigned long CfgParseConfig(char *addr, int size)
{
  int  off = 0;          // Current offset in CONFIG.SYS memory area
  char line[MAXLENGTH];  // here I store the lines I read
  int  len = 0;          // length of returned line
  char c;                // Current character in CONFIG.SYS
  int  lineno = 0;

  len = 0;

  while ( off < size &&
          len < MAXLENGTH )
  {
    c = addr[off];
    line[len] = c;

    if (c == '\r' ||
        c == '\n')
    {
      line[len] = '\0';

      if ( ! CfgParseLine(line, len, lineno) )
        error("parse: an error occured\n");

      len = 0;
      lineno++;
    }
    else
    {
      len++;
    }

    off++;

    if ( off < size &&
         ( addr[off] == '\r' ||
           addr[off] == '\n' ) )
      off++;
  }

  return 0;
};

unsigned long CfgGetenv(char *name, char **value)
{
  int i;
  unsigned long j;
  char *p;

  io_log("name=%s\n", name);
  for(i = 0; i < type[3].ip; i++)
  {
    /* find '=' in string */

    for(j = 0, p = type[3].sp[i].string; *p && *p != '='; j++, p++)
    {
      if (*p != toupper(name[j]))
        break;
    }

    /* if we met '=' sign, i.e. name found */

    if (*p == '=')
    {
      /* skip '=' sign */

      p++;
      strcpy(*value, p);
      io_log("value=%s\n", p);
      return NO_ERROR;
    }
  }

  io_log("not found\n");
  return ERROR_ENVVAR_NOT_FOUND ;
}
