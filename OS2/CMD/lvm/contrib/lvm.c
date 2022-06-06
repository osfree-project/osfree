/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: lvm.c
 */

/*
 * Change History:
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include "constant.h"
#include "display.h"
#include "getkey.h"

#include "panels.h"
#include "user.h"
#include "strings.h"
#include "LVM_Cli.h"
#include "lvm2.h"
#include "LVM_Interface.h"
#include "Logging.h"

#define DEFAULT_ATTRIBUTE       PANEL_ATTRIBUTE
#define EXIT_ATTRIBUTE          BLACK_BG | WHITE_FG  /* screen on exit */


//PUBLIC
bool    Install_time = FALSE;                              /* installing or not */

void InitializeUserInterface ( uint default_attribute, uint exit_attribute );
uint RunUserInterface ( uint  min_install_size );
int Help(void);



/*
 * signal_handler ignores the break signal
 *
 */

PRIVATE
int   last_signal_value;

PRIVATE
void
signal_handler ( int   signal_value )
{
    last_signal_value = signal_value;

    signal ( SIGINT, signal_handler );
}


/*
 * main
 * Returns the last error code that was received from the engine on
 * Commit_Changes. So the return value is one of the LVM engine codes,
 * with 0 being no error.
 *
 */


PUBLIC
uint
main ( int  argc,
       char *argv[] )
{
    uint                    min_install_size = 0,
                            error;
    LVMCLI_BackEndToVIO     *cli;

    if ( argc > 1 ) {
        cli = lvmcli ( argc, argv );
        if ( cli->operation & DisplayMinInstallSize ) {
            min_install_size = cli->minPartitonInstallSize;
            Install_time = TRUE;
        }
    }

    #ifndef DEBUG
        signal ( SIGINT, signal_handler );
    #endif

    InitializeUserInterface ( DEFAULT_ATTRIBUTE, EXIT_ATTRIBUTE );
    error = RunUserInterface ( min_install_size );

    ReInitializePanels ( EXIT_ATTRIBUTE, EXIT_ATTRIBUTE );

    return  error;
}

static char *LVM_cmd[]=
{
 "BOOTMGR","CREATE", "DELETE",  "EXPAND",       "HIDE",
 "NEWMBR", "QUERY",  "SETNAME", "SETSTARTABLE", "STARTLOG",
 "REDISCOVERPRM",
 NULL
};

LVMCLI_BackEndToVIO* lvmcli( int argc, char * argv[] )
{
static  LVMCLI_BackEndToVIO   cli;
CARDINAL32 error;
  int i, iscmd;
  char *pstr;

  cli.operation = 0;

  if( !strcmp(argv[1],"/?") || !strcmp(argv[1],"-?") || !stricmp(argv[1],"-h") )
  { Help();
  }
  pstr = argv[1];
  if(*pstr == '/') pstr++;
  else if(*pstr == '-') pstr++;
  iscmd = -1;
  for(i=0; LVM_cmd[i]; i++)
  {  if(!strnicmp(pstr, LVM_cmd[i],strlen(LVM_cmd[i])) )
     {  iscmd = i;
        break;
     }
  }
  if(iscmd == -1) Help();
  if( argc > 1) //STARTLOG
  {    int loglevel=0;
       pstr = argv[argc-1];
       if(*pstr == '/') pstr++;
       else if(*pstr == '-') pstr++;
       if(!strnicmp(pstr, LVM_cmd[9],strlen(LVM_cmd[9])) )
       { argc--;
         pstr += strlen(LVM_cmd[9]);
         if(*pstr == '+') { loglevel++; pstr++; }
         if(*pstr != ':')  Help();
         pstr++;
         if(strlen(pstr)<1)  Help();
         Logging_Enabled = TRUE;
         printf("File %s loglevel %i\n", pstr,loglevel);
         Start_Logging(pstr, &error,loglevel);
         if(error)
         {
           printf("Error start logging %i\n", error);
           exit(1);
         }
       }
  }

  switch(iscmd)
  {  case 0: //BOOTMGR
          printf("todo:/BOOTMGR\n");
      break;
     case 1: //CREATE
          printf("todo:/CREATE\n");
      break;
     case 2: //DELETE
          printf("todo:/CREATE\n");
      break;
     case 3: //EXPAND
          printf("todo:/CREATE\n");
      break;
     case 4: //HIDE
          printf("todo:/CREATE\n");
      break;
     case 5: //NEWMBR
          printf("todo:/CREATE\n");
      break;
     case 6: //QUERY
          printf("todo:/CREATE\n");
      break;
     case 7: //SETNAME
          printf("todo:/CREATE\n");
      break;
     case 8: //SETSTARTABLE
          printf("todo:/CREATE\n");
      break;
     case 9: //STARTLOG
/*
        pstr += strlen(LVM_cmd[9]);
        if(*pstr != ':')  Help();
        pstr++;
        if(strlen(pstr)<1)  Help();
        Logging_Enabled = TRUE;
        printf("File %s\n", pstr);
        Start_Logging(pstr, &error,0);
        if(error)
        {
           printf("Error start logging %i\n", error);
           exit(1);
        }
*/
      break;
     case 10:  //REDISCOVERPRM
         Rediscover_PRMs(&error);
         if ( error )
         {  printf("Rediscover_PRMs error=%i\n",error);
            exit(1);
         } else {
            printf("Ok\n");
            exit(0);
         }

      break;
     default:
       printf("todo:%s\n",argv[1]);

  }
  return &cli;
}

int Help(void)
{  printf("ALVM vers %s changes (c)eCo Software\n", ALVM_VERSION);
   printf("Usage: LVM [/cmd] [/STARTLOG[+]:logfile]\n");
   printf("cmd: BOOTMGR | CREATE | DELETE | EXPAND | HIDE | NEWMBR | QUERY\n");
   printf("     REDISCOVERPRM | SETNAME | SETSTARTABLE\n");
   printf("----------------------------------------------------------\n");
   printf("QUERY PRIMARY|LOGICAL|FREESPACE|UNUSABLE|VOLUMES|COMPATIBILITY|LVM|ALL|BOOTABLE\n");
   printf("      [ drive_number|drive_name|ALL ] [ , FS_type]\n");
   printf("----------------------------------------------------------\n");
   printf("CREATE PARTITION , partition_name drv_number|drv_name, size, \n");
   printf("  LOGICAL|PRIMARY , BOOTABLE|NONBOOTABLE  [ , BESTFIT|FIRSTFIT|LASTFIT|\n");
   printf("  FOMLARGEST|FROMSMALLEST|free_space_id ] [ , FOMSTART|FROMEND]\n");

   printf("CREATE VOLUME, COMPATIBILITY BOOTDOS|BOOTOS2|NOOBOOT drv_letter , volume_name,\n");
   printf("       drive_number|drive_name , partition_name\n");

   printf("CREATE VOLUME , LVM drive_letter , volume_name  , volume_name , \n");
   printf("       drive_number|drive_name , partition_name\n");
   printf("----------------------------------------------------------\n");
//   printf("STARTLOG:logfile\n");

/*
query parameters --+-- PRIMARY -+---------------------------------+--------+---
                   |            +-- , optional_query_parameters --+        |
                   +-- LOGICAL -+---------------------------------+--------+
                   |            +-- , optional_query_parameters --+        |
                   +-- FREESPACE -+---------------------------------+------+
                   |              +-- , optional_query_parameters --+      |
                   +-- UNUSABLE --+---------------------------------+------+
                   |              +-- , optional_query_parameters --+      |
                   +-- VOLUMES --+---------------------------------+-------+
                   |             +-- , optional_query_parameters --+       |
                   +-- COMPATIBILITY -+---------------------------------+--+
                   |                  +-- , optional_query_parameters --+  |
                   +-- LVM --+---------------------------------+-----------+
                   |         +-- , optional_query_parameters --+           |
                   +-- ALL --+---------------------------------+-----------+
                   |         +-- , optional_query_parameters --+           |
                   +-- BOOTABLE -+-------------------------+---------------+
                                 +-- , -+-- drive_number --+
                                        +-- drive_name ----+
                                        +-- ALL -----------+
*/

      exit(1);
}
