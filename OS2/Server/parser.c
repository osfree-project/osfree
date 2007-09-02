/******************* config.sys parser ***********************
 * Dedicated to JESUS CHRIST, my lord and savior             *
 *                                                           *
 * Author: Sascha Schmidt <sascha.schmidt@asamnet.de         *
 * Version: 0.11                                             *
 * Copyright 2007 by Sascha Schmidt and the osFree Project   *
 * This is free software under the terms of GPL v2           *
 *                                                           *
 * parses the systems config.sys file during bootup          *
 * This software is not yet useable.                         *
 * first one probably using getc and the others must use     *
 * miniFSD                                                   *
 *************************************************************/

#include <unistd.h>
#include <stdlib.h>  /* Libs have to be replaced by uLibC   */
#include <stdio.h>				 
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#define MAXLENGTH 1512	// Maximal line length


//#define DEBUG 


char *skiplist[]={"DEVICE","BASEDEV","IFS","REM "};

// Arrays of pointers; used to store the content of config.sys
//char  *statement[30];
char  *run[30];
char  *call[30];
char  *set[150];

struct types {
	const char *name;	// Name of config.sys command
	const char sep;	// Not used so far, perhaps I will delete it
	char **sp;		// Pointer to the content
	int ip;		// Number of elements in the arrays
}type[]={
//   {"STATEMENT",'=',statement,0},
   {"CALL",'=',call,0},
   {"RUN",'=',run,0},
   {"SET",'=',set,0}
};


/* Structure containing some environment options. For more information*
 * see http://www.os2world.com/goran/cfgtool.htm for example.         */
struct { 
//	boolean autofail;	// YES or NO (default)
	short int buffers;	// 3-100
	short int clockscale; 	// 1,2,4
//	boolean closefiles;	// TRUE
	char codepage[7];	// ppp,sss (Prim. and sec. codepage)
	char * country;		// =XXX,drive:\OS2\SYSTEM\COUNTRY.SYS
	char * devinfo_kbd;	// =KBD,XX,drive:,\OS2\KEYBOARD.DCP
	char * devinfo_vio;	// =SCR,VGA,drive:\OS2\BOOT\VIOTBL.DCP
	char * diskcache;	// =D,LW,32,AC:CD+E
//	boolean dllbasing;		// ON (default) or OF
	char dumpprocess;	// = drive
//	boolean earlymeminit;	// =TRUE (I think we won't need in in osFree)
//	boolean fakeiss;	// =YES (has something to do with drivers, so we won't need it)
//	boolean i13pages;	// =1
	char * ibm_java_options;// =-Dmysysprop1=tcpip -Dmysysprop2=wait
	char * iopl;		// YES (default) or NO (I would like to see NO as default in osFree...) or a comma seperated list of programms allowed to directly work with hardware
	char * iropt;		// =value
//	boolean javanofpk;	// =YES; deals with fixpacks, therefore I think we need to set it to YES (which disables fixpacks in a case)
	char lastdrive;		// =drive
//	boolean ldrstackopt;	// FALSE or TRUE (default) (I don't think we need this setting)
	char * libpath;		// =.;C:\.... (Syntax like PATH)
	short int maxwait;	// 1 to 255, default is 3; only has an effect if PRIORITY=DYNAMIC
	char * memman;		// =s,m,PROTECT,COMMIT where s=SWAP or NOSWAP; m=MOVE or NOMOVE (is provided for OS/2 1.x compatibility; do we need it?!?)
	char * mode[20];	// can exist multiple times => types?!?
	int mode_id;		// helper variable
//	boolean ndwtimer;	// =OFF
//	boolean pauseonerror;	// YES(default) or NO
	char * printmonbufsize;	// =n,n,n (in byte)
//	boolean priority;	// DYNAMIC(default) or ABSOLUTE
//	boolean priority_disk_io;	// YES (default) or NO
//	boolean protectonly;	// YES or NO; do we need it?
	char * protshell;	// =drive:\OS2\PMSHELL.EXE
//	boolean raskdata;	// =OTE; I think we don't need to have it
//	boolean reipl;		// ON or OFF (default)
	char remote_install_state; // =X
	char reservedriveletter;// =driveletter
	char * setboot;		// configures OS/2 bootmanager => I think we can ignore it
	char * sourcepath;	// =path
	char * strace;		// several possible?!?
	char suppresspopups;	// 0 or driveletter
	char * swappath;	// =drive,path,mmm(in kb),nnn (in kb)
//	boolean sxfakehwfpu;	// =1 (do we need it?)
	char * sysdump;		// nothing or /NOPROMPT if set
	int threads;		// =n; number of threads; isn't this handled in the kernel? do we need it?
	char * timeslice;	// =x,y (in milliseconds)
	char * trace;		// =ON, OFF, n
	int tracebuf;		// =n (inkb)
	char * trapdump;	// =value
	char * traplog;		// =value
//	boolean truemode;	// =1 
	int virtualaddresslimit;// =n (in kb)
//	boolean vme;		// =NO
//	boolean workplace_native;	// =0
//	boolean workplace_primary_cp; // =1
//	boolean workplace_process;	// =NO
	int wp_objhandle;	// =n
} options;


char *options_list[]={"AUTOFAIL","BUFFERS","CLOCKSCALE","CLOSEFILES",
"CODEPAGE","COUNTRY","DEVINFO_KBD","DEVINFO_VIO","DISKCACHE","DLLBASING",
"DUMPPROCESS","EARLYMEMINIT","FAKEISS","I13PAGES","IBM_JAVA_OPTIONS","IOPL",
"IROPT","JAVANOFPK","LASTDRIVE","LDRSTACKOPT","LIBPATH","MAXWAIT","MEMMAN",
"MODE","NDWTIMER","PAUSEONERROR","PRINTMONBUFSIZE","PRIORITY",
"PRIORITY_DISK_IO","PROTECTONLY","PROTSHELL","RASKDATA","REIPL",
"REMOTE_INSTALL_STATE","RESERVEDRIVELETTER","SETBOOT","SOURCEPATH","STRACE",
"SUPPRESSPOPUPS","SWAPPATH","SXFAKEHWFPU","SYSDUMP","THREADS","TIMESLICE",
"TRACE","TRACEBUF","TRAPDUMP","TRAPLOG","TRUEMODE","VIRTUALADDRESSLIMIT",
"VME","WORPLACE_NATIVE","WORKPLACE_PRIMARY_CP","WORKPLACE_PROCESS","WP_OBJHANDLE"};

// Function prototypes
int init_options();
int open_config_sys();
int close_config_sys(int);
int parse(char *, int);
int fgetline(int,char *);
void error(char *);
int print_tree();
int cleanup();


/**********************************************************************
 * Here everything starts. This is the main function of the           *
 * config.sys-parser.                                                 *
 **********************************************************************/
void main(void)
{
	int fd; 		// file descriptor
	char line[MAXLENGTH]; // here I store the lines I read
	int len=0;		// length of returned line
	
	init_options();
	fd=open_config_sys();
	while((len=fgetline(fd,line)))
	{
		if(!parse(line,len)) error("parse: an error occured\n");
	}
		  
	close_config_sys(fd);
	if(!print_tree()) error("print_tree: an error occured\n");
	cleanup();

	exit(EXIT_SUCCESS);
}

/**********************************************************************
 * init_options() sets the default values in the options structure    *
 **********************************************************************/

int init_options()
{
	char * p;

	if(!(p=(char *)malloc(3))) error("main: memory could not be allocated!");
	strcpy(p,"YES");
	options.iopl=p;
	options.maxwait=3;
	options.mode_id=0;
	return(1);
}


/**********************************************************************
 * opens the config.sys file and returns a file-descriptor; must be   *
 * changed to use a miniFSD function, not open                        *
 **********************************************************************/
int open_config_sys()
{
		  int fd;

		  if((fd = open("config.sys",0)) == -1)
			 error("\n\nFATAL: config.sys could not be found\n");
		  return fd;
}

/**********************************************************************
 * closes the filedescriptor pointing to config.sys; must be changed  *
 * to use a miniFSD function instead of close                         *
 **********************************************************************/
int close_config_sys(int fd)
{
					 close(fd);
					 return(0);
}

/**********************************************************************
 * print_tree() is some kind of debug or test function. It only       *
 * prints out the part of config.sys we use. It has to be replaced by *
 * something useful.                                                  *
 **********************************************************************/
int print_tree()
{
	int x=1, i=0;
	  
	printf("Config tree:\n\n");
		  
	for(i=0;i<sizeof(type)/sizeof(struct types);i++) {	
		printf("%s:\n",type[i].name);
		for(x=1;x-1<type[i].ip;x++) {
			printf("\t%s\n",type[i].sp[x-1]); 
		}
	}
	return(1);
}

/**********************************************************************
 * fgetline() reads the content of the file belonging to the file     *
 * descriptor fd and writes it to line.                               *
 **********************************************************************/
int fgetline(int fd, char *line)
{
	int len=0;
	char c;
		  
	while(read(fd,&c,1)> 0 && c != '\n' /*&& c != '\r'*/)
	{
		*line++=c;
		len++;
	}
	*line='\0';
	return(len==0?(int)NULL:len+1);
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
	char c;
	char *pc=NULL;
// 	for(i=0;i<sizeof(skiplist)/sizeof(char *);i++) {
//                for(count=0;count<strlen(skiplist[i]);count++){
                /* Compares line and type[i].name and breaks on a *
		 * difference */
//			if(toupper(line[count])!=skiplist[i][count]) {
//				break;
//			}
//		}
//		if((c=toupper(line[count]))&& c != '=' && c != ' ' && c !='\n') break; 
//		if(count==strlen(skiplist[i])) {
#ifdef DEBUG
//			printf("Skipping: %s\n",line);
#endif			
//			return(2);
//		}
//	}
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
//		if((c=toupper(line[count-1]))&& c!='=') 
//			{
//			printf("Hier: %c",c);
//			break; 
//			}
		if(count==strlen(options_list[i])&& toupper(line[count])=='=') {
			count++;
			strcpy(line,line+count);
			len-=count;
			
			switch(i) {
				case 0:	// AUTOFAIL
					// BOOLEAN		
					break;
				case 1:	// BUFFERS
					options.buffers=(short int)atoi(line);
					break;
				case 2:	// CLOCKSCALE
					helper=atoi(line);
					if(helper==1 || helper==2 || helper==4)
						options.clockscale=(short int)helper;
					else error("Syntax error in config.sys line.");
					break;
				case 3:	// CLOSEFILES
					// BOOLEAN
					break;
				case 4:	// CODEPAGE
					for(helper=0;helper<7;helper++) {
						options.codepage[helper]=line[helper];
					}
					break;
				case 5:	// COUNTRY
					if(!(pc=(char *)malloc((size_t)len)))
						error("Memory could not be allocated!");
					strcpy(pc,line);
					options.country=pc;
					break;
				case 6:	// DEVINFO_KBD
					if(!(pc=(char *)malloc((size_t)len)))
						error("Memory could not be allocated!");
					strcpy(pc,line);
					options.devinfo_kbd=pc;
					break;
				case 7:	// DEVINFO_VIO
					if(!(pc=(char *)malloc((size_t)len)))
						error("Memory could not be allocated!");
					strcpy(pc,line);
					options.devinfo_vio=pc;
					break;
				case 8:	// DISKCACHE
					if(!(pc=(char *)malloc((size_t)len)))
						error("Memory could not be allocated!");
					strcpy(pc,line);
					options.diskcache=pc;		
					break;
				case 9:	// DLLBASING
					// BOOLEAN
					break;
				case 10:// DUMPPROCESS
					options.dumpprocess=line[0];
					break;
				case 11:// EARLYMEMINIT
					// BOOLEAN
					break;
				case 12:// FAKEISS
					// BOOLEAN
					break;
				case 13:// I13PAGES
					// BOOLEAN
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
					// BOOLEAN
					break;
				case 18:// LASTDRIVE
					options.lastdrive=line[0];
					break;
				case 19:// LDRSTACKOPT
					// BOOLEAN
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
					// BOOLEAN
					break;
				case 25:// PAUSEONERROR
					// BOOLEAN
					break;
				case 26:// PRINTMONBUFSIZE
					if(!(pc=(char *)malloc((size_t)len)))
						error("Memory could not be allocated!");
					strcpy(pc,line);
					options.printmonbufsize=pc;
					break;
				case 27:// PRIORITY
					// BOOLEAN
					break;
				case 28:// PRIORITY_DISK_IO
					// BOOLEAN
					break;
				case 29:// PROTECTONLY
					// BOOLEAN
					break;
				case 30:// PROTSHELL
					if(!(pc=(char *)malloc((size_t)len)))
						error("Memory could not be allocated!");
					strcpy(pc,line);
					options.protshell=pc;
					break;
				case 31:// RASKDATA
					// BOOLEAN
					break;
				case 32:// REIPL
					// BOOLEAN
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
					// BOOLEAN
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
					break;
				case 49:// VIRTUALADDRESSLIMIT
					options.virtualaddresslimit=atoi(line);
					break;
				case 50:// VME
					// BOOLEAN
					break;
				case 51:// WORKPLACE_NATIVE
					// BOOLEAN
					break;
				case 52:// WORKPLACE_PRIMARY_CP
					// BOOLEAN
					break;
				case 53:// WORKPLACE_PROCESS
					// BOOLEAN
					break;
				case 54:// WP_OBJHANDLE
					options.wp_objhandle=atoi(line);
					break;
			default:
					printf("Oh, well, this should not occure. Please note down the following to lines:\n%s\n%s\n",options_list[i],line);
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
					 printf("%s\n",type[i].sp[j]);
#endif
					 if(type[i].sp[j]!=NULL)
						 free(type[i].sp[j]);
		  }
}

return 0;
}


/**********************************************************************
 * error() gets an error message as the only argument, prints it to   *
 * the screen and ends the programm.                                  *
 **********************************************************************/
void error(char *msg)
{
		  printf("failure-exit:\n%s",msg);
		  exit(-1);
}


