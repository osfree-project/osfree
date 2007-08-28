/******************* config.sys parser ***********************
 * Dedicated to JESUS CHRIST, my lord and savior             *
 *                                                           *
 * Author: Sascha Schmidt <sascha.schmidt@asamnet.de         *
 * Version: 0.1                                              *
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
#define MAXLENGTH 1512	// Maximal line length


//#define DEBUG 


// Arrays of pointers; used to store the content of config.sys
char  *statement[30];
char  *run[30];
char  *call[30];
char  *set[150];

struct types {
		  const char *name;	// Name of config.sys command
		  const char sep;	// Not used so far, perhaps I will delete it
		  char **sp;		// Pointer to the content
		  int ip;		// Number of elements in the arrays
}type[]={
   {"STATEMENT",'=',statement,0},
   {"CALL",'=',call,0},
   {"RUN",'=',run,0},
   {"SET",'=',set,0}
};



// Function prototypes
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
	char *pc=NULL;
  
	for(i=0;i<sizeof(type)/sizeof(struct types);i++) {
		for(count=0;count<strlen(type[i].name);count++){
	 	/* Compares line and type[i].name and breaks on a
	 	 * difference */
			if(line[count]!=type[i].name[count]) {
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


