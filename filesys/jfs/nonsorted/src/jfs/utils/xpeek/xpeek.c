/* $Id: xpeek.c,v 1.4 2004/03/21 02:43:34 pasha Exp $ */

static char *SCCSID = "@(#)1.11  2/9/99 09:09:08 src/jfs/utils/xpeek/xpeek.c, jfsutil, w45.fs32, 990417.1";
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
 *   MODULE_NAME:       xpeek.c
 *
 *   FUNCTION:          Display and Alter structures in an unmounted filesystem
 *
*/

#include "xpeek.h"

/* libfs includes       */
#include <devices.h>
#include <inode.h>
#include <super.h>

/* Global Data */
int32           bsize;          /* aggregate block size         */
HFILE           fd;             /* Used by libfs routines       */
int16           l2bsize;        /* log2 of aggregate block size */
int64           AIT_2nd_offset; /* Used by find_iag routines    */
int64           fsckwsp_offset; /* Used by fsckcbbl routines    */
int64           jlog_super_offset; /* Used by fsckcbbl routines */

void usage(void);

void usage()
{
                fputs("Usage: xpeek: <drive>\n", stderr);
}

main(
int argc,
char **argv)
{
        int             cmd_len;
        char            *command;
        char            command_line[512];
        char            *device;
        int32           phys_block_size;
        struct superblock       sb;

        /* Check arguments */
        if (argc != 2) {
                usage();
                exit(1);
        }
        device=argv[1];

        /* Open device */
        if (ujfs_open_device(device, &fd, &phys_block_size, RDWR_EXCL) != 0)
        {
                fprintf(stderr, "open failure on drive %s - may be in use.\n", device);
                exit (1);
        }
        /* Verify device is a LVM volume */
        if (ujfs_verify_device_type(fd) != 0)
        {
                fprintf(stderr, "drive %s is not a LVM volume.\n", device);
                exit (1);
        }

        (void) ujfs_beginformat(fd);

        /* Get block size information from the superblock       */
        if (ujfs_get_superblk(fd, &sb, 1)) {
                fputs("xpeek: error reading primary superblock\n", stderr);
                if (ujfs_get_superblk(fd, &sb, 0)) {
                        fputs("xpeek: error reading secondary superblock\n", stderr);
                        goto errorout;
                }
                else
                        fputs("xpeek: using secondary superblock\n", stderr);
        }
        bsize = sb.s_bsize;
        l2bsize = sb.s_l2bsize;
        AIT_2nd_offset = addressPXD( &(sb.s_ait2) ) * bsize;
        fsckwsp_offset = addressPXD( &(sb.s_fsckpxd) ) << l2bsize;
        jlog_super_offset = (addressPXD( &(sb.s_logpxd) ) << l2bsize)
			+ LOGPSIZE;

        printf("\nBlock Size: %d\n\n", bsize);
        printf("xpeek: use the h[elp] command to get help\n\n");


        /* Main Loop */

        fputs("> ", stdout);
        fflush(stdout);
        while (gets(command_line)) {
                command=strtok(command_line,"   "); /* space or tab */
                if (command && *command) {
                        cmd_len=strlen(command);
                        if (strncmp(command, "alter", cmd_len) == 0)
                                alter();
                        else if (strncmp(command, "btree", cmd_len) == 0) {
                                fputs("btree command is not yet implemented.\n", stderr);
                        }
                        else if (cmd_len > 1 &&
                                 strncmp(command, "cbblfsck", cmd_len) == 0)
                                cbblfsck();
                        else if (cmd_len > 2 &&
                                 strncmp(command, "directory", cmd_len) == 0)
                                directory();
                        else if (cmd_len > 1 &&
                                 strncmp(command, "dmap", cmd_len) == 0)
                                dmap();
                        else if (cmd_len > 1 &&
                                 strncmp(command, "dtree", cmd_len) == 0)
                                dtree();
                        else if (cmd_len > 1 &&
                                 strncmp(command, "xtree", cmd_len) == 0)
                                xtree();
                        else if (strncmp(command, "display", cmd_len) == 0)
                                display();
                        else if (cmd_len > 4 &&
                                 strncmp(command, "fsckwsphdr", cmd_len) == 0)
                                fsckwsphdr();
                        else if (strncmp(command, "help", cmd_len) == 0)
                                help();
                        else if (cmd_len > 1 &&
                                 strncmp(command, "iag", cmd_len) == 0)
                                iag();
                        else if (strncmp(command, "inode", cmd_len) == 0)
                                inode();
                        else if (cmd_len > 3 &&
                                 strncmp(command, "logsuper", cmd_len) == 0)
                                logsuper();
                        else if (strncmp(command, "quit", cmd_len) == 0) {
                                (void) ujfs_redeterminemedia(fd);
                                ujfs_close(fd);
                                exit(0);
                        }
                        else if ((cmd_len > 1) &&
                                strncmp(command, "set", cmd_len) == 0) {
                                fputs("set command is not yet implemented.\n", stderr);
                        }
                        else if ((cmd_len > 1) &&
                                strncmp(command, "superblock", cmd_len) == 0)
                                superblock();
                        else if (cmd_len > 2 &&
                                 strncmp(command, "s2perblock", cmd_len) == 0)
                                superblock2();
                        else if (strncmp(command, "unset", cmd_len) == 0) {
                                fputs("unset command is not yet implemented.\n", stderr);
                        }
                        else
                                fprintf(stderr, "Invalid command: %s\n", command);
                }
                fputs("> ", stdout);
                fflush(stdout);
        }
errorout:
        (void) ujfs_redeterminemedia(fd);
        ujfs_close(fd);
        return 0;
}
