#include "ow_dlfcn.h"

void *dlopen (const char *__file, int __mode) {
    return 0;
}

int dlclose (void *__handle) {
    return 0;
}

void *dlsym (void *__restrict __handle,
		    const char *__restrict __name) {
    return 0;
}

char *dlerror (void) {
    return "";
} 
