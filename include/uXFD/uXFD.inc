; //int load(char far *image, unsigned long size, char far *load_addr, struct exe_params *p);
; int load(unsigned long image, unsigned long size, unsigned long load_addr, struct exe_params *p);

extern load:WORD

exe_params struc
   cs      dw      ?
   eip     dd      ?
   ss      dw      ?
   esp     dd      ?
exe_params ends
