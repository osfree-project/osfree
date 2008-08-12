// JPOS2DLL.C - utility DLL for 4OS2; includes KEYSTACK support
// Copyright (c) 1996 - 1997  Rex C. Conn

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "4all.h"

#define INCL_BASE 1
#define INCL_PM 1
#include <osfree.h>


#define MAX_VIRT_KEY_CODES  1024

INT APIENTRY SendKeys( PSZ );
VOID APIENTRY StartHook( HMQ );
VOID APIENTRY PauseKeys(BOOL);
VOID APIENTRY QuitSendKeys(void);
ULONG EXPENTRY KeystackHookProc( HAB, BOOL, PQMSG );

#define SK_NOERROR 0
#define SK_INVALIDKEY 1
#define SK_MISSINGCLOSEPAREN 2
#define SK_INVALIDCOUNT 3
#define SK_STRINGTOOLONG 4
#define SK_CANTINSTALLHOOK 5

#define FBIT 256

#define SHIFT_TAB       15+FBIT
#define ALT_TAB         165+FBIT
#define CTL_TAB         148+FBIT

#define F1              59+FBIT         // function keys
#define F2              60+FBIT
#define F3              61+FBIT
#define F4              62+FBIT
#define F5              63+FBIT
#define F6              64+FBIT
#define F7              65+FBIT
#define F8              66+FBIT
#define F9              67+FBIT
#define F10             68+FBIT
#define F11             0x85+FBIT
#define F12             0x86+FBIT
#define SHFT_F1         84+FBIT
#define SHFT_F2         85+FBIT
#define SHFT_F3         86+FBIT
#define SHFT_F4         87+FBIT
#define SHFT_F5         88+FBIT
#define SHFT_F6         89+FBIT
#define SHFT_F7         90+FBIT
#define SHFT_F8         91+FBIT
#define SHFT_F9         92+FBIT
#define SHFT_F10        93+FBIT
#define SHFT_F11        0x87+FBIT
#define SHFT_F12        0x88+FBIT
#define CTL_F1          94+FBIT
#define CTL_F2          95+FBIT
#define CTL_F3          96+FBIT
#define CTL_F4          97+FBIT
#define CTL_F5          98+FBIT
#define CTL_F6          99+FBIT
#define CTL_F7          100+FBIT
#define CTL_F8          101+FBIT
#define CTL_F9          102+FBIT
#define CTL_F10         103+FBIT
#define CTL_F11         0x89+FBIT
#define CTL_F12         0x8A+FBIT
#define ALT_F1          104+FBIT
#define ALT_F2          105+FBIT
#define ALT_F3          106+FBIT
#define ALT_F4          107+FBIT
#define ALT_F5          108+FBIT
#define ALT_F6          109+FBIT
#define ALT_F7          110+FBIT
#define ALT_F8          111+FBIT
#define ALT_F9          112+FBIT
#define ALT_F10         113+FBIT
#define ALT_F11         0x8B+FBIT
#define ALT_F12         0x8C+FBIT
#define HOME            71+FBIT
#define CUR_UP          72+FBIT
#define PGUP            73+FBIT
#define CUR_LEFT        75+FBIT
#define CUR_RIGHT       77+FBIT
#define END             79+FBIT
#define CUR_DOWN        80+FBIT
#define PGDN            81+FBIT
#define INS             82+FBIT
#define DEL             83+FBIT
#define CTL_LEFT        115+FBIT
#define CTL_RIGHT       116+FBIT
#define CTL_END         117+FBIT
#define CTL_PGDN        118+FBIT
#define CTL_HOME        119+FBIT
#define CTL_PGUP        132+FBIT
#define CTL_UP          141+FBIT
#define CTL_DOWN        145+FBIT

#define SHIFT_LEFT      200+FBIT        // dummy values for TCMD
#define SHIFT_RIGHT     201+FBIT
#define SHIFT_HOME      202+FBIT        // dummy values for TCMD
#define SHIFT_END       203+FBIT
#define CTL_SHIFT_LEFT  204+FBIT
#define CTL_SHIFT_RIGHT 205+FBIT


// Application global variables

struct {
        USHORT usFlags;
        UCHAR  ucRepeat;
        UCHAR  ucScanCode;
        USHORT usChar;
        USHORT usVKey;
} uaKeys[MAX_VIRT_KEY_CODES];


UINT uKSIndex;                  // Index into uaKeys.
UINT uMaxKSIndex;               // Max entries in uaKeys.
UINT nIndex;                    // Index of char in the string

BOOL fDelayKeys;
BOOL bKeyFirstTime;             // The first time in the hook
BOOL bKeyUp;                    // Has the key Down been simulated
const char * pszKeys;

HMQ ghmq;
QMSG qmsgCurrent;               // Message to simulate

LONG lTime,lTimeTillNext,lLastTime;   // Time to wait for key playback


struct {
        UINT uVirtKey;
        const char *szKeyName;
        USHORT uScanCode;
        USHORT uChar;
} g_SpecialKeys[] = {
        { VK_MENU,      "ALT", 0x38, 0    },
        { VK_MENU,      "MENU", 0x38, 0   },
        { VK_CTRL,      "CTRL", 0x1D, 0   },
        { VK_SHIFT,     "SHIFT", 0x2A, 0  },
        { VK_BREAK,     "CANCEL", 0, 0    },
        { VK_TAB,       "TAB", 0, 0x09    },
        { VK_CLEAR,     "CLEAR", 0, 0     },
        { VK_ENTER,     "ENTER", 0, 0x0D  },
        { VK_ENTER,     "RETURN", 0, 0x0D },
        { VK_PAUSE,     "PAUSE", 0x5F, 0  },
        { VK_CAPSLOCK,  "CAPSLOCK", 0x3A, 0  },
        { VK_ESC,       "ESCAPE", 1, 0x1B },
        { VK_ESC,       "ESC", 1, 0x1B    },
        { VK_NUMLOCK,   "NUMLOCK", 0x45, 0   },
        { VK_SCRLLOCK,  "SCROLLLOCK", 0x46, 0 },
        { VK_CTRL,      "BREAK", 0, 0     },
        { VK_BACKSPACE, "BACKSPACE", 0x0E, 0x08 },
        { VK_BACKSPACE, "BS", 0x0E, 0x08  },
        { VK_BACKSPACE, "BKSP", 0x0E, 0x08      },
        { VK_DELETE,    "DELETE", 0x53, 0 },
        { VK_DELETE,    "DEL", 0x53, 0    },
        { VK_INSERT,    "INSERT", 0x52, 0 },
        { VK_INSERT,    "INS", 0x52, 0    },
        { VK_LEFT,      "LEFT", 0x4B, 0   },
        { VK_RIGHT,     "RIGHT", 0x4D, 0  },
        { VK_UP,        "UP", 0x48, 0     },
        { VK_DOWN,      "DOWN", 0x50, 0   },
        { VK_PAGEUP,    "PGUP", 0x49, 0   },
        { VK_PAGEDOWN,  "PGDN", 0x51, 0   },
        { VK_HOME,      "HOME", 0x47, 0   },
        { VK_END,       "END", 0x4F, 0    },
        { VK_F1,        "F1", 0x3B, 0     },
        { VK_F2,        "F2", 0x3C, 0     },
        { VK_F3,        "F3", 0x3D, 0     },
        { VK_F4,        "F4", 0x3E, 0     },
        { VK_F5,        "F5", 0x3F, 0     },
        { VK_F6,        "F6", 0x40, 0     },
        { VK_F7,        "F7", 0x41, 0     },
        { VK_F8,        "F8", 0x42, 0     },
        { VK_F9,        "F9", 0x43, 0     },
        { VK_F10,       "F10", 0x44, 0    },
        { VK_F11,       "F11", 0x57, 0    },
        { VK_F12,       "F12", 0x58, 0    },
        { 0xFF,         NULL, 0, 0        }
};


typedef struct
{
        char VirtKey;           // virtual key code
        char ScanKey;           // scan code
        char ShiftKey;          // shifted key code
        char CtlKey;            // ^key code
        char AltKey;            // Alt-key code
} KEYCODES;

// translate Shift, Ctl, and Alt extended keys
KEYCODES aKeyCodes[] = {
        VK_PAGEUP, 73, 0, 0x84, 0,
        VK_PAGEDOWN, 81, 0, 0x76, 0,
        VK_END, 79, 0xCB, 0x75, 0,
        VK_HOME, 71, 0xCA, 0x77, 0,
        VK_LEFT, 75, 0xC8, 0x73, 0,
        VK_UP, 72, 0, 0x8D, 0,
        VK_RIGHT, 77, 0xC9, 0x74, 0,
        VK_DOWN, 80, 0, 0x91, 0,
        VK_INSERT, 82, 0, 0x92, 0,
        VK_DELETE, 83, 0, 0x93, 0,
        VK_F1, 0x3B, 0x54, 0x5E, 0x68,
        VK_F2, 0x3C, 0x55, 0x5F, 0x69,
        VK_F3, 0x3D, 0x56, 0x60, 0x6A,
        VK_F4, 0x3E, 0x57, 0x61, 0x6B,
        VK_F5, 0x3F, 0x58, 0x62, 0x6C,
        VK_F6, 0x40, 0x59, 0x63, 0x6D,
        VK_F7, 0x41, 0x5A, 0x64, 0x6E,
        VK_F8, 0x42, 0x5B, 0x65, 0x6F,
        VK_F9, 0x43, 0x5C, 0x66, 0x70,
        VK_F10, 0x44, 0x5D, 0x67, 0x71,
        VK_F11, 0x85, 0x87, 0x89, 0x8B,
        VK_F12, 0x86, 0x88, 0x8A, 0x8C,
        0, 0, 0, 0
};


// Function to preprocess the next token in the input string.
static int xPreprocessKeys(void)
{
        char cChar;
        char *arg, szSpecialKey[32], szBuf[32];
        UINT i, uKey, uDelay, uCount;
        int SendKeysErr;
        COUNTRYCODE aCountryCode = {0, 0};

        // Get the next non-space character from the input string.
        for ( ; (( *pszKeys == ' ' ) || ( *pszKeys == '-' )); pszKeys++ )
                ;

        if (( cChar = *pszKeys ) == '\0' )
                return 0;

        // if !, clear the buffer
        if ( cChar == '!' ) {
                // initialize the KEYSTACK string
                memset( (char *)&uaKeys, 0, sizeof(uaKeys) );
                uMaxKSIndex = 0;
                return 0;
        }

        if (( uMaxKSIndex + 1 ) >= sizeof(uaKeys) )
                return SK_STRINGTOOLONG;

        // if /W, check for delay requested
        if (( cChar == '/' ) && (( pszKeys[1] == 'W' ) || ( pszKeys[1] == 'w' ))) {

                for ( pszKeys += 2; ( *pszKeys == ' ' ); pszKeys++ )
                        ;
                for ( uDelay = 0; (( *pszKeys >= '0' ) && ( *pszKeys <= '9' )); pszKeys++ )
                        uDelay = ( uDelay * 10 ) + ( *pszKeys - '0' );

                // delay is in milliseconds
                uaKeys[uMaxKSIndex].usChar = 0xFFFF;
                uaKeys[uMaxKSIndex].usVKey = uDelay;
                goto RepeatCount;
        }

        if ( cChar == '"' ) {

            // sending a character string
            pszKeys++;
            for ( i = 0; ((( cChar = *pszKeys ) != '"' ) && ( cChar != '\0' )); pszKeys++ ) {

                uaKeys[ uMaxKSIndex ].usChar = cChar;
                uKey = cChar;
                DosMapCase( 1, &aCountryCode, (PCHAR)&uKey );
                if ( cChar == uKey )
                        uaKeys[uMaxKSIndex].usFlags |= KC_SHIFT;
                uaKeys[ uMaxKSIndex ].usVKey = 0;
                uaKeys[ uMaxKSIndex ].ucScanCode = 0;
                uaKeys[ uMaxKSIndex++ ].usFlags |= KC_CHAR;
                i++;
            }

            if ( i != 0 )
                uMaxKSIndex--;

            if ( *pszKeys == '"' )
                pszKeys++;

            goto RepeatCount;
        }

        // Beginning of sub-group?
        if ( cChar == '(' ) {

                // While not at the end of the input string and not at a close paren.
                pszKeys++;
                while (( *pszKeys != '\0' ) && ( *pszKeys != ')' )) {
                        // Add the next character to the array.
                        if (( SendKeysErr = xPreprocessKeys()) != SK_NOERROR )
                                return SendKeysErr;
                }

                if ( *pszKeys == 0 )
                        return SK_MISSINGCLOSEPAREN;

                pszKeys++;      // Skip past the close paren.
                return 0;
        }

        // Locate the end of the first token
        for ( i = 0; (( i < 31 ) && ( *pszKeys != '\0' ) && ( *pszKeys != ' ' ) && ( *pszKeys != '-' )); i++, pszKeys++ )
                szSpecialKey[i] = *pszKeys;
        szSpecialKey[i] = '\0';

        if (( szSpecialKey[0] >= '0' ) && ( szSpecialKey[0] <= '9' )) {

                for ( uaKeys[ uMaxKSIndex ].usChar = 0, arg = szSpecialKey; (( *arg >= '0' ) && ( *arg <= '9' )); arg++ )
                        uaKeys[ uMaxKSIndex ].usChar = ( uaKeys[ uMaxKSIndex ].usChar * 10 ) + ( *arg - '0' );
                uaKeys[ uMaxKSIndex ].usVKey = 0;
                uaKeys[ uMaxKSIndex ].ucScanCode = 0;
                uaKeys[ uMaxKSIndex ].usFlags |= KC_CHAR;
                goto RepeatCount;

        } else {

                // Scan the array and compare each of the possible strings.
                strcpy( szBuf, szSpecialKey );
                (void)WinUpper( 0, 0, 0, szBuf );

                // if the special key is found, set the QMSG fields
                for ( i = 0; ( g_SpecialKeys[i].szKeyName != NULL ); i++ ) {

                        if ( strcmp( szBuf, g_SpecialKeys[i].szKeyName ) == 0 ) {
                                uaKeys[ uMaxKSIndex ].usVKey = g_SpecialKeys[i].uVirtKey;
                                if (( uaKeys[ uMaxKSIndex ].usChar = g_SpecialKeys[i].uChar) != 0 )
                                        uaKeys[ uMaxKSIndex ].usFlags |= KC_CHAR;
                                if (( uaKeys[ uMaxKSIndex ].ucScanCode = (UCHAR)g_SpecialKeys[i].uScanCode) != 0 )
                                        uaKeys[ uMaxKSIndex ].usFlags |= KC_SCANCODE;
                                uaKeys[ uMaxKSIndex ].usFlags |= KC_VIRTUALKEY;
                                break;
                        }
                }

                if ( g_SpecialKeys[i].szKeyName == NULL ) {

                        // If the special key is a single character, convert it
                        //   to its character code equivalent.
                        if ( strlen( szSpecialKey ) == 1 ) {
                                uaKeys[ uMaxKSIndex ].usChar = szSpecialKey[0];
                                uaKeys[ uMaxKSIndex ].ucScanCode = 0;
                                uaKeys[ uMaxKSIndex ].usVKey = 0;
                                uaKeys[ uMaxKSIndex ].usFlags &= ~(KC_VIRTUALKEY | KC_SCANCODE);
                                if (( uaKeys[ uMaxKSIndex ].usFlags & (KC_ALT | KC_CTRL)) == 0 )
                                        uaKeys[ uMaxKSIndex ].usFlags |= KC_CHAR;
                                goto RepeatCount;
                        }

                        return SK_INVALIDKEY; // Error, special key not found
                }
        }

        if ( *pszKeys == '-' )
                pszKeys++;

        i = uaKeys[ uMaxKSIndex ].usVKey;
        if (( i == VK_SHIFT ) || ( i == VK_CTRL ) || ( i == VK_MENU )) {

                if ( i == VK_SHIFT )
                        uaKeys[ uMaxKSIndex ].usFlags |= KC_SHIFT;
                else if ( i == VK_CTRL )
                        uaKeys[ uMaxKSIndex ].usFlags |= KC_CTRL;
                else
                        uaKeys[ uMaxKSIndex ].usFlags |= KC_ALT;

                if (( SendKeysErr = xPreprocessKeys()) != SK_NOERROR)
                        return SendKeysErr;
                uMaxKSIndex--;

                // check for Shift/Ctrl/Alt extended key
                for ( i = 0, uKey = 0; ( aKeyCodes[i].VirtKey != 0 ); i++ ) {

                    if ( uaKeys[ uMaxKSIndex ].usVKey == aKeyCodes[i].VirtKey ) {

                        if ( uaKeys[ uMaxKSIndex ].usFlags & KC_SHIFT ) {
                            // check for Ctrl-Shift-Left & Right
                            if (( uaKeys[ uMaxKSIndex ].usFlags & KC_CTRL) && (( uaKeys[ uMaxKSIndex ].usVKey == VK_LEFT ) || ( uaKeys[ uMaxKSIndex ].usVKey == VK_RIGHT )))
                                uKey = (( uaKeys[ uMaxKSIndex ].usVKey == VK_LEFT ) ? ( CTL_SHIFT_LEFT >> 8 ) : ( CTL_SHIFT_RIGHT >> 8 ));
                            else
                                uKey = aKeyCodes[i].ShiftKey;

                        } else if ( uaKeys[ uMaxKSIndex ].usFlags & KC_CTRL )
                            uKey = aKeyCodes[i].CtlKey;

                        else if ( uaKeys[ uMaxKSIndex ].usFlags & KC_ALT )
                            uKey = aKeyCodes[i].AltKey;

                        break;
                    }
                }

                // store the scan code in the high byte, 0 in the low
                if ( uKey != 0 )
                        uaKeys[ uMaxKSIndex ].usChar = ( uKey << 8 );

        } else if ( uaKeys[ uMaxKSIndex ].usChar == 0 )
                uaKeys[ uMaxKSIndex ].usChar = (((USHORT)uaKeys[ uMaxKSIndex].ucScanCode) << 8);

RepeatCount:
        // Just a special word/character without a repeat count.
        uaKeys[ uMaxKSIndex ].ucRepeat = 1;

        // Get the next non-space character from the input string.
        for ( ; (( *pszKeys == ' ' ) || ( *pszKeys == '-' )); pszKeys++ )
                ;

        // Calculate the repeat count for this special character.
        if ( *pszKeys == '[' ) {

                pszKeys++;
                for ( uCount = 0; ( *pszKeys != '0' ); ) {
                        if (( *pszKeys >= '0' ) && ( *pszKeys <= '9' ))
                                uCount = ( uCount * 10 ) + ( *pszKeys - '0' );
                        if ( *pszKeys++ == ']' )
                                break;
                }

                if ( uCount == 0 )
                        return SK_INVALIDCOUNT;         // Error, invalid count value

                while ( --uCount > 0 ) {
                        memmove( &uaKeys[uMaxKSIndex+1], &uaKeys[uMaxKSIndex], sizeof(uaKeys[0]) );
                        uMaxKSIndex++;
                }
        }

        uMaxKSIndex++;

        return 0;
}


int APIENTRY PreprocessKeys( const char * lpszPassedKeys )
{
        int SKError;

        // Clear out the array and the index into the array.
        uMaxKSIndex = uKSIndex = 0;
        fDelayKeys = 0;
        bKeyFirstTime = TRUE;
        bKeyUp = 0;

        // Use an internal variable for input string parsing.
        pszKeys = lpszPassedKeys;

        // While there are more characters in the string to be parsed,
        // call the preprocessor to evaluate the next token.
        while ( *pszKeys != '\0' ) {
                if (( SKError = xPreprocessKeys()) != SK_NOERROR )
                        return SKError;
        }

        return 0;
}


INT APIENTRY SendKeys( char *lpszKeys )
{
        int SKError;

        // cancel any existing KEYSTACK playback hook
        QuitSendKeys();

        memset( (char *)&uaKeys, 0, sizeof(uaKeys) );

        if (( SKError = PreprocessKeys( lpszKeys )) != SK_NOERROR )
                return SKError;

        return 0;
}


VOID APIENTRY StartHook( HMQ hmq )
{
        ghmq = hmq;
}


// suspend / restart KEYSTACK'ing
VOID APIENTRY PauseKeys( BOOL fPause )
{
        fDelayKeys = fPause;
}


// turn off keystacking (probably called by ^C in Take Command)
VOID APIENTRY QuitSendKeys( void )
{
        nIndex = uMaxKSIndex;
}


// This is the Journal Playback hook callback function.  Every time it is
// called, PM is requesting the next keyboard event to be played back.
// This function fills a QMSG structure with the correct information about
// the keyboard event and playback time and returns to let OS/2 process it.
// After all events have been played back, the function uninstalls itself.
ULONG EXPENTRY KeystackHookProc( HAB hab, BOOL fSkip, PQMSG pQmsg )
{
        if ( bKeyFirstTime == TRUE ) {

                // initialize playback info
                bKeyUp = FALSE;         // Do not play the keyup yet
                nIndex = 0;
                lTime = 0L;
                lLastTime = WinGetCurrentTime( hab );

                lTimeTillNext = 0L;
                while ( uaKeys[ nIndex ].usChar == 0xFFFF )
                        lTimeTillNext += uaKeys[ nIndex++ ].usVKey;

                // get the first character to play and play it with key down
                memset( &qmsgCurrent, '\0', sizeof(QMSG) );
                qmsgCurrent.msg = WM_CHAR;
                qmsgCurrent.mp1 = MPFROMSH2CH( uaKeys[ nIndex ].usFlags, 0x01, uaKeys[ nIndex ].ucScanCode );
                qmsgCurrent.mp2 = MPFROM2SHORT( uaKeys[ nIndex ].usChar, uaKeys[ nIndex ].usVKey );

                bKeyUp = TRUE;          // Play the key up message next
                bKeyFirstTime = FALSE;
        }

        if ( fSkip == 0 ) {             // If not going to next msg

                lTime = lTimeTillNext - WinGetCurrentTime( hab ) + lLastTime;
                if ( lTime < 0L ) {
                        lTime = 0L;
                        lTimeTillNext = 0L;
                }

                // Copy over msg so it is played by PM
                if ( pQmsg )
                        memmove( pQmsg, &qmsgCurrent, sizeof(QMSG) );
                return ( lTime );
        }

        // Read next record
        while ( uaKeys[ nIndex ].usChar == 0xFFFF )
                lTimeTillNext += uaKeys[ nIndex++ ].usVKey;

        lLastTime = WinGetCurrentTime( hab );

        if ( bKeyUp ) {

                bKeyUp = FALSE;

                qmsgCurrent.mp1 = MPFROMSH2CH( (KC_LONEKEY | KC_KEYUP | (uaKeys[ nIndex ].usFlags & ~KC_CHAR)), 0x01, uaKeys[ nIndex ].ucScanCode );
                qmsgCurrent.mp2 = MPFROM2SHORT( uaKeys[ nIndex ].usChar, uaKeys[ nIndex ].usVKey );
                nIndex++;

        } else if ( nIndex >= uMaxKSIndex ) {

                // end of string - remove hook
                (void)WinPostQueueMsg( ghmq, WM_USER, 0L, 0L );

                memset( &qmsgCurrent, '\0', sizeof(QMSG) );
                qmsgCurrent.msg = WM_MOUSEMOVE;
                if ( pQmsg ) {
                        qmsgCurrent.ptl.x = pQmsg->ptl.x;
                        qmsgCurrent.ptl.y = pQmsg->ptl.y;
                }

                return 25L;

        } else {
                qmsgCurrent.mp1 = MPFROMSH2CH( uaKeys[ nIndex ].usFlags, 0x01, uaKeys[ nIndex ].ucScanCode );
                qmsgCurrent.mp2 = MPFROM2SHORT( uaKeys[ nIndex ].usChar, uaKeys[ nIndex ].usVKey );
                bKeyUp = TRUE;
        }

        return 0L;
}



