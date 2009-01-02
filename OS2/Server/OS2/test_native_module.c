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
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
//#include "../modlx.h"
//#include "../fixuplx.h"
//#include "../gcc_os2def.h"
#include <native_dynlink.h>

void test_native_module(void);

int main() {

        test_native_module();

        return 0;
}

typedef APIRET APIENTRY (*tmydl_DosPutMessage)(HFILE , ULONG , PCHAR);

tmydl_DosPutMessage mydl_DosPutMessage;

void test_native_module(void) {
        char mod_path[4096];
//        struct t_os2process proc;
        char * module_to_find;
        void * handle_msg;
        char* native_libpath[] = {
                "c:\\os2\\dll"
                        };

        module_to_find = "msg";

        init_native_dynlink();

        printf("Testar native_dynlink. \n");
        printf("Looking for '%s' \n", module_to_find);

        set_native_libpath(native_libpath, 2); /* Zero-based number i native_libpath.*/

        native_find_module_path(module_to_find, (char *)&mod_path);
        if(mod_path) {
                printf("native_find_module_path found: %s \n", mod_path);
                handle_msg = native_find_module(module_to_find);

                mydl_DosPutMessage = (tmydl_DosPutMessage)
                                                        native_get_func_ptr_ord_modname(5, module_to_find);
                mydl_DosPutMessage(0, 5, "Hello");
                printf("\n");
        }


        native_print_module_table();
}
