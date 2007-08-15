/******************* config.sys parser ***********************
 *		Dedicated to JESUS CHRIST, my lord and savior			 *
 *																				 *
 *		parses the systems config.sys file during bootup       *
 *		read, open and close still have to be redone, the 		 *
 *		first one probably using getc and the others must use  *
 *		miniFSD																 *
 *************************************************************/

#include <unistd.h>
#include <stdio.h>		/* Only for testing purpose 				*/
#include <stdlib.h> 		/*	To be replaced by miniFSD-functions */
#include <string.h>
#define BUFSIZE 4096
#define MAXLENGTH 1512

struct ct{
	char  basedev[50][255];
	char  device[50][255];
	char  ifs[20][255];
	char  env[50][255];
//	struct {
//		char key[50][255];
//		char value[50][1255];
//	} keys;
} config_tree;

int t0=0,t1=0,t2=0,t3=0;
struct types {
	const char *name;
	const char sep;
	char *sp;
	int ip;
}type[]={
   {"BASEDEV",'=',config_tree.basedev,0},
   {"IFS",'=',config_tree.ifs,0},
   {"DEVICE",'=',config_tree.device,0},
   {"SET",'=',config_tree.env,0}
};


int open_config_sys();
int close_config_sys(int);
int parse(char *, int);
int fgetline(int,char *);
void error(char *);
int print_tree();

void main(void)
{
	int fd;	/* file descriptor */
	char line[MAXLENGTH];
	int len=0;	/* length of returned line */

	fd=open_config_sys();
	while((len=fgetline(fd,line)))
	{
#ifdef DEBUG
		printf("line length: %d\n",len);
#endif
		if(!parse(line,len)) error("parse: an error occured\n");
	}
	close_config_sys(fd);
	if(!print_tree()) error("print_tree: an error occured\n");
}


/* opens the config.sys file and returns a file-descriptor; must be
 * changed to use a miniFSD function, not open */
int open_config_sys()
{
	int fd;

	if((fd = open("config.sys",0)) == -1)
		error("\n\nFATAL: config.sys could not be found\n");
	return fd;
}

/* closes the filedescriptor pointing to config.sys; must be changed to
 * use a miniFSD function instead of close*/
int close_config_sys(int fd)
{
		close(fd);
		return(0);
}


int print_tree()
{
	int x;
	
	printf("Config tree:\n\n");
	for(x=1;x<type[0].ip;x++) {
//		if(!*(config_tree.basedev+x-1)) return x;
		printf("%s\n\r",config_tree.basedev[x-1]);
	}
	return(1);
}

int fgetline(int fd, char *line)
{
	int len=0;
	char c;
	
	while(read(fd,&c,1)> 0 && c != '\n')
	{
		*line++=c;
		len++;
	}
	*line++='\0';
	return(len==0?(int)NULL:len-1);
}




/* The main parse function; gets a line as argument and returns 1 on
 * success and NULL on error */
int parse(char line[], int len)
{
	int count=0,i,x,y=0;
	char * pc;

	
	for(i=0;i<4;i++) {
		for(count=0;count<strlen(type[i].name);count++)
		{
			if(line[count]!=type[i].name[count]) {
#ifdef DEBUG
				printf("Unterschied an Stelle %d\n",count);
#endif
				break;
			}
		}
		if(count==strlen(type[i].name)) {
#ifdef DEBUG			
			printf("%s:	%s\n",type[i].name,line);
#else
//			printf("%s\n",line);
#endif
			x=++count;
			y=type[i].ip;
			while(line[x]) {
				pc=&type[i].sp[y*sizeof(void*)];
				pc[x-count]=line[x];
				x++;
			}
			pc[x-count]='\0';
//			printf("%s\n",&type[i].sp[y-1]);
//			printf("%s\n",&type[i].sp[y]);
			type[i].ip++;

			// type[i].sp++;
		}
		x=0;
	}
	return(1);
}

void error(char *msg)
{
	printf("failure-exit:\n%s",msg);
	exit(-1);
}


