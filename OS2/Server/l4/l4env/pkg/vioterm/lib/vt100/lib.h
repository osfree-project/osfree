/**
 * \file   l4vfs/term_server/lib/vt100/lib.h
 * \brief  
 *
 * \date   08/10/2004
 * \author Martin Pohlack  <mp26@os.inf.tu-dresden.de>
 */
/* (c) 2004 Technische Universitaet Dresden
 * This file is part of DROPS, which is distributed under the terms of the
 * GNU General Public License 2. Please see the COPYING file for details.
 */
#ifndef __L4VFS_TERM_SERVER_LIB_VT100_LIB_H_
#define __L4VFS_TERM_SERVER_LIB_VT100_LIB_H_

#include <l4/term_server/vt100.h>
#include <l4/semaphore/semaphore.h>

#define DEFAULT_FG 7
#define DEFAULT_BG 0

#define VT100ID "\033[?1;2c"
#define VT102ID "\033[?6c"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

// defines usually found in sys/ttydefaults.h which does not
// belong to dietlibc
#define TTYDEF_IFLAG    (BRKINT | ISTRIP | ICRNL | IMAXBEL | IXON | IXANY)
#define TTYDEF_OFLAG    (OPOST | ONLCR | XTABS)
#define TTYDEF_LFLAG    (ECHO | ICANON | ISIG | IEXTEN | ECHOE|ECHOKE|ECHOCTL)
#define TTYDEF_CFLAG    (CREAD | CS7 | PARENB | HUPCL)
#define TTYDEF_SPEED    (B9600)

#define IS_CTRL(x)  ((x)<32)

#ifdef USE_DIETLIBC
#define CTRL(x)     ((x)&037)
#define CEOF        CTRL('d')
#define CEOL        _POSIX_VDISABLE
#define CERASE      0177
#define CINTR       CTRL('c')
#define CSTATUS     _POSIX_VDISABLE
#define CKILL       CTRL('u')
#define CQUIT       034
#define CMIN        1
#define CSUSP       CTRL('z')
#define CSTART      CTRL('q')
#define CSTOP       CTRL('s')
#define CLNEXT      CTRL('v')
#define CDISCARD    CTRL('o')
#define CWERASE     CTRL('w')
#define CREPRINT    CTRL('r')
#define CEOT        CEOF
#define CBRK        CEOL
#define CRPRNT      CREPRINT
#define CFLUSH      CDISCARD
#endif

void scroll(termstate_t *term);
void scroll_region(termstate_t *term, int start, int end);
void rev_scroll(termstate_t *term);
void rev_scroll_region(termstate_t *term, int start, int end);
void set_char_xy(termstate_t *term, l4_int8_t c, int x, int y);
void set_char(termstate_t *term, l4_int8_t c);

void cursor_move_abs(termstate_t *term, int x, int y);
void cursor_cr(termstate_t *term);
void cursor_lf(termstate_t *term);
void cursor_nl(termstate_t *term);
void cursor_tab(termstate_t *term);
void cursor_next(termstate_t *term);
void clean_to_eol(termstate_t *term);
void clean_to_eos(termstate_t *term);
void clean_from_sol(termstate_t *term);
void clean_from_sos(termstate_t *term);
void clean_line(termstate_t *term, int y);
void clean_screen(termstate_t *term, unsigned char c);
void invert_screen(termstate_t *term);
void csi_m(termstate_t * term);
void identify( termstate_t *term );
int set_mode(termstate_t * term, int on_off);
void insert_line(termstate_t *term);
void insert_char(termstate_t *term, char c);
void erase_line(termstate_t *term);

void cursor_up(termstate_t *term);
void cursor_down(termstate_t *term);
void cursor_left(termstate_t *term);
void cursor_right(termstate_t *term);

void set_tab( termstate_t *term, int index );
void remove_tab( termstate_t *term, int index );
void clear_tabs( termstate_t *term );
int  next_tab( termstate_t *term, int pos );

char uppercase( char c );

// not needed anymore ???
//void update_cursor(termstate_t *term);

enum {ESnormal, ESesc, ESsquare, ESgetpars, ESgotpars, ESfunckey,
      EShash, ESsetG0, ESsetG1, ESpercent, ESignore, ESnonstd,
      ESpalette};

#endif
