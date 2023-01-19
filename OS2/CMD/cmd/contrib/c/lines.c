//  LINES.C - line drawing for 4os2
//
// Draws a horizontal or vertical line of the specified width (single or
//   double) using line drawing characters, starting at the specified row
//   and column of the current window and continuing for "len" characters.
//   The line can overlay what is already on the screen, so that the
//   appropriate T, cross or corner character is generated if it runs into
//   another line on the screen at right angles to itself.
//
// The algorithm involves looking at adjacent character cells to determine
// if the character generated should connect in one direction or another.

#include "product.h"

#include <stdio.h>
#include <string.h>

#include "4all.h"


int _line(int, int, int, int, int, int, int);
static int get_line_char(int, int);


#define N   1
#define S   2
#define E   4
#define W   8
#define H2  16
#define V2  32


static unsigned char breakdown[] = {
    N|S,        // '³'
    N|S|W,      // '´'
    N|S|W|H2,   // 'µ'
    N|S|W|V2,   // '¶'
    S|W|V2,     // '·'
    S|W|H2,     // '¸'
    N|S|W|H2|V2,    // '¹'
    N|S|V2,     // 'º'
    W|S|H2|V2,  // '»'
    N|W|H2|V2,  // '¼'
    N|W|V2,     // '½'
    N|W|H2,     // '¾'
    W|S,        // '¿'
    N|E,        // 'À'
    N|W|E,      // 'Á'
    W|E|S,      // 'Â'
    N|S|E,      // 'Ã'
    E|W,        // 'Ä'
    N|S|E|W,    // 'Å'
    N|S|E|H2,   // 'Æ'
    N|S|E|V2,   // 'Ç'
    N|E|H2|V2,  // 'È'
    E|S|H2|V2,  // 'É'
    N|E|W|H2|V2,    // 'Ê'
    E|W|S|H2|V2,    // 'Ë'
    N|S|E|H2|V2,    // 'Ì'
    E|W|H2,     // 'Í'
    N|S|E|W|H2|V2,  // 'Î'
    N|E|W|H2,   // 'Ï'
    N|E|W|V2,   // 'Ð'
    E|W|S|H2,   // 'Ñ'
    E|W|S|V2,   // 'Ò'
    N|E|V2,     // 'Ó'
    N|E|H2,     // 'Ô'
    S|E|H2,     // 'Õ'
    S|E|V2,     // 'Ö'
    N|S|E|W|V2, // '×'
    N|S|E|W|H2, // 'Ø'
    N|W,        // 'Ù'
    E|S,        // 'Ú'
};


static unsigned char line_chars[] = {
    ' ',        // empty
    '³',        // N
    '³',        // S
    '³',        // S|N
    'Ä',        // E
    'À',        // E|N
    'Ú',        // E|S
    'Ã',        // E|S|N
    'Ä',        // W
    'Ù',        // W|N
    '¿',        // W|S
    '´',        // W|S|N
    'Ä',        // W|E
    'Á',        // W|E|N
    'Â',        // W|E|S
    'Å',        // W|E|S|N
    ' ',        // H2
    '³',        // H2|N
    '³',        // H2|S
    '³',        // H2|S|N
    'Í',        // H2|E
    'Ô',        // H2|E|N
    'Õ',        // H2|E|S
    'Æ',        // H2|E|S|N
    'Í',        // H2|W
    '¾',        // H2|W|N
    '¸',        // H2|W|S
    'µ',        // H2|W|S|N
    'Í',        // H2|W|E
    'Ï',        // H2|W|E|N
    'Ñ',        // H2|W|E|S
    'Ø',        // H2|W|E|S|N
    ' ',        // V2
    'º',        // V2|N
    'º',        // V2|S
    'º',        // V2|S|N
    'Ä',        // V2|E
    'Ó',        // V2|E|N
    'Ö',        // V2|E|S
    'Ç',        // V2|E|S|N
    'Ä',        // V2|W
    '½',        // V2|W|N
    '·',        // V2|W|S
    '¶',        // V2|W|S|N
    'Ä',        // V2|W|E
    'Ð',        // V2|W|E|N
    'Ò',        // V2|W|E|S
    '×',        // V2|W|E|S|N
    ' ',        // V2|H2
    'º',        // V2|H2|N
    'º',        // V2|H2|S
    'º',        // V2|H2|S|N
    'Í',        // V2|H2|E
    'È',        // V2|H2|E|N
    'É',        // V2|H2|E|S
    'Ì',        // V2|H2|E|S|N
    'Í',        // V2|H2|W
    '¼',        // V2|H2|W|N
    '»',        // V2|H2|W|S
    '¹',        // V2|H2|W|S|N
    'Í',        // V2|H2|W|E
    'Ê',        // V2|H2|W|E|N
    'Ë',        // V2|H2|W|E|S
    'Î'         // V2|H2|W|E|S|N
};


// draw a horizontal or vertical line directly to the display
int drawline_cmd( int argc, char **argv )
{
    int attribute = -1;
    int row, col, len, width;

    // get the arguments & colors
    if (( argc >= 6 ) && ( sscanf( argv[1], "%d%d%d%d", &row, &col, &len, &width ) == 4 ))
        attribute = GetColors( argv[5], 0 );

    return ((( attribute == -1 ) || ( verify_row_col( row, col )) || ( _line( row, col, len, width,(_ctoupper( argv[0][4] ) == 'V' ), attribute, 1 ) != 0 )) ? usage( DRAWLINE_USAGE ) : 0 );
}


#define BOX_SHADOWED 1
#define BOX_ZOOMED 2

// draw a box directly to display memory
int drawbox_cmd(int argc, char **argv)
{
    char *arg, *pszLine;
    int top, left, bottom, right, style, attribute = -1, fill = -1;
    int box_flags = 0;

    // get the arguments & colors
    if ((argc >= 7) && (sscanf(argv[1],"%d%d%d%d%d",&top,&left,&bottom,&right,&style) == 5)) {

        pszLine = argv[6];
        attribute = GetColors(pszLine,0);

        // check for a FILL color
        if ((*pszLine) && (_strnicmp(first_arg(pszLine),BOX_FILL,3) == 0) && ((arg = first_arg(next_arg(pszLine,1))) != NULL)) {

            if (_strnicmp(arg,BRIGHT,3) == 0) {
                // set intensity bit
                fill = 0x80;
                if ((arg = first_arg(next_arg(pszLine,1))) == NULL)
                    return (usage(DRAWBOX_USAGE));
            } else
                fill = 0;

            if ((argc = color_shade(arg)) <= 15) {
                fill |= (argc << 4);
                (void)next_arg(pszLine,1);
            }
        }

        // check for a SHADOW or ZOOM
        while (*pszLine) {
            if (_strnicmp(pszLine,BOX_SHADOW,3) == 0)
                box_flags |= BOX_SHADOWED;
            else if (_strnicmp(pszLine,BOX_ZOOM,3) == 0)
                box_flags |= BOX_ZOOMED;
            (void)next_arg(pszLine,1);
        }
    }

    if ((attribute == -1) || (verify_row_col(top,left)) || (verify_row_col(bottom,right)))
        return (usage(DRAWBOX_USAGE));

    _box(top,left,bottom,right,style,attribute,fill,box_flags,1);

    return 0;
}


// draw a box, with an optional shadow & connectors
void _box(int top, int left, int bottom, int right, int style, int attribute, int fill, int box_flags, int connector)
{
    int width;
    int v_zoom_top, v_zoom_bottom, h_zoom_left, h_zoom_right;
    int row_diff, col_diff, row_inc, col_inc, fWait = 0;

    // zoom the window?
    if (box_flags & BOX_ZOOMED) {

        fWait = 1;

        // zooming requires a fill color; use default if none specified
        if (fill == -1)
            GetAtt((unsigned int *)&fill,(unsigned int *)&v_zoom_top);

        v_zoom_top = v_zoom_bottom = (top + bottom) / 2;
        h_zoom_left = h_zoom_right = (left + right) / 2;

        // get the increment for each zoom
        // (This makes the zoom smooth in all dimensions)
        if ((row_diff = v_zoom_top - top) <= 0)
            row_diff = 1;
        if ((col_diff = h_zoom_left - left) <= 0)
            col_diff = 1;

        if (row_diff > col_diff) {
            // tall skinny box
            row_inc = (row_diff / col_diff);
            col_inc = 1;
        } else {
            // short wide box
            col_inc = (col_diff / row_diff);
            row_inc = 1;
        }

    } else {

        v_zoom_top = top;
        v_zoom_bottom = bottom;
        h_zoom_left = left;
        h_zoom_right = right;
    }

    do {

        if (box_flags & BOX_ZOOMED) {

            // if zooming, increment the box size
            v_zoom_top -= row_inc;
            if (v_zoom_top < top)
                v_zoom_top = top;

            v_zoom_bottom += row_inc;
            if (v_zoom_bottom > bottom)
                v_zoom_bottom = bottom;

            h_zoom_left -= col_inc;
            if (h_zoom_left < left)
                h_zoom_left = left;

            h_zoom_right += col_inc;
            if (h_zoom_right > right)
                h_zoom_right = right;
        }

        // clear the box to the specified attribute
        if (fill != -1)
            Scroll(v_zoom_top,h_zoom_left,v_zoom_bottom,h_zoom_right,0,fill);

        if (style == 0)
            width = 0;
        else if ((style == 2) || (style == 4))
            width = 2;
        else
            width = 1;

        // draw the two horizontals & the two verticals
        _line(v_zoom_top,h_zoom_left,(h_zoom_right-h_zoom_left)+1,width,0,attribute,connector);
        _line(v_zoom_bottom,h_zoom_left,(h_zoom_right-h_zoom_left)+1,width,0,attribute,connector);

        if (style == 3)
            width = 2;
        else if (style == 4)
            width = 1;

        _line(v_zoom_top,h_zoom_left,(v_zoom_bottom-v_zoom_top)+1,width,1,attribute,connector);
        _line(v_zoom_top,h_zoom_right,(v_zoom_bottom-v_zoom_top)+1,width,1,attribute,connector);

        // if on a fast system, slow things down a bit
        if (fWait)
            SysBeep(0,1);

    } while ((box_flags & BOX_ZOOMED) && ((v_zoom_top > top) || (v_zoom_bottom < bottom) || (h_zoom_left > left) || (h_zoom_right < right)));

    // check for a shadow
    if (box_flags & BOX_SHADOWED) {

        left += 2;
        right++;
        if ( left >= right )
            left = right - 1;

        // read the character and attribute, and change
        //   the attribute to black background, low intensity
        //   foreground
        SetLineColor(++bottom,left,(right-left),7);

        // shadow the right side of the window
        for (top++; (top <= bottom); top++)
            SetLineColor(top,right,2,7);
    }
}


// draw a line, making proper connectors along the way
int _line(int row, int col, int len, int width, int direction, int attribute, int connector)
{
    int     ch, i;
    int     s_row, s_col, bits, bottom, right;
    char    buffer[256];

    // truncate overly long lines
    if ( len > 255 )
        len = 255;

    // save starting row & column
    s_row = row;
    s_col = col;

    bottom = GetScrRows();
    right = GetScrCols() - 1;

    for ( i = 0; ( i < len ); ) {

        // Read original character - if not a line drawing char,
        //   just write over it.  Otherwise, try to make a connector
        //   if "connector" != 0
        if ( width == 0 )
            buffer[i] = gchBlock;

        else if (((connector == 0) && (i != 0) && (i != len - 1)) || ((ch = get_line_char(row,col)) == -1)) {

            if (direction == 0)
                buffer[i] = (char)((width == 1) ? 'Ä' : 'Í');
            else
                buffer[i] = (char)((width == 1) ? '³' : 'º');

        } else {

            bits = (char)((direction == 0) ? (breakdown[ch] & ~H2) | W | E | ((width == 1) ? 0 : H2) : (breakdown[ch] & ~V2) | N | S | ((width == 1) ? 0 : V2));

            if (( i == 0 ) || ( direction )) {

                // at start look & see if connect needed
                bits &= ~W;

                if (( col > 0 ) && (( ch = get_line_char( row, col-1 )) >= 0 )) {
                    if ( breakdown[ ch ] & E )
                        bits |= W;
                }
            }

            if (( i == len - 1 ) || ( direction )) {

                // at end look & see if connect needed
                bits &= ~E;

                if ((col < right) && ((ch = get_line_char(row, col+1)) >= 0)) {
                    if (breakdown[ch] & W)
                        bits |= E;
                }
            }

            if (( direction == 0 ) || ( i == 0 )) {

                // at start look & see if connect needed
                bits &= ~N;

                if (( row > 0 ) && (( ch = get_line_char(row-1, col)) >= 0)) {
                    if ( breakdown[ ch ] & S )
                        bits |= N;
                }
            }

            if (( direction == 0 ) || ( i == len - 1 )) {

                // at end look & see if connect needed
                bits &= ~S;

                if (( row < bottom ) && (( ch = get_line_char(row+1, col)) >= 0)) {
                    if ( breakdown[ ch ] & N )
                        bits |= S;
                }
            }

            buffer[i] = line_chars[ bits ];
        }

        i++;

        if ( direction == 0 ) {
            if ( ++col > right )
                break;
        } else {
            if ( ++row > bottom )
                break;
        }
    }

    buffer[i] = '\0';

    // write the line directly to the display
    if ( direction == 0 )
        WriteStrAtt( s_row, s_col, attribute, buffer );
    else
        WriteVStrAtt( s_row, s_col, attribute, buffer );

    return 0;
}


// Make sure the specified row & column are on the screen!
int verify_row_col( unsigned int row, unsigned int col )
{
    return ((( row > (unsigned int)GetScrRows() ) && ( row != 999 )) || (( col > (unsigned int)( GetScrCols() - 1 )) && ( col != 999 )));
}


// Read the character at the specified cursor location.
//   Return -1 if not a line drawing char, or the offset into the "breakdown"
//   table if it is.
static int get_line_char( int row, int col )
{
    char        cell[4];
    VIO_UTYPE   length = 2;

    (void)ReadCellStr( cell, length, row, col );

    return ((( cell[0] < 179 ) || ( cell[0] > 218 )) ? -1 : cell[0] - 179 );
}

