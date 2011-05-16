
#include <l4/x86emu/x86emu.h>

#include <l4/log/l4log.h>
#include <l4/l4con/l4con.h>
#include <l4/l4con/l4con-client.h>

#include <l4/sys/types.h>
#include <l4/sys/ipc.h>
#include <l4/sys/syscalls.h>
#include <l4/sys/kdebug.h>
//#include <l4/rmgr/librmgr.h>
#include <l4/util/rdtsc.h>
#include <l4/names/libnames.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include <l4/l4con/l4contxt.h>



void
printk(const char *format,...)
{
  va_list list;
  va_start(list, format);
  vprintf(format, list);
  va_end(list);
}

#define TEST

// Emulated memory
static unsigned char emu_mem [1024*1024];

static u8   X86API my_inb(X86EMU_pioAddr addr);
static u16  X86API my_inw(X86EMU_pioAddr addr);
static u32  X86API my_inl(X86EMU_pioAddr addr);
static void X86API my_outb(X86EMU_pioAddr addr, u8 val);
static void X86API my_outw(X86EMU_pioAddr addr, u16 val);
static void X86API my_outl(X86EMU_pioAddr addr, u32 val);
static u8   X86API my_rdb(u32 addr);
static u16  X86API my_rdw(u32 addr);
static u32  X86API my_rdl(u32 addr);
static void X86API my_wrb(u32 addr, u8 val);
static void X86API my_wrw(u32 addr, u16 val);
static void X86API my_wrl(u32 addr, u32 val);
static void X86API VDM_int(int num);


static u8
X86API my_inb(X86EMU_pioAddr addr)
{
  int r;
  asm volatile ("inb %w1, %b0" : "=a" (r) : "d" (addr));
  return r;
}

static u16
X86API my_inw(X86EMU_pioAddr addr)
{
  u16 r;
  asm volatile ("inw %w1, %w0" : "=a" (r) : "d" (addr));
  return r;
}

static u32
X86API my_inl(X86EMU_pioAddr addr)
{
  u32 r;
  asm volatile ("inl %w1, %0" : "=a" (r) : "d" (addr));
  return r;
}

static void
X86API my_outb(X86EMU_pioAddr addr, u8 val)
{
//  printf("%04x:%04x outb %x -> %x\n", M.x86.R_CS, M.x86.R_IP, val, addr);
  asm volatile ("outb %b0, %w1" : "=a" (val), "=d" (addr)
                                : "a" (val), "d" (addr));
}

static void
X86API my_outw(X86EMU_pioAddr addr, u16 val)
{
//  printf("%04x:%04x outw %x -> %x\n", M.x86.R_CS, M.x86.R_IP, val, addr);
  asm volatile ("outw %w0, %w1" : "=a" (val), "=d" (addr)
                                : "a" (val), "d" (addr));
}

static void
X86API my_outl(X86EMU_pioAddr addr, u32 val)
{
//  printf("%04x:%04x outl %x -> %x\n", M.x86.R_CS, M.x86.R_IP, val, addr);
  asm volatile ("outl %0, %w1" : "=a"(val), "=d" (addr) 
                               : "a" (val), "d" (addr));
}

static u8
X86API my_rdb(u32 addr)
{
  return *(u32*)(M.mem_base + addr);
}

static u16
X86API my_rdw(u32 addr)
{
  return *(u16*)(M.mem_base + addr);
}

static u32
X86API my_rdl(u32 addr)
{
  return *(u32*)(M.mem_base + addr);
}

static void 
X86API my_wrb(u32 addr, u8 val)
{
  *(u8*)(M.mem_base + addr) = val;
}

static void 
X86API my_wrw(u32 addr, u16 val)
{
  *(u16*)(M.mem_base + addr) = val;
}

static void 
X86API my_wrl(u32 addr, u32 val)
{
  *(u32*)(M.mem_base + addr) = val;
}

// Int 10 Handler
static void 
X86API VDM_int_10(void)
{
//  LOG("INT 10h AH=%02x\n", M.x86.R_AH);
  if (M.x86.R_AH==0x0e) printf("%c", M.x86.R_AL);
};

// Int 11 Handler
static void 
X86API VDM_int_11(void)
{
  M.x86.R_AX=1; // One floppy
};

// Int 12 Handler
static void 
X86API VDM_int_12(void)
{
  M.x86.R_AX=1024; // Size of memory in 1k blocks
};

// Int 13 Handler
static void 
X86API VDM_int_13(void)
{
  LOG("%02x", M.x86.R_AH);
  if (M.x86.R_AH==0) // reset drive
  {
    return;
  }
  else if (M.x86.R_AH==8) // get geometry
  {
    LOG("DL=%02x", M.x86.R_DL);
    if (M.x86.R_DL<0x80) // floppy
    {
      M.x86.R_BL=4;
      M.x86.R_DL=1;
      return;
    } else {
      M.x86.R_FLG=FB_CF; // error
      M.x86.R_DL=0;
    }
    return;
  }
  else if (M.x86.R_AH==0x15) // get disk type
  {
    if (M.x86.R_DL<0x80)
    {
      M.x86.R_AH=2; // diskette with changing logic
    } else
    {
      M.x86.R_AH=0; // no device
    }
    return;
  }
  else if (M.x86.R_AH==0x41) // get extended disk support
  {
    M.x86.R_FLG=FB_CF; // Return 'interface not present'
//    exit(1);
    return;
  }
  
  exit(1);
};

// Int 16 Handler
static void 
X86API VDM_int_16(void)
{
  LOG("%02x", M.x86.R_AH);
  if (M.x86.R_AH==1)
  {
    M.x86.R_FLG=FB_ZF; // Return 'key not ready'
    return;
  }
  exit(1);
};

int bcd2dec(int b)
{
  return ((b >> 4) & 0x0f) * 10 + (b & 0x0f);
}

int dec2bcd(int d)
{
  return ((d / 10) << 4) | (d % 10);
}


// Int 1a Handler
static void 
X86API VDM_int_1a(void)
{
  struct tm *t;
  time_t t1;
  LOG("%02x", M.x86.R_AH);
  if (M.x86.R_AH==0) // getticks
  {
    return;
  }
  else if (M.x86.R_AH==1) // setticks
  {
    return;
  }
  else if (M.x86.R_AH==2) // gettime
  {
    t1=time(NULL);
    t=localtime(&t1);
    M.x86.R_CH=dec2bcd(t->tm_hour);
    M.x86.R_CL=dec2bcd(t->tm_min);
    M.x86.R_BH=dec2bcd(t->tm_sec);
    return;
  }
  else if (M.x86.R_AH==3) // settime
  {
    return;
  }
  else if (M.x86.R_AH==4) // getdate
  {
    t1=time(NULL);
    t=localtime(&t1);
    M.x86.R_CH=dec2bcd(19+(t->tm_year/100));
    M.x86.R_CL=dec2bcd(t->tm_year-100*(19+(t->tm_year/100)));
    M.x86.R_DH=dec2bcd(t->tm_mon);
    M.x86.R_DL=dec2bcd(t->tm_mday);
    return;
  }
  else if (M.x86.R_AH==5) // setdate
  {
    return;
  }

  exit(1);
};

// Interrupts handler
static void 
X86API VDM_int(int num)
{
  if (num!=0x10) LOG("INT %02x", num);
  if (num==0x10) VDM_int_10();
  else if (num==0x11) VDM_int_11();
  else if (num==0x12) VDM_int_12();
  else if (num==0x13) VDM_int_13();
  else if (num==0x16) VDM_int_16();
  else if (num==0x1a) VDM_int_1a();
  else
  exit(1);
};

X86EMU_pioFuncs my_pioFuncs =
{
  my_inb,
  my_inw,
  my_inl,
  my_outb,
  my_outw,
  my_outl
};

X86EMU_memFuncs my_memFuncs =
{
  my_rdb,
  my_rdw,
  my_rdl,
  my_wrb,
  my_wrw,
  my_wrl
};

X86EMU_intrFuncs VDM_int_table[256]=
{
  VDM_int,  // 00
  VDM_int,  // 01
  VDM_int,  // 02
  VDM_int,  // 03
  VDM_int,  // 04
  VDM_int,  // 05
  VDM_int,  // 06
  VDM_int,  // 07
  VDM_int,  // 08
  VDM_int,  // 09
  VDM_int,  // 0a
  VDM_int,  // 0b
  VDM_int,  // 0c
  VDM_int,  // 0d
  VDM_int,  // 0e
  VDM_int,  // 0f
  VDM_int,  // 10
  VDM_int,  // 11
  VDM_int,  // 12
  VDM_int,  // 13
  VDM_int,  // 14
  VDM_int,  // 15
  VDM_int,  // 16
  VDM_int,  // 17
  VDM_int,  // 18
  VDM_int,  // 19
  VDM_int,  // 1a
  VDM_int,  // 1b
  VDM_int,  // 1c
  VDM_int,  // 1d
  VDM_int,  // 1e
  VDM_int,  // 1f
  VDM_int,  // 20
  NULL,     // 21
  VDM_int,  // 22
  VDM_int,  // 23
  VDM_int,  // 24
  VDM_int,  // 25
  VDM_int,  // 26
  VDM_int,  // 27
  VDM_int,  // 28
  NULL,     // 29
  NULL,     // 2a
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int,
  VDM_int
};

int 
main(int argc, char **argv)
{
  int error;
  l4_umword_t address;
  FILE *f=0;
  unsigned long size;
#ifdef TEST
  l4_cpu_time_t start, stop;
#endif
LOG("1");
  l4_sleep(5000);
  contxt_init(65536, 11);

  //  rmgr_init();
  l4_calibrate_tsc();

  //emu_mem = (((l4_umword_t)&_end)+2*L4_PAGESIZE-1) & L4_PAGEMASK;
  
  LOG("emu_mem = %08x\n", &emu_mem);

  /* build 16-bit address space starting at emu_mem */

  LOG("4\n");

  f = fopen("/file/system/kernel.sys", "rb");
  LOG("file opened");
  if(f) {
     fseek(f, 0, SEEK_END);
     size = ftell(f);  /* Extract the size of the file and reads it into a buffer.*/
     rewind(f);
     fread(&emu_mem[0x600], size, 1, f);
     fclose(f);
     LOG("successful return");
     LOG("%02x", emu_mem[0x600]);
  }

  M.mem_base = emu_mem;
  M.mem_size = 1024*1024;
  M.x86.debug = 0;
  
  LOG("1\n");
  X86EMU_setupPioFuncs(&my_pioFuncs);
  LOG("2\n");
  X86EMU_setupMemFuncs(&my_memFuncs);
  LOG("3\n");
  X86EMU_setupIntrFuncs(VDM_int_table);
  LOG("4\n");

#ifdef TEST

  M.x86.R_AX = 0x4F02;
  M.x86.R_BX = 0x114;
  M.x86.R_SP = L4_PAGESIZE;
  M.x86.R_IP = 0;
  M.x86.R_CS = 0x60;
  M.x86.R_DS = M.x86.R_CS;
  M.x86.R_ES = M.x86.R_CS;
  M.x86.R_SS = M.x86.R_CS;

  LOG("Starting emulator\n");

  start = l4_rdtsc();
  X86EMU_exec();
  stop  = l4_rdtsc();

  stop -= start;
  
  LOG("Stopping emulator (time was %d ms)\n",
      ((unsigned int)l4_tsc_to_ns(stop))/1000000);

#else

    {
      l4_threadid_t sender;
      l4_msgdope_t result;
      struct
	{
	  l4_fpage_t fp;
	  l4_msgdope_t size_dope;
	  l4_msgdope_t send_dope;
	  l4_umword_t dw[12];
	} msg;

#if 0
      if (!names_register("X86EMU"))
	{
	  printf("Error registering at names\n");
	  exit(-1);
	}
#endif
	  
      for (;;)
	{
	  msg.size_dope = L4_IPC_DOPE(12, 0);
	  msg.send_dope = L4_IPC_DOPE(12, 0);
  
	  error = l4_ipc_wait(&sender, 
			      &msg, &msg.dw[0], &msg.dw[1],
			      L4_IPC_NEVER, &result);

	  while (!error)
	    {
	      M.x86.R_EAX = msg.dw[0];
	      M.x86.R_EBX = msg.dw[1];
	      M.x86.R_ECX = msg.dw[2];
	      M.x86.R_EDX = msg.dw[3];
	      M.x86.R_ESI = msg.dw[4];
	      M.x86.R_EDI = msg.dw[5];
	      M.x86.R_EBP = msg.dw[6];
	      M.x86.R_EIP = msg.dw[7];
	      M.x86.R_EFLG = msg.dw[8];
	  
	      M.x86.R_CS = msg.dw[9];
	      M.x86.R_DS = msg.dw[10];
	      M.x86.R_ES = msg.dw[11];
	  
	      M.x86.R_ESP = L4_PAGESIZE;
	      M.x86.R_SS = 0x0100;
  
	      printf("Starting emulator:\n"
		     "eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx\n"
		     "esi=%08lx edi=%08lx ebp=%08lx esp=%08lx\n"
		     "eip=%08lx eflags=%08lx\n"
		     "cs=%04x ds=%04x es=%04x ss=%04x\n",
		     M.x86.R_EAX, M.x86.R_EBX, M.x86.R_ECX, M.x86.R_EDX,
		     M.x86.R_ESI, M.x86.R_EDI, M.x86.R_EBP, M.x86.R_ESP,
		     M.x86.R_EIP, M.x86.R_EFLG,
		     M.x86.R_CS, M.x86.R_DS, M.x86.R_ES, M.x86.R_SS);

	      enter_kdebug("stop");

#if 0
	      start = l4_rdtsc();
	      X86EMU_exec();
	      stop  = l4_rdtsc();

	      stop -= start;
  
	      printf("Stopping emulator (time was %d ms)\n",
		  ((unsigned int)l4_tsc_to_ns(stop))/1000000);
#endif

	      msg.dw[0] = M.x86.R_EAX;
	      msg.dw[1] = M.x86.R_EBX;
	      msg.dw[2] = M.x86.R_ECX;
	      msg.dw[3] = M.x86.R_EDX;
	      msg.dw[4] = M.x86.R_ESI;
	      msg.dw[5] = M.x86.R_EDI;
	      msg.dw[6] = M.x86.R_EBP;
	      msg.dw[7] = M.x86.R_EIP;
	      msg.dw[8] = M.x86.R_EFLG;
	  
	      msg.dw[9] = M.x86.R_CS;
	      msg.dw[10] = M.x86.R_DS;
	      msg.dw[11] = M.x86.R_ES;
	      
	      error = l4_ipc_reply_and_wait(sender, 
					    &msg, msg.dw[0], msg.dw[1],
					    &sender,
					    &msg, &msg.dw[0], &msg.dw[1],
					    L4_IPC_SEND_TIMEOUT_0,
					    &result);
	    }
	}
    }
  
#endif
  
  return 0;
}

