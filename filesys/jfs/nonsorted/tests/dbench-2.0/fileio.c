/*
   dbench version 1.2
   Copyright (C) 1999 by Andrew Tridgell <tridge@samba.org>
   Copyright (C) 2001 by Martin Pool <mbp@samba.org>

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

#define MAX_FILES 200

static char buf[70000];

char *server = NULL;
extern int sync_open, sync_dirs;
#ifdef OS2
extern int flat_namespace;
#endif

static struct {
	int fd;
	int handle;
} ftable[MAX_FILES];

#ifdef OS2
static int pwrite(int handle, const void *buffer, unsigned int count, unsigned pos)
{
    int rc = lseek(handle, pos, SEEK_SET);
    if (rc != -1)
        rc = write(handle, buffer, count);
    return rc;
}

static int pread(int handle, void *buffer, unsigned int count, unsigned pos)
{
    int rc = lseek(handle, pos, SEEK_SET);
    if (rc != -1)
        rc = read(handle, buffer, count);
    return rc;
}
#endif

static int find_handle(struct child_struct *child, int handle)
{
	int i;
	for (i=0;i<MAX_FILES;i++) {
		if (ftable[i].handle == handle) return i;
	}
	printf("(%d) ERROR: handle %d was not found\n",
	       child->line, handle);
	exit(1);
}


/* Find the directory holding a file, and flush it to disk.  We do
   this in -S mode after a directory-modifying mode, to simulate the
   way knfsd tries to flush directories.  MKDIR and similar operations
   are meant to be synchronous on NFSv2. */
void sync_parent(char *fname)
{
	char *copy_name;
	int dir_fd;
	char *slash;

	if (strchr(fname, '/')) {
		copy_name = strdup(fname);
		slash = strrchr(copy_name, '/');
		*slash = '\0';
	} else {
		copy_name = strdup(".");
	}
	
	dir_fd = open(copy_name, O_RDONLY);
	if (dir_fd == -1) {
		printf("open directory \"%s\" for sync failed: %s\n",
		       copy_name,
		       strerror(errno));
	} else {
                #ifndef OS2
		if (fdatasync(dir_fd) == -1) {
			printf("datasync directory \"%s\" failed: %s\n",
			       copy_name,
			       strerror(errno));
		}
                #endif
		if (close(dir_fd) == -1) {
			printf("close directory failed: %s\n",
			       strerror(errno));
		}
	}
	free(copy_name);
}


void nb_setup(struct child_struct *child)
{
}

void nb_unlink(struct child_struct *child, char *fname)
{
	if (unlink(fname) != 0) {
		printf("(%d) unlink %s failed (%s)\n",
		       child->line, fname, strerror(errno));
	}
	if (sync_dirs) sync_parent(fname);
}

void nb_rmdir(struct child_struct *child, char *fname)
{
	if (rmdir(fname) != 0) {
		printf("(%d) rmdir %s failed (%s)\n",
		       child->line, fname, strerror(errno));
	}
	if (sync_dirs) sync_parent(fname);
}

void nb_createx(struct child_struct *child, char *fname,
		unsigned create_options, unsigned create_disposition, int fnum)
{
	int fd, i;
	int flags = O_RDWR;

	if (sync_open) flags |= O_SYNC;

	if (create_disposition == FILE_CREATE) {
		flags |= O_CREAT;
	}

	if (create_disposition == FILE_OVERWRITE ||
	    create_disposition == FILE_OVERWRITE_IF) {
		flags |= O_CREAT | O_TRUNC;
	}

#ifndef OS2
	if (create_options & FILE_DIRECTORY_FILE) {
		/* not strictly correct, but close enough */
		mkdir(fname, 0700);
	}

	if (create_options & FILE_DIRECTORY_FILE) flags = O_RDONLY|O_DIRECTORY;
	
	fd = open(fname, flags, 0600);
	if (fd == -1) {
		if (fnum != -1) {
			printf("(%d) open %s failed for handle %d (%s)\n",
			       child->line, fname, fnum, strerror(errno));
		}
		return;
	}
#else
	if (create_options & FILE_DIRECTORY_FILE) {
		/* not strictly correct, but close enough */
		mkdir(fname, 0700);
                fd = NULL;
	}
        else
        {
            if (create_options & FILE_DIRECTORY_FILE) flags = O_RDONLY|O_DIRECTORY;
            fd = open(fname, flags, 0600);
            if (fd == -1) {
                    if (fnum != -1) {
                            printf("(%d) open %s failed for handle %d (%s)\n",
                                   child->line, fname, fnum, strerror(errno));
                    }
                    return;
            }
        }
#endif

	if (fnum == -1) {
		printf("(%d) open %s succeeded for handle %d\n",
		       child->line, fname, fnum);
		close(fd);
		return;
	}

	for (i=0;i<MAX_FILES;i++) {
		if (ftable[i].handle == 0) break;
	}
	if (i == MAX_FILES) {
		printf("file table full for %s\n", fname);
		exit(1);
	}
	ftable[i].handle = fnum;
	ftable[i].fd = fd;
}

void nb_writex(struct child_struct *child, int handle, int offset,
	       int size, int ret_size)
{
	int i = find_handle(child, handle);

	if (buf[0] == 0) memset(buf, 1, sizeof(buf));

	if (pwrite(ftable[i].fd, buf, size, offset) != ret_size) {
		printf("write failed on handle %d\n", handle);
		exit(1);
	}

	child->bytes_out += size;
}

void nb_readx(struct child_struct *child, int handle, int offset,
	      int size, int ret_size)
{
	int i = find_handle(child, handle);

	if (pread(ftable[i].fd, buf, size, offset) != ret_size) {
		printf("read failed on handle %d\n", handle);
	}

	child->bytes_in += size;
}

void nb_close(struct child_struct *child, int handle)
{
	int i = find_handle(child, handle);
	close(ftable[i].fd);
	ftable[i].handle = 0;
}

void nb_rename(struct child_struct *child, char *old, char *new)
{
	if (rename(old, new) != 0) {
		printf("rename %s %s failed (%s)\n",
		       old, new, strerror(errno));
		exit(1);
	}
	if (sync_dirs) sync_parent(new);
}

void nb_flush(struct child_struct *child, int handle)
{
	find_handle(child, handle);
	/* noop */
}

void nb_qpathinfo(struct child_struct *child, const char *fname)
{
	struct stat st;
	stat(fname, &st);
}

void nb_qfileinfo(struct child_struct *child, int handle)
{
	struct stat st;
	int i = find_handle(child, handle);
	fstat(ftable[i].fd, &st);
}

void nb_qfsinfo(struct child_struct *child, int level)
{
	/* hmm, should do this one */
}

void nb_findfirst(struct child_struct *child, char *fname)
{
	DIR *dir;
	struct dirent *d;
	char *p;
	p = strrchr(fname, '/');
	if (!p) return;
	*p = 0;
	dir = opendir(fname);
	if (!dir) return;
	while ((d = readdir(dir))) ;
	closedir(dir);
}

void nb_cleanup(struct child_struct *child)
{
	rmdir("clients");
}

void nb_deltree(struct child_struct *child, char *dname)
{
	char path[100];
        #ifndef OS2
	sprintf(path, "/bin/rm -rf %s", dname);
        #else
	sprintf(path, "rm -rf %s", dname);
        #endif
	system(path);
}
