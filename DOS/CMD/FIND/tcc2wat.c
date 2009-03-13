/* 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Currently missing functions are:
 * bcd bcd(int x);      or      bcd(double x, int decimals);
 * int bdosptr(int dosfun, void arg, unsigned dosal);
 * double conj(complex z);
 * struct country *country(int countrycode, country countryp);
 * void geninterrupt(int intr_num);
 * int gsignal(sig);
 * int movetext(int left, int top, int right, int bottom, int newleft, int newtop);
 * double norm(complex z);
 * complex polar(double magnit, double angle);
 * double poly(double x, int num, double cf[]);
 * int (*ssignal(int sig, int *action))();
 * Most graphics functions are missing; right now I'm focusing on text and regular functions
 */
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <bios.h>
#include <io.h>
#include <graph.h>
#include <errno.h>
#include <direct.h>
#include "tcc2wat.h"
#undef _matherr

complex __complex(double real, double imag)
{
	complex z;
	z.x = real;
	z.y = imag;
	return z;
}

int random(int num)
{
	int i = rand();
	while((num > 0 && i > num) || (num < 0 && i < num)) i = rand();
	return i;
}

int stime(time_t *tp)
{
	int a, b;
	struct time tm;
	struct date dt;
	unixtodos(*tp, &dt, &tm);
	a = _dos_setdate(&dt);
	b = _dos_settime(&tm);
	if(a == 0 && b == 0) return 0;
	else return -1;
}

void scroll(int direction, int num_lines, int vattrib, int ulrow, int ulcomumn, int lrrow, int lrcolumn)
{
      union REGS regs;
 
      /*
            BH = attribute to be used on blank line
            CH = row of upper left corner of scroll window
            CL = column of upper left corner of scroll window
            DH = row of lower right corner of scroll window
            DL = column of lower right corner of scroll window
      */
 
      regs.h.al = (unsigned char)num_lines;
      regs.h.bh = (unsigned char)vattrib;
      regs.h.ch = (unsigned char)ulrow;
      regs.h.cl = (unsigned char)ulcomumn;
      regs.h.dh = (unsigned char)lrrow;
      regs.h.dl = (unsigned char)lrcolumn;
 
      if (direction == SCROLL_UP)
            regs.h.ah = 0x06;
      else  regs.h.ah = 0x07;
 
      int86(0x10, &regs, &regs);
}

void GetCurPos(int *col, int *row)
{
      union REGS regs;

      regs.h.ah = 0x03;
      regs.h.bh = VIDPAGE;
      int86(0x10, &regs, &regs);
      *row = regs.h.dh;
      *col = regs.h.dl;
}

int GetCurAtr(void)
{
      int row, col;
      unsigned short chat;

      GetCurPos(&col, &row);
      chat = *((unsigned FAR *)MK_FP(SCREENSEG,
            (row * SCREENCOLS + col) << 1));
      return (chat >> 8);
}

#if __WATCOMC__ < 1280
int _chmod(const char *filename, int func, ...)	/* Get/set file attributes */
{
	va_list l;
	int attributes;
	unsigned attr;
	va_start(l, func);
	attributes = va_arg(l, int);
	va_end(l);
	if (func == 0)
	{
		if (_dos_getfileattr(filename, &attr) != 0) return -1;
		return attr;
	}
	if (_dos_setfileattr(filename, attributes) != 0) return -1;
	return 0;
}
#endif

int getftime (int handle, struct ftime *ftimep)	/* Get a file's time */
{
      int retval = 0;
      union
      {
            struct
            {
                  unsigned time;
                  unsigned date;
            } msc_time;
            struct ftime bc_time;
      } FTIME;

      if (0 == (retval = DOS_GETFTIME(handle, (unsigned short *)&FTIME.msc_time.date,
            (unsigned short *)&FTIME.msc_time.time)))
      {
            *ftimep = FTIME.bc_time;
      }
      return retval;
}

int setftime (int handle, struct ftime *ftimep)	/* Set a file's time */
{
      union
      {
            struct
            {
                  unsigned time;
                  unsigned date;
            } msc_time;
            struct ftime bc_time;
      } FTIME;

      FTIME.bc_time = *ftimep;

      return DOS_SETFTIME(handle, FTIME.msc_time.date, FTIME.msc_time.time);
}

char *searchpath(const char *filename)
{
	char fullpath[_MAX_PATH];
	_searchenv(filename, "PATH", fullpath);
	if (fullpath[0] == '\0') return NULL;
	return fullpath;
}

int biosdisk(int cmd, int drive, int head, int track, int sector, int nsects, void *buffer)
{
	struct diskinfo_t di;
	di.drive = drive;
	di.head = head;
	di.track = track;
	di.sector = sector;
	di.nsectors = nsects;
	di.buffer = buffer;
	return _bios_disk(cmd, &di);
}

long *biostime(int cmd, long *newtime)
{
	if(cmd == 0)
	{
		_bios_timeofday(_TIME_GETCLOCK, newtime);
		return newtime;
	}
	if(cmd == 1) _bios_timeofday(_TIME_SETCLOCK, newtime);
	return 0;
}

void ctrlbrk(int (*fptr)(void))
{
	_dos_setvect(0x23, (void (__interrupt __far *))fptr);
}

int getcbrk(void)
{
      union REGS regs;

      regs.h.ah = 0x33;
      intdos(&regs, &regs);
      return (int)regs.h.dl;
}

int getcurdir(int drive, char *direc)
{
	direc = _getdcwd(drive, NULL, 0);
	if (direc == NULL) return -1;
	return 0;
}

char far *getdta()
{
	union REGS reg;
	struct SREGS sreg;
        reg.h.ah = 0x2F;
        reg.x.bx = 0;
        reg.x.cx = 0;
        reg.x.dx = 0;
        intdosx(&reg, &reg, &sreg);
	return MK_FP(sreg.es, reg.x.bx);
}

void setdta(char far *dta)
{
	union REGS reg;
	struct SREGS sreg;
	reg.h.ah = 0x1A;
	sreg.ds = FP_SEG(dta);
	reg.x.dx = FP_OFF(dta);
	intdosx(&reg, &reg, &sreg);
}

void getfat(unsigned char drive, struct fatinfo *fatblkp)
{
	union REGS reg;
	struct SREGS sreg;
	reg.h.ah = 0x1C;
	reg.h.dl = drive;
        intdosx(&reg, &reg, &sreg);
	fatblkp->fi_sclus = reg.h.al;
	fatblkp->fi_fatid = sreg.ds + reg.x.bx;
	fatblkp->fi_nclus = reg.x.dx;
	fatblkp->fi_bysec = reg.x.cx;
}

void getfatd(struct fatinfo *fatblkp)
{
	union REGS reg;
	struct SREGS sreg;
	reg.h.ah = 0x1B;
        intdosx(&reg, &reg, &sreg);
	fatblkp->fi_sclus = reg.h.al;
	fatblkp->fi_fatid = (char)MK_FP(sreg.ds, reg.x.bx);
	fatblkp->fi_nclus = reg.x.dx;
	fatblkp->fi_bysec = reg.x.cx;
}

unsigned getpsp(void)
{
	union REGS reg;
	reg.h.ah = 0x62;
	intdos(&reg, &reg);
	return reg.x.bx;
}

int fnsplit(const char *path, char *drive, char *dir, char *name, char *ext)
{
	int a = 0;
	char rname[_MAX_FNAME];
	_splitpath(path, drive, dir, rname, ext);
	if (strchr(path, '.') != NULL) a+=EXTENSION;
	if (strlen(rname) > 0) a+=FILENAME;
	if (strchr(path, '\\') != NULL) a+=DIRECTORY;
	if (strchr(path, ':') != NULL) a+=DRIVE;
	if (strchr(path, '*') != NULL || strchr(path, '?') != NULL) a+=WILDCARDS;
	name = rname;
	return a;
}

char *parsfnm(const char *cmdline, struct fcb *fcbptr, int option)
{
	union REGS reg;
	struct SREGS sreg;
	reg.h.ah = 0x29;
	reg.h.dl = option;
	sreg.ds = FP_SEG(cmdline);
	reg.x.si = FP_OFF(cmdline);
	sreg.es = FP_SEG(fcbptr);
	reg.x.di = FP_SEG(fcbptr);
	intdosx(&reg, &reg, &sreg);
	return (char *)MK_FP(sreg.ds, reg.x.si);
}

int randbrd(struct fcb *fcbptr, int reccnt)
{
	union REGS reg;
	struct SREGS sreg;
	reg.h.ah = 0x27;
	reg.x.cx = reccnt;
	sreg.ds = FP_SEG(fcbptr);
	reg.x.dx = FP_OFF(fcbptr);
	intdosx(&reg, &reg, &sreg);
	return reg.h.al;
}

int randbwr(struct fcb *fcbptr, int reccnt)
{
	union REGS reg;
	struct SREGS sreg;
	reg.h.ah = 0x28;
	reg.x.cx = reccnt;
	sreg.ds = FP_SEG(fcbptr);
	reg.x.dx = FP_OFF(fcbptr);
	intdosx(&reg, &reg, &sreg);
	return reg.h.al;
}

int ioctl(int handle, int cmd, ...)
{
	union REGS reg;
	reg.x.bx = handle;
	if (cmd == 0)
	{
		reg.x.ax = 0x4400;
		intdos(&reg, &reg);
	}
	if (cmd == 1)
	{
		va_list l;
		void *argdx;
		va_start(l, cmd);
		argdx = va_arg(l, void *);
		va_end(l);
		reg.x.ax = 0x4401;
		reg.x.dx = (short)argdx;
		intdos(&reg, &reg);
	}
	if (cmd == 2 || cmd == 3 || cmd == 4 || cmd == 5)
	{
		struct SREGS sreg;
		va_list l;
		void *argdx;
		int argcx;
		va_start(l, cmd);
		argdx = va_arg(l, void *);
		argcx = va_arg(l, int);
		va_end(l);
		if (cmd == 4 || cmd == 5) reg.h.bl = argcx;
		else reg.x.cx = argcx;
		sreg.ds = FP_SEG(argdx);
		reg.x.dx =FP_OFF(argdx);
		if(cmd == 2) reg.x.ax = 0x4402;
		if(cmd == 3) reg.x.ax = 0x4403;
		if(cmd == 4) reg.x.ax = 0x4404;
		if(cmd == 5) reg.x.ax = 0x4405;
		intdosx(&reg, &reg, &sreg);
		argdx = (void *)MK_FP(sreg.ds, reg.x.dx);
		return (reg.x.cflag ? -1 : reg.x.ax);
	}
	if (cmd == 6 || cmd == 7)
	{
		if (cmd == 6) reg.x.ax = 0x4406;
		if (cmd == 7) reg.x.ax = 0x4407;
		intdos(&reg, &reg);
		return (reg.x.cflag ? -1 : reg.h.al);
	}
	if (cmd == 8)
	{
		reg.x.ax = 0x4408;
		reg.h.bl = handle;
		intdos(&reg, &reg);
		return (reg.x.cflag ? -1 : reg.x.ax);
	}
	if (cmd == 11)
	{
		reg.x.ax = 0x440B;
		reg.x.dx = handle;
		intdos(&reg, &reg);
		return (reg.x.cflag ? reg.x.ax : 0);
	}
	if (cmd == 0 || cmd == 1) return (reg.x.cflag ? -1 : reg.x.dx);
	errno = EINVAL;
	return -1;
}

char *getpass(const char *prompt)
{
	char *password;
	char key = 0;
	cprintf("%s", prompt);
	password = (char *)malloc(9);
	sprintf(password, "        ");
	sprintf(password, "%c", key = getch());
	while(strlen(password) < 8 && key != 0x0D)
	{
		key = getch();
		sprintf(password, "%s%c", password, key);
	}
	putch('\n');
	return password;
}

double __matherr(_mexcep why, char *fun, double arg1p, double arg2p, double retval)
{
	struct _exception x;
	x.type = why;
	x.arg1 = arg1p;
	x.arg2 = arg2p;
	x.retval = retval;
	return _matherr(&x);
}

int _read(int handle, void *buf, unsigned nbyte)
{
	int a;
	unsigned bytes;
	if (a = _dos_read(handle, buf, nbyte, &bytes) == 0) return bytes;
	if (eof(handle)) return 0;
	return a;
}

#if __WATCOMC__ < 1280
int _write(int handle, void *buf, int nbyte)
{
	unsigned a;
	int oldmode = setmode(handle, O_BINARY);
	if(_dos_write(handle, buf, nbyte, &a) == 0) {setmode(handle, oldmode);return a;}
	setmode(handle, oldmode);
	return -1;
}
#endif

int _creat(const char *filename, int attrib)
{
	int a;
	if(_dos_creat(filename, attrib, &a) == 0) return a;
	else return -1;
}

int creatnew(const char *filename, int attrib)
{
	int handle;
	if(_dos_creatnew(filename, attrib, &handle) == 0) return handle;
	else return -1;
}

int setcbrk(int value)
{
	if(value == 1) {break_on();return 1;}
	if(value == 0) break_off();
	return 0;
}

int getdisk(void)
{
	unsigned drive;
	_dos_getdrive(&drive);
	return drive-1;
}

int setdisk(int drive)
{
	unsigned a;
	_dos_setdrive(drive-1, &a);
	return a;
}

void textbackground(int color)
{
	long hex;
	short tcolor;
	if (color > 15){_settextcolor(_gettextcolor() + BLINK);color-=16;}
	if (color == 0) hex = _BLACK;
	if (color == 1) hex = _BLUE;
	if (color == 2) hex = _GREEN;
	if (color == 3) hex = _CYAN;
	if (color == 4) hex = _RED;
	if (color == 5) hex = _MAGENTA;
	if (color == 6) hex = _BROWN;
	if (color == 7) hex = _WHITE;
	if (color == 8) hex = _GRAY;
	if (color == 9) hex = _LIGHTBLUE;
	if (color == 10)hex = _LIGHTGREEN;
	if (color == 11)hex = _LIGHTCYAN;
	if (color == 12)hex = _LIGHTRED;
	if (color == 13)hex = _LIGHTMAGENTA;
	if (color == 14)hex = _YELLOW;
	if (color == 15)hex = _BRIGHTWHITE;
	_setbkcolor(hex);
}

void textcolor(int color)
{
	_settextcolor(color);
}

void textattr(int attr)
{
	int bg = (attr >> 4) & 7, fg = attr & 15, blink = (attr >> 7) & 1;
	textbackground(bg);
	textcolor(fg + blink);
}

void highvideo (void)
{
	textcolor(15); /* White */
	textbackground(0); /* Black */
}

void lowvideo (void)
{
	textbackground(0); /* Black */
	textcolor(8); /* Should be darkgray */
}

void normvideo (void)
{
	textbackground(0); /* Black */
	textcolor(7); /* Lightgray */
}

int creattemp(char *filename, int attrib)
{
	char *tmpfile;
	if(filename[strlen(filename)-1] != '\\') {errno = ENOENT;return -1;}
	tmpfile = (char *)malloc(9);
	tmpfile = _mktemp("tmXXXXXX");
	if (tmpfile == NULL) return -1;
	return _creat(tmpfile, attrib);
}

void gettextinfo(struct text_info inforec)
{
	short left, top, right, bottom;
	long bkcolor;
	struct videoconfig vc;
	struct rccoord xy;
	_gettextwindow(&top, &left, &bottom, &right);
	_getvideoconfig(&vc);
	bkcolor = _getbkcolor();
	xy = _gettextposition();
	inforec.winleft = left;
	inforec.wintop = top;
	inforec.winright = right;
	inforec.winbottom = bottom;
	inforec.attribute = _gettextcolor();
	if (bkcolor == _BLACK) inforec.normattr = BLACK;
	if (bkcolor == _BLUE) inforec.normattr = BLUE;
	if (bkcolor == _GREEN) inforec.normattr = GREEN;
	if (bkcolor == _CYAN) inforec.normattr = CYAN;
	if (bkcolor == _RED) inforec.normattr = RED;
	if (bkcolor == _MAGENTA) inforec.normattr = MAGENTA;
	if (bkcolor == _BROWN) inforec.normattr = BROWN;
	if (bkcolor == _WHITE) inforec.normattr = LIGHTGRAY;
	if (bkcolor == _GRAY) inforec.normattr = DARKGRAY;
	if (bkcolor == _LIGHTBLUE) inforec.normattr = LIGHTBLUE;
	if (bkcolor == _LIGHTGREEN) inforec.normattr = LIGHTGREEN;
	if (bkcolor == _LIGHTCYAN) inforec.normattr = LIGHTCYAN;
	if (bkcolor == _LIGHTRED) inforec.normattr = LIGHTRED;
	if (bkcolor == _LIGHTMAGENTA) inforec.normattr = LIGHTMAGENTA;
	if (bkcolor == _YELLOW) inforec.normattr = YELLOW;
	if (bkcolor == _BRIGHTWHITE) inforec.normattr = WHITE;
	inforec.currmode = vc.mode;
	inforec.screenheight = bottom - top;
	inforec.screenwidth = right - left;
	inforec.curx = xy.col;
	inforec.cury = xy.row;
}

int wherex(void)
{
	struct rccoord xy;
	xy = _gettextposition();
	return xy.col;
}

int wherey(void)
{
	struct rccoord xy;
	xy = _gettextposition();
	return xy.row;
}

void delline (void)
{
	short left, top, right, bottom;
	int xcount, ycur;
	_gettextwindow(&top, &left, &bottom, &right);
	bottom = right-left;
	ycur = wherey();
	gotoxy (0, ycur);
	for (xcount = 0; xcount < bottom; xcount++) putchar (' ');
	gotoxy (0, ycur);
}

void gotoxy(int col, int row)
{
      union REGS regs;

      setbuf(stdout, NULL);
      regs.h.dh = (unsigned)row;
      regs.h.dl = (unsigned)col;
      regs.h.bh = VIDPAGE;
      regs.h.ah = 2;
      int86(0x10, &regs, &regs);
}

void clreol(void)
{
      int row, col;

      GetCurPos(&col, &row);
      scroll(0, 0, GetCurAtr(), row, col, row, SCREENCOLS);
}

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma aux absread =  \
      "push bp"           \
      "int 0x25"          \
      "sbb ax, ax"        \
      "popf"              \
      "pop bp"            \
      parm [ax] [cx] [dx] [bx] \
      modify [si di] \
      value [ax];

#pragma aux abswrite =  \
      "push bp"           \
      "int 0x26"          \
      "sbb ax, ax"        \
      "popf"              \
      "pop bp"            \
      parm [ax] [cx] [dx] [bx] \
      modify [si di] \
      value [ax];

char *stpcpy (char *dest, const char *src)
{
  register char *d = dest;
  register const char *s = src;

  do
    *d++ = *s;
  while (*s++ != '\0');

  return d - 1;
}

unsigned long dostounix (struct date *d, struct time *t)
{
  struct tm tm;

  if (!d || !t)
     return (0);

  tm.tm_year  = d->da_year - 1900;
  tm.tm_mon   = d->da_mon - 1;
  tm.tm_mday  = d->da_day;
  tm.tm_isdst = 0;
  tm.tm_hour  = t->ti_hour;
  tm.tm_min   = t->ti_min;
  tm.tm_sec   = t->ti_sec;
  return mktime (&tm);
}

void unixtodos (time_t time, struct date *d, struct time *t)
{
  struct tm *tm = localtime (&time);

  if (d && tm)
  {
    d->da_year = tm->tm_year;
    d->da_mon  = tm->tm_mon;
    d->da_day  = tm->tm_mday;
  }
  if (t && tm)
  {
    t->ti_hour = tm->tm_hour;
    t->ti_min  = tm->tm_min;
    t->ti_sec  = tm->tm_sec;
    t->ti_hund = 0;
  }
}
