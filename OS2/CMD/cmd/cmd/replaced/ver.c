/* Replaced by osFree team implementation
// print 4OS2 version number
int ver_cmd( int argc, char **argv )
{
        extern int ExternalCommands( char *, char * );

        if (( argc = switch_arg( argv[1], "R" )) < 0 )
                return ( usage( VER_USAGE ));

        printf( OS2_VERSION, PROGRAM, gszOsVersion );

        if ( argc == 1 ) {
                printf( OS2_REVISION, VER_REVISION, VER_BUILD, gnOS2_Revision );
                printf( COPYRIGHT );
                printf( COPYRIGHT2 );
        }

        return 0;
}

*/
