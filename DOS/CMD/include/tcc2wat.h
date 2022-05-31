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
 */

#ifndef __TCC2WAT
#define __TCC2WAT 1
#ifdef __WATCOMC__
#include <graph.h>
#include <time.h>
#include <sys/types.h>

struct ftime /* As defined by Borland C */
{
    unsigned    ft_tsec  : 5;   /* Two second interval */
    unsigned    ft_min   : 6;   /* Minutes */
    unsigned    ft_hour  : 5;   /* Hours */
    unsigned    ft_day   : 5;   /* Days */
    unsigned    ft_month : 4;   /* Months */
    unsigned    ft_year  : 7;   /* Year */
};

struct fatinfo
{
    char fi_sclus;           /* Sectors per cluster */
    char fi_fatid;           /* The FAT id byte */
    int fi_nclus;            /* Number of clusters */
    int fi_bysec;            /* Bytes per sector */
};

struct	fcb
{
    char	fcb_drive;		/* 0 = default, 1 = A, 2 = B */
    char	fcb_name[8];		/* File name */
    char	fcb_ext[3];		/* File extension */
    short	fcb_curblk;		/* Current block number */
    short	fcb_recsize;		/* Logical record size in bytes */
    long	fcb_filsize;		/* File size in bytes */
    short	fcb_date;		/* Date file was last written */
    char	fcb_resv[10];		/* Reserved for DOS */
    char	fcb_currec;		/* Current record in block */
    long	fcb_random;		/* Random record number */
};

struct text_info
{
    unsigned char winleft;        /* left window coordinate */
    unsigned char wintop;         /* top window coordinate */
    unsigned char winright;       /* right window coordinate */
    unsigned char winbottom;      /* bottom window coordinate */
    unsigned char attribute;      /* text attribute */
    unsigned char normattr;       /* normal attribute */
    unsigned char currmode;       /* video mode */
    unsigned char screenheight;   /* bottom - top */
    unsigned char screenwidth;    /* right - left */
    unsigned char curx;           /* x coordinate in window */
    unsigned char cury;           /* y coordinate in window */
};

#define DOS_GETFTIME _dos_getftime
#define DOS_SETFTIME _dos_setftime
#define FAR _far

#define MAXPATH _MAX_PATH
#define MAXFILE _MAX_FNAME
#define MAXEXT _MAX_EXT
#define MAXDRIVE _MAX_DRIVE
#define MAXDIR _MAX_DIR

#define FA_RDONLY _A_RDONLY
#define FA_ARCH _A_ARCH
#define FA_SYSTEM _A_SYSTEM
#define FA_HIDDEN _A_HIDDEN
#define FA_DIREC _A_SUBDIR
#define FA_LABEL _A_VOLID
#define mktemp _mktemp

#define findfirst(x,y,z) _dos_findfirst(x,z,y)
#define findnext _dos_findnext
#define ffblk find_t
#define ff_name name
#define ff_attrib attrib
#define ff_reserved reserved
#define ff_ftime wr_time
#define ff_date wr_date
#define ff_fsize size

#define getdfree _dos_getdiskfree
#define dfree diskfree_t
#define df_avail avail_clusters
#define df_sclus sectors_per_cluster
#define df_bsec bytes_per_sector

#define fnmerge(a,b,c,d,e) _makepath(a,b,c,d,e)
#define SKIPSEP 0x1
#define NODRIVE 0x2
#define NOFNAME 0x4
#define NOEXT 0x8
#define WILDCARDS 0x01
#define EXTENSION 0x02
#define FILENAME 0x04
#define DIRECTORY 0x08
#define DRIVE 0x10

#define date dosdate_t
#define da_year year
#define da_mon month
#define da_day day
#define ti_hour hour
#define ti_min minute
#define ti_sec second
#define ti_hund hsecond
#define time dostime_t
#define getdate _dos_getdate
#define gettime _dos_gettime
#define setdate _dos_setdate
#define settime _dos_settime

#define bioscom(x,y,z) _bios_serialcom(x,z,y)
#define biosequip _bios_equiplist
#define bioskey _bios_keybrd
#define biosprint(x,y,z) _bios_printer(x,z,y)

#define inport inpw
#define inportb (char)inp
#define outport outpw
#define outportb(x,y) outp(x,(char)y)

#define r_ax w.ax
#define r_bx w.bx
#define r_cx w.cx
#define r_dx w.dx
#define r_bp w.bp
#define r_si w.si
#define r_di w.di
#define r_ds w.ds
#define r_es w.es
#define r_flags w.flags

#define setverify(value) bdos(0x2e,0,(value))
#define getverify() bdos(0x54,0,0)&0xff
#define getvect _dos_getvect
#define setvect _dos_setvect
#define keep _dos_keep

#define _mexcep int
#define pow10(x) pow(10,(double)x)
#define _matherr __matherr
typedef struct _complex complex;
#define complex(x,y) __complex(x,y)
#define imag(a) (a.y)
#define real(a) (a.x)
#define arg(a) (hypot(a.x, a.y))
#define randomize() srand( ((unsigned int)time(NULL)) | 1)

/*#define absread(a,b,c,d) _absrw(_DISK_READ,a,b,c,d)
#define abswrite(a,b,c,d) _absrw(_DISK_WRITE,a,b,c,d)*/

#define strncmpi strnicmp

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define LIGHTGRAY 7
#define DARKGRAY 8
#define LIGHTBLUE 9
#define LIGHTGREEN 10
#define LIGHTCYAN 11
#define LIGHTRED 12
#define LIGHTMAGENTA 13
#define YELLOW 14
#define WHITE 15
#define BLINK 16

#if !defined(COLORMODE)
#define COLORMODE  ((*(char FAR *)0x0449) != 7)
#define EXT_KBD    (*(char FAR *)0x0496 & 16)
#define VIDPAGE    (*((unsigned char far *)0x0462))
#define ROWSIZE    (*(int FAR *)0x044A)
#define SCANLINES  ((int)*(char FAR*)0x0461)
#define SCRBUFF    ((unsigned FAR *)((COLORMODE)?0xB8000000:0xB0000000))
#define SCREENSEG  ((unsigned)((COLORMODE)?0xB800:0xB000))
#define SCREENSIZE ((*(int FAR *)0x044C) >> 1)
#define SCREENCOLS (*(int FAR *)0x044A)
#define SCREENROWS ((*(char FAR *)0x0484)?1+(*(char FAR *)0x0484):25)
#endif
#define SCROLL_UP 0
#define SCROLL_DN 1

#define LASTMODE _DEFAULTMODE
#define BW40 _TEXTBW40
#define C40 _TEXTC40
#define BW80 _TEXTBW80
#define C80 _TEXTC80
#define MONO _TEXTMONO
#define textmode _setvideomode
#define window(a,b,c,d) _settextwindow(b,a,d,c)
#define clrscr() _clearscreen(_GWINDOW)
#define cleardevice() _clearscreen(_GCLEARSCREEN)
#define clearviewport() _clearscreen(_GVIEWPORT)
#define closegraph() _setvideomode(_DEFAULTMODE)
#define rectangle(a,b,c,d) _rectangle(_GFILLINTERIOR, (short)a, (short)b, (short)c, (short)d)

#define _NOCURSOR 0x2000
#define _SOLIDCURSOR 0x0007
#define _NORMALCURSOR 0x0607
#define _setcursortype _settextcursor

#define toascii(c) ((c)&0177)

struct _complex complex(double real, double imag);
int random(int num);
int stime(time_t *tp);
#if __WATCOMC__ < 1280
int _chmod(const char *filename, int func, ...);
#endif
int getftime (int handle, struct ftime *ftimep);
int setftime (int handle, struct ftime *ftimep);
char *searchpath(const char *filename);
int biosdisk(int cmd, int drive, int head, int track, int sector, int nsects, void *buffer);
int absread(int DosDrive, int nsects, int foo, void *diskReadPacket);
int abswrite(int DosDrive, int nsects, int foo, void *diskReadPacket);
long *biostime(int cmd, long *newtime);
void ctrlbrk(int (*fptr)(void));
int getcbrk(void);
int getcurdir(int drive, char *direc);
int fnsplit(const char *path, char *drive, char *dir, char *name, char *ext);
char far *getdta();
void setdta(char far *dta);
void getfat(unsigned char drive, struct fatinfo *fatblkp);
void getfatd(struct fatinfo *fatblkp);
unsigned getpsp(void);
int ioctl(int handle, int cmd, ...);
char *getpass(const char *prompt);
double _matherr(_mexcep why, char *fun, double arg1p, double arg2p, double retval);
int _read(int handle, void *buf, unsigned nbyte);
char *parsfnm(const char *cmdline, struct fcb *fcbptr, int option);
int setcbrk(int value);
int getdisk(void);
int setdisk(int drive);
char *stpcpy (char *dest, const char *src);
void textbackground(int color);
void textcolor(int color);
void textattr(int attr);
void highvideo (void);
void lowvideo (void);
void normvideo (void);
#if __WATCOMC__ < 1280
int _write(int handle, void *buf, int nbyte);
#endif
int _creat(const char *filename, mode_t attrib);
int creatnew(const char *filename, int attrib);
int creattemp(char *filename, int attrib);
void gettextinfo(struct text_info inforec);
int randbrd(struct fcb *fcbptr, int reccnt);
int randbwr(struct fcb *fcbptr, int reccnt);
int wherex(void);
int wherey(void);
void gotoxy(int col, int row);
void clreol(void);
unsigned long dostounix (struct date *d, struct time *t);
void unixtodos (unsigned long time, struct date *d, struct time *t);

#endif /*__WATCOMC__*/
#endif /*__TCC2WAT*/
