/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Ros�n (aka Viking)

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
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    Or see <http://www.gnu.org/licenses/>
 */

#ifndef _FIXUPLX_H
#define _FIXUPLX_H

#ifdef __cplusplus
  extern "C" {
#endif

void print_struct_r32_rlc_info(struct r32_rlc * rlc);

int get_srcoff_cnt1_size(struct r32_rlc * rlc);
int get_mod_ord1_size(struct r32_rlc * rlc);
int get_imp_ord1_size(struct r32_rlc * rlc);
int get_additive_size(struct r32_rlc * rlc);
int get_trgoff_size(struct r32_rlc * rlc);

int get_srcoff_cnt1_rlc(struct r32_rlc * rlc);

int get_mod_ord1_rlc(struct r32_rlc * rlc);

int get_imp_ord1_rlc(struct r32_rlc * rlc);

int get_additive_rlc(struct r32_rlc * rlc);

int get_srcoff12_rlc(struct r32_rlc * rlc, int idx);

unsigned int get_imp_name_rlc(struct r32_rlc * rlc);

int get_reloc_size_rlc(struct r32_rlc * rlc);

/*Testar ett sidfel på ett ställe i det virtuella utrymmet.
  Då ska den leta efter vilken sida den finns i, läsa in den från
  filen, göra fixups på den. */
/*void fake_pagefault_code(struct LX_module * lx_mod, unsigned int off, struct t_os2process * proc);*/


/* newexe.h */
#define NRSTYP          0x0f            /* Source type mask */
#define NRSBYT          0x00            /* lo byte (8-bits)*/
#define NRSSEG          0x02            /* 16-bit segment (16-bits) */
#define NRSPTR          0x03            /* 16:16 pointer (32-bits) */
#define NRSOFF          0x05            /* 16-bit offset (16-bits) */
#define NRPTR48         0x06            /* 16:32 pointer (48-bits) */
#define NROFF32         0x07            /* 32-bit offset (32-bits) */
#define NRSOFF32        0x08            /* 32-bit self-relative offset (32-bits) */

/*
 *  Format of NR_FLAGS(x) and R32_FLAGS(x):
 *
 *       7 6 5 4 3 2 1 0  - bit no
 *                 | | |
 *                 | +-+--- Reference type
 *                 +------- Additive fixup
 */

#define NRADD           0x04            /* Additive fixup */
#define NRRTYP          0x03            /* Reference type mask */
#define NRRINT          0x00            /* Internal reference */
#define NRRORD          0x01            /* Import by ordinal */
#define NRRNAM          0x02            /* Import by name */
#define NRROSF          0x03            /* Operating system fixup */
#define NRICHAIN        0x08            /* Internal Chaining Fixup */


#ifdef __cplusplus
  };
#endif


#endif /* _FIXUPLX_H */
