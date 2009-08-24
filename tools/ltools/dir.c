/* LTOOLS
   Programs to read, write, delete and change Linux extended 2 filesystems under DOS

   Module dir.c
   This file contains routines for handling directories.

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include "ext2.h"
#include "proto.h"
#include "ldir.h"

extern int isDebug;
extern int DebugOut(int level, const char *fmt,...);

extern super_block sb;
extern group_desc *gt;
extern unsigned short BLOCK_SIZE;
extern enum
{   LDIR, LREAD, LWRITE, LTEST
}
modus;
extern enum
{   CHANGE, DEL, COPY, REN
}
u_modus;
/*
   CHANGE       =change file access rights, GID or UID
   DEL          =delete file
   REN		=rename file
   COPY     =copy file from DOS to Linux
 */
enum
{   SDIR, SFILE
}
source_type = SDIR;
#ifdef UNIX
#define STDOUT stdout
#define STDERR stdout
#else
extern FILE *STDOUT, *STDERR;
#endif
extern void toLba(unsigned long g, unsigned long h, unsigned long *lba, unsigned short *offset);

#ifdef DEBUG_ALLOC
unsigned int allocations = 0;					//for debugging memory allocation
#endif

void *MALLOC(size_t size)
{   void *p = NULL;
    if (size)
    {   p = malloc(size);
#ifdef DEBUG_ALLOC
	printf("malloc: #=%2u  size=%8u  p=%10lp\n", ++allocations, (unsigned int) size, p);
#endif
    }
    return (p);
}
void FREE(void *block)
{   if (block != NULL)
    {   free(block);
#ifdef DEBUG_ALLOC
	printf("free:   #=%2u                 p=%10lp\n", allocations--, block);
#endif
    }
}

unsigned long eatpath(char *path, char *inode_name, char ctrl)
{   inode *i;
    unsigned long inode_num, prev_inode_num, sym_inode_num = 0;
    char *path_seg, *block_ptr, *sym_end;
    static char tail[MAX_PATH_LENGTH], sym_path[MAX_PATH_LENGTH];

    if (path[0] == '/')
	strcpy(sym_path, path);
    else
    {   *sym_path = '/';
	strcpy(sym_path + 1, path);
    }
    strcpy(path, sym_path);

  start:

    DebugOut(8,"----------Executing eatpath------------------------ %s\n", path);

    path_seg = strtok(path, "/");				// break out first path element

    // Start with root directory
    inode_num = (unsigned long) 2;
    prev_inode_num = (unsigned long) 2;

    while (1)
    {   i = load_inode(inode_num);				//read in directory

	/* If this node is a symlink, we have to restart eatpath() with the
	   absolute path of the linked file. However, if we are deleting or renaming symlinks, we
	   really want the symlink, not the symlinks target, so we skip.
	    */
	if (S_ISLNK(i->i_mode) && !((modus==LWRITE)&&((u_modus==DEL)||(u_modus==REN)||(u_modus==CHANGE))&&!path_seg))
        {

	    DebugOut(8, "Resolving symbolic link %s from path %s",
		    (char *) &i->i_block[0], sym_path);

	    if (i->i_size >= 64)
            {   fprintf(STDERR, "Symbolic link '%s' too long, cannot be processed.\n", path_seg);
		exit(-1);
	    }
	    block_ptr = (char *) &i->i_block[0];

	    if (!path_seg)
	    {							// symlink is at the end of the path
		sym_end = sym_path + strlen(sym_path);
		if (*(sym_end - 1) == '/')			// preserve trailing slash
		    --sym_end;
		else
		    sym_inode_num = prev_inode_num;
	    } else
		sym_end = sym_path + (path_seg - path) - 1;

	    strcpy(tail, sym_end);
	    *sym_end = 0;

	    if (*block_ptr == '/')				// absolute path
		strncpy(sym_path, block_ptr, MAX_PATH_LENGTH);

	    else
	    {							// relative path
		*(strrchr(sym_path, '/') + 1) = 0;
		strncat(sym_path, block_ptr, MAX_PATH_LENGTH - strlen(sym_path));
	    }

	    strncat(sym_path, tail, MAX_PATH_LENGTH);
	    strcpy(path, sym_path);

	    DebugOut(8,"\n\tblock: %s\n\ttail: %s\n\tpath: %s\n\tpath_seg: %s\n",
		   block_ptr, tail, path, (path_seg ? path_seg : "(null)"));

	    if (strlen(path) >= MAX_PATH_LENGTH)
		perror("Symbolic link resolution makes path too long.  Truncated.");

	    goto start;
	}
	/*to catch symlinks, we have to read one inode beyond path_seg==NULL,
	   therefore the break statement is in the middle of the while-loop */
	if (path_seg == NULL)
        {
	    source_type = (S_ISDIR(i->i_mode) && sym_path[strlen(sym_path) - 1] == '/') ? SDIR : SFILE;
	    if ((modus == LDIR) || ((modus == LWRITE) && ((u_modus == DEL)||(u_modus==REN))))
            {   if (source_type == SFILE)
                {   if (sym_inode_num)
			inode_num = sym_inode_num;
		    else
			inode_num = prev_inode_num;
		}
		break;
	    } else if ((modus == LREAD) || ((modus == LWRITE) && (u_modus != DEL) && (u_modus!=REN)))
            {
            	if ((S_ISDIR(i->i_mode)) && (modus != LWRITE))
                {
		    if (modus==LREAD)
		    	;	//exit(-1); //return inode_num;
		    else
		    {   fprintf(STDERR, "%s is a directory, cannot be read or deleted\n", sym_path);
		    	exit(-1);
		    }
		}
		if ((S_ISBLK(i->i_mode) || S_ISCHR(i->i_mode)) && (modus != LWRITE))
                {   fprintf(STDERR, "%s is a block or character device, cannot be read or deleted\n", sym_path);
		    exit(-1);
		}
		break;
	    }
	}
	prev_inode_num = inode_num;
	inode_num = search_dir(i, path_seg, inode_name, ctrl);


	DebugOut(8,"Inode for %s = %ld\n", path_seg, inode_num);

	if (inode_num == 0)
        {   if ((ctrl == INIT) && (!((modus == LWRITE) && (u_modus == COPY))))
		fprintf(STDERR, "Can't find '%s'.\n", path_seg);
	    return (inode_num);
	}
	// See if there's more string to parse
	path_seg = strtok(NULL, "/");				// Get next section of path
    }

    strcpy(path, sym_path);
    return (inode_num);
}

//compares strings, returns 0, if strings are equal, accepts wildcards
int comp_name(char *filename, char *name)
{   int i, j;

    for (i = 0, j = 0; i <= strlen(filename); i++, j++)		//compare including \0
    {   if (filename[i] == '?')
        {   continue;						//wildcard '?' -> skip this character
	} else if (filename[i] == '*')				//wildcard '*' -> rest of string does not matter
        {   for (; filename[i] != '.'; i++)
            {   if (filename[i] == '\0')
		    return (0);					//strings are equal
	    }
	    for (; name[j] != '.'; j++)
            {   if (name[j] == '\0')
		    return (1);					//point in filename, none in name, strings are equal
	    }
	} else if (filename[i] != name[j])
        {   return (1);						//character not equal -> string not equal
	}
    }
    return (0);							//strings are equal
}

// List a directory (names and inode numbers) with inode struct as input
void list_dir(inode * i, char *path)
{   char *name, *buf;
    char filename[MAX_PATH_LENGTH];
    inode *in;
    unsigned long c_time, inode_num;
    unsigned int rec_len, name_len, count = 0;
    unsigned long size;

    DebugOut(8,"----------Executing list_dir------------------------\n");

    if (source_type == SFILE)					//if it's a file to list, break out filename without path
    {   strcpy(filename, path);
	name = strtok(filename, "/");
	while (name)
        {   buf = name;
	    name = strtok(NULL, "/");
	}
	strcpy(filename, buf);
    }
    size = (unsigned long) ((i->i_blocks) * DISK_BLOCK_SIZE);

    if ((buf = (char *) MALLOC(size)) == NULL)
    {   fprintf(STDERR, "Memory problem in list_dir --%s--%s--%s--\n", filename, buf, path);
	exit(-1);
    }
    if (read_inode(i, buf, size, RELEASE_BUF) == -1)
    {   fprintf(STDERR, "Error with read_inode in dir.c\n");
	exit(-1);
    }
    while ((count + 8) < size)					// Don't run over end
    {
#ifndef UNIX
	inode_num = *((unsigned long *) &buf[count]);
	rec_len = *((unsigned short *) &buf[count + 4]);
	//name_len = *((unsigned short *) &buf[count + 6]);
	name_len = *((unsigned char *) &buf[count + 6]);
#else
	inode_num = le32_to_cpu(*((unsigned long *) &buf[count]));
	rec_len = le16_to_cpu(*((unsigned short *) &buf[count + 4]));
	name_len = *((unsigned char *) &buf[count + 6]);
#endif

	DebugOut(8,"\nDir Entry: size=%lu count=%i inode_num=%lu rec_len=%i name_len=%i\n", size, count, inode_num, rec_len, name_len);

	if ((inode_num < 1) || (inode_num > sb.s_inodes_count))
	    break;
	if (inode_num == 0)					// This means a deleted entry
        {   count += rec_len;					// keep reading...
	    continue;
	}
	if ((name = (char *) MALLOC(name_len + 1)) == NULL)
        {   fprintf(STDERR, "Directory too big: build_dir ran out of memory.\n");
	    exit(-1);
	}
	strncpy(name, &buf[count + 8], name_len);
	name[name_len] = '\0';					// terminate name string

	in = load_inode(inode_num);
	if (in == NULL)
        {   fprintf(STDERR, "Error with load_inode in dir.c\n");
	    exit(-1);						// We hit a problem
	}
	c_time = in->i_mtime + TIMEZONE;

	if (source_type == SFILE)
        {   if (!comp_name(filename, name))
            {   fprintf(STDOUT, "%10s %4lu %4lu %9lu %s %s",
			build_mode(in), in->i_uid+(((unsigned long) in->i_uid_high)<<16), in->i_gid+(((unsigned long) in->i_gid_high)<<16), in->i_size,
			strtok(ctime((time_t *) & c_time), "\n"), name);
		if (S_ISLNK(in->i_mode))			//Print fast symbolic links (name < 64 char)
                {   if (in->i_size < 64)
                    {   fprintf(STDOUT, " -> %s", (char *) &in->i_block[0]);
		    } else
                    {   fprintf(STDOUT, "-> Symlink name too long");
		    }
		}
		fprintf(STDOUT, "\n");
	    }
	} else
        {   fprintf(STDOUT, "%10s %4lu %4lu %9lu %s %s",
		    build_mode(in), in->i_uid+(((unsigned long) in->i_uid_high)<<16), in->i_gid+(((unsigned long) in->i_gid_high)<<16), in->i_size,
		    strtok(ctime((time_t *) & c_time), "\n"), name);
	    if (S_ISLNK(in->i_mode))				//Print fast symbolic links (name < 64 char)
            {   if (in->i_size < 64)
                {   fprintf(STDOUT, " -> %s", (char *) &in->i_block[0]);
		} else
                {   fprintf(STDOUT, "-> Symlink name too long");
		}
	    }
	    fprintf(STDOUT, "\n");
	}
	FREE(name);
	count += rec_len;					// increment counter
	if ((rec_len == 0)) /*|| ((rec_len > 255) && (rec_len > name_len + 16)))*/
	    break;						//End of List, but is this correct ???
    }
    FREE(buf);
}

/* Read inode's data blocks as a directory structure, search for entry 'str',
   return inode_name and inode_number if found, 0 if not
   ctrl = INIT		first search, will initialize buffer and save data for recursive search
   	  KEEP_BUF      recursive search (first call must be with INIT)
   	  KILL_BUF	release buffers, no search

 */
unsigned long search_dir(inode * i, char *str, char *inode_name, char ctrl)
{   unsigned int rec_len, name_len;
    unsigned long inode_num = 0;
    static char *name = NULL, *buf = NULL, *str_search = NULL;
    static long buf_size;
    static unsigned int count = 0;

    DebugOut(8,"----------Executing search_dir--------------%s-----%i----\n", str, ctrl);

    if (ctrl == INIT)
    {   count = 0;
	FREE(buf);
	FREE(str_search);
	if ((buf = (char *) MALLOC((i->i_blocks) * DISK_BLOCK_SIZE)) == NULL)
        {   fprintf(STDERR, "Memory problem in search_dir.\n");
	    exit(-1);
	}
	if (read_inode(i, buf, (i->i_blocks) * DISK_BLOCK_SIZE, RELEASE_BUF) == -1)
        {   fprintf(STDERR, "Problem in search_dir\n");
	    exit(-1);
	}
	buf_size = i->i_blocks * DISK_BLOCK_SIZE;
	str_search = strdup(str);
    } else if (ctrl == KILL_BUF)
    {   FREE(buf);
	FREE(str_search);
	return (0);
    }
    while (count + 8 < buf_size)				// Don't run over end
    {
#ifndef UNIX
	inode_num = *((unsigned long *) &buf[count]);
	rec_len = *((unsigned short *) &buf[count + 4]);
	//name_len = *((unsigned short *) &buf[count + 6]);
	name_len = *((unsigned char *) &buf[count + 6]);
#else
	inode_num = le32_to_cpu(*((unsigned long *) &buf[count]));
	rec_len = le16_to_cpu(*((unsigned short *) &buf[count + 4]));
	name_len = *((unsigned char *) &buf[count + 6]);	// dir_entry changed
#endif

	if ((inode_num < 1) || (inode_num > sb.s_inodes_count))
        {   inode_num = 0;
	    break;
	}
	if (inode_num == 0)					// This means a deleted entry
        {   count += rec_len;					// keep reading...
	    continue;
	}
	if ((name = (char *) MALLOC(name_len + 1)) == NULL)
        {   fprintf(STDERR, "Directory too big: search_dir_new ran out of memory.\n");
	    exit(-1);
	}
	strncpy(name, &buf[count + 8], name_len);
	name[name_len] = '\0';					// terminate name string
	count += rec_len;					// increment counter

	DebugOut(8,"str=%s  name=%s\n",str_search,name);

	if (!comp_name(str_search, name))			// Do these match?
        {   strcpy(inode_name, name);
	    FREE(name);						// if yes don't search the rest
	    break;
	}
	FREE(name);
	if ((rec_len == 0)) /* || ((rec_len > 255) && (rec_len > name_len + 16)))*/
        {   count = 0;
	    inode_num = 0;
	    break;
	}
	inode_num = 0;
    }
    return (inode_num);
}



/* Read inode's data blocks as a directory structure, search for entry 'str',
   erase the entry, return inode_number if found, 0 if not
   mode=0  erase      entry, newDirEntry received values of deleted entry
   mode=1  insert new entry  newDirEntry
   mode=2  test
 */
unsigned long modify_dir(inode *i, char *str, struct dir *newDirEntry, int mode, int * i_mode)
{   unsigned int rec_len, name_len, rec_len_old=0, rec_len_new, name_len_new, rec_len_mod;
    unsigned long inode_num = 0;
    unsigned char file_type;
    static char *name = NULL, *buf = NULL, *str_search = NULL;
    static unsigned long buf_size;
    static unsigned int count = 0;
    inode in;
    unsigned long lba;
    unsigned short offset;

    DebugOut(8,"----------Executing modify_dir--------------%s---------\n", str);

    name_len_new = strlen(newDirEntry->name);
    rec_len_new = ((8 + name_len_new) / 4 + 1) * 4;

    count = 0;
    if ((buf = (char *) MALLOC((i->i_blocks) * DISK_BLOCK_SIZE)) == NULL)
    {   fprintf(STDERR, "Memory problem in modify_dir.\n");
	exit(-1);
    }
    if (read_inode(i, buf, (i->i_blocks) * DISK_BLOCK_SIZE, RELEASE_BUF) == -1)
    {   fprintf(STDERR, "Problem in modify_dir\n");
	exit(-1);
    }
    buf_size = i->i_blocks * DISK_BLOCK_SIZE;
    str_search = strdup(str);

    while (count + 8 < buf_size)				// Don't run over end
    {
#ifndef UNIX
	inode_num = *((unsigned long *) &buf[count]);
	rec_len = *((unsigned short *) &buf[count + 4]);
	name_len = *((unsigned char *) &buf[count + 6]);
	file_type= *((unsigned char *) &buf[count + 7]);
#else
	inode_num = le32_to_cpu(*((unsigned long *) &buf[count]));
	rec_len = le16_to_cpu(*((unsigned short *) &buf[count + 4]));
	name_len = *((unsigned char *) &buf[count + 6]);
	file_type= *((unsigned char *) &buf[count + 7]);
#endif

	if ((inode_num < 1) || (inode_num > sb.s_inodes_count))
        {   inode_num = 0;
	    break;
	}
	if (inode_num == 0)					// This means a deleted entry
        {   fprintf(STDERR, "Empty directory entry found\n");
	    count += rec_len;
	    continue;
	}
	if ((name = (char *) MALLOC(name_len + 1)) == NULL)
        {   fprintf(STDERR, "Directory too big: modify_dir_new ran out of memory.\n");
	    exit(-1);
	}
	strncpy(name, &buf[count + 8], name_len);
	name[name_len] = '\0';					// terminate name string
	if (((mode==0)||(mode==2)) && (!comp_name(str_search, name)))	// Do these match?
        {

	    DebugOut(8,"modify_dir found: %s  inode #%ld\n", name, inode_num);

	    //load the associated inode to get the directory entry's file mode
/*	    loc = gt[(inode_num - 1) / sb.s_inodes_per_group].bg_inode_table * BLOCK_SIZE +
		((inode_num - 1) % sb.s_inodes_per_group) * sizeof(inode);
	    lba = loc / DISK_BLOCK_SIZE;
	    offset = loc % DISK_BLOCK_SIZE;
*/
	    toLba(gt[(inode_num - 1) / sb.s_inodes_per_group].bg_inode_table, (inode_num - 1) % sb.s_inodes_per_group, &lba, &offset);
	    if (readdisk((byte *) & in, lba, offset, sizeof(inode)) != sizeof(inode))
            {   fprintf(STDERR, "Disk problem in modify_inode.\n");
		exit(-1);
	    }
#ifdef UNIX
	    inode_fix_byteorder(&in);
#endif

	    *i_mode = in.i_mode;

	    //directories can not be deleted
/*	    if ((mode!=2) && S_ISDIR(in.i_mode))
            {   fprintf(STDERR, "Cannot delete directory '%s'\n", name);
		return 0;
	    }
*/
	    /*clear entry in the directory table
	       add length of deleted entry to previous entry */
#ifndef UNIX
	    *((unsigned short *) &buf[count - rec_len_old + 4]) = rec_len + rec_len_old;
#else
	    *((unsigned short *) &buf[count - rec_len_old + 4]) = cpu_to_le16(rec_len + rec_len_old);
#endif
	    //delete directory entry
	    memset(&buf[count], 0, sizeof(long) + 2 * sizeof(short) + name_len);
	    //write directory entry to disk
	    if (mode==0)
	    	if (write_inode(i, buf, (i->i_blocks) * DISK_BLOCK_SIZE) == -1)
            	{   fprintf(STDERR, "Problem in modify_dir\n");
		    exit(-1);
	    	}
	    FREE(name);						// if yes don't search the rest
	    break;
	}
//                printf("found: name=%s  name_len=%d  rec_len=%d\n",name,name_len,rec_len);

	rec_len_mod = ((8 + name_len) / 4 + 1) * 4;

	if ((mode == 1) && (rec_len > (rec_len_mod + rec_len_new)))
	{							//free directory entry found
//                printf("last:  name=%s  name_len=%d  rec_len=%d\n",name,name_len,rec_len);
	    rec_len_old = rec_len;				//adapt record length of last entry
#ifndef UNIX
	    *(short *) &buf[count + 4] = rec_len_mod;

	    count += rec_len_mod;				//insert new last entry
	    *(long *) &buf[count] = (long) newDirEntry->inode_num;
	    *(short *) &buf[count + 4] = (short) (rec_len_old - rec_len_mod);
	    *(char *) &buf[count + 6] = (char) name_len_new;
	    if ((sb.s_rev_level!=EXT2_GOOD_OLD_REV) && (sb.s_feature_incompat & EXT2_FEATURE_INCOMPAT_FILETYPE))
	    	*(char *) &buf[count + 7] = (char) newDirEntry->file_type; // set file type
	    else
	    	*(char *) &buf[count + 7] = (char) 0;
#else
	    *(short *) &buf[count + 4] = cpu_to_le16(rec_len_mod);

	    count += rec_len_mod;				//insert new last entry
	    *(long *) &buf[count] = cpu_to_le32(newDirEntry->inode_num);
	    *(short *) &buf[count + 4] = cpu_to_le16(rec_len_old - rec_len_mod);
	    *(char *) &buf[count + 6] = (char) name_len_new;		// also sets type to zero..
	    if ((sb.s_rev_level!=EXT2_GOOD_OLD_REV) && (sb.s_feature_incompat & EXT2_FEATURE_INCOMPAT_FILETYPE))
	    	*(char *) &buf[count + 7] = (char) newDirEntry->file_type; // set file type
	    else
	    	*(char *) &buf[count + 7] = (char) 0;
#endif
	    strncpy(&buf[count + 8], newDirEntry->name, name_len_new);

	    if (write_inode(i, buf, (i->i_blocks) * DISK_BLOCK_SIZE) == -1)
            {   fprintf(STDERR, "Problem in modify_dir\n");
		exit(-1);
	    }
	    FREE(name);
	    count = 0;
	    inode_num = 0;
	    break;
	}
	count += rec_len;					// increment counter
	FREE(name);
	rec_len_old = rec_len;
	inode_num = 0;
    }

   if (mode==0)
    {	newDirEntry->inode_num=inode_num;
    	newDirEntry->rec_len=rec_len;
    	newDirEntry->name_len=name_len;
    	newDirEntry->file_type=file_type;
	strncpy(newDirEntry->name,str,255);
    }
    FREE(buf);
    FREE(str_search);
    return (inode_num);
}



char *build_mode(inode * i)
{   static char modestr[11];
    char fullstr[11];
    int x;

    DebugOut(8,"----------Executing build_mode------------------------ %o\n",i->i_mode);

    strcpy(modestr, "?---------");
    strcpy(fullstr, "-rwxrwxrwx");

    for (x = 0; x < 9; x++)
	if ((i->i_mode >> x) & 1)
	    modestr[9 - x] = fullstr[9 - x];

    // here order counts right now
    if (S_ISLNK(i->i_mode))
	modestr[0] = 'l';
    else if (S_ISDIR(i->i_mode))
	modestr[0] = 'd';
    else if (S_ISFIL(i->i_mode))
	modestr[0] = '-';
    else if (S_ISCHR(i->i_mode))
	modestr[0] = 'c';
    else if (S_ISBLK(i->i_mode))
	modestr[0] = 'b';
    else if (S_ISSOCK(i->i_mode))
	modestr[0] = 's';
    else if (S_ISFIFO(i->i_mode))
	modestr[0] = 'p';
    else
	fprintf(STDOUT, "Unknown Inode Mode %X\n", i->i_mode);

    return modestr;

}

