/*
 * This software is Copyright (c) 1989, 1990, 1991 by Patrick J. Wolfe.
 *
 * Permission is hereby granted to copy, distribute or otherwise
 * use any part of this package as long as you do not try to make
 * money from it or pretend that you wrote it.  This copyright
 * notice must be maintained in any copy made.
 *
 * Use of this software constitutes acceptance for use in an AS IS
 * condition. There are NO warranties with regard to this software.
 * In no event shall the author be liable for any damages whatsoever
 * arising out of or in connection with the use or performance of this
 * software.  Any use of this software is at the user's own risk.
 *
 * If you make modifications to this software that you feel
 * increases it usefulness for the rest of the community, please
 * email the changes, enhancements, bug fixes as well as any and
 * all ideas to me. This software is going to be maintained and
 * enhanced as deemed necessary by the community.
 *
 *              Patrick J. Wolfe
 *              uunet!uiucuxc!kailand!pwolfe
 *              pwolfe@kailand.kai.com
 *
 * Additions of DOS keys and extended function key descriptions
 * made by Mark Hessling.
 *
 */

/* System V curses key names and codes returned by getch */

/*
$Id: getch.h,v 1.2 2001/04/21 05:30:53 mark Exp $
*/


#ifndef KEY_MIN
#define KEY_MIN         0x101           /* Minimum curses key */
#endif
#ifndef KEY_BREAK
#define KEY_BREAK	0x101	/* Not on PC KBD		 */
#endif
#ifndef KEY_DOWN
#define KEY_DOWN	0x102	/* Down arrow key		 */
#endif
#ifndef KEY_UP
#define KEY_UP		0x103	/* Up arrow key			 */
#endif
#ifndef KEY_LEFT
#define KEY_LEFT	0x104	/* Left arrow key		 */
#endif
#ifndef KEY_RIGHT
#define KEY_RIGHT	0x105	/* Right arrow key		 */
#endif
#ifndef KEY_HOME
#define KEY_HOME	0x106	/* home key			 */
#endif
#ifndef KEY_BACKSPACE
#define KEY_BACKSPACE	0x107	/* not on pc			 */
#endif
#ifndef KEY_F0
#define KEY_F0		0x108	/* function keys. space for	 */
#define KEY_F(n)    (KEY_F0+(n))/* 64 keys are reserved.	 */
#endif
#ifndef KEY_DL
#define KEY_DL		0x148	/* not on pc			 */
#endif
#ifndef KEY_IL
#define KEY_IL		0x149	/* insert line			 */
#endif
#ifndef KEY_DC
#define KEY_DC		0x14a	/* delete character		 */
#endif
#ifndef KEY_IC
#define KEY_IC		0x14b	/* insert char or enter ins mode */
#endif
#ifndef KEY_EIC
#define KEY_EIC		0x14c	/* exit insert char mode	 */
#endif
#ifndef KEY_CLEAR
#define KEY_CLEAR	0x14d	/* clear screen			 */
#endif
#ifndef KEY_EOS
#define KEY_EOS		0x14e	/* clear to end of screen	 */
#endif
#ifndef KEY_EOL
#define KEY_EOL		0x14f	/* clear to end of line		 */
#endif
#ifndef KEY_SF
#define KEY_SF		0x150	/* scroll 1 line forward	 */
#endif
#ifndef KEY_SR
#define KEY_SR		0x151	/* scroll 1 line back (reverse)	 */
#endif
#ifndef KEY_NPAGE
#define KEY_NPAGE	0x152	/* next page			 */
#endif
#ifndef KEY_PPAGE
#define KEY_PPAGE	0x153	/* previous page		 */
#endif
#ifndef KEY_STAB
#define KEY_STAB	0x154	/* set tab			 */
#endif
#ifndef KEY_CTAB
#define KEY_CTAB	0x155	/* clear tab			 */
#endif
#ifndef KEY_CATAB
#define KEY_CATAB	0x156	/* clear all tabs		 */
#endif
#ifndef KEY_ENTER
#define KEY_ENTER	0x157	/* enter or send (unreliable)	 */
#endif
#ifndef KEY_SRESET
#define KEY_SRESET	0x158	/* soft/reset (partial/unreliable)*/
#endif
#ifndef KEY_RESET
#define KEY_RESET	0x159	/* reset/hard reset (unreliable) */
#endif
#ifndef KEY_PRINT
#define KEY_PRINT	0x15a	/* print/copy			 */
#endif
#ifndef KEY_LL
#define KEY_LL		0x15b	/* home down/bottom (lower left) */
#endif
#ifndef KEY_ABORT
#define KEY_ABORT	0x15c	/* abort/terminate key (any)	 */
#endif
#ifndef KEY_SHELP
#define KEY_SHELP	0x15d	/* short help			 */
#endif
#ifndef KEY_LHELP
#define KEY_LHELP	0x15e	/* long help			 */
#endif
#ifndef KEY_BTAB 
#define KEY_BTAB        0x15f            /* Back tab key */
#endif
#ifndef KEY_BEG  
#define KEY_BEG         0x160            /* beg(inning) key */
#endif
#ifndef KEY_CANCEL
#define KEY_CANCEL      0x161             /* cancel key */
#endif
#ifndef KEY_CLOSE 
#define KEY_CLOSE       0x162             /* close key */
#endif
#ifndef KEY_COMMAND
#define KEY_COMMAND     0x163             /* cmd (command) key */
#endif
#ifndef KEY_COPY   
#define KEY_COPY        0x164             /* copy key */
#endif
#ifndef KEY_CREATE 
#define KEY_CREATE      0x165             /* create key */
#endif
#ifndef KEY_END    
#define KEY_END         0x166             /* end key */
#endif
#ifndef KEY_EXIT   
#define KEY_EXIT        0x167             /* exit key */
#endif
#ifndef KEY_FIND   
#define KEY_FIND        0x168             /* find key */
#endif
#ifndef KEY_HELP   
#define KEY_HELP        0x169             /* help key */
#endif
#ifndef KEY_MARK   
#define KEY_MARK        0x16a             /* mark key */
#endif
#ifndef KEY_MESSAGE
#define KEY_MESSAGE     0x16b             /* message key */
#endif
#ifndef KEY_MOVE   
#define KEY_MOVE        0x16c             /* move key */
#endif
#ifndef KEY_NEXT   
#define KEY_NEXT        0x16d             /* next object key */
#endif
#ifndef KEY_OPEN   
#define KEY_OPEN        0x16e             /* open key */
#endif
#ifndef KEY_OPTIONS
#define KEY_OPTIONS     0x16f             /* options key */
#endif
#ifndef KEY_PREVIOUS
#define KEY_PREVIOUS    0x170             /* previous object key */
#endif
#ifndef KEY_REDO    
#define KEY_REDO        0x171             /* redo key */
#endif
#ifndef KEY_REFERENCE
#define KEY_REFERENCE   0x172             /* ref(erence) key */
#endif
#ifndef KEY_REFRESH  
#define KEY_REFRESH     0x173             /* refresh key */
#endif
#ifndef KEY_REPLACE  
#define KEY_REPLACE     0x174             /* replace key */
#endif
#ifndef KEY_RESTART  
#define KEY_RESTART     0x175             /* restart key */
#endif
#ifndef KEY_RESUME   
#define KEY_RESUME      0x176             /* resume key */
#endif
#ifndef KEY_SAVE     
#define KEY_SAVE        0x177             /* save key */
#endif
#ifndef KEY_SBEG     
#define KEY_SBEG        0x178             /* shifted beginning key */
#endif
#ifndef KEY_SCANCEL  
#define KEY_SCANCEL     0x179             /* shifted cancel key */
#endif
#ifndef KEY_SCOMMAND 
#define KEY_SCOMMAND    0x17a             /* shifted command key */
#endif
#ifndef KEY_SCOPY    
#define KEY_SCOPY       0x17b             /* shifted copy key */
#endif
#ifndef KEY_SCREATE  
#define KEY_SCREATE     0x17c             /* shifted create key */
#endif
#ifndef KEY_SDC      
#define KEY_SDC         0x17d             /* shifted delete char key */
#endif
#ifndef KEY_SDL      
#define KEY_SDL         0x17e             /* shifted delete line key */
#endif
#ifndef KEY_SELECT   
#define KEY_SELECT      0x17f             /* select key */
#endif
#ifndef KEY_SEND     
#define KEY_SEND        0x180             /* shifted end key */
#endif
#ifndef KEY_SEOL     
#define KEY_SEOL        0x181             /* shifted clear line key */
#endif
#ifndef KEY_SEXIT    
#define KEY_SEXIT       0x182             /* shifted exit key */
#endif
#ifndef KEY_SFIND    
#define KEY_SFIND       0x183             /* shifted find key */
#endif
#ifndef KEY_SHOME    
#define KEY_SHOME       0x184             /* shifted home key */
#endif
#ifndef KEY_SIC      
#define KEY_SIC         0x185             /* shifted input key */
#endif
#ifndef KEY_SLEFT    
#define KEY_SLEFT       0x187             /* shifted left arrow key */
#endif
#ifndef KEY_SMESSAGE 
#define KEY_SMESSAGE    0x188             /* shifted message key */
#endif
#ifndef KEY_SMOVE    
#define KEY_SMOVE       0x189             /* shifted move key */
#endif
#ifndef KEY_SNEXT    
#define KEY_SNEXT       0x18a             /* shifted next key */
#endif
#ifndef KEY_SOPTIONS 
#define KEY_SOPTIONS    0x18b             /* shifted options key */
#endif
#ifndef KEY_SPREVIOUS
#define KEY_SPREVIOUS   0x18c             /* shifted prev key */
#endif
#ifndef KEY_SPRINT   
#define KEY_SPRINT      0x18d             /* shifted print key */
#endif
#ifndef KEY_SREDO    
#define KEY_SREDO       0x18e             /* shifted redo key */
#endif
#ifndef KEY_SREPLACE 
#define KEY_SREPLACE    0x18f             /* shifted replace key */
#endif
#ifndef KEY_SRIGHT   
#define KEY_SRIGHT      0x190             /* shifted right arrow */
#endif
#ifndef KEY_SRSUME   
#define KEY_SRSUME      0x191             /* shifted resume key */
#endif
#ifndef KEY_SSAVE    
#define KEY_SSAVE       0x192             /* shifted save key */
#endif
#ifndef KEY_SSUSPEND 
#define KEY_SSUSPEND    0x193             /* shifted suspend key */
#endif
#ifndef KEY_SUNDO    
#define KEY_SUNDO       0x194             /* shifted undo key */
#endif
#ifndef KEY_SUSPEND  
#define KEY_SUSPEND     0x195             /* suspend key */
#endif
#ifndef KEY_UNDO     
#define KEY_UNDO        0x196             /* undo key */
#endif
#ifndef KEY_A1       
#define KEY_A1          0x197             /* Upper left of keypad */
#endif
#ifndef KEY_A2
#define KEY_A2          0x198             /* Upper centre of keypad */
#endif
#ifndef KEY_A3       
#define KEY_A3          0x199             /* Upper right of keypad */
#endif
#ifndef KEY_B1
#define KEY_B1          0x19a             /* Middle left of keypad */
#endif
#ifndef KEY_B2       
#define KEY_B2          0x19b             /* Middle centre of keypad */
#endif
#ifndef KEY_B3
#define KEY_B3          0x19c             /* Middle right of keypad */
#endif
#ifndef KEY_C1       
#define KEY_C1          0x19d             /* Lower left of keypad */
#endif
#ifndef KEY_C2
#define KEY_C2          0x19e             /* Lower centre of keypad */
#endif
#ifndef KEY_C3       
#define KEY_C3          0x19f             /* Lower right of keypad */
#endif
#ifndef KEY_MAX      
#define KEY_MAX         0xfff             /* Maximum curses key */
#endif
  
#define KEY_DELETE  127

#if !defined(DOS) && !defined(OS2) && !defined(XCURSES) && !defined(WIN32)

/* vt220 special keys */

#define CTL_LEFT       0x302
#define CTL_RIGHT      0x303
#define CTL_UP         0x304
#define CTL_DOWN       0x305
#define CTL_HOME       0x306
#define CTL_END        0x307
#define CTL_PGUP       0x308
#define CTL_PGDN       0x309

# ifndef KEY_RETURN
#  define KEY_RETURN     KEY_ENTER
# endif

# ifndef KEY_NUMENTER
#  define KEY_NUMENTER   KEY_ENTER
# endif

#endif

#if defined(DOS) || defined(OS2) || defined(XCURSES) || defined(WIN32)
/* normal characters */
#define KEY_a      0x61
#define KEY_b      0x62
#define KEY_c      0x63
#define KEY_d      0x64
#define KEY_e      0x65
#define KEY_f      0x66
#define KEY_g      0x67
#define KEY_h      0x68
#define KEY_i      0x69
#define KEY_j      0x6a
#define KEY_k      0x6b
#define KEY_l      0x6c
#define KEY_m      0x6d
#define KEY_n      0x6e
#define KEY_o      0x6f
#define KEY_p      0x70
#define KEY_q      0x71
#define KEY_r      0x72
#define KEY_s      0x73
#define KEY_t      0x74
#define KEY_u      0x75
#define KEY_v      0x76
#define KEY_w      0x77
#define KEY_x      0x78
#define KEY_y      0x79
#define KEY_z      0x7a
#define KEY_0      0x30
#define KEY_1      0x31
#define KEY_2      0x32
#define KEY_3      0x33
#define KEY_4      0x34
#define KEY_5      0x35
#define KEY_6      0x36
#define KEY_7      0x37
#define KEY_8      0x38
#define KEY_9      0x39
#define KEY_BQUOTE      0x60
#define KEY_MINUS      0x2d
#define KEY_EQUAL      0x3d
#define KEY_LBRACKET      0x5b
#define KEY_RBRACKET      0x5d
#define KEY_BSLASH      0x5c
#define KEY_SEMICOLON      0x3b
#define KEY_FQUOTE      0x27
#define KEY_COMMA      0x2c
#define KEY_STOP      0x2e
#define KEY_FSLASH      0x2f
#define KEY_SPACE      0x20
#define KEY_RETURN     0xd
#define KEY_TAB      0x9
#define KEY_BKSP      0x8
#define KEY_PGUP      KEY_PPAGE
#define KEY_PGDN      KEY_NPAGE
#define KEY_CURU      KEY_UP
#define KEY_CURD      KEY_DOWN
#define KEY_CURL      KEY_LEFT
#define KEY_CURR      KEY_RIGHT
#define KEY_INS      KEY_IC
#define KEY_DEL      KEY_DC
#define KEY_NUMENTER PADENTER
/* shifted characters */
#define KEY_S_a      0x41
#define KEY_S_b      0x42
#define KEY_S_c      0x43
#define KEY_S_d      0x44
#define KEY_S_e      0x45
#define KEY_S_f      0x46
#define KEY_S_g      0x47
#define KEY_S_h      0x48
#define KEY_S_i      0x49
#define KEY_S_j      0x4a
#define KEY_S_k      0x4b
#define KEY_S_l      0x4c
#define KEY_S_m      0x4d
#define KEY_S_n      0x4e
#define KEY_S_o      0x4f
#define KEY_S_p      0x50
#define KEY_S_q      0x51
#define KEY_S_r      0x52
#define KEY_S_s      0x53
#define KEY_S_t      0x54
#define KEY_S_u      0x55
#define KEY_S_v      0x56
#define KEY_S_w      0x57
#define KEY_S_x      0x58
#define KEY_S_y      0x59
#define KEY_S_z      0x5a
#define KEY_S_0      0x29
#define KEY_S_1      0x21
#define KEY_S_2      0x40
#define KEY_S_3      0x23
#define KEY_S_4      0x24
#define KEY_S_5      0x25
#define KEY_S_6      0x5e
#define KEY_S_7      0x26
#define KEY_S_8      0x2a
#define KEY_S_9      0x28
#define KEY_S_BQUOTE      0x7e
#define KEY_S_MINUS      0x5f
#define KEY_S_EQUAL      0x2b
#define KEY_S_LBRACKET      0x7b
#define KEY_S_RBRACKET      0x7d
#define KEY_S_BSLASH      0x7c
#define KEY_S_SEMICOLON      0x3a
#define KEY_S_FQUOTE      0x22
#define KEY_S_COMMA      0x3c
#define KEY_S_STOP      0x3e
#define KEY_S_FSLASH      0x3f
#define KEY_S_TAB     KEY_BTAB
#define KEY_S_CURU    KEY_SUP
#define KEY_S_CURD    KEY_SDOWN
#define KEY_S_INS     KEY_INS
#define KEY_S_DEL     KEY_DEL
#define KEY_S_NUMCENTER  KEY_5
/* control characters */
/* Control-A to Control-Z at end */
#define KEY_C_2      0x0
#define KEY_C_6      0x1e
#define KEY_C_MINUS      0x1f
#define KEY_C_LBRACKET      0x1b
#define KEY_C_RBRACKET      0x1d
#define KEY_C_BSLASH      0x1c
#endif

/* special definition for UNIX systems */
#ifndef KEY_RETURN
#define KEY_RETURN 0xd
#endif

#define CSI          0233
#define KEY_ESC      0x1b

/* control alpha keys */
#define KEY_C_a      0x1
#define KEY_C_b      0x2
#define KEY_C_c      0x3
#define KEY_C_d      0x4
#define KEY_C_e      0x5
#define KEY_C_f      0x6
#define KEY_C_g      0x7
#define KEY_C_h      0x8
#define KEY_C_i      0x9
#define KEY_C_j      0xa
#define KEY_C_k      0xb
#define KEY_C_l      0xc
#define KEY_C_m      0xd
#define KEY_C_n      0xe
#define KEY_C_o      0xf
#define KEY_C_p      0x10
#define KEY_C_q      0x11
#define KEY_C_r      0x12
#define KEY_C_s      0x13
#define KEY_C_t      0x14
#define KEY_C_u      0x15
#define KEY_C_v      0x16
#define KEY_C_w      0x17
#define KEY_C_x      0x18
#define KEY_C_y      0x19
#define KEY_C_z      0x1a

/* unshifted function keys */
#define KEY_F1      KEY_F(1)
#define KEY_F2      KEY_F(2)
#define KEY_F3      KEY_F(3)
#define KEY_F4      KEY_F(4)
#define KEY_F5      KEY_F(5)
#define KEY_F6      KEY_F(6)
#define KEY_F7      KEY_F(7)
#define KEY_F8      KEY_F(8)
#define KEY_F9      KEY_F(9)
#define KEY_F10     KEY_F(10)
#define KEY_F11     KEY_F(11)
#define KEY_F12     KEY_F(12)

/* shifted function keys */
#define KEY_S_F1      KEY_F(13)
#define KEY_S_F2      KEY_F(14)
#define KEY_S_F3      KEY_F(15)
#define KEY_S_F4      KEY_F(16)
#define KEY_S_F5      KEY_F(17)
#define KEY_S_F6      KEY_F(18)
#define KEY_S_F7      KEY_F(19)
#define KEY_S_F8      KEY_F(20)
#define KEY_S_F9      KEY_F(21)
#define KEY_S_F10     KEY_F(22)
#define KEY_S_F11     KEY_F(23)
#define KEY_S_F12     KEY_F(24)

/* control function keys */
#define KEY_C_F1      KEY_F(25)
#define KEY_C_F2      KEY_F(26)
#define KEY_C_F3      KEY_F(27)
#define KEY_C_F4      KEY_F(28)
#define KEY_C_F5      KEY_F(29)
#define KEY_C_F6      KEY_F(30)
#define KEY_C_F7      KEY_F(31)
#define KEY_C_F8      KEY_F(32)
#define KEY_C_F9      KEY_F(33)
#define KEY_C_F10     KEY_F(34)
#define KEY_C_F11     KEY_F(35)
#define KEY_C_F12     KEY_F(36)

/* alt function keys */
#define KEY_A_F1      KEY_F(37)
#define KEY_A_F2      KEY_F(38)
#define KEY_A_F3      KEY_F(39)
#define KEY_A_F4      KEY_F(40)
#define KEY_A_F5      KEY_F(41)
#define KEY_A_F6      KEY_F(42)
#define KEY_A_F7      KEY_F(43)
#define KEY_A_F8      KEY_F(44)
#define KEY_A_F9      KEY_F(45)
#define KEY_A_F10     KEY_F(46)
#define KEY_A_F11     KEY_F(47)
#define KEY_A_F12     KEY_F(48)

/* remaining curses function keys */
#define KEY_F13       KEY_F(49)
#define KEY_F14       KEY_F(50)
#define KEY_F15       KEY_F(51)
#define KEY_F16       KEY_F(52)
#define KEY_F17       KEY_F(53)
#define KEY_F18       KEY_F(54)
#define KEY_F19       KEY_F(55)
#define KEY_F20       KEY_F(56)
#define KEY_S_F13     KEY_F(57)
#define KEY_S_F14     KEY_F(58)
#define KEY_S_F15     KEY_F(59)
#define KEY_S_F16     KEY_F(60)
#define KEY_S_F17     KEY_F(61)
#define KEY_S_F18     KEY_F(62)
#define KEY_S_F19     KEY_F(63)

#define KEY_Find       KEY_FIND
#define KEY_InsertHere KEY_IC
#define KEY_Remove     KEY_REPLACE
#define KEY_Select     KEY_SELECT
#define KEY_PrevScreen KEY_PPAGE
#define KEY_NextScreen KEY_NPAGE
#define KEY_PF1        0x350
#define KEY_PF2        0x351
#define KEY_PF3        0x352
#define KEY_PF4        0x353
#define KEY_PadComma   0x354
#define KEY_PadMinus   0x355
#define KEY_PadPeriod  0x356
#define KEY_PadPlus    0x357
#define KEY_PadStar    0x358
#define KEY_PadSlash   0x359

#ifdef PAD0
# define KEY_Pad0      PAD0
#else
# define KEY_Pad0      0x360
#endif

/* Mouse button definitions */

#define KEY_PB1        0x400
#define KEY_PB2        0x401
#define KEY_PB3        0x402
#define KEY_S_PB1      0x403
#define KEY_S_PB2      0x404
#define KEY_S_PB3      0x405
#define KEY_C_PB1      0x406
#define KEY_C_PB2      0x407
#define KEY_C_PB3      0x408
#define KEY_A_PB1      0x409
#define KEY_A_PB2      0x40a
#define KEY_A_PB3      0x40b

#define KEY_RB1        0x410
#define KEY_RB2        0x411
#define KEY_RB3        0x412
#define KEY_S_RB1      0x413
#define KEY_S_RB2      0x414
#define KEY_S_RB3      0x415
#define KEY_C_RB1      0x416
#define KEY_C_RB2      0x417
#define KEY_C_RB3      0x418
#define KEY_A_RB1      0x419
#define KEY_A_RB2      0x41a
#define KEY_A_RB3      0x41b

#define KEY_DB1        0x420
#define KEY_DB2        0x421
#define KEY_DB3        0x422
#define KEY_S_DB1      0x423
#define KEY_S_DB2      0x424
#define KEY_S_DB3      0x425
#define KEY_C_DB1      0x426
#define KEY_C_DB2      0x427
#define KEY_C_DB3      0x428
#define KEY_A_DB1      0x429
#define KEY_A_DB2      0x42a
#define KEY_A_DB3      0x42b

