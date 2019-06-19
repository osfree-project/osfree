/* This file should only be used and compiled if you want to use checker.
 * It contains wrappers für PDCurses for X11.
 */
#define CURSES_LIBRARY
/* Set the include path either to the checker install path or copy
 * checker_api.h to a standard path:
 */
#include <checker_api.h>

#ifdef __CHECKER__
#  error This file can only be compiled without checkergcc. Use gcc for this file and checkergcc for the rest of your files!
#endif

#define CHECK_WIN(win) stubs_chkr_check_addr(win,sizeof(WINDOW),CHKR_TW,"win")


extern int chkr_PDC_chadd(register WINDOW *win, chtype ch,bool xlat, bool advance) __asm__ ("chkr.PDC_chadd");
int chkr_PDC_chadd(register WINDOW *win, chtype ch,bool xlat, bool advance)
{
   CHECK_WIN(win);
   return(PDC_chadd(win,ch,xlat,advance));
}

extern int chkr_PDC_getclipboard(char **contents, long *length) __asm__ ("chkr.PDC_getclipboard");
int chkr_PDC_getclipboard(char **contents, long *length)
{
   int rc;

   stubs_chkr_check_addr(contents, sizeof (char **), CHKR_MW, "contents");
   stubs_chkr_check_addr(length, sizeof (long *), CHKR_MW, "length");
   if ((rc = PDC_getclipboard(contents,length)) == PDC_CLIP_SUCCESS)
      stubs_chkr_set_right(*contents, *length, CHKR_RW);
   return(rc);
}

extern int chkr_PDC_setclipboard(char *contents, long length) __asm__ ("chkr.PDC_setclipboard");
int chkr_PDC_setclipboard(char *contents, long length)
{
   stubs_chkr_check_addr(contents, length, CHKR_RO, "contents");
   return(PDC_setclipboard(contents,length));
}

extern void chkr_PDC_set_title(char *title) __asm__ ("chkr.PDC_set_title");
void chkr_PDC_set_title(char *title)
{
   stubs_chkr_check_str(title, CHKR_RO, "title");
   PDC_set_title(title);
}

extern int chkr_PDC_ungetch(int ch) __asm__ ("chkr.PDC_ungetch");
int chkr_PDC_ungetch(int ch)
{
   return(PDC_ungetch(ch));
}

extern int chkr_beep(void) __asm__ ("chkr.beep");
int chkr_beep(void)
{
   return(beep());
}

extern int chkr_cbreak(void) __asm__ ("chkr.cbreak");
int chkr_cbreak(void)
{
   return(cbreak());
}

extern int chkr_curs_set(int arg) __asm__ ("chkr.curs_set");
int chkr_curs_set(int arg)
{
   return(curs_set(arg));
}

extern int chkr_delwin(WINDOW *win) __asm__ ("chkr.delwin");
int chkr_delwin(WINDOW *win)
{
   CHECK_WIN(win);
   return(delwin(win));
}

extern int chkr_doupdate(void) __asm__ ("chkr.doupdate");
int chkr_doupdate(void)
{
   return(doupdate());
}

extern int chkr_endwin(void) __asm__ ("chkr.endwin");
int chkr_endwin(void)
{
   return(endwin());
}

extern int chkr_init_pair(short a,short b,short c) __asm__ ("chkr.init_pair");
int chkr_init_pair(short a,short b,short c)
{
   return(init_pair(a,b,c));
}

extern WINDOW * chkr_initscr(void) __asm__ ("chkr.initscr");
WINDOW * chkr_initscr(void)
{
   WINDOW *retval = initscr();
   SCREEN *NewSP;
   if (retval != NULL)
      {
         stubs_chkr_set_right(retval,sizeof(WINDOW), CHKR_RW);
         stubs_chkr_set_right(stdscr,sizeof(WINDOW), CHKR_RW);
         stubs_chkr_set_right(curscr,sizeof(WINDOW), CHKR_RW);
         /* SP was allocated by calloc which is implemented with errors
            by checker. We redo it.
          */
         NewSP = malloc(sizeof(SCREEN));
         *NewSP = *SP;
         stubs_chkr_set_right(NewSP,sizeof(SCREEN),CHKR_RW);
         SP = NewSP;
      }
   return(retval);
}

extern WINDOW * chkr_Xinitscr(int argc,char argv[]) __asm__ ("chkr.Xinitscr");
WINDOW * chkr_Xinitscr(void)
{
   WINDOW *retval = initscr();
   SCREEN *NewSP;
   int i;

   if (argv != NULL)
      stubs_chkr_check_addr(argv, sizeof(char *) * argc, CHKR_RO, "argv");
   for (i = 0;i <argc,i++)
      stubs_chkr_check_str(argv[i], CHKR_RO, "argv[i]");
   retval = Xinitscr(argc,argv);
   if (retval != NULL)
      {
         stubs_chkr_set_right(retval,sizeof(WINDOW), CHKR_RW);
         stubs_chkr_set_right(stdscr,sizeof(WINDOW), CHKR_RW);
         stubs_chkr_set_right(curscr,sizeof(WINDOW), CHKR_RW);
         /* SP was allocated by calloc which is implemented with errors
            by checker. We redo it.
          */
         NewSP = malloc(sizeof(SCREEN));
         *NewSP = *SP;
         stubs_chkr_set_right(NewSP,sizeof(SCREEN),CHKR_RW);
         SP = NewSP;
      }
   return(retval);
}

extern int chkr_mouse_set(unsigned long ul) __asm__ ("chkr.mouse_set");
int chkr_mouse_set(unsigned long ul)
{
   return(mouse_set(ul));
}

extern WINDOW * chkr_newwin(int nlines, int ncols, int begy, int begx) __asm__ ("chkr.newwin");
WINDOW * chkr_newwin(int nlines, int ncols, int begy, int begx)
{
   WINDOW *retval = newwin(nlines,ncols,begy,begx);
   if (retval != NULL)
      stubs_chkr_set_right(retval,sizeof(WINDOW), CHKR_RW);
   return(retval);
}

extern int chkr_nodelay(WINDOW *win,int arg) __asm__ ("chkr.nodelay");
int chkr_nodelay(WINDOW *win,int arg)
{
   CHECK_WIN(win);
   return(nodelay(win,arg));
}

extern int chkr_noecho(void) __asm__ ("chkr.noecho");
int chkr_noecho(void)
{
   return(noecho());
}

extern int chkr_notimeout(WINDOW *win,bool arg) __asm__ ("chkr.notimeout");
int chkr_notimeout(WINDOW *win,bool arg)
{
   return(notimeout(win,arg));
}

extern int chkr_raw(void) __asm__ ("chkr.raw");
int chkr_raw(void)
{
   return(raw());
}

extern int chkr_raw_output(bool arg) __asm__ ("chkr.raw_output");
int chkr_raw_output(bool arg)
{
   return(raw_output(arg));
}

extern int chkr_request_mouse_pos(void) __asm__ ("chkr.request_mouse_pos");
int chkr_request_mouse_pos(void)
{
   return(request_mouse_pos());
}

extern int chkr_reset_prog_mode(void) __asm__ ("chkr.reset_prog_mode");
int chkr_reset_prog_mode(void)
{
   return(reset_prog_mode());
}

extern int chkr_resize_term(int nlines,int ncols) __asm__ ("chkr.resize_term");
int chkr_resize_term(int nlines,int ncols)
{
   return(resize_term(nlines,ncols));
}

extern int chkr_sb_get_horz(int *total,int *viewport,int *cur) __asm__ ("chkr.sb_get_horz");
int chkr_sb_get_horz(int *total,int *viewport,int *cur)
{
   int retval;
   stubs_chkr_check_addr(total, sizeof (int), CHKR_MW, "total");
   stubs_chkr_check_addr(viewport, sizeof (int), CHKR_MW, "viewport");
   stubs_chkr_check_addr(cur, sizeof (int), CHKR_MW, "cur");
   retval = sb_get_horz(total,viewport,cur);
   if (retval == OK)
      {
         stubs_chkr_set_right(total,sizeof(int), CHKR_RW);
         stubs_chkr_set_right(viewport,sizeof(int), CHKR_RW);
         stubs_chkr_set_right(cur,sizeof(int), CHKR_RW);
      }
   return(retval);
}

extern int chkr_sb_get_vert(int *total,int *viewport,int *cur) __asm__ ("chkr.sb_get_vert");
int chkr_sb_get_vert(int *total,int *viewport,int *cur)
{
   int retval;
   stubs_chkr_check_addr(total, sizeof (int), CHKR_MW, "total");
   stubs_chkr_check_addr(viewport, sizeof (int), CHKR_MW, "viewport");
   stubs_chkr_check_addr(cur, sizeof (int), CHKR_MW, "cur");
   retval = sb_get_vert(total,viewport,cur);
   if (retval == OK)
      {
         stubs_chkr_set_right(total,sizeof(int), CHKR_RW);
         stubs_chkr_set_right(viewport,sizeof(int), CHKR_RW);
         stubs_chkr_set_right(cur,sizeof(int), CHKR_RW);
      }
   return(retval);
}

extern int chkr_sb_init(void) __asm__ ("chkr.sb_init");
int chkr_sb_init(void)
{
   return(sb_init());
}

extern int chkr_sb_refresh(void) __asm__ ("chkr.sb_refresh");
int chkr_sb_refresh(void)
{
   return(sb_refresh());
}

extern int chkr_sb_set_horz(int total,int viewport,int cur) __asm__ ("chkr.sb_set_horz");
int chkr_sb_set_horz(int total,int viewport,int cur)
{
   return(sb_set_horz(total,viewport,cur));
}

extern int chkr_sb_set_vert(int total,int viewport,int cur) __asm__ ("chkr.sb_set_vert");
int chkr_sb_set_vert(int total,int viewport,int cur)
{
   return(sb_set_vert(total,viewport,cur));
}

extern int chkr_slk_clear(void) __asm__ ("chkr.slk_clear");
int chkr_slk_clear(void)
{
   return(slk_clear());
}

extern int chkr_slk_init(int fmt) __asm__ ("chkr.slk_init");
int chkr_slk_init(int fmt)
{
   return(slk_init(fmt));
}

extern int chkr_slk_noutrefresh(void) __asm__ ("chkr.slk_noutrefresh");
int chkr_slk_noutrefresh(void)
{
   return(slk_noutrefresh());
}

extern int chkr_slk_set(int labnum,char *label,int fmt) __asm__ ("chkr.slk_set");
int chkr_slk_set(int labnum,char *label,int fmt)
{
   if (label != NULL)
      stubs_chkr_check_str(label, CHKR_RO, "label");
   return(slk_set(labnum,label,fmt));
}

extern int chkr_slk_touch(void) __asm__ ("chkr.slk_touch");
int chkr_slk_touch(void)
{
   return(slk_touch());
}

extern int chkr_start_color(void) __asm__ ("chkr.start_color");
int chkr_start_color(void)
{
   return(start_color());
}

extern int chkr_typeahead(int filedes) __asm__ ("chkr.typeahead");
int chkr_typeahead(int filedes)
{
   if (filedes != -1)
      fd_used_by_prog(filedes);
   return(typeahead(filedes));
}

extern int chkr_waddchnstr(WINDOW *win,chtype *ch,int n) __asm__ ("chkr.waddchnstr");
int chkr_waddchnstr(WINDOW *win,chtype *ch,int n)
{
   CHECK_WIN(win);
   stubs_chkr_check_addr(ch, sizeof (chtype) * n, CHKR_MW, "ch");
   return(waddchnstr(win,ch,n));
}

extern int chkr_waddstr(WINDOW *win,char *str) __asm__ ("chkr.waddstr");
int chkr_waddstr(WINDOW *win,char *str)
{
   CHECK_WIN(win);
   stubs_chkr_check_str(str, CHKR_RO, "str");
   return(waddstr(win,str));
}

extern int chkr_wattrset(WINDOW *win,attr_t attrs) __asm__ ("chkr.wattrset");
int chkr_wattrset(WINDOW *win,attr_t attrs)
{
   CHECK_WIN(win);
   return(wattrset(win,attrs));
}

extern int chkr_wbkgd(WINDOW *win,chtype ch) __asm__ ("chkr.wbkgd");
int chkr_wbkgd(WINDOW *win,chtype ch)
{
   CHECK_WIN(win);
   return(wbkgd(win,ch));
}

extern int chkr_wborder(WINDOW *win, chtype ls, chtype rs, chtype ts,chtype bs, chtype tl, chtype tr, chtype bl, chtype br) __asm__ ("chkr.wborder");
int chkr_wborder(WINDOW *win, chtype ls, chtype rs, chtype ts,chtype bs, chtype tl, chtype tr, chtype bl, chtype br)
{
   CHECK_WIN(win);
   return(wborder(win,ls,rs,ts,bs,tl,tr,bl,br));
}

extern int chkr_wclrtobot(WINDOW *win) __asm__ ("chkr.wclrtobot");
int chkr_wclrtobot(WINDOW *win)
{
   CHECK_WIN(win);
   return(wclrtobot(win));
}

extern int chkr_wdelch(WINDOW *win) __asm__ ("chkr.wdelch");
int chkr_wdelch(WINDOW *win)
{
   CHECK_WIN(win);
   return(wdelch(win));
}

extern int chkr_wgetch(WINDOW *win) __asm__ ("chkr.wgetch");
int chkr_wgetch(WINDOW *win)
{
   CHECK_WIN(win);
   return(wgetch(win));
}

extern int chkr_whline(WINDOW *win,chtype ch,int n) __asm__ ("chkr.whline");
int chkr_whline(WINDOW *win,chtype ch,int n)
{
   CHECK_WIN(win);
   return(whline(win,ch,n));
}

extern int chkr_winsch(WINDOW *win,chtype ch) __asm__ ("chkr.winsch");
int chkr_winsch(WINDOW *win,chtype ch)
{
   CHECK_WIN(win);
   return(winsch(win,ch));
}

extern void chkr_wmouse_position(WINDOW *win,int *y,int *x) __asm__ ("chkr.wmouse_position");
void chkr_wmouse_position(WINDOW *win,int *y,int *x)
{
   CHECK_WIN(win);
   stubs_chkr_check_addr(y, sizeof (int), CHKR_MW, "y");
   stubs_chkr_check_addr(x, sizeof (int), CHKR_MW, "x");
   wmouse_position(win,y,x);
}

extern int chkr_wmove(WINDOW *win,int y,int x) __asm__ ("chkr.wmove");
int chkr_wmove(WINDOW *win,int y,int x)
{
   CHECK_WIN(win);
   return(wmove(win,y,x));
}

extern int chkr_wnoutrefresh(WINDOW *win) __asm__ ("chkr.wnoutrefresh");
int chkr_wnoutrefresh(WINDOW *win)
{
   CHECK_WIN(win);
   return(wnoutrefresh(win));
}

extern int chkr_wrefresh(WINDOW *win) __asm__ ("chkr.wrefresh");
int chkr_wrefresh(WINDOW *win)
{
   CHECK_WIN(win);
   return(wrefresh(win));
}

extern int chkr_wtouchln(WINDOW *win,int arg1,int arg2,int arg3) __asm__ ("chkr.wtouchln");
int chkr_wtouchln(WINDOW *win,int arg1,int arg2,int arg3)
{
   CHECK_WIN(win);
   return(wtouchln(win,arg1,arg2,arg3));
}

extern int chkr_wvline(WINDOW *win,chtype arg1,int arg2) __asm__ ("chkr.wvline");
int chkr_wvline(WINDOW *win,chtype arg1,int arg2)
{
   CHECK_WIN(win);
   return(wvline(win,arg1,arg2));
}
