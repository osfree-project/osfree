/*
 *
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
 * Module: os2stats.c
 */
struct statJCache {
        int32   reclaim;
        int32   read;
        int32   recycle;
        int32   lazywriteawrite;
        int32   recycleawrite;
        int32   logsyncawrite;
        int32   write;
};
static struct statNCache {
    uint32  lookups;    /* 4: dnlc lookup */
    uint32  hits;       /* 4: dnlc hit */
    uint32  misses;     /* 4: dnlc miss */
    uint32  enters;     /* 4: dnlc enter */
    uint32  deletes;    /* 4: dnlc delete */
    uint32  nam2long;   /* 4: name length > DNLCNAMEMAX */
};
struct statLCache {
	uint32	commit;		/* # of commit */
	uint32	pageinit;	/* # of pages written */
	uint32	pagedone;	/* # of page write */
	uint32	sync;		/* # of logsysnc() */
	uint32	maxbufcnt;	/* max # of buffers allocated */
};
struct statICache {
    uint32  ninode;
    uint32  reclaim;
    uint32  recycle;
    uint32  release;
};
