/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef REGINA_TYPES_H_INCLUDED
#define REGINA_TYPES_H_INCLUDED

typedef struct varbox *variableptr ;
typedef const struct varbox *cvariableptr ;

/*
 * var_hashtable is a table with size entries. The entries are added according
 * to their hash value.
 * r is the number of read accesses.
 * w is the number of write accesses.
 * c is the number of collisions on access or while creating/copying the table.
 * e is the number of true elements in the tbl, hashed to size buckets.
 * Note that tbl contains one more hidden element used for variable
 * maintenance if size > 0.
 */
typedef struct {
   variableptr    *tbl;
   unsigned        reads;
   unsigned        writes;
   unsigned        collisions;
   unsigned        elements;
   unsigned        size;
} var_hashtable;

typedef struct varbox {
   var_hashtable *index;
   variableptr next, prev, realbox ;
   struct strengtype *name, *value ;
   int guard ;
   num_descr *num ;
   int flag ;
   unsigned hash ;
   long hwired, valid ; /* FGC: at least valid may be too small for many
                                recursions with short instead of long.
                                27.09.98 (09/27/98). */
   variableptr stem ;
} variable ;

/* typedef int bool ; */

typedef struct {
   unsigned int lnum:1 ;    /* left side of comparison is a number or string constant */
   unsigned int rnum:1 ;    /* right side of comparison is a number or string constant */
   unsigned int lsvar:1 ;   /* left side of comparison is a simple variable */
   unsigned int rsvar:1 ;   /* right side of comparison is a simple variable */
   unsigned int lcvar:1 ;   /* left side of comparison is a compound variable */
   unsigned int rcvar:1 ;   /* right side of comparison is a compound variable */
} compflags ;

typedef enum {
   awtUNKNOWN = 0,
   awtSTREAM = 1,
   awtSTEM = 2,
   awtLIFO = 3,
   awtFIFO = 4
} AddressWithType;

typedef enum {
   antUNKNOWN = 0,
   antSTRING = 1,
   antSIMSYMBOL = 2
} AddressNameType;

typedef enum {
   fpdRETAIN = 0,
   fpdCLEAR = 1
} FilePtrDisposition;

typedef struct {
   unsigned int append:1;
   unsigned int isinput:1;
   unsigned int iserror:1;
   unsigned int awt:3; /* overlay with AddressWithType */
   unsigned int ant:2; /* overlay with AddressNameType */
   unsigned int noeol:1; /* for INPUT NOEOL */
} outputflags; /* used by ADDRESS WITH resourceo */

typedef enum {
   PROTECTED_DelayedScriptExit,
   PROTECTED_DelayedInterpreterExit,
   PROTECTED_DelayedRexxSignal
} delayed_error_type_t;

typedef enum {
   QisUnused,
   QisSESSION,
   QisInternal,
   QisExternal,
   QisTemp
} queue_type_t ;

typedef struct  {
   time_t sec ;
   time_t usec ;
} rexx_time;

struct _tsd_t; /* If this won't work change "struct _tsd_t *" to "void *"
                * below. This will require more changes. Let your compiler
                * choose the places.
                */

typedef struct pparambox *paramboxptr ;
typedef const struct pparambox *cparamboxptr ;
typedef struct pparambox {
   paramboxptr next ;
   int dealloc ;
   struct strengtype *value ;
} parambox ;

typedef struct tnode {
   unsigned int type ;
   int charnr, lineno ;
   union {                 /* saves memory which is really needed          */
      int called;          /* used during execution *and* don't need init  */
      struct tnode *last;  /* used during parsing                          */
   } o ;
   struct strengtype *name ;
   rexx_time *now ;
   struct tnode *p[4] ;
   union {
      /*
       * WARNING: When changing check whether to modify dumptree and the
       *          whole instore stuff. Seek for X_CEXPRLIST.
       */
      streng *(*func)(struct _tsd_t *,cparamboxptr) ;
      streng *strng ;
      struct tnode *node ;
      num_descr *number ;
      compflags flags ;
      variable *varbx ;
      outputflags of ;
      int parseflags ;
      int nonansi ;
      int trace_only ; /* For labels */
   } u ;
   struct tnode *next ;
   unsigned long nodeindex ; /* for an effectiv relocation, never change! */
} treenode ;
typedef struct tnode *nodeptr ;
typedef const struct tnode *cnodeptr ;

typedef struct lineboxx *lineboxptr ;
typedef const struct lineboxx *clineboxptr ;
typedef struct lineboxx {
   lineboxptr next, prev ;
   struct strengtype *line ;
   int lineno ;
} linebox ;

typedef struct labelboxx *labelboxptr ;
typedef const struct labelboxx *clabelboxptr ;
typedef struct labelboxx {
   labelboxptr next ;
   unsigned long hash ;
   nodeptr entry ;
} labelbox ;

typedef struct trap_type  /* index is type of signal */
{
   unsigned int on_off:1 ;     /* true if trap is active/on */
/* unsigned int trapped:1 ; */ /* true if condition has been raised */
   unsigned int def_act:1 ;    /* true if default action is to ignore */
   unsigned int delayed:1 ;    /* true if trap is in delay mode */
   unsigned int ignored:1 ;    /* true if ignored when in delayed mode */
   unsigned int invoked:1 ;    /* true if invoked by SIGNAL */
   streng *name ;          /* label to transfer control to */
} trap ;

typedef struct sig_type
{
   int type ;
   streng *info ;
   streng *descr ;
   int invoke ;     /* is true if invoked with SIGNAL */
   int rc, subrc, lineno ;
} sigtype ;

typedef void (*signal_handler)(int);

typedef struct __regina_option
{
   char *name ;
   int offset ;
   char *contains ;
} option_type ;

typedef struct _StackLine {
   /* A stack line is a double linked list element of a streng. The streng
    * never contains a line end and is never NULL.
    * A read operations will happen at the top end, as well as a LIFO
    * operation.
    * A FIFO operation will happen at the bottom end.
    */
   struct _StackLine *higher ;
   struct _StackLine *lower ;
   streng *contents ;
} StackLine ;

typedef struct _Buffer {
   /* A buffer is a double linked list of stack line bundles.
    * MAKEBUF() adds a new buffer at the newest end.
    */
   struct _Buffer *higher ;
   struct _Buffer *lower ;

   /* The "content" of the buffer. See StackLine for a description.
    * For a faster response of QUEUED(), we count the elements of lines.
    */
   StackLine *top ;
   StackLine *bottom ;
   unsigned elements ;
} Buffer ;

typedef struct {
   /* A queue is one of different implementations of queue types.
    * See the picture and the description at the beginning of stack.c.
    */
   queue_type_t type;
   union {
      struct { /* internal or SESSION: i */
         /* name is the uppercased name of the queue. In rare cases this
          * may be NULL for QisSESSION after initialisation.
          */
         streng *name ;
         /*
          * Indicates if the queue is a "real" queue
          * or a false queue as a result of a rxqueue('set', 'qname')
          * on a queue that doesn't exist. This is crap behaviour!
          * but that's how Object Rexx works :-(
          */
         int isReal ;
         /*
          * Content: We have a double linked list of buffers. See Buffer
          * for a description.
          */
         Buffer *top ;
         Buffer *bottom ;

         /* This is the count of buffers including the zeroth buffer */
         unsigned buffers ;

         /* This is the overall count of elements in all buffers */
         unsigned elements ;
      } i ;
      struct { /* external: e */
         /*
          * The port number for the current connection to an external rxstack
          * Valid values: 1..0xFFFF, unused: 0
          */
         int portno;
         /*
          * The socket fd for the current connection to an external rxstack
          * Valid values: not -1, unused/error: -1
          */
         int socket;
         /*
          * The server address of the current connection. Used to determine if
          * we need to disconnect from one rxstack server and connect to
          * another.
          * The true data type is a 32 bit system depending type.
          * Valid values: not 0, unused: 0
          */
         int address;
         /*
          * A boolean value which indicates if the external queue has had a timeout
          * set on it via RXQUEUE( 'Timeout' )
          * Valid values: 0 or 1
          */
         int timeoutSet;
         streng *name;
      } e ;
      Buffer t ; /* temp: t */
   } u ;
} Queue ;

typedef struct { /* one for each redirection in environment */
   streng      *name;     /* name if any, but not expanded      */
   outputflags  flags;
   streng      *base;     /* "number" if name is a stem         */
   streng      *currname; /* expanded name + ".number" if name  */
                          /* is a stem                          */
   int          currnamelen;  /* len(currname) without ".number"*/
   int          currnum;  /* current number for a stem position */
                          /* or -1 if unknown                   */
   int          maxnum;   /* maximum number for a stem position */
                          /* or -1 if unknown                   */
   void        *file;     /* fileboxptr of the file with the    */
                          /* above name or NULL.                */
   Queue       *queue;    /* queue with the above name or NULL. */
   Queue       *tmp_queue;/* temporary queue for the redirection*/
                          /* or NULL without this helper.       */
   unsigned int SameAsOutput:2;   /* locally used in shell.c    */
   unsigned int FileRedirected:1; /* locally used in shell.c    */
   char        *tempname; /* locally used filename in shell.c   */
   int          type;     /* locally used source in shell.c     */
   int          hdls[3];  /* locally used connection in shell.c */
} environpart;

typedef struct {
   struct strengtype *name; /* stemname or streamname if any */
   int subtype;             /* SUBENVIR_... */
   int subcomed;            /* has this environment been redirected to an API program via RexxRegistreSubcom???() */
   environpart input;
   environpart output;
   environpart error;
} environment;

typedef struct proclevelbox *proclevel ;
typedef const struct proclevelbox *cproclevel ;
typedef struct proclevelbox {
   int numfuzz, currnumsize, numform ;
   int mathtype ;
   rexx_time rx_time ;
   proclevel prev, next ;
   var_hashtable *vars ;
   paramboxptr args ;
   struct strengtype *environment, *prev_env ;
   char tracestat, traceint, varflag ; /* MDW 30012002 */
   sigtype *sig ;
   trap *traps ;
   jmp_buf *signal_continue; /* see jump_rexx_signal() */
   unsigned long options;
   int pool;
} proclevbox ;

typedef struct _ttree { /* bucket list of treenodes which allows ultra fast
                         * loading of instore macros.
                         */
   struct _ttree *next;
   unsigned long  max; /* maximum number of elements in the bucket */
   unsigned long  num; /* current number of elements in the bucket */
   unsigned long  sum; /* sum of indices until element 0 */
   treenode *     elems;
} ttree; /* treenode type */

typedef struct { /* offsrcline: offset based source lines */
   unsigned long length;
   unsigned long offset;
   /* That's all. You need the source string of the incore macro to create
    * a real sourceline using this information.
    */
} offsrcline;

typedef struct _otree { /* bucket list of offscrlines which allows ultra fast
                         * loading of instore macros.
                         */
   struct _otree *next;
   unsigned long  max; /* maximum number of elements in the bucket */
   unsigned long  num; /* current number of elements in the bucket */
   unsigned long  sum; /* sum of indices until element 0 */
   offsrcline *   elems;
} otree; /* offsrcline type */

typedef struct { /* internal_parser_type is a structure containing data from a
                  * parsing operation.
                  */
   lineboxptr     first_source_line; /* Either this two values  */
   lineboxptr     last_source_line ; /* exist or srclines below */
   int            tline;             /* line number where error occurred */
   int            tstart;            /* column number where error occurred */
   int            if_linenr;         /* line number of last IF keyword */
   int            when_linenr;       /* line number of last WHEN keyword */
   int            select_linenr;     /* line number of last SELECT keyword */
   labelboxptr    first_label;
   labelboxptr    last_label;
   unsigned long  numlabels;
   labelboxptr    sort_labels ;
   int            result;
   nodeptr        root;
   ttree *        nodes;
   otree *        srclines;          /* Either this two values exist */
   const char *   incore_source;     /* or the two values above      */
   streng *       kill; /* Probably the true source of incore_source in case
                         * of an "INTERPRET" instruction or other not user
                         * generated but interpreted strings; else NULL.
                         */
   struct _tsd_t *TSD; /* needed during the parsing step */
} internal_parser_type;

typedef struct { /* extstring: external (instore) string */
   unsigned long length;
   /* and directly following the string's content */
} extstring;

typedef struct { /* external_parser_type: the instore macro in user space */
   /* Never change from here until source including since it allows the
    * reconstruction of the source if the machine type or Regina version
    * doesn't match.
    */
   char     Magic[32]; /* "Regina's Internal Format\r\n" filled with 0 */
#define MAGIC "Regina's Internal Format\r\n"
   char     ReginaVersion[64]; /* PARSE_VERSION_STRING */

   /* The following structure allows the detection of different
    * architectures. We don't want to try to decode something from
    * a 64 bit big endian encoded parsing tree on an i586 for
    * example.
    */
   union {
      char ignore[4 * 256/8]; /* Allow 256 bit machines */
      struct {
         unsigned long one; /* value one */
         unsigned long two; /* value two */
         void *   ptr3; /* value (void*)3, size may be different to unsigned */
         void *   ptr4; /* value (void*)4 */
      } s;
   } arch_detector;

   unsigned long OverallSize; /* in byte of this structure and all dependencies */

   unsigned long version; /* INSTORE_VERSION */

   /* We describe the sourcelines first */
   unsigned long NumberOfSourceLines;
   unsigned long source;
   /* Offset to table of source lines. Imagine a value of 1000 and
    * 5 source lines (previous value). This structure has an address
    * of 500 in the memory (what you get from malloc or something else).
    * Then: The table has 5 entries at position 1500 in memory.
    * Each table entry is an offsrcline structure. The source string is the
    * instore[0] string of RexxStart or another source string.
    */

   unsigned long NumberOfTreeElements;
   unsigned long TreeStart; /* Within 0 .. (NumberOfTreeElements-1) */
   unsigned long tree;
   /* Offset to table of nodes. Imagine a value of 2000 and
    * 6 elements (NumberIfTreeElements). This structure has an address
    * of 500 in the memory (what you get from malloc or something else).
    * Then: The table has 6 treenodes at position 2500 in memory.
    * Each table entry (treenode) must be relocated. Every nodeptr within a
    * element is just an index within this table. If node->p[2] of one picked
    * table entry is ((nodeptr) 4) then the address is calculated as follows:
    * 500 + tree + (4*sizeof(treenode))
    * This value should be assigned to node->p[2] to use the value in its
    * normal manner.
    * A NULL value is represented by (nodeptr) (unsigned) -1.
    * Every string within a treenode is relocated by adding 500. Imagine a
    * value of 3000 for node->name. Then an extstring structure is located
    * at 3500 in memory which represents the string's content.
    */
} external_parser_type;

typedef struct systeminfobox *sysinfo ;
typedef const struct systeminfobox *csysinfo ;
typedef struct systeminfobox {
   struct strengtype *input_file ; /* must be 0-terminated without counting of the '\0' */
   streng *environment ;
   FILE *input_fp;
   int tracing ;
   int interactive ;
   jmp_buf *script_exit;     /* see jump_script_exit() */
   streng *result ;
   proclevbox *currlevel0 ;
   struct systeminfobox *previous ;
   nodeptr *callstack ;
   int cstackcnt, cstackmax ;
   int hooks ;
   int invoked ;
   int trace_override;
   internal_parser_type tree;
   int ctrlcounter;
} sysinfobox ;

#ifndef DONT_TYPEDEF_PFN
typedef unsigned long (*PFN)() ;
#endif


struct library {
   streng *name;
   void *handle;
   unsigned long used;

   struct library *next, *prev;
};


struct entry_point {
   streng *name;
   PFN addr;
   union {
      void *gci_info;             /* for function handlers */
      unsigned char user_area[8]; /* for suncommand and exit handlers */
   } special;

   unsigned long hash;
   struct library *lib;
   struct entry_point *next, *prev;
};

/*
 * Every major OS has some functionality that isn't shared with others.
 * Most functions relate to the command execution/redirection stuff.
 * The global function collection *OS_Dep will point to this structure.
 * Some systems, Win9x and OS/2-EMX, even use the DOS entry table because
 * they can't take advantage of the performance functions the system
 * usually provides.
 * init() may map some "stupid" function on advanced function, as for
 * instance Win9x doesn't support redirection correctly.
 */
struct _tsd_t;
struct regina_utsname;
typedef struct _OS_Dep_funcs {
   void   (*init)                       (void);
   int    (*setenv_exec)                (const char *name, const char *value);
   int    (*fork_exec)                  (struct _tsd_t *TSD, environment *env, const char *cmdline, int *rc);
   int    (*wait_exec)                  (int process);
   int    (*open_subprocess_connection) (const struct _tsd_t *TSD, environpart *ep);
   void   (*unblock_handle)             (int *handle, void *async_info);
   void   (*restart_file)               (int hdl);
   int    (*close_exec)                 (int handle, void *async_info);
   void   (*close_special)              (int handle);
   int    (*read_exec)                  (int hdl, void *buf, unsigned size, void *async_info) ;
   int    (*write_exec)                 (int hdl, const void *buf, unsigned size, void *async_info);
   void*  (*create_async_info)          (const struct _tsd_t *TSD);
   void   (*delete_async_info)          (void *async_info);
   void   (*reset_async_info)           (void *async_info);
   void   (*add_async_waiter)           (void *async_info, int handle, int add_as_read_handle);
   void   (*wait_async_info)            (void *async_info);
   int    (*uname_exec)                 (struct regina_utsname *name);
} OS_Dep_funcs;

#include "regina64.h"


#endif /* REGINA_TYPES_H_INCLUDED */
