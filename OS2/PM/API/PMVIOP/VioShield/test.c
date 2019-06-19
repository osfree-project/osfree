#define INCL_KBD
#include <os2.h>

#include <stdio.h>
#include <string.h>

#define SH_LCTRL_DOWN       0x0100
#define SH_LALT_DOWN        0x0200
#define SH_RCTRL_DOWN       0x0400
#define SH_RALT_DOWN        0x0800

#define DDF_KEYTYPE     0x003F
#define DDF_BREAK       0x0040
#define DDF_SECONDARY   0x0080
#define DDF_MULTITAKE   0x0100
#define DDF_ACCENTED    0x0200

#define ST_SHIFT_WITHOUT_CHAR      0x01
#define ST_EXTENDED_KEY            0x02
#define ST_IMM_CONVERSION_REQ      0x20
#define ST_BIT6_7                  ( 0x80 | 0x40 )
#define ST_UNDEFINED               0x00
#define ST_FINAL_INTERIM_OFF       0x40
#define ST_INTERIM                 0x80
#define ST_FINAL_INTERIM_ON        ( 0x80 | 0x40 )

int main( void )
{
    KBDKEYINFO ki;
    CHAR       result[ 256 ];

    printf("Press ESC to exit\n");

    do
    {
        KbdCharIn( &ki, IO_WAIT, 0 );

        sprintf(result,"Char=0x%02x,Scan=0x%02x,Status=0x%04x,State=0x%04x, time=%lu\n",
                ki.chChar,ki.chScan,ki.fbStatus,
                ki.fsState, ki.time);

        if( ki.fsState & SH_LCTRL_DOWN )
            sprintf( result + strlen( result ),"LCTRL\n");
        if( ki.fsState & SH_LALT_DOWN )
            sprintf( result + strlen( result ),"LALT\n");
        if( ki.fsState & SH_RCTRL_DOWN )
            sprintf( result + strlen( result ),"RCTRL\n");
        if( ki.fsState & SH_RALT_DOWN )
            sprintf( result + strlen( result ),"RALT\n");

        if( ki.fbStatus & ST_SHIFT_WITHOUT_CHAR )
            sprintf( result + strlen( result ),"Shift without Char\n");
        if( ki.fbStatus & ST_EXTENDED_KEY )
            sprintf( result + strlen( result ),"Extended key\n");
        else
            sprintf( result + strlen( result ),"Character key\n");
        if( ki.fbStatus & ST_IMM_CONVERSION_REQ )
            sprintf( result + strlen( result ),"Immediate conversion request\n");

        switch( ki.fbStatus & ST_BIT6_7 )
        {
            case ST_FINAL_INTERIM_OFF :
                sprintf( result + strlen( result ),"Final character without interim\n");
                break;

            case ST_FINAL_INTERIM_ON :
                sprintf( result + strlen( result ),"Final character with interim\n");
                break;

            case ST_INTERIM :
                sprintf( result + strlen( result ),"Interim character\n");
                break;

            default :
                sprintf( result + strlen( result ),"Undefined\n");
                break;
        }

        printf("%s-----\n", result);
    } while( ki.chScan != 1 );

    return 0;
}

