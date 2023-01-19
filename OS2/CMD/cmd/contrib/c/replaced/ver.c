/* Replaced by osFree team implementation

// print 4OS2 version number
int ver_cmd( int argc, char **argv )
{
    extern int ExternalCommands( char *, char * );

    if ( ( argc = switch_arg( argv[1], "R" )) < 0 )
        return( usage( VER_USAGE ));

    printf( "\n4OS2  %u.%02u%s     OS/2 Version is %s\n", VER_MAJOR, VER_MINOR, VER_REVISION, gszOsVersion );

    if ( argc == 1 ) {
#ifdef __DEBUG__
        printf( OS2_REVISION, VER_MAJOR, VER_MINOR, VER_REVISION, VER_BUILD, "DEBUG " );
#else
        printf( OS2_REVISION, VER_MAJOR, VER_MINOR, VER_REVISION, VER_BUILD, "" );
#endif
        printf( COPYRIGHT );
        printf( COPYRIGHT2 );
    }

    return 0;
}

*/
