/* Structure containing some environment options. For more information*
 * see http://www.os2world.com/goran/cfgtool.htm for example.         */
typedef struct {
        short int autofail;     // YES or NO (default)
        short int buffers;      // 3-100
        short int clockscale;   // 1,2,4
        short int closefiles;   // TRUE
        char codepage[7];       // ppp,sss (Prim. and sec. codepage)
        char * country;         // =XXX,drive:\OS2\SYSTEM\COUNTRY.SYS
        char * devinfo_kbd;     // =KBD,XX,drive:,\OS2\KEYBOARD.DCP
        char * devinfo_vio;     // =SCR,VGA,drive:\OS2\BOOT\VIOTBL.DCP
        char * diskcache;       // =D,LW,32,AC:CD+E
        short int dllbasing;            // ON (default) or OFF
        char dumpprocess;       // = drive
        short int earlymeminit; // =TRUE (I think we won't need in in osFree)
        short int fakeiss;      // =YES (has something to do with drivers, so we won't need it)
        short int i13pages;     // =1
        char * ibm_java_options;// =-Dmysysprop1=tcpip -Dmysysprop2=wait
        char * iopl;            // YES (default) or NO (I would like to see NO as default in osFree...) or a comma seperated list of programms allowed to directly work with hardware
        char * iropt;           // =value
        short int javanofpk;    // =YES; deals with fixpacks, therefore I think we need to set it to YES (which disables fixpacks in a case)
        char lastdrive;         // =drive
        short int ldrstackopt;  // FALSE or TRUE (default) (I don't think we need this setting)
        char * libpath;         // =.;C:\.... (Syntax like PATH)
        short int maxwait;      // 1 to 255, default is 3; only has an effect if PRIORITY=DYNAMIC
        char * memman;          // =s,m,PROTECT,COMMIT where s=SWAP or NOSWAP; m=MOVE or NOMOVE (is provided for OS/2 1.x compatibility; do we need it?!?)
        char * mode[20];        // can exist multiple times => types?!?
        int mode_id;            // helper variable
        short int ndwtimer;     // =OFF
        short int pauseonerror; // YES(default) or NO
        char * printmonbufsize; // =n,n,n (in byte)
        short int priority;     // DYNAMIC(default=1) or ABSOLUTE(=0)
        short int priority_disk_io;     // YES (default) or NO
        short int protectonly;  // YES or NO; do we need it?
        char * protshell;       // =drive:\OS2\PMSHELL.EXE
        short int raskdata;     // =OTE; I think we don't need to have it
        short int reipl;                // ON or OFF (default)
        char remote_install_state; // =X
        char reservedriveletter;// =driveletter
        char * setboot;         // configures OS/2 bootmanager => I think we can ignore it
        char * sourcepath;      // =path
        char * strace;          // several possible?!?
        char suppresspopups;    // 0 or driveletter
        char * swappath;        // =drive,path,mmm(in kb),nnn (in kb)
        short int sxfakehwfpu;  // =1 (do we need it?)
        char * sysdump;         // nothing or /NOPROMPT if set
        int threads;            // =n; number of threads; isn't this handled in the kernel? do we need it?
        char * timeslice;       // =x,y (in milliseconds)
        char * trace;           // =ON, OFF, n
        int tracebuf;           // =n (inkb)
        char * trapdump;        // =value
        char * traplog;         // =value
        short int truemode;     // =1
        int virtualaddresslimit;// =n (in kb)
        short int vme;          // =NO
        short int workplace_native;     // =0
        short int workplace_primary_cp; // =1
        short int workplace_process;    // =NO
        int wp_objhandle;       // =n
        short int debugmodmgr;     // =NO
        short int debugixfmgr;     // =NO
        short int debugprcmgr;     // =NO
        short int debugmemmgr;     // =NO
        char * configfile;     // =C:\CONFIG.SYS
        char * bootdrive;     // =C:
} cfg_opts;

extern cfg_opts options;

struct name
{
  unsigned line;
  char *string;
};

struct types {
        const char *name;       // Name of config.sys command
        const char sep; // Not used so far, perhaps I will delete it
        struct name *sp;              // Pointer to the content
        int ip;         // Number of elements in the arrays
};

extern struct types type[];

// Function prototypes
unsigned long CfgInitOptions(void);
unsigned long CfgParseLine(char line[], int len, int lineno);
unsigned long CfgCleanup(void);
unsigned long CfgParseConfig(void * addr, int size);
unsigned long CfgGetenv(char *name, char **value);
