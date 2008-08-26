unsigned long LXIdentify(void * addr, unsigned long size);
unsigned long LXLoad(void * addr, unsigned long size, void ** lx_exe_mod);
unsigned long LXFixup(void * lx_exe_mod);
