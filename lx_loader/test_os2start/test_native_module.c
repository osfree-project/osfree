

#include <stdlib.h>
#include <stdio.h>
#include "../modlx.h" 
#include "../fixuplx.h"
#include "../gcc_os2def.h"
#include "../native_dynlink.h" 

void test_native_module(void);

int main() {

	test_native_module();

	return 0;
}

typedef unsigned int (*tmydl_DosPutMessage)(unsigned int , unsigned int , char *);

void test_native_module(void) {
	init_native_dynlink();
	
	printf("Testar native_dynlink. \n");
	
	char mod_path[4096];
	struct t_processlx proc;
	
	char * module_to_find = "msg";
	printf("Looking for '%s' \n", module_to_find);
	
	native_find_module_path(module_to_find, (char *)&mod_path);
	if(mod_path) {
		printf("native_find_module_path found: %s \n", mod_path);
		void * handle_msg = native_find_module(module_to_find, &proc);
		
		/* unsigned int DosPutMessage(unsigned int hfile, unsigned int cbMsg, char * pBuf)
		  int (*mydltest)(const char *s);
		*/
		
		tmydl_DosPutMessage mydl_DosPutMessage = (tmydl_DosPutMessage) 
							native_get_func_ptr_str_modname("DosPutMessage", module_to_find);	
		mydl_DosPutMessage(1, 5, "Hello");	
		printf("\n");
	}
	
	printf("Looking for 'libc061' \n");	
	native_find_module_path("libc061", (char *)&mod_path);
	if(mod_path)
		printf("native_find_module_path hittar: %s \n", mod_path);
	
	printf("Looking for 'opps' \n");	
	native_find_module_path("opps", (char *)&mod_path);
	if(mod_path)
		printf("native_find_module_path hittar: %s \n", mod_path);
	else 
		printf("native_find_module_path hittar inte opps: %s \n", mod_path);
	
	native_print_module_table();	
}
