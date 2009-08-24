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

#define MAX_FILES 1000

static char buf[70000];

char *server = "localhost";

static int sock;

/* emulate a single SMB packet exchange */
static void do_packets(unsigned long send_size, unsigned long recv_size)
{
	uint32 *ubuf = (uint32 *)buf;
	static int counter;

	ubuf[0] = htonl(send_size-4);
	ubuf[1] = htonl(recv_size-4);

	if (write_sock(sock, buf, send_size) != send_size) {
		printf("error writing %d bytes\n", (int)send_size);
		exit(1);
	}

	if (read_sock(sock, buf, 4) != 4) {
		printf("error reading header\n");
		exit(1);
	}

	if (ntohl(ubuf[0]) != recv_size-4) {
		printf("lost sync (%d %d)\n", 
		       (int)recv_size-4, (int)ntohl(ubuf[0]));
		exit(1);
	}

	if (recv(sock, buf, recv_size-4, MSG_WAITALL|MSG_TRUNC) != 
	    recv_size-4) {
		printf("error reading %d bytes\n", (int)recv_size-4);
		exit(1);
	}

	if (ntohl(ubuf[0]) != recv_size-4) {
		printf("lost sync (%d %d)\n", 
		       (int)recv_size-4, (int)ntohl(ubuf[0]));
	}
}

void nb_setup(struct child_struct *child)
{
	extern char *tcp_options;

	sock = open_socket_out(server, TCP_PORT);
	if (sock == -1) {
		printf("client %d failed to start\n", child->id);
		exit(1);
	}
	set_socket_options(sock, tcp_options);

	do_packets(8, 8);
}


void nb_unlink(struct child_struct *child, char *fname)
{
	do_packets(83, 39);
}

void nb_rmdir(struct child_struct *child, char *fname)
{
	do_packets(83, 39);
}

void nb_createx(struct child_struct *child, char *fname, 
		unsigned create_options, unsigned create_disposition, int fnum)
{
	do_packets(111, 69);
}

void nb_writex(struct child_struct *child, int handle, int offset, 
	       int size, int ret_size)
{
	child->bytes_out += size;
	do_packets(51+size, 41);
}

void nb_readx(struct child_struct *child, int handle, int offset, 
	      int size, int ret_size)
{
	child->bytes_in += size;
	do_packets(55, size+4);
}

void nb_close(struct child_struct *child, int handle)
{
	do_packets(45, 39);
}

void nb_rename(struct child_struct *child, char *old, char *new)
{
	do_packets(41+strlen(old)+strlen(new), 39);
}

void nb_qpathinfo(struct child_struct *child, const char *fname)
{
	do_packets(41+strlen(fname), 80);
}

void nb_qfileinfo(struct child_struct *child, int handle)
{
	do_packets(41, 80);
}

void nb_qfsinfo(struct child_struct *child, int level)
{
	do_packets(41, 80);
}

void nb_findfirst(struct child_struct *child, char *fname)
{
	do_packets(41+strlen(fname), 200);
}

void nb_flush(struct child_struct *child, int handle)
{
	do_packets(41, 39);
}

void nb_cleanup(struct child_struct *child)
{
	/* noop */
}

void nb_deltree(struct child_struct *child, char *dname)
{
	/* noop */
}
