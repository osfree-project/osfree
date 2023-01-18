// WINDOW.C - popup windows for 4os2 family
//   Copyright (c) 1991 - 1997 Rex C. Conn

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "4all.h"


#define SORT_LIST 1
#define NO_CHANGE 2
#define CDD_COLORS 4
#define BATCH_DEBUG 0x10

static void ScrollBar( int, int );
static char * wSelect( char * *, int, int, char * );

static POPWINDOWPTR aWin[4];        // array of active windows
static POPWINDOWPTR win = NULL;     // pointer to current window
static int nCurrentWindow = -1;
int gnPopExitKey;
int gnPopupSelection;
static unsigned int uDefault, uNormal, uInverse;
static char szSearch[32];
char *pszBatchDebugLine;


// Open a window with the specified coordinates.
POPWINDOWPTR wOpen( int top, int left, int bottom, int right, int attribute, char *title, char *pszBottomTitle )
{
    POPWINDOWPTR wn;
    unsigned int window_size;
    VIO_UTYPE width;
    char * fptr;

    wn = (POPWINDOWPTR)malloc( sizeof(POPWINDOW) );

    wn->fShadow = ( right < (int)GetScrCols() - 1 );

    // get window size, including drop shadow (1 row & 2 columns)
    width = ((( right + (( wn->fShadow ) ? 3 : 1 )) - left ) * 2 );
    window_size = ((( bottom + wn->fShadow + 1 ) - top ) * width ) + 2;

    if (( wn->screen_save = AllocMem( &window_size )) == 0L ) {
        free( wn );
        return NULL;
    }

    // store parameters in window control block
    wn->top = top;
    wn->left = left;
    wn->bottom = bottom;
    wn->right = right;

    wn->attrib = attribute;
    wn->hoffset = 0;
    wn->c_row = wn->c_col = 0;

    // save old cursor position
    GetCurPos( &(wn->old_row), &(wn->old_col) );

    // Copy existing text where window will be placed to the save buffer.
    for ( fptr = wn->screen_save; ( top <= ( bottom + wn->fShadow )); top++ ) {

        ReadCellStr( fptr, width, top, left );
        fptr += width;
    }

    // draw the window border (with a shadow) and clear the text area
    _box( wn->top, left, bottom, right, 1, wn->attrib, wn->attrib, wn->fShadow, 0 );

    if ( strlen( title ) >= (unsigned int)( right - ( left + 1 )))
        title[ right - ( left + 1 ) ] = '\0';

    WriteStrAtt( wn->top, left + (( ++right - ( left + strlen( title ))) / 2 ), wn->attrib, title );

    if ( pszBottomTitle != NULL ) {
        if ( strlen( pszBottomTitle ) >= (unsigned int)( right - ( left + 1 )))
            pszBottomTitle[ right - ( left + 1 ) ] = '\0';
        WriteStrAtt( wn->bottom, left + (( ++right - ( left + strlen( pszBottomTitle ))) / 2 ), wn->attrib, pszBottomTitle );
    }

    // set active window
    win = aWin[ ++nCurrentWindow ] = wn;

    // set the window cursor
    wSetCurPos( 0, 0 );

    // remove blink & intensity, and ROR 4 to get inverse
    attribute &= 0x77;

    if (( wn->inverse = gpIniptr->LBBar ) == 0 )
        wn->inverse = ( attribute >> 4 ) + (( attribute << 4 ) & 0xFF );

    return wn;
}


// Remove the specified window.  Must be the "top" window if overlapping
//   windows are used; tiled windows can be removed randomly.
void wRemove( POPWINDOWPTR wn )
{
    unsigned int width;
    char *fptr;

    if ( wn == NULL )
        return;

    width = ((( wn->right + (( wn->fShadow ) ? 3 : 1 )) - wn->left ) * 2 );

    // repaint the saved screen image
    for ( fptr = wn->screen_save; ( wn->top <= ( wn->bottom + wn->fShadow )); wn->top++ ) {

        WriteCellStr( fptr, width, wn->top, wn->left );
        fptr += width;
    }

    // reset cursor to original location
    SetCurPos( wn->old_row, wn->old_col );

    // free dynamic storage
    FreeMem( wn->screen_save );
    free( wn );
    if ( --nCurrentWindow >= 0 )
        win = aWin[ nCurrentWindow ];
    else
        win = NULL;
}


// clear the "active" part of a window and home internal text cursor                                                *
void wClear( void )
{
    Scroll( win->top + 1, win->left + 1, win->bottom - 1, win->right - 1, 0, win->attrib );
    wSetCurPos( 0, 0 );
}


// set the active window cursor location (0,0 based)
void wSetCurPos( int row, int column )
{
    win->c_row = row;
    win->c_col = column;
    SetCurPos( win->top + row + 1, win->left + column + 1 );
}


// write a string to a window, truncating at right margin
void wWriteStrAtt( int row, int col, int attribute, char *line )
{
    int length;
    int column;
    char *start_line = line;

    for ( column = win->left + 1, length = 0; ( *line != '\0' ); line++ ) {

        incr_column( *line, &column );

        // check right & left margins
        if ( column >= ( win->right + win->hoffset ))
            break;

        if ( column > ( win->left + win->hoffset + 1 ))
            length++;
        else
            start_line++;
    }

    SetCurPos( win->top + row + 1, win->left + col + 1 );
    color_printf( attribute, FMT_FAR_PREC_STR, length, start_line );
}


// Display a scroll bar along the right border of the active window.
static void ScrollBar( int position, int outof )
{
    int i, high;

    high = ( win->bottom - win->top ) - 3;

    WriteChrAtt( win->top+1, win->right, win->inverse, gchUpArrow );
    WriteChrAtt( win->bottom-1, win->right, win->inverse, gchDownArrow );

    for ( i = 0; ( i < high ) ; i++ )
        WriteChrAtt( win->top + 2 + i, win->right, win->attrib, gchDimBlock);

    // position the "thumbwheel"
    if ( position > 1 ) {
        if (( i = ((( high * position ) / outof ) + (((high * position) % outof) != 0))) == 1)
            i++;
        if (( i == high ) && ( position < outof ))
            i--;
    } else
        i = 1;

    WriteChrAtt( win->top + i + 1, win->right, win->attrib, gchBlock );
}


static void ScrollBack( char * *list, int first )
{
    Scroll( win->top+1, win->left+1, win->bottom-1, win->right-1, -1, win->attrib );
    wWriteStrAtt( 0, 0, win->attrib, list[first-1] );
}


static void ScrollForward( char * *list, int first, int last )
{
    Scroll( win->top+1, win->left+1, win->bottom-1, win->right-1, 1, win->attrib);
    wWriteStrAtt(( last - first ), 0, win->attrib, list[last-1] );
}


// display a selection list
static char * wSelect( char * *list, int count, int current, char *pszChars )
{
    int i;
    int height, j, key, indent, nSearch;
    int turnoff, width, first, last, rows, bar = 0, nLoop = 0;
    char *fptr;
    char szSearchDisp[32];
    extern int _line(int, int, int, int, int, int, int);

    gnPopExitKey = 0;
    indent = win->left + 1;

    width = ( win->right - win->left ) - 1;

    rows = ( win->bottom - win->top ) - 1;

redraw:
    // turn on scrollbar if more than 1 page
    if (( height = count ) > rows ) {
        if ( rows > 2 )
            bar = 1;
        height = rows;
    }

    // set page start
    if (( current > rows ) && ((( win->fPopupFlags & NO_CHANGE ) == 0 ) || ( nLoop == 0 ))) {
        if (( count - current ) < rows / 2 )
            first = ( count - rows ) + 1;
        else
            first = ( current - ( rows / 2 )) + 1;
        if ( first < 1 )
            first = 1;
    } else
        first = 1;

    nLoop = 1;

    // clear window & write a page's worth
Redraw2:
    wClear();
    for ( i = 1, j = first - 1; (( j < count ) && ( i <= height )); i++, j++ )
        wWriteStrAtt( i - 1, 0, win->attrib, list[j] );

    szSearch[0] = '\0';
    nSearch = 0;
    for ( ; ; ) {

        if (( win->fPopupFlags & NO_CHANGE ) == 0 ) {

            while ( current < first ) {
            first--;
            // only scroll within page
            if (( current + rows ) >= first )
                ScrollBack( list, first );
            }
        }

        last = first + height - 1;

        if (( win->fPopupFlags & NO_CHANGE ) == 0 ) {

            while ( current > last ) {

            first++;
            last++;

            // only scroll within page
            if (( current - rows ) <= last )
                ScrollForward( list, first, last );
            }
        }

        turnoff = current;

        if ( bar )
            ScrollBar( current, count );

        // flip the current line to inverse video
        i = win->top + 1 + ( current - first );
        if (( current >= first ) && ( current <= last )) {
            SetLineColor( i, indent, width, win->inverse );
            SetCurPos( i, indent );
        } else
            SetCurPos( win->top, win->left );

        // with DOS, get the key from the BIOS, because
        //   STDIN might be redirected (%@SELECT[con,...])
        key = cvtkey( GetKeystroke( EDIT_NO_ECHO | EDIT_BIOS_KEY | EDIT_UC_SHIFT ), MAP_GEN | MAP_HWIN );

        switch ( key ) {

        case LF:
        case CR:
ExitKey:
            gnPopExitKey = key;
            gnPopupSelection = current - 1;
            if ( win->fPopupFlags & CDD_COLORS ) {

                fptr = list[ gnPopupSelection ] + strlen( list[ gnPopupSelection ] ) + 1;
                if ( *fptr == 4 ) {
                fptr++;
                return fptr;
                }
            }
            return ( list[ gnPopupSelection ] );

        case ESC:
            return 0L;

        case F1:

            // popup some help
            _help( (( win->fPopupFlags & BATCH_DEBUG ) ? "Debugging" : NULL ), NULL );
            continue;

        case 4:
            // delete the current line (history & CD lists only!)
            if ((( list[0] < glpHistoryList ) || ( list[0] >= ( glpHistoryList + gpIniptr->HistorySize ))) && (( list[0] < glpDirHistory ) || ( list[0] >= ( glpDirHistory + gpIniptr->DirHistorySize )))) {
                honk();
                break;
            }

            i = current - 1;

            // collapse the current entry from the list block
            //   and History / Directory list
            j = strlen( list[i] ) + 1;
            count--;

            memmove( list[i], list[i] + j, (unsigned int)((end_of_env(list[count]) - list[i])+1) - j );

            // adjust the list array pointers
            for ( ; ( i < count ); i++ )
                list[i] = ( list[i+1] - j );

            if (( current > count ) && ( --current <= 0 ))
                return 0L;

            goto redraw;

        case CUR_LEFT:
            if ( win->hoffset > 0 ) {
                win->hoffset -= 4;
                goto Redraw2;
            }
            honk();
            break;

        case CUR_RIGHT:
            win->hoffset += 4;
            goto Redraw2;

        case PgUp:

            if ( win->fPopupFlags & NO_CHANGE ) {

                if (( first -= height - 1 ) < 1 )
                    first = 1;
                goto Redraw2;

            } else if (( current -= height - 1 ) < 1 )
                current = 1;
            break;

        case PgDn:
            if ( win->fPopupFlags & NO_CHANGE ) {

                if (( first + ( height - 1 )) <= count )
                    first += height - 1;
                goto Redraw2;

            } else if (( current += height - 1 ) > count )
                current = count;
            break;

        case HOME:
        case CTL_PgUp:

            first = 1;
            win->hoffset = 0;
            if ( win->fPopupFlags & NO_CHANGE )
                goto Redraw2;
            current = 1;
            goto redraw;

        case END:
        case CTL_PgDn:

            if ( win->fPopupFlags & NO_CHANGE ) {
                if (( first = count - ( height - 1 )) < 1 )
                    first = 1;
                goto Redraw2;
            }
            current = count;
            goto redraw;

        case CUR_UP:

            if ( win->fPopupFlags & NO_CHANGE ) {

                if ( first > 1 ) {
                    first--;
                    ScrollBack( list, first );
                    continue;
                }

            } else if ( current > 1 ) {
                current--;
                break;
            }

            honk();
            break;

        case CUR_DOWN:

            if ( win->fPopupFlags & NO_CHANGE ) {

                if ( last < count ) {
                    first++;
                    last++;
                    ScrollForward( list, first, last );
                    continue;
                }

            } else if ( current < count ) {
                current++;
                break;
            }

            honk();
            break;

        case BS:
            nSearch = 0;
            break;

        default:

            if (( win->fPopupFlags & BATCH_DEBUG ) && ( pszChars != NULL )) {

            if ( key == F8 )
                key = 'T';
            else if ( key == F10 )
                key = 'S';

            if ( strchr( pszChars, key ) != NULL ) {

                extern void PopupEnvironment( int );
                extern int ListEntry( char * );

                // check for popup alias or environment list
                if ( key == 'A' )
                    PopupEnvironment( 1 );

                else if ( key == 'V' )
                    PopupEnvironment( 0 );

                else if ( key == 'L' ) {

                    // list file
                    char szFileName[MAXFILENAME];
                    POPWINDOWPTR wn;
                    jmp_buf saved_env;

                    wn = wOpen( 5, 2, 7, 77, uInverse, "LIST", NULL );
                    wn->attrib = uNormal;
                    wClear();

                    wWriteStrAtt( 0, 1, uNormal, "File: " );
                    egets( szFileName, 72, ( EDIT_DIALOG | EDIT_BIOS_KEY | EDIT_NO_CRLF ));
                    wRemove( wn );

                    if ( szFileName[0] ) {

                        // save the old "env" (^C destination)
                        memmove( saved_env, cv.env, sizeof(saved_env) );

                        // kludge to save/restore screen
                        wn = wOpen( 0, 0, GetScrRows(), GetScrCols()-1, uDefault, NULLSTR, NULL );
                        ListEntry( szFileName );
                        wRemove( wn );

                        // restore the old "env"
                        memmove( cv.env, saved_env, sizeof(saved_env) );
                    }

                } else if ( key == 'X' ) {

                    char * xlist[2];

                    // display the expanded line
                    xlist[0] = (char *)pszBatchDebugLine;
                    wPopSelect( 5, 2, 1, 75, xlist, 1, 1, "Expanded Line", NULL, NULL, 0 );

                } else
                    goto ExitKey;
                continue;
            }

            } else if ( key <= 0xFF ) {

            // search for entry beginning with szSearch
ResetSearch:
            if ( nSearch >= 32 )
                nSearch = 0;
            szSearch[ nSearch++ ] = key;
            szSearch[ nSearch ] = '\0';

            for ( j = 0, i = current - 1; ( i < count ); i++ ) {
StartLoop:
                fptr = list[ i ];
                while (( *fptr == ' ' ) || ( *fptr == '\t'))
                fptr++;

                if ( strnicmp( szSearch, fptr, strlen( szSearch )) == 0 ) {
                current = i + 1;
                goto NoHonk;
                }

                if ( i == ( count - 1 )) {
                if ( j == 0 ) {
                    // try again from the beginning
                    j = 1;
                    i = 0;
                    goto StartLoop;
                } else if ( j == 1 ) {
                    if ( nSearch == 1 ) {
                        nSearch = 0;
                    break;
                    }
                    nSearch = 0;
                    goto ResetSearch;
                }
                }
            }
            }

            honk();
        }
NoHonk:
        if (( win->fPopupFlags & BATCH_DEBUG ) == 0 ) {

            // display the search string in the bottom right border
            _line( win->bottom, (win->left + 1), (win->right - win->left - 1), 1, 0, win->attrib, 0 );

            if ( nSearch > 0 ) {
            sprintf( szSearchDisp, FMT_PREC_STR, (win->right - win->left - 1), szSearch);
            wWriteStrAtt( (win->bottom - win->top - 1), (win->right - win->left - strlen(szSearchDisp) - 1), win->attrib, szSearchDisp );
            }
        }

        // flip the old line back to normal video
        if (( win->fPopupFlags & NO_CHANGE ) == 0 )
            SetLineColor( win->top + 1 + (turnoff - first), indent, width, win->attrib );
    }
}


// do a Shell sort on the list (much smaller & less stack than Quicksort)
static void ssort( char * * list, unsigned int entries )
{
    unsigned int i, gap;
    long j;
    char *pTmp;

    for ( gap = ( entries >> 1 ); ( gap > 0 ); gap >>= 1 ) {

        for ( i = gap; ( i < entries ); i++ ) {

            for ( j = ( i - gap ); ( j >= 0L ); j -= gap ) {

                if ( stricmp( list[ j ], list[ j + gap ] ) <= 0 )
                    break;

                // swap the two records
                pTmp = list[ j ];
                list[ j ] = list[ j + gap ];
                list[ j + gap ] = pTmp;
            }
        }
    }
}


// popup a selection list
char * wPopSelect( int top, int left, int height, int width, char * *list, int entries, int current, char *title, char *pszBottomTitle, char *pszKeys, int fOptions )
{
    int i, j, bottom, right;
    char *fptr = 0L;
    jmp_buf saved_env;
    int iCurPosRow, iCurPosCol;

    if ( fOptions & SORT_LIST )
        ssort( list, entries );

    // check for valid Top and Left parameters
    if (( i = (( GetScrCols() - 1 ) - width )) < 0 )
        i = 0;

    if ( left > i )
        left = (( left == 999 ) ? i / 2 : i );

    // check if requested height is bigger than window
    if (( i = (( GetScrRows() - 1 ) - height )) < 0 )
        i = 0;

    // 20090417 AB: if PopupPosRelative is set, calculate y pos relative to current cursor
    if ( gaInifile.PopupPosRelative ) {
        GetCurPos( &iCurPosRow, &iCurPosCol );
        if ( entries < height ) {
            height = entries;
        }
        j = (int) GetScrRows();
        if ( top < 0 ) {
            // popup should be below cursor line
            i = iCurPosRow - top;
            if ( ( i + height + 3 ) > j ) {
                // popup would cross lower window boundary
                i = j - height - 2;
            }
        } else {
            // popup above cursor line
            i = iCurPosRow - top - height - 1;
        }
        if ( i < 1 ) {
            i = 1;
        }
        top = i;
    } else {
        if ( top > i ) {
            top = (( top == 999 ) ? i / 2 : i );
        }
        if ( top < 0 ) {
            top = 0;
        }
    }

    bottom = ( top + height ) + 1;
    right = ( left + width ) + 1;

    // check for valid sizes for current screen
    i = GetScrRows();
    if ( bottom > i )
        bottom = i;
    i--;

    if (( bottom > i ) && ( top > 0 )) {
        if (( top -= ( bottom - i )) < 0 )
            top = 0;
        bottom = i;
    }

    i = GetScrCols() - 1;
    if ( right > i )
        right = i;
    i -= 2;

    if (( right > i ) && ( left > 0 )) {
        if (( left -= ( right - i )) < 0 )
            left = 0;
        right = i;
    }

    // if less than 1 page of entries, size window downwards
    if ( entries < (( bottom - top ) - 1 ))
        bottom = top + entries + 1;

    // get the popup window color, or default to inverse of current
    if ( win == NULL ) {

        // save original color
        GetAtt( &uDefault, &uInverse );

        if (( uInverse = (( fOptions & CDD_COLORS ) ? gpIniptr->CDDColor : gpIniptr->PWColor )) != 0 ) {
            // ROR 4 to get inverse
            uNormal = ( uInverse >> 4 ) + (( uInverse << 4 ) & 0xFF );
        } else
            GetAtt( &uNormal, &uInverse );
    }

    // save the old "env" (^C destination)
    memmove( saved_env, cv.env, sizeof(saved_env) );

    if ( setjmp( cv.env ) != -1 ) {

        // open the popup window
        if ( wOpen( top, left, bottom, right, uInverse, title, pszBottomTitle ) == NULL )
            return 0L;

        win->fPopupFlags = fOptions;

        // call the selection window
        fptr = wSelect( list, entries, current, pszKeys );
    }

    // remove the window, restore the screen & free memory
    wRemove( win );

    // restore the old "env"
    memmove( cv.env, saved_env, sizeof(saved_env) );

    // disable signal handling momentarily
    HoldSignals();

    return fptr;
}

