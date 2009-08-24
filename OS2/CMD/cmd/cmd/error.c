// ERROR.C - error routines for 4os2
//   (c) 1988 - 1995  Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// force definition of the global variables
#define DEFINE_GLOBALS 1

#include "4all.h"


static char errorfile[] = "C:\\OS2\\SYSTEM\\OSO001.MSG";

#include <errno.h>

USHORT os2_errors[] = {
	1,
	1,
	206,		// ERANGE
	1,
	1,
	1,
	1,
	1,
	1,
	32,		// EISOPEN file open -> sharing violation
	1,		// 10
	1,
	1,
	8,		// EOUTOFMEM
	1,
	1,
	80,		// EEXIST
	1,
	1,
	1,
	1,		// 20
	1,
	1,
	2,		// ENOENT
	5,		// EACCESS
	4,		// 25 - EMFILE
	1,
	1,
	1,
	1,
	1,		// 30
	1,
	1,
	1,
	1,
	1,
	87,		// EINVAL
	1,
	1,
	1,
	1,		// 40
	1,
	6,		// EBADF
	1,
	1,
	112,		// ENOSPC
	1,
	1,
	1,
	1,
	1,		// 50
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,		// 60 - EOS2ERR
	1,
	1,
	1,
	1
};

void CheckOnError( void );		// also called from INOUT.ASM
static void CheckOnErrorMsg( void );


// display batch file name & line number
static void BatchErrorMsg( void )
{
	if (( cv.bn >= 0 ) && (( bframe[ cv.bn ].flags & BATCH_REXX ) == 0 ))
		qprintf( STDERR, "%s [%d]  ", bframe[ cv.bn ].pszBatchName, bframe[ cv.bn ].uBatchLine );
}


// display proper command usage
int usage( char *s )
{
	char szBuffer[256];

	gnInternalErrorLevel = USAGE_ERR;
	CheckOnError();

	BatchErrorMsg();
	qprintf( STDERR, USAGE_MSG, gpInternalName );

	s = strcpy( szBuffer, s );
	for ( ; *s; s++ ) {

		// expand ? to [d:][path]filename
		if ( *s == '?' )
			strins( s + 1, FILE_SPEC );

		// expand ~ to [d:]pathname
		else if ( *s == '~' )
			strins( s + 1, PATH_SPEC );

		// expand # to [bri] [bli] fg ON bg
		else if ( *s == '#' )
			strins( s + 1, COLOR_SPEC );
		else
			continue;

		strcpy( s, s + 1 );
	}

	qprintf( STDERR, FMT_STR_CRLF, szBuffer );

	CheckOnErrorMsg();

	return USAGE_ERR;
}


// display an OS or 4xxx error message w/optional argument
int error( int code, char *s )
{
	// OS/2 message file
	PCHAR ppchVTable[2];
	char message[512];
	unsigned long msglen;

	gnInternalErrorLevel = ERROR_EXIT;
	CheckOnError();

	// print the error message, optionally labeling it as belonging to 4xxx
	if ( gpIniptr->INIDebug & INI_DB_ERRLABEL )
		qprintf( STDERR, "%s: ", NAME_HEADER );

	// display batch file name & line number
	BatchErrorMsg();

	if ( code < OFFSET_4DOS_MSG ) {

		// C Set/2 doesn't always set _doserrno!
		if ( code <= 0 )
			code = os2_errors[ errno ];
		gnSysError = code;

		// get the OS/2 error message from the message file
		ppchVTable[0] = s;
		errorfile[0] = gchSysBootDrive;
		msglen = 0;
		if ( DosGetMessage( ppchVTable, ( s != NULL ), message, sizeof(message), code, errorfile, &msglen ) != 0 )
			(void)DosGetMessage( ppchVTable, ( s != NULL ), message, sizeof(message), code, fname_part( errorfile ), &msglen );
		message[ msglen ] = '\0';
		strip_trailing( message, "\r\n" );

		qprintf( STDERR, FMT_STR, message );

	} else {
		// it's a 4OS2 message
		qprintf( STDERR, FMT_STR, int_4dos_errors[code - OFFSET_4DOS_MSG] );
	}

	// if s != NULL, print it (probably a filename) in quotes
	if ( s != NULL )
		qprintf(STDERR, (( *s == '"' ) ? " %s": " \"%s\"" ), s );

	qputc( STDERR, '\n' );

	CheckOnErrorMsg();

	return ERROR_EXIT;
}



// check for "ON ERROR" in batch files
void CheckOnError( void )
{
	if (( cv.bn >= 0 ) && ( bframe[cv.bn].OnError != NULL )) {
		cv.exception_flag |= BREAK_ON_ERROR;
		BreakOut();
	}
}


// check for "ON ERRORMSG" in batch files
static void CheckOnErrorMsg( void )
{
	if (( cv.bn >= 0 ) && ( bframe[cv.bn].OnErrorMsg != NULL )) {
		cv.exception_flag |= BREAK_ON_ERROR;
		BreakOut();
	}
}

