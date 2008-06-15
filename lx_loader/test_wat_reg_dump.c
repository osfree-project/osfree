
#include <stdio.h>

#ifdef __WATCOMC__

#endif 



void showRegDump_wat();

void main() {

	showRegDump_wat();
}

void showRegDump_wat() {
	unsigned long int _EDI,_ESI,_EIP,_ESP,_EBP;
	unsigned short int _DS,_ES,_FS,_GS,_CS,_SS;
	unsigned char _FLG=0;
	
	extern void show_regs(void);
    #pragma aux show_regs = \
          "mov _ESP, esp \n" \
		  "mov _EBP, ebp \n" \
		  "mov _EDI, edi \n" \
		  "mov _ESI, esi \n" \
		  "mov _DS,  ds  \n" \
		  "mov _ES,  es  \n" \
		  "mov _FS,  fs  \n" \
		  "mov _GS,  gs  \n" \
		  "mov _CS,  cs  \n" \
		  "mov _SS,  ss";
		   /*
					: [_EBP]  "=m" (_EBP),
					  [_EDI]  "=m" (_EDI),
					  [_ESI]  "=m" (_ESI),
					  //[EIP]  "=m" (EIP),
					  //[FLG]  "=m" (FLG),
					  [_DS]  "=m" (_DS),
					  [_ES]  "=m" (_ES),
					  [_FS]  "=m" (_FS),
					  [_GS]  "=m" (_GS),
					  [_CS]  "=m" (_CS),
					  [_SS]  "=m" (_SS),
					  [_ESP]  "=m" (_ESP)
					  */  
			/*"movl %%eip, %[EIP] \n"	
		  "movb %%fl, %[FLG] \n" */
		show_regs();  
	printf("ESI=%lx  EDI=%lx\n",_ESI,_EDI);
	printf("DS=%x  ES=%x  FS=%x  GS=%x \n", _DS, _ES, _FS, _GS);
	/*printf("DS=%x  DSACC=****  DSLIM=********\n", DS);
	printf("ES=%x  ESACC=****  ESLIM=********\n", ES);
	printf("FS=%x  FSACC=****  FSLIM=********\n", FS);
	printf("GS=%x  GSACC=****  GSLIM=********\n", GS);*/
	printf("CS:EIP=%x:%lx  CSACC=****  CSLIM=********\n", _CS,_EIP );
	printf("SS:ESP=%x:%lx  SSACC=****  SSLIM=********\n", _SS,_ESP );
	printf("EBP=%lx  FLG=%x\n", _EBP, _FLG);
}


