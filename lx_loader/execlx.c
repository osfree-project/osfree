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


#include "execlx.h"
#include "fixuplx.h"
/*#include "msg.h"*/



/* #define __USE_POSIX199309 */
/*#include <signal.h>*/
#include <assert.h>
#include <sys/types.h>



#include <sys/mman.h> 
#ifndef TEST_OS2START
#ifdef SDIOS 
 #include <unistd.h>  
#endif
#endif
#include <errno.h>
/* #include <sched.h> */

#include <stdio.h>



/*
Systemfel - OS/2 - fönster
 SYS3271
 
Programfel (exception 80000001) vid 1e9b3719. Felet kan inte
hanteras p g a brist på stackutrymme. Processen har avbrutits.
FSLIB.DLL 0002:00003719
P1=00000002  P2=04f00000  P3=XXXXXXXX  P4=XXXXXXXX
EAX=00000000  EBX=00002020  ECX=00000028  EDX=00000001
ESI=04f01920  EDI=04efffd8
DS=0053  DSACC=f0f3  DSLIM=ffffffff
ES=0053  ESACC=f0f3  ESLIM=ffffffff
FS=150b  FSACC=00f3  FSLIM=00000030
GS=0000  GSACC=****  GSLIM=********
CS:EIP=005b:1e9b3719  CSACC=f0df  CSLIM=ffffffff
SS:ESP=0053:04f00000  SSACC=f0f3  SSLIM=ffffffff
EBP=04d70fe0  FLG=00012216

*/
/*
asm("movl %%esp, %[esp_data] \n" 
	"movl %%ebp, %[ebp_data]" 		
					: [ebp_data]  "=m" (ebp_data),
					  [esp_data]  "=m" (esp_data) : );
*/					  
/*inline */
void showRegDump() {
	unsigned long int EDI,ESI,EIP,ESP,EBP;
	unsigned short int DS,ES,FS,GS,CS,SS;
	unsigned char FLG=0;
	
	asm("movl %%esp, %[ESP] \n" 
		  "movl %%ebp, %[EBP] \n" 
		  "movl %%edi, %[EDI] \n"	
		  "movl %%esi, %[ESI] \n"	
		  /*"movl %%eip, %[EIP] \n"	
		  "movb %%fl, %[FLG] \n" */
		  "movl %%ds, %[DS] \n"
		  "movl %%es, %[ES] \n"
		  "movl %%fs, %[FS] \n"
		  "movl %%gs, %[GS] \n"
		  "movl %%cs, %[CS] \n"
		  "movl %%ss, %[SS]"
					: [EBP]  "=m" (EBP),
					  [EDI]  "=m" (EDI),
					  [ESI]  "=m" (ESI),
					  /*[EIP]  "=m" (EIP),
					  [FLG]  "=m" (FLG),*/
					  [DS]  "=m" (DS),
					  [ES]  "=m" (ES),
					  [FS]  "=m" (FS),
					  [GS]  "=m" (GS),
					  [CS]  "=m" (CS),
					  [SS]  "=m" (SS),
					  [ESP]  "=m" (ESP)  );
	printf("ESI=%lx  EDI=%lx\n",ESI,EDI);
	printf("DS=%x  ES=%x  FS=%x  GS=%x \n", DS, ES, FS, GS);
	/*printf("DS=%x  DSACC=****  DSLIM=********\n", DS);
	printf("ES=%x  ESACC=****  ESLIM=********\n", ES);
	printf("FS=%x  FSACC=****  FSLIM=********\n", FS);
	printf("GS=%x  GSACC=****  GSLIM=********\n", GS);*/
	printf("CS:EIP=%x:%lx  CSACC=****  CSLIM=********\n", CS,EIP );
	printf("SS:ESP=%x:%lx  SSACC=****  SSLIM=********\n", SS,ESP );
	printf("EBP=%lx  FLG=%x\n", EBP, FLG);
}


#ifdef BLA
/*   Not used.
Here's an example (pseudo-code) handler: 
*/

        void handle_fault(int sig_num, siginfo_t sig_info, void /*ucontext_t*/ *uc) 
        { 
                void * /*caddr_t*/         fault_addr; 
                int             fault_reason; 

                /*assert(sig_info.si_signo == SIGSEGV); */
				assert(sig_info.si_signo == SIGSEGV);

                fault_reason = sig_info.si_code; 
                fault_addr = sig_info.si_addr; 

                /* fault_reason is one of: 
                        SEGV_MAPERR - address not mapped to object 
                        SEGV_ACCERR - invalid permissions for mapped object 
                */ 

                /* uc points to the processor context which was 
                   interrupted by the fault.  ("man -s5 ucontext") */ 
        }
#endif


//    unsigned long       e32_startobj;   /* Object # for instruction pointer */
//    unsigned long       e32_eip;        /* Extended instruction pointer */
//    unsigned long       e32_stackobj;   /* Object # for stack pointer */
//    unsigned long       e32_esp;        /* Extended stack pointer */

void do_mmap_code_stack(struct LX_module * lx_exe_mod) {

}

/* 
     /pub/projekt_src/mylxread/hello_os2v2.exe
     /mnt/c/Projekt/Fandango on core/im_really_small_lx.exe
    /mnt/rei3/OS2/Fandango on core/im_really_small_lx_mz_fixad.exe
    /mnt/rei3/OS2/os2_program/prog_iso/Mplayer_os2/mplayer/mplayer.exe
  visa info om exe filen: 
    /pub/projekt_src/mylxread/dump_hello_os2v2_exe.txt
	/pub/projekt_src/mylxread/info om hello_os2v2 fran lxread.txt
*/

#ifndef SDIOS


void exec_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc) {
	
	
	struct o32_obj * kod_obj = (struct o32_obj *) get_code(lx_exe_mod);
	
	struct o32_obj * stack_obj = (struct o32_obj *) get_data_stack(lx_exe_mod);
	
	unsigned long eip = get_eip(lx_exe_mod) + kod_obj->o32_base;	
	unsigned long esp = get_esp(lx_exe_mod);
	
	void * main_ptr = (void *)eip;
	void * data_mmap = (void *)stack_obj->o32_base;
	
	void * my_execute = /*(int (*)()) (void*)*/ main_ptr;
		printf("Exerkverar LX program**********************\n");
		unsigned long int tmp_data_mmap = (unsigned long int) data_mmap;
		tmp_data_mmap += esp - 8;
		printf(" (esp+data_mmap-8) %lu (0x%lx)\n", (tmp_data_mmap), (tmp_data_mmap) );
		printf(" Sätter esp=0x%lx, ebp=0x%lx \n", (tmp_data_mmap), (tmp_data_mmap));
		printf(" my_execute: %p, eip: 0x%lx \n", my_execute, get_eip(lx_exe_mod));
		
		/* DosPutMessage(1, 6, "Hello\n"); */
		unsigned int esp_data=0;
		unsigned int ebp_data=0;

		/* Kopierar esp till variabeln esp_data. esp_data är en ut-variabel.*/
		/*asm("movl %%esp, %[esp_data]" 
					: [esp_data]  "=m" (esp_data): ); 
		asm("movl %%ebp, %[ebp_data]" 		
					: [ebp_data]  "=m" (ebp_data): ); */
			
		/* Save the registers ebp and esp. */		
		asm("movl %%esp, %[esp_data] \n" 
			"movl %%ebp, %[ebp_data]" 		
					: [ebp_data]  "=m" (ebp_data),
					  [esp_data]  "=m" (esp_data)  );
					
		/*unsigned int main_int = (unsigned int) *((char *)main_ptr);*/
		
		/* Put the values of ebp and esp in our new stack. */
		unsigned long int tmp_ptr_data_mmap = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap += esp-4;
		(*((unsigned long int *)(tmp_ptr_data_mmap)))   = esp_data;	
		
		tmp_ptr_data_mmap = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap += esp-8;
		(*((unsigned long int *)(tmp_ptr_data_mmap)))   = ebp_data;
		
		/* Kopiera de nya värdena för esp, ebp och my_execute till temp register.
		   Kanske EAX, EBX, ECX. Uppdatera esp och ebp. Anropa sen funktionen my_execute
		   med call EAX nånting.
		   Efter funktionen har körts? Var finns den gamla esp och ebp? ebp minus nånting
		   eller plus nånting? Ta reda på det och kopiera till temp register och sen 
		   uppdatera esp och ebp igen.
		*/
		tmp_ptr_data_mmap = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap += esp-12;
		esp_data = tmp_ptr_data_mmap;
		ebp_data = tmp_ptr_data_mmap;	
		
		unsigned long int tmp_ptr_data_mmap_16 = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap_16 += esp-16;
		
		unsigned long int tmp_ptr_data_mmap_21 = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap_21 += esp-21;
						
		/* Kopierar variabeln esp_data till esp! esp_data är en in-variabel.*/				
		asm("movl %[esp_data], %%eax \n"    /* Put old esp in eax */
			"movl %[ebp_data], %%ebx \n"    /* Put old ebp in ebx */
			"movl %[my_execute], %%ecx \n"
			
			"movl %%ebp, %%edx \n" /* Copy ebp to edx. Base pointer for this functions local variables.*/
			"movl %%eax, %%esp \n" /* Copy eax to esp. Stack pointer*/
			"movl %%ebx, %%ebp \n"
							/* We have changed the stack so it now points to out LX image.*/
			"push %%edx \n" /* Put the value of our ebp on our new stack*/
														/* "push $0xff \n" */
			"call *%%ecx \n" /* Call our main() */
					:  
					  :[esp_data]   "m" (tmp_ptr_data_mmap_16), /* esp+ data_mmap+8+*/
					   [ebp_data]   "m" (tmp_ptr_data_mmap_21), /* esp+ data_mmap+8+*/
					   [my_execute] "m" (my_execute) );

		/* OBS! Stacken är ändrad här !!!!! */
		/* Funkar inte, my_execute är en variabel med en pekare i stacken som
		 inte kan läsas efter att stacken ändrats! Baseras på ebp!
		 Alla värden måste läsas in i register och sen placeras på rätt 
		 ställen. */	
		 int tcc_bugg_;
		asm("pop %%ebp \n"  /* Restore base pointer so we don't crash as soon we access local variables.*/
		    "pop %%ebx \n" 
		    "pop %%ebx \n"
					: /*: [tcc_bugg_] "m" (tcc_bugg_)*/ ); 
					
		/*  my_execute("" , "" , 0, (unsigned int)lx_exe_mod );   */

		printf("Slutförd exerkvering LX program**********************\n");
	
}

#ifdef NOT_DEFINED_AND_NOT_USED

/* Old stuff. My first attempts :) with LX loading.*/

/* Exerkverar en process. */		
void exec_lx_old_for_linux(struct LX_module * lx_exe_mod, struct t_processlx * proc) {		
	/* struct o32_obj * kod_obj = get_obj(lx_exe_mod, lx_exe_mod->lx_head_e32_exe->e32_startobj); */
	struct o32_obj * kod_obj = (struct o32_obj *) get_code(lx_exe_mod);
	
	struct o32_obj * stack_obj = (struct o32_obj *) get_data_stack(lx_exe_mod);
	
	/*unsigned long       o32_size;        Object virtual size 
    unsigned long       o32_base;        Object base virtual address 
    unsigned long       o32_flags;       Attribute flags 
    unsigned long       o32_pagemap;     Object page map index 
    unsigned long       o32_mapsize;     Number of entries in object page map 
    unsigned long       o32_reserved;    Reserved 
	*/
	printf("Objektnr för kod: %lu, virtuell storlek: %lu \n", 
				lx_exe_mod->lx_head_e32_exe->e32_startobj,
				kod_obj->o32_size);
	printf("basaddress(vir): %lu, obj.pg.map.idx: %lu\n",				
				kod_obj->o32_pagemap,
				kod_obj->o32_mapsize);
	/* struct o32_obj * kod_obj = get_code(lx_exe_mod); */
	unsigned long eip = get_eip(lx_exe_mod);	
	unsigned long esp = get_esp(lx_exe_mod);
	void * lxfile = 0, * data_mmap = 0;
	
	printf("eip: %lu (%lx) \n", eip, eip); 
	printf("esp: %lu (0x%lx)\n", esp, esp);
	
	print_o32_obj_info(kod_obj, " Info om kod_obj ");
	print_o32_obj_info(stack_obj, " Info om stack_obj ");
	
	/* ladda kod. */
	
	/* ladda stack data. */
	
	/*
	const int START_OFF = 4096; */
	
	/* Samma objekt för data och kod.*/
	
	int err = errno;
	/* Testar att minnesmappa (läs- och körbar) hela filen från en absolutaddressen 4096. */
	/*lxfile = mmap(kod_obj->o32_base, kod_obj->o32_size, 
					 PROT_READ | PROT_EXEC,
					 MAP_PRIVATE, lx_exe_mod->fh, 
					 lx_exe_mod->lx_head_e32_exe->e32_datapage);
				*/
	err = errno;
	printf("(1)Minnesmappar kodobj på %lu (%p), (kod)filstorlek: %lu från %lu \n", 
		    (unsigned long int)lxfile, lxfile, kod_obj->o32_size, 
		   lx_exe_mod->lx_head_e32_exe->e32_datapage);

	/*if(lxfile == MAP_FAILED) { */
	/* printf("Fel vid mappning av fh: %d. Prövar igen.\n", lx_exe_mod->fh); */
	
    /* Allokerar virtuellt minne för kod, antar alltid att koden finns.  */
	
	lxfile = mmap((void *)kod_obj->o32_base, kod_obj->o32_size, 
				 PROT_WRITE | PROT_READ | PROT_EXEC,
				 MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0/*lx_exe_mod->fh*/, 
				 0 /*lx_exe_mod->lx_head_e32_exe->e32_datapage*/);
	/* Anonymt virt. minne, fh och offset ignoreras.*/
	err = errno;

	/* Olika objekt för data och kod. 
	   Allokerar virtuellt minne för stack/data objktet, om det behövs. */
	   
	if(get_code_num(lx_exe_mod) != get_data_stack_num(lx_exe_mod)) {
		data_mmap = mmap((void *)stack_obj->o32_base, stack_obj->o32_size, 
				 PROT_WRITE | PROT_READ | PROT_EXEC  ,       /* | PROT_EXEC */
				  MAP_GROWSDOWN | MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0/*lx_exe_mod->fh*/, 
				 0 /*lx_exe_mod->lx_head_e32_exe->e32_datapage*/);;
		proc->stack_mmap = data_mmap;
		if(data_mmap == MAP_FAILED) 
			printf("Fel vid mappning av (stack)\n");
		else {
			printf("Ok av mappning av (stack)\n");
			
			/* Hämta första objektet för stack/data till en början.*/
			/* Fältet o32_pagemap är numret på första sidan i "Object Page Table"
			   Antal poster står i fältet o32_mapsize. 
			   Så för att läsa en sida behövs bara tas reda på var alla sidor börjar
			   i fältet "Data Pages Offset" i LX huvudet, fältet är o32_datapage. 
			   Fältet o32_datapage är antal byte mätt från allra början på exefilen,
			   från DOS-stubben.
			*/
			int startpage = stack_obj->o32_pagemap;
			int lastpage  = stack_obj->o32_pagemap + stack_obj->o32_mapsize;
			unsigned int data_pages_offs =  get_e32_datapage(lx_exe_mod);
			unsigned int stack_mmap_pos = 0;
			int page_nr=0;
			for(page_nr=startpage; page_nr<=lastpage; page_nr++) {
			 	struct o32_map * obj_pg_sta = get_obj_map(lx_exe_mod ,page_nr);
				int ofs_page_sta = (obj_pg_sta->o32_pagedataoffset << get_e32_pageshift(lx_exe_mod))
									+ data_pages_offs;
								 
				lx_exe_mod->lx_fseek(lx_exe_mod, ofs_page_sta, SEEK_SET);

				unsigned long int tmp_proc_stack_mmap = (unsigned long int) proc->stack_mmap;
				tmp_proc_stack_mmap += stack_mmap_pos;
				
				lx_exe_mod->lx_fread((void *) tmp_proc_stack_mmap,  
									obj_pg_sta->o32_pagesize, 1, lx_exe_mod);
				stack_mmap_pos += obj_pg_sta->o32_pagesize;
			}
		}
	}
	err = errno;
	/*e32_itermap; */   /* Object iterated data map offset */
	printf("(2)Minnesmappar kodobj på %p (%p), (kod)filstorlek: %lu från %lu \n", 
		    lxfile, lxfile, kod_obj->o32_size, 
			lx_exe_mod->lx_head_e32_exe->e32_datapage);

	if(lxfile == MAP_FAILED) 
		printf("Fel vid mappning.\n");
	else {
		printf("Ok av mappning (kodobj) .\n" );
		proc->code_mmap = lxfile;

		int startpage = kod_obj->o32_pagemap;
		int lastpage  = kod_obj->o32_pagemap + kod_obj->o32_mapsize;
		unsigned int data_pages_offs =  get_e32_datapage(lx_exe_mod);
		unsigned int code_mmap_pos = 0;
		int page_nr=0;
		/* struct o32_map * obj_pg_ett = get_obj_map(lx_exe_mod ,startpage); */
		  /*  Läser in alla sidor från kodobjektet till det angivna virtuella minnesutrymmet. */
		for(page_nr=startpage; page_nr<lastpage; page_nr++) {
			struct o32_map * obj_pg_sta = get_obj_map(lx_exe_mod ,page_nr);
			int ofs_page_sta = (obj_pg_sta->o32_pagedataoffset << get_e32_pageshift(lx_exe_mod)) 
								+ data_pages_offs;

			lx_exe_mod->lx_fseek(lx_exe_mod, ofs_page_sta, SEEK_SET);

			unsigned long int tmp_proc_code_mmap = (unsigned long int) proc->code_mmap;
			tmp_proc_code_mmap += code_mmap_pos;

			lx_exe_mod->lx_fread((void*) tmp_proc_code_mmap,  
								obj_pg_sta->o32_pagesize, 1, lx_exe_mod);
			code_mmap_pos += obj_pg_sta->o32_pagesize;
		}
		
		/* Hämta första objektet för koden till en början.*/
		/*
		struct o32_map * obj_pg_ett = get_obj_map(lx_exe_mod ,kod_obj->o32_pagemap);
		int ofs_first_page = obj_pg_ett->o32_pagedataoffset + 
							 lx_exe_mod->lx_head_e32_exe->e32_datapage;
							 
		lx_exe_mod->lx_fseek(lx_exe_mod, ofs_first_page, SEEK_SET);		
		lx_exe_mod->lx_fread(proc->code_mmap, obj_pg_ett->o32_pagesize, 1, lx_exe_mod);
		*/
		
		unsigned long int tmp_lxfile = (unsigned long int) lxfile;
		tmp_lxfile += get_eip(lx_exe_mod);
		
		void * main_ptr = (void *) tmp_lxfile;
		printf(" main_ptr: %d (%p) \n", (unsigned int)main_ptr, main_ptr);
		unsigned int main_int = (unsigned int) *((char *)main_ptr);
		printf(" main_int : %d (0x%x) \n", main_int, main_int );
//------------------------------------	kod_obj	
/*		if((off > obj_pg_ett->o32_pagedataoffset)   
				&& (off < (obj_pg_ett->o32_pagedataoffset + obj_pg_ett->o32_pagesize))) */
		/* Är tänkt att göra alla "fixups" på första sidan för koden. */
		/* unsigned int tmp_off=1; */
//		 while( (tmp_off > obj_pg_ett->o32_pagedataoffset)   
// 				&& (tmp_off < (obj_pg_ett->o32_pagedataoffset + obj_pg_ett->o32_pagesize))) {
// 			printf("while( (%d > %d) && (%d < (%d + %d)) )\n", tmp_off, obj_pg_ett->o32_pagedataoffset, 
// 															tmp_off, obj_pg_ett->o32_pagedataoffset,
// 															obj_pg_ett->o32_pagesize);
// 		   fake_pagefault_code(lx_exe_mod, tmp_off, proc); /* Fejka sidfel på programkoden i lxfilen. */
// 		   tmp_off=+ 512;
// 		} 
		
		/* Utför fixup på varje sida i page_nr. Leta efter sidans fixup struktur i 
		   "Fixup Page Table".
		*/
		startpage = kod_obj->o32_pagemap;
		lastpage  = kod_obj->o32_pagemap + kod_obj->o32_mapsize;
		printf("--------------------Listar fixup data ------------------------- \n");
		for(page_nr=startpage; page_nr < lastpage; page_nr++) {
			
			/* Hämtar byteposition för fixup från sidan logisk_sida. */
			int pg_offs_fix = get_fixup_pg_tbl_offs(lx_exe_mod, page_nr);

			/*struct r32_rlc * get_fixup_rec_tbl_obj(struct LX_module * lx_mod, int offs) */

			/* Hämtar relokeringsstruktur från offset pg_offs_fix.*/
			struct r32_rlc * min_rlc = get_fixup_rec_tbl_obj(lx_exe_mod, pg_offs_fix);
						
			print_struct_r32_rlc_info(min_rlc);
			printf(" pg_offs_fix = %d (0x%x)\n", pg_offs_fix, pg_offs_fix);
		}
		
//------------------------------------
		int (* my_execute)(/*void* cmd_line, char* env_data , 
								 int reserved, unsigned int mod_handle*/
								 /*, return_address*/);
		/*
		clone - create a child process
       	#include <sched.h>
       	int clone(int (*fn)(void *),  void *child_stack,  int flags,  void *arg);
		*/
		/*int ret = clone(my_execute, get_esp(lx_exe_mod) + proc->stack_mmap,
		CLONE_VM, ""); */
		
		 /* (int (*)()) */
		
		my_execute = (int (*)()) (void*) main_ptr;
		printf("Exerkverar LX program**********************\n");
		unsigned long int tmp_data_mmap = (unsigned long int) data_mmap;
		tmp_data_mmap += esp - 8;
		printf(" (esp+data_mmap-8) %lu (0x%lx)\n", (tmp_data_mmap), (tmp_data_mmap) );
		printf(" Sätter esp=0x%lx, ebp=0x%lx \n", (tmp_data_mmap), (tmp_data_mmap));
		
		/* DosPutMessage(1, 6, "Hello\n"); */
		unsigned int esp_data=0;
		unsigned int ebp_data=0;

		/* Kopierar esp till variabeln esp_data. esp_data är en ut-variabel.*/
		/*asm("movl %%esp, %[esp_data]" 
					: [esp_data]  "=m" (esp_data): ); 
		asm("movl %%ebp, %[ebp_data]" 		
					: [ebp_data]  "=m" (ebp_data): ); */
					
		asm("movl %%esp, %[esp_data] \n" 
			"movl %%ebp, %[ebp_data]" 		
					: [ebp_data]  "=m" (ebp_data),
					  [esp_data]  "=m" (esp_data)  );
					
		/*unsigned int main_int = (unsigned int) *((char *)main_ptr);*/
		
		unsigned long int tmp_ptr_data_mmap = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap += esp-4;
		(*((unsigned long int *)(tmp_ptr_data_mmap)))   = esp_data;	
		
		tmp_ptr_data_mmap = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap += esp-8;
		(*((unsigned long int *)(tmp_ptr_data_mmap)))   = ebp_data;
		
		/* Kopiera de nya värdena för esp, ebp och my_execute till temp register.
		   Kanske EAX, EBX, ECX. Uppdatera esp och ebp. Anropa sen funktionen my_execute
		   med call EAX nånting.
		   Efter funktionen har körts? Var finns den gamla esp och ebp? ebp minus nånting
		   eller plus nånting? Ta reda på det och kopiera till temp register och sen 
		   uppdatera esp och ebp igen.
		*/
		tmp_ptr_data_mmap = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap += esp-12;
		esp_data = tmp_ptr_data_mmap;
		ebp_data = tmp_ptr_data_mmap;	
		
		unsigned long int tmp_ptr_data_mmap_16 = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap_16 += esp-16;
		
		unsigned long int tmp_ptr_data_mmap_21 = (unsigned long int)data_mmap;
		tmp_ptr_data_mmap_21 += esp-21;
						
		/* Kopierar variabeln esp_data till esp! esp_data är en in-variabel.*/				
		asm("movl %[esp_data], %%eax \n"    /* Sätter esp med det föregående värdet! */
			"movl %[ebp_data], %%ebx \n"    /* Sätter ebp med det föregående värdet! */
			"movl %[my_execute], %%ecx \n"
			
			"movl %%ebp, %%edx \n"
			"movl %%eax, %%esp \n"
			"movl %%ebx, %%ebp \n"
			
			"push %%edx \n"
														//"push $0xff \n"
			"call *%%ecx \n" 
					:  
					  :[esp_data]   "m" (tmp_ptr_data_mmap_16), /* esp+ data_mmap+8+*/
					   [ebp_data]   "m" (tmp_ptr_data_mmap_21), /* esp+ data_mmap+8+*/
					   [my_execute] "m" (my_execute) );

		/* OBS! Stacken är ändrad här !!!!! */
		/* Funkar inte, my_execute är en variabel med en pekare i stacken som
		 inte kan läsas efter att stacken ändrats! Baseras på ebp!
		 Alla värden måste läsas in i register och sen placeras på rätt 
		 ställen. */	
		int tcc_bugg2_;
		asm("pop %%ebp \n" /* Testar */
		    "pop %%ebx \n" 
		    "pop %%ebx \n"
					:[tcc_bugg2_] "m" (tcc_bugg2_) ); 
					
		/*  my_execute("" , "" , 0, (unsigned int)lx_exe_mod );   */

		printf("Slutförd exerkvering LX program**********************\n");

		/* my_execute ==  o32_base + eip, get_eip(lx_exe_mod) + lxfile
		my_execute(cmd_line, env_data, reserved, mod_handle, return_address) */
	} 
		
	/*} else {
		printf(" INGEN MAPPNING HAR SKETT FÖR EXEFILEN HAR MER ÄN ETT OBJEKT!\n");
	}*/
	
}


#endif

#else 
/* SDIOS */

/* */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
/* #include <unistd.h> */

#include <elf.h>

#include <sdi/log.h>
#include <sdi/panic.h>

#include <idl4glue.h>
#include <if/ifmemory.h>

//#define IDL4_ARCH ia32
//#define IDL4_API v4
//#include <if/ifmemory.h>
 
//#include <idl4/macros.h>
//#include <idl4/corba.h>
//#include <idl4/api/v4/interface.h>
//#include <l4/thread.h>
//#include <idl4glue.h>



// the main difficulty while reading this function is that there are two
// address spaces involved: the current one and the target one. the stack in
// the current address space in a shared page, in the destination the stack
// begins at 0xC0000000
static L4_Word_t makenewstack(L4_ThreadId_t pagerid, L4_ThreadId_t newthread,
							  /*const*/ char *const argv[], /*const*/ char *const envp[], 
							  struct o32_obj * stack_obj)
{
	typedef L4_Word_t addr_t;

	printf(">>makenewstack():%d\n", __LINE__);
	// 16 kb maximum environment and arguments. raise if needed. using this
	// limit saves from doing argument wrappings on fpage boundaries
	static const unsigned int maxenvstacksize = 0x8000;

	//static addr_t stackend = 0xC0000000;
	
	static addr_t stackend = 0x90000000 + stack_obj->o32_size + stack_obj->o32_base;
	//static addr_t stackend = 0x90000000 + stack_obj->o32_size;
	//printf(" %d: stackend=%p\n", __LINE__, stackend);
	//printf(" Stackomrade: %x - %x\n", 0x90000000 + stack_obj->o32_base, 0x90000000 + stack_obj->o32_size + stack_obj->o32_base);
	// request the stack fpage from the pager
	
	CORBA_Environment env (idl4_default_environment);

	//L4_Flush(L4_Fpage(0x90000000, 0x28000) + L4_FullyAccessible);
	
	if (L4_Pager() != pagerid) {
		// set receive window: this is evil as it has to take some
		// arbitrary position in memory
	    idl4_set_rcv_window(&env, L4_Fpage(0x90000000, maxenvstacksize));
	    //idl4_set_rcv_window(&env, L4_Fpage(stack_obj->o32_base, maxenvstacksize));
	
	} else {
		// much nicer: let our pager decide where to place the shared page
		idl4_set_rcv_window(&env, L4_CompleteAddressSpace);
	}

	idl4_fpage_t stackpage;
	//printf("\nAllocated: 0x%08X \n", stackpage);
	memset(&stackpage, 0, sizeof(stackpage));

	//printf(">>stackend: 0x%x, maxenvstacksize: 0x%x\n", stackend, 
	//						maxenvstacksize);
	unsigned int o32_size = stack_obj->o32_size /* + stack_obj->o32_size + 0x17000*/;
	printf("%d:_GetSharedRegion(%p, %p, 0x%x, 0x%x, %p, %p)\n",__LINE__, 
					pagerid, &newthread, stack_obj->o32_base, o32_size, //maxenvstacksize, 
					&stackpage, &env);
	
	/*
	This call is implemented in sdios06/src/pager/pager-impl.h:212				
	IDL4_INLINE void pager_GetSharedRegion_implementation(CORBA_Object _caller, 
													const L4_ThreadId_t *destthread, 
													const L4_Word_t destaddress, 
													const L4_Word_t pagesize, 
													idl4_fpage_t *page, 
													idl4_server_environment *_env)
	*/
	
	IF_MEMORY_GetSharedRegion(pagerid,
							  &newthread,
							  stack_obj->o32_base, //stackend - maxenvstacksize,
							  o32_size, //maxenvstacksize,
							  &stackpage, 
							  &env);

	if (env._major != CORBA_NO_EXCEPTION)
	{
		LogMessage("elfexec: pager call GetSharedRegion failed for stack, code: %p", 
					CORBA_exception_id(&env));
		return 0x00000000;
	}
	if (L4_IsNilFpage(idl4_fpage_get_page(stackpage)))
	{
		LogMessage("elfexec: pager call GetSharedRegion return nilpage.");
		printf(" stackpage:0x%x\n", L4_Address (idl4_fpage_get_page(stackpage)));
		return 0x00000000; 
	}
	L4_Word_t stackpageaddr = L4_Address (idl4_fpage_get_page(stackpage));
	//L4_Word_t stackpageaddr = 0x90000000 + stack_obj->o32_base;  
	 
	if (L4_Pager() == pagerid) {
		printf("idl4_fpage_get_base(stackpage=%lu)\n", stackpage); /* stack_obj->o32_size + */
		stackpageaddr = idl4_fpage_get_base(stackpage);
	}
	printf(">>stackpageaddr: 0x%x, stackpage: 0x%x\n", stackpageaddr, stackpage); 

	// ** construct stack

	if (!argv) {
		static /*const*/ char* argvnull[] = { NULL };
		argv = argvnull;
	}
	if (!envp) {
		static /*const*/ char* envpnull[] = { NULL };
		envp = envpnull;
	}
			/* 0xC0000000; */
	addr_t top = stack_obj->o32_size + stack_obj->o32_base;

	// evil: heavily depends on wrapping of unsigned ints
	/*  from char* to addr_t*  */ 
	/*addr_t *stack = reinterpret_cast<addr_t*>(stackpageaddr) + maxenvstacksize - top;*/
	addr_t *stack = reinterpret_cast<addr_t*>(stackpageaddr);
	
	printf("%d>>stack: %lu(0x%x), stackpageaddr:%lu(0x%x), maxenvstacksize: %lu(0x%x), top: %lu(0x%x)\n", 
				__LINE__,stack,stack, stackpageaddr,stackpageaddr, 
				maxenvstacksize,maxenvstacksize, 
				top,top);
	//showRegDump();

	// count arguments and environment
	unsigned int argvnum = 0;
	while(argv[argvnum]) argvnum++;

	unsigned int envpnum = 0;
	while(envp[envpnum]) envpnum++;

	// allocate buffers for string pointers on stack
	addr_t argvptr[argvnum+1];
	addr_t envpptr[envpnum+1];

	//printf("put a zero word at the bottom of the stack\n");
	// printf(" %d: (stack(%p) + top(%p)):%p = 0\n", __LINE__, stack,top, stack + top);
	//printf("Kabooom!!!\n");
	// put a zero word at the bottom of the stack
	/*unsigned int x;											// 0x400=1024
	for(x=0x90000000; x<stackend; x+=4096) {
		printf("(x):%x = 0\n", x);
		*(uint32_t*)(x) = 0x00000000;
	}*/
	
	top -= sizeof(uint32_t);  
	addr_t top_stack = top;
	
	unsigned int itop = top;
	unsigned int istack = (unsigned int)stack;
	
	top_stack = (itop - stack_obj->o32_base + istack);
	printf(" %d: top_stack:0x%x (stack(%p), *stack(%p)+ top(%p)):%p = 0\n", 
			__LINE__, top_stack, stack, stack,top, top_stack);
	printf(" itop = %lu (%x)\n", itop, itop);
	printf(" istack = %lu (%x)\n", istack, istack);
	printf(" top = %lu (%x)\n", top, top);
	printf(" stack = %lu (%x)\n", stack, stack);
	printf(" top_stack = %lu (%x)\n", top_stack, top_stack);
	printf(" *(uint32_t*)(top_stack)=%p \n", *(uint32_t*)(top_stack));

	*(uint32_t*)(top_stack) = 0x00000000;
	
	//printf(" envpptr=%p\n", envpptr);
	// put the environment variables on the stack
	for(unsigned int ei = 0; envp[ei]; ei++)
	{
		unsigned int el = strlen(envp[ei]) + 1; // with null char

		top -= el;
		//memcpy(top+stack, envp[ei], el);
		memcpy((void*)top_stack, envp[ei], el);

		envpptr[ei] = top;
	}

	envpptr[envpnum] = (addr_t)NULL;

	// put command line parameters on the stack
	for(unsigned int ai = 0; argv[ai]; ai++)
	{
		unsigned int al = strlen(argv[ai]) + 1;
		
		top -= al;
		memcpy(top+stack, argv[ai], al);

		argvptr[ai] = top;
	}

	argvptr[argvnum] = (addr_t)NULL;

	// align stack pointer to 4 bytes
	top -= ((unsigned int)top % 4);

	
	// add pointer arrays for environment and argv
	top -= sizeof(char*) * (envpnum+1);
	//printf(" %d:memcpy\n", __LINE__);
	//memcpy((addr_t*)top+(*stack), envpptr, sizeof(char*) * (envpnum+1));
	memcpy((void*)top_stack, envpptr, sizeof(char*) * (envpnum+1));
	addr_t envpstart = top;

	top -= sizeof(char*) * (argvnum+1);
	//memcpy((addr_t*)top+(*stack), argvptr, sizeof(char*) * (argvnum+1));
	memcpy((void*)top_stack, argvptr, sizeof(char*) * (argvnum+1));
	addr_t argvstart = top;
	
	// construct main()'s parameters:
	// printf("construct main()'s parameters\n");
	top -= sizeof(char*);
	// *(addr_t*)(top+stack) = envpstart;  	// envp
	*(addr_t*)((void*)top_stack) = envpstart;  	// envp

	top -= sizeof(char*);
	// *(addr_t*)(top+stack) = argvstart;		// argv
	*(addr_t*)((void*)top_stack) = argvstart;		// argv

	top -= sizeof(int);
	// *(int*)(top+stack) = argvnum;			// argc;
	*(int*)((void*)top_stack) = argvnum;			// argc;

	// return stack fpage
	
	L4_Fpage_t stackfpage = L4_Fpage(stackpageaddr, maxenvstacksize);
	IF_MEMORY_FreeSharedRegion(pagerid, &newthread, 
								stack_obj->o32_base, //stackend - maxenvstacksize, stack_obj->o32_base  o32_size
								&stackfpage, &env);
	printf("FreeSharedRegion (, ..., stack_obj->o32_base=0x%x, stackfpage.raw=ox%x\n", 
			stack_obj->o32_base, //stackend - maxenvstacksize, 
			stackfpage.raw);
	if (env._major != CORBA_NO_EXCEPTION)
	{
		LogMessage("elfexec: pager call FreeSharedRegion failed, code: %d", CORBA_exception_id(&env));
		return 0x00000000;
	}

	// trying to work-around a bug in L4 which happens after numerous mapping
	// of different size on the same region
	printf("work-around a bug in L4... stackpageaddr=%p, maxenvstacksize=%p\n", 
			stackpageaddr, maxenvstacksize);
	L4_Flush(L4_Fpage(stackpageaddr, maxenvstacksize) + L4_FullyAccessible);
	

	// return esp of new task
	printf("makenewstack(), Ok. \n");
	return top;
}



// Implementation of loading an LX file into memory in SDIOS. I am reusing parts from elfexec.cc.

void exec_lx(struct LX_module * lx_exe_mod, struct t_processlx * proc) {

	L4_ThreadId_t pagerid = L4_Pager();

	
	CORBA_Environment env (idl4_default_environment);

	L4_ThreadId_t newthread = L4_nilthread;

	IF_MEMORY_CreateTask(pagerid, &newthread, &env);
	if (env._major != CORBA_NO_EXCEPTION)
	{
		printf("lxexec:%d pager call CreateTask failed, code: %d", __LINE__, CORBA_exception_id(&env));
		return ;//L4_nilthread;
	} 

	//printf("Lyckades skapa en 'task': %p \n", &newthread);
	
	struct o32_obj * kod_obj = (struct o32_obj *) get_code(lx_exe_mod);
	
	struct o32_obj * stack_obj = (struct o32_obj *) get_data_stack(lx_exe_mod);
	
	printf("Objekt för kod: %lu, virtuell storlek: %lu \n", 
				lx_exe_mod->lx_head_e32_exe->e32_startobj,
				kod_obj->o32_size);
	printf("basaddress(vir): %lu, obj.pg.map.idx: %lu\n",				
				kod_obj->o32_pagemap,
				kod_obj->o32_mapsize);
	
	unsigned long eip = get_eip(lx_exe_mod);	
	unsigned long esp = get_esp(lx_exe_mod);
	void * code_mmap = 0, * data_mmap = 0;
	
	//printf("eip: %lu (0x%lx) \n", eip, eip); 
	printf("esp: %lu (0x%lx)\n", esp, esp);
	printf("eip+kod_obj->o32_base: %lu (0x%lx) \n", eip+kod_obj->o32_base, eip+kod_obj->o32_base);
	eip += kod_obj->o32_base; 
	esp += stack_obj->o32_base;
	//print_o32_obj_info(kod_obj, " Info om kod_obj ");
	//print_o32_obj_info(stack_obj, " Info om stack_obj ");
	
	// Allocate virtual memory staring from kod_obj->o32_base with size of kod_obj->o32_size.
	//kod_obj->o32_base, kod_obj->o32_size
	
	
	// figure out the start of the program image in the file
	L4_Word_t fstart = get_e32_datapage(lx_exe_mod);  //phdr.p_offset;

	// the start of the image in virtual memory
	L4_Word_t mstart = kod_obj->o32_base;  //phdr.p_vaddr;
	
	// the size of the fpage we are going to fill, not the size of the
	// program image
	L4_Word_t msize = get_e32_pagesize(lx_exe_mod);  //phdr.p_filesz
	
	//struct e32_exe {
	// e32_pagesize;   /* .EXE page size */
	// e32_pageshift;  /* Page alignment shift in .EXE */
	
	// load the image blockwise and try to align blocks to this modulo
	const L4_Word_t mblock = get_e32_pagesize(lx_exe_mod);  //0x8000;
	
	
	if ((mstart & (mblock - 1)) != 0) {
			LogMessage("lx warning: program image (0x%lx) is not aligned to 0x%lx bytes",
					   mstart, mblock);
			panic("lxexec: program image is not aligned to n bytes");
	}
	
	/* Hämta första objektet till en början.*/
		struct o32_map * obj_pg_ett = get_obj_map(lx_exe_mod ,kod_obj->o32_pagemap);
		
		int ofs_first_page = obj_pg_ett->o32_pagedataoffset + 
							 lx_exe_mod->lx_head_e32_exe->e32_datapage;
		//fseek(lx_exe_mod->fh, ofs_first_page, SEEK_SET);

		/*read (lx_exe_mod->fh, proc->code_mmap, obj_pg_ett->o32_pagesize );*/
		
		//fread(proc->code_mmap, 1, obj_pg_ett->o32_pagesize, lx_exe_mod->fh);
	
	for(L4_Word_t moff = 0; moff < msize; moff += mblock)
		{
			// request a shared fpage from the pager
		   if (L4_Pager() != pagerid) {
				// set receive window: this is evil as it has to take some
				// arbitrary position in memory
				idl4_set_rcv_window(&env, L4_Fpage(0x90000000, mblock));
			//idl4_set_rcv_window(&env, L4_Fpage(kod_obj->o32_base, mblock));
			} else {
				// much nicer: let our pager decide where to place the shared page
				idl4_set_rcv_window(&env, L4_CompleteAddressSpace);
			}
	
			idl4_fpage_t mpage;
			memset(&mpage, 0, sizeof(mpage));

			// IDL4_INLINE void pager_GetSharedRegion_implementation(CORBA_Object _caller, 
			//					 const L4_ThreadId_t *destthread, 
			//					 const L4_Word_t destaddress, 
			//					 const L4_Word_t pagesize, 
			//					 idl4_fpage_t *page, 
			//					 idl4_server_environment *_env)
			//
			//	pagerid         our pager 
			//	&newthread,     our new thread 
			//	mstart + moff,  start of virtual memory + copied contents.
			//	mblock, &mpage, &env 
			//  
			//  
			printf("_GetSharedRegion(pagerid, &newthread, mstart + moff, mblock, &mpage, &env)\n");
			printf("lxexec:%d _GetSharedRegion(%u, %p, %d, %d, \n", __LINE__,
					pagerid, &newthread, mstart + moff, mblock);
			printf("%p, %p)\n", &mpage, &env);
			printf(" mpage: .base=%lu .fpage=%lu \n", mpage.base, mpage.fpage);
			L4_Fpage_t tmp_fpage;
			tmp_fpage.raw = mpage.fpage;
			printf(" (L4_Fpage_t)mpage.fpage: .raw=%d .X.s=%d .X.rwx=%d\n",
					tmp_fpage.raw, tmp_fpage.X.s, tmp_fpage.X.rwx);
			
			IF_MEMORY_GetSharedRegion(pagerid, &newthread, mstart + moff, mblock, &mpage, &env);
	
			if (env._major != CORBA_NO_EXCEPTION)
			{
				LogMessage("lxexec: pager call GetSharedRegion failed, code: %d", CORBA_exception_id(&env));
				return ; //L4_nilthread;
			}
			if (L4_IsNilFpage(idl4_fpage_get_page(mpage)))
			{
				LogMessage("lxexec: pager call GetSharedRegion return nilpage.");
				return ; //L4_nilthread;
			}

			L4_Word_t mpageaddr = (L4_Pager() == pagerid) ? idl4_fpage_get_base(mpage) : 0x90000000;
			L4_Fpage_t mfpage = L4_Fpage(mpageaddr, mblock);

			L4_Word_t mcopylen = obj_pg_ett->o32_pagesize; //mblock; // kod_obj->
			if (moff + mcopylen > msize) mcopylen = msize - moff;
	
			
//#ifdef SHOW_INFO
			LogMessage("lxexec: fread file 0x%lx len 0x%lx -> vmem: 0x%lx at shared 0x%lx\n",
					   fstart + moff, mcopylen, mstart + moff, mpageaddr);
//#endif

			fseek(lx_exe_mod->fh, fstart + moff, SEEK_SET);

			if (fread((void*)mpageaddr, 1, mcopylen, lx_exe_mod->fh) != mcopylen) {
				LogMessage("lxexecf: could not read from file.");
				return ; //L4_nilthread;
			}

			IF_MEMORY_FreeSharedRegion(pagerid, &newthread, mstart + moff, &mfpage, &env);
			if (env._major != CORBA_NO_EXCEPTION)
			{
				LogMessage("elfexec: pager call FreeSharedRegion failed, code: %d", CORBA_exception_id(&env));
				return ; //L4_nilthread;
			}

			// trying to work-around a bug in L4 which happens after numerous
			// mapping of different size on the same region
			//L4_Flush(L4_Fpage(mpageaddr, mblock) + L4_FullyAccessible);
			//printf("work-around a bug in L4... mpageaddr=%p, mblock=%p\n", 
			//		mpageaddr, mblock);
		

		// the following .bss section is automaticly zeroed by the pager
	}

	//fclose(lx_exe_mod->fh);

	// kick start task
	char * /*const*/ argv[] = { NULL }; 
	char * /*const*/ envp[] = { NULL }; //{ "STDIN","STDOUT","STDERR" };
	// create an environment for the program
	//char **progenviron = dupenviron();
	
	printf("STDIN=%s\n",getenv("STDIN"));
	// strcpy(&envp[0][0], getenv("STDIN"));
    // strcpy(&envp[0][1], getenv("STDOUT"));
    // strcpy(&envp[0][2], getenv("STDERR"));
	//sleep(3);
	 
	esp = makenewstack(pagerid, newthread, argv, envp, stack_obj);
	//esp = makenewstack(pagerid, newthread, argv, progenviron, stack_obj);
	if (esp == 0x00000000) {
		panic("lxexec: invalid stack frame");
	}
	unsigned int i_esp = esp;
	//i_esp += stack_obj->o32_base;
	//i_esp -= 0x27000;  
	// Funkar inte: 163816 159720 131048 126952 73704 
	// OK för: 28648 32744 16360
	// Sidfel inne i LX-processen vid: 28644 32740
	printf(" %d:IF_MEMORY_StartTask(pagerid=%x, newthread=0x%x, eip=%lu, esp=%lu, &env=%p)\n",
			 __LINE__, pagerid, &newthread, eip, i_esp, &env);
	//IF_MEMORY_StartTask(pagerid, &newthread, ehdr.e_entry, esp, &env);
	IF_MEMORY_StartTask(pagerid, &newthread, eip, i_esp, &env);
	if (env._major != CORBA_NO_EXCEPTION)
	{
		LogMessage("lxexec: pager call StartTask failed, code: %d", CORBA_exception_id(&env));
		return ; //L4_nilthread;
	}
	printf("                       exec_lx(), Ok. \n\n");
	return ; //newthread;
}









// Test of to load doscalls from an elf file.


// the main difficulty while reading this function is that there are two
// address spaces involved: the current one and the target one. the stack in
// the current address space in a shared page, in the destination the stack
// begins at 0xC0000000
static L4_Word_t elf_makenewstack(L4_ThreadId_t pagerid, L4_ThreadId_t newthread,
							  const char *const argv[], const char *const envp[])
{
	typedef L4_Word_t addr_t;

	// 16 kb maximum environment and arguments. raise if needed. using this
	// limit saves from doing argument wrappings on fpage boundaries
	static const unsigned int maxenvstacksize = 0x8000;

	static addr_t stackend = 0xC0000000;

	// request the stack fpage from the pager
	CORBA_Environment env (idl4_default_environment);

	if (L4_Pager() != pagerid) {
		// set receive window: this is evil as it has to take some
		// arbitrary position in memory
		idl4_set_rcv_window(&env, L4_Fpage(0x90000000, maxenvstacksize));
	} else {
		// much nicer: let our pager decide where to place the shared page
		idl4_set_rcv_window(&env, L4_CompleteAddressSpace);
	}

	idl4_fpage_t stackpage;
	memset(&stackpage, 0, sizeof(stackpage));

	IF_MEMORY_GetSharedRegion(pagerid,
							  &newthread,
							  stackend - maxenvstacksize,
							  maxenvstacksize,
							  &stackpage,
							  &env);

	if (env._major != CORBA_NO_EXCEPTION)
	{
		LogMessage("elfexec: pager call GetSharedRegion failed for stack, code: %d", CORBA_exception_id(&env));
		return 0x00000000;
	}
	if (L4_IsNilFpage(idl4_fpage_get_page(stackpage)))
	{
		LogMessage("elfexec: pager call GetSharedRegion return nilpage.");
		return 0x00000000;
	}

	L4_Word_t stackpageaddr = 0x90000000;

	if (L4_Pager() == pagerid) {
		stackpageaddr = idl4_fpage_get_base(stackpage);
	}

	// ** construct stack

	if (!argv) {
		static const char* argvnull[] = { NULL };
		argv = argvnull;
	}
	if (!envp) {
		static const char* envpnull[] = { NULL };
		envp = envpnull;
	}

	addr_t top = 0xC0000000;

	// evil: heavily depends on wrapping of unsigned ints
	char *stack = reinterpret_cast<char*>(stackpageaddr) + maxenvstacksize - top;

	// count arguments and environment
	unsigned int argvnum = 0;
	while(argv[argvnum]) argvnum++;

	unsigned int envpnum = 0;
	while(envp[envpnum]) envpnum++;

	// allocate buffers for string pointers on stack
	addr_t argvptr[argvnum+1];
	addr_t envpptr[envpnum+1];

	// put a zero word at the bottom of the stack
	top -= sizeof(uint32_t);
	*(uint32_t*)(stack + top) = 0x00000000;

	// put the environment variables on the stack
	for(unsigned int ei = 0; envp[ei]; ei++)
	{
		unsigned int el = strlen(envp[ei]) + 1; // with null char

		top -= el;
		memcpy(stack + top, envp[ei], el);

		envpptr[ei] = top;
	}

	envpptr[envpnum] = (addr_t)NULL;

	// put command line parameters on the stack
	for(unsigned int ai = 0; argv[ai]; ai++)
	{
		unsigned int al = strlen(argv[ai]) + 1;
		
		top -= al;
		memcpy(stack + top, argv[ai], al);

		argvptr[ai] = top;
	}

	argvptr[argvnum] = (addr_t)NULL;

	// align stack pointer to 4 bytes
	top -= ((unsigned int)top % 4);

	// add pointer arrays for environment and argv
	top -= sizeof(char*) * (envpnum+1);
	memcpy(stack + top, envpptr, sizeof(char*) * (envpnum+1));
	addr_t envpstart = top;

	top -= sizeof(char*) * (argvnum+1);
	memcpy(stack + top, argvptr, sizeof(char*) * (argvnum+1));
	addr_t argvstart = top;
	
	// construct main()'s parameters:

	top -= sizeof(char*);
	*(addr_t*)(stack+top) = envpstart;		// envp

	top -= sizeof(char*);
	*(addr_t*)(stack+top) = argvstart;		// argv

	top -= sizeof(int);
	*(int*)(stack+top) = argvnum;			// argc;

	// return stack fpage

	L4_Fpage_t stackfpage = L4_Fpage(stackpageaddr, maxenvstacksize);
	IF_MEMORY_FreeSharedRegion(pagerid, &newthread, stackend - maxenvstacksize, &stackfpage, &env);
	if (env._major != CORBA_NO_EXCEPTION)
	{
		LogMessage("elfexec: pager call FreeSharedRegion failed, code: %d", CORBA_exception_id(&env));
		return 0x00000000;
	}

	// trying to work-around a bug in L4 which happens after numerous mapping
	// of different size on the same region
	L4_Flush(L4_Fpage(stackpageaddr, maxenvstacksize) + L4_FullyAccessible);

	// return esp of new task
	return top;
}

static bool check_elfheader_so(const Elf32_Ehdr* ehdr, unsigned int datalen)
{
	// check magic bytes
	if ((ehdr->e_ident[EI_MAG0] !=  ELFMAG0) || 
		(ehdr->e_ident[EI_MAG1] !=  ELFMAG1) || 
		(ehdr->e_ident[EI_MAG2] !=  ELFMAG2) ||
		(ehdr->e_ident[EI_MAG3] !=  ELFMAG3))
	{
		LogMessage("elfexec: invalid elf header: wrong magic bytes.\n");
		return false;
	}
	if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
		LogMessage("elfexec: invalid elf header: not 32 bit image.\n");
		return false;
	}
	if (ehdr->e_ident[EI_VERSION] != EV_CURRENT) {
		LogMessage("elfexec: invalid elf header: invalid elf version.\n");
		return false;
	}

	// check file type
	/*if (ehdr->e_type != ET_EXEC) {
		LogMessage("elfexec: invalid elf header: not an executable elf binary.\n");
		return false;
	}*/
	if (ehdr->e_type != ET_DYN) {
		LogMessage("elfexec: invalid elf header: not an shared object elf binary.\n");
		return false;
	}
	if (ehdr->e_machine != EM_386) {
		LogMessage("elfexec: invalid elf header: does not contain Intel 80386 code.\n");
		return false;
	}
	if (ehdr->e_version != EV_CURRENT) {
		LogMessage("elfexec: invalid elf header: invalid elf version.\n");
		return false;
	}
	if (ehdr->e_flags != 0) {
		LogMessage("elfexec: invalid elf header: unknown processor flags.\n");
		return false;
	}

#ifdef SHOW_INFO
	LogMessage("elfexec: Entry point: 0x%x", ehdr->e_entry);
#endif

	if (ehdr->e_phoff + (ehdr->e_phnum * sizeof(Elf32_Phdr)) > datalen) {
		LogMessage("elfexec: invalid elf file: program header table is beyond file end.\n");
		return false;
	}

	if (ehdr->e_phnum == 0) {
		LogMessage("elfexec: invalid elf file: no program header in file.\n");
		return false;
	}

	return true;
}



L4_ThreadId_t elfexecf(L4_ThreadId_t pagerid,
					   const char* path,
					   const char *const argv[], const char *const envp[],
					   L4_ThreadId_t newthread)
{
	FILE* fimage = fopen(path, "r");
	if (!fimage) {
		LogMessage("elfexecf: invalid path name: file does not exist?\n");
		return L4_nilthread;
	}

	Elf32_Ehdr ehdr;

	if (fread(&ehdr, 1, sizeof(ehdr), fimage) != sizeof(ehdr)) {
		LogMessage("elfexecf: invalid elf header: file too short\n");
		return L4_nilthread;
	}

	fseek(fimage, 0, SEEK_END);
	unsigned int fsize = ftell(fimage);

	if (!check_elfheader_so(&ehdr, fsize))
		return L4_nilthread;

	// *** create new task

	if(L4_IsNilThread(pagerid)) {
		pagerid = L4_Pager();
	}

	CORBA_Environment env (idl4_default_environment);

	//L4_ThreadId_t newthread = L4_nilthread;

	/* Don't create a thread for doscalls.elf, just use the previous task created for 
		the LX program we just loaded.
	  
	IF_MEMORY_CreateTask(pagerid, &newthread, &env);
	if (env._major != CORBA_NO_EXCEPTION)
	{
		LogMessage("elfexec: pager call CreateTask failed, code: %d", CORBA_exception_id(&env));
		return L4_nilthread;
	}
	*/

	// load each program header's data into a page

	for (int phi = 0; phi < ehdr.e_phnum; phi++)
	{
		Elf32_Phdr phdr;

		fseek(fimage, ehdr.e_phoff + phi * sizeof(phdr), SEEK_SET);
		
		if (fread(&phdr, 1, sizeof(phdr), fimage) != sizeof(phdr)) {
			LogMessage("elfexecf: invalid elf header: file too short\n");
		    return L4_nilthread;
		}

		if (phdr.p_type != PT_LOAD) continue;

		if(phdr.p_memsz == 0)
			continue;

		// figure out the start of the program image in the file
		L4_Word_t fstart = phdr.p_offset;

		// the start of the image in virtual memory
		L4_Word_t mstart = phdr.p_vaddr;

		// the size of the fpage we are going to fill, not the size of the
		// program image
		L4_Word_t msize = phdr.p_filesz;
		
		// load the image blockwise and try to align blocks to this modulo
		const L4_Word_t mblock = 0x8000;

		if ((mstart & (mblock - 1)) != 0) {
			LogMessage("elf warning: program image (0x%lx) is not aligned to 0x%lx bytes",
					   mstart, mblock);
			panic("elfexec: program image is not aligned to n bytes");
		}

		for(L4_Word_t moff = 0; moff < msize; moff += mblock)
		{
			// request a shared fpage from the pager

			if (L4_Pager() != pagerid) {
				// set receive window: this is evil as it has to take some
				// arbitrary position in memory
				idl4_set_rcv_window(&env, L4_Fpage(0x90000000, mblock));
			} else {
				// much nicer: let our pager decide where to place the shared page
				idl4_set_rcv_window(&env, L4_CompleteAddressSpace);
			}
		
			idl4_fpage_t mpage;
			memset(&mpage, 0, sizeof(mpage));

			IF_MEMORY_GetSharedRegion(pagerid, &newthread, mstart + moff, mblock, &mpage, &env);

			if (env._major != CORBA_NO_EXCEPTION)
			{
				LogMessage("elfexec: pager call GetSharedRegion failed, code: %d", CORBA_exception_id(&env));
				return L4_nilthread;
			}
			if (L4_IsNilFpage(idl4_fpage_get_page(mpage)))
			{
				LogMessage("elfexec: pager call GetSharedRegion return nilpage.");
				return L4_nilthread;
			}

			L4_Word_t mpageaddr = (L4_Pager() == pagerid) ? idl4_fpage_get_base(mpage) : 0x90000000;
			L4_Fpage_t mfpage = L4_Fpage(mpageaddr, mblock);

			L4_Word_t mcopylen = mblock;
			if (moff + mcopylen > msize) mcopylen = msize - moff;

#ifdef SHOW_INFO
			LogMessage("elfexec: fread file 0x%lx len 0x%lx -> vmem: 0x%lx at shared 0x%lx\n",
					   fstart + moff, mcopylen, mstart + moff, mpageaddr);
#endif

			fseek(fimage, fstart + moff, SEEK_SET);

			if (fread((void*)mpageaddr, 1, mcopylen, fimage) != mcopylen) {
				LogMessage("elfexecf: could not read from file.");
				return L4_nilthread;
			}

			IF_MEMORY_FreeSharedRegion(pagerid, &newthread, mstart + moff, &mfpage, &env);
			if (env._major != CORBA_NO_EXCEPTION)
			{
				LogMessage("elfexec: pager call FreeSharedRegion failed, code: %d", CORBA_exception_id(&env));
				return L4_nilthread;
			}

			// trying to work-around a bug in L4 which happens after numerous
			// mapping of different size on the same region
			L4_Flush(L4_Fpage(mpageaddr, mblock) + L4_FullyAccessible);
		}

		// the following .bss section is automaticly zeroed by the pager
	}

	fclose(fimage);

	// kick start task

	L4_Word_t esp = elf_makenewstack(pagerid, newthread, argv, envp);
	if (esp == 0x00000000) {
		panic("elfexec: invalid stack frame");
	}

	/* 
	IF_MEMORY_StartTask(pagerid, &newthread, ehdr.e_entry, esp, &env);
	if (env._major != CORBA_NO_EXCEPTION)
	{
		LogMessage("elfexec: pager call StartTask failed, code: %d", CORBA_exception_id(&env));
		return L4_nilthread;
	}
	*/

	return newthread;
}

/*L4_ThreadId_t elfexecf(L4_ThreadId_t pagerid,
					   const char* path,
					   const char *const argv[], const char *const envp[],
					   L4_ThreadId_t newthread); */
					   
int test_elfexecf(const char* path) {
	L4_ThreadId_t t,i;
	t.raw = 10;
	i.raw = 6;
	L4_ThreadId_t ret = elfexecf(t, path, 0, 0, i);
	return ret.raw;
}

#endif
/* SDIOS */


/*
// elfexec.cc

*/

	
/*
To install the handler, do the following: 

        struct sigaction act; 

        act.sa_sigaction = handle_fault; 
        sigemptyset(&act.sa_mask); 
        act.sa_flags = SA_SIGINFO | SA_RESTART; 
        sigaction(SIGSEGV, &act, NULL);
}

*/


/*  www.iecc.com_linker_linker08.html
Basic loading
We touched on most of the basics of loading in Chapter 3, in the context of 
object file design. Loading is a little different depending on whether a program 
is loaded by mapping into a process address space via the virtual memory system 
or just read in using normal I/O calls. 


On most modern systems, each program is loaded into a fresh address space, which 
means that all programs are loaded at a known fixed address, and can be linked 
for that address. In that case, loading is pretty simple: 

	Read enough header information from the object file to find out how much address 
	space is needed.
	 
	Allocate that address space, in separate segments if the object format has 
	separate segments. 
	
	Read the program into the segments in the address space. 
	
	Zero out any bss space at the end of the program if the virtual memory system 
	doesn't do so automatically. 
	
	Create a stack segment if the architecture needs one. 
	
	Set up any runtime information such as program arguments or environment variables. 
	
	Start the program. 
	
If the program isn't mapped through the virtual memory system, reading in the object 
file just means reading in the file with normal "read" system calls. On systems which 
support shared read-only code segments, the system needs to check whether there's 
already a copy of the code segment loaded in and use that rather than making another copy.

*/
