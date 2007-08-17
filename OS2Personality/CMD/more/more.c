/* "A better More" version 1.1
   (C) 1993 Stephen M. Lacy

   This program still needs a bit of refinement and addition of options,
   although its kind of nice for now.

   Revision history:

   Version 1.0: Well, sort of. This was used by only me for about two
   days.  Bugs: Defaulted to 80x24, and printed the last line twice

   Version 1.1: First release version.  The 0-9 keys were added, along, 
   with the 'q' key to quit.  The last line problem was fixed, and I added
   some code to see how big the window was, it used to default to 80x24.

   Version 2.0: Completely ignores carriage return characters, but on a
   line feed, does both line feed and carriage return.

   KNOWN BUGS:

   Version 1.1: The disappering cursor thingy.  Icky formatting issues.
   

   */

/* #define INCL_DOS */
#define INCL_VIO
#define INCL_KBD
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _dllist {
	char *string;
	struct _dllist *next,*previous;
} dllist, *pdllist, **ppdllist;


#define CONTROL_S 19
#define PROMPT "-- MORE v2.0á (C)1993 --"
#define EOF_STRING "-- EOF --"
#define CELLSIZE 2
#define DEFAULT_SCROLLUP 4
#define DEFAULT_TABSIZE 4
#define DEFAULT_ATTRIB 0x07
#define DEFAULT_BOLD 0x0f
#define DEFAULT_PROMPT 0x1e
#define DEFAULT_UNDERSCORE 0x0e

#define MORE_EOF 1

int tabsize,options,scrollup;
CHAR bright[2],attrib[2],blankcell[2],colorcell[2],underscore,bold;

void add_dllist(ppdllist first, ppdllist prev, ppdllist new) {
	if (*prev) {
		(*new)->next=(*prev)->next;
		(*new)->previous=(*prev);
		(*prev)->next=(*new);
		(*prev)=(*new);
	}
	else {
		(*new)->next=NULL;
		(*new)->previous=NULL;
		*prev=*new;
		*first=*new;
	}
}

char *get_single_line(FILE *input,int width) {
	char *result,readin;
	int counter,j,end,reread;
	
	result=(char*)malloc((width+1)*2*sizeof(char));
	for (counter=0;counter<width*2;counter++) {
		result[counter++]=' ';
		result[counter]=attrib[0];
	}
	counter=0;
	end=FALSE;
	reread=TRUE;
	while(counter<width*2 && !end) {
		if (reread) readin=fgetc(input);
		switch (readin) {
		case '\t':
			for (j=0;j<tabsize;j++) {
				result[counter]=' ';
				counter+=CELLSIZE;
			}
			break;
		case '\b':
			readin=fgetc(input);
			switch(result[counter-CELLSIZE]) {
			case '_':
				result[counter-CELLSIZE+1]=underscore;
				break;
			default:
				result[counter-CELLSIZE+1]=bold;
				break;
			}
			result[counter-CELLSIZE]=readin;
			break;
		case '\r':
			break;
		case '\n':
			end=TRUE;
			break;
		default:
			result[counter]=readin;
			counter+=CELLSIZE;
			break;
		}
	}
	return(result);
}

int get_screenful(FILE *input,
				  ppdllist first,
				  ppdllist after,
				  int width,
				  int height) {
	int counter;
	char *temp;
	pdllist new_element;
	
	counter=0;
	while (counter<height && !feof(input)) {
		new_element=(pdllist)malloc(sizeof(dllist));
		new_element->string=get_single_line(input,width);
		add_dllist(first,after,&new_element);
		counter++;
	}
	return(counter);
}

void display_screenful(pdllist first,int height,int width, int read) {
	int i;
	char *display;
	pdllist current;
	int counter,j;
	USHORT row,col;

	VioGetCurPos(&row,&col,0);
	display=(char*)malloc(2*(height+1)*(width+1)*sizeof(char));
	current=first;
	counter=0;
	j=0;
	while (current && j<height ) {
		memcpy((&display[counter]),current->string,width*2);
		counter+=width*2;
		current=current->next;
		j++;
	}

	if (-read==height) read=-read;
	if (read<0) {
		read=-read;
		VioScrollDn(0,0,height-1,width,read,blankcell,0);
		VioWrtCellStr(display,width*read*2,0,0,0);
	} else  if (row+read<height) {
		VioSetCurPos(row+read,0,0);
		VioWrtCellStr(display,width*read*2,row,0,0);
	} else if (row+read>height && read<height) {
		if (first->previous!=NULL) display+=width*2*(height-read);
		VioScrollUp(0,0,height,width,abs(read-(height-row)),blankcell,0);
		VioWrtCellStr(display,width*read*2,height-read,0,0);
		VioSetCurPos(height,0,0);
	} else {
		VioWrtCellStr(display,width*height*2,0,0,0);
		VioSetCurPos(height,0,0);
	}
	free(display);
}

int backup(ppdllist which, int n) {
	int i=0;
	while ((*which)->previous && n) {
		(*which)=(*which)->previous;
		n--;
		i++;
	}
	return(i);
}

void forward_search(ppdllist list,
					ppdllist end_of_list,
					FILE *input,
					char *string,
					int rows,
					int cols,
					int *offscreen) {
	char expanded[600];
	INT counter,i;
	pdllist new_element;

	counter=0;
	for (i=0;i<512;) {
		expanded[i++]=string[counter++];
		expanded[i++]=attrib[0];
	}
	while(1) {
		if (strstr((*list)->string,expanded)) break;
		if (feof(input) && (*end_of_list)->next==NULL) break;
		if ((*list)->next) {
			(*list)=(*list)->next;
			(*offscreen)--;
		} else {
			new_element=(pdllist)malloc(sizeof(dllist));
			new_element->string=get_single_line(input,cols);
			add_dllist(list,end_of_list,&new_element);
		}
	}
	backup(end_of_list,rows);
}

void more_file(FILE *input) {
	PVIOMODEINFO vmi;
	KBDKEYINFO kbdi;
	SHORT rows,cols;
	CHAR string[255],searchbuffer[255],*tempstr;
	CHAR *display;
/*	CHAR searchstring[255],previous_search[255]; */
	STRINGINBUF stringbuf;
	pdllist whole_list,current,end_of_list;
	int i,val,read,lines,offscreen;
	USHORT rc,upval,r,c;
	KBDINFO kbdinfo;

	KbdGetStatus(&kbdinfo,0);
	vmi=(PVIOMODEINFO)malloc(sizeof(VIOMODEINFO));
	vmi->cb=sizeof(VIOMODEINFO);
	rc=VioGetMode(vmi,0);
	if (rc!=0) {
		printf("Error doing VioGetMode! %d\n",rc);
		exit(0);
	}
	rows=vmi->row-1;
	cols=vmi->col;
	whole_list=NULL;
	end_of_list=NULL;
	lines=rows;
	offscreen=0;
	while (options&MORE_EOF?1:!feof(input)) {
		if (lines-offscreen>0) {
			read=get_screenful(input,
							   &whole_list,
							   &end_of_list,
							   cols,
							   lines-offscreen);
			if (read!=lines-offscreen) 
				backup(&whole_list,lines-offscreen-read);
			display_screenful(whole_list,rows,cols,read);
		} else {
			display_screenful(whole_list,rows,cols,lines);
		}
		if (lines>0) offscreen-=lines;
		if (offscreen<0) offscreen=0; 
		if (options&MORE_EOF?1:!feof(input))  {
			if (feof(input)) 
				VioWrtCharStrAtt(EOF_STRING,strlen(EOF_STRING),
								 rows,0,bright,0);
			else VioWrtCharStrAtt(PROMPT,strlen(PROMPT),rows,0,bright,0);
			KbdCharIn(&kbdi,0,0);
			if (kbdi.fbStatus & 0x0002) {
				switch (kbdi.chScan) {
				case 72:
					/* Up arrow */
					lines=scrollup;
					offscreen+=lines=backup(&whole_list,lines);
					lines=-lines;
					break;
				case 80:
					/* Down Arrow */
					lines=scrollup;
					break;
				case 73:
					/* Page up */
					lines=rows;
					offscreen+=lines=backup(&whole_list,lines);
					lines=-lines;
					break;
				case 81:
					/* Page Down */
					lines=rows;
					break;
				default:
					if (feof(input)) return;
					break;
				}
			} else 
				switch(kbdi.chChar) {
/*				case '/':
					lines=0;
					VioWrtNCell(colorcell,cols,rows,0,0);
					stringbuf.cb=255;
					kbdinfo.fsMask|=KEYBOARD_ASCII_MODE;
					kbdinfo.fsMask&=~KEYBOARD_BINARY_MODE;
					KbdSetStatus(&kbdinfo,0);
					KbdStringIn(searchstring,&stringbuf,0,0);
					searchstring[strlen(searchstring)-1]='\0';
					kbdinfo.fsMask&=~KEYBOARD_ASCII_MODE;
					kbdinfo.fsMask|=KEYBOARD_BINARY_MODE;
					KbdSetStatus(&kbdinfo,0);
					if (strcmp(searchstring,"\0D"))
						forward_search(&whole_list,
									   &end_of_list,
									   input,
									   searchstring,
									   rows,
									   cols,
									   &offscreen);
					else if (strcmp(previous_search,"\0D"))
						forward_search(&whole_list,
									   &end_of_list,
									   input,
									   previous_search,
									   rows,
									   cols,
									   &offscreen);
					strcpy(previous_search,searchstring);
					break;
*/					
				case 'f':
				case 'F':
					lines=scrollup;
					break;
				case 'b':
				case 'B':
					lines=scrollup;
					offscreen+=lines=backup(&whole_list,lines);
					lines=-lines;
					break;
				case 13:
					lines=scrollup;
					break;
				case 'n':
				case 'N':
					while (input==stdin && !feof(input)) fgetc(input);
					fclose(input);
					VioWrtNCell(blankcell,cols,rows,0,0);
					KbdFlushBuffer(0);
					return;
					break;
				
				case 'q':
				case 'Q':
					while (input==stdin && !feof(input)) fgetc(input);
					fclose(input);
					VioWrtNCell(blankcell,cols,rows,0,0);
					KbdFlushBuffer(0);
					return;
				default:
					lines=rows;
					if (feof(input)) return;
					break;
				}
			for (i=0;i<lines && whole_list->next;i++)
				whole_list=whole_list->next;
		}
	}
	KbdFlushBuffer(0);
}

char *strupper(char *string) {
	int i;
	char *original;
	original=string;
	while (*string) {
		if (*string>='a' && *string<='z')
			*string-='a'-'A';
		string++;
	}
	return(original);
}

void main (int argc, char *argv[]) {
	FILE *input;
	USHORT row, col;
	pdllist file_list,current,new;
	HDIR hdirDir;
	ULONG file_attrib,searchCount,infoLevel;
	USHORT rc;
	FILEFINDBUF3 ffbData;
	CHAR prompt[128];
	CHAR result[255];
	PSZ temp;
	KBDKEYINFO kbdi;
	KBDINFO kbdinfo;
	INT i;

	tabsize=DEFAULT_TABSIZE;
	scrollup=DEFAULT_SCROLLUP;
	underscore=DEFAULT_UNDERSCORE;
	bold=DEFAULT_BOLD;
	attrib[0]=DEFAULT_ATTRIB;
	bright[0]=DEFAULT_PROMPT;
	blankcell[0]=' ';
	blankcell[1]=DEFAULT_ATTRIB;
	colorcell[0]=' ';
	colorcell[1]=DEFAULT_ATTRIB;

	options=MORE_EOF;
	file_list=current=NULL;
	KbdGetStatus(&kbdinfo,0);
	kbdinfo.fsMask&=~KEYBOARD_ASCII_MODE;
	kbdinfo.fsMask|=KEYBOARD_BINARY_MODE;
	KbdSetStatus(&kbdinfo,0);
	if (argc==1) {
		more_file(stdin);
	}
	else {
		for (i=1;i<argc;i++) {
			input=fopen(argv[i],"r");
			if (input==NULL) {
				printf("Error opening file '%s'.\n",argv[i]);
				printf("Most likely, file non-existant.\n");
				exit(0);
			}
			more_file(input);
			if (i<argc-1) {
				sprintf(prompt,"-- BEGIN FILE %s --",strupper(argv[i+1]));
				VioGetCurPos(&row,&col,0);
				VioWrtCharStrAtt(prompt,strlen(prompt),row,0,bright,0);
				VioWrtNCell(blankcell,col,row,0,0);
				KbdCharIn(&kbdi,0,0);
				switch(kbdi.chChar) {
				case 'q':
				case 'Q':
					VioGetCurPos(&row,&col,0);
					VioSetCurPos(row-1,0,0);
					exit(0);
				}
			}
		}
	}

	VioGetCurPos(&row,&col,0);
	VioSetCurPos(row-1,0,0);
}
