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

/* JH 20-10-99 */  /* To make Direct setting of stems Direct and not Symbolic. */


/*
 * Routines in lexsrc.l
 */
#ifndef FLEX_SCANNER
   extern YY_CHAR_TYPE *yysptr, yysbuf[] ;
#endif
   void fetch_file(tsd_t *TSD,
                   FILE *fptr,
                   internal_parser_type *ipt);
   void fetch_string(tsd_t *TSD,
                     const streng *str,
                     internal_parser_type *ipt);

/*
 * Routines in parsing.c
 */
   int bmstrstr( const streng *heystack, int start, const streng *needle, int caseless ) ;
   void doparse( tsd_t *TSD, const streng *source, cnodeptr subtree, int caseless ) ;
   void parseargtree(tsd_t *TSD, cparamboxptr argbox, cnodeptr thisptr, int flags );


/*
 * Routines in debug.c
 */
   void dumpvars( const tsd_t *TSD );
   void dumptree( const tsd_t *TSD, const treenode *thisptr, int level, int newline ) ;
   streng *getsourceline( const tsd_t *TSD, int line, int charnr, const internal_parser_type *ipt ) ;
#ifdef TRACEMEM
   void marksource( clineboxptr ) ;
#endif


/*
 * Routines in files.c
 */
   streng *readkbdline( tsd_t *TSD ) ;
   void mark_filetable( const tsd_t *TSD) ;
   int filename_cmp( const streng *name1, const streng *name2 );
   void purge_filetable( tsd_t *TSD ) ;
   int init_filetable( tsd_t *TSD ) ;
   void fixup_file( tsd_t *TSD, const streng * ) ;
   streng *get_external_routine( const tsd_t *TSD, const char *inname,
                                 FILE **fp );
   void find_shared_library(const tsd_t *TSD,const char *, const char *, char *);
   void CloseOpenFiles ( const tsd_t *TSD, FilePtrDisposition fpd ) ;
   streng *ConfigStreamQualified( tsd_t *TSD, const streng *name );
   streng *std_charin( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_charout( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_chars( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_linein( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_lineout( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_lines( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_stream( tsd_t *TSD, cparamboxptr parms ) ;
   void closefile( tsd_t *TSD, const streng *name )  ;
   void *addr_reopen_file( tsd_t *TSD, const streng *filename, char code, int iserror );
   streng *addr_io_file( tsd_t *TSD, void *fileptr, const streng *line ) ;
   void addr_reset_file( tsd_t *TSD, void *fileptr );
   streng *addr_file_info( tsd_t *TSD, const streng *source, int defchannel );
#ifndef NDEBUG
   streng *dbg_dumpfiles( tsd_t *TSD, cparamboxptr parms ) ;
#endif
   streng *unx_open( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_close( tsd_t *TSD, cparamboxptr parms ) ;
#if !defined(HAVE__SPLITPATH2) && !defined(HAVE__SPLITPATH) && !defined(__EMX__) && !defined(DJGPP)
   int my_splitpath2( const char *in, char *out, char **drive, char **dir, char **name, char **ext ) ;
#endif
   int my_fullpath( tsd_t *TSD, char *dst, const char *src );
   int my_fullpathstreng( const tsd_t *TSD, char *dst, const streng *src );
   streng *arexx_exists( tsd_t *TSD, cparamboxptr parms ) ;

/*
 * Routines in expr.c
 */
   int isboolean( tsd_t *TSD, nodeptr, int suberror, const char *op ) ;
   num_descr *calcul( tsd_t *TSD, nodeptr, num_descr**, int leftorright, int oper ) ;
   int init_expr( tsd_t *TSD ) ;
   streng *evaluate( tsd_t *TSD, nodeptr thisptr, streng **kill ) ;

/*
 * Routines in cmsfuncs.c
 */
   streng *cms_sleep( tsd_t *TSD, cparamboxptr parms ) ;
   streng *cms_makebuf( tsd_t *TSD, cparamboxptr parms ) ;
   streng *cms_justify( tsd_t *TSD, cparamboxptr parms ) ;
   streng *cms_find( tsd_t *TSD, cparamboxptr parms ) ;
   streng *cms_index( tsd_t *TSD, cparamboxptr parms ) ;
   streng *cms_desbuf( tsd_t *TSD, cparamboxptr parms ) ;
   streng *cms_buftype( tsd_t *TSD, cparamboxptr parms ) ;
   streng *cms_dropbuf( tsd_t *TSD, cparamboxptr parms ) ;
   streng *cms_state( tsd_t *TSD, cparamboxptr parms ) ;

/*
 * Routines in memory.c
 */
   int init_memory( tsd_t *TSD ) ;
#ifdef FLISTS
   void *get_a_chunk( int bytes ) ;
   void *get_a_chunkTSD( const tsd_t *TSD, int bytes ) ;
   streng *get_a_streng( int bytes ) ;
   streng *get_a_strengTSD( const tsd_t *TSD, int bytes ) ;
   void  give_a_chunk( void *ptr ) ;
   void  give_a_chunkTSD( const tsd_t *TSD, void *ptr ) ;
   void  give_a_streng( streng *ptr ) ;
   void  give_a_strengTSD( const tsd_t *TSD, streng *ptr ) ;
   int show_free_lists(const tsd_t *TSD);
   void purge_flists(const tsd_t *TSD);
#endif
#ifdef TRACEMEM
   void *mymalloc( int bytes ) ;
   void *mymallocTSD( const tsd_t *TSD, int bytes ) ;
   void  myfree( void *ptr ) ;
   void  myfreeTSD( const tsd_t *TSD, void *ptr ) ;
   void  regmarker( const tsd_t *TSD, void (*func)(const tsd_t *TSD) ) ;
   int   have_allocated( tsd_t *TSD, int flag ) ;
   void  markmemory( void *ptr, int i ) ;
   int   listleaked( const tsd_t *TSD, int pflag ) ;
   void  memory_stats( const tsd_t *TSD ) ;
   void  mark_listleaked_params( const tsd_t *TSD ) ;
#endif /* TRACEMEM */

#ifdef TRACEMEM
# define Malloc(bytes)             mymalloc(bytes)
# define Malloc_TSD(TSD,bytes)     mymallocTSD(TSD,bytes)
# define MallocTSD(bytes)          mymallocTSD(TSD,bytes)
# define Free(ptr)                 myfree(ptr)
# define Free_TSD(TSD,ptr)         myfreeTSD(TSD,ptr)
# define FreeTSD(ptr)              myfreeTSD(TSD,ptr)
# define Free_string(ptr)          myfree(ptr)
# define Free_string_TSD(TSD,ptr)  myfreeTSD(TSD,ptr)
# define Free_stringTSD(ptr)       myfreeTSD(TSD,ptr)
#elif defined(FLISTS)
# define Malloc(bytes)             get_a_chunk(bytes)
# define Malloc_TSD(TSD,bytes)     get_a_chunkTSD(TSD,bytes)
# define MallocTSD(bytes)          get_a_chunkTSD(TSD,bytes)
# define Free(ptr)                 give_a_chunk(ptr)
# define Free_TSD(TSD,ptr)         give_a_chunkTSD(TSD,ptr)
# define FreeTSD(ptr)              give_a_chunkTSD(TSD,ptr)
# define Free_string(ptr)          give_a_streng(ptr)
# define Free_string_TSD(TSD,ptr)  give_a_strengTSD(TSD,ptr)
# define Free_stringTSD(ptr)       give_a_strengTSD(TSD,ptr)
#else /* neither TRACEMEM nor FLISTS */
# define Malloc(bytes)             Malloc_TSD(__regina_get_tsd(),bytes)
# define Malloc_TSD(TSD,bytes)     (TSD)->MTMalloc(TSD,bytes)
# define MallocTSD(bytes)          Malloc_TSD(TSD,bytes)
# define Free(ptr)                 Free_TSD(__regina_get_tsd(),ptr)
# define Free_TSD(TSD,ptr)         (TSD)->MTFree(TSD,ptr)
# define FreeTSD(ptr)              Free_TSD(TSD,ptr)
# ifdef CHECK_MEMORY
   void  give_a_streng( streng *ptr ) ;
   void  give_a_strengTSD( const tsd_t *TSD, streng *ptr ) ;
#  define Free_string(ptr)         give_a_streng(ptr)
#  define Free_string_TSD(TSD,ptr) give_a_strengTSD(TSD,ptr)
#  define Free_stringTSD(ptr)      give_a_strengTSD(TSD,ptr)
# else
#  define Free_string(ptr)         Free_string_TSD(__regina_get_tsd(),ptr)
#  define Free_string_TSD(TSD,ptr) (TSD)->MTFree(TSD,ptr)
#  define Free_stringTSD(ptr)      Free_string_TSD(TSD,ptr)
# endif /* CHECK_MEMORY */
#endif /* ifdef TRACEMEM */


/*
 * Routines in stack.c
 */
   int init_stacks( tsd_t *TSD ) ;
   int external_queues_used( const tsd_t *TSD ) ;
   void purge_stacks( const tsd_t *TSD ) ;
   streng *popline( tsd_t *TSD, const streng *queue_name, int *result, unsigned long waitflag ) ;
   int stack_lifo( tsd_t *TSD, streng *line, const streng *queue_name ) ;
   int stack_fifo( tsd_t *TSD, streng *line, const streng *queue_name ) ;
   int lines_in_stack( tsd_t *TSD, const streng *queue_name ) ;
#ifdef TRACEMEM
   void mark_stack( const tsd_t *TSD ) ;
#endif /* TRACEMEM */
   int drop_buffer( const tsd_t *TSD, int num ) ;
   int make_buffer( tsd_t *TSD ) ;
   void type_buffer( tsd_t *TSD ) ;
   Queue *find_free_slot( const tsd_t *TSD ) ;
   Queue *fill_input_queue_stem( tsd_t *TSD, streng *stemname, int stem0 ) ;
   Queue *fill_input_queue_stream( tsd_t *TSD, void *fileptr );
   void flush_stack( const tsd_t *TSD, Queue *src, Queue *dst, int is_fifo ) ;
   streng *stack_to_line( const tsd_t *TSD, Queue *q );
   int create_queue( tsd_t *TSD, const streng *queue_name, streng **result );
   int delete_queue( tsd_t *TSD, const streng *queue_name );
   int timeout_queue( tsd_t *TSD, const streng *timeout, const streng *queue_name );
   streng *get_queue( tsd_t *TSD );
   void fill_queue_name( const tsd_t *TSD, int *len, char **name );
   streng *set_queue( tsd_t *TSD, const streng *queue_name );
   Queue *addr_reopen_queue( tsd_t *TSD, const streng *queuename, char code ) ;
   int addr_same_queue( const tsd_t *TSD, const Queue *q1, const Queue *q2 ) ;
   Queue *addr_redir_queue( const tsd_t *TSD, Queue *q ) ;
   void addr_purge_queue( const tsd_t *TSD, Queue *q ) ;
   streng *addr_io_queue( tsd_t *TSD, Queue *q, streng *buffer, int isFIFO ) ;
   void addr_close_queue( const tsd_t *TSD, Queue *q ) ;


/*
 * Routines in misc.c
 */
   void set_locale_info( const char *info );
   void mem_upper( void *m, int len );
   void mem_lower( void *m, int len );
   int mem_cmpic( const void *buf1, const void *buf2, int len );
   const char *system_type( void ) ;
   double cpu_time( void ) ;
   void getsecs( time_t *secs, time_t *usecs ) ;
   unsigned hashvalue(const char *string, int length ) ;
   unsigned hashvalue_ic(const char *string, int length ) ;
   unsigned hashvalue_var(const streng *name, int start, int *stop );
#ifdef SKYOS
   clock_t clock( void );
#endif
   char **makeargs(const char *string, char escape);
   char *splitoffarg(const char *string, const char **trailer, char escape);
   char **makesimpleargs(const char *string);
   void destroyargs(char **args);



/*
 * Routines in error.c
 */
   int init_error( tsd_t *TSD ) ;
   const streng *errortext( const tsd_t *TSD, int errorno, int suberrnum, int request_english, int apply_inserts ) ;
   void exiterror( int errorno , int suberrorno, ... ) ;
   void __reginaerror( char *errtext ) ;
   const char *getsym( int numb ) ;
   int lineno_of( cnodeptr ) ;
   void clear_errortext_buffers( const tsd_t *TSD );


/*
 * Routines in variable.c
 */
   int known_reserved_variable( const char *name, unsigned length );
   /* We need a fast determination: */
#define KNOWN_RESERVED(n,l) ((l) && (*(n)=='.') && known_reserved_variable((n),(l)))

   void detach( const tsd_t *TSD, variableptr ptr ) ;
   int init_vars( tsd_t *TSD ) ;
   void expand_to_str( const tsd_t *TSD, variableptr ptr ) ;
   int var_was_found( const tsd_t *TSD ) ;
   var_hashtable *create_new_varpool( const tsd_t *TSD, int size ) ;
   void setdirvalue( tsd_t *TSD, const streng *name, streng *value ) ;
   void setvalue( tsd_t *TSD, const streng *name, streng *value, int pool ) ;
   num_descr *fix_compoundnum( tsd_t *TSD, nodeptr thisptr, num_descr *newdescr,
                               streng *string_val );
   void setshortcutnum( tsd_t *TSD, nodeptr thisptr, num_descr *value,
                        streng *string_val );
   const streng *getdirvalue( tsd_t *TSD, const streng *name ) ;
   const streng *getvalue( tsd_t *TSD, const streng *name, int pool ) ;
   const streng *isvariable( tsd_t *TSD, const streng *name ) ;
   int variables_per_SAA( tsd_t *TSD );
   void restore_variable_state( const tsd_t *TSD, int state );
   const streng *get_it_anyway( tsd_t *TSD, const streng *name, int pool ) ;
   const streng *get_it_anyway_compound( tsd_t *TSD, const streng *str ) ;
   void expose_var( tsd_t *TSD, const streng *name ) ;
   void drop_var( tsd_t *TSD, const streng *name ) ;
   void drop_dirvar( tsd_t *TSD, const streng *name ) ;
   void upper_var( tsd_t *TSD, const streng *name ) ;
   void set_ignore_novalue( const tsd_t *TSD ) ;
   int valid_var_symbol( const streng *symbol ) ;
   void clear_ignore_novalue( const tsd_t *TSD ) ;
#ifdef TRACEMEM
   void markvariables( const tsd_t *TSD, cproclevel ) ;
#endif /* TRACEMEM */
   const streng *shortcut( tsd_t *TSD, nodeptr ) ;
   num_descr *shortcutnum( tsd_t *TSD, nodeptr ) ;
   void setshortcut( tsd_t *TSD, nodeptr thisptr, streng *value ) ;
   streng *fix_compound( tsd_t *TSD, nodeptr thisptr, streng *value ) ;
   void kill_variables( const tsd_t *TSD, var_hashtable *array ) ;
   variableptr get_next_variable( tsd_t *TSD, int reset ) ;
   void set_reserved_value( tsd_t *TSD, int poolid, streng *val_str,
                            int val_int, int vflag );


/*
 * Routines in shell.c
 */
   void put_stem( tsd_t *TSD, environpart *e, streng *str );
   int init_shell( tsd_t *TSD ) ;
   void cleanup_envirpart(const tsd_t *TSD, environpart *ep);
   int posix_do_command( tsd_t *TSD, const streng *command, int flag, environment *env, Queue *redir ) ;



/*
 * Routines in tracing.c
 */
   int init_tracing( tsd_t *TSD ) ;
   void set_trace( tsd_t *TSD, const streng * ) ;
   void set_trace_char( tsd_t *TSD, char ) ;
   void flush_trace_chars( tsd_t *TSD ) ;
   void queue_trace_char( const tsd_t *TSD, char ch ) ;
   void tracenumber( tsd_t *TSD, const num_descr *num, char type ) ;
   void tracebool( tsd_t *TSD, int value, char type ) ;
   int pushcallstack( const tsd_t *TSD, treenode *thisptr ) ;
   void popcallstack( const tsd_t *TSD, int value ) ;
   void traceerror( tsd_t *TSD, const treenode *thisptr, int RC ) ;
   void tracecompound( tsd_t *TSD, const streng *stem, int length, const streng *index, char trch ) ;
   void starttrace( const tsd_t *TSD ) ;
   int intertrace( tsd_t *TSD ) ;
   void tracevalue( tsd_t *TSD, const streng *str, char type ) ;
   void traceline( tsd_t *TSD, const treenode *thisptr, char tch, int offset ) ;
   void traceback( tsd_t *TSD ) ;


/*
 * Routines in interp.c
 */
   streng *dointerpret( tsd_t *TSD, streng *string ) ;



/*
 * Routines in interpret.c
 */
   void update_envirs( const tsd_t *TSD, proclevel level ) ;
   proclevel newlevel( tsd_t *TSD, proclevel oldlevel ) ;
#define IPRT_BUFSIZE 2 /* buffer elements for the state in InterpreterStatus */
   void SaveInterpreterStatus(const tsd_t *TSD,unsigned *state);
   void RestoreInterpreterStatus(const tsd_t *TSD,const unsigned *state);
   streng *CallInternalFunction( tsd_t *TSD, nodeptr node, nodeptr thisptr,
                                 paramboxptr args );
   streng *interpret( tsd_t * volatile TSD, nodeptr volatile rootnode ) ;
   nodeptr getlabel( const tsd_t *TSD, const streng *name ) ;
   void removelevel( tsd_t *TSD, proclevel level ) ;
   void jump_rexx_signal( tsd_t *TSD );
   void jump_interpreter_exit( tsd_t *TSD, int processExitCode );
   void jump_script_exit( tsd_t *TSD, streng *result );
   int init_spec_vars( tsd_t *TSD ) ;


/*
 * Routines in yaccsrc.y
 */
   int yylex( void ) ;
   int __reginaparse( void ) ;
   void kill_lines( const tsd_t *TSD, lineboxptr first ) ;
   void destroytree( const tsd_t *TSD, nodeptr thisptr ) ;
   void newlabel( const tsd_t *TSD, internal_parser_type *ipt, nodeptr thisptr ) ;



/*
 * Routines in funcs.c
 */
   streng *buildtinfunc( tsd_t *TSD, nodeptr thisptr ) ;
   void deallocplink( tsd_t *TSD, paramboxptr first ) ;
   paramboxptr initplist( tsd_t *TSD, cnodeptr thisptr );
   paramboxptr initargs( tsd_t *TSD, int argc, const int *lengths,
                         const char **strings );
   int myatol( const tsd_t *TSD, const streng *text ) ;
   void checkparam( cparamboxptr params, int min, int max, const char *name ) ;
   char getoptionchar( tsd_t *TSD, const streng *param, const char *bif, int argnum, const char *ansi_options, const char *regina_options ) ;
   int atozpos( tsd_t *TSD, const streng *text, const char *bif, int argnum ) ;
   rx_64 atozposrx64( tsd_t *TSD, const streng *text, const char *bif, int argnum ) ;
   int atopos( tsd_t *TSD, const streng *text, const char *bif, int argnum ) ;
   rx_64 atoposrx64( tsd_t *TSD, const streng *text, const char *bif, int argnum ) ;
   int atoposorzero( tsd_t *TSD, const streng *text, const char *bif, int argnum ) ;
   char getonechar( tsd_t *TSD, const streng *text, const char *bif, int argnum ) ;
   char getonespecialchar( tsd_t *TSD, const streng *text, const char *bif, int argnum ) ;
   streng *int_to_streng( const tsd_t *TSD, int input ) ;
   streng *rx64_to_streng( const tsd_t *TSD, rx_64 input ) ;
   streng *rx64u_to_streng( const tsd_t *TSD, rx_64u input ) ;
   int convert_date(tsd_t *TSD, const streng *, char, struct tm *, char);
   int convert_time(const tsd_t *TSD,const streng *, char, struct tm *, time_t *);
   int basedays(int);
#ifdef TRACEMEM
   void mark_param_cache( const tsd_t *TSD ) ;
#endif
   void addtwostrings( char *one, char *two, char *answer ) ;
   const char *BIFname( tsd_t *TSD );


/*
 * Routines in cmath.c
 */
   double myatof( const tsd_t *TSD, const streng *string ) ;
   int myisnumber( const tsd_t *TSD, const streng *string ) ;
   int myisinteger( const streng *string ) ;


/*
 Functions in rexx.c
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef RXLIB
# if defined(__LCC__)
   int __regina_faked_main(int argc,char *argv[]) ;
# else
#  ifdef __MINGW32__
#   ifdef __REGINA_DLL
   int APIENTRY __regina_faked_main(int argc,char *argv[]) __declspec(dllexport);
#   else
   int APIENTRY __regina_faked_main(int argc,char *argv[]) __declspec(dllimport);
#   endif
#  else
   int APIENTRY __regina_faked_main(int argc,char *argv[]) ;
#  endif
# endif
#else
   int main(int argc,char *argv[]) ;
#endif

#ifdef __cplusplus
}
#endif

   int __regina_reexecute_main(int argc, char **argv);
   void mark_systeminfo( const tsd_t *TSD) ;
   nodeptr treadit( cnodeptr ) ;
   void setup_system( tsd_t *TSD, int isclient );
   sysinfobox *creat_sysinfo( const tsd_t *TSD, streng *envir );
   void setGlobalTSD( tsd_t *TSD);
   tsd_t *getGlobalTSD( void );


/*
 * Functions in signals.h
 */
   trap *gettraps( const tsd_t *TSD, proclevel level ) ;
   sigtype *getsigs( proclevel level ) ;
   void mark_signals( const tsd_t *TSD ) ;
   signal_handler regina_signal(int signum, signal_handler action);
   void halt_raised( tsd_t *TSD );
   int condition_hook( tsd_t *TSD, int, int, int, int, streng *, streng * ) ;
#ifdef WIN32
   void __regina_Win32RaiseCtrlC( tsd_t *TSD );
#endif

   void signal_setup( const tsd_t *TSD ) ;
   int identify_trap( int type ) ;
   void set_rexx_halt( tsd_t *TSD ) ;

/*
 * Routines in library.c
 */
   int init_library( tsd_t *TSD ) ;
   void free_orphaned_libs( tsd_t *TSD );
   void purge_library( tsd_t *TSD );
   struct library *find_library( const tsd_t *TSD, const streng *name );
   void set_err_message( const tsd_t *TSD, const char *message1, const char *message2 ) ;
   streng *rex_rxfuncerrmsg( tsd_t *TSD, cparamboxptr parms ) ;
   streng *rex_rxfuncquery( tsd_t *TSD, cparamboxptr parms ) ;
   streng *rex_rxfuncadd( tsd_t *TSD, cparamboxptr parms ) ;
   streng *rex_rxfuncdefine( tsd_t *TSD, cparamboxptr parms );
   streng *rex_gciprefixchar( tsd_t *TSD, cparamboxptr parms );
   streng *rex_rxfuncdrop( tsd_t *TSD, cparamboxptr parms ) ;
   int IfcRegFunc( const tsd_t *TSD, const char *rxname, const char *module,
                   const char *objnam, PFN entry );
   int IfcRegExit( const tsd_t *TSD, const char *rxname, const char *module,
                   const char *objnam, PFN entry, void *user_area );
   int IfcRegSubcom( const tsd_t *TSD, const char *rxname, const char *module,
                     const char *objnam, PFN entry, void *user_area );
   int IfcDelFunc( tsd_t *TSD, const char *rxname );
   int IfcDelExit( tsd_t *TSD, const char *rxname, const char *module );
   int IfcDelSubcom( tsd_t *TSD, const char *rxname, const char *module );
   int IfcQueryFunc( const tsd_t *TSD, const char *rxname );
   int IfcQueryExit( const tsd_t *TSD, const char *rxname, const char *module,
                     void *user_area );
   int IfcQuerySubcom( const tsd_t *TSD, const char *rxname,
                       const char *module, void *user_area );
   struct entry_point *loaded_lib_func( const tsd_t *TSD, const streng *name );
   struct entry_point *exit_hook( const tsd_t *TSD, const char *env, int len );
   struct entry_point *subcom_hook( const tsd_t *TSD, const char *com, int len );

/*
 * functions in mt_notmt.c/mt_<os>.c
 */
   int IfcReginaCleanup( void );

/*
 * functions in macros.c
 */
   void killsystem( tsd_t *TSD, sysinfo systm ) ;
   internal_parser_type enter_macro( tsd_t *TSD, const streng *source,
                                     void **ept, unsigned long *extlength);
   streng *do_instore( tsd_t * volatile TSD, const streng *name, paramboxptr args,
                       const streng *envir, int * volatile RetCode, int hooks,
                       const void *instore, unsigned long instore_length,
                       const char *instore_source,
                       unsigned long instore_source_length,
                       const internal_parser_type *ipt,
                       int ctype ) ;
   streng *execute_external( tsd_t * volatile TSD, const streng *command,
                             paramboxptr args, const streng *envir,
                             int * volatile RetCode, int hooks, int ctype ) ;
   int count_params( cparamboxptr ptr, int soft ) ;
   streng *get_parameter( paramboxptr ptr, int number ) ;


/*
 * Functions in envir.c
 */
   void post_process_system_call( tsd_t *TSD, const streng *cmd,
                                  int rc_code, const streng *rc_value,
                                  cnodeptr thisptr );
   streng *perform( tsd_t *TSD, const streng *command, const streng *envir, cnodeptr thisptr, cnodeptr overwrite );
   void add_envir( tsd_t *TSD, const streng *name, int type, int subtype ) ;
   int envir_exists( const tsd_t *TSD, const streng *name );
   streng *get_envir_details( const tsd_t *TSD, char opt, const streng *name );
   int set_subcomed_envir( const tsd_t *TSD, const streng *name, int subcomed );
   int get_subcomed_envir( const tsd_t *TSD, const streng *name );
   int init_envir( tsd_t *TSD ) ;
   void del_envir( tsd_t *TSD, const streng *name ) ;
   int set_envir( const tsd_t *TSD, const streng *envirname, const nodeptr ios ) ;
   streng *run_popen( tsd_t *TSD, const streng *command, const streng *envir ) ;


/*
 * Functions in client.c
 */
   int init_client( tsd_t *TSD ) ;
   int hookup( tsd_t *TSD, int type ) ;
   int hookup_output( tsd_t *TSD, int type, const streng *outdata ) ;
   int hookup_output2( tsd_t *TSD, int type, const streng *outdata1, const streng *outdata2 ) ;
   int hookup_input( tsd_t *TSD, int type, streng **indata ) ;
   int hookup_input_output( tsd_t *TSD, int type, const streng *outdata, streng **indata ) ;
   streng *call_unknown_external( tsd_t *TSD, const streng *name, cparamboxptr parms, char called ) ;
   streng *call_known_external( tsd_t *TSD, const struct entry_point *vbox, cparamboxptr parms, char called ) ;
   streng *SubCom( tsd_t *TSD, const streng *command, const streng *envir, int *rc ) ;

/*
 * Routines in options.c
 */
   void do_options( const tsd_t *TSD, proclevel pl, streng *options, int ) ;
   int get_options_flag( cproclevel, int ) ;
   void set_options_flag( proclevel, int, int ) ;


/*
 * Routines in rexxext.c
 */
   char *mygetenv( const tsd_t *TSD, const char *name, char *buf, int bufsize ) ;
   void set_pause_at_exit( );
   streng *rex_userid( tsd_t *TSD, cparamboxptr parms ) ;
   streng *rex_getcaller( tsd_t *TSD, cparamboxptr parms ) ;
   streng *rex_getcallstack( tsd_t *TSD, cparamboxptr parms ) ;
   streng *rex_rxqueue( tsd_t *TSD, cparamboxptr parms ) ;
#if defined(WIN32) && !defined(__WINS__) && !defined(__EPOC32__)
   void set_pause_at_exit( void );
#endif

/*
 * Routines in vmscmd.c
 */
   int init_vms( tsd_t *TSD ) ;
   int vms_do_command( tsd_t *TSD, const streng *cmd, int io_flags, environment *env, Queue *redir );
   int vms_killproc( tsd_t *TSD ) ;
   streng *vms_resolv_symbol( tsd_t *TSD, streng *name, streng *newstr, streng *pool ) ;

/*
 * Routines in builtin.c
 */
   int init_builtin( tsd_t *TSD ) ;
   streng *ext_pool_value( tsd_t *TSD, streng *name, streng *value, streng *env );
   streng *std_abbrev( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_abs( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_address( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_arg( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_bitand( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_bitor( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_bitxor( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_center( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_changestr( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_compare( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_condition( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_copies( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_countstr( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_datatype( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_date( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_delstr( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_delword( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_digits( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_errortext( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_form( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_format( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_fuzz( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_insert( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_lastpos( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_length( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_left( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_max( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_min( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_overlay( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_pos( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_qualify( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_queued( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_right( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_reverse( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_random( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_sign( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_sourceline( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_space( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_strip( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_substr( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_subword( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_symbol( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_time( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_trace( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_translate( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_trunc( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_value( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_verify( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_word( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_wordindex( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_wordlength( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_wordpos( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_words( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_xrange( tsd_t *TSD, cparamboxptr parms ) ;
   streng *rex_poolid( tsd_t *TSD, cparamboxptr parms ) ;
   streng *rex_lower( tsd_t *TSD, cparamboxptr parms );

/*
 * Routines in convert.c
 */
   streng *std_b2x( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_c2d( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_c2x( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_d2c( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_d2x( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_x2b( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_x2c( tsd_t *TSD, cparamboxptr parms ) ;
   streng *std_x2d( tsd_t *TSD, cparamboxptr parms ) ;

/*
 * Routines in dbgfuncs.c
 */
#ifdef FLISTS
   streng *dbg_freelists( tsd_t *TSD, cparamboxptr parms ) ;
#endif
#ifndef NDEBUG
# ifdef TRACEMEM
   streng *dbg_allocated( tsd_t *TSD, cparamboxptr parms ) ;
# endif
   streng *dbg_dumpvars( tsd_t *TSD, cparamboxptr parms ) ;
   streng *dbg_dumptree( tsd_t *TSD, cparamboxptr parms ) ;
# ifdef TRACEMEM
   streng *dbg_listleaked( tsd_t *TSD, cparamboxptr parms ) ;
   streng *dbg_memorystats( tsd_t *TSD, cparamboxptr parms ) ;
# endif /* TRACEMEM */
#endif /* !NDEBUG */
   streng *dbg_traceback( tsd_t *TSD, cparamboxptr parms ) ;

/*
 * Routines in vmsfuncs.c
 */
#ifdef VMS
   streng *vms_f_directory( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_file_attributes( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_getdvi( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_getjpi( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_getqui( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_getsyi( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_identifier( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_message( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_mode( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_pid( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_privilege( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_process( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_string( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_time( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_setprv( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_user( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_locate( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_length( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_integer( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_trnlnm( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_logical( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_parse( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_search( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_type( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_file_attributes( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_extract( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_element( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_cvui( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_cvsi( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_cvtime( tsd_t *TSD, cparamboxptr parms ) ;
   streng *vms_f_fao( tsd_t *TSD, cparamboxptr parms ) ;
#endif

/*
 * Routines in unxfuncs.c
 */
   streng *unx_getpath( tsd_t *TSD, cparamboxptr dummy ) ;
   streng *unx_popen( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_getpid( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_gettid( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_eof( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_uname( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_fork( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_unixerror( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_chdir( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_getenv( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_putenv( tsd_t *TSD, cparamboxptr parms ) ;
   streng *unx_crypt( tsd_t *TSD, cparamboxptr parms ) ;

/*
 * Routines in os2funcs.c
 */
   streng *os2_directory( tsd_t *TSD, cparamboxptr parms ) ;
   streng *os2_beep( tsd_t *TSD, cparamboxptr parms ) ;
   streng *os2_filespec( tsd_t *TSD, cparamboxptr parms ) ;

/*
 * Routines in rexxsaa.c
 */
   int init_rexxsaa( tsd_t *TSD ) ;
   void deinit_rexxsaa( tsd_t *TSD ) ;

/*
 * Routines in vmsfuncs.c
 */
   int init_vmf( tsd_t *TSD ) ;

/*
 * Routines in wrappers.c
 */
   void *wrapper_load( const tsd_t *TSD, const streng *module ) ;
   void wrapper_unload( const tsd_t *TSD, void *libhandle ) ;
   PFN wrapper_get_addr( const tsd_t *TSD, const struct library *lptr, const streng *name) ;

/*
 * Routines in staticld.c
 */
#ifdef DYNAMIC_STATIC
   void *static_dlopen( char *name );
   int static_dlsym( void *addr, char *name, void **faddr );
   void static_list_packages();
#endif

/*
 * Routines in strings.c
 * string and streng routines are ugly but we need the speedup of passing the
 * TSD to take profit from flists in the multi-threading environment.
 */
   void __regina_Str_first( void );
#if !defined(FLISTS) || defined(TRACEMEM)
   streng *__regina_Str_make( int size ) ;
   streng *__regina_Str_make_TSD( const tsd_t *TSD, int size ) ;
#  define Str_make(a)         __regina_Str_make(a)
#  define Str_make_TSD(TSD,a) __regina_Str_make_TSD(TSD,a)
#  define Str_makeTSD(size)   Str_make_TSD(TSD,size)
#else
#  define Str_make(a)         get_a_streng(a)
#  define Str_make_TSD(TSD,a) get_a_strengTSD(TSD,a)
#  define Str_makeTSD(a)      get_a_strengTSD(TSD,a)
#endif
#define nullstringptr() Str_makeTSD(0)
   streng *Str_ndup( const streng *name, int length ) ;
   streng *Str_ndup_TSD( const tsd_t *TSD, const streng *name, int length ) ;
#define Str_ndupTSD(name,length) Str_ndup_TSD(TSD,name,length)
   streng *Str_nodup( const streng *name, int length, int offset ) ;
   streng *Str_nodup_TSD( const tsd_t *TSD, const streng *name, int offset, int length ) ;
#define Str_nodupTSD(name,length,offset) Str_nodup_TSD(TSD,name,length,offset)
   streng *Str_dup( const streng *name ) ;
   streng *Str_dup_TSD( const tsd_t *TSD, const streng *input ) ;
#define Str_dupTSD(name) Str_dup_TSD(TSD,name)
   streng *Str_dupstr( const streng *name ) ;
   streng *Str_dupstr_TSD( const tsd_t *TSD, const streng *input ) ;
#define Str_dupstrTSD(name) Str_dupstr_TSD(TSD,name)
   streng *Str_ncpy( streng *to, const streng *from, int length ) ;
   streng *Str_ncpy_TSD( const tsd_t *TSD, streng *to, const streng *from, int length ) ;
#define Str_ncpyTSD(to,from,length) Str_ncpy_TSD(TSD,to,from,length)
   streng *Str_ncre( const char *from, int length ) ;
   streng *Str_ncre_TSD( const tsd_t *TSD, const char *from, int length ) ;
#define Str_ncreTSD(from,length) Str_ncre_TSD(TSD,from,length)
   streng *Str_cat( streng *to, const streng *from ) ;
   streng *Str_cat_TSD( const tsd_t *TSD, streng *first, const streng *second ) ;
#define Str_catTSD(first,second) Str_cat_TSD(TSD,first,second)
   streng *Str_ncat( streng *to, const streng *from, int length ) ;
   streng *Str_ncat_TSD( const tsd_t *TSD, streng *first, const streng *second, int length ) ;
#define Str_ncatTSD(first,second,length) Str_ncat_TSD(TSD,first,second,length)
   streng *Str_nocat( streng *to, const streng *from, int length, int offset ) ;
   streng *Str_nocat_TSD( const tsd_t *TSD, streng *first, const streng *second, int length, int offset ) ;
#define Str_nocatTSD(first,second,length,offset) Str_nocat_TSD(TSD,first,second,length,offset)
   int Str_ncmp( const streng *one, const streng  *two, int length ) ;
   int Str_cmp( const streng *one, const streng *two ) ;
   int Str_ccmp( const streng *one, const streng *two ) ;
   int Str_cncmp( const streng *one, const streng *two, int length ) ;
   int Str_cnocmp( const streng *one, const streng *two, int length, int offset ) ;
   streng *Str_cre( const char *value ) ;
   streng *Str_cre_TSD( const tsd_t *TSD, const char *input ) ;
#define Str_creTSD(value) Str_cre_TSD(TSD,value)
   streng *Str_crestr( const char *value );
   streng *Str_crestr_TSD( const tsd_t *TSD, const char *input ) ;
#define Str_crestrTSD(input) Str_crestr_TSD(TSD,input)
   streng *Str_catstr( streng *in, const char *more ) ;
   streng *Str_catstr_TSD( const tsd_t *TSD, streng *base, const char *append ) ;
#define Str_catstrTSD(base,append) Str_catstr_TSD(TSD,base,append)
   streng *Str_ncatstr( streng *base, const char *input, int length ) ;
   streng *Str_ncatstr_TSD( const tsd_t *TSD, streng *base, const char *input, int length ) ;
#define Str_ncatstrTSD(base,input,length) Str_ncatstr_TSD(TSD,base,input,length)
   char *str_of( const tsd_t *TSD, const streng *input ) ;
#define str_ofTSD(input) str_of(TSD,input)
#ifndef EXTERNAL_TO_REGINA
   volatile char *tmpstr_of( tsd_t *TSD, const streng *input ) ;
#endif
   streng *Str_upper( streng *in ) ;
   streng *Str_lower( streng *in ) ;
   streng *Str_strp( streng * input, char chr, char opt ) ;
   void __regina_Str_last( void );

/*
 * Routines in strmath.c
 */
#define DIVTYPE_NORMAL    0
#define DIVTYPE_INTEGER   1
#define DIVTYPE_REMAINDER 2
#define DIVTYPE_BOTH      3 /* use only with str_div directly ! */

   int descr_sign( const void * ) ;
   streng *str_sign( tsd_t *TSD, const streng*);
   streng *str_trunc( tsd_t *TSD, const streng*, int ) ;
   streng *str_normalize( const tsd_t *TSD, const streng* ) ;
   streng *str_digitize( tsd_t *TSD, streng*, int, int, const char *, int ) ;
   streng *str_format( tsd_t *TSD, const streng*, int, int, int, int ) ;
   streng *str_binerize( tsd_t *TSD, num_descr *, int, int, int, const char *, int ) ;
   int str_true( const tsd_t *TSD, const streng* ) ;
   streng *str_abs( tsd_t *TSD, const streng* ) ;
   num_descr *get_a_descr( tsd_t *TSD, const char *big, int argno, const streng* ) ;
   void free_a_descr( const tsd_t *TSD, num_descr* ) ;
#ifdef TRACEMEM
   void mark_descrs( const tsd_t *TSD ) ;
#endif
   int init_math( tsd_t *TSD ) ;
   void descr_copy( const tsd_t *TSD, const num_descr *f, num_descr *s ) ;
   num_descr *string_incr( tsd_t *TSD, num_descr *input, cnodeptr node );
   void string_mul( tsd_t *TSD, const num_descr *f, const num_descr *s,
                    num_descr *r, cnodeptr left, cnodeptr right );
   void string_div( tsd_t *TSD, const num_descr *f, const num_descr *s,
                    num_descr *r, num_descr *r2, int type, cnodeptr left,
                    cnodeptr right );
   int myiswnumber( tsd_t *TSD, const streng *number, num_descr **num,
                    int noDigitsLimit );
   void str_round( num_descr *descr, int size ) ;
   void str_round_lostdigits( tsd_t *TSD, num_descr *descr, int size );
   void string_pow( tsd_t *TSD, const num_descr *num, num_descr *acc,
                    num_descr *res, cnodeptr lname, cnodeptr rname);
   int descr_to_int( const tsd_t *TSD, const num_descr *input, int, int, const char *, int ) ;
   num_descr *is_a_descr( const tsd_t *TSD, const streng *input ) ;
   int getdescr( const tsd_t *TSD, const streng *num, num_descr *descr ) ;
   int string_test( const tsd_t *TSD, const num_descr *fdescr,
                    const num_descr *sdescr ) ;
   void string_add( tsd_t *TSD, const num_descr *f, const num_descr *s,
                    num_descr *r, cnodeptr left, cnodeptr right );
   void str_strip( num_descr *num ) ;
   streng *str_norm( const tsd_t *TSD, num_descr *in, streng *trystr )  ;
   int streng_to_int( const tsd_t *TSD, const streng *input, int *error ) ;
   rx_64 streng_to_rx64( const tsd_t *TSD, const streng *input, int *error ) ;

/*
 * Routines in instore.c
 */
   void NewProg(void);
   void EndProg(nodeptr Start);
   nodeptr FreshNode(void);
   void RejectNode(nodeptr NoLongerUsed);
   offsrcline *FreshLine(void);
   void DestroyInternalParsingTree(const tsd_t *TSD, internal_parser_type *ipt);
   external_parser_type *TinTree(const tsd_t *TSD,
                                 const internal_parser_type *ipt,
                                 unsigned long *length);
   internal_parser_type ExpandTinnedTree(const tsd_t *TSD,
                                         const external_parser_type *ept,
                                         unsigned long length,
                                         const char *incore_source,
                                         unsigned long incore_source_length);
   int IsValidTin(const external_parser_type *ept, unsigned long eptlen);

/*
 * Routines in arxfuncs.c
 */
   int init_arexxf( tsd_t *TSD );
   streng *arexx_open ( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_close( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_writech( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_writeln( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_seek( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_readch( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_readln( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_eof( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_b2c( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_c2b( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_bitchg( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_bitclr( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_bitset( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_bittst( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_bitcomp( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_hash( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_compress( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_trim( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_upper( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_randu( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_getspace( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_freespace( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_import( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_export( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_storage( tsd_t *TSD, cparamboxptr parm1 );
   streng *arexx_show( tsd_t *TSD, cparamboxptr parm1 );

/*
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************
 * global variables
 */

/* externals which are protected by regina_parser (see lexsrc.l) */
extern internal_parser_type parser_data;
extern int retlength ; /* value shared by lexsrc.l and yaccsrc.y only */
extern char retvalue[] ; /* value shared by lexsrc.l and yaccsrc.y only */
extern char *__reginatext ; /* value shared by lexsrc.l and yaccsrc.y only */
extern unsigned SymbolDetect ; /* value shared by lexsrc.l and yaccsrc.y only */
#define SD_INPUT        0x00000001
#define SD_OUTPUT       0x00000002
#define SD_ERROR        0x00000004
#define SD_NORMAL       0x00000008
#define SD_APPEND       0x00000010
#define SD_REPLACE      0x00000020
#define SD_STREAM       0x00000040
#define SD_STEM         0x00000080
#define SD_LIFO         0x00000100
#define SD_FIFO         0x00000200
#define SD_ADDRWITH     0x000003FF /* All the above */
#ifndef NDEBUG
extern int __reginadebug ;
#endif
/* end of externals protected by regina_parser (see lexsrc.l) */

extern unsigned countthreads; /* Not protected and not save in use! */

extern const char *months[];
extern const char *signalnames[];
#ifdef WIN32
extern volatile int __regina_Win32CtrlCRaised;
#endif
extern unsigned char_info[];
extern unsigned char u_to_l[];
extern unsigned char l_to_u[];
extern const char *numeric_forms[] ;
extern const char *invo_strings[] ;
extern const char *argv0 ;
/* Don't terminate the following lines by a semicolon */
EXPORT_GLOBAL_PROTECTION_VAR(regina_globals)

