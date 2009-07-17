/*
 *
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>		// From the "Developer Connection Device Driver Kit" version 2.0

#include <mb_info.h>
#include "serial.h"
#include "struct.h"

//#include <string.h>
#include <ifs.h>

#define CHECKRC \
    if (rc) \
    {	    \
      kprintf("MFSH_PHYSTOVIRT() failed, @%s, line #%lu\n", \
	      __FILE__, __LINE__); \
      return 1; \
    }

int serial_init (long port, long speed,
		int word_len, int parity, int stop_bit_len);

extern char FS_NAME[12];
extern struct dpb far *pdpb; // pointer to our DPB
extern unsigned short hVPB;
extern int open_files;
extern unsigned long mbi;

unsigned short FlatR0CS;
unsigned short FlatR0DS;

char debug = 0;

char mount_name[] = "FS_MOUNT";
char open_name[]  = "FS_OPENCREATE";

#pragma pack(1)

struct save_item
{
  struct cdfsi far *pcdfsi;
  struct cdfsd far *pcdfsd;
  struct sffsi far *psffsi;
  struct sffsd far *psffsd;
  unsigned long ulOpenMode;
  unsigned short usOpenFlag;
  char pName[0x3a];
};

#pragma pack()

//char far *boot;
char boot[0x800];
char save_map[0x80]; // open files indicate area
struct save_item save_area[0x80];
struct save_item *save_pos;
int save_index = 0;

extern struct devcaps devcaps;
extern char volchars[0x20];
extern struct vpfsi far *pVPfsi;
extern struct vpfsd far *pVPfsd;

#pragma pack(1)

struct p48
{
  unsigned long  off;
  unsigned short sel;
};

#pragma pack()

int _cdecl _loadds GetFlatSelectors(void);
int _cdecl _loadds GetProcAddr(char far *fpszModName,
			       unsigned short cchModName,
			       char far *fpszProcName,
			       unsigned char cchProcName,
			       struct p48 far *fp48EP);
int _cdecl _loadds GetModule(char far *fpszModName,
			     unsigned short cchModName,
			     struct p48 far *fpMte48);

unsigned char drvletter = 0;
unsigned char cd_drvletter = 0;

struct multiboot_info far *mbi_far;
struct mod_list far *mods_addr;
unsigned long mods_count;
unsigned short sel;
unsigned short sel1;
unsigned short mods_sel;

unsigned long filemax;
unsigned long filepos;
unsigned long filebase;
char far *fileaddr;

// our command line
char cmdline[0x400];

#pragma aux mbi 	     "*"
#pragma aux FS_NAME	     "*"
#pragma aux FlatR0CS	     "*"
#pragma aux FlatR0DS	     "*"
#pragma aux GetFlatSelectors "*"
#pragma aux GetProcAddr      "*"
#pragma aux GetModule	     "*"

void far *fmemset (void far *start, int c, int len);
int fstrlen (const char far *str);
void far *fmemmove (void far *_to, const void far *_from, int _len);
char far *fstrcpy (char far *dest, const char far *src);
int toupper (int c);
int isspace (int c);
char far *strip(char far *s);
char far *fstrstr (const char far *s1, const char far *s2);
char *strstr (const char *s1, const char *s2);
char *skip_to (int after_equal, char *cmdline);
int safe_parse_maxint (char **str_ptr, long *myint_ptr);
int strlen (const char *str);
int strcmp (const char *s1, const char *s2);
char *strcpy (char *dest, const char *src);
void *memmove (void *_to, const void *_from, int _len);
void *memset (void *start, int c, int len);

int far pascal MFSH_INTERR(char far *pcMsg, unsigned short cbMsg);
int far pascal MFSH_SETBOOTDRIVE(unsigned short usDrive);
int far pascal MFSH_PHYSTOVIRT(unsigned long ulAddr,
			       unsigned short usLen,
			       unsigned short far *pusSel);
int far pascal MFSH_UNPHYSTOVIRT(unsigned short usSel);
void far pascal FSH_GETVOLPARM(unsigned short hVPB,
			       struct vpfsi far * far *ppVPBfsi,
			       struct vpfsd far * far *ppVPBfsd);
void far pascal FSH_INTERR(char far *pMsg,
			   unsigned short cbMsg);
int far pascal FSH_DOVOLIO(unsigned short operation,
			   unsigned short fAllowed,
			   unsigned short hVPB,
			   char far *pData,
			   unsigned short far *pcSec,
			   unsigned long iSec);

typedef int far pascal (*mount_t)(unsigned short flag,
				  struct vpfsi far *pvpfsi,
				  struct vpfsd far *pvpfsd,
				  unsigned short hVPB,
				  char far *pBoot);
typedef int far pascal (*open_t)(struct cdfsi far *pcdfsi,
				 struct cdfsd far *pcdfsd,
				 char far *pName,
				 unsigned short iCurDirEnd,
				 struct sffsi far *psffsi,
				 struct sffsd far *psffsd,
				 unsigned long ulOpenMode,
				 unsigned short usOpenFlag,
				 unsigned short far *pusAction,
				 unsigned short usAttr,
				 char far *pcEABuf,
				 unsigned short far *pfgenflag);

void serout(char *s);
int kprintf(const char *format, ...);

// refresh fileaddr value with
// filepos change
void advance_ptr(void)
{
    MFSH_UNPHYSTOVIRT(sel1);

    // convert file read point phys addr to char far *
    MFSH_PHYSTOVIRT(filebase + filepos, 0xffff, &sel1);
    fileaddr = (char far *)MAKEP(sel1, 0);
}

int far pascal _loadds MFS_CHGFILEPTR(
    long  offset,			/* offset	*/
    unsigned short type 		/* type 	*/
)
{
    kprintf("**** MFS_CHGFILEPTR\n");

    switch (type)
    {
    case 0:
      filepos = offset;
      break;
    case 1:
      filepos += offset;
      break;
    case 2:
      filepos = filemax + offset;
      break;
    default:
      return 1;
    }
    advance_ptr();

    return NO_ERROR;
}

int far pascal _loadds MFS_INIT(
    void far *bootdata, 		/* bootdata	*/
    char far *number,			/* number io	*/
    long far *vectorripl,		/* vectorripl	*/
    void far *bpb,			/* bpb		*/
    unsigned long far *pMiniFSD,	/* pMiniFSD	*/
    unsigned long far *dump		/* dump address */
) {
    int i, rc;
    struct mod_list far *mod;
    unsigned short sl, sl1;
    char far *p, far *q;
    long port = 0x3f8;
    long speed = 9600;
    char *pp, *r;
    char panic_msg[] = "MBI:mbi uninitialized, panic!\n";

    // mbi as RIPL data
    //mbi = *((unsigned long far *)bootdata);

    //__asm {
    //	int 3
    //}

    if (mbi == 0xffffffff)
      MFSH_INTERR(panic_msg, strlen(panic_msg));

    // get GDT selector to mbi structure
    rc = MFSH_PHYSTOVIRT(mbi, sizeof(struct multiboot_info), &sel);
    CHECKRC
    mbi_far = (struct multiboot_info far *)MAKEP(sel, 0);

    rc = MFSH_PHYSTOVIRT(mbi_far->cmdline, 0xffff, &sl1);
    CHECKRC
    q = (char far *)MAKEP(sl1, 0);
    fmemmove(cmdline, q, fstrlen(q));
    MFSH_UNPHYSTOVIRT(sl1);

    if (mbi_far->flags & MB_INFO_CMDLINE)
    {
      if (pp = strstr(cmdline, "--debug"))
      {
	debug = 1;
      }

      // if "--serial=..." specified on the command line
      if (pp = strstr(cmdline, "--port"))
      {
	pp = skip_to(1, pp);
	safe_parse_maxint(&pp, &port);
      }

      if (pp = strstr(cmdline, "--speed"))
      {
	pp = skip_to(1, pp);
	safe_parse_maxint(&pp, &speed);
      }

      if (pp = strstr(cmdline, "--ifs"))
      {
	pp = skip_to(1, pp);
	// find name end
	for (r = pp; *r && *r != ' '; r++) ;
	memmove(FS_NAME, pp, r - pp);
	FS_NAME[r - pp] = '\0';
	// make FS_NAME uppercase
	r = FS_NAME;
	while (*r) *r++ = toupper(*r);
      }

      if (pp = strstr(cmdline, "--hd"))
      {
	pp = skip_to(1, pp);
	drvletter = toupper(pp[0]);
      }

      if (pp = strstr(cmdline, "--cd"))
      {
	pp = skip_to(1, pp);
	cd_drvletter = toupper(pp[0]);
      }
    }

    // init serial port
    serial_init(port, speed, UART_8BITS_WORD, UART_NO_PARITY, UART_1_STOP_BIT);

    kprintf("**** MFS_INIT\n");
    kprintf("Hello MBI minifsd!\n");
    kprintf("comport = 0x%x\n", port);
    kprintf("ifs: %s\n", FS_NAME);
    kprintf("drive letter: %c\n", drvletter);
    kprintf("RIPL data address: 0x%04x:0x%04x\n",
	    ((unsigned long)bootdata >> 16),
	    ((unsigned long)bootdata & 0xffff));
    kprintf("mbi phys. address: 0x%08lx\n", mbi);
    kprintf("allocated mbi selector: 0x%04x\n", sel);
    kprintf("mem_lower = %lu Kb, mem_upper = %lu Mb\n",
	    mbi_far->mem_lower, mbi_far->mem_upper >> 10);
    kprintf("boot_device = 0x%08lx\n", mbi_far->boot_device);
    kprintf("kernel cmdline @0x%08lx\n", mbi_far->cmdline);
    kprintf("cmdline = %s\n", cmdline);
    kprintf("mods_count = %lu, mods_addr = 0x%08lx\n",
	    mbi_far->mods_count, mbi_far->mods_addr);
    kprintf("syms = 0x%lx\n", mbi_far->syms);
    kprintf("mmap_length = 0x%lu, mmap_addr = 0x%08lx\n",
	    mbi_far->mmap_length, mbi_far->mmap_addr);
    kprintf("drives_length = 0x%lu, drives_addr = 0x%08lx\n",
	    mbi_far->drives_length, mbi_far->drives_addr);
    kprintf("config_table = 0x%08lx\n", mbi_far->config_table);
    kprintf("boot_loader_name @0x%08lx\n",
	    mbi_far->boot_loader_name);

    rc = MFSH_PHYSTOVIRT(mbi_far->mods_addr, sizeof(struct mod_list) * mbi_far->mods_count, &mods_sel);
    CHECKRC
    kprintf("allocated mods selector: 0x%04x\n", mods_sel);
    mods_addr  = MAKEP(mods_sel, 0);
    mods_count = mbi_far->mods_count;
    mod = (struct mod_list far *)mods_addr;
    kprintf("mods cmd lines: \n");

    for (i = 0; i < mods_count; i++, mod++)
    {
      rc = MFSH_PHYSTOVIRT(mod->cmdline, 0xffff, &sl);
      CHECKRC
      p = MAKEP(sl, 0);
      kprintf("%u: %s\n", i, p);
      MFSH_UNPHYSTOVIRT(sl);
    }

    rc = MFSH_SETBOOTDRIVE(drvletter - 'A'); // u:
    kprintf("MFSH_SETBOOTDRIVE() returned: 0x%x\n", rc);

    return NO_ERROR;
}

int far pascal _loadds MFS_OPEN(
    char far *name,			/* name 	*/
    long far *size	       /* size	       */
)
{
    char far buf1[0x100];
    char far buf2[0x100];
    struct mod_list far *mod;
    char far *p, far *q, far *l;
    char far *s;
    int  n, rc;

    kprintf("**** MFS_OPEN(\"%s\")", name);

    fmemset(buf1, 0, sizeof(buf1));
    fmemset(buf2, 0, sizeof(buf2));

    // get GDT selector to mbi structure
    rc = MFSH_PHYSTOVIRT(mbi, sizeof(struct multiboot_info), &sel);
    CHECKRC
    mbi_far = (struct multiboot_info far *)MAKEP(sel, 0);

    // get mods selector
    rc = MFSH_PHYSTOVIRT(mbi_far->mods_addr, sizeof(struct mod_list) * mbi_far->mods_count, &mods_sel);
    CHECKRC
    mods_addr  = MAKEP(mods_sel, 0);

    if (mbi_far->flags & MB_INFO_MODS) // If there are modules
    {
      mod = mods_addr;

      // search for a given filename
      for (n = 0; n < mods_count; n++)
      {
	// convert module cmdline phys addr to char far *
	rc = MFSH_PHYSTOVIRT(mod->cmdline, 0xffff, &sel1);
	CHECKRC
	s = (char far *)MAKEP(sel1, 0);

	// copy to buffers
	fstrcpy(buf1, s);
	fstrcpy(buf2, name);

	// deallocate a selector
	MFSH_UNPHYSTOVIRT(sel1);

	// translate '/' to '\' in command line
	for (p = buf1; *p; p++) if (*p == '/') *p = '\\';

	// make it uppercase
	for (p = buf1; *p; p++) *p = toupper(*p);
	for (p = buf2; *p; p++) *p = toupper(*p);

	p = strip(buf1); q = strip(buf2);

	if (*p == '(')
	{
	  while (*p && *p != ')') p++;
	  p++;
	}

	if (!strcmp(p, q))   // with starting '\\'
	    break;

	if (*p == '\\') p++;

	if (!strcmp(p, q))   // without starting '\\'
	    break;

	//l = strstr(p, q); /* (cd)/os2/boot/bvhvga.dll bvhvga; (hd0,0)0+0x200 *bootsec* */
	//if (l && ((p + strlen(p)) == (l + strlen(q))))
	//    break;

	mod++;
      };

      // we have gone through all mods, and no given filename
      if (n == mods_count)
      {
	kprintf(" failed!\n");
	return ERROR_FILE_NOT_FOUND;
      }

      // filename found
      filepos  = 0;
      filemax  = mod->mod_end - mod->mod_start;

      // convert module cmdline phys addr to char far *
      rc = MFSH_PHYSTOVIRT(mod->mod_start, 0xffff, &sel1);
      CHECKRC
      s = (char far *)MAKEP(sel1, 0);

      fileaddr = s;
      filebase = mod->mod_start;

      *size = filemax;
      kprintf(" succeeded\n");
      return NO_ERROR;
    }

  kprintf(" failed!\n");
  return ERROR_FILE_NOT_FOUND;
}

int far pascal _loadds MFS_READ(
    char far *data,		/* data 	*/
    unsigned short far *length	 /* length	 */
)
{
  kprintf("**** MFS_READ(0x%04x:0x%04x, ",
	  (unsigned long)data >> 16,
	  (unsigned long)data & 0xffff);
  kprintf("%u)\n", *length);

  if (fileaddr && data && *length)
  {
    fmemmove(data, fileaddr, *length);
    filepos += *length;
  }
  else
    return 1;

  advance_ptr();

  return NO_ERROR;
}

int far pascal _loadds MFS_CLOSE(void) {
    kprintf("**** MFS_CLOSE\n");

    if (sel1)	  MFSH_UNPHYSTOVIRT(sel1);
    if (mods_sel) MFSH_UNPHYSTOVIRT(mods_sel);
    if (sel)	  MFSH_UNPHYSTOVIRT(sel);

    return NO_ERROR;
}

int far pascal _loadds MFS_TERM(void)
{
  struct p48 p48;
  open_t  p_open  = 0;
  mount_t p_mount = 0;
  unsigned char drv;
  struct vpfsi far *pvpfsi = 0;
  struct vpfsd far *pvpfsd = 0;
  int rc;
  unsigned short usAction;
  unsigned short flags;
  char msg_erropen[] = "Error reopening file!\n";
  struct cdfsi cdfsi;
  char far *pBoot = 0;

  kprintf("**** MFS_TERM\n");
  kprintf("hello stage3!\n");
  // Get FLAT selectors
  if (!GetFlatSelectors())
    kprintf("FlatR0CS = 0x%x, FlatR0DS = 0x%x\n", FlatR0CS, FlatR0DS);
  else
    kprintf("Flat selectors not found.\n");

  // Get FS_MOUNT address
  if (!GetProcAddr(FS_NAME,
	      strlen(FS_NAME),
	      mount_name,
	      strlen(mount_name),
	      &p48))
  {
    *((unsigned short *)&p_mount) = (unsigned short)p48.off;
    *((unsigned short *)(&p_mount) + 1) = p48.sel;
    kprintf("IFS FS_MOUNT addr: 0x%04x:0x%08lx\n", p48.sel, p48.off);
  }
  else
    kprintf("IFS FS_MOUNT addr not found.\n");

  // Get FS_OPENCREATE address
  if (!GetProcAddr(FS_NAME,
	      strlen(FS_NAME),
	      open_name,
	      strlen(open_name),
	      &p48))
  {
    *((unsigned short *)&p_open) = (unsigned short)p48.off;
    *((unsigned short *)(&p_open) + 1) = p48.sel;
    kprintf("IFS FS_OPENCREATE addr: 0x%04x:0x%08lx\n", p48.sel, p48.off);
  }
  else
  {
    kprintf("IFS FS_OPENCREATE addr not found.\n");
    __asm {
      int 3
    }
  }

  kprintf("p_mount = 0x%08lx\n", p_mount);
  kprintf("p_open  = 0x%08lx\n", p_open);

  kprintf("Our hVPB: 0x%04x\n", hVPB);
  FSH_GETVOLPARM(hVPB, &pvpfsi, &pvpfsd);
  pdpb = (struct dpb far *)pvpfsi->vpi_hDEV;

  drv = drvletter;
  if (drv != cd_drvletter) drv = cd_drvletter;

  // search for boot drive hVPB in DPB structures
  while (pdpb) {
    kprintf("pdpb = 0x%08lx\n", pdpb);
    hVPB = pdpb->dpb_hVPB;
    kprintf("drive: %c:, hVPB: 0x%04x\n", 'a' + pdpb->dpb_drive, hVPB);
    if ('A' + pdpb->dpb_drive == drv) break;
    pdpb = pdpb->dpb_next_dpb;
  };

  if (drvletter == 'A' + pdpb->dpb_drive)
  {
    pvpfsi->vpi_pDCS = &devcaps;
    pvpfsi->vpi_pVCS = &volchars;
    pBoot = boot;
  }

  kprintf("vpi_pDCS = 0x%08lx, vpi_pVCS = 0x%08lx\n",
	  pvpfsi->vpi_pDCS,
	  pvpfsi->vpi_pVCS);

  // call the FS_MOUNT entry point of the IFS:
  rc = (*p_mount)(0, pvpfsi, pvpfsd, hVPB, pBoot);
  kprintf("FS_MOUNT rc = %u\n", rc);
  if (rc)
    __asm{
      int 3
    };

  // reopen by an IFS all files open by minifsd
  for (save_index = 0; save_index < 0x80; save_index++)
  {
    if (save_map[save_index]) // if a file is open
    {
      save_pos = save_area + save_index;
      // zero out cdfsi buffer
      memset(&cdfsi, 0, sizeof(struct cdfsi));
      cdfsi.cdi_hVPB = hVPB;
      if(!(rc = (*p_open)(&cdfsi,
			  save_pos->pcdfsd,
			  save_pos->pName,
			  0,
			  save_pos->psffsi,
			  save_pos->psffsd,
			  save_pos->ulOpenMode,
			  save_pos->usOpenFlag,
			  &usAction,		 //dummy address
			  0,
			  0,
			  &flags)))
	kprintf("FS_OPENCREATE(\"%s\") = %u\n", save_pos->pName, rc);
      else
      {
	kprintf("FS_OPENCREATE(\"%s\") failed, rc = %u\n", save_pos->pName, rc);
	__asm {
	  int 3
	}
	FSH_INTERR(msg_erropen, strlen(msg_erropen));
      }
    }
  }

  return NO_ERROR;
}
