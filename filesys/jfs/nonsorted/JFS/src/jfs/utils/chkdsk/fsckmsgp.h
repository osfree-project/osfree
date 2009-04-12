/* $Id: fsckmsgp.h,v 1.1.1.1 2003/05/21 13:40:04 pasha Exp $ */

/* static char *SCCSID = "@(#)1.23.1.2  12/2/99 11:13:53 src/jfs/utils/chkdsk/fsckmsgp.h, jfschk, w45.fs32, fixbld";*/
/*
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
 *   MODULE_NAME:		fsckmsgp.h
 *
 *   COMPONENT_NAME: 	jfschk
 *
 */
#ifndef H_FSCKMSGP
#define H_FSCKMSGP

#include "xfsck.h"

/***********************************************************************
 *
 * The message protocol array entries are used to control fsck message
 * traffic and permission (to perform a repair) protocol in compliance
 * with the parameters with which fsck was invoked.
 *
 *  See xfsck.h for declarations of the symbolic constants used
 *              in the following table and to access the following
 *              table.
 */
int16 MsgProtocol[fsck_highest_msgid_defined+1][4]={

/*  msg   output      messaging    local lang          hpfs msg         */
/*  id    stream        level       msg file            number          */
/* ----  ---------  -------------  ------------    ----------------     */
/*  0*/   fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*  1*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*  2*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*  3*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*  4*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*  5*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*  6*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*  7*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*  8*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*  9*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 10*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 11*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 12*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 13*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 14*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 15*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 16*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 17*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 18*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 19*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 20*/  ,fsck_err  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 21*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 22*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 23*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 24*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 25*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 26*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 27*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 28*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 29*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 30*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 31*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 32*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 33*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 34*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 35*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 36*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 37*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg  
/* 38*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 39*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 40*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

/* 41*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 42*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 43*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 44*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 45*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 46*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 47*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 48*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 49*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 50*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 51*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 52*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 53*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 54*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 55*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 56*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 57*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 58*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 59*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 60*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 61*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 62*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 63*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 64*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 65*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 66*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 67*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 68*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 69*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 70*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 71*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 72*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 73*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 74*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 75*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 76*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 77*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 78*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 79*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 80*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

/* 81*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 82*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 83*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 84*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 85*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 86*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 87*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 88*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 89*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 90*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 91*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 92*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 93*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 94*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 95*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 96*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 97*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/* 98*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/* 99*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*100*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*101*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*102*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*103*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*104*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*105*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*106*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*107*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*108*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*109*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*110*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*111*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*112*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*113*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*114*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*115*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*116*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*117*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*118*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*119*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*120*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

/*121*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*122*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*123*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*124*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*125*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*126*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*127*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*128*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*129*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*130*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*131*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*132*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*133*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*134*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*135*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*136*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*137*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*138*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*139*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*140*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*141*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*142*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*143*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*144*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*145*/  ,fsck_out  ,fsck_autochk  ,jfs_msgfile   ,not_an_hpfs_msg
/*146*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*147*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*148*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*149*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*150*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*151*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*152*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*153*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*154*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*155*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*156*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*157*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*158*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*159*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*160*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

/*161*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*162*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*163*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*164*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*165*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*166*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*167*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*168*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*169*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*170*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*171*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*172*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*173*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*174*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*175*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*176*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*177*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*178*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*179*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*180*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*181*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*182*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*183*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*184*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*185*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*186*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*187*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*188*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*189*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*190*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*191*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*192*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*193*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*194*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*195*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*196*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*197*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*198*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*199*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*200*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg

/*201*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*202*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*203*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*204*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*205*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*206*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*207*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*208*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*209*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*210*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*211*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*212*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*213*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*214*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*215*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*216*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*217*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*218*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*219*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*220*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*221*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*222*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*223*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*224*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*225*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*226*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*227*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*228*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*229*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*230*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*231*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*232*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*233*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*234*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*235*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*236*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*237*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*238*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*239*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*240*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

/*241*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*242*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*243*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*244*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*245*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*246*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*247*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*248*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*249*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*250*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*251*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*252*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*253*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*254*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*255*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*256*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*257*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*258*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*259*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*260*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*261*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*262*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*263*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*264*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*265*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*266*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*267*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*268*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*269*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*270*/  ,fsck_out  ,fsck_hrtbt    ,no_msgfile     ,not_an_hpfs_msg
/*271*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*272*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*273*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*274*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*275*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*276*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*277*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*278*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*279*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*280*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*281*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*282*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*283*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*284*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*285*/ ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*286*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*287*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*288*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*289*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*290*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*291*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*292*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*293*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*294*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*295*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*296*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*297*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*298*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*299*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

/*300*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*301*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*302*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*303*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*304*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*305*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*306*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*307*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*308*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*309*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*310*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*311*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*312*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*313*/  ,fsck_out  ,fsck_quiet    ,hpfs_msgfile   ,568
/*314*/  ,fsck_out  ,fsck_quiet    ,hpfs_msgfile   ,569
/*315*/  ,fsck_out  ,fsck_quiet    ,hpfs_msgfile   ,570
/*316*/  ,fsck_out  ,fsck_quiet    ,hpfs_msgfile   ,633
/*317*/  ,fsck_out  ,fsck_quiet    ,hpfs_msgfile   ,632
/*318*/  ,fsck_out  ,fsck_quiet    ,hpfs_msgfile   ,571
/*319*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*320*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*321*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*322*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*323*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*324*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*325*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*326*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*327*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*328*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*329*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*330*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*331*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*332*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*333*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*334*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*335*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*336*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*337*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*338*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*339*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*340*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

/*341*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*342*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile    ,not_an_hpfs_msg
/*343*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*344*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*345*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile    ,not_an_hpfs_msg
/*346*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile    ,not_an_hpfs_msg
/*347*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile    ,not_an_hpfs_msg
/*348*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*349*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*350*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*351*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*352*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile    ,not_an_hpfs_msg
/*353*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*354*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*355*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*356*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile    ,not_an_hpfs_msg
/*357*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*358*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*359*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*360*/  ,fsck_out  ,fsck_quiet    ,jfs_msgfile    ,not_an_hpfs_msg
/*361*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile     ,not_an_hpfs_msg
/*362*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*363*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*364*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*365*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile      ,not_an_hpfs_msg
/*366*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*367*/  ,fsck_out  ,fsck_quiet     ,jfs_msgfile     ,not_an_hpfs_msg
/*368*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*369*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*370*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*371*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*372*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*373*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*374*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*375*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*376*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*377*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*378*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*379*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*380*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*381*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*382*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*383*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*384*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*385*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*386*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*387*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*388*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*389*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*390*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*391*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*392*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*393*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*394*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*395*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*396*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*397*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*398*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg
/*399*/  ,fsck_out  ,fsck_txtins   ,jfs_msgfile    ,not_an_hpfs_msg

/*400*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*401*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*402*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*403*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*404*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*405*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*406*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*407*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*408*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*409*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*410*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*411*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*412*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*413*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*414*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*415*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*416*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*417*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*418*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*419*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*420*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*421*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*422*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*423*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*424*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*425*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*426*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*427*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*428*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*429*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*430*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*431*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*432*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*433*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*434*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*435*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*436*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*437*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*438*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*439*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*440*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*441*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*442*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*443*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*444*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*445*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*446*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*447*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*448*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*449*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*450*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*451*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*452*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*453*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*454*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*455*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*456*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*457*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*458*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*459*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*460*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*461*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*462*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*463*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*464*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*465*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*466*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*467*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*468*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*469*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*470*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*471*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*472*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*473*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*474*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*475*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*476*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*477*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*478*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*479*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*480*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*481*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*482*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*483*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*484*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*485*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*486*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*487*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*488*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*489*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*490*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*491*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*492*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*493*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*494*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*495*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*496*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*497*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*498*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*499*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

/*500*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*501*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*502*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*503*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*504*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*505*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*506*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*507*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*508*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*509*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*510*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*511*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*512*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*513*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*514*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*515*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*516*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*517*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*518*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*519*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*520*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*521*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*522*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*523*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*524*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*525*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*526*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*527*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*528*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*529*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*530*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*531*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*532*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*533*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*534*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*535*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*536*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*537*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*538*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*539*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*540*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*541*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*542*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*543*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*544*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*545*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*546*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*547*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*548*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*549*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*550*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*551*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*552*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*553*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*554*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*555*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*556*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*557*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*558*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*559*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*560*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*561*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*562*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*563*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*564*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*565*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*566*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*567*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*568*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*569*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*570*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*571*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*572*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*573*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*574*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*575*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*576*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*577*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*578*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*579*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*580*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*581*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*582*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*583*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*584*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*585*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*586*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg         /* @F1 */
/*587*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg         /* @F1 */
/*588*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg         /* @D1 */
/*589*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*590*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*591*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*592*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*593*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*594*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*595*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*596*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*597*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*598*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg
/*599*/  ,fsck_out  ,fsck_debug    ,no_msgfile     ,not_an_hpfs_msg

    };

#endif














