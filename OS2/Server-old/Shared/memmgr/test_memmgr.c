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
#include <stdio.h>
#include <string.h>

#include <memmgr.h>

int main(int argc, char ** argv)
{
        char mod_path[4096];
        const int storlek = 8192;
        const int storlek2 = 8192+100;
        int i_p = 0x20000;
        void * p = (void*)i_p;
        int i_p5 = i_p-8;
        void * p_plus5 = (void*)i_p5;
        int storlek3 =0xffff;
        unsigned long int new_base;

        struct t_mem_area root_mem_area;

        printf("Testar memmgr. \n");

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


        new_base = (unsigned long int) seek_free_mem(&root_mem_area, storlek3);
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
