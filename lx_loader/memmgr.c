/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "memmgr.h"
#include <stdio.h>

/* struct t_mem_area root_area; */

void init_memmgr(struct t_mem_area *root_area) {
	root_area->start = 0;
	root_area->size = 0;
	root_area->next = 0;
}

const int page_sz = 4096;

unsigned long int round_up_to_4KiB(unsigned long int i) {
	unsigned long int align_i = i % page_sz;
	/* printf("Aligned i at %d bytes boundry: %lu, rest: %lu \n", page_sz, i, align_i); */
	if(align_i)
		i += page_sz - align_i;
	/*printf(" i: %lu \n", i); */
	return i;
}

/* seek_free_mem - Söker efter fritt utrymme för ett block med storleken i_size. 
     Startar sökningen efter ett fritt utrymme vid start_pos och letar vid varje sida om den är fri
	 eller inte. Om den stöter på ett allokerar utrymme, öka på start_pos det utrymmets storlek 
	 till start_pos. */
void * seek_free_mem(struct t_mem_area *root_area, unsigned int i_size) {

	unsigned long int start_pos = 0;
	struct t_mem_area *el = root_area;
	unsigned int i_start = (unsigned int) start_pos;
	unsigned int el_start = (unsigned int) el->start;
	
	i_size = round_up_to_4KiB(i_size);
	
	while(el->next) {
		el = (struct t_mem_area*)el->next;
		i_start = start_pos;
		el_start = (unsigned int) el->start;
		
		if(((i_start >= el_start) && (i_start <= (el->size+el_start)))
			|| (((i_size+i_start) >= el_start) && ((i_size+i_start) <= (el->size+el_start))) ) {
			start_pos += el->size+el_start; /* Området används. */	
			
		}
		if((i_start <= el_start) && ((i_size+i_start) >= (el->size+el_start)))
			start_pos += el->size+el_start;
	}
	
	/*el = (struct t_mem_area*)el->next;*/

	i_start = start_pos;
	el_start = (unsigned int) el->start;

	if(((i_start >= el_start) && (i_start <= (el->size+el_start)))
		|| (((i_size+i_start) >= el_start) && ((i_size+i_start) <= (el->size+el_start))) ) {
		start_pos += el->size+el_start; /* Området används. */	

	}
	if((i_start <= el_start) && ((i_size+i_start) >= (el->size+el_start)))
		start_pos += el->size+el_start;
	

	start_pos = round_up_to_4KiB(start_pos);
	/* printf(" start_pos: %lu \n", start_pos); */
	return (void *)start_pos;
}

/* is_mem_used   - Tar reda på om ett minnesområde redan är upptaget, returnerar i
                 så fall SANT annars FALSKT. */
int   is_mem_used(struct t_mem_area *root_area, void * p_start, unsigned int i_size) {
	
	struct t_mem_area *el = root_area;
	unsigned int i_start = (unsigned int) p_start;
	unsigned int el_start = (unsigned int) el->start;
	/* printf("Is allocated?: %p - 0x%lx \n", p_start, (unsigned long int)i_start+i_size); */
	/* print_used_mem(root_area); */
	while(el->next) {
		el = (struct t_mem_area*)el->next;
		i_start = (unsigned int) p_start;
		el_start = (unsigned int) el->start;
		
		/*printf("if((%d >= %d) && (%d <= %d)\n", i_start, el_start, i_start, el->size+el_start); 
		printf("	&& (%d >= %d) && (%d <= %d) )\n",
				i_size+i_start, el_start, i_size+i_start, el->size+el_start );*/
		
	
		/* Om början på blocket är lika med eller större än el->start
		   och början på blocket är mindre än eller lika med el->size. */
		if(((i_start >= el_start) && (i_start <= (el->size+el_start)))
			|| (((i_size+i_start) >= el_start) && ((i_size+i_start) <= (el->size+el_start))) ) {
			/* printf("Yes1.\n"); */
			return 1; /* Området används. */	
		}
		if((i_start <= el_start) && ((i_size+i_start) >= (el->size+el_start))) {
			/* printf("Yes2.\n"); */
			return 1;
		}
		/* printf("No, continue.\n"); */
		
	}	
	i_start = (unsigned int) p_start;
	el_start = (unsigned int) el->start;
	
	/*printf("if((%d >= %d) && (%d <= %d)\n", i_start, el_start, i_start, el->size+el_start); 
	printf("	&& (%d >= %d) && (%d <= %d) )\n",
				i_size+i_start, el_start, i_size+i_start, el->size+el_start );*/
	
	if(((i_start >= el_start) && (i_start <= (el->size+el_start)))
			|| (((i_size+i_start) >= el_start) && ((i_size+i_start) <= (el->size+el_start))) ) {
		/* printf("Yes3.\n"); */
		return 1; /* Området används. */
	}
	if((i_start <= el_start) && ((i_size+i_start) >= (el->size+el_start))) {
		/* printf("Yes4.\n"); */
		return 1;
	}
	/* printf("No, do exit.\n"); */
	return 0;
}


/* alloc_mem_area- Allokerar ett område, returnerar sant om det gick annars
                 falskt. */
int   alloc_mem_area(struct t_mem_area *root_area, void * p_start, unsigned int i_size) {

	if(is_mem_used(root_area, p_start, i_size))
		return 0;
	struct t_mem_area *rot = root_area;
	struct t_mem_area * prev = rot;
	while(rot->next) {
		prev = rot;
		rot = (struct t_mem_area*)rot->next;
	}
		
	struct t_mem_area* p_tmp = (struct t_mem_area*) malloc(sizeof(struct t_mem_area));
	if(!p_tmp) /* Inget minne allokerat för noden.*/
		return 0;
	rot->next = (void*) p_tmp;
	p_tmp->start = p_start;
	p_tmp->size = i_size;
	p_tmp->next = 0; /* Initialize next. */
	return 1;
}

/* dealloc_mem_area - Avallokerar minnesområdet pekaren pekar på. */
int   dealloc_mem_area(struct t_mem_area *root_area, void * p_start) {
	printf("FIXME: memmgr.c:dealloc_mem_area()\n");
	return 0;
}

void  print_used_mem(struct t_mem_area *root_area) {


	unsigned long int start_pos = 0;
	struct t_mem_area *el = root_area;
	unsigned int i_start = (unsigned int) start_pos;
	unsigned int el_start = (unsigned int) el->start;
	printf("--- Used Mem in Memmgr ---\n");
	while(el || el->next) {
		
		i_start = start_pos;
		el_start = (unsigned int) el->start;
		
		printf("Allocated: %p - 0x%lx \n", el->start, (unsigned long int)el->size+el_start);
		if(!el->next)
			break;
		if(el->next)
			el = (struct t_mem_area*)el->next;
	}
	printf("---    End Mem dump    ---\n");	
}
