// Function prototypes for 4xxx family
//   Copyright 1993 - 1997 Rex C. Conn

// Prototypes for internal commands
int activate_cmd( int, char ** );
int batch( int, char ** );
int battext_cmd( int, char ** );
int beep_cmd( int, char ** );
int call_cmd( int, char ** );
int case_cmd( int, char ** );
int cd_cmd( int, char ** );
int cdd_cmd( int, char ** );
int chcp_cmd( int, char ** );
int cmds_cmd( int, char ** );
int color_cmd( int, char ** );
int copy_cmd( int, char ** );
int ctty_cmd( int, char ** );
int del_cmd( int, char ** );
int delay_cmd( int, char ** );
int describe_cmd( int, char ** );
int detach_cmd( int, char ** );
int df_cmd( int, char ** );
int dir_cmd( int, char ** );
int dirs_cmd( int, char ** );
int do_cmd( int, char ** );
int dpath_cmd( int, char ** );
int drawbox_cmd( int, char ** );
int drawline_cmd( int, char ** );
int echo_cmd( int, char ** );
int echos_cmd( int, char ** );
int endlocal_cmd( int, char ** );
int eset_cmd( int, char ** );
int except_cmd( int, char ** );
int exit_cmd( int, char ** );
int external( int, char ** );
int ffind_cmd( int, char ** );
int for_cmd( int, char ** );
int global_cmd( int, char ** );
int gosub_cmd( int, char ** );
int goto_cmd( int, char ** );
int help_cmd( int, char ** );
int history_cmd( int, char ** );
int if_cmd( int, char ** );
int inkey_input_cmd( int, char ** );
int keybd_cmd( int, char ** );
int keys_cmd( int, char ** );
int keystack_cmd( int, char ** );
int list_cmd( int, char ** );
int loadbtm_cmd( int, char ** );
int log_cmd( int, char ** );
int memory_cmd( int, char ** );
int mv_cmd( int, char ** );
int on_cmd( int, char ** );
int option_cmd( int, char ** );
int path_cmd( int, char ** );
int pause_cmd( int, char ** );
int popd_cmd( int, char ** );
int process_cmd( int, char ** );
int prompt_cmd( int, char ** );
int pushd_cmd( int, char ** );
int quit_cmd( int, char ** );
int rd_cmd( int, char ** );
int reboot_cmd( int, char ** );
int remark_cmd( int, char ** );
int ren_cmd( int, char ** );
int ret_cmd( int, char ** );
int scr_cmd( int, char ** );
int scrput_cmd( int, char ** );
int set_cmd( int, char ** );
void set_window( void );
int setdate_cmd( int, char ** );
int select_cmd( int, char ** );
int setdos_cmd( int, char ** );
int setlocal_cmd( int, char ** );
int settime_cmd( int, char ** );
int shift_cmd( int, char ** );
int shralias_cmd( int, char ** );
int start_cmd( int, char ** );
int startat_cmd( int, char ** );
int switch_cmd( int, char ** );
int tee_cmd( int, char ** );
int timer_cmd( int, char ** );
int title_cmd( int, char ** );
int touch_cmd( int, char ** );
int type_cmd( int, char ** );
int unset_cmd( int, char ** );
int verify_cmd( int, char ** );
int window_cmd( int, char ** );
int y_cmd( int, char ** );


// Support routines in all versions

// BATCH.C
int exit_bat( void );
int do_parsing( char * );
int iff_parsing( char *, char * );


// CMDS.C
int findcmd( char *, int );
int findcmdhelp( char *, int );

// DIRCMDS.C
void init_dir( void );
void init_page_size( void );
char *GetSearchAttributes( char * );
char * dir_sort_order( char * );
void dir_free( DIR_ENTRY * );
void _page_break( void );
void ColorizeDirectory( DIR_ENTRY *, unsigned int, int );
int fstrcmp( char *, char *, int );
int SearchDirectory( int, char *, DIR_ENTRY **, unsigned int *, RANGES *, int );


// ENV.C
char * get_variable( char * );
char * get_alias( char * );
char * get_list( char *, PCH );
int add_variable( char * );
int add_list( char *, PCH );
char * next_env( PCH );
char * end_of_env( PCH );


// ERROR.C
int usage( char * );
int error( int, char * );


// EVAL.C
void SetEvalPrecision( char *, unsigned int *, unsigned int * );
int evaluate( char * );

// EXETYPE.C
UINT GetExeType( char * );


// EXPAND.C
void dup_handle( unsigned int, unsigned int );
void RedirToClip( char *, int );
int CopyToClipboard( int );
int CopyFromClipboard( char * );
int redir( char *, REDIR_IO *);
void unredir(REDIR_IO *, int *);
void ClosePipe( char *);
int alias_expand( char *);
int var_expand( char *, int );
void EscapeLine( char *);
void escape( char *);
int escape_char( int );
void addhist( char *);
char *prev_hist( PCH );
char *next_hist( PCH );


// FILECMDS.C
void FilesProcessed( char *, long );
char *show_atts( int );


// INIPARSE.C
extern void IniClear(INIFILE *);
extern int IniParse( char *, INIFILE *, unsigned int );
extern int IniLine( char *, INIFILE *, int, int, int, char **);
extern int ini_string(INIFILE *, int *, char *, int );


// IOFMT.C
void IntToAscii( int, char *);
void IntToAscii2( int, char *);
int sscanf_far(const char *, const char *, ...);
int sprintf_far( char *, const char *, ...);
int qprintf( int, const char *, ...);
int color_printf( int, const char *, ...);
int qputs(const char *);
void crlf( void );
void qputc( int, char);


// KEYPARSE.C
int keyparse( char *, int );
int toklist( char *, TL_HEADER *, int *);


// LINES.C
void _box( int, int, int, int, int, int, int, int, int );
int verify_row_col( unsigned int, unsigned int );


// MISC.C
long QuerySeekSize( int );
long long QuerySeekSizeLL( int );
long RewindFile( int );
int cvtkey( unsigned int, unsigned int );
int iswhite( char );
int isdelim( char );
int QueryIsNumeric( char *);
int QueryIsCON( char *);
int QueryIsRelative( char *);
char * skipspace( char *);
char * GetToken( char *, char *, int, int );
char * first_arg( char *);
char * next_arg( char *, int );
char * last_arg( char *, int * );
char * ntharg( char *, int );
char * scan( char *, char *, char * );
int GetMultiCharSwitch( char *, char *, char *, int );
int GetSwitches( char *, char *, long *, int );
long switch_arg( char *, char * );
int GetRange( char *, RANGES *, int );
int GetStrDate( char *, unsigned int *, unsigned int *, unsigned int * );
int MakeDaysFromDate(unsigned long *, char * );
int MakeDateFromDays(long, unsigned int *, unsigned int *, unsigned int * );
void collapse_whitespace( char *, char * );
void strip_leading( char *, char * );
void strip_trailing( char *, char * );
char *GetTempDirectory( char * );
char * filecase( char *);
void SetDriveString( char * );
char * gcdir( char *, int );
int gcdisk( char *);
int QueryIsDotName( char * );
char * path_part( char * );
char * fname_part( char * );
char * ext_part( char * );
void copy_filename( char *, char *);
void AddCommas( char * );
void StripQuotes( char * );
int AddQuotes( char * );
int mkdirname( char *, char * );
int MakeShortFromLong( char * );
char * mkfname( char *, int );
void insert_path( char *, char *, char * );
int is_file( char * );
int is_file_or_dir( char * );
int is_dir( char * );
int is_net_drive( char * );
char *executable_ext( char * );
int ExcludeFiles( char *, char * );
int wild_cmp( char *, char *, int, int );
void GetINIPath( char *, int );
char * FormatDate( int, int, int, int );
void honk( void );
int QueryInputChar( char *, char * );
char * stristr( char *, char * );
char * strins( char *, char * );
char * strend( char * );
char * strlast( char * );
void more_page( char *, int );
void more_page_bin( char *, int , int );
void incr_column( char, int * );
long GetRandom( long, long );
int OffOn( char * );
int GetCursorRange( char *, int *, int *);
int GetColors( char *, int );
void set_colors( int );
char * ParseColors( char *, int *, int *);
int color_shade( char * );
int process_descriptions( char *, char *, int );
void clear_screen(void);

// MAIN.C
int main( int, char ** );


// PARSER.C
int DoINT2E( char * );
void find_4files( char * );
int BatchCLI( void );
int open_batch_file( void );
void close_batch_file( void );
int getline( int, char *, int, int );

int command( char *, int );

int parse_line( char *, char *, int (*)( int, char **), int );
char * searchpaths( char *, char *, int );
void show_prompt( void );


// SCREENIO.C
int egets( char *, int, int );


// SYSCMDS.C
int __cd( char *, int );
void SaveDirectory( char *, char *);
int MakeDirectory( char *, int );
int DestroyDirectory( char * );
int getlabel( char *);
char * GetLogName( int );
int _log_entry( char *, int );
char * gdate( int );
char * gtime( int );
void _timer( int, char * );


// WINDOW.C
POPWINDOWPTR wOpen( int, int, int, int, int, char *, char * );
void wRemove(POPWINDOWPTR);
void wClear( void );
void wSetCurPos( int, int );
void wWriteStrAtt( int, int, int, char *);
char * wPopSelect( int, int, int, int, char **, int, int, char *, char *, char *, int );


// Environment-specific modules

// EA.C
BOOL _System EAWriteASCII( PCHAR, PCHAR, PCHAR );
BOOL _System EAReadASCII( PCHAR, PCHAR, PCHAR, PINT );


// LISTPM.C
MRESULT EXPENTRY ListTBWndProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ListWndProc(HWND, ULONG, MPARAM, MPARAM);
int ListHScroll(MPARAM, MPARAM);
int ListVScroll(MPARAM, MPARAM);
MRESULT EXPENTRY ListFindDlgProc(HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ListGotoDlgProc(HWND, ULONG, MPARAM, MPARAM);


// OS2CALLS
APIRET APIENTRY BreakHandler( PEXCEPTIONREPORTRECORD, PEXCEPTIONREGISTRATIONRECORD, PCONTEXTRECORD, PVOID );
void BreakOut( void );
void CheckFreeStack( void );
int InitializeREXX( int );
int UninstallREXX( void );
int process_rexx( char *, char *, int );
int nCallRexx( void );
int _help( char *, char *);
int _option( void );
int open_pipe(REDIR_IO *, char *);
void NoInherit(HFILE);
int FileRead( int, char *, unsigned int, unsigned int *);
int FileWrite( int, char *, unsigned int, unsigned int *);
void FreeMem( char *);
char * AllocMem( unsigned int *);
char * ReallocMem( char *, unsigned long );
void HoldSignals( void );
void EnableSignals( void );
void RequestSemaphore(HMTX *, char *);
void FreeSemaphore(HMTX);
void SysWait( unsigned long );
int SysBeep( unsigned int, unsigned int );
void QueryDateTime(DATETIME *);
int SetDateTime(DATETIME *);
int QueryHandleDateTime( int, DATETIME * );
int SetFileDateTime( char *, int, DATETIME *, int );
long QueryFileSize( char *, int );
int QueryFileMode( char *, unsigned int *);
int SetFileMode ( char *, unsigned int );
int QueryVerifyWrite( void );
void SetVerifyWrite( int );
int QueryCodePage( void );
int SetCodePage( int );
unsigned long QueryMemSize( char *);
int QueryDiskInfo( char *, QDISKINFO *, int );
char * QueryVolumeInfo( char *, char *, unsigned long *);
int QueryIsANSI( void );
int ifs_type( char *);
int QueryIsCDROM( char *);
int QueryIsConsole( int );
int app_type( char *);
unsigned int QuerySysInfo( int );
int QueryIsPipeName( char *);
int QueryIsPipeHandle( int );
int QueryIsDevice( char *);
int QueryIsDeviceHandle( int );
int QueryDriveExists( int );
int QueryDriveRemote( int );
int QueryDriveReady( int );
int QueryDriveRemovable( int );
int is_signed_digit( int );
int is_unsigned_digit( int );
void QueryCountryInfo( void );
void SetOSVersion( void );
int _ctoupper( int );
int UniqueFileName( char *);
char *MakeSFN( char *, char * );
char * find_file( int, char *, unsigned int, FILESEARCH *, char *);
void update_task_list( char *);
int start_session(STARTDATA *, char *);
int TCGetPrivateProfileInt( char *, char *, int );
void TCWritePrivateProfileInt( char *, char *, int );
void TCWritePrivateProfileStr( char *, char *, char *);
int FindInstalledFile( char *path, char *filename );
int GetClipboardText( unsigned long *pulState, PCSZ *ppszClipboardText );
void ReleaseClipboardText( unsigned long *pulState );

// OS2INIT.C
void InitOS( int, char **);
void SaveIniData( void );
void DisplayCopyright( void );


// OS2TTY.C
unsigned int GetKeystroke( int );
void GetCurPos(int *, int *);
void GetAtt( unsigned int *, unsigned int *);
void SetCurPos( int, int );
void Scroll( int, int, int, int, int, int );
void WriteChrAtt( int, int, int, int );
void WriteStrAtt( int, int, int, char *);
void WriteVStrAtt( int, int, int, char *);
void WriteTTY( char *);
void SetLineColor( int, int, int, int );
unsigned int GetScrRows( void );
unsigned int GetScrCols( void );
void SetCurSize( int );
void SetBrightBG( void );


// OS2NB.C
MRESULT EXPENTRY NotebookDlgProc(HWND, ULONG, MPARAM, MPARAM);


// TCOS2CFG.C
void ConfigNotebook( void );

// SYSCMDS.C
MRESULT EXPENTRY SSDebugDlgProc( HWND, ULONG, MPARAM, MPARAM );


// 4OS2UTIL.ASM
char *_System DecodeMsg( int, char *);
unsigned int ASMCALL get_cpu( void );
unsigned int ASMCALL get_ndp( void );

// DBCS.C
void InitDBCSLead( void );
BOOL IsDBCSLead( UCHAR );
BOOL IsDBCSLeadStr( char *, char * );
BOOL IsDBCSTrailStr( char *, char * );
