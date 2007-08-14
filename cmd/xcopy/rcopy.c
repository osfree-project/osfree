/*
 * $Source: /netlabs.cvs/osfree/src/cmd/xcopy/rcopy.c,v $
 * $Revision: 1.1 $
 * $Date: 2003/10/20 13:59:42 $
 * $Author: prokushev $
 *
 * Copies directory tree, optional excluding some files or directories
 *
 * Usage of verbosity levels
 *	0	nothing could be done and internal/impossible errors
 *	1	prevents file(s) to be copied
 *	2	errors but may be OK
 *	3	any other messages
 *
 * ---- <Comment of version 1.00> ---------------------------------------
 * AUFRUF
 *	RCOPY  [d:]\[path] [d:]\[path] [/X:name[;name]]
 *
 * BUGS
 *	- Nicht erlauben, da· in ein Unterverzeichnis des Quellverzeichnisses
 *	  kopiert wird (unendliche Rekursion!)
 *
 * PROBLEMS
 *	- CopyTree()
 *	  Leider kann unter DOS immer nur ein Aufruf von
 *	  DosFindFirst/DosFindNext aktiv sein, deshalb kann nicht
 *	  innerhalb einer gro·en Schleife nach Dateien _und_ Verzeichnissen
 *	  gesucht werden (und die Verzeichnisse dort rekursiv kopiert
 *	  werden), sondern erst mÅssen die Namen der Unterverzeichnisse
 *	  gespeichert werden, bevor dann diese bearbeitet werden. Als
 *	  zweiter Schritt kommen dann die Dateien im aktuellen Verzeichnis.
 *	  Kompliziert, nicht wahr?
 *
 * $Log: rcopy.c,v $
 * Revision 1.1  2003/10/20 13:59:42  prokushev
 * + XCOPY
 * * clean target
 *
 * Revision 2.3  1997/09/28 22:39:26  vitus
 * removed copyright notices
 *
 * Revision 2.2  1997/01/24 00:42:24  vitus
 * Corrected file skipping (call to DosFindFirst)
 *
 * Revision 2.1  1997/01/22 00:38:44  vitus
 * Moved to GNU C (emx)
 * Returns error code
 * Uses verbose library
 * ----------------------------------------
 * This code used to be Copyright Vitus Jensen 1996, but I
 * don't think it's worth the trouble.  Just keep my name somewhere
 * in the records if you modify the code.  And don't charge money
 * just for copying it.
 * Autor: Vitus Jensen, can be reached via fidonet 2:2474/424
 */
static char vcid[]="$Id: rcopy.c,v 1.1 2003/10/20 13:59:42 prokushev Exp $";


#define VERSION 	2
#define SUBVERSION	2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>

#include "../../tools/verbose.h"




/* **********************************************************************
 * **** Global variables ************************************************
 * ******************************************************************* */

int	fOverwrite = 0;
int	fDisplayNames = 0;
char	ProgName[_MAX_PATH];
char	szSrcPath[_MAX_PATH];
char	szDstPath[_MAX_PATH];
struct skiplistentry_t {
    struct skiplistentry_t     *next;
    char			name[_MAX_PATH];
} *pSkipListStart=NULL;








/*#
 * NAME
 *	inSkipList
 * CALL
 *	inSkipList(name)
 * PARAMETER
 *	name	absolute file name (no wildcards)
 * RETURNS
 *	0	not in list
 *	/0	in list: skip file
 * GLOBAL
 *	pSkipListStart
 * DESPRIPTION
 *	Searches 'name' in list of files and directories to skip.
 * REMARKS
 */
int
inSkipList(char *name)
{
    struct skiplistentry_t *pSkipEntry = pSkipListStart;

    while( pSkipEntry )
    {
	if( !stricmp(name, pSkipEntry->name) )		/* ignore case */
	    return 1;
	pSkipEntry = pSkipEntry->next;
    }
    return 0;
}




/*#
 * NAME
 *	CheckPath
 * CALL
 *	CheckPath(path,create)
 * PARAMETER
 *	path		absolute directory name
 *	create		create directory if not existing
 * RETURNS
 *	0		directory is now existing
 *	/0		file, not existing, etc.
 * GLOBAL
 *	none
 * DESPRIPTION
 * REMARKS
 */
int
CheckPath(char *path,int create)
{
    char   dir[_MAX_PATH];
    struct stat stbuf;

    strcpy( dir, path );
    if( dir[strlen(dir)-1] == '/'  &&  dir[strlen(dir)-2] != ':' )
        dir[strlen(dir)-1] = '\0';
    if( stat(dir, &stbuf) != 0 )
    {
	if( !create )
	{
	    Verbose(1,"stat(%s) - errno %u (%s)", dir, errno, strerror(errno) );
	    return errno;
	}
	else
	{
	    if( mkdir(dir, 0) )
	    {
		Verbose(1,"mkdir(%s) - errno %u (%s)",dir,errno,strerror(errno));
		return errno;
	    }
	}
    }
    else
    {
	if( (stbuf.st_mode & S_IFMT) != S_IFDIR )
	{
	    Verbose(1,"stat(%s) - no directory",dir);
	    return -1;
	}
    }
    return 0;
}




/*#
 * NAME
 *	MyDosCopy
 * CALL
 *	MyDosCopy(src,dst)
 * PARAMETER
 *	src		name of source file
 *	dst		name of destination (may be existent)
 * RETURNS
 *	0		copy done
 *	/0		file exits, disk full, etc.
 * GLOBAL
 *	overwrite
 * DESPRIPTION
 *	Does things like DosCopy(), OS/2 API
 * REMARKS
 */
#define IOBUF_SIZ	32768U			/* enough? (performance) */
USHORT
MyDosCopy(char *src,char *dst)
{
    int    hSrc, hDst;
    char  *iobuf;
    int    cbTransfer;

    if( (iobuf=malloc(IOBUF_SIZ)) == NULL )
    {
	Verbose(1,"%s\tno more memory",src);
	return (USHORT)-1;
    }
    if( (hSrc=open(src, O_BINARY|O_RDONLY)) == -1 )
    {
	Verbose(1,"open(%s) - errno %u (%s)", src, errno, strerror(errno) );
	free( iobuf );
	return 0;
    }
    if( (hDst=open(dst,
		   O_BINARY|O_WRONLY|O_CREAT|(fOverwrite ? O_TRUNC : O_EXCL),
		   S_IREAD|S_IWRITE)) == -1 )
    {
	Verbose(1,"open(%s) - errno %u (%s)", dst, errno, strerror(errno) );
	free( iobuf );
	close( hSrc );
	return errno;
    }
    while( (cbTransfer=read(hSrc, iobuf, IOBUF_SIZ)) != (USHORT)-1
	  &&  cbTransfer != 0 )
    {
	if( write(hDst, iobuf, cbTransfer) != cbTransfer )
	{
	    Verbose(1,"write(%s) - errno %u (%s)", dst, errno, strerror(errno) );
	    free( iobuf );
	    close( hSrc );
	    close( hDst );
	    return errno;
	}
    }

    free( iobuf );
    close( hSrc );
    close( hDst );
    return 0;
}



/*#
 * NAME
 *	CopyFile
 * CALL
 *	CopyFile(src,dst)
 * PARAMETER
 *	src		absolute path to source file
 *	dst		absolute path to destination file
 * RETURNS
 *	0		file copied, error may be ignored
 *	/0		serious error occured
 * GLOBAL
 *	overwrite	overwrite existing files ?
 * DESPRIPTION
 * REMARKS
 *	Uses API:DosCopy() under OS/2 but has to do all by itself
 *	under MS-DOS.
 */
USHORT
CopyFile(char *src,char *dst)
{
    APIRET	rc;
    USHORT	result = 0;			/* of this function */
    char const * p = NULL;			/* error description */

    if( _osmode == OS2_MODE )
    {
	if( (rc=DosCopy(src, dst, (fOverwrite ? DCPY_EXISTING : 0))) )
	{
	    Verbose(1,"DosCopy(%s,...) - rc %u", src, rc );
	    if( rc == ERROR_ACCESS_DENIED )
	    {
		p = "\taccess denied";
	    }
	    else if( rc == ERROR_SHARING_VIOLATION
		    ||	rc == ERROR_SHARING_BUFFER_EXCEEDED )
	    {
		p = "\tsharing error";
	    }
	    else if( rc == ERROR_FILENAME_EXCED_RANGE )
	    {
		p = "\tfilename too long";
	    }
	    else if( rc == ERROR_DISK_FULL )
	    {
		p = "\tdisk(ette) full";
	    }
	    else if( rc == ERROR_DRIVE_LOCKED )
	    {
		p = "\tdrive locked";
	    }
	    else if( rc == ERROR_INVALID_PARAMETER )
	    {
		p = "\tparameter error (internal error!)";
	    }
	    else if( rc == ERROR_NOT_DOS_DISK )
	    {
		p = "\tno DOS Disk(ette)";
	    }
	    else if( rc == ERROR_PATH_NOT_FOUND )
	    {
		p = "\tpath not found";
	    }
	    else
	    {
		p = "\tunknown error";
	    }
	    result |= rc;
	}
    }
    else
    {
	result = MyDosCopy( src, dst );
    }

    if( fDisplayNames )
	printf("%s%s\n", src, (p != NULL ? p : "") );

    return result;
}




/*#
 * NAME
 *	CopyTree
 * CALL
 *	CopyTree(src,dst)
 * PARAMETER
 *	src		Source Path (existing)
 *	dst		Destination Path (existing)
 * RETURNS
 *	0		OK
 * GLOBAL
 * DESPRIPTION
 *	Copies one subdirectory
 * REMARKS
 */
int
CopyTree(char *src,char *dst)
{
    FILEFINDBUF3  findBuffer;
    HDIR	 hSearch;
    ULONG	 cFound;
    APIRET	 rc;
    int		 result = 0, i;
    char	*nsp, *ndp;
    struct dirlist_t {
	char		  src[_MAX_PATH];
	char		  dst[_MAX_PATH];
	struct dirlist_t *next;
    } *pDirListRoot=NULL, *pDirList=NULL, *pHelp;

    nsp = src + strlen(src);
    ndp = dst + strlen(dst);

    /* Search all subdirectories */

    if( _osmode == OS2_MODE )
	strcpy( nsp, "*" );
    else
	strcpy( nsp, "*.*" );
    hSearch = HDIR_SYSTEM;			/* use system handle */
    cFound = 1;					/* only one at a time */
    rc = DosFindFirst( src, &hSearch, MUST_HAVE_DIRECTORY|FILE_DIRECTORY,
		      &findBuffer, sizeof(findBuffer), &cFound, FIL_STANDARD );
    if( !rc )
	do
	{
	    if( !strcmp(findBuffer.achName, ".")
	       ||  !strcmp(findBuffer.achName, "..") )
		continue;
	    if( !(findBuffer.attrFile & FILE_DIRECTORY) )
		continue;
	    Verbose(3,"Found: %s\tattr: %#x",
		    findBuffer.achName, findBuffer.attrFile );

	    strcpy( nsp, findBuffer.achName );
	    strcpy( ndp, findBuffer.achName );
	    if( inSkipList(src) )		/* exclude some dirs/files */
	    {
		if( fDisplayNames )
		    printf("Skipping %s\n", src );
		continue;
	    }
	    strcat( src, "/" );
	    strcat( dst, "/" );

	    /* TyvÑr, vi kan inte kopiera nu */

	    if( pDirList )
	    {
		pDirList->next = malloc( sizeof(struct dirlist_t) );
		assert( pDirList->next != NULL );
		pDirList = pDirList->next;
	    }
	    else
	    {
		pDirList = pDirListRoot = malloc( sizeof(struct dirlist_t) );
		assert( pDirList != NULL );
	    }
	    pDirList->next = NULL;
	    strcpy( pDirList->src, src );
	    strcpy( pDirList->dst, dst );
	}
	while( !(rc=DosFindNext(hSearch, &findBuffer,
				sizeof(findBuffer), &cFound)) );
    DosFindClose( hSearch );
    Verbose(2,"Subdirectory search stopped with rc %u",rc);

    for( pHelp = pDirList = pDirListRoot; pDirList ; pHelp = pDirList )
    {
	pDirList = pDirList->next;
	if( (i=CheckPath(pHelp->dst, 1)) )	/* create destination path */
	{
	    result = i;
	    Verbose(0,"Cannot copy %s", pHelp->src );
	}
	else
	{
	    CopyTree( pHelp->src, pHelp->dst );
	}
	free( pHelp );
    }

    /* Copy the files in actual directory */

    if( _osmode == OS2_MODE )
	strcpy( nsp, "*" );
    else
	strcpy( nsp, "*.*" );
    hSearch = HDIR_SYSTEM;			/* use system handle */
    cFound = 1;					/* only one at a time */
    rc = DosFindFirst( src, &hSearch, FILE_NORMAL,
		      &findBuffer, sizeof(findBuffer), &cFound, FIL_STANDARD );
    if( !rc )
	do
	{
	    if( findBuffer.attrFile & FILE_DIRECTORY )
		continue;
	    Verbose(3,"Found: %s\tattr: %#x",
		    findBuffer.achName, findBuffer.attrFile );

	    strcpy( nsp, findBuffer.achName );
	    strcpy( ndp, findBuffer.achName );
	    if( inSkipList(src) )		/* exclude some dir's/file's */
	    {
		if( fDisplayNames )
		    printf("Skipping %s\n", src );
		continue;
	    }
	    i = CopyFile( src, dst );
	    if( i != 0 )
		result = i;
	}
	while( !(rc=DosFindNext(hSearch, &findBuffer,
				sizeof(findBuffer), &cFound)) );
    DosFindClose( hSearch );
    Verbose(2,"File search stopped with rc %u",rc);

    *nsp = '\0';
    *ndp = '\0';
    return result;
}








/*#
 * NAME
 *	ExpandPath
 * CALL
 *	ExpandPath(path,create)
 * PARAMETER
 *	path            relative path
 *	create          create directory if it doesn't exits?
 * RETURNS
 *	0		path exits (may be created)
 *			path contains absolute path
 *	/0		path not existent
 * GLOBAL
 *	none
 * DESPRIPTION
 * REMARKS
 *	Uses CheckPath()
 */
int
ExpandPath(char *path,int create)
{
    int 	rc;
    char	abs_path[_MAX_PATH];

    if( _fullpath(abs_path, path, sizeof(abs_path)) != 0 )
    {
	Verbose(0,"_fullpath - errno %u (%s)", errno, strerror(errno) );
	return errno;
    }
    if( (rc=CheckPath(abs_path, create)) )
    {
	return rc;
    }
    strcpy( path, abs_path );
    if( path[strlen(path)-1] != '/' )
        strcat( path, "/" );
    return 0;
}




/*#
 * NAME
 *	ExpandXList
 * CALL
 *	ExpandXList(list)
 * PARAMETER
 *	list		/X:__;__;__
 * RETURNS
 *	0		valid list, could expand
 *	/0		errors in processing list
 * GLOBAL
 *	pSkipListStart
 * DESPRIPTION
 *	Expands token in list to absolute path
 * REMARKS
 */
int
ExpandXList(char *list)
{
    char		*pToken;
    char		 path[_MAX_PATH];
    char		*pSrcPath, *pFileMask;
    struct skiplistentry_t	*pSkipEntry = pSkipListStart;
    FILEFINDBUF3 	 findBuffer;
    USHORT		 rc;
    HDIR		 hSearch;
    ULONG		 cFound;

    /* Set pSkipEntry to last Entry in list
     * List should be empty anyway! */

    if( pSkipEntry )
    {
	while( pSkipEntry->next )
	    pSkipEntry = pSkipEntry->next;
    }

    /* Set up for absolute path names */

    strcpy( path, szSrcPath );
    pSrcPath = path + strlen(path);

    /* Scan for tokens in list.
     * Tokens are seperated by ';', like %PATH% or %DPATH% */

    if( (pToken=strtok(list, ";")) == NULL )
	return 1;
    do
    {
	{
	    char *s, *d;
	    for( s = pToken, d = pSrcPath;
		(*d = *s); ++s, ++d )
		if( *d == '\\' )
		    *d = '/';
	}
	Verbose(3,"Filling SkipList whith %s", path );

	hSearch = 1;
        cFound  = 1;
	rc = DosFindFirst( path, &hSearch, FILE_NORMAL|FILE_DIRECTORY,
			  &findBuffer, sizeof(findBuffer), &cFound, FIL_STANDARD );
	if( rc )
	{
	    Verbose(2,"DosFindFirst(%s) - rc %u", path, rc );
	    continue;
	}
	if( (pFileMask=strrchr(path, '/')) == NULL )
	{
	    Verbose(1,"Error in search mask, cannot find '/'");
	    continue;
	}
	++pFileMask;
	do
	{
	    if( !strcmp(findBuffer.achName, ".")
	       ||  !strcmp(findBuffer.achName, "..") )
		continue;
	    Verbose(3,"Found: %s\tattr: %#x",
		    findBuffer.achName, findBuffer.attrFile );

	    if( pSkipEntry )
	    {
		pSkipEntry->next = malloc( sizeof(struct skiplistentry_t) );
		assert( pSkipEntry->next != NULL );
		pSkipEntry = pSkipEntry->next;
	    }
	    else
	    {
		pSkipListStart = malloc( sizeof(struct skiplistentry_t) );
		assert( pSkipListStart != NULL );
                pSkipEntry = pSkipListStart;
	    }
	    pSkipEntry->next = NULL;
	    strcpy( pFileMask, findBuffer.achName );
	    strcpy( pSkipEntry->name, path );
	}
	while( !DosFindNext(hSearch, &findBuffer, sizeof(findBuffer), &cFound) );
    }
    while( (pToken=strtok(NULL, ";")) != NULL );

    return 0;
}




void
usage(void)
{
    printf("%s [d:]\\[path1] [d:]\\[path2] [-?][-v][-o][-l][-x:name[;name]]\n", ProgName );
    return;
}




void
help(void)
{
    printf("\n%s\tEMX Application", ProgName);
    printf("\t\t\t\tVersion %d.%02u\t%s\n", VERSION, SUBVERSION, __DATE__ );
    printf("%s replaces XCOPY /s/e", ProgName );
    printf("\t\t\tby Vitus Jensen 1993-97\n");
    printf("\n%s copies a whole directory tree to another location,\n"
	   "empty directories will be copied, EAs will be copied (if possible).\n"
	   "The destination directory will be created if it doesn't exists.\n"
	   "OBS: by default no files will be overwritten!\n"
	   "\n", ProgName );
    usage();
    printf("\n"
	   "  path1\t\tsource tree (this is a directory name!)\n"
	   "  path2\t\tdestination directory\n"
	   "  -?h\t\tthis text\n"
	   "  -v[:n]\tVerbose level\n"
	   "  -l\t\tLog filenames to screen\n"
	   "  -o\t\tOverwrite files in destination tree\n"
	   "  -x\t\teXclude these files and directories (may be relative to path1)\n"
	   "\nOptions may be placed anywhere on the command line\n");
    return;
}




/*#
 * NAME
 *	ExamineArgs
 * CALL
 *	ExamineArgs(argc,argv)
 * PARAMETER
 *	argc
 *	argv		as to main()
 * RETURNS
 *	nothing
 * GLOBAL
 *	szSrcPath
 *	szDstPath
 * DESPRIPTION
 *	Gets and checks all program parameter
 * REMARKS
 */
void
ExamineArgs(int argc,char *argv[])
{
    int  flSrcDefined=0;
    int  flDstDefined=0;
    char List[_MAX_PATH];

    _splitpath( argv[0], List, List, ProgName, List );
    if( argc == 1 )
    {
        usage();
        exit(1);
    }
    List[0] = '\0';
    while( argc > 1 )
    {
	if( argv[1][0] == '-' )
	{
	    switch( argv[1][1] )
	    {
	      case '?':
	      case 'h':
		help();
		exit(0);

	      case 'v':
		if( argv[1][2] == '\0' )
		    SetLoglevel( 2 );
		else if( argv[1][2] == ':' )
		{
		    unsigned	new;

		    sscanf( &argv[1][3], " %u", &new );
		    SetLoglevel( new );
		}
		break;

	      case 'o':
		fOverwrite = 1;
		break;

	      case 'l':
		fDisplayNames = 1;
		break;

	      case 'x':
		if( argv[1][2] != ':' )
		{
		    Verbose(0,"%s: Syntax error in SkipList (':')", ProgName );
		    exit(1);
		}
		if( argv[1][3] == '@' )
		{
		    Verbose(0,"%s: -x:@listfile not yet implemented", ProgName );
		    exit(1);
		}
		else
		{
		    strcpy( List, &argv[1][3] );
		}
		break;

	      default:
		Verbose(0,"%s: unknown arg %s", ProgName, argv[1] );
		exit(1);
	    }
	}
	else /*Should be source or destination path*/
	{
	    if( !flSrcDefined )
	    {
		strcpy( szSrcPath, argv[1] );
		flSrcDefined = 1;
	    }
	    else if( !flDstDefined )
	    {
		strcpy( szDstPath, argv[1] );
		flDstDefined = 1;
	    }
	    else
	    {
		Verbose(0,"%s: unknown arg %s", ProgName, argv[1] );
		exit(1);
	    }
	}
	--argc;
	++argv;
    }

    /* Check source and destination path */

    if( !flSrcDefined  ||  !flDstDefined )
    {
	Verbose(0,"%s: have to define source and destination path", ProgName );
	exit(1);
    }

    if( ExpandPath( szSrcPath, 0) )
    {
	Verbose(0,"%s: error in source path", ProgName );
	exit(1);
    }
    if( ExpandPath( szDstPath, 1) )
    {
	Verbose(0,"%s: error in destination path", ProgName );
	exit(1);
    }
    if( strlen(List)  &&  ExpandXList(List) )
    {
	Verbose(0,"%s: error in exclude list", ProgName );
	exit(1);
    }
    return;
}




/*#
 * NAME
 * CALL
 * PARAMETER
 * RETURNS
 * GLOBAL
 *	szSrcPath
 *	szDstPath
 * DESPRIPTION
 *	Main function of this program
 * REMARKS
 */
int
main(int argc,char *argv[])
{
    int		rc;
    struct skiplistentry_t *pSkipEntry, *pSkipLast;

    /* CmdLine, creates destination directory */

    ExamineArgs( argc, argv );

    Verbose(3,"Starting file copy (overwrite=%u)", fOverwrite );

    rc = CopyTree( szSrcPath, szDstPath );

    /* Free skip list, clean coding;
     * no dirty tricks in this program! */

    if( (pSkipEntry=pSkipListStart) )
    {
        while( pSkipEntry->next )
	{
            pSkipLast   = pSkipEntry;
            pSkipEntry  = pSkipEntry->next;
            free( pSkipLast );
	}
        free( pSkipEntry );
    }

    return rc;					/* tell OS: OK */
}
