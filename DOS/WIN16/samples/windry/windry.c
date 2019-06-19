
#define COUNT	long
#define ARG 	long

#include "windows.h"

#define LOOPS 10000

extern int    __argc;
extern char **__argv;

char	globchar;
long     globint;
BOOL    globbool;
char    memcopy[512];

ARG	proc3(char *s);
void	proc0();
void	proc1();
void 	proc2(ARG);
 
int PASCAL
WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char *title;
	char  text[512];
	long   flag;
	DWORD  timeStart,timeEnd;

	long   cnt;
	long  time;
	long   rc;

	flag = MB_OKCANCEL;
	cnt  = 0;
	time = 0;
	rc   = 0;
	title = "DryWin 1.0";


	for(;;) {
		wsprintf(text,
			"Press OK to run\nCancel to quit\n%ld: %ld",
			cnt,rc);
		if(MessageBox(0, text, title, flag) != IDOK)
			break;
		timeStart = GetCurrentTime();
		proc0();
		timeEnd   = GetCurrentTime();
		time += timeEnd - timeStart;
		cnt++;
		rc = time/cnt;
	} 
}


ARG
proc3(char *s)
{
	strcpy(memcopy,s);
	return (ARG)strlen(memcopy);	
}

void
proc0()
{
	ARG	i;
	ARG	cnt;
	ARG	a,b;

	cnt = LOOPS;
	for(i=0;i<cnt;i++) {
		proc1();
		b = 5 * i;
		a = proc3("this is a test");
		a *= b;	
		proc2(a);
	}
}

void
proc1()
{
	globchar = 'A';
	globint  = 0;
}

void
proc2(ARG a)
{
	globint = a;
	globbool = ((globint & 1) == 1);
}
