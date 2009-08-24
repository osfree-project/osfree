/* LTOOLS
   Programs to read, write, delete and change Linux extended 2 and ReiserFS filesystems under DOS

   Module rfs.c
   Wrapper routines for ReiserFS, uses RFSTOOL copyrighted by Gerson Kurz.

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C
 */

#define RFSTOOL_EXTENDED					//Using our own RFSTOOL version

#ifdef __WATCOM__
#include "porting.h"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext2.h"
#include "proto.h"
#include "ldir.h"

//#if defined(__WIN32__)					//Currently, ReiserFS support is for Win 32 only

#ifdef NEWOUT
	extern FILE *STDERR;
	extern FILE *STDOUT;
#else
	#ifdef UNIX
		#define STDOUT stdout
		#define STDERR stdout
	#else
		extern FILE *STDERR;
		extern FILE *STDOUT;
	#endif
#endif

extern enum { NO, YES } quiet;
extern enum { LDIR, LREAD, LWRITE, LTEST } modus;
extern enum { CHANGE, DEL, COPY, REN } u_modus;

extern unsigned int disk_no;
extern unsigned int part_no;
extern char source[], destin[];
extern int isNT;
extern int isWin;
extern char Fmode[32], Uid[32], Gid[32];


#ifndef UNIX
#define popen 	_popen
#define pclose 	_pclose
#endif

#define rfsStartColumnLinuxFileName 	49			//rfstool start column of filenames
#define rfsStartLinuxFileSize 		12			//rfstool start column of filenames

#define rfsERROR_SUCCESS		 0			//Error definitions for rfstool
#define rfsERROR_PARAMETER_ERROR	-1
#define rfsERROR_READ_ONLY_ERROR	-2
#define rfsERROR_COULD_NOT_OPEN_RFSTOOL	-3
#define rfsERROR_COULD_NOT_READ_RFSTOOL -4
#define rfsERROR_FIRST_LINE_NOT_FOUND	-5


#if !defined(UNIX)
#ifdef __BORLANDC__
FILE *popen(const char *command, const char *mode)
{   strcat((char*) command, ">li");
    system(command);
    return fopen("li","rt+");
}

int pclose(FILE *fd)
{   int i;
    i=fclose(fd);
    system("if exist li del li");
    return i;
}
#endif
#endif

//Get RFSTOOL version information
int rfsVersion(void)
{   FILE *fIn;
    char buffer[256];
    int i;

#ifndef UNIX
#if 0
    if (!(isWin || isNT))
    {   fprintf(STDOUT, "RFSTOOL: Sorry, ReiserFS not supported under DOS\n");
   	return rfsERROR_SUCCESS;
    }
#endif
#endif

#if defined(UNIX)
    if ((fIn=popen("rfstool -?","r"))==NULL)			//Open rfstool input via a pipe
#else
    if ((fIn=popen("rfstool -?","rt"))==NULL)			//Open rfstool input via a pipe
#endif
    {   DebugOut(64,"Could not open RFSTOOL");
    	return rfsERROR_COULD_NOT_OPEN_RFSTOOL;
    }
    for (i=0; i < 3; i++)
    {   fgets(buffer, sizeof(buffer), fIn);
        fprintf(STDOUT, "%s", buffer);
    }
    fprintf(STDOUT, "------------------------------------------------------------------------------\n");
    pclose(fIn);
    if (quiet==YES)
    {	fclose(STDOUT);
        printf("---Close STDOUT in rfsVersion\n");
    }
    return rfsERROR_SUCCESS;
}

//Get ReiserFS partition info
int rfsPart(void)
{   FILE *fIn;
    char buffer[256], *p, *q;
    int i;

#ifndef UNIX
#if 0
    if (!(isWin || isNT))
    {   fprintf(STDOUT, "RFSTOOL: Sorry, ReiserFS not supported under DOS\n");
   	return rfsERROR_SUCCESS;
    }
#endif
#endif

#if defined(UNIX)
    if ((fIn=popen("rfstool autodetect","r"))==NULL)			//Open rfstool input via a pipe
#else
    if ((fIn=popen("rfstool.exe autodetect","rt"))==NULL)		//Open rfstool input via a pipe
#endif
    {   DebugOut(64,"Could not open RFSTOOL");
    	return rfsERROR_COULD_NOT_OPEN_RFSTOOL;
    }
    fprintf(STDOUT, "# RFSTOOL infos --------------------------------------------------------------\n");
    while (fgets(buffer, sizeof(buffer), fIn)!=NULL)
    {   if (strstr(buffer,"is a ReiserFS"))
        {   p = strstr(buffer,"Drive");
            q = strstr(buffer,"Partition");
            fprintf(STDOUT, "##### Disk %3u = /dev/rd%c\n",atoi(p+5)+128, atoi(p+5)+'a');
            fprintf(STDOUT, "# %2u Type:Linux ReiserFS ", atoi(q+10));
        } else if (strstr(buffer,"Partition Size"))
        {   p = strstr(buffer,"(");
            fprintf(STDOUT, "%uMB                                         \n", atoi(p+1));
        }

    }
    fprintf(STDOUT, "------------------------------------------------------------------------------\n");
    pclose(fIn);
    if (quiet==YES)
    {	fclose(STDOUT);
        printf("---Close STDOUT in rfsPart\n");
    }
    return rfsERROR_SUCCESS;
}


//Convert RFSTOOL's output into LDIR's format
int rfsParseOutput(char *command, int argc, char* argv[])
{   FILE *fIn;
    char buffer[256], *p, *q, *s;
    int i=0;

    if ((fIn=popen(command,"r"))==NULL)				//Execute rfstool and connect to its output via a pipe
    {	DebugOut(64,"Could not open RFSTOOL");
    	return rfsERROR_COULD_NOT_OPEN_RFSTOOL;
    }
    if (quiet)
    {  	fclose(STDOUT);
    	printf("---Reopening STDOUT/STDERR in rfsParseOutput\n");
#ifdef NEWOUT
	STDOUT = freopen("zzz.zwz", "w+", STDOUT);
#else
    	STDOUT = fopen("zzz.zwz", "w+");
#endif
	STDERR = STDOUT;
    }

    while (fgets(buffer, sizeof(buffer), fIn)!=NULL)		//Skip lines, but not more than 8
    {
//    	fprintf(STDOUT, ">>>>>%s", buffer);
    	if (!strncmp(buffer,"d",1) || !strncmp(buffer,"-",1))
    	    break;
    	if (i++ > 8)
    	{   DebugOut(64,"Could not find begin of RFSTOOL output");
    	    return rfsERROR_FIRST_LINE_NOT_FOUND;
    	}
    }

//  drwxr-xr-x    0    0     12288 Sun Mar 10 11:00:57 2002 lost+found

    do
    {	//fprintf(STDOUT,">>>>>buffer=%s<<<<<\n",buffer);

    	if (!strncmp(buffer,"ERROR",5))				//Copy error messages direct to output
    	{   fprintf(STDOUT,   "Executing LTOOLS   command ");
    	    for (i=0; i < argc; i++)
    	    	fprintf(STDOUT,"%s ", argv[i]);
    	    fprintf(STDOUT, "\nERROR: Executing RFSTOOL command %s\n", command);
    	    fprintf(STDOUT, "%s", buffer);
    	    continue;
    	} else if (!strncmp(buffer,"\n",1))			//Skip empty lines
    	{   continue;
	} else if (!strncmp(buffer," ",1))			//Skip lines beginning with ' '
	{   continue;
	} else if (strstr(buffer,"done"))
	{   rewind(STDOUT);
	    break;
	}
								//Mark . and .. as directories
    	if ((!strncmp(buffer,"----------",10)) && (buffer[rfsStartColumnLinuxFileName]=='.'))
    	    buffer[0]='d';

    	if ((q = strtok(buffer," \n"))==NULL)			//Rights
	{   continue;
	}
	fprintf(STDOUT, "%s", q);

#ifdef RFSTOOL_EXTENDED
    	if ((q = strtok(NULL," \n"))==NULL)			//UID
	{   continue;
	}
	fprintf(STDOUT, "%5s", q);

    	if ((q = strtok(NULL," \n"))==NULL)			//GID
	{   continue;
	}
	fprintf(STDOUT, "%5s", q);
#else
	fprintf(STDOUT, "    0    0");				//rfstool does not provide UID/GID, display as 0
#endif

	if ((q = strtok(NULL," \n"))==NULL)			//Size
	{   continue;
	} else if ((s = strstr(q, "<DIR>"))!=NULL)
        {   s[0]='_';						//-- substitute <DIR> by _DIR_, because it
            s[4]='_';						//-- causes problems for lreadsrv's HTML output
        }
	fprintf(STDOUT, " %9s", q);

	if ((q = strtok(NULL," \n"))==NULL)			//Name of Day
	{   continue;
	}
	fprintf(STDOUT, " %3s", q);

	if ((q = strtok(NULL," \n"))==NULL)			//Name of Month
	{   continue;
	}
	fprintf(STDOUT, " %3s", q);

	if ((q = strtok(NULL," \n"))==NULL)			//Day
	{   continue;
	}
	fprintf(STDOUT, " %2s", q);

	if ((q = strtok(NULL," \n"))==NULL)			//Time
	{   continue;
	}
	fprintf(STDOUT, " %8s", q);

	if ((q = strtok(NULL," \n"))==NULL)			//Year
	{   continue;
	}
	fprintf(STDOUT, " %4s", q);

	if ((q = strtok(NULL," \n"))==NULL)			//Filename
	{   continue;
	}
	fprintf(STDOUT, " %s", q);

//printf("\n***Filename:>>>%s<<<\n", q);

	if ((q = strtok(NULL,"\n"))==NULL)			//Link-Target or rest of filename, if filename contains blanks
	{   fprintf(STDOUT, "\n");
	} else
	{   if (q[strlen(q)-1]==' ')				//--- Remove trailing blanks
	    	q[strlen(q)-1]= 0;
	    fprintf(STDOUT, " %s\n", q);
	}
//if (q!=NULL) printf("\n***Link:>>>%s<<<\n", q);

    } while (fgets(buffer, sizeof(buffer), fIn)!=NULL);

    pclose(fIn);
    return rfsERROR_SUCCESS;
}

int rfsMain(int argc, char* argv[])
{   int i;
    char command[256] = "rfstool ";
    char temp[256];

#ifndef UNIX
#if 0
    if (!(isWin || isNT))
    {   fprintf(STDOUT, "RFSTOOL: Sorry, ReiserFS not supported under DOS\n");
   	return rfsERROR_SUCCESS;
    }
#endif
#endif

    if (modus==LDIR)
    {	strcat(command,"LS ");
    } else if (modus==LREAD)
    {	strcat(command,"CP ");
    } else if (modus==LWRITE)
    {
#ifdef RFSTOOL_EXTENDED
        if (u_modus==CHANGE)
	{   strcat(command,"change ");
	} else
    	{   fprintf(STDERR, "ERROR: ReiserFS write support is for changing UID, GID and file access rights only - sorry\n");
	    return rfsERROR_READ_ONLY_ERROR;
	}
#else
    	fprintf(STDERR, "ERROR: ReiserFS support is read only - sorry\n");
	return rfsERROR_READ_ONLY_ERROR;
#endif
    } else
    {	fprintf(STDERR, "ERROR: RFSTOOL parameter error - LTOOLS internal error\n");
	return rfsERROR_PARAMETER_ERROR;
    }

#ifndef UNIX
    sprintf(temp,"-p%u.%u ",disk_no-128, part_no);
#else
//    sprintf(temp,"-p/dev/hd%c%u ",disk_no-128+'a',part_no);
    sprintf(temp,"-p%u.%u ",disk_no-128, part_no);
#endif
    strcat(command,temp);

    if (modus==LDIR)
    {	sprintf(temp,"\"%s\" ",source);
    	strcat(command,temp);
    } else if (modus==LREAD)
    {	sprintf(temp,"\"%s\" \"%s\" ",source, destin);
    	strcat(command,temp);
#ifdef RFSTOOL_EXTENDED
    } else if (modus==LWRITE)
    {	if (strlen(Uid)>0)
    	{   sprintf(temp, "-u%s ", Uid);
    	    strcat(command,temp);
        }
        if (strlen(Gid)>0)
    	{   sprintf(temp, "-g%s ", Gid);
    	    strcat(command,temp);
        }
        if (strlen(Fmode)>0)
    	{   sprintf(temp, "-f%s ", Fmode);
    	    strcat(command,temp);
        }
    	if (u_modus==CHANGE)
    	{   sprintf(temp,"\"%s\"",source);
    	    strcat(command,temp);
    	} else
    	{   sprintf(temp,"\"%s\" \"%s\"",source, destin);
    	    strcat(command,temp);
    	}
#endif
    }
    DebugOut(64,"Executing command>>>%s<<<\n",command);

    if ((i=rfsParseOutput(command, argc, argv))!=0)
	fprintf(STDERR, "ERROR: Error %d occurred when executing RFSTOOL command %s\n", i, command);

    if (quiet==YES)
    {	fclose(STDOUT);
        printf("---Close STDOUT in rfsMain\n");
    }
    return i;
}

//#endif
