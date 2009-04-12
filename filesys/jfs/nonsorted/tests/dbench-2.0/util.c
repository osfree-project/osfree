/*
   dbench version 1
   Copyright (C) Andrew Tridgell 1999

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "dbench.h"
#ifdef OS2
#undef FILE_OPEN
#undef FILE_CREATE
#define INCL_DOSMEMMGR
#define INCL_DOSSEMAPHORES
#include <os2.h>
#endif


static struct timeval tp1,tp2;

void start_timer(void)
{
	gettimeofday(&tp1,NULL);
}

double end_timer(void)
{
	gettimeofday(&tp2,NULL);
	return((tp2.tv_sec - tp1.tv_sec) +
	       (tp2.tv_usec - tp1.tv_usec)*1.0e-6);
}


/* return a pointer to a anonymous shared memory segment of size "size"
   which will persist across fork() but will disappear when all processes
   exit

   The memory is not zeroed

   This function uses system5 shared memory. It takes advantage of a property
   that the memory is not destroyed if it is attached when the id is removed
   */
void *shm_setup(int size)
{
#ifndef OS2
	int shmid;
	void *ret;

	shmid = shmget(IPC_PRIVATE, size, SHM_R | SHM_W);
	if (shmid == -1) {
		printf("can't get private shared memory of %d bytes: %s\n",
		       size,
		       strerror(errno));
		exit(1);
	}
	ret = (void *)shmat(shmid, 0, 0);
	if (!ret || ret == (void *)-1) {
		printf("can't attach to shared memory\n");
		return NULL;
	}
	/* the following releases the ipc, but note that this process
	   and all its children will still have access to the memory, its
	   just that the shmid is no longer valid for other shm calls. This
	   means we don't leave behind lots of shm segments after we exit

	   See Stevens "advanced programming in unix env" for details
	   */
	shmctl(shmid, IPC_RMID, 0);
	
	return ret;
#else
    void  * pv;
    int     rc;
    rc = DosAllocSharedMem(&pv, "\\SHAREMEM\\DBENCH",
                           (size + 0xfff) & 0xfffff000,
                           PAG_WRITE | PAG_COMMIT);
    if (rc)
    {
        fprintf(stderr, "DosAllocSharedMem failed with rc=%d cb=%d\n", rc, (size + 0xfff) & 0xfffff000);
        return (void*)-1;
    }
return pv;
#endif
}

#ifdef OS2
void shm_attach_os2(void)
{
    void  * pv;
    int     rc;
    rc = DosGetNamedSharedMem(&pv, "\\SHAREMEM\\DBENCH", PAG_WRITE | PAG_READ);
    if (rc)
        fprintf(stderr, "DosGetNamedSharedMem failed with rc=%d\n", rc);
}

HEV hev;

void sem_create_os2(void)
{
    int     rc;
    rc = DosCreateEventSem("\\SEM32\\DBENCH", &hev, DC_SEM_SHARED, FALSE);
    if (rc)
        fprintf(stderr, "DosCreateEventSem failed with rc=%d\n", rc);
}

void sem_wait_os2(void)
{
    HEV hevChild = NULLHANDLE;
    int rc;
    rc = DosOpenEventSem("\\SEM32\\DBENCH", &hevChild);
    if (rc) fprintf(stderr, "DosOpenEventSem failed with rc=%d\n", rc);
    rc = DosWaitEventSem(hevChild, -1);
    if (rc) fprintf(stderr, "DosWaitEventSem failed with rc=%d\n", rc);
}

void sem_signal_os2(void)
{
    int rc;
    rc = DosPostEventSem(hev);
    if (rc) fprintf(stderr, "DosPostEventSem failed with rc=%d\n", rc);
}
#endif


void strupper(char *s)
{
	while (*s) {
		*s = toupper(*s);
		s++;
	}
}


/****************************************************************************
similar to string_sub() but allows for any character to be substituted.
Use with caution!
****************************************************************************/
void all_string_sub(char *s,const char *pattern,const char *insert)
{
	char *p;
	size_t ls,lp,li;

	if (!insert || !pattern || !s) return;

	ls = strlen(s);
	lp = strlen(pattern);
	li = strlen(insert);

	if (!*pattern) return;
	
	while (lp <= ls && (p = strstr(s,pattern))) {
		memmove(p+li,p+lp,ls + 1 - (((int)(p-s)) + lp));
		memcpy(p, insert, li);
		s = p + li;
		ls += (li-lp);
	}
}


/****************************************************************************
  Get the next token from a string, return False if none found
  handles double-quotes.
Based on a routine by GJC@VILLAGE.COM.
Extensively modified by Andrew.Tridgell@anu.edu.au
****************************************************************************/
BOOL next_token(char **ptr,char *buff,char *sep)
{
	static char *last_ptr=NULL;
	char *s;
	BOOL quoted;
	
	if (!ptr) ptr = &last_ptr;
	if (!ptr) return(False);
	
	s = *ptr;
	
	/* default to simple separators */
	if (!sep) sep = " \t\n\r";
	
	/* find the first non sep char */
	while(*s && strchr(sep,*s)) s++;
	
	/* nothing left? */
	if (! *s) return(False);
	
	/* copy over the token */
	for (quoted = False; *s && (quoted || !strchr(sep,*s)); s++) {
		if (*s == '\"')
			quoted = !quoted;
		else
			*buff++ = *s;
	}
	
	*ptr = (*s) ? s+1 : s;
	*buff = 0;
	last_ptr = *ptr;
	
	return(True);
}
