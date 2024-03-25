/****************************************************************************
 *
 *  msgextrt.c -- Message File Extracter (MSGEXTRT)
 *
 *  ========================================================================
 *
 *    Version 1.0       2024 Yuri Prokushev <yuri.prokushev@gmail.com>
 *
 *  ========================================================================
 *
 *  Description: Simple msg decompiler tool for OS/2 - ArcaOS files.
 *
 *  Based on previous work:
 *      (C) 2005 Veit Kannegieser (E_MSGF)
 *      (C) 2023 Michael K Greene (MKMSGD)
 *
 *  ========================================================================
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

#define INCL_DOSNLS /* National Language Support values */

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef  __LINUX__
#include <io.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h>
#include "mkmsgf.h"
#include "mkmsgerr.h"
#include "version.h"
#include "dlist.h"

int readheader(MESSAGEINFO *messageinfo);
int readmessages(MESSAGEINFO *messageinfo);
int outputheader(MESSAGEINFO *messageinfo);
int readcontrol(MESSAGEINFO *messageinfo);

// ouput display/helper functions
void usagelong(void);
void prgheading(void);
void helpshort(void);
void helplong(void);
void ProgError(int exnum, char *dispmsg);
void displayinfo(MESSAGEINFO *messageinfo);

#if __WATCOMC__ <= 1290
int getline (char **lineptr, unsigned int *n, FILE *stream);

/* getline.c -- Replacement for GNU C library function getline ()

   Copyright (C) 1992 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#ifdef  MAX_CANON
#undef  MAX_CANON
#endif
#define MAX_CANON 64

/* Read up to (and including) a newline from STREAM into *LINEPTR
   (and NUL-terminate it). *LINEPTR is a pointer returned from malloc (or
   NULL), pointing to *N characters of space.  It is realloc'd as
   necessary.  Returns the number of characters read (not including the
   null terminator), or -1 on error or EOF.  */

int
getline (lineptr, n, stream)
  char **lineptr;
  unsigned int *n;
  FILE *stream;
{
  int nchars_avail;
  char *read_pos;

  if (!lineptr || !n || !stream)
    return -1;

  nchars_avail = *n;

  if (!*lineptr)
    {
      if (!(*lineptr = malloc (MAX_CANON)))
	return -1;

      *n = nchars_avail = MAX_CANON;
    }

  read_pos = *lineptr;

  for (;;)
    {
      register char c = getc (stream);

      /* We always want at least one char left in buffer since we
	 always (unless we get an error while reading the first char)
	 NUL-terminate the line buffer. */

      if (nchars_avail < 1)
	{
	  if (*n > MAX_CANON)
	    {
	      nchars_avail = *n;
	      *n *= 2;
	    }
	  else
	    {
	      nchars_avail = MAX_CANON;
	      *n += MAX_CANON;
	    }

	  *lineptr = realloc (*lineptr, *n);
	  read_pos = *lineptr + (*n - nchars_avail);
	}

      /* EOF or error */
      if (feof (stream) || ferror (stream))

	/* Return partial line, if any */
	if (read_pos == *lineptr)
	  return -1;
	else
	  break;

      *read_pos++ = c;
      nchars_avail--;

      /* Return line if NL */
      if (c == '\n')
	break;
    }

  /* Done - NUL terminate and return number of chars read */
  *read_pos = '\0';
  return (*n - nchars_avail);
}

#endif

/*************************************************************************
 * Main( )
 *
 * Entry into the program
 *
 * Expects a valid MSG file only. Will name the output file using the input
 * file and the TXT extention if an output filename is not provided.
 *
 **********************************/

int main(int argc, char *argv[])
{
    int rc = 0; // return code
    unsigned long dlrc = 0; // return code
    int ch = 0; // getopt variable

    MESSAGEINFO messageinfo;     // holds all the info
    messageinfo.verbose = 0;     // start being quiet
    messageinfo.fixlastline = 0; // try to fix last line problems

    // no args - print usage and exit
    if (argc == 1)
    {
        prgheading(); // display program heading
        helpshort();
        exit(MKMSG_NOERROR);
    }

    // Get program arguments using getopt()
    while ((ch = getopt(argc, argv, "vVh")) != -1)
    {
        switch (ch)
        {
        case 'v':
            messageinfo.verbose += 1;
            break;

        case 'V':
            messageinfo.verbose += 2;
            break;

        case 'h':
            prgheading();
            exit(MKMSG_NOERROR);
            break;

        default:
            ProgError(MKMSG_GETOPT_ERROR, "MSGEXTRT: Syntax error unknown option");
            break;
        }
    }

    if (optind == 1 || optind == 2)
    {
        // optind 1 should be input script file
        strncpy(messageinfo.infile, argv[optind], sizeof(messageinfo.infile));
        if (access(messageinfo.infile, F_OK) != 0)
            ProgError(MKMSG_INPUT_ERROR, "MSGEXTRT: Input file does not exist.");

        _splitpath(messageinfo.infile,
                   messageinfo.indrive,
                   messageinfo.indir,
                   messageinfo.infname,
                   messageinfo.inext);

        optind++;

        if (optind != argc)
            // provide output file
            strncpy(messageinfo.outfile, argv[optind], sizeof(messageinfo.outfile));
        else
            // need to make an output file
            sprintf(messageinfo.outfile, "%s%s", messageinfo.infname, ".txt");
    }
    else
    {
        prgheading(); // display program heading
        helpshort();
        exit(MKMSG_NOERROR);
    }

    // check input == output file
    if (!strcmp(messageinfo.infile, messageinfo.outfile))
        ProgError(MKMSG_IN_OUT_COMPARE, "MSGEXTRT: Input file same as output file");

    // ************ done with args ************
	
	messageinfo.msgids=CreateList();
	// Parse control file
	rc = readcontrol(&messageinfo);
    if (rc != MKMSG_NOERROR)
        ProgError(rc, "MSGEXTRT: Control file read error");

    // decompile header
    rc = readheader(&messageinfo);
    if (rc != MKMSG_NOERROR)
        ProgError(rc, "MSGEXTRT: MSG Header read error");

    // display info on screen
    displayinfo(&messageinfo);

    // write out header
    rc = outputheader(&messageinfo);
    if (rc != MKMSG_NOERROR)
        ProgError(rc, "MSGEXTRT: Error generating header");

    // decompile the messages and write
    rc = readmessages(&messageinfo);
    if (rc != MKMSG_NOERROR)
        ProgError(rc, "MSGEXTRT: Error read MSG messages");

	DestroyList(&messageinfo.msgids, FALSE, &dlrc);

    // if you don't see this then I screwed up
    printf("\nEnd Decompile\n");

    return (MKMSG_NOERROR);
}


int readcontrol(MESSAGEINFO *messageinfo)
{
    FILE *fp = fopen(messageinfo->infile, "r");
	size_t n = 0;
	char *line = NULL;
	char msgnum[5]={0};
	int rc = 0;
	unsigned long dlrc = 0;

    getline(&line, &n, fp);
    getline(&line, &n, fp);
    if (strncmp("<", line, 1)!=0)
	{
		ProgError(rc, "MSGEXTRT: Error input file string in control file");
	}

	char *src, *dst;
    for (src = dst = &line[1]; *src != '\0'; src++) {
        *dst = *src;
        if ((*dst != '\r') && (*dst != '\n')) dst++;
    }
    *dst = '\0';
	strncpy(messageinfo->infile, &line[1], sizeof(messageinfo->infile));
	
	while(!feof(fp))
	{
		if (!getline(&line, &n, fp)) break;
		
		if feof(fp) break;

        if (strncmp("DOS", line, 3)!=0)
		{
			ProgError(rc, "MSGEXTRT: Error ID in control file");
		}

        strncpy(msgnum, &line[3], 4);

		InsertItem (messageinfo->msgids,
					sizeof(msgnum),
					msgnum,
					atoi(msgnum),
					NULL,
					AppendToList,
					FALSE,
					&dlrc);

	}

    fclose(fp);
	
	return (MKMSG_NOERROR);
}

/*************************************************************************
 * Function:  readheader( )
 *
 * Reads in all the MSG file info and stores in MESSAGEINFO structure
 *
 * 1. Read in the MSG file header
 * 2. Check for valid signature
 * 3. Transfer header info into MESSAGEINFO structure
 * 4. Read in FILECOUNTRYINFO block into MESSAGEINFO structure
 * 5. Check for extention block and read if exists
 * 6. Calculate message start
 * 7. Calculate index offset and size
 *
 * Return:    returns error code or 0 for all good
 *************************************************************************/

int readheader(MESSAGEINFO *messageinfo)
{
    MSGHEADER *msgheader = NULL;
    FILECOUNTRYINFO *cpheader = NULL;
    EXTHDR *extheader = NULL;

    // open input file
    FILE *fp = fopen(messageinfo->infile, "rb");
    if (fp == NULL)
        return (MKMSG_OPEN_ERROR);

    // buffer to read in header
    char *header = (char *)calloc(sizeof(MSGHEADER), sizeof(char));
    if (header == NULL)
        return (MKMSG_MEM_ERROR1);

    // read header
    int read = fread(header, sizeof(char), sizeof(MSGHEADER), fp);
    if (ferror(fp))
        return (MKMSG_READ_ERROR);

    // MSGHEADER point to header buffer
    msgheader = (MSGHEADER *)header;

    // check header signature, return error no match
    for (int x = 0; x < sizeof(signature); x++)
        if (signature[x] != msgheader->magic_sig[x])
            return (MKMSG_HEADER_ERROR);

    // Pulls all header information into MESSAGEINFO
    for (int x = 0; x < 3; x++)
        messageinfo->identifier[x] = msgheader->identifier[x];

    messageinfo->numbermsg = msgheader->numbermsg;
    messageinfo->firstmsg = msgheader->firstmsg;
    messageinfo->offsetid = msgheader->offset16bit;

    // found old MSG files that do not fill this in
    // manually set it and try for read.
    if (msgheader->hdroffset)
        messageinfo->hdroffset = msgheader->hdroffset;
    else
        messageinfo->hdroffset = 0x1F;

    messageinfo->version = msgheader->version;

    // 3 Sep 23 - I was running some MSG files from the Arca ISO and
    // ran into what looks to be pre-version 2 MSG files or that the
    // was 0 version -- and most info did not exist - fixed below

    // make sure this is a version 2 MSG
    if (messageinfo->version == 2)
    {
        messageinfo->countryinfo = msgheader->countryinfo;
        messageinfo->extenblock = msgheader->extenblock;
        for (int x = 0; x < 5; x++)
            messageinfo->reserved[x] = msgheader->reserved[x];

        // *** Get country info
        // re-allocate buffer to size of FILECOUNTRYINFO
        header = (char *)realloc(header, sizeof(FILECOUNTRYINFO));
        if (header == NULL)
            return (MKMSG_MEM_ERROR2);

        // seek to the block for read
        fseek(fp, messageinfo->countryinfo, SEEK_SET);

        // read header
        read = fread(header, sizeof(char), sizeof(FILECOUNTRYINFO), fp);
        if (ferror(fp))
            return (MKMSG_READ_ERROR);

        // FILECOUNTRYINFO point to header buffer
        cpheader = (FILECOUNTRYINFO *)header;

        // Pulls all country information into MESSAGEINFO
        messageinfo->bytesperchar = cpheader->bytesperchar;
        messageinfo->country = cpheader->country;
        messageinfo->langfamilyID = cpheader->langfamilyID;
        messageinfo->langversionID = cpheader->langversionID;
        messageinfo->codepagesnumber = cpheader->codepagesnumber;
        strcpy(messageinfo->filename, cpheader->filename);
        for (int x = 0; x < messageinfo->codepagesnumber; x++)
            messageinfo->codepages[x] = cpheader->codepages[x];
    }
    else
    {
        messageinfo->countryinfo = 0;
        messageinfo->extenblock = 0;
    }

    // quick check of extended header, it's a small block but be
    // consistent. I do not have an example yet so this is kind of a stub
    if (!messageinfo->extenblock)
    {
        // No ext header so set to 0
        messageinfo->extlength = 0;
        messageinfo->extnumblocks = 0;
    }
    else
    {
        // re-allocate buffer to size of EXTHDR
        header = (char *)realloc(header, sizeof(EXTHDR));
        if (header == NULL)
            return (MKMSG_MEM_ERROR3);

        // seek to the block for read
        fseek(fp, messageinfo->extenblock, SEEK_SET);

        // read header
        read = fread(header, sizeof(char), sizeof(EXTHDR), fp);
        if (ferror(fp))
            return (MKMSG_READ_ERROR);

        // FILECOUNTRYINFO point to header buffer
        extheader = (EXTHDR *)header;

        messageinfo->extlength = extheader->hdrlen;
        messageinfo->extnumblocks = extheader->numblocks;
    }

    // index starts after main header
    messageinfo->indexoffset = messageinfo->hdroffset;

    // again - in versions < 2 FILECOUNTRYINFO doesnot exists
    // so fix

    // get index size in bytes based on offsetid
    if (messageinfo->offsetid)
        messageinfo->indexsize = messageinfo->numbermsg * 2;
    else
        messageinfo->indexsize = messageinfo->numbermsg * 4;

    // start of message area
    if (messageinfo->version == 2)
        messageinfo->msgoffset = messageinfo->countryinfo +
                                 sizeof(FILECOUNTRYINFO);
    else
        messageinfo->msgoffset = messageinfo->hdroffset + messageinfo->indexsize;

    // Since we will determine a message length from the message index
    // contents (next message offset - current message offset) there will
    // be an issue with the last message, there is no next message offset).
    // However, if (messageinfo->extenblock) is true then it can be used.
    // If (messageinfo->extenblock) is false then seek to get end of file
    // and add 1 which will be used as the (final next message offset).
    if (messageinfo->extenblock && messageinfo->version == 2)
        messageinfo->msgfinalindex = messageinfo->extenblock;
    else
    {
        fseek(fp, 0L, SEEK_END);
        // don't panic - I need eof + 1
        messageinfo->msgfinalindex = (unsigned long)ftell(fp) + 1;
    }

    // close up and get out
    fclose(fp);
    free(header);

    return (MKMSG_NOERROR);
}

/*************************************************************************
 * Function:  outputheader()
 *
 * Creates output file and writes out info header as comments
 * Params: loaded MESSAGEINFO structure as an input
 *
 * Return:    returns error code or 0 for all good
 *
 *************************************************************************/

int outputheader(MESSAGEINFO *messageinfo)
{
    // write output file open for append
    FILE *fpo = fopen(messageinfo->outfile, "wb");
    if (fpo == NULL)
        return (MKMSG_OPEN_ERROR);

    // buffer to write out message heaeder - I just pick
    // 140 size just because
    char *write_buffer = (char *)calloc(140, sizeof(char));
    if (write_buffer == NULL)
        return (MKMSG_MEM_ERROR4);

    sprintf(write_buffer, "%s\n;\n",
            "; ********** MSGEXTRT Message file extracter **********");
    fwrite(write_buffer, strlen(write_buffer), 1, fpo);

    sprintf(write_buffer, "; Input filename           %s\n",
            messageinfo->infile);
    fwrite(write_buffer, strlen(write_buffer), 1, fpo);

    sprintf(write_buffer, "; MSG File Version:        %d\n",
            messageinfo->version);
    fwrite(write_buffer, strlen(write_buffer), 1, fpo);

    sprintf(write_buffer, "; Component Identifier:    %c%c%c\n",
            messageinfo->identifier[0],
            messageinfo->identifier[1],
            messageinfo->identifier[2]);
    fwrite(write_buffer, strlen(write_buffer), 1, fpo);

    sprintf(write_buffer, "; Number of messages:      %d\n",
            messageinfo->numbermsg);
    fwrite(write_buffer, strlen(write_buffer), 1, fpo);

    sprintf(write_buffer, "; First message number:    %d\n;\n",
            messageinfo->firstmsg);
    fwrite(write_buffer, strlen(write_buffer), 1, fpo);

    if (messageinfo->version == 2)
    {
        sprintf(write_buffer, "%s\n;\n",
                "; ******************* Country Info *******************");
        fwrite(write_buffer, strlen(write_buffer), 1, fpo);

        sprintf(write_buffer, "; Bytes per character:       %d\n",
                messageinfo->bytesperchar);
        fwrite(write_buffer, strlen(write_buffer), 1, fpo);

        sprintf(write_buffer, "; Country Code:              %d\n",
                messageinfo->country);
        fwrite(write_buffer, strlen(write_buffer), 1, fpo);

        sprintf(write_buffer, "; Language family ID:        %d\n",
                messageinfo->langfamilyID);
        fwrite(write_buffer, strlen(write_buffer), 1, fpo);

        sprintf(write_buffer, "; Language version ID:       %d\n",
                messageinfo->langversionID);
        fwrite(write_buffer, strlen(write_buffer), 1, fpo);

        sprintf(write_buffer, "; Number of codepages:       %d\n",
                messageinfo->codepagesnumber);
        fwrite(write_buffer, strlen(write_buffer), 1, fpo);

        memset(write_buffer, 0x00, _msize(write_buffer));
        for (int x = 0; x < messageinfo->codepagesnumber; x++)
        {
            sprintf(write_buffer, "; Codepage %d        0x%02X (%d)\n",
                    (x + 1), messageinfo->codepages[x], messageinfo->codepages[x]);
            fwrite(write_buffer, strlen(write_buffer), 1, fpo);
        }

        sprintf(write_buffer, ";\n; File name:                 %s\n",
                messageinfo->filename);
        fwrite(write_buffer, strlen(write_buffer), 1, fpo);

        if (messageinfo->extenblock)
        {
            sprintf(write_buffer, "%s\n;\n",
                    ";\n; ** Has an extended header **");
            fwrite(write_buffer, strlen(write_buffer), 1, fpo);

            sprintf(write_buffer, "; Ext header length:        %d\n",
                    messageinfo->extlength);
            fwrite(write_buffer, strlen(write_buffer), 1, fpo);

            sprintf(write_buffer, "; Number ext blocks:        %d\n;\n",
                    messageinfo->extnumblocks);
            fwrite(write_buffer, strlen(write_buffer), 1, fpo);
        }
        else
        {
            sprintf(write_buffer, "%s\n;\n",
                    ";\n; ** No an extended header **");
            fwrite(write_buffer, strlen(write_buffer), 1, fpo);
        }
    }
    // close up and get out
    fclose(fpo);
    free(write_buffer);

    return (MKMSG_NOERROR);
}

/*************************************************************************
 * Function:  readmessages()
 *
 * 1. Opens input and output message files
 * 2. Setup buffers for index, read, and write
 * 3. Read in full index
 * 4. Write out idenifier -- needs 0x0D 0x0A ending
 * 5. Setup for uint8 or uint32 index read
 * 6. Main loop
 * 6.1 Calculate message number
 * 6.2 If last message - get final pointer for read
 * 6.3 Calculate message length from index
 * 6.4 Seek to message start
 * 6.5 Resize read buffer if needed
 * 6.6 Clear read buffer with all 0x00
 * 6.7 Read in the current message
 * 6.8 Verify msg length (current_msg_len) using strlen
 * 6.9 Check for no 0x0D 0x0A end - if not add %, 0, 0x0D, 0x0A
 * 6.10 Setup scratch pointer and move past msg_type
 * 6.11 Resize write buffer if needed
 * 6.12 Generate message header and write
 * 6.13 Write message
 * 6.14 If V option print to screen
 * 7 Close files and free buffers
 * 8 Return
 *
 * Return:    returns error code or 0 for all good
 *
 *************************************************************************/

typedef
struct tagParam
{
	int num;
	FILE * f;
	char * msg;
	MESSAGEINFO *messageinfo;
} Param;

void handleitem(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{
	if (((Param *)Parameters)->num==ObjectTag)
	{
        fwrite(((Param *)Parameters)->msg, strlen(((Param *)Parameters)->msg), 1, ((Param *)Parameters)->f);
        // print to screen if you really want it
        if (((Param *)Parameters)->messageinfo->verbose == 2)
            printf("%s", ((Param *)Parameters)->msg);
	}

}

int readmessages(MESSAGEINFO *messageinfo)
{
    // index pointers
    uint16_t *small_index = NULL;      // used if index pointers uint16
    uint32_t *large_index = NULL;      // used if index pointers uint32
    char msginfo[10] = {0};            // current message header
    char *scratchptr = NULL;           // scratch pointer
    unsigned long msg_curr = 0;        // pointer to current index msg
    unsigned long msg_next = 0;        // pointer to next index msg
    unsigned long current_msg = 0;     // current msg number being processed
    unsigned long intial_len = 0;      // save intial length
    unsigned long current_msg_len = 0; // current msg length
    unsigned long last_message;        // track last message

    // open input file
    FILE *fpi = fopen(messageinfo->infile, "rb");
    if (fpi == NULL)
        return (MKMSG_OPEN_ERROR);

    // write output file open for append
    FILE *fpo = fopen(messageinfo->outfile, "ab");
    if (fpo == NULL)
        return (MKMSG_OPEN_ERROR);

    // buffer to read in index
    char *index_buffer = (char *)calloc(messageinfo->indexsize, sizeof(char));
    if (index_buffer == NULL)
        return (MKMSG_MEM_ERROR5);

    // buffer to read in a message - start with a 80 size buffer
    // if for some reason bigger is needed realloc latter
    char *read_buffer = (char *)calloc(80, sizeof(char));
    if (read_buffer == NULL)
        return (MKMSG_MEM_ERROR6);

    // buffer to write in a message - start with a 80 size buffer
    // if for some reason bigger is needed realloc latter
    char *write_buffer = (char *)calloc(80, sizeof(char));
    if (write_buffer == NULL)
        return (MKMSG_MEM_ERROR7);

    // *** get full index into buffer (index_buffer)

    // seek to the start of index for read
    fseek(fpi, messageinfo->indexoffset, SEEK_SET);

    // read index into buffer
    fread(index_buffer, sizeof(char), messageinfo->indexsize, fpi);
    if (ferror(fpi))
        return (MKMSG_READ_ERROR);

    // not pretty, but the old IBM MKMSGF expects this
    // line to end with 0x0D 0x0A
    write_buffer[0] = messageinfo->identifier[0];
    write_buffer[1] = messageinfo->identifier[1];
    write_buffer[2] = messageinfo->identifier[2];
    write_buffer[3] = 0x0D;
    write_buffer[4] = 0x0A;
    write_buffer[5] = 0x00;

    fwrite(write_buffer, strlen(write_buffer), 1, fpo);

    // pick the pointer based on index uint16 or uint32
    if (messageinfo->offsetid)
        small_index = (uint16_t *)index_buffer;
    else
        large_index = (uint32_t *)index_buffer;

    // last message number
    last_message = (messageinfo->numbermsg + messageinfo->firstmsg - 1);

    // **** main read - read/write loop
    for (int count = 0; count < messageinfo->numbermsg; count++)
    {
        // do the message number counting
        current_msg = messageinfo->firstmsg + count;

        // handle the uint16 and uint32 index differences
        if (messageinfo->offsetid)
        {
            msg_curr = (unsigned long)*small_index++;
            msg_next = (unsigned long)*small_index;
        }
        else
        {
            msg_curr = *large_index++;
            msg_next = *large_index;
        }

        // if we are on the last message then the msg_next
        // needs to be the end of the message block + 1 as calculated
        // with the fseek to end above.
        // As a note, I am going to use msg_curr and msg_next to
        // get the message length.
        if (count == (messageinfo->numbermsg - 1))
            msg_next = messageinfo->msgfinalindex;

        // just calc current message length for readability
        current_msg_len = (msg_next - msg_curr);
        intial_len = current_msg_len; // for fix last line

        // seek to the start of message to read
        fseek(fpi, msg_curr, SEEK_SET);

        // Read buffer sizing **********************************
        //
        // check read buffer size -- Do we need a bigger buffer?
        // Note: the +5 size is to give me room for %0 or <CR>
        // and I am paranoid :)
        // I did not need to do this, but just for fun I contract
        // the buffer
        // I am using needed size +5 in case I need to append %0
        // and <CR> while giving me a 0x00 final char for strlen
        if (((current_msg_len + 5) > _msize(read_buffer)) ||
            (_msize(read_buffer) > (current_msg_len * 4)))
        {
            read_buffer = (char *)realloc(read_buffer, (current_msg_len + 5));
            if (read_buffer == NULL)
                return (MKMSG_MEM_ERROR8);
        }

        // clear the read_buffer -- set all to 0x00 this will
        // give me a clean strlen return
        memset(read_buffer, 0x00, _msize(read_buffer));

        // read in the message to the read buffer
        fread(read_buffer, sizeof(char), current_msg_len, fpi);

        // had a couple questionable messages (which could have been
        // my fault) so this will give me a know string to change
        // the right end of the string
        current_msg_len = strlen(read_buffer);

        // As a side note - any message can use the <CR> option!
        // If the original message ended with %0, it is then compiled
        // without a <CR>. So we need to check each input line for
        // 0x0D 0x0A and if does not exist then add %0 and 0x0A

        if (read_buffer[(current_msg_len - 1)] != 0x0A &&
            read_buffer[(current_msg_len - 2)] != 0x0D)
        {
            read_buffer[(current_msg_len + 0)] = '%';
            read_buffer[(current_msg_len + 1)] = '0';
            read_buffer[(current_msg_len + 2)] = 0x0D;
            read_buffer[(current_msg_len) + 3] = 0x0A;
            current_msg_len += 4;
        }

        // set up scratch pointer to skip Msg_Type (1)
        scratchptr = read_buffer;
        *scratchptr++;
        current_msg_len -= 1;

        // Write buffer sizing **********************************
        //
        // check write buffer size -- Do we need a bigger buffer?
        // Note: the +5 size is to give me room for %0 or <CR>
        // and I am paranoid :)
        // I did not need to do this, but just for fun I contract
        // the buffer

        // check write buffer size -- Do we need a bigger buffer?
        if ((current_msg_len + 15) > _msize(write_buffer) ||
            (_msize(write_buffer) > (current_msg_len * 5)))
        {
            write_buffer = (char *)realloc(write_buffer, (current_msg_len + 15));
            if (write_buffer == NULL)
                return (MKMSG_MEM_ERROR9);
        }

        // clear the read_buffer -- set all to 0x00
        memset(write_buffer, 0x00, _msize(write_buffer));

        // write the message header file to the write buffer
        // Comp_ID (3) + Msg_Num (4) + Msg_Type (1) + ": " (2) = 10
        sprintf(msginfo, "%c%c%c%04d%c: ",
                messageinfo->identifier[0],
                messageinfo->identifier[1],
                messageinfo->identifier[2],
                current_msg,
                read_buffer[0]);

        // add the msginfo to the write buffer
        strncpy(write_buffer, msginfo, 10);

        // add the message to the write buffer
        strncat(write_buffer, scratchptr, current_msg_len);

        // if -f option try to fix last line issues
        if ((current_msg == last_message) && messageinfo->fixlastline)
        {
            printf("Last Message  Initial %d  Current %d\n", intial_len, current_msg_len);
        }

        // write the record to the output file
        // just a note here: The write_buffer is larger than
        // needed (see +15 above) and I memset to fill with 0x00
        // given this, we will get the write size using strlen()
        // which returns a size up to the 0x00
		Param p;
		unsigned long dlrc=0;
		p.num=current_msg;
		p.f=fpo;
		p.msg=write_buffer;
		p.messageinfo=messageinfo;
		
				ForEachItem(messageinfo->msgids,
					&handleitem,
					(ADDRESS)&p,
					TRUE,
					&dlrc);
#if 0		
        fwrite(write_buffer, strlen(write_buffer), 1, fpo);

        // print to screen if you really want it
        if (messageinfo->verbose == 2)
            printf("%s", write_buffer);
#endif
    }

    // close up and get out
    fclose(fpo);
    fclose(fpi);
    free(read_buffer);
    free(write_buffer);
    free(index_buffer);

    return (MKMSG_NOERROR);
}

/*
 * User message functions
 */
void usagelong(void)
{
    helpshort();
    helplong();
}

void helpshort(void)
{
    printf("\nMSGEXTRT [-v] infile.msg [outfile.[txt] ]\n\n");
}

void helplong(void)
{
    printf("\nUse MSGEXTRT as follows:\n");
    printf("        [-v] infile.msg [outfile.[txt] ]\n");
}

void prgheading(void)
{
    printf("\nOperating System/2 Make Message File Extractor (MSGEXTRT)\n");
    printf("Version %s  Yuri Prokushev <yuri.prokushev@gmail.com>\n", SYSLVERSION);
    printf("Compiled with Open Watcom %d.%d  %s\n", OWMAJOR, OWMINOR, __DATE__);
}

/*************************************************************************
 * Function:  displayinfo()
 *
 * Display MESSAGEINFO data to screen
 *
 *************************************************************************/

void displayinfo(MESSAGEINFO *messageinfo)
{
    printf("\n*********** Header Info ***********\n\n");

    printf("Input filename         %s\n", messageinfo->infile);
    printf("Component Identifier:  %c%c%c\n", messageinfo->identifier[0],
           messageinfo->identifier[1], messageinfo->identifier[2]);
    printf("Number of messages:    %d\n", messageinfo->numbermsg);
    printf("First message number:  %d\n", messageinfo->firstmsg);
    printf("OffsetID:              %d  (Offset %s)\n", messageinfo->offsetid,
           (messageinfo->offsetid ? "uint16_t" : "uint32_t"));
    printf("MSG File Version:      %d\n", messageinfo->version);
    printf("Header offset:         0x%02X (%d)\n",
           messageinfo->hdroffset, messageinfo->hdroffset);
    printf("Country Info:          0x%02X (%d)\n",
           messageinfo->countryinfo, messageinfo->countryinfo);
    printf("Extended Header:       0x%02X (%lu)\n",
           messageinfo->extenblock, messageinfo->extenblock);
    printf("Reserved area:         ");
    for (int x = 0; x < 5; x++)
        printf("%02X ", messageinfo->reserved[x]);
    printf("\n");

    if (messageinfo->reserved[0] == 0x4D &&
        messageinfo->reserved[1] == 0x4B &&
        messageinfo->reserved[2] == 0x47)
        printf("Built with MKMSGF clone (signature):  %s\n", messageinfo->reserved);

    if (messageinfo->version == 2)
    {
        printf("\n*********** Country Info  ***********\n\n");
        printf("Bytes per character:       %d\n", messageinfo->bytesperchar);
        printf("Country Code:              %d\n", messageinfo->country);
        printf("Language family ID:        %d\n", messageinfo->langfamilyID);
        printf("Language version ID:       %d\n", messageinfo->langversionID);
        printf("Number of codepages:       %d\n", messageinfo->codepagesnumber);
        for (int x = 0; x < messageinfo->codepagesnumber; x++)
            printf("0x%02X (%d)  ", messageinfo->codepages[x], messageinfo->codepages[x]);
        printf("\n");
        printf("File name:                 %s\n\n", messageinfo->filename);
        if (messageinfo->extenblock)
        {
            printf("** Has an extended header **\n");
            printf("Ext header length:        %d\n", messageinfo->extlength);
            printf("Number ext blocks:        %d\n\n", messageinfo->extnumblocks);
        }
        else
            printf("** No an extended header **\n\n");
    }
    return;
}

/* ProgError( )
 *
 * stardard message print
 *
 * if exnum < 0 then print heading (if not yet displayed) and
 * return.
 * else do the above and exit
 *
 */

void ProgError(int exnum, char *dispmsg)
{
    char buffer[80] = {0};

    sprintf(buffer, "\n%s (%d)\n", dispmsg, exnum);

    if (exnum < 0)
    {
        printf("%s", buffer);
        return;
    }
    else
    {
        helpshort();
        printf("%s", buffer);
        exit(exnum);
    }
}
