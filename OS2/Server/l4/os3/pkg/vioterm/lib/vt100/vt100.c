/**
 * \file   l4vfs/term_server/lib/vt100/vt100.c
 * \brief  parse vt100 sequences
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#include <l4/log/l4log.h>
#include <l4/term_server/vt100.h>

#include "lib.h"

extern int _DEBUG;

// you may want to have a look into /usr/src/linux/drivers/char/console.c
void parse_character(termstate_t * term, l4_uint8_t c)
{
    int i;

    // these must be handled independetly of state
    switch (c)
    {
    case 0: // ignore
        return;
    case 7: // bell
        // todo: impl. me
        return;
    case 8: // backspace
        if (term->cur_x > 0)
            term->cur_x--;
        return;
    case 9: // tab
        cursor_tab(term);
        return;
    case 10: // newline (lf)
    case 11: // ?
    case 12: // ?
        cursor_nl(term);
        return;
    case 13: // do a cr here, maybe a smart one (+ lf)
        cursor_cr(term);
        return;
    case 27: // ESC
        term->esc_state = ESesc;
        return;
    }

    // now check for the state-dependant characters
    switch (term->esc_state)
    {
    case ESnormal:
        // normal character
        if ((c >= 0x20 && c <= 0x7e) || (c >= 0xa1 && c <= 0xfe))
        {
            if (term->insert_mode == VT100_INSMODE_REPLACE)
            {
                set_char(term, c);
                //vt100_redraw_xy(term, term->cur_x, term->cur_y);
                cursor_next(term);
            }
            else // VT100_INSMODE_INSERT
            {
                insert_char(term, c);
                //vt100_redraw(term);
            }
            return;
        }
//        switch (c)
//        {
//        }
        break;
    case ESesc: // normal ESC found
        term->esc_state = ESnormal;
        switch (c)
        {
        case '[':
            term->esc_state = ESsquare;
            return;
        case 'D':  // lf
            cursor_lf(term);
            //vt100_redraw(term);
            return;
        case 'E':  // cr + lf
            cursor_nl(term);
            //vt100_redraw(term);
            return;
        case 'H':  // set tab at current position
            set_tab( term, term->cur_x );
            return;
        case 'M':
            rev_scroll(term);
            //vt100_redraw(term);
            return;
        case '7':  // save cursor position and attributes
            term->cur_stored_x    = term->cur_x;
            term->cur_stored_y    = term->cur_y;
            term->cur_stored_attr = term->attrib_mode;
            return;
        case '8':  // restore saved cursor position and attributes
            term->cur_x         = term->cur_stored_x;
            term->cur_y         = term->cur_stored_y;
            term->attrib_mode   = term->cur_stored_attr;
            return;
        case '#':
            term->esc_state = EShash;
            return;
        case 'c':  // reset vt to default settings
            init_termstate(term, term->w, term->phys_h, term->virt_h);
            return;
        }
    case ESsquare:
        for(i = 0; i < NUM_PAR; i++)
            term->par[i] = 0;
        term->used_par = 0;
        term->esc_state = ESgetpars;
        term->ques = (c=='?');
        if (term->ques)
            return;
        // fall-through
    case ESgetpars:
        if (c == ';' && term->used_par < NUM_PAR - 1)
        {
            term->used_par++;
            return;
        }
        else if (c >= '0' && c <= '9')
        {
            term->par[term->used_par] *= 10;
            term->par[term->used_par] += c - '0';
            return;
        } else term->esc_state = ESgotpars;
        // fall-through
    case ESgotpars:
        term->esc_state = ESnormal;
        switch (c)
        {
        case 'h':
            if (term->ques)
            {
                // handle question commands ending with h
                for (i = 0; i <= term->used_par; i++)
                {
                    switch( term->par[i] )
                    {
                    case  5: // activate inverse screen
                        if (set_mode(term, 1))
                            //vt100_redraw(term);
                        break;
                    case  6: // origin mode = scroll region
                        term->origin_mode = VT100_ORIGIN_SCROLL;
                        break;
                    case  7: // autowrap mode on
                        term->autowrap   = 1;
                        break;
                    case  8: // autorepeat on
                        term->autorepeat = 1;
                        break;
                    case 25: // activate cursor
                        term->cursor_vis = 1; return;
                    default:
                        break;
                    }
                }
                return;
            }
            else // handle commands without question mark
            {
                for(i = 0; i <= term->used_par; i++)
                {
                    switch( term->par[i] )
                    {
                    case 4: // insert mode on
                        term->insert_mode = VT100_INSMODE_INSERT;
                        break;
                    case 12: // echo on
                        term->echo = 1;
                        break;
                    case 20: // line feed mode
                        term->newline = 1;
                        break;
                    default: break;
                    }
                }
                return;
            }
        case 'l':
            if (term->ques)
            {
                for(i = 0; i <= term->used_par; i++)
                {
                    // handle question commands ending with l
                    switch( term->par[i] )
                    {
                    case  3: // set 80 column mode, clear screen
                        // todo: impl. resize and use it here
                        break;
                    case  5: // deactivate inverse screen
                        if(set_mode(term, 0))
                            //vt100_redraw(term);
                        break;
                    case  6: // origin mode = whole screen
                        term->origin_mode = VT100_ORIGIN_GLOBAL;
                    case  7: // autowrap mode off
                        term->autowrap = 0;
                        break;
                    case  8: // autorepeat off
                        term->autorepeat = 0;
                        break;
                    case 25: // deactivate cursor
                        term->cursor_vis = 0; return;
                    default: break;
                    }
                    return;
                }
            }
            else // handle commands without question mark
            {
                for( i=0; i <= term->used_par; i++ )
                {
                    switch( term->par[i] )
                    {
                    case 4: // insert mode off
                        term->insert_mode = VT100_INSMODE_REPLACE;
                        break;
                    case 12: // echo off
                        term->echo = 0;
                        break;
                    case 20: // carriage return mode
                        term->newline = 0;
                        break;
                    default:
                        break;
                    }
                    return;
                }
            }
        }
        if (term->ques)
        {
            term->ques = 0;
            return;
        }
        switch (c)
        {
        case 'F':  // CR + up
            term->cur_x     = 0;
            // fall through
        case 'A':  // up
            if (! term->par[0])
                term->par[0]++;
            cursor_up(term);
            return;
        case 'E':  // CR + down
            term->cur_x     = 0;
            // fall through
        case 'B':  // down
            if (! term->par[0])
                term->par[0]++;
            cursor_down(term);
            return;
        case 'C':  // right
            // no parameter -> set parameter to default (=1)
            if (! term->par[0])
                term->par[0]++;
            cursor_right(term);
            return;
        case 'D':  // left
            if (! term->par[0])
                term->par[0]++;
            cursor_left(term);
            return;
        case 'G':  // cursor position horizontal absolute
            if ( (term->par[0] > 0) &&
                 (term->par[0] < term->w) )
                term->cur_x = term->par[0];
            return;
        case 'H':  // absolute position (x,y)
        case 'f':  // f is the same as H (correct me, if I'm wrong)
            // well it is the same in the Linux kernel, in theorie the
            // 'f' commands depend on the PUM (Position unit mode),
            // which can be characters (seems to be default) or inch
            if (term->par[0] > 0)  // convert to (0, 0) based coords
                term->par[0]--;
            if (term->par[1] > 0)
                term->par[1]--;
            cursor_move_abs(term, term->par[1], term->par[0]);
            return;
        case 'I':  // insert tabs ('\t', term->par[0] times)
            if (!term->par[0])
                term->par[0]++;
            for (i=0;i<term->par[0];i++)
                cursor_tab(term);
            return;
        case 'J':
            switch(term->par[0])
            {
            case 0:  // kill to end of screen
                clean_to_eos(term);
                //vt100_redraw(term);
                return;
            case 1:  // kill from start of screen
                clean_from_sos(term);
                //vt100_redraw(term);
                return;
            case 2:  // kill whole screen
                clean_screen(term, ' ');
                //vt100_redraw(term);
                return;
            }
        case 'K':  // kill to end of line
            switch(term->par[0])
            {
            case 0: // clean to end of line
                clean_to_eol(term);
                //vt100_redraw(term);
                return;
            case 1: // clean from start of line
                clean_from_sol(term);
                return;
            case 2: // erase whole line
                clean_line(term, term->cur_y);
                return;
            }
        case 'L':  // insert lines
            if (!term->par[0])
                term->par[0] = 1;
            for (i=0; i<term->par[0]; i++)
                insert_line(term);
            //vt100_redraw(term);
            return;
        case 'M':  // erase lines
            if (!term->par[0])
                term->par[0] = 1;
            for (i=0; i<term->par[0]; i++)
                erase_line(term);
            //vt100_redraw(term);
            return;
        case 'c':  // we were asked to identify the terminal type
            identify(term);
            return;
        case 'g':  // erase tab(s)
            switch(term->par[0])
            {
                case 0:  remove_tab( term, term->cur_x );
                         break;
                case 3:  clear_tabs( term );
                         break;
                default: break;
            }
            return;
        case 'm':
            csi_m(term);
            return;
        case 'r':  // set scrolling region
            if (term->par[0] == 0)
                term->par[0]++;
            if (term->par[1] == 0)
                term->par[1] = term->phys_h;
            /* Minimum allowed region is 2 lines */
            if (term->par[0] < term->par[1] &&
                term->par[1] <= term->phys_h)
            {
                term->scroll_top = term->par[0] - 1;
                term->scroll_bottom = term->par[1]; // this line is excluded
                // if in origin mode, make sure that the cursor is placed
                // inside the scrolling region immediately
                if (term->origin_mode == VT100_ORIGIN_SCROLL)
                {
                    term->cur_x = 0;
                    term->cur_y = term->scroll_top;
                }
            }
            return;
        }
    case EShash:
        term->esc_state = ESnormal;
        switch(c)
        {
        case '8':  // fill the screen with 'E'
            clean_screen(term, 'E');
            //vt100_redraw(term);
            break;
        }
        break;
    }
}

/* moves the cursor to the position xy, if not possible move as close
 * as possible
 */
void cursor_move_abs(termstate_t *term, int x, int y)
{
    if (term->origin_mode == VT100_ORIGIN_GLOBAL)
    {
        term->cur_x = MIN(MAX(0, x), term->w - 1);
        term->cur_y = MIN(MAX(0, y), term->phys_h - 1);
    }
    else // VT100_ORIGIN_SCROLL
    {
        term->cur_x = MIN(MAX(0, x), term->w - 1);
        term->cur_y = MIN(MAX(term->scroll_top, y), term->scroll_bottom - 1);
    }
}

// move cursor to next character
void cursor_next(termstate_t *term)
{
    // inc. cursor pos.
    term->cur_x++;

    // ATTENTION: cur_x is in the range 0-79, while
    // vt100 positions are in range 1-80. We need to
    // check term->w-1 here
    if (term->cur_x >= term->w)
    {
        // if autowrap is on, go to new line
        if (term->autowrap)
        {
           cursor_cr(term);
           cursor_lf(term);
        }
        else // remain at the last position in line
            term->cur_x = term->w - 1;
    }
}

// move cursor to next tabstop
void cursor_tab(termstate_t *term)
{
    // if there are no tabs defined, use tab = 8 chars
    // note: this is not requested by the standard
    if (term->tabs[0]==0 && term->tab_last == -1)
    {
        term->cur_x = ((term->cur_x + 8) / 8) * 8;
        if (term->cur_x > term->w)
            cursor_nl(term);
    }
    else // go to next tab in tab array
    {
        term->cur_x = next_tab( term, term->cur_x );
    }
}

// move cursor to next line
void cursor_nl(termstate_t *term)
{
    // move to start of line, if we are in carriage return
    // mode, move one line down else
    if (term->newline)
        term->cur_x = 0;
    else // care for cursor out of bounds
    {
        if (term->cur_x >= term->w - 1)
            term->cur_x = term->w - 1;
    }

    // if our new cursor position exceeds phys_h we scroll the whole terminal
    // for that the lost line is stored in the history
    if (term->cur_y + 1 > term->phys_h - 1)
        scroll(term);
    // else check if we hit the bottom of a scrolling region
    else if (term->cur_y > term->scroll_bottom - 1)
        scroll_region(term, term->scroll_top, term->scroll_bottom);
    else
        term->cur_y++;
}

// line feed
void cursor_lf(termstate_t *term)
{
    if (term->cur_y + 1 > term->phys_h - 1)
        scroll(term);
    else if (term->cur_y >= term->scroll_bottom)
        scroll_region(term, term->scroll_top, term->scroll_bottom);
    else
        term->cur_y++;

    //vt100_redraw(term);
}

// carriage return
void cursor_cr(termstate_t *term)
{
    term->cur_x = 0;
    //vt100_redraw(term);
}

// clean to end of line
void clean_to_eol(termstate_t *term)
{
    int x;
    for (x = term->cur_x; x < term->w; x++)
        set_char_xy(term, ' ', x, term->cur_y);
}

// clean to end of screen
void clean_to_eos(termstate_t *term)
{
    int y;
    clean_to_eol(term);
    if (term->cur_y + 1 < term->phys_h)
        for (y = term->cur_y + 1; y < term->phys_h; y++)
            clean_line(term,  y);
}

// clean from start of line
void clean_from_sol(termstate_t *term)
{
    int x;
    for (x = 0; x <= term->cur_x; x++)
        set_char_xy(term, ' ', x, term->cur_y);
}

// clean from start of screen
void clean_from_sos(termstate_t *term)
{
    int y;
    clean_from_sol(term);
    if (term->cur_y + 1 < term->phys_h)
        for (y = 0; y < term->cur_y; y++)
            clean_line(term,  y);
}

void clean_line(termstate_t *term, int y)
{
    int x;
    for (x = 0; x < term->w; x++)
        set_char_xy(term, ' ', x, y);
}

void clean_screen(termstate_t *term, unsigned char c)
{
    int x, y;
    for (y = 0; y < term->phys_h; y++)
        for (x = 0; x < term->w; x++)
            set_char_xy(term, c, x, y);
}

// doing this twice should be like a nop
void invert_screen(termstate_t *term)
{
    int x, y;
    int fg, bg, in;

    // swap fg and bg for all chars
    for (y = 0; y < term->phys_h; y++)
        for (x = 0; x < term->w; x++)
        {
            unpack_attribs(term->attrib[xy2index(term, x, y)],
                           &fg, &bg, &in);
            term->attrib[xy2index(term, x, y)] = pack_attribs(bg, fg, in);
        }
}

void csi_m(termstate_t * term)
{
    int i;
    int fg, bg, in;

    for (i = 0; i <= term->used_par; i++)
    {
        switch (term->par[i])
        {
        // more cases here
        case 0:  // clear attributes
            term->attrib_mode = pack_attribs(DEFAULT_FG, DEFAULT_BG, 1);
            term->inverse = 0;
            break;
        case 1:  // bold
            unpack_attribs(term->attrib_mode, &fg, &bg, &in);
            term->attrib_mode = pack_attribs(fg, bg, 2);
            break;
        case 2:  // dimmed
            unpack_attribs(term->attrib_mode, &fg, &bg, &in);
            term->attrib_mode = pack_attribs(fg, bg, 0);
            break;
        case 5:  // blink
            // we dont support blink, currently
            // term->blink = 1;
            break;
        case 7: // inverse
            if (term->inverse == 0)
            {  // flip colors
                unpack_attribs(term->attrib_mode, &fg, &bg, &in);
                term->attrib_mode = pack_attribs(bg, fg, in);
            }
            term->inverse = 1;
            break;
        case 21:
        case 22: // forget bold
            unpack_attribs(term->attrib_mode, &fg, &bg, &in);
            term->attrib_mode = pack_attribs(fg, bg, 1);
            break;
        case 27: // forget inverse
            if (term->inverse == 1)
            {  // flip colors back
                unpack_attribs(term->attrib_mode, &fg, &bg, &in);
                term->attrib_mode = pack_attribs(bg, fg, in);
            }
            term->inverse = 0;
            break;
        default:
            if (term->par[i] >= 30 && term->par[i] <= 37)
            {
                unpack_attribs(term->attrib_mode, &fg, &bg, &in);
                term->attrib_mode = pack_attribs(term->par[i] - 30, bg, in);
            }
            else if (term->par[i] >= 40 && term->par[i] <= 47)
            {
                unpack_attribs(term->attrib_mode, &fg, &bg, &in);
                term->attrib_mode = pack_attribs(fg, term->par[i] - 40, in);
            }
            break;
        }
    }
}

/* switches some terminal flags on or off
 *
 * returns wether a redraw is necessary
 */
int set_mode(termstate_t * term, int on_off)
{
    int i, _redraw = 0;

    for (i = 0; i <= term->used_par; i++)
    {
        if (term->ques)
        {
            switch(term->par[i])
            {
            case 5:  // Inverted screen on/off
                if (term->decscnm != on_off)
                {
                    term->decscnm = on_off;
                    invert_screen(term);
                    _redraw = 1;
                }
                break;
            }
        }
        else
        {
        }
    }
    return _redraw;
}

void identify(termstate_t * term)
{
    // Here we should respond with our terminal ID: VT100ID or VT102ID.
    // I.e. we must insert the corresponding string into the read
    // buffer.

    vt100_add_ascii(term, VT100ID);
}
