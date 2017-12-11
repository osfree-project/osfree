/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

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

#ifndef _MEMMGR_H_
#define _MEMMGR_H_

#ifdef __cplusplus
  extern "C" {
#endif

struct t_mem_area {
	void * start;
	unsigned int size;
	void * next;
};



void init_memmgr(struct t_mem_area *root_area);

void * seek_free_mem(struct t_mem_area *root_area, unsigned int i_size);
int   is_mem_used(struct t_mem_area *root_area, void * p_start, unsigned int i_size);
int   alloc_mem_area(struct t_mem_area *root_area, void * p_start, unsigned int i_size);
int   dealloc_mem_area(struct t_mem_area *root_area, void * p_start);
void  print_used_mem(struct t_mem_area *root_area);
	
/*
seek_free_mem - Söker efter fritt utrymme för ett block med storleken i_size.
is_mem_used   - Tar reda på om ett minnesområde redan är upptaget, returnerar i
                 så fall SANT annars FALSKT.
alloc_mem_area- Allokerar ett område, returnerar sant om det gick annars
                 falskt.
dealloc_mem_area - Avallokerar minnesområdet pekaren pekar på.
*/

#ifdef __cplusplus
  }
#endif

#endif
