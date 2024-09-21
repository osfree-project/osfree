#include "winemu.h"

LPSTR WINAPI lstrcpy( LPSTR s, LPCSTR t )
{
    LPSTR dst;
    dst = s;
    while( *dst++ = *t++ )
        ;
    return( s );
}

int WINAPI lstrlen( LPCSTR s )
{
    LPCSTR p;

    p = s;
    while( *p != '\0' )
        ++p;
    return( p - s );

}
