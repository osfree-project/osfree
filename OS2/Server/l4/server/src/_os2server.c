/*
 */

// L4 headers
#include <l4/sys/types.h>
#include <l4/env/errno.h>
#include <l4/sys/syscalls.h>
#include <l4/log/l4log.h>
#include <l4/l4rm/l4rm.h>
#include <l4/util/util.h>
#include <l4/env/env.h>
#include <l4/names/libnames.h>
#include <l4/generic_fprov/generic_fprov-client.h>
#include <l4/util/parse_cmd.h>
#include <l4/loader/loader-client.h>
#include <l4/loader/loader.h>
#include <l4/events/events.h>
#include <l4/thread/thread.h>
#include <l4/generic_ts/generic_ts.h>
#include <l4/util/reboot.h>
#include <l4/util/l4_macros.h>
#include <l4/sigma0/sigma0.h>
#include <l4/l4con/l4contxt.h>

// uLibC defines
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>

// os2server header
#include "os2server-server.h"
#include "io.h"

#ifdef __cplusplus
extern "C" {
#endif

void
os2server_VioWrtTTY_component (CORBA_Object _dice_corba_obj,
                               const char* buf,
                               int bufsize,
                               int handle,
                               CORBA_Server_Environment *_dice_corba_env)
{
    #warning "os2server_VioWrtTTY_component is not implemented!"
    io_printf("Hello from VioWrtTTY\n");
}

#ifdef __cplusplus
}
#endif


/******************* config.sys parser ***********************
 * Dedicated to JESUS CHRIST, my lord and savior             *
 *                                                           *
 * Author: Sascha Schmidt <sascha.schmidt@asamnet.de         *
 * Version: 0.12                                             *
 * Copyright 2007 by Sascha Schmidt and the osFree Project   *
 * This is free software under the terms of GPL v2           *
 *                                                           *
 * parses the systems config.sys file during bootup          *
 * using generic file provider interface                     *
 *************************************************************/

#define MAXLENGTH 1512  // Maximal line length


//#define DEBUG

char LOG_tag[9] = "os2server";

char *skiplist[]={"DEVICE","BASEDEV","IFS","REM "};

// Arrays of pointers; used to store the content of config.sys
char  *run[30];
char  *call[30];
char  *set[150];

struct types {
        const char *name;       // Name of config.sys command
        const char sep; // Not used so far, perhaps I will delete it
        char **sp;              // Pointer to the content
        int ip;         // Number of elements in the arrays
}type[]={
//   {"STATEMENT",'=',statement,0},
   {"CALL",'=',call,0},
   {"RUN",'=',run,0},
   {"SET",'=',set,0}
};


/* Structure containing some environment options. For more information*
 * see http://www.os2world.com/goran/cfgtool.htm for example.         */
typedef struct {
        short int autofail;     // YES or NO (default)
        short int buffers;      // 3-100
        short int clockscale;   // 1,2,4
        short int closefiles;   // TRUE
        char codepage[7];       // ppp,sss (Prim. and sec. codepage)
        char * country;         // =XXX,drive:\OS2\SYSTEM\COUNTRY.SYS
        char * devinfo_kbd;     // =KBD,XX,drive:,\OS2\KEYBOARD.DCP
        char * devinfo_vio;     // =SCR,VGA,drive:\OS2\BOOT\VIOTBL.DCP
        char * diskcache;       // =D,LW,32,AC:CD+E
        short int dllbasing;            // ON (default) or OFF
        char dumpprocess;       // = drive
        short int earlymeminit; // =TRUE (I think we won't need in in osFree)
        short int fakeiss;      // =YES (has something to do with drivers, so we won't need it)
        short int i13pages;     // =1
        char * ibm_java_options;// =-Dmysysprop1=tcpip -Dmysysprop2=wait
        char * iopl;            // YES (default) or NO (I would like to see NO as default in osFree...) or a comma seperated list of programms allowed to directly work with hardware
        char * iropt;           // =value
        short int javanofpk;    // =YES; deals with fixpacks, therefore I think we need to set it to YES (which disables fixpacks in a case)
        char lastdrive;         // =drive
        short int ldrstackopt;  // FALSE or TRUE (default) (I don't think we need this setting)
        char * libpath;         // =.;C:\.... (Syntax like PATH)
        short int maxwait;      // 1 to 255, default is 3; only has an effect if PRIORITY=DYNAMIC
        char * memman;          // =s,m,PROTECT,COMMIT where s=SWAP or NOSWAP; m=MOVE or NOMOVE (is provided for OS/2 1.x compatibility; do we need it?!?)
        char * mode[20];        // can exist multiple times => types?!?
        int mode_id;            // helper variable
        short int ndwtimer;     // =OFF
        short int pauseonerror; // YES(default) or NO
        char * printmonbufsize; // =n,n,n (in byte)
        short int priority;     // DYNAMIC(default=1) or ABSOLUTE(=0)
        short int priority_disk_io;     // YES (default) or NO
        short int protectonly;  // YES or NO; do we need it?
        char * protshell;       // =drive:\OS2\PMSHELL.EXE
        short int raskdata;     // =OTE; I think we don't need to have it
        short int reipl;                // ON or OFF (default)
        char remote_install_state; // =X
        char reservedriveletter;// =driveletter
        char * setboot;         // configures OS/2 bootmanager => I think we can ignore it
        char * sourcepath;      // =path
        char * strace;          // several possible?!?
        char suppresspopups;    // 0 or driveletter
        char * swappath;        // =drive,path,mmm(in kb),nnn (in kb)
        short int sxfakehwfpu;  // =1 (do we need it?)
        char * sysdump;         // nothing or /NOPROMPT if set
        int threads;            // =n; number of threads; isn't this handled in the kernel? do we need it?
        char * timeslice;       // =x,y (in milliseconds)
        char * trace;           // =ON, OFF, n
        int tracebuf;           // =n (inkb)
        char * trapdump;        // =value
        char * traplog;         // =value
        short int truemode;     // =1
        int virtualaddresslimit;// =n (in kb)
        short int vme;          // =NO
        short int workplace_native;     // =0
        short int workplace_primary_cp; // =1
        short int workplace_process;    // =NO
        int wp_objhandle;       // =n
} opts;

opts options;


char *options_list[]={"AUTOFAIL","BUFFERS","CLOCKSCALE","CLOSEFILES",
"CODEPAGE","COUNTRY","DEVINFO_KBD","DEVINFO_VIO","DISKCACHE","DLLBASING",
"DUMPPROCESS","EARLYMEMINIT","FAKEISS","I13PAGES","IBM_JAVA_OPTIONS","IOPL",
"IROPT","JAVANOFPK","LASTDRIVE","LDRSTACKOPT","LIBPATH","MAXWAIT","MEMMAN",
"MODE","NDWTIMER","PAUSEONERROR","PRINTMONBUFSIZE","PRIORITY",
"PRIORITY_DISK_IO","PROTECTONLY","PROTSHELL","RASKDATA","REIPL",
"REMOTE_INSTALL_STATE","RESERVEDRIVELETTER","SETBOOT","SOURCEPATH","STRACE",
"SUPPRESSPOPUPS","SWAPPATH","SXFAKEHWFPU","SYSDUMP","THREADS","TIMESLICE",
"TRACE","TRACEBUF","TRAPDUMP","TRAPLOG","TRUEMODE","VIRTUALADDRESSLIMIT",
"VME","WORKPLACE_NATIVE","WORKPLACE_PRIMARY_CP","WORKPLACE_PROCESS","WP_OBJHANDLE"};

// Function prototypes
int init_options(void);
int open_config_sys(void);
int close_config_sys(int);
int parse(char *, int);
int fgetline(int,char *);
void error(char *);
int print_tree(void);
int cleanup(void);
int warn(char *);
void executeprotshell(opts *);

static l4_threadid_t loader_id;         /**< L4 thread id of L4 loader. */
static l4_threadid_t tftp_id;           /**< L4 thread id of TFTP daemon. */

static char searchpath[]                /**< for load_app(). */
        = "/boot/l4/";


/**********************************************************************
 * init_options() sets the default values in the options structure    *
 **********************************************************************/

int init_options()
{
        char * p;

        options.autofail=0;
        options.dllbasing=1;

        if(!(p=(char *)malloc(3))) error("init_options: memory could not be allocated!");
        strcpy(p,"YES");
        options.iopl=p;
        options.ldrstackopt=1;
        options.maxwait=3;
        options.mode_id=0;
        options.pauseonerror=1;
        options.priority=1;
        options.priority_disk_io=1;
        options.reipl=0;
        options.protshell=NULL;
        return(1);
}


/**********************************************************************
 * print_tree() is some kind of debug or test function. It only       *
 * prints out the part of config.sys we use. It has to be replaced by *
 * something useful.                                                  *
 **********************************************************************/
int print_tree()
{
        int x=1, i=0;

        io_printf("Config tree:\n\n");

        for(i=0;i<sizeof(type)/sizeof(struct types);i++) {
                io_printf("%s:\n",type[i].name);
                for(x=1;x-1<type[i].ip;x++) {
                        io_printf("\t%s\n",type[i].sp[x-1]);
                }
        }
        return(1);
}


/**********************************************************************
 * The main parse function; gets a line and its length as arguments,  *
 * parses it, puts the content to the array it belongs to and returns.*
 * On error it returns NULL                                           *
 **********************************************************************/
int parse(char line[], int len)
{
        int count=0,i=0;
        int helper=0;
//      char c;
        char *pc=NULL;
//      for(i=0;i<sizeof(skiplist)/sizeof(char *);i++) {
//                for(count=0;count<strlen(skiplist[i]);count++){
                /* Compares line and type[i].name and breaks on a *
                 * difference */
//                      if(toupper(line[count])!=skiplist[i][count]) {
//                              break;
//                      }
//              }
//              if((c=toupper(line[count]))&& c != '=' && c != ' ' && c !='\n') break;
//              if(count==strlen(skiplist[i])) {
#ifdef DEBUG
//                      io_printf("Skipping: %s\n",line);
#endif
//                      return(2);
//              }
//      }
        for(i=0;i<sizeof(type)/sizeof(struct types);i++) {
                for(count=0;count<strlen(type[i].name);count++){
                /* Compares line and type[i].name and breaks on a
                 * difference */
                        if(toupper(line[count])!=type[i].name[count]) {
                                break;
                        }
                }
                if(count==strlen(type[i].name)) {
                        count++;
                        strcpy(line,line+count);
                        len-=count;

                        if(!(pc=(char *)malloc((size_t)len)))
                                return(0);
                        strcpy(pc,line);
                        type[i].sp[type[i].ip]=pc;

                        type[i].ip++;
                        return(1);
                }
        }
        for(i=0;i<sizeof(options_list)/sizeof(char *);i++) {
                for(count=0;count<strlen(options_list[i]);count++){
                /* Compares line and type[i].name and breaks on a
                 * difference */
                        if(toupper(line[count])!=options_list[i][count]) {
                                break;
                        }
                }
//              if((c=toupper(line[count-1]))&& c!='=')
//                      {
//                      io_printf("Hier: %c",c);
//                      break;
//                      }
                if(count==strlen(options_list[i])&& toupper(line[count])=='=') {
                        count++;
                        strcpy(line,line+count);
                        len-=count;

                        switch(i) {
                                case 0: // AUTOFAIL
                                        if(toupper(line[0])=='N'&&toupper(line[1])=='O')  {
                                                options.autofail=0;
                                        }
                                        else if(toupper(line[0])=='Y'&&toupper(line[1])=='E'&&toupper(line[2])=='S')  {
                                                options.autofail=1;
                                        }
                                        else warn("Wrong AUTOFAIL argument");
                                        break;
                                case 1: // BUFFERS
                                        options.buffers=(short int)atoi(line);
                                        break;
                                case 2: // CLOCKSCALE
                                        helper=atoi(line);
                                        if(helper==1 || helper==2 || helper==4)
                                                options.clockscale=(short int)helper;
                                        else error("Syntax error in config.sys line.");
                                        break;
                                case 3: // CLOSEFILES
                                        if(toupper(line[0])=='T'&&toupper(line[1])=='R'&&toupper(line[2])=='U'&&toupper(line[3])=='E') {
                                                options.closefiles=1;
                                        }
                                        else options.closefiles=0;
                                        break;
                                case 4: // CODEPAGE
                                        for(helper=0;helper<7;helper++) {
                                                options.codepage[helper]=line[helper];
                                        }
                                        break;
                                case 5: // COUNTRY
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.country=pc;
                                        break;
                                case 6: // DEVINFO_KBD
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.devinfo_kbd=pc;
                                        break;
                                case 7: // DEVINFO_VIO
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.devinfo_vio=pc;
                                        break;
                                case 8: // DISKCACHE
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.diskcache=pc;
                                        break;
                                case 9: // DLLBASING
                                        if(toupper(line[0])=='O'&&toupper(line[1])=='N')  {
                                                options.dllbasing=1;
                                        }
                                        else if(toupper(line[0])=='O'&&toupper(line[1])=='F'&&toupper(line[2])=='F')  {
                                                options.dllbasing=0;
                                        }
                                        else warn("Wrong DLLBASING argument");
                                        break;
                                case 10:// DUMPPROCESS
                                        options.dumpprocess=line[0];
                                        break;
                                case 11:// EARLYMEMINIT
                                        if(toupper(line[0])=='T'&&toupper(line[1])=='R'&&toupper(line[2])=='U'&&toupper(line[3])=='E') {
                                                options.earlymeminit=1;
                                        }
                                        else options.earlymeminit=0;
                                        break;
                                case 12:// FAKEISS
                                        if(toupper(line[0])=='Y'&&toupper(line[1])=='E'&&toupper(line[2])=='S') {
                                                options.fakeiss=1;
                                        }
                                        else options.fakeiss=0;
                                        break;
                                case 13:// I13PAGES
                                        if(line[0]=='1') {
                                                options.i13pages=1;
                                        }
                                        else options.i13pages=0;
                                        break;
                                case 14:// IBM_JAVA_OPTIONS
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.ibm_java_options=pc;
                                        break;
                                case 15:// IOPL
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.iopl=pc;
                                        break;
                                case 16:// IROPT
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.iropt=pc;
                                        break;
                                case 17:// JAVANOFPK
                                        if(toupper(line[0])=='Y'&&toupper(line[1])=='E'&&toupper(line[2])=='S') {
                                                options.javanofpk=1;
                                        }
                                        else options.javanofpk=0;
                                        break;
                                case 18:// LASTDRIVE
                                        options.lastdrive=line[0];
                                        break;
                                case 19:// LDRSTACKOPT
                                        if(toupper(line[0])=='T'&&toupper(line[1])=='R'&&toupper(line[2])=='U'&&toupper(line[3])=='E') {
                                                options.ldrstackopt=1;
                                        }
                                        else if(toupper(line[0])=='F'&&toupper(line[1])=='A'&&toupper(line[2])=='L'&&toupper(line[3])=='S'&&toupper(line[4])=='E') {
                                                options.ldrstackopt=0;
                                        }
                                        else warn("Wrong LDRSTACKOPT argument");
                                        break;
                                case 20:// LIBPATH
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.libpath=pc;
                                        break;
                                case 21:// MAXWAIT
                                        helper=atoi(line);
                                        if(helper>=1 && helper<=255)
                                                options.maxwait=(short int)helper;
                                        else error("Syntax error in config.sys line.");
                                        break;
                                case 22:// MEMMAN
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.memman=pc;
                                        break;
                                case 23:// MODE
                                        if(options.mode_id<20){
                                                if(!(pc=(char *)malloc((size_t)len)))
                                                        error("Memory could not be allocated!");
                                                strcpy(pc,line);
                                                options.mode[options.mode_id++]=pc;
                                        }
                                        break;
                                case 24:// NDWTIMER
                                        if(toupper(line[0])=='O'&&toupper(line[1])=='F'&&toupper(line[2])=='F') {
                                                options.ndwtimer=0;
                                        }
                                        else options.ndwtimer=1;
                                        break;
                                case 25:// PAUSEONERROR
                                        if(toupper(line[0])=='N'&&toupper(line[1])=='O')  {
                                                options.pauseonerror=0;
                                        }
                                        else if(toupper(line[0])=='Y'&&toupper(line[1])=='E'&&toupper(line[2])=='S')  {
                                                options.pauseonerror=1;
                                        }
                                        else warn("Wrong PAUSEONERROR argument");
                                        break;
                                case 26:// PRINTMONBUFSIZE
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.printmonbufsize=pc;
                                        break;
                                case 27:// PRIORITY
                                        if(toupper(line[0])=='D'&&toupper(line[1])=='Y'&&toupper(line[2])=='N'&&toupper(line[3])=='A'&&toupper(line[4])=='M'&&toupper(line[5])=='I'&&toupper(line[6])=='C') {
                                                options.priority=1;
                                        }
                                        else if(toupper(line[0])=='A'&&toupper(line[1])=='B'&&toupper(line[2])=='S'&&toupper(line[3])=='O'&&toupper(line[4])=='L'&&toupper(line[5])=='U'&&toupper(line[6])=='T'&&toupper(line[7])=='E') {
                                                options.priority=0;
                                        }
                                        else warn("Wrong PRIORITY argument");
                                        break;
                                case 28:// PRIORITY_DISK_IO
                                        if(toupper(line[0])=='N'&&toupper(line[1])=='O')  {
                                                options.priority_disk_io=0;
                                        }
                                        else if(toupper(line[0])=='Y'&&toupper(line[1])=='E'&&toupper(line[2])=='S')  {
                                                options.priority_disk_io=1;
                                        }
                                        else warn("Wrong PRIORITY_DISK_IO argument");
                                        break;
                                case 29:// PROTECTONLY
                                        if(toupper(line[0])=='N'&&toupper(line[1])=='O')  {
                                                options.protectonly=0;
                                        }
                                        else if(toupper(line[0])=='Y'&&toupper(line[1])=='E'&&toupper(line[2])=='S')  {
                                                options.protectonly=1;
                                        }
                                        else warn("Wrong PROTECTONLY argument");
                                        break;
                                case 30:// PROTSHELL
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.protshell=pc;
                                        break;
                                case 31:// RASKDATA
                                        if(toupper(line[0])=='O'&&toupper(line[1])=='T'&&toupper(line[2])=='E') {
                                                options.raskdata=1;
                                        }
                                        else options.raskdata=0;
                                        break;
                                case 32:// REIPL
                                        if(toupper(line[0])=='O'&&toupper(line[1])=='N')  {
                                                options.reipl=1;
                                        }
                                        else if(toupper(line[0])=='O'&&toupper(line[1])=='F'&&toupper(line[2])=='F')  {
                                                options.reipl=0;
                                        }
                                        else warn("Wrong REIPL argument");
                                        break;
                                case 33:// REMOTE_INSTALL_STATE
                                        options.remote_install_state=line[0];
                                        break;
                                case 34:// RESERVEDRIVELETTER
                                        options.reservedriveletter=line[0];
                                        break;
                                case 35:// SETBOOT
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.setboot=pc;
                                        break;
                                case 36:// SOURCEPATH
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.sourcepath=pc;
                                        break;
                                case 37:// STRACE
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.strace=pc;
                                        break;
                                case 38:// SUPPRESSPOPUPS
                                        options.suppresspopups=line[0];
                                        break;
                                case 39:// SWAPPATH
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.libpath=pc;
                                        break;
                                case 40:// SXFAKEHWFPU
                                        if(line[0]=='1') {
                                                options.sxfakehwfpu=1;
                                        }
                                        else options.sxfakehwfpu=0;
                                        break;
                                case 41:// SYSDUMP
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.sysdump=pc;
                                        break;
                                case 42:// THREADS
                                        options.threads=atoi(line);
                                        break;
                                case 43:// TIMESLICE
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.timeslice=pc;
                                        break;
                                case 44:// TRACE
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.trace=pc;
                                        break;
                                case 45:// TRACEBUF
                                        options.tracebuf=atoi(line);
                                        break;
                                case 46:// TRAPDUMP
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.trapdump=pc;
                                        break;
                                case 47:// TRAPLOG
                                        if(!(pc=(char *)malloc((size_t)len)))
                                                error("Memory could not be allocated!");
                                        strcpy(pc,line);
                                        options.traplog=pc;
                                        break;
                                case 48:// TRUEMODE
                                        if(line[0]=='1') {
                                                options.truemode=1;
                                        }
                                        else options.truemode=0;
                                        break;
                                case 49:// VIRTUALADDRESSLIMIT
                                        options.virtualaddresslimit=atoi(line);
                                        break;
                                case 50:// VME
                                        if(toupper(line[0])=='N'&&toupper(line[1])=='O') {
                                                options.vme=0;
                                        }
                                        else options.vme=1;
                                        break;
                                case 51:// WORKPLACE_NATIVE
                                        if(line[0]=='0') {
                                                options.workplace_native=0;
                                        }
                                        else options.workplace_native=1;
                                        break;
                                case 52:// WORKPLACE_PRIMARY_CP
                                        if(line[0]=='1') {
                                                options.workplace_primary_cp=1;
                                        }
                                        else options.workplace_primary_cp=0;
                                        break;
                                case 53:// WORKPLACE_PROCESS
                                        if(toupper(line[0])=='N'&&toupper(line[1])=='O') {
                                                options.workplace_process=0;
                                        }
                                        else options.workplace_process=1;
                                        break;
                                case 54:// WP_OBJHANDLE
                                        options.wp_objhandle=atoi(line);
                                        break;
                        default:
                                        io_printf("Oh, well, this should not occure. Please note down the following to lines:\n%s\n%s\n",options_list[i],line);
                                        break;
                        }
                        return(1);
                }
        }
        return(2);
}

/**********************************************************************
 * cleanup() does what it name says: It cleans up ;-) It frees the    *
 * memory, the program used.                                          *
 **********************************************************************/
int cleanup()
{
int i=0,j;

for(i=0;i<sizeof(type)/sizeof(struct types);i++) {
                  for(j=0;j<type[i].ip;j++){
#ifdef DEBUG
                                         io_printf("%s\n",type[i].sp[j]);
#endif
                                         if(type[i].sp[j]!=NULL)
                                                 free(type[i].sp[j]);
                  }
}
if(options.iopl!=NULL)
        free(options.iopl);

return 0;
}


/**********************************************************************
 * error() gets an error message as the only argument, prints it to   *
 * the screen and ends the programm.                                  *
 **********************************************************************/
void error(char *msg)
{
                  io_printf("failure-exit:\n%s",msg);
                  exit(-1);
}

/**********************************************************************
 * warn() gets an warning message as the only argument, prints it to  *
 * the screen and returns -1.                                         *
 **********************************************************************/
int warn(char *msg)
{
                  io_printf("warning:\n%s\n",msg);
                  return(-1);
}

/* Print string in hex numbers and remove end of line characters. */
void print_str_hex(char *st) {
    int i=0;
    int l=strlen(st);
    for(i=0; i<l; i++) {
        io_printf("0x%x,",((int)st[i]));
        if( ((int)st[i]) == 0xd) {/* End of line char, remove it. */
            st[i] = ' ';
            io_printf("Fixed eol char at (dec) %d.\n", i);
        }
    }
    io_printf("\n");
}

void executeprotshell(opts * options)
{
  io_printf("PROTSHELL execution. PROTSHELL=%s\n", options->protshell);

  int error = 0;
  CORBA_Environment env = dice_default_environment;
  static char error_msg[1024];
  char *ptr = error_msg;
  l4_taskid_t task_ids[l4loader_MAX_TASK_ID];
  l4dm_dataspace_t dummy_ds = L4DM_INVALID_DATASPACE;
  int i;
  static char cmd_buf[1024];
  char *cur_path = searchpath;

  while (cur_path)
    {
      if (strchr(options->protshell, '/'))
        {
          strncpy(cmd_buf, options->protshell, sizeof(cmd_buf));
          cur_path = NULL;
        }
      else
        {
          char *colonpos = strchr(cur_path, ':');

          if (colonpos)
            *colonpos = 0;
          snprintf(cmd_buf, sizeof(cmd_buf), "%s/%s",
                   cur_path, options->protshell);
          if (colonpos)
            {
              *colonpos = ':';
              cur_path = colonpos + 1;
            }
          else
            cur_path = NULL;
        }

// Enable the following macro to show the usage of the L4LOADER_STOP flag
// when starting a new application.
// #define BREAK_START
#ifdef BREAK_START
      if ((error = l4loader_app_open_call(&loader_id, &dummy_ds, cmd_buf,
                                          &tftp_id, L4LOADER_STOP, task_ids,
                                          &ptr, &env)))
        {
          if (error == -L4_ENOTFOUND)
            continue;

          io_printf("  Error %d (%s) loading application\n",
              error, l4env_errstr(error));
          if (*error_msg)
            io_printf("  (Loader says:'%s')\n", error_msg);
          break;
        }

      io_printf("  Successfully initialized task%s ",
          l4_is_invalid_id(task_ids[1]) ? "" : "s");

      for (i=0; i<l4loader_MAX_TASK_ID && !l4_is_invalid_id(task_ids[i]); i++)
        io_printf("%s" l4util_idfmt,
            i>0 ? ", " : "", l4util_idstr(task_ids[i]));

      io_printf(". Going to start them.\n");

      for (i=0; i<l4loader_MAX_TASK_ID && !l4_is_invalid_id(task_ids[i]); i++)
        {
          if ((error = l4loader_app_cont_call(&loader_id, &task_ids[i], &env)))
            io_printf("  Cannot continue task "l4util_idfmt", error %d (%s)\n",
                l4util_idstr(task_ids[i]), error, l4env_errstr(error));
          else
            io_printf("  Task "l4util_idfmt" successfully started.\n",
                l4util_idstr(task_ids[i]));
        }
      break;
#else
      /* The function is defined in 'tudos/l4/pkg/loader/server/src/idl.c'
         as l4loader_app_open_component at line 51. */
      io_printf("cmd_buf in hex:");
        print_str_hex(cmd_buf);
      io_printf("l4loader_app_open_call(loader_id=0x%x ,, cmd_buf=\"%s\", tftp_id=0x%x \n",
                loader_id, cmd_buf, tftp_id);
      if ((error = l4loader_app_open_call(&loader_id, &dummy_ds, cmd_buf,
                                          &tftp_id, 0, task_ids,
                                          &ptr, &env)))
        {
          if (error == -L4_ENOTFOUND)
            continue;

          io_printf("  Error %d (%s) loading application\n",
              error, l4env_errstr(error));
          if (*error_msg)
            io_printf("  (Loader says:'%s')\n", error_msg);
          break;
        }

      io_printf("  Successfully started task%s ",
          l4_is_invalid_id(task_ids[1]) ? "" : "s");

      for (i=0; i<l4loader_MAX_TASK_ID && !l4_is_invalid_id(task_ids[i]); i++)
        io_printf("%s" l4util_idfmt,
            i>0 ? ", " : "", l4util_idstr(task_ids[i]));

      io_printf(".\n");
      break;
#endif
    }

  if (error == -L4_ENOTFOUND)
    io_printf("run: command not found: %s\n", options->protshell);

//  return error;

}



 /**********************************************************************
 * Here everything starts. This is the main function of the           *
 * os2server.                                                         *
 **********************************************************************/
int main(int argc, const char **argv)
{
    int err, ret = 1;
    l4_threadid_t server_id, dm_id;
    l4dm_dataspace_t ds;
    void *addr;
    l4_size_t size;
    int  off = 0;
    CORBA_Environment _env = dice_default_environment;
    const char *servername;
    const char *filename;
    char line[MAXLENGTH]; // here I store the lines I read
    int len=0;          // length of returned line
    char c;

    contxt_init(4096, 1000);

    io_printf("OS2Server started\n");

    if (!names_register("os2server"))
    {
      io_printf("Error registering in name server. Exiting.\n");
      return 1;
    }


    if (parse_cmdline(&argc, &argv,
                      's',"server","file provider name",
                      PARSE_CMD_STRING, "BMODFS", &servername,
                      'n',"name","file name",
                      PARSE_CMD_STRING, "config.sys", &filename,
                      0))
    {
      io_printf("Error command line parsing.\n");
      return 1;
    }

    if (!names_waitfor_name(servername, &server_id, 10000))
    {
        io_printf("Server \"%s\" not found\n", servername);
        return 1;
    }

    /* Initialize the the file provider thread id. */
    tftp_id = server_id;

    dm_id = l4env_get_default_dsm();
    if (l4_is_invalid_id(dm_id))
    {
        io_printf("No dataspace manager found\n");
        return 1;
    }

    /* Initialize the loader_id with the thread id of Loader. */
    if (!names_waitfor_name("LOADER", &loader_id, 30000))
    {
      io_printf("Dynamic loader LOADER not found -- terminating\n");
      return -2;
    }

    init_options();


    if ((err = l4fprov_file_open_call(&server_id,
                                      filename,
                                      &dm_id,
                                      0, &ds, &size, &_env)))
      {
        io_printf("Error opening file \"%s\" at %s: %s\n",
               filename, servername, l4env_errstr(err));
        return 1;
      }

    if ((err = l4rm_attach(&ds, size, 0,
                           L4DM_RO | L4RM_MAP, &addr)))
      {
        io_printf("Error attaching dataspace: %s\n", l4env_errstr(err));
        return 1;
      }


    len=0;
    while (off<size)
    {
      c=((char *)addr+off)[0];
      line[len]=c;
      if (c=='\n')
      {
        line[len]='\0';
        if(!parse(line,len)) error("parse: an error occured\n");
        len=0;
      } else {
        len++;
      }
      off++;
    }


    /* we are set */
    ret = 0;
    if ((err = l4rm_detach(addr)))
      LOG_Error("l4rm_detach(): %s", l4env_errstr(err));

    if ((err = l4dm_close(&ds)))
      LOG_Error("l4dm_close(): %s", l4env_errstr(err));


    io_printf("PROTSHELL=%s\n", options.protshell);

    // Now execute protected shell pointed by PROTSHELL option
    if (options.protshell)
    {
      l4thread_create(executeprotshell, (void *)&options, L4THREAD_CREATE_ASYNC);
      os2server_server_loop(0);
      return 0;
    } else {
      io_printf("PROTSHELL statament not set. OS2Server panic!\n");
    }
        cleanup();

    return ret;
}


