/* Replaced by osFree team implementation

// clear the screen (using ANSI), with optional color set
int cls_cmd( int argc, char **argv )
{
    int rval = 0;
    VIOOVERSCAN overscan;

    if ( argc > 1 ) {

        if ( ( rval = color_cmd( argc, argv )) != 0 )
            return rval;

    } else if ( gpIniptr->StdColor != 0 ) {

        set_colors( gpIniptr->StdColor & 0xFF );

        // check for default border (high byte)
        if ( ( overscan.color = ( gpIniptr->StdColor >> 8 )) != 0xFF ) {
            overscan.cb = sizeof( overscan );
            overscan.type = 1;
            (void)VioSetState( &overscan, 0 );
        }
    }

    // 4OS2 forces ANSI compatibility, so we _know_ this will work!
    printf( "\033[2J" );

    return rval;
}
*/
