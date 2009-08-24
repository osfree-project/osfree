/* $Id: xpeek.h,v 1.4 2004/03/21 02:43:33 pasha Exp $ */

/* static char *SCCSID = "@(#)1.9  2/9/99 09:09:12 src/jfs/utils/xpeek/xpeek.h, jfsutil, w45.fs32, 990417.1";*/
/*
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   MODULE_NAME:       xpeek.h
 *
 *   FUNCTION:          common data & function prototypes
 *
 */
#define INCL_DOSERRORS
#include <os2.h>
#include <jfs_types.h>
#include <jfs_aixisms.h>

/* system includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* JFS includes */
#include <jfs_dinode.h>
#include <jfs_inode.h>
#include <jfs_imap.h>
#include <jfs_superblock.h>
#include <jfs_xtree.h>

/* Defines */

#define AGGREGATE_2ND_I -1

#define XPEEK_OK        0x00
#define XPEEK_CHANGED   0x01
#define XPEEK_REDISPLAY 0x10
#define XPEEK_ERROR     -1

/* Global Data */
extern int32            bsize;
extern HFILE            fd;
extern int16            l2bsize;

/* xpeek functions */

void alter(void);
void cbblfsck(void);
void directory(void);
void display(void);
void display_iag(iag_t *);
void display_inode(struct dinode *);
int32 display_super(struct superblock *);
void dmap(void);
void dtree(void);
void help(void);
int32 find_iag(ino_t iagnum, ino_t which_table, int64 *address);
int32 find_inode(ino_t inum, ino_t which_table, int64 *address);
void fsckwsphdr(void);
void iag(void);
void inode(void);
void logsuper(void);
int32 m_parse(char *, int32, char **);
int32 more(void);
char prompt(char *);
void superblock(void);
void superblock2(void);
void xtree(void);

int32 xRead(int64, uint32, char*);
int32 xWrite(int64, uint32, char*);

#define fputs(string,fd) { fputs(string,fd); fflush(fd); }
