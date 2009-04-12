/* $Id: help.c,v 1.1.1.1 2003/05/21 13:43:08 pasha Exp $ */

static char *SCCSID = "@(#)1.6  2/9/99 09:08:47 src/jfs/utils/xpeek/help.c, jfsutil, w45.fs32, 990417.1";
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
 *   MODULE_NAME:       help.c
 *
 *   FUNCTION:          explain how to use the xpeek utility
 *
 */

#include "xpeek.h"

void help()
{
        char    *cmd;
        int     cmd_len;

        cmd=strtok(0, "         ");     /* space & tab */
        if (strtok(0, "         ")) {
                fputs("help: called with too many arguments\n\n",stderr);
                return;
        }
        if (cmd == 0 ) {
                fputs("\t\tXpeek Commands\n\n",stdout);
                fputs("a[lter] <block> <offset> <hex string>\n",stdout);
                fputs("b[tree] <block> [<offset>]\n",stdout);
                fputs("cb[blfsck]\n",stdout);
                fputs("dir[ectory] <inode number> [<fileset>]\n",stdout);
                fputs("d[isplay] [<block> [<offset> [<format> [<count>]]]]\n",stdout);
                fputs("dm[ap]\n",stdout);
                fputs("dt[ree] <inode number> [<fileset>]\n",stdout);
                fputs("fsckw[sphdr]\n",stdout);
                fputs("h[elp] [<command>]\n",stdout);
                fputs("ia[g] [<IAG number>] [a | s | <fileset>]\n",stdout);
                fputs("i[node] [<inode number>] [a | s | <fileset>]\n",stdout);
                fputs("logs[uper]\n",stdout);
                fputs("q[uit]\n",stdout);
                fputs("se[t] [<variable> <value>]\n",stdout);
                fputs("su[perblock] [p | s]\n",stdout);
                fputs("s2p[erblock] [p | s]\n",stdout);
                fputs("u[nset] <variable>\n",stdout);
                fputs("xt[ree] <inode number> [<fileset>]\n\n",stdout);
                return;
        }
        cmd_len=strlen(cmd);
        if (strncmp(cmd, "alter", cmd_len) == 0) {
                fputs("a[lter] <block> <offset> <hex string>\n",stdout);
                fputs("\t<block>\t\tblock number (decimal)\n", stdout);
                fputs("\t<offset>\toffset within block (hex)\n", stdout);
                fputs("\t<hex string>\tstring of hex digits\n\n", stdout);
                fputs("alters disk data.  <hex string> should contain an even number of digits\n\n", stdout);
        }
        else if (strncmp(cmd, "btree", cmd_len) == 0) {
                fputs("b[tree] <block> [<offset>]\n",stdout);
                fputs("\t<block>\t\tblock number (decimal)\n",stdout);
                fputs("\t<offset>\toffset within block (hex)\n\n", stdout);
                fputs("displays one node of a btree and enters a subcommand mode in which to\n", stdout);
                fputs("navigate the btree.  Subcommands:\n", stdout);
                fputs("\tl\tvisit left sibling\n",stdout);
                fputs("\tm\tmodify current node\n",stdout);
                fputs("\tp\tvisit parent node\n",stdout);
                fputs("\tr\tvisit right sibling\n",stdout);
                fputs("\t[0-9]\tvisit the nth child node\n",stdout);
                fputs("\tx\texit subcommand mode\n\n", stdout);
        }
        else if (cmd_len > 1 && strncmp(cmd, "cbblfsck", cmd_len) == 0) {
                fputs("cb[blfsck]\n\n",stdout);
                fputs("Displays the area used by ClearBadBlockList \n",stdout);
                fputs("for communication with chkdsk.\n\n",stdout);
        }
        else if (cmd_len > 2 && strncmp(cmd, "directory", cmd_len) == 0) {
                fputs("dir[ectory] <inode number> [<fileset>]\n",stdout);
                fputs("\t<inode number>\tinode number of directory (decimal)\n", stdout);
                fputs("\t<fileset>\tfileset number, currently must be zero\n\n", stdout);
                fputs("Displays directory entries.  Subcommands\n", stdout);
                fputs("\tm\tmodify entries\n",stdout);
                fputs("\tx\texit subcommand mode\n\n", stdout);
        }
        else if (cmd_len > 1 && strncmp(cmd, "dtree", cmd_len) == 0) {
                fputs("dt[ree] <inode number> [<fileset>]\n",stdout);
                fputs("\t<inode number>\tinode number of directory (decimal)\n", stdout);
                fputs("\t<fileset>\tfileset number, currently must be zero\n\n", stdout);
                fputs("Displays root of the directory btree and enters a subcommand mode in which to\n", stdout);
                fputs("navigate the btree.  Subcommands:\n", stdout);
                fputs("\tl\tvisit left sibling\n",stdout);
                fputs("\tm\tmodify current node\n",stdout);
                fputs("\tp\tvisit parent node (not parent directory)\n",stdout);
                fputs("\tr\tvisit right sibling\n",stdout);
                fputs("\t[0-9]\tvisit the nth child node\n",stdout);
                fputs("\tx\texit subcommand mode\n\n", stdout);
        }
        else if (strncmp(cmd, "display", cmd_len) == 0) {
                fputs("d[isplay] [<block> [<offset> [<format> [<count>]]]]\n",stdout);
                fputs("\t<block>\t\tblock number (decimal)\n", stdout);
                fputs("\t<offset>\toffset within block (hex)\n", stdout);
                fputs("\t<format>\tformat in which to display data (see below)\n", stdout);
                fputs("\t<count>\t\tnumber of objects to display (decimal)\n", stdout);
                fputs("displays data in a variety of formats.  Format may be one of the following:\n", stdout);
                fputs("\ta\tascii\n", stdout);
                fputs("\tb\tblock allocation map\n", stdout);
                fputs("\td\tdecimal\n", stdout);
                fputs("\ti\tinode\t\t\t\tstruct dinode\n", stdout);
                fputs("\tI\tinode allocation map\t\tixad_t\n", stdout);
                fputs("\ts\tsuperblock\t\t\tstruct superblock\n", stdout);
                fputs("\tx\thexadecimal\n", stdout);
                fputs("\tX\textent allocation descriptor\txad_t\n\n", stdout);
        }
        else if (cmd_len > 1 && strncmp(cmd, "dmap", cmd_len) == 0) {
                fputs("dm[ap]\n",stdout);
                fputs("Displays Aggregate Disk (Block) Map\n\n",stdout);
        }
        else if (cmd_len > 4 && strncmp(cmd, "fsckwsphdr", cmd_len) == 0) {
                fputs("fsckw[sphdr]\n\n",stdout);
                fputs("Displays the header of the chkdsk workspace in the aggregate.\n\n",stdout);
        }
        else if (strncmp(cmd, "help", cmd_len) == 0) {
                fputs("h[elp] [<command>]\n",stdout);
                fputs("\t<command>\tcommand name\n\n", stdout);
                fputs("prints help text.  Lists all commands if no parameter\n\n", stdout);
        }
        else if (cmd_len > 1 && strncmp(cmd, "iag", cmd_len) == 0) {
                fputs("ia[g] [<IAG number>] [a | <fileset>]\n",stdout);
                fputs("\t<IAG number>\tIAG number (decimal)\n", stdout);
                fputs("\ta\t\tuse primary aggragate inode table\n", stdout);
                fputs("\ts\t\tuse secondary aggragate inode table\n", stdout);
                fputs("\t<fileset>\tfileset number (currently must be zero)\n\n", stdout);
                fputs("Display iag information and enters subcommand mode.  Subcommands:\n",stdout);
                fputs("\te\tdisplay/modify inode extents map\n", stdout);
                fputs("\tm\tmodify iag\n", stdout);
                fputs("\tp\tdisplay/modify persistent map\n", stdout);
                fputs("\tw\tdisplay/modify working map\n\n", stdout);
        }
        else if (strncmp(cmd, "inode", cmd_len) == 0) {
                fputs("i[node] [<inode number>] [a | <fileset>]\n",stdout);
                fputs("\t<inode number>\tinode number (decimal)\n", stdout);
                fputs("\ta\t\tuse primary aggragate inode table\n", stdout);
                fputs("\ts\t\tuse secondary aggragate inode table\n", stdout);
                fputs("\t<fileset>\tfileset number (currently must be zero)\n\n", stdout);
                fputs("Display inode information and enters subcommand mode.  Subcommands:\n",stdout);
                fputs("\ta\tdisplay/modify inode's ACL\n", stdout);
                fputs("\tm\tmodify inode\n", stdout);
        }
        else if (cmd_len > 3 && strncmp(cmd, "logsuper", cmd_len) == 0) {
                fputs("logs[uper]\n\n",stdout);
                fputs("Displays the journal log superblock.\n\n",stdout);
        }
        else if (strncmp(cmd, "quit", cmd_len) == 0) {
                fputs("q[uit]\n\n",stdout);
                fputs("exit xpeek\n\n",stdout);
        }
        else if (cmd_len > 1 && strncmp(cmd, "set", cmd_len) == 0) {
                fputs("se[t] [<variable> <value>]\n",stdout);
                fputs("\t<variable>\tvariable to define.  First character must be alpha\n",stdout);
                fputs("\t<value>\t\tvalue to assign to variable.\n\n",stdout);
                fputs("Sets a user-defined variable.  With no arguments, set lists all\n",stdout);
                fputs("defined variables.\n",stdout);
        }
        else if (cmd_len > 1 && strncmp(cmd, "superblock", cmd_len) == 0) {
                fputs("su[perblock] [p | s]\n",stdout);
                fputs("\tp\tDisplay primary superblock\n",stdout);
                fputs("\ts\tDisplay secondary superblock\n\n",stdout);
                fputs("Displays superblock data\n\n",stdout);
        }
        else if (cmd_len > 2 && strncmp(cmd, "s2perblock", cmd_len) == 0) {
                fputs("su[perblock] [p | s]\n",stdout);
                fputs("\tp\tDisplay primary superblock\n",stdout);
                fputs("\ts\tDisplay secondary superblock\n\n",stdout);
                fputs("Displays alternate superblock data\n\n",stdout);
        }
        else if (strncmp(cmd, "unset", cmd_len) == 0) {
                fputs("u[nset] <variable>\n\n",stdout);
                fputs("\t<variable>\tvariable to delete.\n\n",stdout);
                fputs("Deletes variables defined with set command\n\n",stdout);
        }
        else if (cmd_len > 1 && strncmp(cmd, "xtree", cmd_len) == 0) {
                fputs("xt[ree] <inode number> [<fileset>]\n",stdout);
                fputs("\t<inode number>\tinode number of non-directory (decimal)\n", stdout);
                fputs("\t<fileset>\tfileset number, currently must be zero\n\n", stdout);
                fputs("Displays root of the non-directory btree and enters a subcommand mode in which to\n", stdout);
                fputs("navigate the btree.  Subcommands:\n", stdout);
                fputs("\tl\tvisit left sibling\n",stdout);
                fputs("\tm\tmodify current node\n",stdout);
                fputs("\tp\tvisit parent node (not parent directory)\n",stdout);
                fputs("\tr\tvisit right sibling\n",stdout);
                fputs("\t[0-9]\tvisit the nth child node\n",stdout);
                fputs("\tx\texit subcommand mode\n\n", stdout);
        }
        else
                fprintf(stderr,"help: Unknown command '%s'\n\n",cmd);
}
