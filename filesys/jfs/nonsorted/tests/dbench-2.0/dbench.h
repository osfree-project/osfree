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

#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/ipc.h>
#ifndef OS2
#include <sys/shm.h>
#include <sys/mman.h>
#endif
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#ifndef MSG_WAITALL
#define MSG_WAITALL 0x100
#endif

#define PRINT_FREQ 1

#define MIN(x,y) ((x)<(y)?(x):(y))

#define TCP_PORT 7003
#define TCP_OPTIONS "TCP_NODELAY SO_REUSEADDR"

#define BOOL int
#define True 1
#define False 0
#define uint32 unsigned

struct child_struct {
	int id;
	int nprocs;
	int status;
	int line;
	int done;
	double bytes_in, bytes_out;
};


/* CreateDisposition field. */
#define FILE_SUPERSEDE 0
#define FILE_OPEN 1
#define FILE_CREATE 2
#define FILE_OPEN_IF 3
#define FILE_OVERWRITE 4
#define FILE_OVERWRITE_IF 5

/* CreateOptions field. */
#define FILE_DIRECTORY_FILE       0x0001
#define FILE_WRITE_THROUGH        0x0002
#define FILE_SEQUENTIAL_ONLY      0x0004
#define FILE_NON_DIRECTORY_FILE   0x0040
#define FILE_NO_EA_KNOWLEDGE      0x0200
#define FILE_EIGHT_DOT_THREE_ONLY 0x0400
#define FILE_RANDOM_ACCESS        0x0800
#define FILE_DELETE_ON_CLOSE      0x1000

#ifndef O_DIRECTORY
#define O_DIRECTORY    0200000
#endif

#include "proto.h"

