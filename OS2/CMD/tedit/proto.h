/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

/*
$Id: proto.h,v 1.22 2002/08/13 09:11:47 mark Exp $
*/

                                                         /* commutil.c */
CHARTYPE *get_key_name Args((int,int *));
CHARTYPE *get_key_definition Args((int,bool,bool,bool));
short function_key Args((int,int,bool));
bool is_modifier_key Args((int));
CHARTYPE *build_default_key_definition Args((int, CHARTYPE *));
CHARTYPE *build_synonym_definition Args(( CHARTYPE *, CHARTYPE *, DEFINE * ));
short display_all_keys Args((void));
short command_line Args((CHARTYPE *,bool));
void cleanup_command_line Args((void));
void split_command Args((CHARTYPE *,CHARTYPE *,CHARTYPE *));
short param_split Args((CHARTYPE *,CHARTYPE *[],short,CHARTYPE *,CHARTYPE,CHARTYPE *,bool));
short command_split Args((CHARTYPE *,CHARTYPE *[],short,CHARTYPE *,CHARTYPE *));
LINETYPE get_true_line Args((bool));
CHARTYPE next_char Args((_LINE *,long *,LENGTHTYPE));
short add_define Args((DEFINE **,DEFINE **,int,CHARTYPE *,bool,CHARTYPE *,CHARTYPE));
short remove_define Args((DEFINE **,DEFINE **,int,CHARTYPE *));
short append_define Args((DEFINE **,DEFINE **,int,short,CHARTYPE *,CHARTYPE *,int,CHARTYPE *,CHARTYPE));
int find_key_value Args((CHARTYPE *));
short find_command Args((CHARTYPE *,bool));
void init_command Args((void));
void add_command Args((CHARTYPE *));
CHARTYPE *get_next_command Args(( short,short ));
bool valid_command_to_save Args((CHARTYPE *));
bool is_tab_col Args((LENGTHTYPE));
LENGTHTYPE find_next_tab_col Args((LENGTHTYPE));
LENGTHTYPE find_prev_tab_col Args((LENGTHTYPE));
short tabs_convert Args((_LINE *,bool,bool,bool));
short convert_hex_strings Args((CHARTYPE *));
short marked_block Args((bool));
short suspend_curses Args((void));
short resume_curses Args((void));
short restore_THE Args((void));
short execute_set_sos_command Args((bool,CHARTYPE *));
short valid_command_type Args((bool,CHARTYPE *));
short allocate_temp_space Args((unsigned short,CHARTYPE));
void free_temp_space Args((CHARTYPE));
#ifdef MSWIN
void init_temp_space Args((void));
#endif
CHARTYPE calculate_actual_row Args((CHARTYPE,short,CHARTYPE,bool));
short get_valid_macro_file_name Args((CHARTYPE *,CHARTYPE *,CHARTYPE *,short *));
bool define_command Args((CHARTYPE *));
int find_key_name Args((CHARTYPE *));
int readv_cmdline Args((CHARTYPE *, WINDOW *, int));
short execute_mouse_commands Args((int));
short validate_n_m Args((CHARTYPE *,short *,short *));
void ResetOrDeleteCUABlock Args(( int ));
short execute_locate Args((CHARTYPE *,bool));

                                                            /* print.c */
#ifdef WIN32
void StartTextPrnt Args((void));
void StopTextPrnt Args((void));
#endif
void print_line Args((bool ,LINETYPE,LINETYPE ,short ,CHARTYPE *,CHARTYPE *,short));
short setprintername Args((char*));
short setfontname Args((char*));
short setfontcpi Args((int));
short setfontlpi Args((int));
short setorient Args((char));
short setpagesize Args((int));
                                                           /* target.c */
short split_change_params Args((CHARTYPE *,CHARTYPE **,CHARTYPE **,TARGET *,LINETYPE *,LINETYPE *));
short parse_target Args((CHARTYPE *,LINETYPE,TARGET *,short,bool,bool,bool));
void initialise_target Args((TARGET *));
void free_target Args((TARGET *));
short find_target Args((TARGET *,LINETYPE,bool,bool));
short find_column_target Args((CHARTYPE *,LENGTHTYPE,TARGET *,LENGTHTYPE,bool,bool));
_LINE *find_named_line Args((CHARTYPE *,LINETYPE *,bool));
short find_string_target Args((_LINE *,RTARGET *));
short find_rtarget_target Args((_LINE *,TARGET *,LINETYPE,LINETYPE,LINETYPE *));
bool find_rtarget_column_target Args((CHARTYPE *,LENGTHTYPE,TARGET *,LENGTHTYPE,LENGTHTYPE,LINETYPE *));
LINETYPE find_next_in_scope Args((VIEW_DETAILS *,_LINE *,LINETYPE,short));
LINETYPE find_last_not_in_scope Args((VIEW_DETAILS *,_LINE *,LINETYPE,short));
short validate_target Args((CHARTYPE *,TARGET *,short,LINETYPE,bool,bool));
void calculate_scroll_values Args((short *,LINETYPE *,LINETYPE *,bool *,bool *,bool *,short));
short find_first_focus_line Args((unsigned short *));
short find_last_focus_line Args((unsigned short *));
CHARTYPE find_unique_char Args((CHARTYPE *));
                                                         /* reserved.c */
RESERVED *add_reserved_line Args((CHARTYPE *,CHARTYPE *,short,short,COLOUR_ATTR *));
RESERVED *find_reserved_line Args((CHARTYPE,bool,ROWTYPE,short,short));
short delete_reserved_line Args((short,short));
#ifdef CTLCHAR
chtype *apply_ctlchar_to_reserved_line Args((RESERVED *));
#endif
                                                              /* box.c */
void box_operations Args((short ,CHARTYPE ,bool ,CHARTYPE ));
                                                          /* execute.c */
short execute_os_command Args((CHARTYPE *,bool ,bool ));
short execute_change_command Args((CHARTYPE *,bool ));
short insert_new_line Args((CHARTYPE *,unsigned short,LINETYPE,LINETYPE,bool,bool,bool,CHARTYPE,bool,bool));
short execute_makecurr Args((LINETYPE));
short execute_shift_command Args((short,short,LINETYPE,LINETYPE,bool,short,bool,bool));
short execute_set_lineflag Args(( unsigned int, unsigned int, unsigned int, LINETYPE, LINETYPE, bool, short ));
short do_actual_change_case Args((LINETYPE, LINETYPE,CHARTYPE,bool,short,LENGTHTYPE,LENGTHTYPE));
short execute_change_case Args((CHARTYPE *,CHARTYPE));
short rearrange_line_blocks Args((CHARTYPE,CHARTYPE,LINETYPE,LINETYPE,LINETYPE,short,VIEW_DETAILS*,VIEW_DETAILS*,bool,LINETYPE *));
short execute_set_point Args((CHARTYPE *,LINETYPE ,bool));
short execute_wrap_word Args((unsigned short));
short execute_split_join Args((short,bool,bool));
short execute_put Args((CHARTYPE *,bool));
short execute_macro Args((CHARTYPE *,bool, short*));
short execute_set_on_off Args((CHARTYPE *,bool *));
short execute_set_row_position Args((CHARTYPE *,short *,short *));
short processable_line Args((VIEW_DETAILS *,LINETYPE,_LINE *));
short execute_expand_compress Args((CHARTYPE *,bool,bool,bool,bool));
short execute_select Args((CHARTYPE *,bool,short));
short execute_move_cursor Args((LENGTHTYPE));
short execute_find_command Args((CHARTYPE *,short));
short execute_modify_command Args((CHARTYPE *));
LENGTHTYPE calculate_rec_len Args((short,LENGTHTYPE,LENGTHTYPE,LINETYPE));
short execute_editv Args((short,bool,CHARTYPE *));
short prepare_dialog Args((CHARTYPE *,bool,CHARTYPE *,bool));
short execute_dialog Args((CHARTYPE *,CHARTYPE *,CHARTYPE *,bool,short,short,CHARTYPE *,short,bool,bool));
short prepare_popup Args((CHARTYPE *));
short execute_popup Args((int, int, int, int , int , int , int , int , CHARTYPE **));
short execute_preserve Args((PRESERVED_VIEW_DETAILS **, PRESERVED_FILE_DETAILS **));
short execute_restore Args((PRESERVED_VIEW_DETAILS **, PRESERVED_FILE_DETAILS **));
                                                          /* default.c */
void set_global_defaults Args((void));
void set_global_look_defaults Args((void));
void set_global_feel_defaults Args((void));
void set_file_defaults Args((FILE_DETAILS *));
void set_view_defaults Args((VIEW_DETAILS *));
short get_profile Args((CHARTYPE *,CHARTYPE *));
short defaults_for_first_file Args((void));
short defaults_for_other_files Args((VIEW_DETAILS *));
short default_file_attributes Args((FILE_DETAILS *));
void set_screen_defaults Args((void));
void set_defaults Args((void));
short set_THE_key_defaults Args((int,int));
short set_XEDIT_key_defaults Args((int,int));
short set_ISPF_key_defaults Args((int,int));
short set_KEDIT_key_defaults Args((int,int));
short construct_default_parsers Args((void));
short destroy_all_parsers Args((void));
short construct_default_parser_mapping Args((void));
CHARTYPE *find_default_parser Args((CHARTYPE *,CHARTYPE *));
                                                             /* edit.c */
void editor Args((void));
int process_key Args((int,bool));
short EditFile Args((CHARTYPE *,bool));
                                                            /* error.c */
void display_error Args((unsigned short ,CHARTYPE *,bool));
void clear_msgline Args((int));
void display_prompt Args((CHARTYPE *));
void expose_msgline Args((void));
                                                             /* file.c */
short get_file Args((CHARTYPE *));
_LINE *read_file Args((FILE *,_LINE *,CHARTYPE *,LINETYPE,LINETYPE,bool));
_LINE *read_fixed_file Args((FILE *,_LINE *,CHARTYPE *,LINETYPE,LINETYPE));
short save_file Args((FILE_DETAILS *,CHARTYPE *,bool,LINETYPE,LINETYPE,LINETYPE *,bool,LENGTHTYPE,LENGTHTYPE,bool,bool,bool));
void increment_alt Args((FILE_DETAILS *));
CHARTYPE *new_filename Args((CHARTYPE *,CHARTYPE *,CHARTYPE *,CHARTYPE *));
short remove_aus_file Args((FILE_DETAILS *));
short free_view_memory Args((bool,bool));
void free_a_view Args((void));
short free_file_memory Args((bool));
short read_directory Args((void));
VIEW_DETAILS *find_file Args((CHARTYPE *,CHARTYPE *));
short execute_command_file Args((FILE *));
short process_command_line Args((CHARTYPE *,short));
CHARTYPE *read_file_into_memory Args((CHARTYPE *,int *));
                                                            /* getch.c */
#if !defined(DOS) && !defined(OS2)
int my_getch  Args((WINDOW *));
#endif
                                                          /* nonansi.c */
short file_readable Args((CHARTYPE *));
short file_writable Args((CHARTYPE *));
short file_exists Args((CHARTYPE *));
short remove_file Args((CHARTYPE *));
short splitpath Args((CHARTYPE *));
#ifndef HAVE_RENAME
short rename Args((CHARTYPE *,CHARTYPE *));
#endif
#ifdef OS2
bool LongFileNames Args((CHARTYPE *));
bool IsPathAndFilenameValid Args((CHARTYPE *));
#endif
_LINE *getclipboard Args((_LINE *));
short setclipboard Args((FILE_DETAILS *,CHARTYPE *,bool,LINETYPE,LINETYPE,LINETYPE *,bool,LENGTHTYPE,LENGTHTYPE,bool,bool));
                                                           /* parser.c */
short parse_line Args((CHARTYPE,FILE_DETAILS *,SHOW_LINE *,short));
short parse_paired_comments Args((CHARTYPE,FILE_DETAILS *));
short construct_parser Args((CHARTYPE *, int, PARSER_DETAILS **,CHARTYPE *,CHARTYPE *));
short destroy_parser Args((PARSER_DETAILS *));
bool find_parser_mapping Args((FILE_DETAILS *, PARSER_MAPPING *));
PARSER_DETAILS *find_auto_parser Args((FILE_DETAILS *));
short parse_reserved_line Args((RESERVED *));
                                                           /* prefix.c */
short execute_prefix_commands Args((void));
void clear_pending_prefix_command Args((THE_PPC *,_LINE *));
THE_PPC *delete_pending_prefix_command Args((THE_PPC *,FILE_DETAILS *,_LINE *));
void add_prefix_command Args((_LINE *,LINETYPE,bool));
short add_prefix_synonym Args((CHARTYPE *,CHARTYPE *));
CHARTYPE *find_prefix_synonym Args((CHARTYPE *));
CHARTYPE *find_prefix_oldname Args((CHARTYPE *));
CHARTYPE *get_prefix_command Args((LINETYPE));
                                                             /* show.c */
void show_heading Args((CHARTYPE));
void show_statarea Args((void));
void clear_statarea Args((void));
void display_filetabs Args((VIEW_DETAILS *));
void build_screen Args((CHARTYPE));
void display_screen Args((CHARTYPE));
void show_marked_block Args((void));
void redraw_window Args((WINDOW *));
void repaint_screen Args((void));
void touch_screen Args((CHARTYPE));
void refresh_screen Args((CHARTYPE));
void redraw_screen Args((CHARTYPE));
bool line_in_view Args((CHARTYPE,LINETYPE));
bool column_in_view Args((CHARTYPE,LENGTHTYPE));
LINETYPE find_next_current_line Args((LINETYPE,short));
short get_row_for_focus_line Args((CHARTYPE,LINETYPE,short));
LINETYPE get_focus_line_in_view Args((CHARTYPE,LINETYPE,unsigned short));
LINETYPE calculate_focus_line Args((LINETYPE,LINETYPE));
char *get_current_position Args((CHARTYPE,LINETYPE *,LENGTHTYPE *));
void calculate_new_column Args((COLTYPE,LENGTHTYPE,LENGTHTYPE,COLTYPE *,LENGTHTYPE *));
short prepare_view Args((CHARTYPE));
short advance_view Args((VIEW_DETAILS *,short));
short force_curses_background Args((void));
short THE_Resize Args((int,int));
                                                           /* scroll.c */
short scroll_page Args((short,LINETYPE,bool));
short scroll_line Args((short,LINETYPE,bool,short));
                                                              /* the.c */
void init_colour_pairs Args((void));
int setup_profile_files Args((CHARTYPE *));
void cleanup Args((void));
int allocate_working_memory Args((void));
char **StringToArgv Args(( int *, char* ));
                                                             /* util.c */
CHARTYPE *ebc2asc Args((CHARTYPE *, int, int, int));
CHARTYPE *asc2ebc Args((CHARTYPE *, int, int, int));
short memreveq Args((CHARTYPE *,CHARTYPE ,short ));
short memrevne Args((CHARTYPE *,CHARTYPE ,short ));
CHARTYPE *meminschr Args((CHARTYPE *,CHARTYPE ,short ,short ,short ));
CHARTYPE *meminsmem Args((CHARTYPE *,CHARTYPE *,short ,short ,short ,short ));
CHARTYPE *memdeln Args((CHARTYPE *,short ,short,short ));
CHARTYPE *strdelchr Args((CHARTYPE *,CHARTYPE));
CHARTYPE *memrmdup Args((CHARTYPE *,short *,CHARTYPE));
CHARTYPE *strrmdup Args((CHARTYPE *,CHARTYPE,bool));
short strzne Args((CHARTYPE *,CHARTYPE ));
CHARTYPE *my_strdup Args((CHARTYPE *));
short memne Args((CHARTYPE *,CHARTYPE ,short ));
short strzrevne Args((CHARTYPE *,CHARTYPE ));
short strzreveq Args((CHARTYPE *,CHARTYPE ));
CHARTYPE *strtrunc Args((CHARTYPE *));
CHARTYPE *MyStrip Args((CHARTYPE *,char,char));
short memfind Args((CHARTYPE *,CHARTYPE *,short ,short ,bool ,bool ,CHARTYPE ,CHARTYPE, short*));
short memcmpi Args((CHARTYPE *,CHARTYPE *,short ));
short my_strcmpi Args((char *,char * ));
CHARTYPE *make_upper Args((CHARTYPE *));
unsigned short equal Args((CHARTYPE *,CHARTYPE *,short ));
short valid_integer Args((CHARTYPE *));
short valid_positive_integer Args((CHARTYPE *));
short strzeq Args((CHARTYPE *,CHARTYPE ));
CHARTYPE *strtrans Args((CHARTYPE *,CHARTYPE,CHARTYPE));
_LINE *add_LINE Args((_LINE *,_LINE *,CHARTYPE *,LENGTHTYPE,SELECTTYPE,bool));
_LINE *append_LINE Args((_LINE *,CHARTYPE *,LENGTHTYPE));
_LINE *delete_LINE Args((_LINE *,_LINE *,_LINE *,short));
void put_string Args((WINDOW *,short ,short ,CHARTYPE *,short));
void put_char Args((WINDOW *,chtype ,CHARTYPE ));
short set_up_windows Args((short));
short draw_divider Args((void));
short create_statusline_window Args((void));
short create_filetabs_window Args((void));
void pre_process_line Args((VIEW_DETAILS *,LINETYPE,_LINE *));
short post_process_line Args((VIEW_DETAILS *,LINETYPE,_LINE *,bool));
bool blank_field Args((CHARTYPE *));
void adjust_marked_lines Args((bool ,LINETYPE ,LINETYPE ));
void adjust_pending_prefix Args((VIEW_DETAILS *,bool ,LINETYPE ,LINETYPE ));
CHARTYPE case_translate Args((CHARTYPE ));
void add_to_recovery_list Args((CHARTYPE *,LENGTHTYPE));
void get_from_recovery_list Args((short ));
void free_recovery_list Args((void));
short my_wmove Args((WINDOW *,short ,short ,short ,short ));
short my_isalphanum Args((CHARTYPE));
short get_row_for_tof_eof Args((short,CHARTYPE));
void set_compare_exact Args((bool));
int search_query_item_array Args((void *, size_t, size_t, const char *, int ));
int split_function_name Args((CHARTYPE *, int *));
char *thetmpnam Args((char *));
VIEW_DETAILS *find_filetab Args(( int ));
VIEW_DETAILS *find_next_file Args((VIEW_DETAILS *,short));

#if THIS_APPEARS_TO_NOT_BE_USED
WINDOW *adjust_window Args((WINDOW *,short ,short ,short ,short ));
#endif

void draw_cursor Args((bool));
short my_wclrtoeol Args((WINDOW *));
short my_wdelch Args((WINDOW *));
short get_word Args((CHARTYPE *,LENGTHTYPE,LENGTHTYPE,LENGTHTYPE *,LENGTHTYPE *));
                                                           /* linked.c */
#if 0
void *ll_add Args((void *,void *,unsigned short ));
void *ll_del Args((void *,void *,short ));
void ll_free Args((void *));
void *ll_find Args((void *,LINETYPE ));
#endif

_LINE *lll_add Args((_LINE *,_LINE *,unsigned short ));
_LINE *lll_del Args((_LINE **,_LINE **,_LINE *,short ));
_LINE *lll_free Args((_LINE *));
_LINE *lll_find Args((_LINE *,_LINE *,LINETYPE,LINETYPE));
_LINE *lll_locate Args((_LINE *,CHARTYPE *));
VIEW_DETAILS *vll_add Args((VIEW_DETAILS *,VIEW_DETAILS *,unsigned short ));
VIEW_DETAILS *vll_del Args((VIEW_DETAILS **,VIEW_DETAILS **,VIEW_DETAILS *,short ));
DEFINE *dll_add Args((DEFINE *,DEFINE *,unsigned short ));
DEFINE *dll_del Args((DEFINE **,DEFINE **,DEFINE *,short ));
DEFINE *dll_free Args((DEFINE *));
THE_PPC *pll_add Args((THE_PPC *,THE_PPC *,unsigned short ));
THE_PPC *pll_del Args((THE_PPC **,THE_PPC **,THE_PPC *,short ));
THE_PPC *pll_free Args((THE_PPC *));
THE_PPC *pll_find Args((THE_PPC *,LINETYPE));
RESERVED *rll_add Args((RESERVED *,RESERVED *,unsigned short ));
RESERVED *rll_del Args((RESERVED **,RESERVED **,RESERVED *,short ));
RESERVED *rll_free Args((RESERVED *));
RESERVED *rll_find Args((RESERVED *,short));
PARSER_DETAILS *parserll_add Args((PARSER_DETAILS *,PARSER_DETAILS *,unsigned short ));
PARSER_DETAILS *parserll_del Args((PARSER_DETAILS **,PARSER_DETAILS **,PARSER_DETAILS *,short ));
PARSER_DETAILS *parserll_free Args((PARSER_DETAILS *));
PARSER_DETAILS *parserll_find Args((PARSER_DETAILS *,CHARTYPE *));
PARSE_KEYWORDS *parse_keywordll_add Args((PARSE_KEYWORDS *,PARSE_KEYWORDS *,unsigned short ));
PARSE_KEYWORDS *parse_keywordll_del Args((PARSE_KEYWORDS **,PARSE_KEYWORDS **,PARSE_KEYWORDS *,short ));
PARSE_KEYWORDS *parse_keywordll_free Args((PARSE_KEYWORDS *));
PARSE_FUNCTIONS *parse_functionll_add Args((PARSE_FUNCTIONS *,PARSE_FUNCTIONS *,unsigned short ));
PARSE_FUNCTIONS *parse_functionll_del Args((PARSE_FUNCTIONS **,PARSE_FUNCTIONS **,PARSE_FUNCTIONS *,short ));
PARSE_FUNCTIONS *parse_functionll_free Args((PARSE_FUNCTIONS *));
PARSE_HEADERS *parse_headerll_add Args((PARSE_HEADERS *,PARSE_HEADERS *,unsigned short ));
PARSE_HEADERS *parse_headerll_free Args((PARSE_HEADERS *));
PARSER_MAPPING *mappingll_add Args((PARSER_MAPPING *,PARSER_MAPPING *,unsigned short ));
PARSER_MAPPING *mappingll_del Args((PARSER_MAPPING **,PARSER_MAPPING **,PARSER_MAPPING *,short ));
PARSER_MAPPING *mappingll_free Args((PARSER_MAPPING *));
PARSER_MAPPING *mappingll_find Args((PARSER_MAPPING *,CHARTYPE *,CHARTYPE *));
PARSE_COMMENTS *parse_commentsll_add Args((PARSE_COMMENTS *,PARSE_COMMENTS *,unsigned short ));
PARSE_COMMENTS *parse_commentsll_del Args((PARSE_COMMENTS **,PARSE_COMMENTS **,PARSE_COMMENTS *,short ));
PARSE_COMMENTS *parse_commentsll_free Args((PARSE_COMMENTS *));
PARSE_COMMENTS *parse_commentsll_find Args((PARSE_COMMENTS *,CHARTYPE *));
PARSE_POSTCOMPARE *parse_postcomparell_add Args((PARSE_POSTCOMPARE *,PARSE_POSTCOMPARE *,unsigned short ));
PARSE_POSTCOMPARE *parse_postcomparell_del Args((PARSE_POSTCOMPARE **,PARSE_POSTCOMPARE **,PARSE_POSTCOMPARE *,short ));
PARSE_POSTCOMPARE *parse_postcomparell_free Args((PARSE_POSTCOMPARE *));
                                                             /* rexx.c */
unsigned long MyRexxRegisterFunctionExe Args((CHARTYPE *));
unsigned long MyRexxDeregisterFunction Args((CHARTYPE *));
short initialise_rexx Args((void));
short finalise_rexx Args((void));
short execute_macro_file Args((CHARTYPE *,CHARTYPE *,short *,bool));
short execute_macro_instore Args((CHARTYPE *,short *,CHARTYPE **,int *,int *,int));
short get_rexx_variable Args((CHARTYPE *,CHARTYPE **,int *));
short set_rexx_variable Args((CHARTYPE *,CHARTYPE *,short,short));
CHARTYPE *get_rexx_interpreter_version Args((CHARTYPE *));
                                                           /* os2eas.c */
#if defined(OS2)
bool ReadEAs Args((CHARTYPE *));
bool WriteEAs Args((CHARTYPE *));
#endif
                                                            /* query.c */
short find_query_item Args((CHARTYPE *,int,CHARTYPE *));
short show_status Args((void));
short save_status Args((CHARTYPE *));
short set_extract_variables Args((short));
short get_number_dynamic_items Args((int));
short get_item_values Args((int,short,CHARTYPE *,CHARTYPE,LINETYPE,CHARTYPE *,LINETYPE));
int number_query_item Args(( void ));
int number_function_item Args(( void ));
void format_options Args(( CHARTYPE * ));
                                                         /* directry.c */
short set_dirtype Args((CHARTYPE *));
CHARTYPE *get_dirtype Args((CHARTYPE *));
                                                          /* thematch.c */
int thematch Args((CHARTYPE *,CHARTYPE *,int));
                                                             /* sort.c */
short execute_sort Args((CHARTYPE *));
                                                           /* cursor.c */
short THEcursor_cmdline Args((short));
short THEcursor_column Args((void));
short THEcursor_down Args((short));
short THEcursor_file Args((bool,LINETYPE,LENGTHTYPE));
short THEcursor_home Args((bool));
short THEcursor_left Args((short,bool));
short THEcursor_right Args((short,bool));
short THEcursor_up Args((short));
short THEcursor_move Args((bool,bool,short,short));
short THEcursor_goto Args((LINETYPE,COLTYPE));
short THEcursor_mouse Args((void));
long where_now Args((void));
long what_current_now Args((void));
long what_other_now Args((void));
long where_next Args((long,long,long));
long where_before Args((long,long,long));
bool enterable_field Args((long));
short go_to_new_field Args((long,long));
void get_cursor_position Args((long*,long*,long*,long*));
short advance_focus_line Args((LINETYPE));
short advance_current_line Args((LINETYPE));
short advance_current_or_focus_line Args((LINETYPE));
void resolve_current_and_focus_lines Args((VIEW_DETAILS *, LINETYPE, LINETYPE , short, bool , bool));
                                                           /* colour.c */
short parse_colours Args((CHARTYPE *,COLOUR_ATTR *,CHARTYPE **,bool,bool*));
short parse_modifiers Args((CHARTYPE *,COLOUR_ATTR *));
chtype merge_curline_colour Args((COLOUR_ATTR *, COLOUR_ATTR *));
void set_up_default_colours Args((FILE_DETAILS *,COLOUR_ATTR *,int));
void set_up_default_ecolours Args((FILE_DETAILS *));
CHARTYPE *get_colour_strings Args((COLOUR_ATTR *));
                                                           /* column.c */
short column_command Args((CHARTYPE *,int));
                                                            /* mouse.c */
#if defined(NCURSES_MOUSE_VERSION)
void wmouse_position Args((WINDOW *, int *, int *));
#endif
short THEMouse Args((CHARTYPE *));
short get_mouse_info Args((int *,int *,int *));
void which_window_is_mouse_in Args((CHARTYPE *,int *));
void reset_saved_mouse_pos Args((void));
void get_saved_mouse_pos Args((int *, int *));
void initialise_mouse_commands Args((void));
int mouse_info_to_key Args((int,int,int,int));
CHARTYPE *mouse_key_number_to_name Args((int,CHARTYPE *));
int find_mouse_key_value Args((CHARTYPE *,CHARTYPE *));
short ScrollbarHorz Args((CHARTYPE *));
short ScrollbarVert Args((CHARTYPE *));
                                                           /* memory.c */
void init_memory_table Args(( void ));
void free_memory_flists Args(( void ));
void *get_a_block Args(( int ));
void give_a_block Args(( void * ));
void *resize_a_block Args(( void *, int ));
void the_free_flists  Args(( void ));
                                                           /* single.c */
int initialise_fifo Args(( _LINE *first_file_name, LINETYPE startup_line, LENGTHTYPE startup_column, bool ro ));
int process_fifo_input Args(( int key ));
void close_fifo Args(( void ));
                                                            /* comm*.c */
short Add Args((CHARTYPE *));
short Alert Args((CHARTYPE *));
short All Args((CHARTYPE *));
short Alt Args((CHARTYPE *));
short Arbchar Args((CHARTYPE *));
short Autocolour Args((CHARTYPE *));
short Autosave Args((CHARTYPE *));
short Autoscroll Args((CHARTYPE *));
short Backup Args((CHARTYPE *));
short Backward Args((CHARTYPE *));
short BeepSound Args((CHARTYPE *));
short Bottom Args((CHARTYPE *));
short Boundmark Args((CHARTYPE *));
short Cappend Args((CHARTYPE *));
short Cancel Args((CHARTYPE *));
short Case Args((CHARTYPE *));
short Ccancel Args((CHARTYPE *));
short Cdelete Args((CHARTYPE *));
short Cfirst Args((CHARTYPE *));
short Change Args((CHARTYPE *));
short Cinsert Args((CHARTYPE *));
short Clast Args((CHARTYPE *));
short Clearerrorkey Args((CHARTYPE *));
short Clearscreen Args((CHARTYPE *));
short Clocate Args((CHARTYPE *));
short Clock Args((CHARTYPE *));
short Cmatch Args((CHARTYPE *));
short Cmdarrows Args((CHARTYPE *));
short Cmdline Args((CHARTYPE *));
short Cmsg Args((CHARTYPE *));
short Colour Args((CHARTYPE *));
short Colouring Args((CHARTYPE *));
short Compat Args((CHARTYPE *));
short Compress Args((CHARTYPE *));
short THECommand Args((CHARTYPE *));
short ControlChar Args((CHARTYPE *));
short Copy Args((CHARTYPE *));
short Coverlay Args((CHARTYPE *));
short Creplace Args((CHARTYPE *));
short Ctlchar Args((CHARTYPE *));
short Curline Args((CHARTYPE *));
short Cursor Args((CHARTYPE *));
short CursorStay Args((CHARTYPE *));
short Define Args((CHARTYPE *));
short Defsort Args((CHARTYPE *));
short DeleteLine Args((CHARTYPE *));
short Dialog Args((CHARTYPE *));
short Directory Args((CHARTYPE *));
short Dirinclude Args((CHARTYPE *));
short Display Args((CHARTYPE *));
short Duplicate Args((CHARTYPE *));
short Ecolour Args((CHARTYPE *));
short Emsg Args((CHARTYPE *));
short THEEditv Args((CHARTYPE *));
short Enter Args((CHARTYPE *));
short Eolout Args((CHARTYPE *));
short Equivchar Args((CHARTYPE *));
short Etmode Args((CHARTYPE *));
short Expand Args((CHARTYPE *));
short Extract Args((CHARTYPE *));
short Ffile Args((CHARTYPE *));
short File Args((CHARTYPE *));
short Filetabs Args((CHARTYPE *));
short Fillbox Args((CHARTYPE *));
short Find Args((CHARTYPE *));
short Findup Args((CHARTYPE *));
short Fext Args((CHARTYPE *));
short Filename Args((CHARTYPE *));
short Fmode Args((CHARTYPE *));
short Fname Args((CHARTYPE *));
short Forward Args((CHARTYPE *));
short Fpath Args((CHARTYPE *));
short Fullfname Args((CHARTYPE *));
short Get Args((CHARTYPE *));
short THEHeader Args((CHARTYPE *));
short Help Args((CHARTYPE *));
short Hex Args((CHARTYPE *));
short Hexdisplay Args((CHARTYPE *));
short Hexshow Args((CHARTYPE *));
short Highlight Args((CHARTYPE *));
short Hit Args((CHARTYPE *));
short Idline Args((CHARTYPE *));
short Impmacro Args((CHARTYPE *));
short Impos Args((CHARTYPE *));
short Input Args((CHARTYPE *));
short Inputmode Args((CHARTYPE *));
short Insertmode Args((CHARTYPE *));
short THEInterface Args((CHARTYPE *));
short Join Args((CHARTYPE *));
short Left Args((CHARTYPE *));
short Lineflag Args((CHARTYPE *));
short Linend Args((CHARTYPE *));
short Locate Args((CHARTYPE *));
short Lowercase Args((CHARTYPE *));
short Macro Args((CHARTYPE *));
short SetMacro Args((CHARTYPE *));
short Macroext Args((CHARTYPE *));
short Macropath Args((CHARTYPE *));
short Margins Args((CHARTYPE *));
short Mark Args((CHARTYPE *));
short Modify Args((CHARTYPE *));
short Mouse Args((CHARTYPE *));
short THEMove Args((CHARTYPE *));
short Msg Args((CHARTYPE *));
short Msgline Args((CHARTYPE *));
short Msgmode Args((CHARTYPE *));
short Newlines Args((CHARTYPE *));
short THENext Args((CHARTYPE *));
short Nextwindow Args((CHARTYPE *));
short Nfind Args((CHARTYPE *));
short Nfindup Args((CHARTYPE *));
short Nomsg Args((CHARTYPE *));
short Nondisp Args((CHARTYPE *));
short Nop Args((CHARTYPE *));
short Number Args((CHARTYPE *));
short Overlaybox Args((CHARTYPE *));
short Os Args((CHARTYPE *));
short Osnowait Args((CHARTYPE *));
short Osquiet Args((CHARTYPE *));
short Osredir Args((CHARTYPE *));
short Pagewrap Args((CHARTYPE *));
short Parser Args((CHARTYPE *));
short Pending Args((CHARTYPE *));
short Point Args((CHARTYPE *));
short Popup Args((CHARTYPE *));
short Position Args((CHARTYPE *));
short Prefix Args((CHARTYPE *));
short Preserve Args((CHARTYPE *));
short Prevwindow Args((CHARTYPE *));
short Print Args((CHARTYPE *));
short Pscreen Args((CHARTYPE *));
short THEPrinter Args((CHARTYPE *));
short Put Args((CHARTYPE *));
short Putd Args((CHARTYPE *));
short Qquit Args((CHARTYPE *));
short Quit Args((CHARTYPE *));
short Query Args((CHARTYPE *));
short THEReadonly Args((CHARTYPE *));
short Readv Args((CHARTYPE *));
short Recover Args((CHARTYPE *));
short Reexecute Args((CHARTYPE *));
short Redit Args((CHARTYPE *));
short Redraw Args((CHARTYPE *));
short THERefresh Args((CHARTYPE *));
short Regexp Args((CHARTYPE *));
short Repeat Args((CHARTYPE *));
short Replace Args((CHARTYPE *));
short Reprofile Args((CHARTYPE *));
short Reserved Args((CHARTYPE *));
short Reset Args((CHARTYPE *));
short Restore Args((CHARTYPE *));
short Retrieve Args((CHARTYPE *));
short Rexxoutput Args((CHARTYPE *));
short THERexx Args((CHARTYPE *));
short Rgtleft Args((CHARTYPE *));
short Right Args((CHARTYPE *));
short Save Args((CHARTYPE *));
short Scope Args((CHARTYPE *));
short Scale Args((CHARTYPE *));
short Select Args((CHARTYPE *));
short Set Args((CHARTYPE *));
short Schange Args((CHARTYPE *));
short Slk Args((CHARTYPE *));
short THEScreen Args((CHARTYPE *));
short Shadow Args((CHARTYPE *));
short Shift Args((CHARTYPE *));
short ShowKey Args((CHARTYPE *));
short Sort Args((CHARTYPE *));
short Sos Args((CHARTYPE *));
short Sos_addline Args((CHARTYPE *));
short Sos_blockend Args((CHARTYPE *));
short Sos_blockstart Args((CHARTYPE *));
short Sos_bottomedge Args((CHARTYPE *));
short Sos_cuadelback Args((CHARTYPE *));
short Sos_cuadelchar Args((CHARTYPE *));
short Sos_current Args((CHARTYPE *));
short Sos_cursoradj Args((CHARTYPE *));
short Sos_cursorshift Args((CHARTYPE *));
short Sos_delback Args((CHARTYPE *));
short Sos_delchar Args((CHARTYPE *));
short Sos_delend Args((CHARTYPE *));
short Sos_delline Args((CHARTYPE *));
short Sos_delword Args((CHARTYPE *));
short Sos_doprefix Args((CHARTYPE *));
short Sos_edit Args((CHARTYPE *));
short Sos_endchar Args((CHARTYPE *));
short Sos_execute Args((CHARTYPE *));
short Sos_firstchar Args((CHARTYPE *));
short Sos_firstcol Args((CHARTYPE *));
short Sos_instab Args((CHARTYPE *));
short Sos_lastcol Args((CHARTYPE *));
short Sos_leftedge Args((CHARTYPE *));
short Sos_makecurr Args((CHARTYPE *));
short Sos_marginl Args((CHARTYPE *));
short Sos_marginr Args((CHARTYPE *));
short Sos_pastecmdline Args((CHARTYPE *));
short Sos_parindent Args((CHARTYPE *));
short Sos_prefix Args((CHARTYPE *));
short Sos_qcmnd Args((CHARTYPE *));
short Sos_rightedge Args((CHARTYPE *));
short Sos_settab Args((CHARTYPE *));
short Sos_startendchar Args((CHARTYPE *));
short Sos_tabb Args((CHARTYPE *));
short Sos_tabf Args((CHARTYPE *));
short Sos_tabfieldb Args((CHARTYPE *));
short Sos_tabfieldf Args((CHARTYPE *));
short Sos_tabwordb Args((CHARTYPE *));
short Sos_tabwordf Args((CHARTYPE *));
short Sos_topedge Args((CHARTYPE *));
short Sos_undo Args((CHARTYPE *));
short Span Args((CHARTYPE *));
short Spill Args((CHARTYPE *));
short Split Args((CHARTYPE *));
short Spltjoin Args((CHARTYPE *));
short Ssave Args((CHARTYPE *));
short Statopt Args((CHARTYPE *));
short Status Args((CHARTYPE *));
short Statusline Args((CHARTYPE *));
short Stay Args((CHARTYPE *));
short Suspend Args((CHARTYPE *));
short Synonym Args((CHARTYPE *));
short Tabfile Args((CHARTYPE *));
short Tabkey Args((CHARTYPE *));
short Tabline Args((CHARTYPE *));
short Tabpre Args((CHARTYPE *));
short Tabs Args((CHARTYPE *));
short Tabsin Args((CHARTYPE *));
short Tabsout Args((CHARTYPE *));
short Tag Args((CHARTYPE *));
short Targetsave Args((CHARTYPE *));
short Text Args((CHARTYPE *));
short THighlight Args((CHARTYPE *));
short Timecheck Args((CHARTYPE *));
short Toascii Args((CHARTYPE *));
short Tofeof Args((CHARTYPE *));
short Top Args((CHARTYPE *));
short Trailing Args((CHARTYPE *));
short Trunc Args((CHARTYPE *));
short THETypeahead Args((CHARTYPE *));
short Undoing Args((CHARTYPE *));
short Untaa Args((CHARTYPE *));
short Up Args((CHARTYPE *));
short Uppercase Args((CHARTYPE *));
short Verify Args((CHARTYPE *));
short Width Args((CHARTYPE *));
short _Word Args((CHARTYPE *));
short Wordwrap Args((CHARTYPE *));
short Wrap Args((CHARTYPE *));
short Xedit Args((CHARTYPE *));
short Xterminal Args((CHARTYPE *));
short Zone Args((CHARTYPE *));
short Birthday Args((CHARTYPE *));
