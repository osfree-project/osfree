/*    
	lzc.c	2.5
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */


#include "windows.h"
#include "kerndef.h"
#include "Hunter.h"

#include <sys/types.h>
#include <sys/stat.h>

/*
 * clone of compress/expand from microsoft SDK
 *
 *	single source for both compression and expansion 
 *	cl lzc.c 			to get compression program
 *	cl -Felze -DEXPAND lzc.c	to get expansion program
 *
 *	missing the prompting for input and output files 
 *	when not given on the command line...
 *
 *	does not verify the file size as given by the header
 *	after expanding the file
 *
 *	does not validate that the file is actually compressed
 *	by reading the 4 byte signature
 *
 *	does not support the -r option to automatically name the
 *	output file for compression
 *
 *	does not support multiple file input to a directory output
 *	for compression
 *
 ****************************************************************
 *
 *	still don't know all the fields in the 14 byte header
 *	as far as I can tell
 *
 *	0-3 	are SZDD some kind of signature 
 *	4-9	are 6 bytes of ????
 *	10-13   are the file size 
 *
 */
 
#ifndef EXPAND
/* define the strings when compiled for compression */
char *szexestr = "Compression";		/* what type program */
char *szopstr  = "Compressing";		/* what its doing    */
char *szopcmd  = "compresses";		/* what it does      */
#define LZ	Encode

#else

/* define the strings when compiled for expansion */
char *szexestr = "Expansion";
char *szopstr  = "Expanding";
char *szopcmd  = "expands";
#define LZ	Decode
#endif

/*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define N		4096	/* size of ring buffer */
#define F		16	/* 18 upper limit for match_length */
#define THRESHOLD	2       /* minimum character length to compress */
#define NIL		N	/* index for root of binary search trees */

unsigned long 
		textsize = 0,	/* text size counter */
		codesize = 0;	/* code size counter */
/*********************************************************************
   ring buffer of size N, with extra F-1 bytes to facilitate string 
   comparison of longest match.  These are set by InsertNode()
**********************************************************************/
BYTE 	text_buf[N + F - 1];	

/**********************************************************************
	left & right children & parents
***********************************************************************/
int	match_position, match_length,  
	lson[N + 1], rson[N + 257], dad[N + 1];  

/**********************************************************************
	input & output files
***********************************************************************/
FILE	*infile, *outfile;  

/**********************************************************************
	global name of source file
***********************************************************************/
char	*sourcefile;

/**********************************************************************/
/* this is the header stuck to the front of a compressed file 	      */
/**********************************************************************/
char header[] = {
	'\x53', '\x5A', '\x44', '\x44', 
	'\x88', '\xF0', '\x27', '\x33', '\x41', '\x00'
};

void InitTree(void)  /* initialize trees */
{
	int  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++) rson[i] = NIL;
	for (i = 0; i < N; i++) dad[i] = NIL;
}

void InsertNode(int r)
	/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	   trees (text_buf[r]'th tree) and returns the longest-match position
	   and length via the global variables match_position and match_length.
	   If match_length = F, then removes the old node in favor of the new
	   one, because the old one will be deleted sooner.
	   Note r plays double role, as tree node and position in buffer. */
{
	int  i, p, cmp;
	BYTE  *key;

	cmp = 1;  key = &text_buf[r];  p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;  match_length = 0;
	for ( ; ; ) {
		if (cmp >= 0) {
			if (rson[p] != NIL) p = rson[p];
			else {  rson[p] = r;  dad[r] = p;  return;  }
		} else {
			if (lson[p] != NIL) p = lson[p];
			else {  lson[p] = r;  dad[r] = p;  return;  }
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)  break;
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= F)  break;
		}
	}
	dad[r] = dad[p];  lson[r] = lson[p];  rson[r] = rson[p];
	dad[lson[p]] = r;  dad[rson[p]] = r;
	if (rson[dad[p]] == p) rson[dad[p]] = r;
	else                   lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

void DeleteNode(int p)  /* deletes node p from tree */
{
	int  q;
	
	if (dad[p] == NIL) return;  /* not in tree */
	if (rson[p] == NIL) q = lson[p];
	else if (lson[p] == NIL) q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do {  q = rson[q];  } while (rson[q] != NIL);
			rson[dad[q]] = lson[q];  dad[lson[q]] = dad[q];
			lson[q] = lson[p];  dad[lson[p]] = q;
		}
		rson[q] = rson[p];  dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p) rson[dad[p]] = q;  else lson[dad[p]] = q;
	dad[p] = NIL;
}

void Encode(void)
{
	int  i, c, len, r, s, last_match_length, code_buf_ptr;
	BYTE  code_buf[17], mask;
	struct stat buf;
	unsigned long size;
	
	for(i=0;i<10;i++) {
		putc(header[i],outfile);
	}

	/* now put out four bytes of size */
	stat(sourcefile,&buf);	
	size = buf.st_size;
	putc((size    )&0xff,outfile);
	putc((size>> 8)&0xff,outfile);
	putc((size>>16)&0xff,outfile);
	putc((size>>24)&0xff,outfile);
	
	InitTree();  /* initialize trees */

/***************************************************************************
 	code_buf[1..16] saves eight units of code, and
	code_buf[0] works as eight flags, "1" representing that the unit
	is an unencoded letter (1 byte), "0" a position-and-length pair
	(2 bytes).  Thus, eight units require at most 16 bytes of code. 
****************************************************************************/
		
	code_buf[0] = 0;  

	code_buf_ptr = mask = 1;

	s = 0;  r = N - F;
	/* Clear the buffer with any character that will appear often. */
	for (i = s; i < r; i++) text_buf[i] = ' ';  

	/* Read F bytes into the last F bytes of the buffer */
	for (len = 0; len < F && (c = getc(infile)) != EOF; len++)
		text_buf[r + len] = c;  

	/* text of size zero */
	if ((textsize = len) == 0) 
		return;  

	/****************************************************************** 
	Insert the F strings, each of which begins with one or more 'space' 
	characters.  Note the order in which these strings are inserted.  
	This way, degenerate trees will be less likely to occur. 
	********************************************************************/
	for (i = 1; i <= F; i++) 
		InsertNode(r - i);  

	/**********************************************************
	Finally, insert the whole string just read.  The
	global variables match_length and match_position are set. 
 	***********************************************************/
	InsertNode(r);  

	do {
		/* match_length may be spuriously long near the end of text. */
		if (match_length > len) 
			match_length = len;  

		/* Not long enough match.  Send one byte. */
		if (match_length <= THRESHOLD) {
			/* Send uncoded. */
			match_length = 1;  
			code_buf[0] |= mask;  /* 'send one byte' flag */
			code_buf[code_buf_ptr++] = text_buf[r];  
		} else {
			/************************************
			Send position and length pair. 
			Note match_length > THRESHOLD. 
			*************************************/
			code_buf[code_buf_ptr++] = (BYTE) match_position;
			code_buf[code_buf_ptr++] = 
				(BYTE) (((match_position >> 4) & 0xf0)
			  	| (match_length - (THRESHOLD + 1)));  
		}

		/* Shift mask left one bit. */
		if ((mask <<= 1) == 0) {  
			/* Send at most 8 units of code together */
			for (i = 0; i < code_buf_ptr; i++)  
				putc(code_buf[i], outfile); 

			codesize += code_buf_ptr;
			code_buf[0] = 0;  
			code_buf_ptr = mask = 1;
		}
		last_match_length = match_length;
		for (i = 0; i < last_match_length &&
				(c = getc(infile)) != EOF; i++) {
			DeleteNode(s);		/* Delete old strings and */
			text_buf[s] = c;	/* read new bytes */
			if (s < F - 1) text_buf[s + N] = c;  

			/**************************************************
			 If the position is near the end of buffer, extend 
			   the buffer to make string comparison easier. 
			***************************************************/

			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				/* Since this is a ring buffer, increment the 
				   position modulo N. */

			/* Register the string in text_buf[r..r+F-1] */
			InsertNode(r);	
		}

		/* After the end of text, */
		while (i++ < last_match_length) {	
			/* no need to read, but */
			DeleteNode(s);					
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);

			/* buffer may not be empty. */
			if (--len) InsertNode(r);		
		}
	} while (len > 0);	
	/* until length of string to be processed is zero */

	/* Send remaining code. */
	if (code_buf_ptr > 1) {		
		for (i = 0; i < code_buf_ptr; i++) 
			putc(code_buf[i], outfile);
		codesize += code_buf_ptr;
	}

	codesize += 14;

	printf("%s: %ld bytes compressed to %ld bytes,",
		sourcefile, buf.st_size, codesize);

	printf(" %d%% savings.\n",
		100 - ((100*codesize) / buf.st_size));
}

void Decode(void)
{
	int  i, j, k, r, c;
	int  ii,jj;
	unsigned int  flags;
	int	phase;
	
	/* skip the header... */
	for (i=0;i<14;i++)
		c = getc(infile);

	for (i = 0; i < N - F; i++) text_buf[i] = ' ';
	r = N - F;  flags = 0;

	for ( ; ; ) {
		phase = 0;			
		if (((flags >>= 1) & 0x100) == 0) {
			if ((c = getc(infile)) == EOF) 
				break;

			/* uses higher byte cleverly to count eight */
			flags = c | 0xff00;		
		}							

		if (flags & 1) {
			phase = 1;
			if ((c = getc(infile)) == EOF) 
				break;

			putc(c, outfile);  
			text_buf[r++] = c;  r &= (N - 1);
		} else {
			phase = 2;
			if ((i = getc(infile)) == EOF) break;
			phase = 3;
			if ((j = getc(infile)) == EOF) break;

			ii = i;
			jj = j;

			i |= ((j & 0xf0) << 4);  j = (j & 0x0f) + THRESHOLD;

			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];

				putc(c, outfile);  
				text_buf[r++] = c;  r &= (N - 1);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	char  *s;
	int i;
	
	if (argc != 3) {
		printf("%s:file1 file2 [%s file1 into file2].\n",
			argv[0],szopcmd);
		return EXIT_FAILURE;
	}
	if ((s = argv[1], (infile  = fopen(s, "rb")) == NULL)
	 || (s = argv[2], (outfile = fopen(s, "wb")) == NULL)) {
		printf("??? %s\n", s);  return EXIT_FAILURE;
	}

	printf("Hunter Systems (R) File %s Utility %s\n", 
		szexestr,"Version 1.00");

	printf("Copyright (C) %s %s All rights reserved.\n\n",
		"Hunter Systems",
		"1990-1992.");
	printf("%s %s to %s.\n",
		szopstr,argv[1],argv[2]);
	sourcefile = argv[1];

	LZ();  
	
	fclose(infile);  fclose(outfile);
	return EXIT_SUCCESS;
}
