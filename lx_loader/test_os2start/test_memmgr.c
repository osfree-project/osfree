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
#include "../memmgr.h"
#include <stdio.h>
#include <string.h>

void find_module_path(char * name, char * full_path_name) {
	const char *mini_libpath[] = {"./mini_msg_dll", 
			"/mnt/rei3/OS2/os2_program/prog_iso/Mplayer_os2/mplayer",
			"/mnt/rei3/OS2/os2_program/prog_iso/Mplayer_os2/libc-0.6.1-csd1",
			};
	const int sz_mini_libpath = 2;
	const char *dll_suf = ".dll";
	char *sep = "/";
	
	char * p_buf = full_path_name;
	int i =0;
	FILE *f=0;
	do {
		p_buf = full_path_name;
		p_buf[0] = 0;
		strcat(p_buf, mini_libpath[i]);
		strcat(p_buf, sep);
		strcat(p_buf, name);
		strcat(p_buf, dll_suf);
		f = fopen(p_buf, "rb");
		++i;
	}while(!f && (i <= sz_mini_libpath));
	if(f)
		fclose(f);		
	else 
		p_buf[0] = 0;
}

int main(int argc, char ** argv) {

	printf("Testar memmgr. \n");
	
	char mod_path[4096];
	
	find_module_path("MSG", (char *)&mod_path);
	if(mod_path)
		printf("find_module_path hittar: %s \n", mod_path);
	find_module_path("libc061", (char *)&mod_path);
	if(mod_path)
		printf("find_module_path hittar: %s \n", mod_path);
		
	find_module_path("opps", (char *)&mod_path);
	if(mod_path)
		printf("find_module_path hittar: %s \n", mod_path);
	else 
		printf("find_module_path hittar inte opps: %s \n", mod_path);
	const int storlek = 8192;
	const int storlek2 = 8192+100;
	int i_p = 0x20000;
	void * p = (void*)i_p;
	int i_p5 = i_p-8;
	void * p_plus5 = (void*)i_p5;
	
	struct t_mem_area root_mem_area;
	init_memmgr(&root_mem_area);
	
	/* Registrate base invalid area. */ 
		alloc_mem_area(&root_mem_area, (void*) 1, 0xfffd);
	printf("Allokerar %p, storlek: %d. (0x%x)\n",(void*) 1, 0xfffe, 0xfffe );
	
	if(is_mem_used(&root_mem_area, p, storlek)) {
		printf("Minnesområdet från %p till 0x%x är ockuperat.\n",p, storlek+i_p);
	}
	else printf("Minnesområdet från %p till 0x%x är tillgängligt.\n",p, storlek+i_p);
	
	if(alloc_mem_area(&root_mem_area, p, storlek))
		printf("Allokerar %p, storlek: %d. (0x%x)\n",p, storlek, storlek );
	else printf("Kan inte allokera %p, storlek: %d. (0x%x)\n",p, storlek, storlek );
	
	
	
	if(is_mem_used(&root_mem_area, p, storlek)) {
		printf("Minnesområdet från %p till 0x%x är ockuperat.\n",p, storlek+i_p);
	}
	else printf("Minnesområdet från %p till 0x%x är tillgängligt.\n",p, storlek+i_p);
	
	/* alloc_mem_area(p_plus5, storlek+100); */
	
	if(is_mem_used(&root_mem_area, p_plus5, storlek2)) {
		printf("Minnesområdet från %p till 0x%x är ockuperat.\n",p_plus5, storlek2+i_p5);
	}
	else printf("Minnesområdet från %p till 0x%x är tillgängligt.\n",p_plus5, storlek2+i_p5);
	
	
		
	
	int storlek3 =0xffff;
	
	unsigned long int new_base = (unsigned long int) seek_free_mem(&root_mem_area, storlek3);
	i_p5 = new_base;
	p_plus5 = (void*)i_p5;
	
	printf("Söker efter fritt utrymme för blocket: %d, %p - 0x%x \n", storlek3, p_plus5, storlek3+i_p5);
	
	if(is_mem_used(&root_mem_area, p_plus5, storlek3)) {
		printf("Minnesområdet från %p till 0x%x är ockuperat.\n",p_plus5, storlek3+i_p5);
	}
	else printf("Minnesområdet från %p till 0x%x är tillgängligt.\n",p_plus5, storlek3+i_p5);
	
	print_used_mem(&root_mem_area);
	return 0;
}
