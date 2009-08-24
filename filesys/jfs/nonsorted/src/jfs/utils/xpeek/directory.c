/* $Id: directory.c,v 1.4 2004/03/21 02:43:33 pasha Exp $ */

static char *SCCSID = "@(#)1.13  6/25/99 09:56:55 src/jfs/utils/xpeek/directory.c, jfsutil, w45.fs32, fixbld";
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
 *   MODULE_NAME:               directory.c
 *
 *   COMPONENT_NAME:    jfsutil
 *
 *   FUNCTION:  Display/modify directory/dtree and file/xtree
 *
*/

#include "xpeek.h"

/* JFS includes */
#include <jfs_inode.h>
#include <jfs_dtree.h>
#include <jfs_filsys.h>

#ifdef  _JFS_UNICODE
#define UNICASERANGEDEFINED
#define UNIUPR_NOUPPER
#include <uni_inln.h>
#else   /* ! _JFS_UNICODE */
#define UniStrncpy strncpy
#endif

/* libfs includes */
#include <inode.h>

void print_direntry(dtslot_t *, uint8);
char display_leaf_slots(dtslot_t *, int8 *, int8, int32 *);
char display_internal_slots(dtslot_t *, int8 *, int8, int32 *);
char display_leaf_xads(xad_t *, int16, int32 *);
char display_internal_xads(xad_t *, int16, int32 *);
char display_internal_xtpage(xad_t );
char display_slot(dtslot_t *, int8, int32, int32 *);
char display_slot(dtslot_t *, int8, int32, int32 *);
char display_extent_page(int64);
void display_xtpage(xtpage_t *);

#ifdef  _JFS_UNICODE
void strToUcs(UniChar *, char *, int32);
#else
#define strToUcs strncpy
#endif

void directory()
{
        char            cmd_line[80];
        dtpage_t        dtree;
        int32           i;
        dinode_t        inode;
        int64           inode_address;
        ino_t           inum;
        int64           node_address;
        idtentry_t      *node_entry;
        dtroot_t        *root;
        dtslot_t        *slot;
        uint8           *stbl;
        char            *token;
        ino_t           which_table = FILESYSTEM_I;

        token = strtok(0, "     ");
        if (token == 0) {
                fputs("directory: Please enter: inum [fileset]\ndirectory> ",
                        stdout);
                gets(cmd_line);
                token=strtok(cmd_line, "        ");
                if (token == 0)
                        return;
        }
        errno = 0;
        inum = strtoul(token, 0, 0);
        if (inum == 0 && errno) {
                fputs("directory: invalid inum\n\n", stderr);
                return;
        }
        token = strtok(0, "     ");
        if (token) {
                if (token[0] != '0') {
                        fputs("directory: invalid fileset\n\n", stderr);
                        return;
                }
        }
        if (strtok(0, "         ")) {
                fputs("directory: Too many arguments\n\n", stderr);
                return;
        }

        if (find_inode(inum, which_table, &inode_address) ||
            xRead(inode_address, sizeof(dinode_t), (char *)&inode)) {
                fputs("directory: error reading inode\n\n", stderr);
                return;
        }
        if ((inode.di_mode & IFMT) != IFDIR) {
                fputs("directory: Not a directory!\n", stderr);
                return;
        }

        root = (dtroot_t *)&(inode.di_btroot);
        printf("idotdot = %d\n\n", root->header.idotdot);

        if (root->header.flag & BT_LEAF) {
                if (root->header.nextindex == 0) {
                        fputs("Empty directory.\n", stdout);
                        return;
                }

                for (i = 0; i < root->header.nextindex; i++) {
                        print_direntry(root->slot, root->header.stbl[i]);
                }
                return;
        }

        /* Root is not a leaf node, we must descend to the leftmost leaf */

        node_entry = (idtentry_t *)&(root->slot[root->header.stbl[0]]);
descend:
        node_address = addressPXD(&(node_entry->xd)) << l2bsize;
        if (xRead(node_address, sizeof(dtpage_t), (char *)&dtree)) {
                fputs("Directory:  Error reading dtree node\n", stderr);
                return;
        }
        stbl = (uint8 *)&(dtree.slot[dtree.header.stblindex]);
        if (!(dtree.header.flag & BT_LEAF)) {
                node_entry = (idtentry_t *)&(dtree.slot[stbl[0]]);
                goto descend;
        }

        /* dtree (contained in node) is the left-most leaf node */

next_leaf:
        for (i = 0; i < dtree.header.nextindex; i++) {
                print_direntry(dtree.slot, stbl[i]);
        }
        if (dtree.header.next) {
                if (xRead(dtree.header.next << l2bsize, sizeof(dtpage_t),
                          (char *)&dtree)) {
                        fputs("directory: Error reading leaf node\n", stderr);
                        return;
                }
                stbl = (uint8 *)&(dtree.slot[dtree.header.stblindex]);
                goto next_leaf;
        }
        return;
}

void print_direntry(
dtslot_t        *slot,
uint8           head_index)
{
        ldtentry_t      *entry;
        int32           len;
        UniChar         *n;
        UniChar         *name;
        int32           namlen;
        int32           next;
        dtslot_t        *s;

        entry = (ldtentry_t *)&(slot[head_index]);
        namlen = entry->namlen;
        name = (UniChar *)malloc(sizeof(UniChar) * (namlen + 1));
        if (name == 0) {
                fputs("dirname: malloc error!\n", stderr);
                return;
        }
        name[namlen] = 0;
        len = MIN(namlen, DTLHDRDATALEN);
        UniStrncpy(name, entry->name, len);
        next = entry -> next;
        n = name + len;

        while (next >= 0) {
                s = &(slot[next]);
                namlen -= len;
                len = MIN(namlen, DTSLOTDATALEN);
                UniStrncpy(n, s->name, len);
                next = s->next;
                n += len;
        }
#ifdef  _JFS_UNICODE
        printf("%d\t%ls\n", entry->inumber, name);
#else
        printf("%d\t%s\n", entry->inumber, name);
#endif
        free(name);
}

void dtree()
{
        int32           changed = 0;
        char            cmd_line[80];
        dtpage_t        *dtree;
        int32           field;
        char            flag_names[64];
        dinode_t        inode;
        int64           inode_address;
        ino_t           inum;
        char            result;
        dtroot_t        *root;
        char            *token;
        ino_t           which_table = FILESYSTEM_I;

        token = strtok(0, "     ");
        if (token == 0) {
                fputs("dtree: Please enter: inum [fileset]\ndtree> ",stdout);
                gets(cmd_line);
                token=strtok(cmd_line, "        ");
                if (token == 0)
                        return;
        }
        errno = 0;
        inum = strtoul(token, 0, 0);
        if (inum == 0 && errno) {
                fputs("dtree: invalid inum\n\n", stderr);
                return;
        }
        token = strtok(0, "     ");
        if (token) {
                if (token[0] != '0') {
                        fputs("dtree: invalide fileset\n\n", stderr);
                        return;
                }
        }
        if (strtok(0, "         ")) {
                fputs("dtree: Too many arguments\n\n", stderr);
                return;
        }

        if (find_inode(inum, which_table, &inode_address) ||
            xRead(inode_address, sizeof(dinode_t), (char *)&inode)) {
                fputs("dtree: error reading inode\n\n", stderr);
                return;
        }
        if ((inode.di_mode & IFMT) != IFDIR) {
                fputs("dtree: Not a directory!\n", stderr);
                return;
        }

        dtree = (dtpage_t *)&(inode.di_btroot);

redisplay:

        if (!(dtree->header.flag & BT_ROOT))
                        fputs("dtree: Should be at root of dtree, but BTROOT not set!\n", stderr);
        root = (dtroot_t *)dtree;

        *flag_names = 0;
        if (root->header.flag & BT_ROOT)
                strcat(flag_names, "BT_ROOT  ");
        if (root->header.flag & BT_LEAF)
                strcat(flag_names, "BT_LEAF  ");
        if (root->header.flag & BT_INTERNAL)
                strcat(flag_names, "BT_INTERNAL  ");
        if (root->header.flag & BT_RIGHTMOST)
                strcat(flag_names, "BT_RIGHTMOST  ");
        if (root->header.flag & BT_LEFTMOST)
                strcat(flag_names, "BT_LEFTMOST  ");

        printf("Root D-Tree Node of inode %d\n\n", inode.di_number);
        printf("[1] DASDlimit\t%lld\n", DASDLIMIT(&(root->header.DASD)));   /* @F1 */
        printf("[2] DASDused\t%lld\n", DASDUSED(&(root->header.DASD)));   /* @F1 */
        printf("[3] thresh (%%)\t%d\n", root->header.DASD.thresh );  /* @F1 */
        printf("[4] delta (%%)\t%d\n", root->header.DASD.delta );     /* @F1 */
        printf("\n");                                                                  /* @F1 */
        printf("[5] flag\t0x%02x\t\t%s\n", root->header.flag, flag_names); /* @F1 */
        printf("[6] nextindex\t%d\n", root->header.nextindex);       /* @F1 */
        printf("[7] freecnt\t%d\n", root->header.freecnt);              /* @F1 */
        printf("[8] freelist\t%d\n", root->header.freelist);               /* @F1 */
        printf("[9] idotdot\t%d\n", root->header.idotdot);               /* @F1 */
        printf("[10] stbl\t{%d,%d,%d,%d,%d,%d,%d,%d}\n",               /* @F1 */
                root->header.stbl[0], root->header.stbl[1],
                root->header.stbl[2], root->header.stbl[3],
                root->header.stbl[4], root->header.stbl[5],
                root->header.stbl[6], root->header.stbl[7]);

retry:
        if (root->header.nextindex) {
                fputs("dtree: Hit enter to see entries, [m]odify, or e[x]it: ",
                        stdout);
        }
        else {
                fputs("dtree: [m]odify, or e[x]it: ", stdout);
        }

        gets(cmd_line);
        token = strtok(cmd_line, "      ");
        if (token) {
                if (*token == 'x')
                        return;

        if (*token == 'm') {
                field = m_parse(cmd_line, 9, &token);
                if (field == 0)
                        goto retry;

                switch (field)
                {
                case 1:                                                                      /* @F1 */
                        setDASDLIMIT(&(root->header.DASD), strtoll(token, 0, 0) );  /* @F1 */
                        break;                                                               /* @F1 */
                case 2:                                                                      /* @F1 */
                        setDASDUSED(&(root->header.DASD), strtoll(token, 0, 0) );  /* @F1 */
                        break;                                                               /* @F1 */
                case 3:                                                                      /* @F1 */
                        root->header.DASD.thresh = strtoul(token, 0, 0);     /* @F1 */
                        break;                                                               /* @F1 */
                case 4:                                                                      /* @F1 */
                        root->header.DASD.delta = strtoul(token, 0, 0);       /* @F1 */
                        break;                                                               /* @F1 */
                case 5:                                                                      /* @F1 */
                        root->header.flag = strtoul(token, 0, 16);
                        break;
                case 6:                                                                      /* @F1 */
                        root->header.nextindex = strtoul(token, 0, 0);
                        break;
                case 7:                                                                      /* @F1 */
                        root->header.freecnt = strtoul(token, 0, 0);
                        break;
                case 8:                                                                      /* @F1 */
                        root->header.freelist = strtoul(token, 0, 0);
                        break;
                case 9:                                                                      /* @F1 */
                        root->header.idotdot = strtoul(token, 0, 0);
                        break;
                }
                if (xWrite(inode_address, sizeof(dinode_t), (char *)&inode)) {
                        fputs("dtree: error writing inode\n\n", stderr);
                        return;
                }
                goto redisplay;
        }
        }
        if (root->header.nextindex == 0)
                return;

        if (root->header.flag & BT_LEAF)
                result = display_leaf_slots(root->slot, root->header.stbl,
                                root->header.nextindex, &changed);
        else
                result = display_internal_slots(root->slot, root->header.stbl,
                                root->header.nextindex, &changed);

        if (changed) {
                if (xWrite(inode_address, sizeof(dinode_t), (char *)&inode)) {
                        fputs("dtree: error writing inode\n\n", stderr);
                        return;
                }
                changed = 0;
        }

        if (result == 'u')
                goto redisplay;

        return;
}

void xtree()
{
        int32           changed = 0;
        char            cmd_line[80];
        xtpage_t        *xtree;
        int32           field;
        char            flag_names[64];
        dinode_t        inode;
        int64           inode_address;
        ino_t           inum;
        char            result;
        char            *token;
        ino_t           which_table = FILESYSTEM_I;

        token = strtok(0, "     ");
        if (token == 0) {
                fputs("xtree: Please enter: inum [fileset]\ndtree> ",stdout);
                gets(cmd_line);
                token=strtok(cmd_line, "        ");
                if (token == 0)
                        return;
        }
        errno = 0;
        inum = strtoul(token, 0, 0);
        if (inum == 0 && errno) {
                fputs("xtree: invalid inum\n\n", stderr);
                return;
        }
        token = strtok(0, "     ");
        if (token) {
                if (token[0] == 'a')
                        which_table = AGGREGATE_I;
                else if (token[0] == 's')
                        which_table = AGGREGATE_2ND_I;
                else if (token[0] != '0') {
                        fputs("inode: invalide fileset\n\n", stderr);
                        return;
                }
        }
        if (strtok(0, "         ")) {
                fputs("xtree: Too many arguments\n\n", stderr);
                return;
        }

        if (find_inode(inum, which_table, &inode_address) ||
            xRead(inode_address, sizeof(dinode_t), (char *)&inode)) {
                fputs("xtree: error reading inode\n\n", stderr);
                return;
        }
        if ((inode.di_mode & IFMT) == IFDIR) {
                fputs("xtree: INODE IS A DIRECTORY!\n", stderr);
                return;
        }

        xtree = (xtpage_t *)&(inode.di_btroot);

redisplay:

        printf("Root X-Tree Node of inode %d\n\n", inode.di_number);
        display_xtpage(xtree);

retry:
        if (xtree->header.nextindex > 2) {
                fputs("xtree: Hit enter to see entries, [m]odify, or e[x]it: ",
                        stdout);
        }
        else {
                fputs("xtree: [m]odify, or e[x]it: ", stdout);
        }

        gets(cmd_line);
        token = strtok(cmd_line, "      ");
        if (token) {
                if (*token == 'x')
                        return;

        if (*token == 'm') {
                field = m_parse(cmd_line, 6, &token);
                if (field == 0)
                        goto retry;

                switch (field)
                {
                case 1:
                        xtree->header.flag = strtoul(token, 0, 16);
                        break;
                case 2:
                        xtree->header.nextindex = strtoul(token, 0, 0);
                        break;
                case 3:
                        xtree->header.maxentry = strtoul(token, 0, 0);
                        break;
                case 4:
                        xtree->header.self.len = strtoul(token, 0, 0);
                        break;
                case 5:
                        xtree->header.self.addr1 = strtoul(token, 0, 0);
                        break;
                case 6:
                        xtree->header.self.addr2 = strtoul(token, 0, 0);
                        break;
                }
                if (xWrite(inode_address, sizeof(dinode_t), (char *)&inode)) {
                        fputs("xtree: error writing inode\n\n", stderr);
                        return;
                }
                goto redisplay;
        }
        }
        if (xtree->header.nextindex <= 2)
                return;

        if (xtree->header.flag & BT_LEAF)
                result = display_leaf_xads(xtree->xad,
                                xtree->header.nextindex, &changed);
        else
                result = display_internal_xads(xtree->xad,
                                xtree->header.nextindex, &changed);

        if (changed) {
                if (xWrite(inode_address, sizeof(dinode_t), (char *)&inode)) {
                        fputs("xtree: error writing inode\n\n", stderr);
                        return;
                }
                changed = 0;
        }

        if (result == 'u')
                goto redisplay;

        return;
}

void display_xtpage(xtpage_t *xtree)
{

        char            flag_names[64];


        *flag_names = 0;
        if (xtree->header.flag & BT_ROOT)
                strcat(flag_names, "BT_ROOT  ");
        if (xtree->header.flag & BT_LEAF)
                strcat(flag_names, "BT_LEAF  ");
        if (xtree->header.flag & BT_INTERNAL)
                strcat(flag_names, "BT_INTERNAL  ");
        if (xtree->header.flag & BT_RIGHTMOST)
                strcat(flag_names, "BT_RIGHTMOST  ");
        if (xtree->header.flag & BT_LEFTMOST)
                strcat(flag_names, "BT_LEFTMOST  ");

        printf("[1] flag\t0x%02x\t%s\n", xtree->header.flag, flag_names);
        printf("[2] nextindex\t%d\t\t", xtree->header.nextindex);
        printf("[5] self.addr1\t0x%02x\n", xtree->header.self.addr1);
        printf("[3] maxentry\t%d\t\t", xtree->header.maxentry);
        printf("[6] self.addr2\t0x%08x\n", xtree->header.self.addr2);
        printf("[4] self.len\t0x%06x\t", xtree->header.self.len);
        printf("    self.addr\t%lld\n", addressPXD(&xtree->header.self));
}  /* end display_xtpage */

char display_leaf_slots(
dtslot_t        *slot,
int8            *stbl,
int8            nextindex,
int32           *changed)
{
        char            cmd_line[512];
        int32           i;
        int32           field;
        ldtentry_t      *leaf;
        char            result = 'u'; /* default returned if no leaf->next */
        int32           slot_number;
        char            *token;

        for (i = 0; i < nextindex; i++) {
                slot_number = stbl[i];
                leaf = (ldtentry_t *) &(slot[slot_number]);

redisplay2:
                printf("stbl[%d] = %d\n", i, slot_number);
                printf("[1] inumber\t%d\n", leaf->inumber);
                printf("[2] next\t%d\n", leaf->next);
                printf("[3] namlen\t%d\n", leaf->namlen);
#ifdef _JFS_UNICODE
                printf("[4] name\t%.13ls\n", leaf->name);
#else
                printf("[4] name\t%.26s\n", leaf->name);
#endif

retry2:
                fputs("dtree: Press enter for next, [m]odify, [u]p, or e[x]it > ",
                        stdout);
                gets(cmd_line);
                token = strtok(cmd_line, "      ");
                if (token) {
                        if (*token == 'u' || *token == 'x' )
                                return *token;
                        if (*token == 'm') {
                                field = m_parse(cmd_line, 4, &token);
                                if (field == 0)
                                        goto retry2;

                                switch (field)
                                {
                                case 1:
                                        leaf->inumber = strtoul(token,0,0);
                                        break;
                                case 2:
                                        leaf->next = strtoul(token,0,0);
                                        break;
                                case 3:
                                        leaf->namlen = strtoul(token,0,0);
                                        break;
                                case 4:
                                        strToUcs(leaf->name, token,
                                                 DTLHDRDATALEN);
                                        break;
                                }
                                *changed = 1;
                                goto redisplay2;
                        }
                }

                if (leaf->next >= 0) {
                        result = display_slot(slot, leaf->next, 1, changed);
                        if (result == 'u' || result == 'x')
                                return result;
                }
        }
        return result;
}

char display_slot(
dtslot_t        *slot,
int8            index,
int32           isleaf,
int32           *changed)
{
        char    result;

        printf("[1] next\t%d\n", slot[index].next);
        printf("[2] cnt\t\t%d\n", slot[index].cnt);
#ifdef  _JFS_UNICODE
        printf("[3] name\t%.15ls\n", slot[index].name);
#else
        printf("[3] name\t%.30s\n", slot[index].name);
#endif

        if (isleaf)
                result = prompt("dtree: press enter for next or [u]p or e[x]it > ");
        else
                result = prompt("dtree: press enter for next or [u]p, [d]own or e[x]it > ");

        if (result == 'u' || result == 'd' || result == 'x')
                return result;

        if (slot[index].next >= 0)
                return display_slot(slot, slot[index].next, isleaf, changed);
        else
                return result;
}

char display_internal_slots(
dtslot_t        *slot,
int8            *stbl,
int8            nextindex,
int32           *changed)
{
        int32           i;
        idtentry_t      *entry;
        int64           node_address;
        char            result;
        int32           slot_number;

        for (i = 0; i < nextindex; i++) {
                slot_number = stbl[i];
                entry = (idtentry_t *)&(slot[slot_number]);
                node_address = addressPXD(&(entry->xd));

                printf("stbl[%d] = %d\n", i, slot_number);
                printf("[1] xd.len\t    0x%06x\t\t", entry->xd.len);
                printf("[4] next\t%d\n", entry->next);
                printf("[2] xd.addr1\t  0x%02x\t\t\t", entry->xd.addr1);
                printf("[5] namlen\t%d\n", entry->namlen);
                printf("[3] xd.addr2\t  0x%08x\t\t", entry->xd.addr2);
                printf("     xd.addr\t%lld\n", node_address);
#ifdef  _JFS_UNICODE
                printf("[6] name\t%.11ls\n", entry->name);
#else
                printf("[6] name\t%.22s\n", entry->name);
#endif
                printf("addressPXD(xd)\t%lld\n", node_address);

                result = prompt("dtree: press enter for next or [u]p, [d]own or e[x]it > ");
                if (result == 'x' || result == 'u' )
                        return result;
                else if (result != 'd' && entry->next >= 0) {
                        result = display_slot(slot, entry->next, 0, changed);
                        if (result == 'x' || result == 'u' )
                                return result;
                }
                if (result == 'd')
                        /* descend to the child node */
                        return(display_extent_page(node_address));
        }
        return result;
}

char display_leaf_xads(
xad_t           *xad,
int16           nextindex,
int32           *changed)
{
        int32           i;
        idtentry_t      *entry;
        int64           node_address;
        char            result;
        int32           slot_number;

        for (i = 2; i < nextindex; i++) {

                printf("XAD # = %d\n", i);
                printf("[1] xad.flag\t  %x\t\t", xad[i].flag);
                printf("[4] xad.len\t  0x%06x\n", xad[i].len);
                printf("[2] xad.off1\t  0x%02x\t\t", xad[i].off1);
                printf("[5] xad.addr1\t  0x%02x\n", xad[i].addr1);
                printf("[3] xad.off2\t  0x%08x\t", xad[i].off2);
                printf("[6] xad.addr2\t  0x%08x\n", xad[i].addr2);
                printf("    xad.off  \t  %lld\t\t", offsetXAD(&(xad[i])));
                printf("    xad.addr\t  %lld\n", addressXAD(&(xad[i])));

                result = prompt("xtree: press enter for next or e[x]it > ");
                if (result == 'x' || result == 'u' )
                        return result;
        }
        return result;
}
char display_internal_xads(
xad_t           *xad,
int16           nextindex,
int32           *changed)
{
        int32           i;
        idtentry_t      *entry;
        int64           node_address;
        char            result;
        int32           slot_number;

        for (i = 2; i < nextindex; i++) {

                printf("XAD # = %d\n", i);
                printf("[1] xad.flag\t  %x\t\t", xad[i].flag);
                printf("[4] xad.len\t  0x%06x\n", xad[i].len);
                printf("[2] xad.off1\t  0x%02x\t\t", xad[i].off1);
                printf("[5] xad.addr1\t  0x%02x\n", xad[i].addr1);
                printf("[3] xad.off2\t  0x%08x\t", xad[i].off2);
                printf("[6] xad.addr2\t  0x%08x\n", xad[i].addr2);
                printf("    xad.off  \t  %lld\t\t", offsetXAD(&(xad[i])));
                printf("    xad.addr\t  %lld\n", addressXAD(&(xad[i])));

                result = prompt("xtree: press enter for next or [u]p, [d]own or e[x]it > ");
                if (result == 'x' || result == 'u' )
                        return result;
                else if (result == 'd') {
                        result = display_internal_xtpage(xad[i]);
                        return result;
                }
        }
        return result;
}

char display_internal_xtpage(xad_t xad)
{
     int32           changed = 0;
     char            cmd_line[80];
     xtpage_t        xtree_area;
     xtpage_t        *xtree=&xtree_area;
     int32           field;
     char            flag_names[64];
     int64           xtpage_address;
     ino_t           inum;
     char            result = 'u';
     char            *token;
     ino_t           which_table = FILESYSTEM_I;


     xtpage_address = addressXAD(&xad);
     xtpage_address = xtpage_address*bsize;

     if (xRead(xtpage_address, sizeof(xtpage_t), (char *)xtree)) {
             fputs("xtree: error reading xtpage\n\n", stderr);
     }
     else {

redisplay:

        display_xtpage(xtree);

retry:
        if (xtree->header.nextindex > 2) {
                fputs("xtree: Hit enter to see entries, [m]odify, or e[x]it: ",
                        stdout);
        }
        else {
                fputs("xtree: [m]odify, or e[x]it: ", stdout);
        }

        gets(cmd_line);
        token = strtok(cmd_line, "      ");
        if (token) {
                if (*token == 'x')
                        return result;

          if (*token == 'm') {
                field = m_parse(cmd_line, 6, &token);
                if (field == 0)
                        goto retry;

                switch (field)
                {
                case 1:
                        xtree->header.flag = strtoul(token, 0, 16);
                        break;
                case 2:
                        xtree->header.nextindex = strtoul(token, 0, 0);
                        break;
                case 3:
                        xtree->header.maxentry = strtoul(token, 0, 0);
                        break;
                case 4:
                        xtree->header.self.len = strtoul(token, 0, 0);
                        break;
                case 5:
                        xtree->header.self.addr1 = strtoul(token, 0, 0);
                        break;
                case 6:
                        xtree->header.self.addr2 = strtoul(token, 0, 0);
                        break;
                }
                if (xWrite(xtpage_address, sizeof(xtpage_t), (char *)xtree)) {
                        fputs("xtree: error writing xtpage\n\n", stderr);
                        return result;
                }
                goto redisplay;
          }
        }
        if (xtree->header.nextindex <= 2)
                return result;

        if (xtree->header.flag & BT_LEAF)
                result = display_leaf_xads(xtree->xad,
                                xtree->header.nextindex, &changed);
        else
                result = display_internal_xads(xtree->xad,
                                xtree->header.nextindex, &changed);

        if (changed) {
                if (xWrite(xtpage_address, sizeof(xtpage_t), (char *)xtree)) {
                        fputs("xtree: error writing xtpage\n\n", stderr);
                        return result;
                }
                changed = 0;
        }

        if (result == 'u')
                goto redisplay;
     } /* end else */

     return result;
}

char display_extent_page(
int64   address)
{
        int32           changed = 0;
        char            flag_names[64];
        dtpage_t        node;
        char            result;
        int8            *stbl;

        if (xRead(address << l2bsize, sizeof(dtpage_t), (char *)&node)) {
                fprintf(stderr, "display_extent_page: Error reading node\n");
                return 'u';
        }

redisplay5:
        *flag_names = 0;
        if (node.header.flag & BT_ROOT) {
                fputs("display_extent_page:  Warning!  extent dtree page has BT_ROOT flag set!\n",
                        stderr);
                strcat(flag_names, "BT_ROOT  ");
        }
        if (node.header.flag & BT_LEAF)
                strcat(flag_names, "BT_LEAF  ");
        if (node.header.flag & BT_INTERNAL)
                strcat(flag_names, "BT_INTERNAL  ");
        if (node.header.flag & BT_RIGHTMOST)
                strcat(flag_names, "BT_RIGHTMOST  ");
        if (node.header.flag & BT_LEFTMOST)
                strcat(flag_names, "BT_LEFTMOST  ");

        printf("Internal D-tree node at block %lld\n", address);

        printf("[1] flag\t0x%02x\t\t%s\n", node.header.flag, flag_names);
        printf("[2] nextindex\t%3d\n", node.header.nextindex);
        printf("[3] freecnt\t%3d\t\t", node.header.freecnt);
        printf("[7] rsrvd\tNOT DISPLAYED\n");
        printf("[4] freelist\t%3d\t\t", node.header.freelist);
        printf("[8] self.len\t0x%06x\n",node.header.self.len);
        printf("[5] maxslot\t%3d\t\t", node.header.maxslot);
        printf("[8] self.addr1\t0x%02x\n",node.header.self.addr1);
        printf("[6] stblindex\t%d\t\t", node.header.stblindex);
        printf("[9] \t0x%08x\n",node.header.self.addr2);

        if (node.header.nextindex) {
                result = prompt("dtree: Hit enter to see entries, [u]p or e[x]it: ");
                if (result == 'u' || result == 'x')
                        return(result);
        }
        else {
                fputs("display_extent_page: Strange ... empty d-tree node.\n",
                        stderr);
                return 'u';
        }

        stbl = (int8 *)&(node.slot[node.header.stblindex]);

        if (node.header.flag & BT_LEAF)
                result = display_leaf_slots(node.slot, stbl,
                                                node.header.nextindex,
                                                &changed);
        else
                result = display_internal_slots(node.slot, stbl,
                                                node.header.nextindex,
                                                &changed);

        if (changed) {
                if (xWrite(address << l2bsize, sizeof(dtpage_t), (char *)&node)) {
                        fputs("display_extent_page: error writing node\n\n", stderr);
                        return 0;
                }
                changed = 0;
        }
        if (result == 'u')
                goto redisplay5;

        return result;
}
#ifdef  _JFS_UNICODE
void strToUcs(
UniChar *target,
char    *source,
int32   len)
{
        while ((*(target++) = *(source++)) && --len);
        return;
}
#endif  /* _JFS_UNICODE */
