;	DEBUG.ASM	NASM/YASM assembler source for a clone of DEBUG.COM
;           Version 1.13, 02/27/2008.

;	To assemble, use:
;		nasm debug.asm -D PM=0 -O 2 -o debug.com
;
;   To create the DPMI aware version of debug, use:
;		nasm debug.asm -D PM=1 -O 2 -o debugx.com

; ============================================================================
;
; Copyright (c) 1995-2003  Paul Vojta
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to
; deal in the Software without restriction, including without limitation the
; rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
; sell copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
; PAUL VOJTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
; IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
; CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;
; ============================================================================
;
; Japheth: all extensions made by me are Public Domain. This does not
;          affect other copyrights.
;          This file is now best viewed with TAB size 4!
; ============================================================================
;	Revision history:
;	    0.95e [11 January 2003]  Fixed a bug in the assember.
;	    0.95f [10 September 2003]  Converted to NASM; fixed some syntax
;		incompatibilities.
;	    0.98 [27 October 2003]  Added EMS commands and copyright conditions.
;
;       The changes which were done by my, japheth, are described in
;       README.TXT.
;
;	To do:
;		*.HEX files
;		make C work with 32bit offsets in protected-mode
;		add MMX instructions for A and U
;		Fix RN in protected mode (other structure of FCS and FOP)
;		allow to modify floating point registers
;		allow a single segment prefix in A (like MS Debug does)

BS		equ	8
TAB		equ	9
LF		equ	10
CR		equ	13
TOLOWER	equ	20h
TOUPPER	equ	0dfh
TOUPPER_W equ 0dfdfh
MNEMONOFS equ 28	;offset in output line where mnemonics start (disassember)

%if PM
%define VDD        1	;try to load DEBXXVDD.DLL
%define NOEXTENDER 1	;don't assume DPMI host includes a DOS extender
%define WIN9XSUPP  1	;avoid to hook DPMI entry when running in Win9x
%define DOSEMU     1	;avoid to hook DPMI entry when running in DosEmu
%define EXCCSIP    1	;display CS:IP where exception occured
%define DISPHOOK   1	;display "DPMI entry hooked..."
%define DBGNAME "DEBUGX"
%define DBGNAME2 "DebugX"
%else
%define VDD 0
%define NOEXTENDER 0
%define DBGNAME "DEBUG"
%define DBGNAME2 "Debug"
%endif

;%define DOSNAME "FreeDOS"
%define DOSNAME	"DOS"
%define MCB 1			;support DM command
%define USESDA 1		;use SDA to get/set PSP in real-mode
%define MMXSUPP	0		;support MMX specific commands

LINE_IN_LEN equ	257		;length of line_in (including header stuff)

;	PSP offsets

ALASAP	equ	02h	;Address of Last segment allocated to program
TPIVOFS	equ	0ah	;Terminate Program Interrupt Vector offset
TPIVSEG	equ	0ch	;Terminate program Interrupt Vector segment
CCIV	equ	0eh	;Control C Interrupt Vector
CEIV	equ	12h	;Critical Error Interrupt Vector
SPSAV	equ	2eh	;Save the stack pointer here
DTA		equ	80h	;Program arguments; also used to store file name (N cmd)

%if 0
	[segment _TEXT]
%else    
	org	100h
%endif    

%if NASM
%macro cpufpu 1
	cpu %1
%endmacro    
%else
%macro cpufpu 1
	cpu %1 fpu
%endmacro    
%endif

%macro mne 3
	dw %1
    db %2, %3
%endmacro

%if VDD
%macro RegisterModule 0
	db 0C4h, 0C4h, 58h, 0
%endmacro
%macro UnRegisterModule 0
	db 0C4h, 0C4h, 58h, 1
%endmacro
%macro DispatchCall 0
	db 0C4h, 0C4h, 58h, 2
%endmacro
%endif

	cpu 8086

	jmp	initcode

cmdlist	dw aa,error,cc,ddd,ee,ff,gg,hh,ii,error,error,ll,mm,nn,oo
		dw pp,qq,rr,sss,tt,uu,error,ww,xx

savesp	dw 0		;save stack pointer
errret	dw 0		;return here if error
run_sp	dw 0		;stack pointer when running
spadjust dw	40h 	;adjust sp by this amount for save
pspdbe  dw 0		;debuggee's program segment prefix
pspdbg  dw 0		;debugger's program segment prefix (always a segment)
run2324	dw 0,0,0,0	;debuggee's interrupt vectors 23 and 24 (both modes)
%if PM
		dw 0,0
dbg2324 dw myi23, myi24
%if VDD
hVdd	dw -1
%endif
%endif
sav2324	dw 0,0,0,0	;debugger's interrupt vectors 23 and 24 (real-mode only)
hakstat	db 0		;whether we have hacked the vectors or not
psp22	dw 0,0		;original terminate address in the PSP
parent	dw 0		;original PSP of process parent (must be next)
%if MCB
wMCB	dw 0		;start of MCB chain
%endif
pInDOS	dd 0		;far16 address of InDOS flag
%if PM
InDosSegm dw 0		;saved segment value of pInDOS
%endif
%if USESDA
pSDA	dd 0		;far16 address of DOS swappable data area
%if PM
SDASegm	dw 0		;saved segment value of pSDA
%endif
%endif
machine	db 0		;type of this processor
regsdmp db 0		;bit 0: 1=386 register display
tmode	db 0		;bit 0: 1=ms-dos compatible trace mode
has_87	db 0		;if there is a math coprocessor present
bInDbg  db 0		;1=debugger is running
mach_87	db 0		;type of coprocessor present
notatty	db LF		;if standard input is from a file
switchar db	0		;switch character
swch1	db ' '		;switch character if it's a slash
promptlen dw 0		;length of prompt
bufnext	dw line_in+2	;address of next available character
bufend	dw line_in+2	;address + 1 of last valid character

a_addr	dw 0,0,0	;address for next A command
d_addr	dw 0,0,0	;address of last D command; must follow a_addr
u_addr	dw 0,0,0	;address of last U command; must follow d_addr
%if PM
x_addr	dd 0		;address of last DX command
%endif
eqflag	db 0		;flag indicating presence of `=' operand
eqladdr	dw 0,0,0	;address of `=' operand in G, P and T command
run_cs	dw 0		;save original CS when running in G
run_int	dw 0		;interrupt type that stopped the running
lastcmd dw dmycmd
bInit	db 0		;0=ensure a valid opcode is at debuggee's CS:IP
fileext	db 0		;file extension (0 if no file name)

EXT_OTHER	equ	1
EXT_COM		equ	2
EXT_EXE		equ	4
EXT_HEX		equ	8

inttab
	db 0
	dw intr0		;table of interrupt initialization stuff
	db 1
	dw intr1
	db 3
	dw intr3
	db 22h
	dw debug22
%if PM
	db 2Fh
%endif

intsave	dd 0		;save area for interrupt vectors 0, 1, 3, 22h, 2Fh
	dd 0
	dd 0
	dd 0
%if PM    
oldi2f	dd 0
%endif

;	Parameter block for exec call.

execblk	dw 0	;(0) copy the parent's environment
	dw 0,0		;(2) address of command tail to copy
	dw 5ch,0	;(6) address of first FCB to copy
	dw 6ch,0	;(10) address of second FCB to copy
	dw 0,0		;(14) initial SS:SP
	dw 0,0		;(18) initial CS:IP

;	Register save area (32 words).
;   must be WORD aligned!

	align 4

regs:
reg_ax	dw 0		;+0  ax
reg_bx	dw 0		;+2  bx
reg_cx	dw 0		;+4  cx
reg_dx	dw 0		;+6  dx
reg_sp	dw 0		;+8  sp
reg_bp	dw 0		;+10 bp
reg_si	dw 0		;+12 si
reg_di	dw 0		;+14 di
reg_ds	dw 0		;+16 ds
reg_es	dw 0		;+18 es
reg_ss	dw 0		;+20 ss
reg_cs	dw 0		;+22 cs
reg_fs	dw 0		;+24 fs
reg_gs	dw 0		;+26 gs

reg_ip	dw 0		;+28 ip
reg_fl	dw 0		;+30 fl(ags)

regshi:
regh_eax dw 0		;+32
regh_ebx dw 0		;+34
regh_ecx dw 0		;+36
regh_edx dw 0		;+38
regh_esp dw 0		;+40
regh_ebp dw 0		;+42
regh_esi dw 0		;+44
regh_edi dw 0		;+46
         dw 0,0,0,0,0,0	;hiwords of segment registers (not used)
regh_eip dw 0		;+60
regh_efl dw 0		;+62

regnames dw	'AX','BX','CX','DX','SP','BP','SI','DI','DS','ES'
	dw	'SS','CS','FS','GS','IP','FL'

;	Instruction set information needed for the 'p' command.

ppbytes	db	66h,67h,26h,2eh,36h,3eh,64h,65h,0f2h,0f3h	;prefixes
	db	0ach,0adh,0aah,0abh,0a4h,0a5h	;lods,stos,movs
	db	0a6h,0a7h,0aeh,0afh		;cmps,scas
	db	6ch,6dh,6eh,6fh			;ins,outs
	db	0cch,0cdh				;int instructions
	db	0e0h,0e1h,0e2h			;loop instructions
	db	0e8h					;call rel16/32
	db	09ah					;call far seg16:16/32
;	(This last one is done explicitly by the code.)
;	db	0ffh			;ff/2 or ff/3:  indirect call

;	Info for the above, respectively.
;	80h = prefix; 82h = operand size prefix; 81h = address size prefix.
;	If the high bit is not set, the next highest bit (40h) indicates that
;	the instruction size depends on whether there is an address size prefix,
;	and the remaining bits tell the number of additional bytes in the
;	instruction.

ppinfo	db	82h,81h,80h,80h,80h,80h,80h,80h,80h,80h
	db	0,0,0,0,0,0
	db	0,0,0,0
	db	0,0,0,0
	db	0,1
	db	1,1,1
	db	42h
	db	44h

PPLEN	equ	31		;size of the above table

;	Strings.

prompt1	db	'-'		;main prompt
prompt2	db	':'		;prompt for register value
%if PM
prompt3	db	'#'		;protected-mode prompt
%endif

helpmsg	db	DOSNAME, ' ', DBGNAME2, ' v1.13 help screen',CR,LF
	db	'assemble',9,	'A [address]',CR,LF
	db	'compare',9,9,	'C range address',CR,LF
	db	'dump',9,9,		'D [range]',CR,LF
%if PM    
	db	'dump interrupt',9,'DI interrupt [count]',CR,LF
	db	'dump LDT',9,	'DL selector [count]',CR,LF
%endif    
%if MCB
	db	'dump MCB chain',9,'DM',CR,LF
%endif
%if PM    
	db	'dump ext memory',9,'DX physical_address',CR,LF
%endif
	db	'enter',9,9,	'E address [list]',CR,LF
	db	'fill',9,9,		'F range list',CR,LF
	db	'go',9,9,		'G [=address] [breakpts]',CR,LF
	db	'hex add/sub',9,'H value1 value2',CR,LF
	db	'input',9,9,	'I[W|D] port',CR,LF
	db	'load program',9,'L [address]',CR,LF
	db	'load sectors',9,'L address drive sector count',CR,LF
	db	'move',9,9,		'M range address',CR,LF
	db	'80x86 mode',9,	'M [x (0..6, ? for query)]',CR,LF
	db	'set FPU mode',9,'MC 387, MNC none, MC2 287',CR,LF
	db	'set name',9,	'N [[drive:][path]progname [arglist]]',CR,LF
	db	'output',9,9,	'O[W|D] port value',CR,LF
	db	'proceed',9,9,	'P [=address] [count]',CR,LF
	db	'quit',9,9,		'Q',CR,LF
	db	'register',9,	'R [register [value]]',CR,LF
%if PM
    db  '$'
helpmsg2:    
%endif
    db  'FPU register',9,'RN',CR,LF
    db  'toggle 386 regs',9,'RX',CR,LF
	db	'search',9,9,	'S range list',CR,LF
%if PM==0
    db  '$'
helpmsg2:    
%endif
	db	'trace',9,9,	'T [=address] [count]',CR,LF
	db	'trace mode',9,	'TM [0|1]',CR,LF
	db	'unassemble',9,	'U [range]',CR,LF
	db	'write program',9,'W [address]',CR,LF
	db	'write sectors',9,'W address drive sector count',CR,LF
	db	'expanded mem',9,'XA/XD/XM/XR/XS,X? for help'
%if PM    
    db   CR,LF,CR,LF
	db  "prompts: '-' = real/v86-mode; '#' = protected-mode"
%endif    
crlf	db	CR,LF,'$'

presskey db '[more]$'

xhelpmsg db	'Expanded memory (EMS) commands:',CR,LF
	db	'  Allocate	XA count',CR,LF
	db	'  Deallocate	XD handle',CR,LF
	db	'  Map memory	XM logical-page physical-page handle',CR,LF
	db	'  Reallocate	XR handle count',CR,LF
	db	'  Show status	XS',CR,LF,'$'

errcarat db	'^ Error',CR,LF,'$'

dskerrs dw	dskerr0,dskerr1,dskerr2,dskerr3,dskerr4,dskerr9,dskerr6,dskerr7
	dw	dskerr8,dskerr9,dskerra,dskerrb,dskerrc
dskerr0	db	'Write protect error',0
dskerr1	db	'Unknown unit error',0
dskerr2	db	'Drive not ready',0
dskerr3	db	'Unknown command',0
dskerr4	db	'Data error (CRC)',0
dskerr6	db	'Seek error',0
dskerr7	db	'Unknown media type',0
dskerr8	db	'Sector not found',0
dskerr9	db	'Unknown error',0
dskerra	db	'Write fault',0
dskerrb	db	'Read fault',0
dskerrc	db	'General failure',0
reading	db	' read',0
writing	db	' writ',0
drive	db	'ing drive '
driveno	db	'_',0
msg8088	db	'8086/88',0
msgx86	db	'x86',0
no_copr	db	' without coprocessor',0
has_copr db	' with coprocessor',0
has_287	db	' with 287',0
regs386 db  '386 regs o'
regs386s db '  ',0
tmodes  db  'trace mode is '
tmodev  db  '  - INTs are ',0
tmode1  db  'traced',0
tmode0  db  'processed',0
unused	db	' (unused)',0
needsmsg db	'[needs x86]'
needsmsg_L equ	11
needsmath db	'[needs math coprocessor]'
needsmath_L equ	24
obsolete db	'[obsolete]'
obsolete_L equ	10
int0msg	db	'Divide error',CR,LF,'$'
int1msg	db	'Unexpected single-step interrupt',CR,LF,'$'
int3msg	db	'Unexpected breakpoint interrupt',CR,LF,'$'

%if PM
%if 0
exc6msg	db	'Invalid opcode fault',CR,LF,'$'
%endif
%if 1
excCmsg	db	'Stack fault',CR,LF,'$'
%endif
excDmsg	db	'General protection fault',CR,LF,'$'
%if EXCCSIP
excloc	db	'CS:IP='
exccsip db '    :    '
		db  CR,LF,'$'
%endif        
excEmsg	db	'Page fault.',CR,LF,'$'
nodosext db 'Command not supported in protected-mode without a DOS-Extender',CR,LF,'$'
nopmsupp db 'Command not supported in protected-mode',CR,LF,'$'
%if DISPHOOK
dpmihook db 'DPMI entry hooked, new entry='
dpmihookcsip db '    :    ',CR,LF,'$'
%endif
%if DOSEMU
dDosEmuDate db "02/25/93"
%endif
nodesc	db 'resource not accessible in real-mode',CR,LF,'$'
;descwrong db 'descriptor not accessible',CR,LF,'$'
gatewrong db 'gate not accessible',CR,LF,'$'
%endif
cantwritebp db "Can't write breakpoint",CR,LF,'$'

progtrm	db	CR,LF,'Program terminated normally ('
progexit db '    )',CR,LF,'$'
nowhexe	db	'EXE and HEX files cannot be written',CR,LF,'$'
nownull	db	'Cannot write: no file name given',CR,LF,'$'
wwmsg1	db	'Writing $'
wwmsg2	db	' bytes',CR,LF,'$'
diskful	db	'Disk full',CR,LF,'$'
wwerr	db	'Error '
wwerr1	db	'____ opening file',CR,LF,'$'
doserr2	db	'File not found',CR,LF,'$'
doserr3	db	'Path not found',CR,LF,'$'
doserr5	db	'Access denied',CR,LF,'$'
doserr8	db	'Insufficient memory',CR,LF,'$'
;emmname	db	'EMMXXXX0'
emsnot	db	'EMS not installed',CR,LF,'$'
emserr1	db	'EMS internal error',CR,LF,'$'
emserr3	db	'Handle not found',CR,LF,'$'
emserr5	db	'No free handles',CR,LF,'$'
emserr7	db	'Total pages exceeded',CR,LF,'$'
emserr8	db	'Free pages exceeded',CR,LF,'$'
emserr9	db	'Parameter error',CR,LF,'$'
emserra	db	'Logical page out of range',CR,LF,'$'
emserrb	db	'Physical page out of range',CR,LF,'$'
emserrs	dw	emserr1,emserr1,0,emserr3,0,emserr5,0,emserr7,emserr8,emserr9
	dw	emserra,emserrb
emserrx	db	'EMS error '
emserrxa db	'__',CR,LF,'$'
xaans	db	'Handle created = '
xaans1	db	'____',CR,LF,'$'
xdans	db	'Handle '
xdans1	db	'____ deallocated',CR,LF,'$'
xrans	db	'Handle reallocated',CR,LF,'$'
xmans	db	'Logical page '
xmans1	db	'__ mapped to physical page '
xmans2	db	'__',CR,LF,'$'
xsstr1	db	'Handle '
xsstr1a	db	'____ has '
xsstr1b	db	'____ pages allocated',CR,LF,'$'
xsstr2	db	'phys. page '
xsstr2a	db	'__ = segment '
xsstr2b	db	'____  ','$'
xsstr3	db	'____ of a total '
xsstr3a	db	'____ EMS $'
xsstr4	db	'es have been allocated',CR,LF,'$'
xsstrpg	db	'pag$'
xsstrhd	db	'handl$'
xsnopgs db	'no mappable pages',CR,LF,CR,LF,'$'

;	Equates for instruction operands.
;	First the sizes.

OP_ALL	equ	40h		;byte/word/dword operand (could be 30h but ...)
OP_1632	equ	50h		;word or dword operand
OP_8	equ	60h		;byte operand
OP_16	equ	70h		;word operand
OP_32	equ	80h		;dword operand

OP_SIZE	equ	OP_ALL		;the lowest of these

;	These operand types need to be combined with a size.

OP_IMM		equ	0		;immediate
OP_RM		equ	2		;reg/mem
OP_M		equ	4		;mem (but not reg)
OP_R_MOD 	equ	6		;register, determined from MOD R/M part
OP_MOFFS 	equ	8		;memory offset; e.g., [1234]
OP_R		equ	10		;reg part of reg/mem byte
OP_R_ADD 	equ	12		;register, determined from instruction byte
OP_AX		equ	14		;al or ax or eax

;	These don't need a size.

OP_M64		equ	2		; 0 qword memory
OP_MFLOAT 	equ	4		; 1 float memory
OP_MDOUBLE 	equ	6		; 2 double-precision floating memory
OP_M80		equ	8		; 3 tbyte memory
OP_MXX		equ	10		; 4 memory (size unknown)
OP_FARMEM 	equ	12		; 5 far memory
OP_FARP		equ	14		; 6 far pointer
OP_REL8		equ	16		; 7 byte address relative to IP
OP_REL1632 	equ	18		; 8 word or dword address relative to IP
OP_1CHK		equ	20		; 9 check for ST(1)
OP_STI		equ	22		;10 ST(I)
OP_CR		equ	24		;11 CRx
OP_DR		equ	26		;12 DRx
OP_TR		equ	28		;13 TRx
OP_SEGREG 	equ	30		;14 segment register
OP_IMMS8 	equ	32		;15 sign extended immediate byte
OP_IMM8		equ	34		;16 immediate byte (other args may be (d)word)
OP_ECX		equ	36		;17 ECX if 32-bit operands; otherwise nothing (LOOPx)
OP_SHOSIZ 	equ	38		;18 set flag to always show the size

OP_1		equ	40		;1 (simple "string" ops from here on)
OP_3		equ	42		;3
OP_DX		equ	44		;DX
OP_CL		equ	46		;CL
OP_ST		equ	48		;ST (top of coprocessor stack)
OP_CS		equ	50		;CS
OP_DS		equ	52		;DS
OP_ES		equ	54		;ES
OP_FS		equ	56		;FS
OP_GS		equ	58		;GS
OP_SS		equ	60		;SS

;	Instructions that have an implicit operand subject to a segment override
;	(outs, movs, cmps, lods, xlat).

prfxtab	db	06eh,06fh,0a4h,0a5h,0a6h,0a7h,0ach,0adh,0d7h
P_LEN	equ	9

;	Instructions that can be used with REP/REPE/REPNE.

replist	db	06ch,06eh,0a4h,0aah,0ach	;REP
		db	0a6h,0aeh					;REPE/REPNE

	%include "debugtbl.inc"

%if PM

	%include "fptostr.inc"

dpmientry dd 0	;dpmi entry point returned by dpmi host
dpmiwatch dd 0	;
dssel     dw 0	;debugger's segment DATA
cssel     dw 0	;debugger's segment CODE
scratchsel dw 0	;scratch selector used for various purposes
dpmi32    db 0	;00=16-bit client, else 32-bit client
bNoHook2F db 0  ;int 2F, ax=1687h cannot be hooked (win9x dos box, DosEmu?)
bCSAttr   db 0  ;CS attribute (D bit)
bAddr32	  db 0  ;Address attribute. if 1, hiword(edx) is valid

exctab:
	db 0
    dw exc00, exc0032
	db 1
    dw exc01, exc0132
	db 3
    dw exc03, exc0332
%if 0
	db 06h
    dw exc06, exc0632
%endif
%if 1
	db 0Ch
    dw exc0c, exc0c32
%endif
	db 0Dh
    dw exc0d, exc0d32
	db 0Eh
    dw exc0e, exc0e32
    db -1

;--- int 2F handler

debug2F:
	pushf
	cmp	ax,1687h
dpmidisable:		;set [IP+1]=0 if hook 2F is to be disabled    
    jz dpmiquery
    popf
    jmp far [cs:oldi2f]
dpmiquery:
	call far [cs:oldi2f]
    and ax,ax
    jnz nodpmi
	mov [cs:dpmientry+0],di
	mov [cs:dpmientry+2],es
    mov di,mydpmientry
    push cs
    pop es
nodpmi:    
	iret
mydpmientry:
	mov [cs:dpmi32],al
	call far [cs:dpmientry]
    jc dpmifailed
    call installdpmi
dpmifailed:
    retf

%if NASM
	cpu 286
%else    
	cpu 286 protected
%endif

;--- inp: [sp+4] = client real-mode CS
    
installdpmi:
    pusha
    mov bp,sp		;[bp+16] = ret, [bp+18]=ip, [bp+20]=cs
    push ds
    mov bx,cs
    mov ax,000Ah	;get a data descriptor for DEBUG's segment
    int 31h
    jnc id_0
    jmp fataldpmierr
id_0:
    mov ds,ax
    mov [cssel],cs
    mov [dssel],ds
    mov cx,2		;alloc 2 descriptors
    xor ax,ax
    int 31h
    jnc id_1
    jmp fataldpmierr
id_1:    
    mov [scratchsel],ax	;the first is used as scratch descriptor
    mov bx,ax
    xor cx,cx
%if 1
	cmp byte [machine],3	;is at least a 80386?
    jb instdpmi_1
%else
    cmp byte [dpmi32],0		;is a 16-bit client?
    jz instdpmi_1
%endif    
    dec cx			;set a limit of FFFFFFFFh
instdpmi_1:
    or dx,-1
    mov ax,0008h
    int 31h
    add bx,8		;the second selector is clients CS
    xor cx,cx		;this limit is FFFF even for 32-bits
    mov ax,0008h
    int 31h
    mov dx,[bp+20]	;get client's CS
    call setrmaddr	;set base
    mov ax,cs
    lar cx,ax
    shr cx,8		;CS remains 16-bit
    mov ax,0009h
    int 31h
    mov [bp+20],bx
%if 1
    mov bx,[run_cs]	;convert saved (by G) CS segment
    and bx,bx
    jz norun_cssegm
    mov ax,0002h
    int 31h
    jc  fataldpmierr
    mov [run_cs],ax
norun_cssegm:
%endif
    mov bx,word [pInDOS+2]
    mov ax,0002h	;convert the INDOS segment 
    int 31h
    jc  fataldpmierr
    mov word [pInDOS+2],ax
%if USESDA
    mov bx,word [pSDA+2]
    mov ax,0002h	;convert the SDA segment
    int 31h
    jc  fataldpmierr
    mov word [pSDA+2],ax
%endif
%if 0    
    mov bx,[d_addr+4]	;convert the disassemblers's current address
    and bx,bx
    jz  nodsegm
    mov ax,0002h
    int 31h
    jc  fataldpmierr
    mov [d_addr+4],ax
nodsegm:    
%endif
%if 1
    mov bx,[a_addr+4]	;convert the assembler's current address
    and bx,bx
    jz  noasegm
    mov ax,0002h
    int 31h
    jc  fataldpmierr
    mov [a_addr+4],ax
noasegm:
%endif
	mov si,exctab		;hook several exceptions
    cld
next_exc:    
    lodsb
    cmp al,-1
    jz exc_done
    mov bl,al
    lodsw
    mov dx,ax
    lodsw
    mov cx,cs
    cmp byte [dpmi32],0
    jz is16
    cpu 386
    push edx
    movzx edx,ax
    mov ax,0203h
    int 31h
    pop edx
    cpu 286
    jmp next_exc
is16:
    mov ax,0203h
    int 31h
    jmp next_exc
exc_done:    
    mov bl,2Fh			;get int 2Fh real-mode vector
    mov ax,200h
    int 31h
    cmp cx,[pspdbg]		;did we hook it and are the last in chain?
    jnz int2fnotours
    mov dx,[oldi2f+0]
    xor cx,cx
    xchg cx,[oldi2f+2]	;then unhook
    mov ax,201h
    int 31h
int2fnotours:    
    pop ds
    popa
    clc
    ret
fataldpmierr:
	mov ax,4CFFh
    int 21h

;--- dpmi16 exception frame:
;--- +0     regs
;--- +16    exc no/msg
;--- +18    retf (dpmi return far16)
;--- +22    error code
;--- +24    e/ip
;--- +26    cs
;--- +28    e/fl
;--- +30    e/sp
;--- +32    ss

fr16_exc equ 16
fr16_ip equ 24
fr16_cs equ 26
fr16_fl equ 28
fr16_sp equ 30
fr16_ss equ 32

exc00:
	push int0msg
    jmp exc_xx
exc01:
	push int1msg
    jmp exc_xx
exc03:
	push int3msg
    jmp exc_xx
%if 0
exc06:
	push exc6msg
    jmp exc_xx
%endif    
%if 1    
exc0c:
	push excCmsg
    jmp exc_xx
%endif    
exc0d:
	push excDmsg
    jmp exc_xx
exc0e:
	push excEmsg
exc_xx:    
    pusha
    mov bp,sp
    push ds
    mov ds,[cs:dssel]
    mov ax,[bp + fr16_ip]
    mov bx,[bp + fr16_cs]
    mov cx,[bp + fr16_fl]
    mov dx,[bp + fr16_sp]
    mov si,[bp + fr16_ss]
    mov [bp + fr16_cs], cs
    mov [bp + fr16_ss], ds
    mov di,[bp+fr16_exc]
	mov	word [run_int],di
    cmp [bInDbg],byte 0	;did the exception occur inside DEBUG?
    jz isdebuggee16
    push ds
    pop es
%if EXCCSIP
    mov di,exccsip		;render CS:IP if internal GPF
    mov ax,bx
    call hexword
    inc di
    mov ax,[bp + fr16_ip]
    call hexword

%endif    
    mov word [bp + fr16_ip],tt2x
    mov ax,[savesp]
    mov [bp + fr16_sp], ax
    and byte [bp + fr16_fl+1], ~1	;reset TF
    pop ax
    jmp isdebugger16
isdebuggee16:
    mov word [bp + fr16_ip], intrtn2
    and byte [bp + fr16_fl+1], ~3	;reset IF + TF
    mov word [bp + fr16_sp], reg_ss
    mov [reg_ip],ax
    mov [reg_cs],bx
    mov [reg_fl],cx
    mov [reg_sp],dx
    mov [reg_ss],si
    pop ax
    mov [reg_ds],ax
	mov ds,ax
isdebugger16:
    popa
    add sp,2
    retf

	cpu 386

fr32_exc equ 16*2
fr32_eip equ 24*2
fr32_cs  equ 26*2
fr32_efl equ 28*2
fr32_esp equ 30*2
fr32_ss  equ 32*2
    
exc0032:
	push dword int0msg
    jmp exc32_xx
exc0132:
	push dword int1msg
    jmp exc32_xx
exc0332:
	push dword int3msg
    jmp exc32_xx
%if 0
exc0632:
	push dword exc6msg
    jmp exc32_xx
%endif    
%if 1    
exc0c32:
	push dword excCmsg
    jmp exc32_xx
%endif    
exc0d32:
	push dword excDmsg
    jmp exc32_xx
exc0e32:
	push dword excEmsg
exc32_xx:
    pushad
    mov ebp,esp
    push ds
    mov ds,[cs:dssel]
    mov eax,[ebp + fr32_eip]
    mov bx, [ebp + fr32_cs]
    mov ecx,[ebp + fr32_efl]
    mov edx,[ebp + fr32_esp]
    mov si, [ebp + fr32_ss]
    mov [ebp + fr32_cs], cs
    mov [ebp + fr32_ss], ds
    mov di,[ebp + fr32_exc]
	mov	word [run_int],di
    cmp [bInDbg],byte 0	;did the exception occur inside DEBUG?
    jz isdebuggee32
    push ds
    pop es
%if EXCCSIP
    mov di,exccsip
    mov ax,bx
    call hexword
    inc di
    mov ax,[ebp + fr32_eip]
    call hexword
%endif    
    mov dword [ebp + fr32_eip],tt2x
    movzx eax,word [savesp]
    mov [ebp + fr32_esp], eax
    and byte [ebp+fr32_efl+1], ~1	;reset TF
    pop ax
    jmp isdebugger32
isdebuggee32:
    mov dword [ebp + fr32_eip], intrtn2
    and byte  [ebp + fr32_efl+1], ~3;reset IF + TF
    mov dword [ebp + fr32_esp], reg_ss
    mov [reg_ip],ax
    mov [reg_cs],bx
    mov [reg_fl],cx
    mov [reg_sp],dx
    mov [reg_ss],si
    shr eax,16
;    shr ecx,16
    shr edx,16
    mov [regh_eip],ax
;    mov [regh_efl],cx
    mov [regh_esp],dx
    pop ax
    mov [reg_ds],ax
	mov ds,ax
isdebugger32:
    popad
    add esp,4
	db 66h
    retf

;--- get attribute of selector in BX into AX

getattrhigh:
    lar eax,ebx
    shr eax,8
    test ah,40h
    ret

myi23:
	cmp [cs:dpmi32],byte 0
    jz doretf2
    db 66h
    retf 4
doretf2:
	retf 2
myi24:
	cmp [cs:dpmi32],byte 0
    jz doiret16
	db 66h
doiret16:
	iret
    

	cpu 8086
    
%endif	;PM

;	debug22 - INT 22 (Program terminate) interrupt handler.
;	This is for DEBUG itself:  it's a catch-all for the various INT 23
;	and INT 24 calls that may occur unpredictably at any time.
;	What we do is pretend to be a command interpreter (which we are,
;	in a sense, just a different sort of command) by setting the PSP of
;	our parent equal to our own PSP so that DOS does not free our memory
;	when we quit.  Therefore control ends up here when Control-Break or
;	an Abort in Abort/Retry/Fail is selected.

debug22:
	cld			;reestablish things
	mov	ax,cs
	mov	ds,ax
	mov	ss,ax

;	Begin main command loop.

cmd3:
	mov	sp,[savesp]	;restore stack (this must be first)
	mov	word [errret],cmd3
    push ds
    pop es
    call isdebuggeeloaded
    jnz taskok
    call createdummytask	;if no task is active, create a dummy one
taskok:
	mov	dx,prompt1
%if PM    
    call ispm
    jnz cmd3_1
    mov dx,prompt3
cmd3_1:
%endif
	mov	cx,1
	call getline	;prompted input
	cmp	al,CR
	jnz	isnotblank
    mov dx, [lastcmd]
    dec si
    jmp cmd4
isnotblank:    
	cmp	al,';'
	je	cmd3		;if comment
	cmp	al,'?'
	je	help		;if request for help
	or	al,TOLOWER
	sub	al,'a'
	cmp	al,'x'-'a'
	ja	errorj1		;if not recognized
	cbw
	xchg bx,ax
	call skipcomma
	shl	bx,1
    mov dx,[cmdlist+bx]
    mov word [lastcmd],dmycmd
cmd4:    
	mov	di,line_out
	call dx
	jmp	cmd3		;back to the top

dmycmd:
	ret

waitkey:
	cmp byte [notatty],0
    jnz nowait
	mov bx,1
    mov ax,4400h	;is stdout a device?
    int 21h
    test dl,80h
    jz nowait
	push es
    mov ax,40h		;0040h is a bimodal segment/selector
    mov es,ax
    cmp byte [es:84h],30
	pop es
    jnc nowait
    mov dx,presskey
	call int21ah9	;print string
    mov ah,8
    int 21h
    mov dl,13
    mov ah,2
    int 21h
nowait:    
    ret

help:
	mov	dx,helpmsg
	call int21ah9	;print string
    call waitkey
	mov	dx,helpmsg2
prnquit:
	call int21ah9	;print string
	jmp	cmd3		;done

errorj1:jmp	error

;	A command - tiny assembler.

asm_mn_flags	db	0	;flags for the mnemonic
AMF_D32		equ	1		;32bit opcode/data operand
AMF_WAIT	equ	2
AMF_A32		equ	4		;address operand is 32bit
AMF_SIB		equ	8		;there's a SIB in the arguments
AMF_MSEG	equ	10h		;if a seg prefix was given b4 mnemonic
AMF_FSGS	equ	20h		;if FS or GS was encountered

AMF_D16		equ 40h		;16bit opcode/data operand
AMF_ADDR	equ	80h 	;address operand is given

aa_saved_prefix	db	0		;WAIT or REP... prefix
;		aa_saved_prefix and aa_seg_pre must be consecutive.
aa_seg_pre	db	0		;segment prefix

mneminfo	dw	0		;address associated with the mnemonic

;	The following 7  words (through a_opcode) must all be consecutive.

rmaddr		dw	0		;address of operand giving the R/M byte
regmem		db	0		;mod reg r/m part of instruction
						;regmem and sibbyte must be consecutive
sibbyte		db	0		;SIB byte
immaddr		dw	0		;address of operand giving the immed stf
xxaddr		dw	0		;address of additional stuff
dismach		db	0		;type of processor needed
dmflags		db	0		;flags for extra processor features

DM_COPR		equ	1		;math coprocessor
DM_MMX		equ	2		;MMX extensions

opcode_or	db	0		;extra bits in the op code
opsize		db	0		;size of this operation
varflags	db	0		;flags for this variant

VAR_LOCKABLE	equ	1	;variant is lockable
VAR_MODRM		equ	2	;if there's a MOD R/M here
VAR_SIZ_GIVN	equ	4	;if a size was given
VAR_SIZ_FORCD	equ	8	;if only one size is permitted
VAR_SIZ_NEED	equ	10h	;if we need the size
VAR_D16			equ	20h	;if operand size is WORD
VAR_D32			equ	40h	;if operand size is DWORD

a_opcode	dw	0		;op code info for this variant
a_reqsize	db	0		;size that this arg should be

a_opcode2	dw	0		;copy of a_opcode for obs-instruction
		dw	0dbe0h,0dbe1h,0dbe4h,124h,126h	;obs. instruction codes
					;(the above must follow a_opcode2)

obsmach		db	1,1,2,4,4	;max permissible machine for the above

aadbsiz		db	0,4,2,1		;table for max size of db operand
aadbsto		dw	0,aa28,aa29,aa30;table for routine to store a number

modrmtab	db	11,0,13,0,15,0,14,0	;[bx], [bp], [di], [si]
		db	15,13,14,13,15,11,14,11	;[bp+di],[bp+si],[bx+di],[bx+si]

aam_args	db	'a',CR

;	Equates for parsed arguments.

ARG_DEREF		equ	1	;non-immediate memory reference
ARG_MODRM		equ	2	;if we've computed the MOD R/M byte
ARG_JUSTREG		equ	4	;a solo register
ARG_WEIRDREG	equ	8	;if it's a segment register or CR, etc.
ARG_IMMED		equ	10h	;if it's just a number
ARG_FARADDR		equ	20h	;if it's of the form xxxx:yyyyyyyy

;	For each operand type in the following table, the first byte is
;	the bits, at least one of which must be present; the second is the
;	bits all of which must be absent.

bittab	db	ARG_IMMED				;OP_IMM
		db	ARG_DEREF+ARG_JUSTREG	;OP_RM
		db	ARG_DEREF				;OP_M
		db	ARG_JUSTREG				;OP_R_MOD
		db	ARG_DEREF				;OP_MOFFS
		db	ARG_JUSTREG				;OP_R
		db	ARG_JUSTREG				;OP_R_ADD
		db	ARG_JUSTREG				;OP_AX
		db	ARG_DEREF				;OP_M64
		db	ARG_DEREF				;OP_MFLOAT
		db	ARG_DEREF				;OP_MDOUBLE
		db	ARG_DEREF				;OP_M80
		db	ARG_DEREF				;OP_MXX
		db	ARG_DEREF				;OP_FARMEM
		db	ARG_FARADDR				;OP_FARP
		db	ARG_IMMED				;OP_REL8
		db	ARG_IMMED				;OP_REL1632
		db	ARG_WEIRDREG			;OP_1CHK
		db	ARG_WEIRDREG			;OP_STI
		db	ARG_WEIRDREG			;OP_CR
		db	ARG_WEIRDREG			;OP_DR
		db	ARG_WEIRDREG			;OP_TR
		db	ARG_WEIRDREG			;OP_SEGREG
		db	ARG_IMMED				;OP_IMMS8
		db	ARG_IMMED				;OP_IMM8
		db	ARG_JUSTREG				;OP_ECX
		db	0ffh					;OP_SHOSIZ
		db	ARG_IMMED				;OP_1
		db	ARG_IMMED				;OP_3
		db	ARG_JUSTREG				;OP_DX
		db	ARG_JUSTREG				;OP_CL
		db	ARG_WEIRDREG			;OP_ST
		db	ARG_WEIRDREG			;OP_CS
		db	ARG_WEIRDREG			;OP_DS
		db	ARG_WEIRDREG			;OP_ES
		db	ARG_WEIRDREG			;OP_FS
		db	ARG_WEIRDREG			;OP_GS
		db	ARG_WEIRDREG			;OP_SS

;	Jump table for operand types.

asm_jmp1:	
		dw	ao04,ao01,ao01,ao01	;OP_IMM, OP_RM, OP_M, OP_R_MOD
		dw	ao05,ao02,ao03,ao06	;OP_MOFFS, OP_R, OP_R_ADD, OP_AX

		dw	ao17,ao17,ao17		;OP_M64, OP_MFLOAT, OP_MDOUBLE
		dw	ao17,ao17,ao17		;OP_M80, OP_MXX, OP_FARMEM
		dw	ao21,ao23,ao25		;OP_FARP, OP_REL8, OP_REL1632
		dw	ao29,ao30,ao31		;OP_1CHK, OP_STI, OP_CR
		dw	ao34,ao35,ao39		;OP_DR, OP_TR, OP_SEGREG
		dw	ao41,ao42,ao43m		;OP_IMMS8, OP_IMM8, OP_ECX
		dw	ao44,ao46,ao47		;OP_SHOSIZ, OP_1, OP_3
		dw	ao48,ao48,ao48		;OP_DX, OP_CL, OP_ST
		dw	ao48,ao48,ao48		;OP_CS, OP_DS, OP_ES
		dw	ao48,ao48,ao48		;OP_FS, OP_GS, OP_SS

;--- special ops DX, CL, ST, CS, DS, ES, FS, GS, SS, MMX
;--- entry required if ao48 is set above

asm_regnum	db	10,1,30				;DX, CL, ST
			db	25,27,24,28,29,26	;CS, DS, ES, FS, GS, SS

asm_siznum	db	5, 6, 7, 8		;qword, float, double, tbyte
			db	-1, 12			;none, far

aa00:
	jmp	cmd3		;done with this command

aa:	mov	word [errret],aa01
	cmp	al,CR
	je	aa01		;if end of line
	mov	bx,[reg_cs]	;default segment to use
aa00a:
	call getaddr	;get address into bx:(e)dx
	call chkeol		;expect end of line here
	mov	[a_addr],dx	;save the address
%if PM    
    cmp byte [machine],3
    jb aa00_1
    cpu 386
    mov [a_addr],edx
    cpu 8086
aa00_1:
%endif
	mov	[a_addr+4],bx

;	Begin loop over input lines.

aa01:
	mov	sp,[savesp]	;restore the stack (this implies no "ret")
	mov	di,line_out
	mov	ax,[a_addr+4]
	call hexword
	mov	al,':'
	stosb
	mov	byte [asm_mn_flags],0
%if PM    
	call ispm
    jnz aa01_1
    cmp byte [machine],3
    jb aa01_1
    mov bx,[a_addr+4]
    call getattrhigh	;set Z flag
    mov [bCSAttr],ah
    jz aa01_1
    mov ax,[a_addr+2]
	call hexword
;	mov	byte [asm_mn_flags],AMF_D32
aa01_1:   
%endif
	mov	ax,[a_addr+0]
	call hexword
	mov	al,' '
	stosb
	call getline00
	cmp	al,CR
	je	aa00		;if done
	cmp	al,';'
	je	aa01		;if comment
	mov	word [aa_saved_prefix],0 ;clear aa_saved_prefix and aa_seg_pre

;	Get mnemonic and look it up.

aa02:
	mov	di,line_out	;return here after LOCK/REP/SEG prefix
	push si			;save position of mnemonic
aa03:
	cmp	al,'a'
	jb	aa04		;if not lower case letter
	cmp	al,'z'
	ja	aa04
	and	al,TOUPPER	;convert to upper case
aa04:
	stosb
	lodsb
	cmp	al,CR
	je	aa05		;if end of mnemonic
	cmp	al,' '
	je	aa05
	cmp	al,TAB
	jne	aa03
aa05:
	or byte [di-1],80h	;set highest bit of last char of mnemonic
	call skipwh0	;skip to next field
	dec	si
	push si			;save position in input line
	mov	al,0
	stosb
	mov	si,mnlist
aa06:               ;<--- next mnemonic
	cmp si,end_mnlist
    jnc aa13		;if end of table reached
    mov bx,si
	add si,2
    mov cx,si
aa07:
	lodsb			;skip to end of string
	and	al,al
	jns	aa07		;if not end of string
    xchg cx,si
    push cx
    sub cx,si		;size of opcode in mnlist
	mov di,line_out
	repe cmpsb
	pop	si
	je	aa14		;if found it
    jmp aa06
aa13:
	pop si			;skip position in input line
aa13a:
	pop si			;skip position of mnemonic
aa13b:
	jmp	error		;complain

;	We found the mnemonic.

aa14:
	mov si,[bx]		;get the offset into asmtab
	add	si,asmtab

;	Now si points to the spot in asmtab corresponding to this mnemonic.
;	The format of the assembler table is as follows.
;	First, there is optionally one of the following bytes:
;		ASM_DB		db mnemonic
;		ASM_DW		dw mnemonic
;		ASM_DD		dd mnemonic
;		ASM_WAIT	the mnemonic should start with a wait
;					instruction.
;		ASM_D32		This is a 32 bit instruction variant.
;		ASM_D16		This is a 16 bit instruction variant.
;		ASM_AAX		Special for AAM and AAD instructions:
;					put 0ah in for a default operand.
;		ASM_SEG		This is a segment prefix.
;		ASM_LOCKREP	This is a LOCK or REP... prefix.
;
;	Then, in most cases, this is followed by one or more of the following
;	sequences, indicating an instruction variant.
;	ASM_LOCKABLE	(optional) indicates that this instruction can
;					follow a LOCK prefix.
;	ASM_MACHx	Indicates the first machine on which this
;				instruction appeared.
;	[word]		This is a 16-bit integer, most significant byte
;				first, giving ASMMOD * a + b, where b is an
;				index into the array opindex (indicating the
;				key, or type of operand list), and a is as
;				follows:
;				0-255	  The (one-byte) instruction.
;				256-511	  The lower 8 bits give the second byte of
;						  a two-byte instruction beginning with 0fh.
;				512-575	  Bits 2-0 say which floating point instruction
;						  this is (0d8h-0dfh), and 5-3 give the /r
;						  field.
;				576-...	  (a-576)/8 is the index in the array agroups
;						  (which gives the real value of a), and the
;						  low-order 3 bits gives the /r field.
;
;	[byte]		This gives the second byte of a floating
;				instruction if 0d8h <= a <= 0dfh.
;
;	Following these is an ASM_END byte.
;
;	Exceptions:
;		ASM_SEG and ASM_LOCKREP are followed by just one byte, the
;		prefix byte.
;		ASM_DB, ASM_DW, and ASM_DD don't need to be followed by
;		anything.
;

ASM_END		equ	0ffh
ASM_DB		equ	0feh
ASM_DW		equ	0fdh
ASM_DD		equ	0fch
ASM_ORG		equ	0fbh
ASM_WAIT	equ	0fah
ASM_D32		equ	0f9h
ASM_D16		equ	0f8h
ASM_AAX		equ	0f7h
ASM_SEG		equ	0f6h
ASM_LOCKREP	equ	0f5h

ASM_LOCKABLE	equ	0f4h
ASM_MACH6		equ	0f3h
ASM_MACH0		equ	0edh

	cmp	byte [si],ASM_LOCKREP	;check for mnemonic flag byte
	jb	aa15					;if none
	lodsb						;get the prefix
	sub	al,ASM_LOCKREP			;convert to 0-9
	je	aa18					;if LOCK or REP...
	cbw
	dec	ax
	jz	aa17					;if segment prefix (ASM_SEG)
	dec	ax
	jz	aa16					;if aad or aam (ASM_AAX)
	dec	ax
	jz	aa15_1  				;if ASM_D16
	cmp	al,3
	jae	aa20					;if ASM_ORG or ASM_DD or ASM_DW or ASM_DB
	or	[asm_mn_flags],al		;save AMF_D32 or AMF_WAIT (1 or 2)
aa15:
	jmp	ab01			  		;now process the arguments
aa15_1:
	or	byte [asm_mn_flags],AMF_D16
    inc si						;skip the ASM_D32 byte
	jmp	ab01			  		;now process the arguments

aa16:
	jmp	ab00

;	segment prefix

aa17:
	lodsb			;get prefix value
	mov	[aa_seg_pre],al
	or	byte [asm_mn_flags],AMF_MSEG
	pop	si
	pop	ax
	lodsb
	cmp	al,CR
	je	aa13a		;if end of line ( ==> error)
	cmp	al,';'
	je	aa13a		;if end of line (comment) ( ==> error)
	jmp	aa02		;back for more

;	LOCK or REP prefix

aa18:
	lodsb			;get prefix value
	xchg al,[aa_saved_prefix]
	cmp	al,0
	jnz	aa13a		;if there already was a saved prefix
	pop	si
	pop	ax
	lodsb
	cmp	al,CR
	je	aa19		;if end of line
	cmp	al,';'
	je	aa19		;if end of line (comment)
	jmp	aa02		;back for more

aa19:
	mov	al,[aa_saved_prefix]	;just a prefix, nothing else
	mov	di,line_out
	stosb
	jmp	aa27

;	Pseudo ops (org or db/dw/dd).

aa20:
	cmp	word [aa_saved_prefix],0
	jnz	aa13a		;if there was a prefix or a segment
	pop	si			;get position in input line
	sub	al,3		;AX=0 if org, 1 if dd, 2 if dw, 3 if db.
	jnz	aa20m		;if not ORG

;	Process ORG pseudo op.

	call skipwhite
	cmp	al,CR
	je	aa20a			;if nothing
	mov	bx,[a_addr+4]	;default segment
	jmp	aa00a			;go to top
aa20a:
	jmp	aa01			;get next line

;	Data instructions (DB/DW/DD).

aa20m:
	mov	di,line_out		;put the bytes here when we get them
	xchg ax,bx			;mov bx,ax
	mov	al,[aadbsiz+bx]	;move maximum size
	mov	[aadbsiz],al
	shl	bx,1
	mov	ax,[aadbsto+bx]	;move address of storage routine
	mov	[aadbsto],ax
	call skipwhite
	cmp	al,CR
	je	aa27			;if end of line

aa21:
	cmp	al,'"'
	je	aa22			;if string
	cmp	al,"'"
	je	aa22			;if string
	call aageti			;get a numerical value into dx:bx, size into cl
	cmp	cl,[aadbsiz]
	jg	aa24			;if overflow
	xchg ax,bx
	call [aadbsto]		;store the value
	cmp	di,real_end
	ja	aa24			;if output line overflow
	xchg ax,bx
	jmp	aa26			;done with this one

aa22:
	mov	ah,al
aa23:
	lodsb
	cmp	al,CR
	je	aa24		;if end of line
	cmp	al,ah
	je	aa25		;if end of string
	stosb
	cmp	di,real_end
	jbe	aa23		;if output line not overflowing
aa24:
	jmp	aa13b		;error
aa25:
	lodsb
aa26:
	call skipcomm0
	cmp	al,CR
	jne	aa21		;if not end of line

;	End of line.  Copy it over.

aa27:
	mov	si,line_out
	mov	cx,di
	sub	cx,si
	mov	di,[a_addr+0]
	mov	es,[a_addr+4]
	rep	movsb		;copy it
	push ds
	pop	es
	mov	[a_addr],di
	jmp	aa01

;	Routines to store a byte/word/dword.

aa28:
	stosw			;store a dword value
	xchg ax,dx
	stosw
	xchg ax,dx
	ret

aa29:
	stosw			;store a word value
	ret

aa30:
	stosb			;store a byte value
	ret

;	Here we process the AAD and AAM instructions.  They are special
;	in that they may take a one-byte argument, or none (in which case
;	the argument defaults to 0ah = ten).

ab00:
	mov	[mneminfo],si	;save this address
	pop	si
	lodsb
	cmp	al,CR
	je	ab00a		;if end of line
	cmp	al,';'
	jne	ab01b		;if not end of line
ab00a:
	mov	si,aam_args	;fake a 0ah argument
	jmp	ab01a

;	Process normal instructions.

;	First we parse each argument into a 12-byte data block, stored
;	consecutively at line_out, line_out+12, etc.
;	This is stored as follows.

;	[di]	Flags (ARG_DEREF, etc.)
;	[di+1]	Unused
;	[di+2]	Size argument, if any (1=byte, 2=word, 3=(unused), 4=dword,
;		5=qword, 6=float, 7=double, 8=tbyte, 9=short, 10=long, 11=near,
;		12=far).
;	[di+3]	Size of MOD R/M displacement
;	[di+4]	First register, or MOD R/M byte
;	[di+5]	Second register or index register or SIB byte
;	[di+6]	Index factor
;	[di+7]	Sizes of numbers are or-ed here
;	[di+8]	(dword) number

;	For arguments of the form xxxx:yyyyyyyy, xxxx is stored in [di+5],
;	and yyyyyyyy in [di+8].  The number of bytes in yyyyyyyy is stored in
;	opaddr, 2 is stored in [di+4], and di is stored in xxaddr.

ab01:
	mov	[mneminfo],si	;save this address
	pop	si				;get position in line
ab01a:
	lodsb
ab01b:
	mov	di,line_out

;	Begin loop over operands.

ab02:
	cmp	al,CR
	je	ab03		;if end of line
	cmp	al,';'
	jne	ab04		;if not end of line
ab03:
	jmp	ab99		;to next phase

ab04:
	push di			;clear out the next storage area
	mov	cx,6
	xor	ax,ax
	rep	stosw
	pop	di

;	Small loop over "BYTE PTR" and segment prefixes.

ab05:
	dec	si
	mov	ax,[si]
	and	ax,TOUPPER_W
	cmp	byte [di+2],0
	jne	ab07		;if already have a size qualifier ("BYTE PTR",...)
	push di
	mov	di,sizetcnam
	mov	cx,12
	repne scasw
	pop	di
	jne	ab07		;if not found
	or cx,cx
	jnz	ab06		;if not 'FA'
	mov	al,[si+2]
	and	al,TOUPPER
	cmp	al,'R'
	jne	ab09		;if not 'FAR' (could be hexadecimal)
ab06:
	sub	cl,12
	neg	cl			;convert to 1, ..., 12
	mov	[di+2],cl
	call skipalpha	;skip rest of size qualifier (WOrd, DWord, ...) + WS
;	call skipwh0
	mov	ah,[si]
	and	ax,TOUPPER_W
%if 0
	cmp	ax,'FA'
	jne	ab06a		;if not 'FA'
	mov	al,[si+1]
	and	al,TOUPPER
	cmp	al,'R'
	jne	ab05		;if not 'FAR'
	mov	byte [di+2],12	;set 'FAR'
	or	byte [asm_mn_flags],AMF_D32
	call skipalpha
;	call skipwh0
	mov	ah,[si]
	and	ax,TOUPPER_W
%endif    
ab06a:
	cmp	ax,'PT'
	jne	ab05		;if not 'PTR'
	call skipalpha
;	call skipwh0
	jmp	ab05

ab07:
	cmp	byte [aa_seg_pre],0
	jne	ab09		;if we already have a segment prefix
	push di
	mov	di,segrgnam
	mov	cx,6
	repne scasw
	pop	di
	jne	ab09		;if not found
	push si			;save si in case there's no colon
	lodsw
	call skipwhite
	cmp	al,':'
	jne	ab08		;if not followed by ':'
	pop	ax			;discard saved si
	call skipwhite	;skip it
	mov	bx,prefixlist + 5
	sub	bx,cx
	mov	al,[bx]		;look up the prefix byte
	mov	[aa_seg_pre],al	;save it away
	jmp	ab05

ab08:
	pop	si

;	Begin parsing main part of argument.

ab09:
	push di			;check for solo registers
	mov	di,rgnam816
	mov	cx,27
	call aagetreg
	pop	di
	jc	ab14		;if not a register
	or	byte [di],ARG_JUSTREG
	mov	[di+4],bl	;save register number
	cmp	bl,24
	jae	ab09a		;if it's not a normal register
	xchg ax,bx		;mov al,bl
	mov	cl,3
	shr	al,cl		;al = size:  0 -> byte, 1 -> word, 2 -> dword
	add	al,-2
	adc	al,3		;convert to 1, 2, 4 (respectively)
	jmp	ab13

ab09a:
	xor	byte [di],ARG_JUSTREG + ARG_WEIRDREG
	mov	al,2		;register size
	cmp	bl,30
	ja	ab11		;if it's CR, DR, TR, or MM
	je	ab09b		;if it's ST
	cmp	bl,28
	jb	ab13		;if it's a normal segment register
	or	byte [asm_mn_flags],AMF_FSGS	;flag it
	jmp	ab13

ab09b:
	cmp	byte [si],'('
	jne	ab12		;if just plain ST
	lodsb
	lodsb
	sub	al,'0'
	cmp	al,7
	ja	ab10		;if not 0..7
	mov	[di+5],al	;save the number
	lodsb
	cmp	al,')'
	je	ab12		;if not error
ab10:
	jmp	aa13b		;error

ab11:
	lodsb			;other registers (CR, DR, TR, MM)
	sub	al,'0'
	cmp	al,7
	ja	ab10		;if error
	mov	[di+5],al	;save the number
	mov	al,4		;register size
	cmp	bl,31
	jne	ab13		;if not MM register
ab12:
	mov	al,0
ab13:
	cmp	al,[di+2]	;compare with stated size
	je	ab13a		;if same
	xchg al,[di+2]
	cmp	al,0
	jne	ab10		;if wrong size given
ab13a:
	jmp	ab44		;done with this operand

;	It's not a register reference.  Try for a number.

ab14:
	lodsb
	call aaifnum
	jc	ab17		;it's not a number
ab14a:
	call aageti		;get the number
	mov	[di+7],cl
	mov	[di+8],bx
	mov	[di+10],dx
	call skipwh0
	cmp	cl,2
	jg	ab17		;if we can't have a colon here
	cmp	al,':'
	jne	ab17		;if not xxxx:yyyy
	call skipwhite
	call aageti
	mov	cx,[di+8]
	mov	[di+5],cx
	mov	[di+8],bx
	mov	[di+10],dx
	or	byte [di],ARG_FARADDR
	jmp	ab43		;done with this operand

;	Check for [...].

ab15:
	jmp	ab30		;do post-processing

ab16:
	call skipwhite
ab17:
	cmp	al,'['		;begin loop over sets of []
	jne	ab15		;if not [
	or	byte [di],ARG_DEREF ;set the flag
ab18:
	call skipwhite
ab19:
	cmp	al,']'		;begin loop within []
	je	ab16		;if done

;	Check for a register (within []).

	dec	si
	push di
	mov	di,rgnam16
	mov	cx,8
	call aagetreg
	pop	di
	jc	ab25		;if not a register
	cmp	bl,16
	jae	ab20		;if 32-bit register
	add	bl,8		;adjust 0..7 to 8..15
	jmp	ab21
ab20:
	cmp	byte [di+5],0
	jnz	ab21		;if we already have an index
	call skipwhite
	dec	si
	cmp	al,'*'
	jne	ab21		;if not followed by '*'
	inc	si
	mov	[di+5],bl	;save index register
	call skipwhite
	call aageti
	call aaconvindex
	jmp	ab28		;ready for next part

ab21:
	cmp	byte [di+4],0
	jne	ab22		;if there's already a register
	mov	byte [di+4],bl
	jmp	ab23
ab22:
	cmp	byte [di+5],0
	jne	ab24		;if too many registers
	mov	byte [di+5],bl
ab23:
	call skipwhite
	jmp	ab28		;ready for next part

ab24:	jmp	aa13b		;error

;	Try for a number (within []).

ab25:
	lodsb
ab26:
	call aageti		;get a number (or flag an error)
	call skipwh0
	cmp	al,'*'
	je	ab27		;if it's an index factor
	or	[di+7],cl
	add	[di+8],bx
	adc	[di+10],dx
	jmp	ab28		;next part ...

ab27:
	call aaconvindex
	call skipwhite
	dec	si
	push di
	mov	di,rgnam16
	xor	cx,cx
	call aagetreg
	pop	di
	jc	ab24		;if error
	cmp	byte [di+5],0
	jne	ab24		;if there is already a register
	mov	[di+5],bl
	call skipwhite

;	Ready for the next term within [].

ab28:
	cmp	al,'-'
	je	ab26		;if a (negative) number is next
	cmp	al,'+'
	jne	ab29		;if no next term (presumably)
	jmp	ab18
ab29:
	jmp	ab19		;back for more

;	Post-processing for complicated arguments.

ab30:
	cmp	word [di+4],0
	jnz	ab32		;if registers were given ( ==> create MOD R/M)
	cmp	byte [di+7],0
	jz	ab31		;if nothing was given ( ==> error)
	cmp	byte [di],0
	jnz	ab30b		;if it was not immediate
	or	byte [di],ARG_IMMED
ab30a:
	jmp	ab43		;done with this argument
ab30b:
	or	byte [asm_mn_flags],AMF_ADDR
	mov	al,2		;size of the displacement
	test byte [di+7],4
	jz	ab30c		;if not 32-bit displacement
	inc	ax
	inc	ax
	or	byte [asm_mn_flags],AMF_A32	;32-bit addressing
ab30c:	
	mov	byte [di+3],al	;save displacement size
	jmp	ab30a		;done with this argument
ab31:	
	jmp	aa13b		;flag an error

;	Create the MOD R/M byte.
;	(For disp-only or register, this will be done later as needed.)

ab32:
	or	byte [di],ARG_MODRM
	mov	al,[di+4]
	or	al,[di+5]
	test al,16
	jnz	ab34		;if 32-bit addressing
	test byte [di+7],4
	jnz	ab34		;if 32-bit addressing
;	or byte [asm_mn_flags], AMF_ADDR | AMF_A32
	or byte [asm_mn_flags], AMF_ADDR
	mov	ax,[di+4]
	cmp	al,ah
	ja	ab33		;make sure al >= ah
	xchg al,ah
ab33:
	push di
	mov	di,modrmtab
	mov	cx,8
	repne scasw
	pop	di
	jne	ab31		;if not among the possibilities
	mov	bx,206h		;max disp = 2 bytes; 6 ==> (non-existent) [bp]
	jmp	ab39		;done (just about)

ab34:	
	or	byte [asm_mn_flags],AMF_A32	+ AMF_ADDR;32-bit addressing
	mov	al,[di+4]
	or	al,[di+6]
	jnz	ab35		;if we can't optimize [EXX*1] to [EXX]
	mov	ax,[di+4]
	xchg al,ah
	mov	[di+4],ax
ab35:
	mov	bx,405h		;max disp = 4 bytes; 5 ==> (non-existent) [bp]
	cmp	byte [di+5],0
	jne	ab36		;if there's a SIB
	mov	cl,[di+4]
	cmp	cl,16
	jl	ab31		;if wrong register type
	and	cl,7
	cmp	cl,4		;check for ESP
	jne	ab39		;if not, then we're done (otherwise do SIB)

ab36:
	or	byte [asm_mn_flags],AMF_SIB	;form SIB
	mov	ch,[di+6]	;get SS bits
	mov	cl,3
	shl	ch,cl		;shift them halfway into place
	mov	al,[di+5]	;index register
	cmp	al,20
	je	ab31		;if ESP ( ==> error)
	cmp	al,0
	jne	ab37		;if not zero
	mov	al,20		;set it for index byte 4
ab37:
	cmp	al,16
	jl	ab31		;if wrong register type
	and	al,7
	or	ch,al		;put it into the SIB
	shl	ch,cl		;shift it into place
	inc	cx		;R/M for SIB = 4
	mov	al,[di+4]	;now get the low 3 bits
	cmp	al,0
	jne	ab38		;if there was a first register
	or	ch,5
	jmp	ab42		;MOD = 0, disp is 4 bytes

ab38:
	cmp	al,16
	jl	ab45		;if wrong register type
	and	al,7		;first register
	or	ch,al		;put it into the SIB
	cmp	al,5
	je	ab40		;if it's EBP, then we don't recognize disp=0
				;otherwise bl will be set to 0

;	Find the size of the displacement.

ab39:
	cmp	cl,bl
	je	ab40		;if it's [(E)BP], then disp=0 is still 1 byte
	mov	bl,0		;allow 0-byte disp

ab40:
	push cx
	mov	al,[di+8]
	mov	cl,7
	sar	al,cl
	pop	cx
	mov	ah,[di+9]
	cmp	al,ah
	jne	ab41		;if it's bigger than 1 byte
	cmp	ax,[di+10]
	jne	ab41		;ditto
	mov	bh,0		;no displacement
	or	bl,[di+8]
	jz	ab42		;if disp = 0 and it's not (E)BP
	inc	bh		;disp = 1 byte
	or	cl,40h		;set MOD = 1
	jmp	ab42		;done

ab41:
	or	cl,80h		;set MOD = 2

ab42:
	mov	[di+3],bh	;store displacement size
	mov	[di+4],cx	;store MOD R/M and maybe SIB

;	Finish up with the operand.

ab43:
	dec	si
ab44:
	call skipwhite
	add	di,12
	cmp	al,CR
	je	ab99		;if end of line
	cmp	al,';'
	je	ab99		;if comment (ditto)
	cmp	al,','
	jne	ab45		;if not comma ( ==> error)
	cmp	di,line_out+36
	jae	ab45		;if too many operands
	call skipwhite
	jmp	ab02

ab45:
	jmp	aa13b		;error jump

ab99:
	mov	byte [di],-1;end of parsing phase
	push si			;save the location of the end of the string

;	For the next phase, we match the parsed arguments with the set of
;	permissible argument lists for the opcode.  The first match wins.
;	Therefore the argument lists should be ordered such that the
;	cheaper ones come first.

;	There is a tricky issue regarding sizes of memory references.
;	Here are the rules:
;	   1.	If a memory reference is given with a size, then it's OK.
;	   2.	If a memory reference is given without a size, but some
;		other argument is a register (which implies a size),
;		then the memory reference inherits that size.
;			Exceptions:	OP_CL does not imply a size
;					OP_SHOSIZ
;	   3.	If 1 and 2 do not apply, but this is the last possible argument
;		list, and if the argument list requires a particular size, then
;		that size is used.
;	   4.	In all other cases, flag an error.

ac01:
	xor	ax,ax		;zero out rmaddr through varflags or a_opcode
	mov	di,rmaddr
	mov	cx,7
	rep	stosw
	mov	si,[mneminfo]	;address of the next argument variant

;	Sort out initial bytes.  At this point:
;	si = address of next argument variant
;	di = address of next parsed argument table

ac02:
	lodsb
	sub	al,ASM_MACH0
	jb	ac05		;if no more special bytes
	cmp	al,ASM_LOCKABLE-ASM_MACH0
	je	ac03		;if ASM_LOCKABLE
	ja	ac04		;if ASM_END ( ==> error)
	mov	byte [dismach],al	;save machine type
	jmp	ac02		;back for next byte
ac03:
	or	byte [varflags],VAR_LOCKABLE
	jmp	ac02		;back for next byte

ac04:
	jmp	aa13a		;error

;	Get and unpack the word.

ac05:
	dec	si
	lodsw
	xchg al,ah			;put into little-endian order
	xor	dx,dx
	mov	bx,ASMMOD
	div	bx				;ax = a_opcode; dx = index into opindex
	mov	[a_opcode],ax	;save ax
	mov	[a_opcode2],ax	;save the second copy
	cmp	ax,0dfh
	ja	ac05a			;if not coprocessor instruction
	cmp	al,0d8h
	jb	ac05a			;ditto
	or	byte [dmflags],DM_COPR	;flag it as an x87 instruction
	mov	ah,al			;ah = low order byte of opcode
	lodsb				;get extra byte
	mov	[regmem],al		;save it in regmem
	mov	[a_opcode2],ax	;save this for obsolete-instruction detection
	or	byte [varflags],VAR_MODRM	;flag its presence
ac05a:
	mov	[mneminfo],si	;save si back again
	mov	si,dx
	mov	bl,[opindex+si]
	lea	si,[oplists+bx]	;si = the address of our operand list
	mov	di,line_out

;	Begin loop over operands.  At this point si and di are as for ac02.

ac06:
	lodsb			;get next operand byte
	cmp	al,0
	je	ac10		;if end of list
	cmp	byte [di],-1
	je	ac01		;if too few operands were given
	cmp	al,OP_SIZE
	jb	ac07		;if no size needed
	mov	ah,0
	mov	cl,4
	shl	ax,cl
	shr	al,cl
	mov	[a_reqsize],ah	;save it away
	jmp	ac08

ac07:
	add	al,16-2		;move these to the end of the previous list

ac08:
	cbw
	xchg ax,bx		;now bx contains the offset
	mov	cx,[asm_jmp1+bx] ;subroutine address
	shr	bx,1
	mov	al,[bittab+bx]
	test al,[di]
	jz	ac09		;if no required bits are present
	call cx			;call its specific routine
	cmp	word [si-1],(OP_1632+OP_R)*256+(OP_1632+OP_R_MOD)
	je	ac06		;(hack) for IMUL instruction
	add	di,12		;next operand
	jmp	ac06		;back for more

ac09:
	jmp	ac01		;back to next possibility

;	End of operand list.

ac10:
	cmp	byte [di],-1
	jne	ac09		;if too many operands were given

;	Final check on sizes

	mov	al,[varflags]
	test al,VAR_SIZ_NEED
	jz	ac12		;if no size needed
	test al,VAR_SIZ_GIVN
	jnz	ac12		;if a size was given
	test al,VAR_SIZ_FORCD
	jz	ac09		;if the size was not forced ( ==> reject)
	mov	si,[mneminfo]
	cmp	byte [si],ASM_END
	je	ac12		;if this is the last one
ac11:
	jmp	aa13a		;it was not ==> error (not a retry)

;	Check other prefixes.

ac12:
	mov	al,[aa_saved_prefix]
	cmp	al,0
	jz	ac14		;if no saved prefixes to check
	cmp	al,0f0h
	jne	ac13		;if it's a rep prefix
	test byte [varflags],VAR_LOCKABLE
	jz	ac11		;if this variant is not lockable ( ==> error)
	jmp	ac14		;done

ac13:
	mov	ax,[a_opcode]	;check if opcode is OK for rep{,z,nz}
	and	al,~1		;clear low order bit
	cmp	ax,0ffh
	ja	ac11		;if it's not a 1 byte instruction
	mov	di,replist	;list of instructions that go with rep
	mov	cx,7
	repne	scasb
	jnz	ac11		;if it's not among them

ac14:
	test byte [asm_mn_flags],AMF_MSEG
	jz	ac15		;if no segment prefix before mnemonic
	mov	ax,[a_opcode]	;check if opcode allows this
	cmp	ax,0ffh
	ja	ac11		;if it's not a 1 byte instruction
	mov	di,prfxtab
	mov	cx,P_LEN
	repne	scasb
	jnz	ac11		;if it's not in the list

ac15:
	mov	bx,[immaddr]
	or	bx,bx
	jz	ac16		;if no immediate data
	mov	al,[opsize]
	neg	al
	shl	al,1
	test al,[bx+7]
	jnz	ac11		;if the immediate data was too big

;	Put the instruction together
;	(maybe is this why they call it an assembler)

;	First, the prefixes (including preceding WAIT instruction)

ac16:
	mov	di,[a_addr]
	mov	es,[a_addr+4]
	test byte [asm_mn_flags],AMF_WAIT
	jz	ac17		;if no wait instruction beforehand
	mov	al,9bh
	stosb

ac17:
	mov	al,[aa_saved_prefix]
	cmp	al,0
	jz	ac18		;if no LOCK or REP prefix
	stosb

ac18:	

;--- a 67h address size prefix is needed
;--- 1. for CS32: if AMF_ADDR=1 and AMF_A32=1
;--- 2. for CS16: if AMF_ADDR=1 and AMF_A32=0

	mov al,[asm_mn_flags]
    test al,AMF_ADDR
    jz ac19
	and al,AMF_A32
%if PM
	mov ah,[bCSAttr]
    and ah,40h
    or al,ah
%endif
    and al,AMF_A32 + 40h
    jz ac19
    cmp al,AMF_A32 + 40h
    jz ac19
	mov	al,67h
	stosb
ac19:

;--- a 66h data size prefix is needed
;--- for CS16: if VAR_D32 == 1 or AMF_D32 == 1
;--- for CS32: if VAR_D16 == 1 or AMF_D16 == 1

	mov ah,[asm_mn_flags]
	mov al,[varflags]
%if PM    
    test byte [bCSAttr],40h
    jz ac20
    test al, VAR_D16
    jnz ac20_1
    test ah, AMF_D16
    jnz ac20_1
    jmp ac21
ac20:
%endif
    test al,VAR_D32
    jnz ac20_1
    test ah,AMF_D32
    jz ac21
ac20_1:
	mov	al,66h
	stosb			;store operand-size prefix
ac21:
	mov	al,[aa_seg_pre]
	cmp	al,0
	jz	ac22		;if no segment prefix
	stosb
	cmp	al,64h
	jb	ac22		;if not 64 or 65 (FS or GS)
	or	byte [asm_mn_flags],AMF_FSGS	;flag it

;	Now emit the instruction itself.

ac22:
	mov	ax,[a_opcode]
	mov	bx,ax
	sub	bx,576
	jae	ac23		;if 576-...
	cmp	ax,512
	jb	ac24		;if regular instruction
	or	byte [dmflags],DM_COPR	;flag it as an x87 instruction
	and	al,038h		;get register part
	or	[regmem],al
	xchg	ax,bx		;mov ax,bx (the low bits of bx are good)
	and	al,7
	or	al,0d8h
	jmp	ac25		;on to decoding the instruction

ac23:
	mov	cl,3		;one instruction of a group
	shr	bx,cl
	and	al,7
	shl	al,cl
	or	[regmem],al
	shl	bx,1
	mov	ax,[agroups+bx]	;get actual opcode

ac24:
	cmp	ah,0
	jz	ac25		;if no 0fh first
	push	ax		;store a 0fh
	mov	al,0fh
	stosb
	pop	ax

ac25:
	or	al,[opcode_or]	;put additional bits into the op code
	stosb			;store the op code itself

;	Now store the extra stuff that comes with the instruction.

	mov	ax,word [regmem]
	test byte [varflags],VAR_MODRM
	jz	ac26		;if no mod reg/mem
	stosb
	test byte [asm_mn_flags],AMF_SIB
	jz	ac26		;if no SIB
	dec	di
	stosw			;store the MOD R/M and SIB, too

ac26:
	mov	bx,[rmaddr]
	or	bx,bx
	jz	ac27		;if no offset associated with the R/M
	mov	cl,[bx+3]
	mov	ch,0
	lea	si,[bx+8]
	rep	movsb		;store the R/M offset (or memory offset)

;	Now store immediate data

ac27:
	mov	bx,[immaddr]
	or	bx,bx
	jz	ac28		;if no immediate data
	mov	al,[opsize]
	cbw
	xchg ax,cx		;mov cx,ax
	lea	si,[bx+8]
	rep	movsb

;	Now store additional bytes (needed for, e.g., enter instruction)

ac28:
	mov	bx,[xxaddr]
	or	bx,bx
	jz	ac29		;if no additional data
	lea	si,[bx+4]
	lodsb
	cbw
	xchg ax,cx		;mov cx,ax
	rep	movsb

;	Done emitting.

ac29:	
	push ss		;restore ES
	pop	es
	mov	[a_addr],di

;	Compute machine type.

	cmp	byte [dismach],3
	jae	ac31		;if we already know a 386 is needed
	test byte [asm_mn_flags],AMF_D32 | AMF_A32 | AMF_FSGS
	jnz	ac30		;if 386
	test byte [varflags],VAR_D32
	jz	ac31		;if not 386
ac30:
	mov	byte [dismach],3
ac31:
	mov	di,a_opcode2+2	;info on this instruction
	call showmach		;get machine message into si, length into cx
	jcxz ac33			;if no message

ac32:
	mov	di,line_out
	rep	movsb		;copy the line to line_out
	call putsline

ac33:
	jmp	aa01		;back for the next input line

;	||| This is debugging code.  It assumes that the original value
;	of a_addr is on the top of the stack.

;	pop	si		;get orig. a_addr
;	mov	ax,[a_addr+2]
;	mov	[u_addr],si
;	mov	[u_addr+2],ax
;	mov	bx,[a_addr]
;	sub	bx,si
;	mov	di,line_out
;	mov	cx,10
;	mov	al,' '
;	rep	stosb
;	mov	ds,[a_addr+2]

;ax1:	lodsb
;	call hexbyte		;display the bytes generated
;	dec	bx
;	jnz	ax1
;	push	cs
;	pop	ds
;	call putsline
;	mov	byte disflags,0
;	call disasm		;disassemble the new instruction
;	jmp	aa01		;back to next input line

;	Routines to check for specific operand types.
;	Upon success, the routine returns.
;	Upon failure, it pops the return address and jumps to ac01.
;	The routines must preserve si and di.

;	OP_RM and OP_M:  form MOD R/M byte.

ao01:
	call ao90		;form reg/mem byte
	jmp	ao07		;go to the size check

;	OP_R:  register.

ao02:
	mov	al,[di+4]	;register number
	and	al,7
	mov	cl,3
	shl	al,cl		;shift it into place
	or	[regmem],al	;put it into the reg/mem byte
	jmp	ao07		;go to the size check

;	OP_R_ADD:  register, added to the instruction.

ao03:
	mov	al,[di+4]
	and	al,7
	mov	[opcode_or],al	;put it there
	jmp	ao07		;go to the size check

;	OP_IMM:  immediate data.

ao04:
	mov	[immaddr],di	;save the location of this
	jmp	ao07		;go to the size check

;	OP_MOFFS:  just the memory offset

ao05:
	test byte [di],ARG_MODRM
	jnz	ao11		;if MOD R/M byte ( ==> reject)
	mov	[rmaddr],di	;save the pointer
	jmp	ao07		;go to the size check

;	OP_AX:  check for AL/AX/EAX

ao06:
	test byte [di+4],7
	jnz	ao11		;if wrong register
	;jmp ao07		;go to the size check

;	Size check

ao07:
	or	byte [varflags],VAR_SIZ_NEED
	mov	al,[a_reqsize]
	sub	al,5		;OP_1632 >> 4
	jl	ao12		;if OP_ALL
	jz	ao13		;if OP_1632
	add	al,-3
	adc	al,3		;convert 3 --> 4
ao08:
	or	byte [varflags],VAR_SIZ_FORCD + VAR_SIZ_NEED
	mov	bl,[di+2]
	or	bl,bl
	jz	ao09		;if no size given
	or	byte [varflags],VAR_SIZ_GIVN
	cmp	al,bl
	jne	ao11		;if sizes conflict
ao09:
	cmp	al,[opsize]
	je	ao10		;if sizes agree
	xchg al,[opsize]
	cmp	al,0
	jnz	ao11		;if sizes disagree
ao10:
	ret

ao11:
	jmp	ao50		;reject

;	OP_ALL - Allow all sizes.

ao12:
	mov	al,[di+2]
	cmp	al,1
	je	ao15		;if byte
	jb	ao14		;if unknown
	or	byte [opcode_or],1	;set bit in instruction
	jmp	ao14		;if size is 16 or 32

;	OP_1632 - word or dword.

ao13:
	mov	al,[di+2]
ao14:
	cmp	al,0
	je	ao16		;if still unknown
	cmp	al,2
	jne	ao15_1		;if word
	or	byte [varflags],VAR_D16
    jmp ao15
ao15_1:    
	cmp	al,4
	jne	ao11		;if not dword
	or	byte [varflags],VAR_D32	;set flag
ao15:
	mov	[opsize],al
	or	byte [varflags],VAR_SIZ_GIVN
ao16:
	ret

;	OP_M64 - 64-bit memory reference.
;	OP_MFLOAT - single-precision floating point memory reference.
;	OP_MDOUBLE - double-precision floating point memory reference.
;	OP_M80 - 80-bit memory reference.
;	OP_MXX - memory reference, size unknown.
;	OP_FARMEM - far memory pointer

ao17:
	call ao90		;form reg/mem byte
	mov	al,[asm_siznum+bx-(OP_M64+16-2)/2]
	jmp	ao08		;check size

;	OP_FARP - far memory address contained in instruction

ao21:
	mov	al,2
%if PM    
    test byte [bCSAttr],40h
    jnz ao21_1
%endif    
	cmp	word [di+10],0
	jz	ao22		;if 16 bit address
ao21_1:
	or	byte [varflags],VAR_D32
	mov	al,4
ao22:
	mov	byte [di+4],2
	mov	[immaddr],di
	mov	[opsize],al
ao22a:
	mov	[xxaddr],di
	ret

;	OP_REL8 - relative address

ao23:
	mov	al,9		;short
	call ao60		;check the size
	mov	ax,[a_addr]	;offset
	inc	ax
	inc	ax			;$
    mov cl,[asm_mn_flags]

; JxCXZ, LOOPx, LOOPZx and LOOPNZx need a 67h, not a 66h prefix

	test cl,AMF_D32 | AMF_D16
	jz	ao23b		;if not JxCXZ, LOOPx
    test cl,AMF_D32
    jz ao23a
    or cl,AMF_A32
ao23a:
	and cl,~(AMF_D32 | AMF_D16)
	or  cl, AMF_ADDR
    mov [asm_mn_flags],cl
%if PM
    mov ch,[bCSAttr]
    and ch,40h
%else
	mov ch,0
%endif
	and cl,AMF_A32
	or cl,ch
    jz ao23b
    cmp cl,AMF_A32+40h
    jz ao23b
	inc	ax
ao23b:
	xchg ax,bx
	xor	cx,cx
	mov	ax,[di+8]
	mov	dx,[di+10]
	sub	ax,bx
	sbb	dx,cx
	mov	[di+5],al
	mov	cl,7
	sar	al,cl
	cmp	al,ah
	jne	ao24		;if too big
	cmp	ax,dx
	jne	ao24		;if too big
	mov	byte [di+4],1	;save the length
	jmp	ao22a		;save it away


;	OP_REL1632:  relative jump/call to a longer address.

ao25:
	mov	bx,[a_addr+0]
    mov cx,[a_addr+2]
    mov ax,3
	cmp	word [a_opcode],100h	;???
	jb	ao26
	inc	ax
ao26:
	add bx,ax
    adc cx,0
	mov	dx,[di+10]
	mov	al,[di+2]
	cmp	al,0
	je	ao27		;if no size given
	cmp	al,10
	je	ao27		;if "long" given
	cmp	al,4
	je	ao27_1		;if the size given was "dword"
ao24:    
	jmp	ao50		;reject

ao27:
%if PM
	test byte [bCSAttr],40h
    jnz ao27_1
%endif
	or	dx,dx
	jnz	ao24		;if operand is too big
	mov	al,2
    jmp ao28
ao27_1:
	mov al,4
	or	byte [varflags],VAR_D32
    mov ah,0
    push ax
    dec ax
%if PM
	test byte [bCSAttr],40h
    jz ao27_2
    dec ax			;no prefix needed
ao27_2:
%endif
    add bx,ax
    adc cx,0
    pop ax
ao28:
	mov	[di+4],al	;store the size
	mov	ax,[di+8]
	xor	cx,cx
	sub	ax,bx		;compute DX:AX - CX:BX
	sbb	dx,cx
	mov	[di+5],ax
	mov	[di+7],dx
	mov	[xxaddr],di
	ret

;	OP_1CHK - The assembler can ignore this one.

ao29:
	pop	ax		;discard return address
	jmp	ac06		;next operand

;	OP_STI - ST(I).

ao30:
	mov	al,30		;code for ST
	mov	bl,[di+5]
	jmp	ao38		;to common code

;	OP_CR

ao31:
	mov	al,[di+5]	;get the index
	cmp	al,4
	ja	ao24		;if too big
	jne	ao32		;if not CR4
	mov	byte [dismach],5	;CR4 is new to the 586
ao32:
	cmp	al,1
	jne	ao33
	cmp	byte [di+12],-1
	jne	ao24		;if another arg (can't mov CR1,xx)
ao33:
	mov	al,32		;code for CR
	jmp	ao37		;to common code

;	OP_DR

ao34:
	mov	al,33		;code for DR
	jmp	ao37		;to common code

;	OP_TR

ao35:
	mov	al,[di+5]	;get the index
	cmp	al,3
	jb	ao24		;if too small
	cmp	al,6
	jae	ao36
	mov	byte [dismach],4	;TR3-5 are new to the 486
ao36:
	mov	al,34		;code for TR

;	Common code for these weird registers.

ao37:
	mov	bl,[di+5]
	mov	cl,3
	shl	bl,cl
ao38:
	or	[regmem],bl
	or	byte [varflags],VAR_MODRM
	cmp	al,[di+4]	;check for the right numbered register
	je	ao40		;if yes, then return
ao38a:
	jmp	ao50

;	OP_SEGREG

ao39:
	mov	al,[di+4]
	sub	al,24
	cmp	al,6
	jae	ao38a		;if not a segment register
	mov	cl,3
	shl	al,cl
	or	[regmem],al
ao40:
	ret

;	OP_IMMS8 - Sign-extended immediate byte (PUSH xx)

ao41:
    and byte [varflags],~VAR_SIZ_NEED	;added for v1.09. Ok?
	mov	ax,[di+8]
	mov	cl,7
	sar	al,cl
	jmp	ao43		;common code

;	OP_IMM8 - Immediate byte

ao42:
	mov	ax,[di+8]
	mov	al,0
ao43:
	cmp	al,ah
	jne	ao50		;if too big
	cmp	ax,[di+10]
	jne	ao50		;if too big
	mov	al,1
	call ao60		;check that size == 0 or 1
	mov	ah,[di+8]	;store the byte, length 1
	mov	[di+4],ax
	mov	[xxaddr],di
ao43r:
	ret

;	OP_ECX - CX or ECX, determining the operand size
;   used for LOOPx

ao43m:
%if 0
	cmp	byte [di+4],9	;CX
	je	ao43r			;if CX (do nothing)
	or	byte [varflags],VAR_D32
	mov	al,17
	jmp	ao48a			;require ECX
%endif    
    ret

;	OP_SHOSIZ - force the user to declare the size of the next operand

ao44:
	test	byte [varflags],VAR_SIZ_NEED
	jz	ao45		;if no testing needs to be done
	test	byte [varflags],VAR_SIZ_GIVN
	jz	ao50		;if size was given ( ==> reject)
ao45:
	and	byte [varflags],~VAR_SIZ_GIVN	;clear the flag
	cmp	byte [si],OP_IMM8
	je	ao45a		;if OP_IMM8 is next, then don't set VAR_SIZ_NEED
	or	byte [varflags],VAR_SIZ_NEED
ao45a:
	mov	byte [opsize],0
	pop	ax		;discard return address
	jmp	ac06		;next operand

;	OP_1

ao46:
	cmp	word [di+7],101h
	jmp	ao49		;test it later

;	OP_3

ao47:
	cmp	word [di+7],301h
	jmp	ao49		;test it later

;	OP_DX, OP_CL, OP_ST, OP_ES, ..., OP_GS

ao48:
	mov	al,[asm_regnum+bx-(OP_DX+16-2)/2]
ao48a:
	cbw
	cmp	ax,[di+4]

ao49:
	je	ao51

;	Reject this operand list.

ao50:
	pop	ax			;discard return address
	jmp	ac01		;go back to try the next alternative

ao51:
	ret

;	AASIZCHK - Check that the size given is 0 or AL.

ao60:
	cmp	byte [di+2],0
	je	ao51
	cmp	byte [di+2],al
	je	ao51
	pop	ax		;discard return address
	jmp	ao50

;	Do reg/mem processing.
;	Uses	AX

ao90:
	test byte [di],ARG_JUSTREG
	jnz	ao92		;if just register
	test byte [di],ARG_MODRM
	jz	ao91		;if no precomputed MOD R/M byte
	mov	ax,word [di+4]	;get the precomputed bytes
	jmp	ao93		;done

ao91:
	mov	al,6		;convert plain displacement to MOD R/M
	test byte [asm_mn_flags],AMF_A32
	jz	ao93		;if 16 bit addressing
	dec	ax
	jmp	ao93		;done

ao92:
	mov	al,[di+4]	;convert register to MOD R/M
	and	al,7		;get low 3 bits
	or	al,0c0h

ao93:
	or	word [regmem],ax	;store the MOD R/M and SIB
	or	byte [varflags],VAR_MODRM	;flag its presence
	mov	[rmaddr],di	;save a pointer
	ret			;done

;	AAIFNUM - Determine if there's a number next.
;	Entry	AL	First character of number
;		SI	Address of next character of number
;	Exit	CY	Clear if there's a number, set otherwise.
;	Uses	None.

aaifnum:
	cmp	al,'-'
	je	aai2		;if minus sign (carry is clear)
	push	ax
	sub	al,'0'
	cmp	al,10
	pop	ax
	jb	aai1		;if a digit
	push	ax
	and	al,TOUPPER
	sub	al,'A'
	cmp	al,6
	pop	ax
aai1:	cmc			;carry clear <==> it's a number
aai2:	ret

;	AAGETI - Get a number from the input line.
;	Entry	AL	First character of number
;		SI	Address of next character of number
;	Exit	DX:BX	Resulting number
;		CL	1 if it's a byte, 2 if a word, 4 if a dword
;		AL	Next character not in number
;		SI	Address of next character after that
;	Uses	AH, CH

aageti:
	cmp	al,'-'
	je	aag1		;if negative
	call aag4		;get the bare number
	mov	cx,1		;set up cx
	or	dx,dx
	jnz	aag2		;if dword
	or	bh,bh
	jnz	aag3		;if word
	ret			;it's a byte

aag1:
	lodsb
	call aag4		;get the bare number
	mov	cx,bx
	or	cx,dx
	mov	cx,1
	jz	aag1a		;if -0
	not	dx		;negate the answer
	neg	bx
	cmc
	adc	dx,0
	test dh,80h
	jz	aag7		;if error
	cmp	dx,-1
	jne	aag2		;if dword
	test bh,80h
	jz	aag2		;if dword
	cmp	bh,-1
	jne	aag3		;if word
	test bl,80h
	jz	aag3		;if word
aag1a:
	ret			;it's a byte

aag2:
	inc	cx		;return:  it's a dword
	inc	cx
aag3:
	inc	cx		;return:  it's a word
	ret

aag4:
	xor	bx,bx		;get the basic integer
	xor	dx,dx
	call getnyb
	jc	aag7		;if not a hex digit
aag5:
	or	bl,al		;add it to the number
	lodsb
	call getnyb
	jc	aag1a		;if done
	test dh,0f0h
	jnz	aag7		;if overflow
	mov	cx,4
aag6:
	shl	bx,1		;shift it by 4
	rcl	dx,1
	loop aag6
	jmp	aag5

aag7:
	jmp	aa13b		;error

;	AACONVINDEX - Convert results from AAGETI and store index value
;	Entry	DX:BX,CL As in exit from AAGETI
;		DI	Points to information record for this arg
;	Exit	SS bits stored in [di+6]
;	Uses	DL

aaconvindex:
	cmp	cl,1
	jne	aacv1		;if the number is too large
	cmp	bl,1
	je	aacv2		;if 1
	inc	dx
	cmp	bl,2
	je	aacv2		;if 2
	inc	dx
	cmp	bl,4
	je	aacv2		;if 4
	inc	dx
	cmp	bl,8
	je	aacv2		;if 8
aacv1:
	jmp	aa13b		;error

aacv2:
	mov	[di+6],dl	;save the value
	ret

;	AAGETREG - Get register for the assembler.
;	Entry	DI	Start of register table
;		CX	Length of register table
;		SI	Address of first character in register name
;	Exit	SI	Updated if a register was found
;		BX	Register number, defined as in the table below.
;	Uses	AX, CX, DI

;	Exit value of BX:
;		DI = rgnam816, CX = 27	DI = rgnam16, CX = 8
;		----------------------	--------------------
;		0  ..  7:  AL .. BH	0  ..  7:  AX .. DI
;		8  .. 15:  AX .. DI	16 .. 23:  EAX..EDI
;		16 .. 23:  EAX..EDI
;		24 .. 29:  ES .. GS
;		30 .. 34:  ST .. MM

aagetreg:
	mov	ax,[si]
	and	ax,TOUPPER_W	;convert to upper case
	cmp	al,'E'		;check for EAX, etc.
	jne	aagr1		;if not
	push ax
	mov	al,ah
	mov	ah,[si+2]
	and	ah,TOUPPER
	push di
	mov	di,rgnam16
	push cx
	mov	cx,8
	repne scasw
	mov	bx,cx
	pop	cx
	pop	di
	pop	ax
	jne	aagr1		;if no match
	inc	si
	not	bx
	add	bl,8+16		;adjust BX
	jmp	aagr2		;finish up

aagr1:
	mov	bx,cx		;(if cx = 0, this is always reached with
	repne	scasw		; ZF clear)
	jne	aagr3		;if no match
	sub	bx,cx
	dec	bx
	cmp	bl,16
	jb	aagr2		;if AL .. BH or AX .. DI
	add	bl,8
aagr2:
	inc	si		;skip the register name
	inc	si
	clc
	ret
aagr3:
	stc			;not found
	ret

;	C command - compare bytes.

cc:	
	call parsecm		;parse arguments
%if PM    
    cmp byte [cs:bAddr32],0
	jz cc0_1
    cpu 386
    inc ecx
    jmp cc1
    cpu 8086
cc0_1:
%endif
	inc	cx
cc1:
	push ds
	push es
	push ss		;ds := cs
	pop	ds
	call dohack	;do the interrupt pointer hack
	pop	es
	pop	ds
%if PM    
    cmp byte [cs:bAddr32],0
    jz cc1_1
    repe cmpsb
    lahf
    mov dl,[esi-1]
    mov dh,[es:edi-1]
    jmp cc1_2
cc1_1:    
%endif    
	repe	cmpsb		;start comparing
	lahf
	mov	dl,[si-1]	;save the possibly errant characters
	mov	dh,[es:di-1]
cc1_2:    
	push ds
	push es
	push ss
	pop	ds
	call unhack	;undo the interrupt pointer hack
	pop	es
	pop	ds
	sahf
	jne cc1_21
	jmp cc2		;if we're done
cc1_21:
	push cx
	push es
	push ss
	pop	es
	mov	bx,di
%if PM
    cmp byte [cs:bAddr32],0
    jz cc1_3
    cpu 386
    mov ebx,edi
    cpu 8086
cc1_3:
%endif
	mov	di,line_out
	mov	ax,ds
	call hexword
	mov	al,':'
	stosb
	lea	ax,[si-1]
%if PM
    cmp byte [cs:bAddr32],0
    jz cc1_4
    cpu 386
    lea eax,[esi-1]
    push ax
    shr eax,16
    call hexword
    pop ax
    cpu 8086
cc1_4:    
%endif
	call hexword
	mov	ax,'  '
	stosw
	mov	al,dl
	call hexbyte
	mov	ax,'  '
	stosw
	mov	al,dh
	call hexbyte
	mov	ax,'  '
	stosw
	pop	ax
	push	ax
	call hexword
	mov	al,':'
	stosb
	lea	ax,[bx-1]
%if PM
    cmp byte [cs:bAddr32],0
    jz cc1_5
    cpu 386
    lea eax,[ebx-1]
    push ax
    shr eax,16
    call hexword
    pop ax
    cpu 8086
cc1_5:    
%endif
	call hexword
	push ds
	push ss
	pop	ds
	push bx
	call putsline
	pop	di
	pop	ds
	pop	es
	pop	cx
%if PM    
    cmp byte [cs:bAddr32],0
    jz cc1_6
    cpu 386
    jecxz cc2
	cpu 8086    
    jmp cc1
cc1_6:
%endif
	jcxz cc2
	jmp cc1		;if not done yet
cc2:
	push ss		;restore segment registers
	pop	ds
	push ss
	pop	es
	ret

%if PM

descr    db '     base='
descbase db '         limit='
desclim  db '         attr='
descattr db '    ',CR,LF,'$'

gater	db 'int '
gateno	db '   '
		db '123456789012345678901234567890'	;max pattern is 'XXXX:XXXXXXXX XXXX:XXXXXXXX',CRLF,'$'

cleardesc:    
    push di
    mov ax,'??'
	stosw
	stosw
	stosw
	stosw
    add di,7
    stosw
    stosw
    stosw
    stosw
    add di,6
    stosw
    stosw
    pop di
    ret

descout:    
%if NASM
    cpu 286
%else
    cpu 286 protected
%endif    
    call skipwhite
    call getword	;get word into DX
	mov bx,dx
    call skipcomm0
    mov dx,1
    cmp al,CR
    jz descout_0
    call getword
    call chkeol
    and dx,dx
    jnz descout_0
    inc dx
descout_0:
	mov si,dx		;save count
    call ispm
    jz nextdesc
	mov dx,nodesc
    jmp int21ah9
desc_done:
	ret
nextdesc:
	dec si
    js desc_done
    mov di,descr
    mov ax,bx
    call hexword
    mov di,descbase
    call cleardesc
;    lar ax,bx
;    jnz skipdesc	;tell that this descriptor is invalid
    mov ax,6
    int 31h
    jc desc_o1
    mov ax,cx
    call hexword
    mov ax,dx
    call hexword
desc_o1:    
    mov di, desclim
    cmp byte [machine],3
    jb use16desc
    cpu 386
    lsl eax,ebx
    jnz desc_out
    push ax
    shr eax,16
    call hexword
    pop ax
    call hexword
    lar eax,ebx
    shr eax,8
desc_o2:    
    mov di,descattr
    call hexword
desc_out:    
    mov dx,descr
    call int21ah9
    add bx,8
    jmp nextdesc
%if NASM
    cpu 286
%else
    cpu 286 protected
%endif    
use16desc:
	lsl ax,bx
    jnz desc_out
    call hexword
    mov ax,"  "
    stosw
    stosw
    lar ax,bx
    shr ax,8
    jmp desc_o2

;--- DI command

gateout:    
    call skipwhite
    call getbyte	;get byte into DL
	mov bx,dx
    call skipcomm0
    mov dx,1
    cmp al,CR
    jz gateout_0
    call getbyte	;get byte into DL
    call chkeol
gateout_0:    
    call prephack
    mov si,dx		;save count
gateout_00:    		;<--- next int/exc
    call dohack
    mov di,gateno
    mov al,bl
    call hexbyte
    inc di
    call ispm
    jnz gaterm
%if NASM
    cpu 286
%else
    cpu 286 protected
%endif
    mov ax,204h
    cmp bl,20h
    adc bh,1
gateout_01:
    int 31h
    jc gatefailed
    mov ax,cx
    call hexword
    mov al,':'
    stosb
    cmp byte [dpmi32],0
    jz gate16
    cpu 386
    shld eax,edx,16
    call hexword
    cpu 8086
gate16:    
    mov ax,dx
    call hexword
	mov al,' '
    stosb
    mov ax,0202h
	dec bh
    jnz gateout_01
	mov ax,LF*256 + CR
    stosw
	mov al,'$'
    stosb
    mov dx,gater
gate_exit:    
    call unhack
    call int21ah9
    inc bx
    dec si
    jnz gateout_00
    ret
gaterm:
    mov cl,2
    push bx
    shl bx,cl
    push ds
    xor ax,ax
    mov ds,ax
    mov ax,[bx+2]
    mov dx,[bx+0]
    pop ds
    pop bx
    call hexword
    mov al,':'
    stosb
    mov bh,1
    jmp gate16
gatefailed:
	mov dx,gatewrong
    mov si,1
    jmp gate_exit
%endif

    cpu 8086

%if MCB
mcbout:
    mov di,line_out
    mov ax,"PS"
    stosw
    mov ax,"P:"
    stosw
    mov ax,[pspdbe]
    call hexword
    call putsline	;destroys cx,dx,bx
    
    mov si,[wMCB]
nextmcb:    
    mov di,line_out
    push ds
    call setds2si
    mov ch,[0000]
    mov bx,[0001]	;psp
    mov dx,[0003]
    mov ax,si
    call hexword	;segment address of MCB
    mov al,' '
    stosb
    mov al,ch
    call hexbyte	;'M' or 'Z'
    mov al,' '
    stosb
    mov ax,bx
    call hexword	;MCB owner
    mov al,' '
    stosb
    mov ax,dx
    call hexword	;MCB size in paragraphs
    mov al,' '
    stosb
    and bx,bx
    jz mcbisfree
    push si
    push cx
    push dx
    mov si,8
    mov cx,2 
    cmp bx,si		;is it a "system" MCB?
    jz nextmcbchar
    dec bx
    call setds2bx
    mov cx,8
nextmcbchar:		;copy "name" of owner MCB
    lodsb
    stosb
    and al,al
    loopnz nextmcbchar
    pop dx
    pop cx
    pop si
mcbisfree:    
    pop ds
    add si,dx
    jc mcbout_done
    inc si
    push cx
    call putsline	;destroys cx,dx,bx
    pop cx
    cmp ch,'Z'
    jz  nextmcb
    cmp ch,'M'
    jz  nextmcb
mcbout_done:
	ret
setds2si:
	mov bx,si
setds2bx:
%if PM    
	call ispm
    jnz sd2s_ex
    mov dx,bx
    call setrmsegm
sd2s_ex:
%endif
    mov ds,bx
	ret
%endif

;--- DX command. Display extended memory

%if PM
    cpu 386
extmem:
	mov dx,[x_addr+0]
	mov bx,[x_addr+2]
    call skipwhite
    cmp al,CR
    jz extmem_1
	call getdword	;get linear address into bx:dx
	call chkeol		;expect end of line here
extmem_1:
	mov word [lastcmd],extmem
	push bx
    push dx
    pop ebp
    mov di,real_end	;create a GDT for Int 15h, ah=87h
    xor ax,ax
    mov cx,8
    rep stosw
    mov ax,007Fh
    stosw
    mov ax,dx
    stosw
    mov al,bl
    stosb
    mov ax,0093h
    stosw
    mov al,bh
    stosb
    mov ax,007Fh
    stosw
    mov ax,line_in+128
    mov bx,[pspdbg]
    movzx ebx,bx
    shl ebx,4
    movzx eax,ax
    add eax,ebx
    stosw
    shr eax,16
    stosb
    mov bl,ah
    mov ax,0093h
    stosw
    mov al,bl
    stosb
    mov cx,8
    xor ax,ax
    rep stosw
	call ispm
    mov si,real_end
    mov cx,0040h
    mov ah,87h
    jnz extmem_rm
    push word [cs:pspdbg]
    push 15h
    call intcall
    jmp i15ok
extmem_rm:    
    int 15h
i15ok:
    jc extmem_exit
    mov si,line_in+128
    mov ch,8h
nexti15l:    
	mov di,line_out
    mov eax,ebp
    shr eax,16
	call hexword
    mov ax,bp
	call hexword
    mov ax,"  "
    stosw
    mov bx,line_out+10+3*16
    mov cl,10h
nexti15b:    
    lodsb
    mov ah,al
    cmp al,20h
    jnc noctrl
    mov ah,'.'
noctrl:    
	mov [bx],ah
    inc bx
    push cx
    call hexbyte
    pop cx
    mov al,' '
    stosb
	dec cl
    jnz nexti15b
    mov byte [di-(8*3+1)],'-'	;display a '-' after 8 bytes
    add di,16
    push cx
    call putsline
    pop cx
    add ebp,10h
    dec ch
    jnz nexti15l
    mov [x_addr],ebp
extmem_exit:
    ret
	cpu 8086
%endif

;	D command - hex/ascii dump.

ddd:
	cmp	al,CR
	jne	dd1		;if an address was given
lastddd:    
	mov	dx,[d_addr]	;compute range of 80h or until end of segment
	mov	si,dx
    mov bx,[d_addr+4]
%if PM    
    cmp byte [machine],3
    jb dd0_1
    cpu 386
    mov edx,[d_addr]
    mov esi,edx
    cpu 8086
dd0_1:
%endif
	add	dx,7fh
	jnc	dd2		;if no overflow
	mov	dx,0ffffh
	jmp	dd2

dd1:
%if PM
	or  al,TOLOWER
    cmp al,'l'
    jnz dd1_0
    jmp  descout
dd1_0:    
    cmp al,'i'
    jnz dd1_01
    jmp  gateout
dd1_01:   
	cmp al,'x'
    jnz dd1_02
    cmp byte [machine],3
    jb dd1_02
    jmp extmem
dd1_02:    
%endif
%if MCB
    cmp al,'m'
    jnz dd1_03
    jmp mcbout
dd1_03:
%endif
	mov	cx,80h		;default length
	mov	bx,[reg_ds]
	call getrangeX	;get address range into bx:(e)dx
	call chkeol		;expect end of line here

	mov	[d_addr+4],bx	;save segment (offset is saved later)
	mov	si,dx
%if PM    
    cmp byte [machine],3
    jb dd1_1
    cpu 386
    mov esi,edx
    cpu 8086
dd1_1:
%endif
	mov	dx,cx		;dx = end address
    
    jmp dd2_1

;	Parsing is done.  Print first line.

dd2:	
%if PM
	call ispm
    jnz dd2_1
	cpu 286 protected
    verr bx
    jz dd2_1
    mov bx,[reg_ds]
    mov [d_addr+4],bx
	cpu 8086
%endif
dd2_1:	
	mov word [lastcmd],lastddd
	mov	ax,[d_addr+4]
	call hexword
	mov	al,':'
	stosb
	mov	ax,si
%if PM    
	xor bp,bp
	call ispm
    jnz dd2_2

    cmp byte [machine],3
    jb dd2_2
    push ax
    call getattrhigh	;sets Z flag
    pop ax
    jz dd2_2
    inc bp
    cpu 386
    shld eax,esi,16		;AX=HiWord(esi)
    cpu 8086
	call hexword
    mov ax,si
dd2_2:    
%endif

	and	al,0f0h
	push ax
	call hexword
	mov	ax,'  '
	stosw
	pop	ax
	lea	bx,[di+3*16]
;	mov	byte [bx-1],' '
	call prephack		;set up for faking int vectors 23 and 24

;--- blank the start of the line if offset isn't para aligned

dd3:	
	cmp	ax,si			;skip to position in line
	je	dd4				;if we're there yet
	push ax
	mov	ax,'  '
	stosw
	stosb
	mov	[es:bx],al
	inc	bx
	pop	ax
	inc	ax
	jmp	dd3

;	Begin main loop over lines of output.

dd4:
	mov	cx,si
	or	cl,0fh
	cmp	cx,dx		;compare with end address
	jb	dd5			;if we write to the end of the line
	mov	cx,dx
dd5:
	sub	cx,si
	inc	cx			;cx = number of bytes to print this line
	call dohack		;substitute interrupt vectors
	mov	ds,[d_addr+4]
dd6:    
%if PM
	and bp,bp
    jz dd6_1
    db 67h			;do a lods [esi] 
dd6_1:
%endif
	lodsb
dd6_2:    
	push ax
	push cx
	call hexbyte
	pop	cx
	mov	al,' '
	stosb
	pop	ax
	cmp	al,' '
	jb	dd7		;if control character
	cmp	al,'~'
	jbe	dd8		;if printable
dd7:
	mov	al,'.'
dd8:
	mov	[es:bx],al
	inc	bx
	loop dd6
    
dd9:	
	test si,0fh		;space out till end of line
	jz	dd10
	mov	ax,'  '
	stosw
	stosb
	inc	si
	jmp	dd9

dd10:
	push ss		;restore ds
	pop	ds
	mov	[d_addr],si
%if PM
    cmp byte [machine],3
    jb dd10_1
    cpu 386
    mov [d_addr],esi
    cpu 8086
dd10_1:    
%endif    
	mov	byte [di-25],'-'
	call unhack
	mov	di,bx
	push dx
	call putsline
	pop	dx
	dec	si
	cmp	si,dx
	jae	dd11		;if we're done
	inc	si
%if 0    
	mov	di,line_out+5	;set up for next time
	mov	ax,si
	call hexword
	inc	di
	inc	di
	lea	bx,[di+50]
	jmp	dd4
%else
	mov di,line_out
    mov bx,[d_addr+4]
	jmp dd2
%endif
dd11:	inc	dx		;set up the address for the next 'D' command.
	mov	[d_addr],dx
	ret

errorj4:jmp	error

;	E command - edit memory.

ee:	
	call prephack
	mov	bx,[reg_ds]
	call getaddr	;get address into bx:(e)dx
	call skipcomm0
	cmp	al,CR
	je	ee1			;if prompt mode
	push dx			;save destination offset
	call getstr		;get data bytes
	mov	cx,di
	mov	dx,line_out
	sub	cx,dx		;length of byte string
	pop	di
	mov	ax,cx
	dec	ax
	add	ax,di
	jc	errorj4		;if it wraps around
	call dohack
	mov	si,dx
	mov	es,bx
%if PM
	cmp [bAddr32],byte 0
    jz ee_2
    cpu 386
    mov dx,di		;dx was destroyed
    mov edi,edx
    movzx esi,si
    movzx ecx,cx
    db 67h
    cpu 8086
ee_2:    
%endif
	rep	movsb

;	Restore ds + es and undo the interrupt vector hack.
;	This code is also used by the `m' command.

ee0a:
	push ss			;restore ds
	pop	ds
	push ss			;restore es
	pop	es
	mov	di,run2324	;debuggee's int 23/24 values
	call prehak1	;copy things back
	call unhack
	ret

;	Prompt mode.

ee1:

;	Begin loop over lines.

ee2:				;<--- next line
	mov	ax,bx		;print out segment and offset
	call hexword
	mov	al,':'
	stosb
%if PM    
	cmp [bAddr32],byte 0
    jz ee_21
    cpu 386
    shld eax,edx,16
    cpu 8086
    call hexword
ee_21:    
%endif
	mov	ax,dx
	call hexword

;	Begin loop over bytes.

ee3:				;<--- next byte
	mov	ax,'  '		;print old value of byte
	stosw
	call dohack		;do the INT pointer hack
    push bx
    call readmem	;read mem at BX:(E)DX
    pop bx
	call unhack		;undo the INT pointer hack
	call hexbyte
	mov	al,'.'
	stosb
	push bx
    push dx
	call puts
    pop dx
	pop	bx
	mov	si,line_out+16	;address of buffer for characters
	xor	cx,cx		;number of characters so far

ee4:
	cmp	byte [notatty],0
	jz	ee9			;if it's a tty
	push si
	mov	di,line_in+2
	mov	si,[bufnext]
ee5:
	cmp	si,[bufend]
	jb	ee6			;if there's a character already
	call fillbuf
	mov	al,CR
	jc	ee8			;if eof
ee6:
	cmp	byte [notatty],CR
	jne	ee7			;if no need to compress CR/LF
	cmp	byte [si],LF
	jne	ee7			;if not a line feed
	inc	si			;skip it
	inc	byte [notatty]	;avoid repeating this
	jmp	ee5			;next character

ee7:
	lodsb			;get the character
	mov	[notatty],al
ee8:
	mov	[bufnext],si
	pop	si
	jmp	ee10

ee9:
	mov	ah,8		;console input without echo
	int	21h

ee10:
	cmp	al,' '
	je	ee13		;if done with this byte
	cmp	al,CR
	je	ee13		;ditto
	cmp	al,BS
	je	ee11		;if backspace
	cmp	al,'-'
	je	ee112		;if '-'
	cmp	cx,2		;otherwise, it should be a hex character
	jae	ee4			;if we have a full byte already
	mov	[si],al
	call getnyb
	jc ee4			;if it's not a hex character
	inc	cx
	lodsb			;get the character back
	jmp	ee12
ee112:
	call stdoutal
    dec dx			;decrement offset part
	mov	di,line_out
    jmp ee15
ee11:
	jcxz ee4		;if nothing to backspace over
	dec	cx
	dec	si
    call fullbsout
    jmp ee4
ee12:
	call stdoutal
	jmp	ee4			;back for more

;	We have a byte (if CX != 0).

ee13:	
	jcxz ee14		;if no change for this byte
	mov	[si],al		;terminate the string
	sub	si,cx		;point to beginning
	push cx
    push dx
	lodsb
	call getbyte	;convert byte to binary (DL)
    mov al,dl
    pop dx
	pop	cx
	call dohack		;do the INT pointer hack
	call writemem	;write AL at BX:(E)DX
	mov	di,run2324	;debuggee's int 23/24
	call prehak1	;copy things back
	call unhack		;undo the INT pointer hack

;	End the loop over bytes.

ee14:
	inc	dx			;increment offset
	mov	di,line_out
	cmp	al,CR
	je	ee16		;if done
	test dl,7
	jz	ee15		;if new line
	not	cx
	add	cx,4		;compute 3 - cx
	mov	al,' '
	rep	stosb		;store that many spaces
	jmp	ee3			;back for more

ee15:
	mov	ax,LF * 256 + CR;terminate this line
	stosw
	jmp	ee2			;back for a new line

ee16:
	jmp	putsline	;call putsline and return

;	F command - fill memory

ff:	xor	cx,cx		;get address range (no default length)
	mov	bx,[reg_ds]
	call getrange	;get address range into bx:(e)dx
%if PM
	cmp byte [bAddr32],0
    jz  ff_0
    cpu 386
    sub ecx,edx
    inc ecx
    push ecx
    push edx
    cpu 8086
    jmp ff_01
ff_0:    
%endif
	sub	cx,dx
	inc	cx		;cx = number of bytes
	push cx		;save it
	push dx		;save start address
ff_01:
	call skipcomm0
	call getstr		;get string of bytes
	mov	cx,di
	sub	cx,line_out
%if PM
	cmp byte [bAddr32],0
    jz	ff_1
    cpu 386
    pop edi
    mov es,bx
    cmp ecx,1
    je  ff3_32
    pop eax
    cdq
    div ecx
    or  eax,eax
    jz  ff2_32
ff1_32:
	mov	esi,line_out
	push ecx
	a32 rep	movsb
	pop	ecx
	dec	eax
	jnz	ff1_32
ff2_32:
	mov	ecx,edx
	jecxz ff4		;if no partial copies
	mov	si,line_out
	a32 rep	movsb
	jmp	ff4		;done (almost)
ff3_32:
	pop ecx
    mov al,[line_out]
    a32 rep stosb
    jmp ff4
    cpu 8086
ff_1:    
%endif
	pop	di
	mov	es,bx
	cmp	cx,1
	je	ff3		;a common optimization
	pop	ax		;get size
	xor	dx,dx	;now size in DX:AX
    cmp ax,1
    adc dx,0	;convert 0000:0000 to 0001:0000
	div	cx		;compute number of whole repetitions
	or	ax,ax
	jz	ff2		;if less than one whole rep
ff1:
	mov	si,line_out
	push cx
	rep	movsb
	pop	cx
	dec	ax
	jnz	ff1		;if more to go
ff2:
	mov	cx,dx
	jcxz ff4		;if no partial copies
	mov	si,line_out
	rep	movsb
	jmp	ff4		;done (almost)
ff3:
	pop	cx
	mov	al,[line_out]
    stosb		;cx=0 -> 64 kB
    dec cx
    rep stosb
ff4:
	push ss		;restore es
	pop	es
	ret

;	G command - go.

gg:
	xchg ax,bx		;mov bx,ax
	mov	ax,[reg_cs]	;save original CS
	mov	[run_cs],ax
	mov	[eqladdr+4],ax
	xchg ax,bx		;mov ax,bx
	call parseql	;process =addr
	mov	di,line_out+2
	mov	[di-2],si	;save si

;	Parse the rest of the line to make sure there are no errors.

gg1:				;<--- next BP
	call skipcomm0
	cmp	al,CR
	je	gg2			;if end of line
	call getaddr	;get address into bx:(e)dx
	jmp	gg1			;back for more

;	Store breakpoint bytes in the given locations.

gg2:	
	mov	si,[di-2]
gg3:	
	dec	si
	call skipcomma
	cmp	al,CR
	je	gg4			;if done
	mov	bx,[eqladdr+4]	;default segment
	call getaddr	;get address into bx:(e)dx
    mov al,0cch		;INT 3 instruction
    call writemem
	stosb			;save the old contents
	jmp	gg3			;back for more

;	Now run the program.

gg4:	
;	call seteq		;make the = operand take effect
	call run

;	Restore breakpoint bytes.

	mov	di,line_out+2
	mov	si,[di-2]
gg5:	
	dec	si
	call skipcomma
	cmp	al,CR
	je	gg6			;if done
	mov	bx,[run_cs]	;default segment
	call getaddr	;get address into bx:(e)dx
	mov	al,[di]		;get old contents
	inc	di
	cmp	al,0cch		;this is because of possible duplicate values
	je	gg5			;if it was INT 3 originally
    call writemem	;write AL to BX:(E)DX
	jmp	gg5			;back for more

;	Finish up.  Check if it was one of _our_ breakpoints.

gg6:
	cmp	word [run_int],int3msg
	jne	gg9			;if not CC interrupt
	dec	word [reg_ip]
	mov	si,[line_out]	;loop over the set breakpoints
gg7:
	dec	si
	call skipcomma
	cmp	al,CR
	je	gg8				;if done
	mov	bx,[run_cs]		;default segment
	call getaddr		;get address into bx:(e)dx
    call readmem		;read byte at BX:(E)DX
    cmp al,0CCh
    je gg7

;	It's one of ours.

	call dumpregs
	ret

;	It's not.

gg8:	
	inc	word [reg_ip]
gg9:
	jmp	tt2		;print messages and quit.

;	H command - hex addition and subtraction.

hh:	
	call getdword
	push bx
	push dx
	call skipcomm0
	call getdword
	call chkeol		;expect end of line here
    pop	cx
    pop	ax			;first value in AX:CX, second in BX:DX
    mov	si,ax
    or	si,bx
    jnz	hh32		;32bit values
	mov	ax,cx
	add	ax,dx
    push cx
	call hexword
    pop cx
	mov	ax,'  '
	stosw
	mov	ax,cx
	sub	ax,dx
	call hexword
	call putsline
	ret
hh32:
	mov si,ax
    mov bp,cx			;first value in SI:BP now
	mov	ax,cx
	add	ax,dx
    push ax
    mov ax,si
    adc ax,bx
	call hexword
    pop ax
    call hexword
	mov	ax,'  '
	stosw
	mov	ax,bp
	sub	ax,dx
    push ax
    mov ax,si
    sbb ax,bx
	call hexword
    pop ax
    call hexword
	call putsline
	ret

;	I command - input from I/O port.

ii:
	mov bl,0
    mov ah,al
	and ah,TOUPPER
	cmp ah,'W'
    jne ii_1
    inc bx
    call skipwhite
    jmp ii_2
ii_1:
    cmp [machine],byte 3
    jb  ii_2
	cmp ah,'D'
    jne ii_2
%if 1    
    mov ah,[si-2]		;distiguish 'id' and 'i d'
    and ah,TOUPPER
    cmp ah,'I'
    jnz ii_2
%endif    
    inc bx
    inc bx
    call skipwhite
ii_2:
	call getword		;get word into DX
	call chkeol			;expect end of line here
    cmp bl,1
    jz ii_3
    cmp bl,2
    jz ii_4
	in	al,dx
	call hexbyte
    jmp ii_5
ii_3:
	in ax,dx
	call hexword
    jmp ii_5
ii_4:
	cpu 386
	in eax,dx
    push ax
    shr eax,16
	call hexword
    pop ax
	call hexword
    cpu 8086
ii_5:    
	call putsline
iiret:	ret

errorj5:jmp	error

%if PM
ispm:
	push ax
	mov ax,1686h
    int 2Fh
    and ax,ax
    pop ax
    ret
%endif

setpsp:
	mov ah,50h
%if PM
	call ispm
    jnz setpsp_rm
%if NOEXTENDER
    cpu 286
	push cx
    push dx
    push bx
    push ax
	mov ax,6
    int 31h
    pop ax
    shl cx,12
    shr dx,4
    or dx,cx
    mov bx,dx
    call doscallx
    pop bx
    pop dx
    pop cx
    ret
    cpu 8086
%else    
	jmp doscall_rm
%endif
setpsp_rm:
%endif

%if USESDA
	cmp word [pSDA+2],0
    jz doscall_rm
    push ds
    push si
    lds si,[pSDA]
    mov [si+10h],bx
    pop si
    pop ds
    ret
%else
	jmp doscall_rm
%endif
    
getpsp:
	mov ah,51h
%if PM    
	call ispm
    jnz getpsp_rm
%if NOEXTENDER
    call doscallx
    mov ax,2
    int 31h
    mov bx,ax
    ret
%else    
    jmp doscall_rm
%endif
getpsp_rm:    
%endif

%if USESDA
	cmp word [pSDA+2],0
    jz doscall_rm
    push ds
    lds bx,[pSDA]
    mov bx,[bx+10h]
    pop ds
    ret
%else
	jmp doscall_rm
%endif

doscall:    
%if NOEXTENDER
	call ispm
    jnz doscall_rm
    cpu 286
doscallx:
    push word [cs:pspdbg]
	push 21h
    call intcall
    ret
    cpu 8086
%endif
doscall_rm:    
	int	21h
    ret

%if PM
intcall:    
    cmp byte [cs:dpmi32],0
    jz intcall_pm16
    cpu 386
    movzx edi,di
    cpu 8086
intcall_pm16:    
    push bp
    sub sp,32h
    mov bp,sp
    push es
    push ss
    pop es
    mov [bp+00h],di
    mov [bp+04h],si
    mov [bp+10h],bx
    mov [bp+14h],dx
    mov [bp+18h],cx
    mov [bp+1Ch],ax
    mov ax,[bp+32h]
    mov [bp+08h],ax		;bp
    xor ax,ax
    mov [bp+20h],ax     ;flags
    mov [bp+2Eh],ax		;sp
    mov [bp+30h],ax		;ss
    mov ax,[bp+38h]		;usually [cs:pspdbg]
    mov [bp+22h],ax     ;es
    mov [bp+24h],ax     ;ds
    mov di,bp
    mov bx,[bp+36h]		;int#
    xor cx,cx
    mov ax,0300h
    int 31h
    mov ah,[bp+20h]
    lahf
    mov di,[bp+00h]
    mov si,[bp+04h]
    mov bx,[bp+10h]
    mov dx,[bp+14h]
    mov cx,[bp+18h]
    mov ax,[bp+1Ch]
    pop es
    lea bp,[bp+32h]
    mov sp,bp
    pop bp
    ret 4
%endif    

%if PM

;--- this proc is called in pmode only
;--- DS is unknown!

isextenderavailable:
	cpu 286
    push ds
    push es
    pusha
	push ss
    pop ds
	mov si,szMSDOS
    cmp byte [machine],3
    jb iea_1
    cpu 386
    movzx esi,si
    cpu 286
iea_1:
    mov ax,168ah
    int 2Fh
    cmp al,1
    cmc
    popa
    pop es
    pop ds
    ret
    cpu 8086
szMSDOS:
	db "MS-DOS",0
nodosextinst:
	push ss
    pop ds
	mov dx,nodosext
    jmp int21ah9
%endif

isdebuggeeloaded:    
	mov		ax,[pspdbe]
    cmp		ax,[pspdbg]
    ret
    
;--- ensure a debuggee is loaded
;--- set SI:DI to CS:IP, preserve AX, BX, DX
    
ensuredebuggeeloaded:
    push ax
	call isdebuggeeloaded
    jnz ll5_2
    push bx
    push dx
    call createdummytask
    mov si,[reg_cs]
    mov di,[reg_ip]
	pop dx
    pop bx
ll5_2:
    pop ax
	ret

;	L command - read a program, or disk sectors, from disk.

ll:	
	call parselw	;parse it
	jz	ll1			;if request to read program
%if NOEXTENDER
	call ispm
    jnz ll0_rm
    call isextenderavailable
    jc nodosextinst
ll0_rm:    
%endif
	cmp [cs:usepacket],byte 2
    jb  ll0_1
    mov dl,al		;A=0,B=1,C=2,...
    xor si,si		;read drive
%if VDD
	mov ax,[hVdd]
	cmp ax,-1
	jnz callvddread
%endif
    inc dl			;A=1,B=2,C=3,...
    mov ax,7305h	;DS:(E)BX -> packet
    stc
    int 21h			;use int 21h here, not doscall!
    jmp ll0_2
%if VDD
callvddread:
	mov cx,5
    add cl,[dpmi32]
	DispatchCall
    jmp ll0_2
%endif
ll0_1:
	int	25h
ll0_2:
	mov	dx,reading
	jmp	ww1

;	For .com or .exe files, we can only load at cs:100.  Check that first.

ll1:
	test byte [fileext],EXT_COM+EXT_EXE
	jz	ll4		;if not .com or .exe file
	cmp	bx,[reg_cs]
	jne	ll2		;if segment is wrong
	cmp	dx,100h
	je	ll4		;if address is OK (or not given)
ll2:
	jmp	error		;can only load .com or .exe at cs:100

;	load (any) file (if not .EXE or .COM, load at BX:DX)

ll3:
	cmp	byte [fileext],0
    jnz ll4
	jmp	iiret

;	open file and get length

ll4:
	mov	si,bx		;save destination address, segment
	mov	di,dx		;and offset
	mov	ax,3d00h	;open file for reading
	mov	dx,DTA
	call doscall
	jnc	ll5			;if no error
	jmp	ll16		;error
ll5:
	xchg ax,bx		;mov bx,ax
	mov	ax,4202h	;lseek
	xor	cx,cx
	xor	dx,dx
	int	21h

;	Split off file types
;	At this point:
;		bx	file handle
;		dx:ax	file length
;		si:di	load address (CS:100h for .EXE or .COM)

	test	byte [fileext],EXT_COM | EXT_EXE
	jnz	ll13		;if .com or .exe file

%if PM
;--- dont load a file in protected mode, 
;--- the read loop makes some segment register arithmetic
	call ispm
    jnz ll5_1
    mov dx,nopmsupp
    call int21ah9
    jmp ll12
ll5_1:
%endif

;	Load it ourselves.
;	For non-.com/.exe files, we just do a read, and set BX:CX to the
;	number of bytes read.
	
	call ensuredebuggeeloaded	;make sure a debuggee is loaded
	mov es,[pspdbe]

;	Check the size against available space.

	push	si
	push	bx

	cmp	si,[es:ALASAP]
	pushf
	neg	si
	popf
	jae	ll6				;if loading past end of mem, allow through ffff
	add	si,[es:ALASAP]	;si = number of paragraphs available
ll6:
	mov	cx,4
	xor	bx,bx
ll7:
	shl	si,1
	rcl	bx,1
	loop	ll7
	sub	si,di
	sbb	bx,cx  		;bx:si = number of words left
	jb	ll9			;if already we're out of space
	cmp	bx,dx
	jne	ll8
	cmp	si,ax
ll8:
	jae	ll10		;if not out of space
ll9:
	pop	bx			;out of space
	pop	si
	mov	dx,doserr8	;not enough memory
	call int21ah9	;print string
	jmp	ll12

ll10:
	pop	bx
	pop	si

;	Store length in registers

; seems a bit unwise to modify registers if a debuggee is running 
; but MS DEBUG does it as well

%if 0
	mov cx,[reg_cs]
    cmp cx,[pspdbe]
    jnz noregmodify
    cmp word [reg_ip],100h
    jnz noregmodify
%endif    
	mov	[reg_bx],dx
	mov	[reg_cx],ax
noregmodify:    

;	Rewind the file

	mov	ax,4200h	;lseek
	xor	cx,cx
	xor	dx,dx
	int	21h

	mov	dx,0fh
	and	dx,di
	mov	cl,4
	shr	di,cl
	add	si,di		;si:dx is the address to read to

;	Begin loop over chunks to read

ll11:	
	mov	ah,3fh		;read from file into DS:(E)DX
	mov	cx,0fe00h	;read up to this many bytes
	mov	ds,si
	int 21h
    
	add	si,0fe0h	;wont work in protected-mode!
	cmp	ax,cx
	je	ll11		;if end of file reached

;	Close the file and finish up.

ll12:
	mov	ah,3eh		;close file
	int	21h
	push ss		;restore ds
	pop	ds
	ret			;done

ll13:

;   file is .EXE or .COM
;	Close the file

	push ax
	mov	ah,3eh		;close file
	int	21h
	pop	bx		;dx:bx is the file length

%if 1

;	adjust .exe size by 200h (who knows why)

	test	byte [fileext],EXT_EXE
	jz	ll14		;if not .exe
	sub	bx,200h
	sbb	dx,0
%endif

;	Clear registers

ll14:
    push bx
    push dx
;	mov	[reg_bx],dx
;	mov	[reg_cx],bx

;--- cancel current process (unless there is none)
;--- this will also put cpu back in real-mode!!!

	call isdebuggeeloaded
    jz notask
	call freemem	
notask:

	mov di, regs
    mov cx, 16*2	;(8 std, 6 seg, ip, fl) * 2
    xor ax, ax
    rep stosw
    
	pop word [reg_bx]
    pop word [reg_cx]

;	Fix up interrupt vectors in PSP

	mov	si,CCIV		;address of original INT 23 and 24 (in PSP)
	mov	di,run2324
	movsw
	movsw
	movsw
	movsw

;	Actual program loading.  Use the DOS interrupt.

	mov	ax,4b01h	;load program
	mov	dx,DTA		;offset of file to load
	mov	bx,execblk	;parameter block
	int	21h		;load it
	jnc ll14_1
	jmp ll16		;if error
ll14_1:
	mov	ax,sp
	sub	ax,[SPSAV]
	cmp	ax,80h
	jb	ll15		;if in range
	mov	ax,80h
ll15:
	mov	[spadjust],ax
	les	si,[execblk+14]
	es lodsw		;recover ax
	mov	[reg_ax],ax
	mov	[reg_sp],si
	mov	[reg_ss],es
	les	si,[execblk+18]
	mov	[reg_ip],si
	mov	[reg_cs],es
    mov byte [bInit],0
	push cs
	pop	es
    clc
    pushf
    pop word [reg_fl]
    call getpsp
	xchg	ax,bx		;mov ax,bx
	mov	[pspdbe],ax
	mov	di,reg_ds
	stosw
	stosw			;reg_es
	mov	bx,cs
    call setpsp

;	Finish up.  Set termination address.

	mov	ax,2522h	;set interrupt vector 22
	mov	dx,int22
	int	21h
	mov	ds,[pspdbe]
	mov	word [TPIVOFS],int22
	mov	[TPIVSEG],cs
	push cs
	pop	ds

;	Set up initial addresses for 'a', 'd', and 'u' commands.

adusetup:
	mov	ax,[reg_ip]
    mov cx,[regh_eip]
	mov	bx,[reg_cs]
	mov	di,a_addr
	stosw				;IP
    mov [di],cx
    add di,2
	xchg	ax,bx
	stosw				;CS
	xchg	ax,bx		;d_addr
	stosw				;IP
    mov [di],cx
    add di,2
	xchg	ax,bx
	stosw				;CS
	xchg	ax,bx		;u_addr
	stosw				;IP
    mov [di],cx
    add di,2
	xchg	ax,bx
	stosw				;CS
	ret

;	Error messages.  Print and quit.

ll16:
	call ww15		;print error message
	ret

;	M command - move from place to place.

;   first check if there is more than 1 argument
;   0 or 1 arguments are handled by the 'M [cpu]' code
mm:
	push si
    cmp al,CR
    jz m1
    call getdword
	cmp al,CR
    jz m1
    call skipwhite
m1:    
    pop si
    dec si
    cmp al,CR
    je m5 				; Normal 'm' command (move).
	lodsb
	call parsecm		;parse arguments (DS:(E)SI, ES:(E)DI, ECX)
	push cx
%if PM    
    call ispm
    jnz m2_1
;--- TODO: do this check in protected-mode    
	clc
    jmp m3
m2_1:    
%endif    
	mov	cl,4
	shr	dx,cl
	add	dx,bx		;upper 16 bits of destination
	mov	ax,si
	shr	ax,cl
	mov	bx,ds
	add	ax,bx
	cmp	ax,dx
	jne	m3		;if we know which is larger
	mov	ax,si
	and	al,0fh
	mov	bx,di
	and	bl,0fh
	cmp	al,bl
m3:	pop	cx
	lahf
	push ds
	push es
	push ss		;ds := cs
	pop	ds
	call dohack	;do the interrupt pointer hack
	pop	es
	pop	ds
%if PM
	cmp byte [cs:bAddr32],0
    jz m3_1
    cpu 386
    sahf
    jae m4_32
    add esi,ecx
    add edi,ecx
    std
m4_32:
	a32 rep movsb
    a32 movsb
    cld
    jmp ee0a
    cpu 8086
m3_1:    
%endif
	sahf
	jae	m4		;if forward copy is OK
	add	si,cx
	add	di,cx
	std
m4:	
	rep	movsb		;do the move
	movsb			;one more byte
	cld				;restore flag
	jmp	ee0a		;restore ds and es and undo the int pointer hack

;	Other 'm' command:  set machine type.

m5:
	lodsb
	cmp	al,CR
	je	m10		;if just an 'm' (query machine type)
	cmp	al,'?'
	je	m10		;if '?' (also query)
	cmp	al,'0'
	jb	m6		;if not a digit
	cmp	al,'6'
	ja	m6		;ditto
	sub	al,'0'
	mov	[machine],al	;set machine type
	mov	[mach_87],al	;coprocessor type, too
    cmp al,3
    jnc m5_1
	and [regsdmp],byte ~1	;reset 386 register display
m5_1:    
	ret

m6:	or	al,TOLOWER
	cmp	al,'c'
	je	m7		;if coprocessor declaration
	cmp	al,'n'
	jne	errorj3		;if something else
	lodsb
	or	al,TOLOWER
	cmp	al,'c'
	jne	errorj3		;if not 'c' after that
	lodsb
	call chkeol
	mov	byte [has_87],0	;clear coprocessor flag
	ret			;done

m7:	
	call skipwhite	;get next nonblank character
	mov	ah,[machine]
	cmp	ah,3
	jb	m9		;if not a 386
	cmp	al,'3'
	je	m8		;if declaring a 387
	cmp	al,'2'
	jne	m9		;if not '2'
	mov	ah,2
m8:
	call skipwhite
m9:	
	cmp	al,CR
	jne	errorj3		;if not end of line
	mov	byte [has_87],1	;set coprocessor flag
	mov	[mach_87],ah	;set copr. type
	ret

;	Display machine type.

m10:
	mov	si,msg8088
	mov	al,[machine]
	cmp	al,0
	je	m11		;if 8088
	mov	si,msgx86
	add	al,'0'
	mov	[si],al
m11:
	call showstring
	mov	si,no_copr
	cmp	byte [has_87],0
	je	m12		;if no coprocessor
	mov	si,has_copr
	mov	al,[mach_87]
	cmp	al,[machine]
	je	m12		;if has coprocessor same as processor
	mov	si,has_287
m12:
	call showstring	;show string
	jmp	putsline	;call puts and quit

errorj3:jmp	error

;	N command - change the name of the program being debugged.

nn:	mov	di,DTA		;destination address

;	Copy and canonicalize file name.

nn1:
	cmp	al,CR
	je	nn3		;if end of line
	call	ifsep		;check for separators space, TAB, comma, ;, =
	je	nn3		;if end of file name
	cmp	al,[swch1]
	je	nn3		;if '/' (and '/' is the switch character)
	cmp	al,'a'
	jb	nn2		;if not lower case
	cmp	al,'z'
	ja	nn2		;ditto
	and	al,TOUPPER	;convert to upper case
nn2:
	stosb
	lodsb
	jmp	nn1		;back for more

nn3:
	mov	al,0		;null terminate the file name string
	stosb
	mov	[execblk+2],di	;save start of command tail

;	Determine file extension

	cmp	di,DTA+1
	je	nn3d		;if no file name at all
	cmp	di,85h
	jb	nn3c		;if no extension (name too short)
	mov	al,EXT_HEX
	cmp	word [di-5],'.H'
	jne	nn3a		;if not .HEX
	cmp	word [di-3],'EX'
	je	nn3d		;if .HEX
nn3a:
	mov	al,EXT_EXE
	cmp	word [di-5],'.E'
	jne	nn3b		;if not .EXE
	cmp	word [di-3],'XE'
	je	nn3d		;if .EXE
nn3b:
	mov	al,EXT_COM
	cmp	word [di-5],'.C'
	jne	nn3c		;if not .COM
	cmp	word [di-3],'OM'
	je	nn3d		;if .COM
nn3c:
	mov	al,EXT_OTHER
nn3d:
	mov	[fileext],al

;	Finish the N command

	push	di
	mov	di,line_out
	dec	si
nn4:
	lodsb			;copy the remainder to line_out
	stosb
	cmp	al,CR
	jne	nn4

;	Set up FCBs.

	mov	si,line_out
	mov	di,5ch
	call nn6		;do first FCB
	mov	byte [reg_ax],al
	mov	di,6ch
	call nn6		;second FCB
	mov	byte [reg_ax+1],al

;	Copy command tail.

	mov	si,line_out
	pop	di
	push di
	inc	di
nn5:
	lodsb
	stosb
	cmp	al,CR
	jne	nn5		;if not end of string
	pop	ax		;recover old DI
	xchg ax,di
	sub	ax,di		;compute length of tail
	dec	ax
	dec	ax
	stosb
	ret			;done

;	Subroutine to process an FCB.

nn6:
	lodsb
	cmp	al,CR
	je	nn7		;if end
	call	ifsep
	je	nn6		;if separator
	cmp	al,[switchar]
	je	nn10		;if switch character
nn7:
	dec	si
	mov	ax,2901h	;parse filename
	call doscall
	push	ax		;save AL
nn8:
	lodsb			;skip till separator
	cmp	al,CR
	je	nn9		;if end
	call	ifsep
	je	nn9		;if separator character
	cmp	al,[swch1]
	jne	nn8		;if not switchar (sort of)
nn9:
	dec	si
	pop	ax		;recover AL
	cmp	al,1
	jne	nn9a		;if not 1
	dec	ax
nn9a:
	ret

;	Handle a switch (differently).

nn10:	lodsb
	cmp	al,CR
	je	nn7		;if end of string
	call	ifsep
	je	nn10		;if another separator
	mov	al,0
	stosb
	dec	si
	lodsb
	cmp	al,'a'
	jb	nn11		;if not a lower case letter
	cmp	al,'z'
	ja	nn11
	and	al,TOUPPER	;convert to upper case
nn11:	stosb
	mov	ax,'  '
	stosw
	stosw
	stosw
	stosw
	stosw
	xor	ax,ax
	stosw
	stosw
	stosw
	stosw
	ret			;return with AL=0

;	O command - output to I/O port.

oo:
	mov bl,0
    mov ah,al
	and ah,TOUPPER
	cmp ah,'W'
    jne oo_1
    inc bx
    call skipwhite
    jmp oo_2
oo_1:
    cmp [machine],byte 3
    jb  oo_2
	cmp ah,'D'
    jne oo_2
%if 1    
    mov ah,[si-2]		;distiguish 'od' and 'o d'
    and ah,TOUPPER
    cmp ah,'O'
    jnz oo_2
%endif    
    inc bx
    inc bx
    call skipwhite
oo_2:
	call getword
	push dx
	call skipcomm0
    cmp bl,1
    jz  oo_4
    cmp bl,2
    jz  oo_5
	call getbyte	;DL
	call chkeol		;expect end of line here
	xchg ax,dx		;al = byte
	pop	dx			;recover port number
	out	dx,al
	ret
oo_4:    
	call getword
	call chkeol		;expect end of line here
	xchg ax,dx		;al = byte
	pop	dx
	out	dx,ax
	ret
oo_5:    
	cpu	386
	call getdword
	call chkeol		;expect end of line here
    push bx
    push dx
    pop	eax
	pop	dx
	out	dx,eax
	ret
    cpu 8086

;--- ensure segment in BX is writeable
;--- out: Carry=1 if segment not writeable
;--- might clear HiWord(EDI)

verifysegm:
%if PM
	call ispm
    jnz is_rm
    push ax
    push di
    push bp
    mov bp,sp
    sub sp,8
    mov di,sp
    cmp [machine],byte 3
    jb vs_1
    cpu 386
    movzx edi,di
    cpu 8086
vs_1:    
    mov ax,000Bh		;get descriptor
    int 31h
    jc vs_2
	test byte [di+5],8	;code segment
    jz vs_2
    and byte [di+5],0F3h;reset CODE+conforming attr
    or byte [di+5],2	;set writable
    mov bx,[scratchsel]
    mov ax,000Ch
    int 31h
vs_2:
    mov sp,bp
    pop bp
    pop di
    pop ax
    ret
is_rm:
%else
	clc
%endif
    ret

%if PM
	cpu 286
setrmsegm:
    mov bx,[cs:scratchsel]
setrmaddr:		;<--- set selector in BX to segment address in DX
    mov cx,dx
    shl dx,4
    shr cx,12
    mov ax,7
    int 31h
    ret
    cpu 8086
%endif

%if PM

; get attribute of current CS
; save it in bCSAttr
; return it in AH, set Z flag if 16bit
; called by P, T, U

getcsattr:
	mov ah,00
    cmp byte [machine],3
    jb getcsattr_1
    call ispm
    jnz getcsattr_1
    mov bx,[reg_cs]
    call getattrhigh
getcsattr_1:    
    mov [bCSAttr],ah
    test ah,40h
    ret
%endif

;--- read [EIP+x] value 
;--- CX=x, CS=[reg_cs], EIP=[regh_eip] & [reg_ip]
;--- returns byte in AL
;--- called by T

getcseipbyte:
    push es
    mov es,[reg_cs]
%if PM
	test byte [bCSAttr],40h 
    jz isrmipx
    cpu 386
    mov bx,[regh_eip]
    shl ebx,16
    mov bx,[reg_ip]
    push edx
    movsx edx,cx
    add ebx,edx
	mov	al,[es:ebx]
    pop edx
    pop es
    ret
    cpu 8086
isrmipx:    
%endif
	mov bx,[reg_ip]
    add bx,cx
	mov	al,[es:bx]
    pop es
    ret
    
;--- set [EIP+x] value 

setcseipbyte:
    push es
    mov bx,[reg_cs]
    call verifysegm
    jc scib_1
    mov es,bx
%if PM
	test byte [bCSAttr],40h 
    jz isrmipxx
    cpu 386
    push word [regh_eip]
    push word [reg_ip]
    pop ebx
    push edx
    movzx edx,cx
	mov	[es:ebx+edx],al
    pop edx
    pop es
    ret
    cpu 8086
isrmipxx:   
%endif
	mov bx,[reg_ip]
    add bx,cx
	mov	[es:bx],al
scib_1:    
    pop es
    ret

;--- write a byte (AL) at BX:E/DX
;--- OUT: AL=old value at that location

writemem:
%if PM
    call ispm
    jnz weip16
    call verifysegm	;make bx a writeable segment
    jc weip_1
    cmp byte [machine],3
    jb  weip16
    push ax
    call getattrhigh
    pop ax
    jz weip16
    cpu 386
    push ds
    mov ds,bx
    xchg al,[edx]
    pop ds
    cpu 8086
weip_1:    
    ret
weip16:
%endif
	push ds
	mov	ds,bx
    push bx
    mov bx,dx
	xchg al,[bx]
    pop bx
    pop ds
    ret

;--- read byte at BX:EDX into AL

readmem:    
%if PM    
    call ispm
    jnz reip16
    cmp [machine],byte 3
    jb reip16
    call getattrhigh
    jz reip16
    push ds
    mov ds,bx
    mov al,[edx]
    pop ds
    ret
reip16:    
%endif    
    push ds
    mov ds,bx
    mov bx,dx
    mov al,[bx]
    pop ds
    ret

%if PM

;--- DS:SI -> protected-mode "flag" (WORD)
;--- returns AX<>0000 if real-mode
;--- modifies BX from segment to selector if switch to pmode
;--- BX:(E)DX = CS:EIP

IsSegm2Sel:    
    mov ax,1686h
    int 2fh
    cmp ax,[si]
    jz nomodechange	;has mode changed?
    and ax, ax
    jnz nomodechange;current mode protected-mode?
    push ax
	mov ax,2
	int 31h			;then transform saved CS to a selector
    mov bx,ax
    pop ax 
    cmp [machine],byte 3
    jb nomodechange
    cpu 386
    movzx edx,dx	;ensure hiword edx is cleared
    cpu 8086
nomodechange:
	ret
%endif

;--- check if an unexpected int 3 has occured
;--- CS:(E)IP in reg_cs:regh_eip & reg_ip
;--- in: DX=0, out: DX=msg
;--- called by P, T

IsUnexpInt3:    
%if PM
	call getcsattr
%endif    
    mov  cx,-1
    call getcseipbyte
    cmp al,0CCh
    jz isunexp_exit
	mov	dx,int3msg
	mov	[reg_ip],bx
%if PM    
    test ah,40h
    jz isunexp_exit
    cpu 386
    shr ebx,16
    mov [regh_eip],bx
    cpu 8086
%endif    
isunexp_exit:
	ret

;	P command - proceed (i.e., skip over call/int/loop/string instruction).

pp:	
	call parse_pt	;process arguments

;	Do it <CX=count> times.  First check the type of instruction.

pp1:
	push cx			;save cx
	mov	dx,15		;DL = number of bytes to go; DH = prefix flags.
%if PM
	call getcsattr
    jz iseip16
    cpu 386
    mov si,[regh_eip]
    shl esi,16
    mov dh,1+2
    cpu 8086
iseip16:    
%endif    
	mov	si,[reg_ip]
pp2:
	call pp16		;get next instruction byte into AL
	mov	di,ppbytes
	mov	cx,PPLEN
	repne scasb
	jne	pp5		;if not one of these
	mov	al,[di+PPLEN-1]	;get corresponding byte in ppinfo
	test al,80h
	jz	pp3		;if not a prefix
;	or	dh,al	;set the flags
	xor	dh,al	;set the flags
	dec	dl
	jnz	pp2		;if not out of bytes
	jmp	pp12

pp3:
	test al,40h
	jz	pp4		;if no size dependency
	and	al,3fh
	and	dh,2
	add	al,dh
pp4:	
	cbw
	call addeip	;add ax to (E)SI
	jmp	pp11	;we have a skippable instruction here

pp5:
	cmp	al,0ffh
    jz pp5_0
	jmp	pp12	;just an ordinary instruction
pp5_0    
	call pp16	;get MOD REG R/M byte
	and	al,~8	;clear lowest bit of REG field (/3 --> /2)
	xor	al,10h	;/2 --> /0
	test al,38h
    jz pp5_1
	jmp	pp12	;if not ff/2 or ff/3
pp5_1:    
	cmp	al,0c0h
	jae	pp11	;if just a register
	test dh,1
	jnz	pp6		;if 32 bit addressing
	cmp	al,6
	je	pp9		;if just plain disp16
	cmp	al,40h
	jb	pp11		;if indirect register
	cmp	al,80h
	jb	pp10	;if disp8[reg(s)]
	jmp	pp9		;it's disp16[reg(s)]

pp6:
	cmp	al,5
	je	pp8		;if just plain disp32
	xor	al,4
	test al,7
	jnz	pp7		;if no SIB byte
	call inceip
pp7:
	cmp	al,40h
	jb	pp11	;if indirect register
	cmp	al,80h
	jb	pp10	;if disp8[reg(s)]
				;otherwise, it's disp32[reg(s)]
pp8:
	call inceip
    call inceip
pp9:
	call inceip
pp10:
	call inceip

pp11:
	mov	bx,[reg_cs]
    jmp pp11x_1

;	Special instruction.  Set a breakpoint and run until we hit it.
;   BX:(E)SI == CS:(E)IP to set breakpoint

pp11x:			;<--- used by T if an INT is to be processed
	mov cx,1
	push cx
pp11x_1:		;BX=CS
	mov dx,si
    cmp [machine],byte 3
    jb pp11x_2
    cpu 386
    mov edx,esi
    cpu 8086
pp11x_2:
	mov	al,0cch
    call writemem
    jnc pp11x_3
    mov dx,cantwritebp
    jmp prnquit
pp11x_3:    
	mov	di,line_out	;save old byte here
	stosb
	xchg ax,si
	stosw
%if PM
	cmp byte [machine],3
    jb noeipsave
    cpu 386
    shld eax,esi,16		;save hiword ESI if machine is a 80386+
	stosw
    cpu 8086
noeipsave:    
%endif
	mov	ax,bx
	stosw
%if PM    
    mov ax,1686h
    int 2Fh
    stosw
%endif    
	call run
    
	mov	si,line_out
	lodsb			;get old byte
	xchg ax,cx		;into CL
	lodsw			;old IP
	xchg ax,dx
%if PM
	cmp byte [machine],3
    jb noeipsave_2
    cpu 386
    lodsw
    push ax
    push dx
    pop edx
    cpu 8086
noeipsave_2:    
%endif
	lodsw			;old CS
	xchg ax,bx		;CS -> BX
%if PM
	call IsSegm2Sel	;change BX to selector if switch to pmode
%endif
	mov al,cl
    call writemem	;write byte AL at BX:E/DX
	xor	dx,dx		;set flag
	cmp	word [run_int],int3msg
	jne	pp13		;if not CC interrupt
    call IsUnexpInt3;test byte at reg_cs:reg_eip-1 if CCh	
    jmp pp13

;	Ordinary instruction.  Just do a trace.

pp12:	
	or byte [reg_fl+1],1	;set single-step mode
	call run
	mov	dx,int1msg

;	Common part to finish up.

pp13:
	cmp	[run_int],dx
	jne	pp15		;if some other interrupt
	call dumpregs
	pop	cx
	loop pp14		;back for more
	ret

pp14:
	jmp	pp1			;back for more
pp15:
	jmp	tt2			;print message about unexpected interrupt
					;and quit

ppout:    			;run code until next RETF
    mov bx,[reg_sp]
    push ds
    mov ds,[reg_ss]
    mov si,[bx+0]
    mov bx,[bx+2]
    pop ds
    jmp pp11x

inceip:    
%if PM
	test byte [bCSAttr],40h
    jnz inceip_1
%endif    
	inc	si
    ret
%if PM
	cpu 386
inceip_1:
	inc esi
    ret
    cpu 8086
%endif
addeip:
%if PM
	test byte [bCSAttr],40h
    jnz addeip_1
%endif    
	add	si,ax
    ret
%if PM    
	cpu 386
addeip_1:
	movzx eax,ax
	add	esi,eax
    ret
    cpu 8086
%endif

;	PPX - Get next byte in instruction stream.

pp16:
	push ds
	mov	ds,[reg_cs]
%if PM
	test byte [cs:bCSAttr],40h
    jz pp16_2
    db 67h	;is a a32
pp16_2:
%endif
	lodsb
	pop	ds
	ret

;	begin loop over instruction bytes.

;	Q command - quit.

qq:	
%if PM
	mov byte [dpmidisable+1],0	;disble DPMI hook
	inc byte [bNoHook2F]		;avoid a new hook while terminating
%endif

; cancel child's process if any
; this will drop to real-mode if debuggee is in pmode

	call freemem

%if VDD
	mov ax,[hVdd]
    cmp ax,-1
    jz novddrelease
    UnRegisterModule
novddrelease:
%endif

;	Restore interrupt vectors.

	mov	di,intsave
	mov	si,inttab
%if PM
    mov cx,5
%else
    mov cx,4
%endif    
nextint:
	lodsb
    mov bl,al
    add si,2
    xchg si,di
	lodsw
    mov dx,ax
    lodsw
    xchg si,di
    cmp bl,22h
    jz norestore
    and ax,ax
    jz norestore
    push ds
    mov ds,ax
    mov al,bl
	mov	ah,25h
	int	21h
    pop ds
norestore:
    loop nextint

;	Restore termination address.

	mov	si,psp22	;restore termination address
	mov	di,TPIVOFS
	movsw
	movsw
	mov	di,16h		;restore PSP of parent
	movsw

;	Really done.

	int	20h
    jmp cmd3		;returned? then something is terribly wrong.

;	R command - manipulate registers.

rr:	cmp	al,CR
	jne	rr1		;if there's an argument
	call dumpregs
	ret

rr1:and al,TOUPPER
	cmp al,'X'
    jne rr1x
    call skipwhite
    cmp al,CR
    je rr1_0
    jmp no386_5    
rr1_0:    
    cmp [machine],byte 3
    jb  rr1_w
    xor [regsdmp],byte 1
    mov ax,"n "
    jnz rr1_x1
    mov ax,"ff"
rr1_x1    
	mov word [regs386s],ax
	mov	di,line_out
    mov si,regs386
    call showstring
    call putsline
rr1_w:    
    ret
rr1x:    
	cmp al,'N'
    jne rr1y
    lodsb
    cmp al,CR
    je rr1x_0
    jmp no386_5    
rr1x_0:    
    cmp [has_87],byte 0
    jz  rr1x_1
    call dumpregsF
rr1x_1:    
    ret
rr1y:
	dec	si
	lodsw
	and	ax,TOUPPER_W
	mov	di,regnames
	mov	cx,16
	repne scasw
	mov	bx,di
	mov	di,line_out
	jne	rr2			;if not found
    cmp [si],byte 20h	;avoid "ES" to be found for "ESI" or "ESP"
    ja rr2
	stosw			;print register name
	mov	al,' '
	stosb
	sub	bx,regnames+2
	call skipcomma	;skip white spaces
	cmp	al,CR
	jne	rr1a		;if not end of line
	push bx			;save bx for later
	mov	ax,[bx+regs]
	call hexword
	call getline0	;prompt for new value
	pop	bx
	cmp	al,CR
	je	rr1b		;if no change required
rr1a:
	call getword
	call chkeol		;expect end of line here
	mov	[bx+regs],dx;save new value
	cmp	bx,reg_ip - regs
	jne	rr1b		;if not changing IP
rr1b:	ret

;	Change flags

rr2:
	cmp	al,'F'
	jne	rr6			;if not 'f'
	dec	si
	lodsb
	cmp	al,CR
	je	rr2b		;if end of line
	cmp	al,' '
	je	rr2a		;if white space
	cmp	al,TAB
	je	rr2a		;ditto
	cmp	al,','
    je rr2a
	jmp	errorj9		;if not, then it's an error
rr2a:
	call skipcomm0
	cmp	al,CR
	jne	rr3			;if not end of line
rr2b:	
	call dmpflags
	call getline0	;get input line (using line_out as prompt)
rr3:	
	cmp	al,CR
	je	rr1b		;return if done
	dec	si
	lodsw
	and	ax,TOUPPER_W	;here's the mnemonic
	mov	di,flgnams
	mov	cx,16
	repne scasw
	jne	rr6		;if no match
	cmp	di,flgnons
	ja	rr4		;if we're clearing
	mov	ax,[di-16-2]
	not	ax
	and	[reg_fl],ax
	jmp	rr5

rr4:	
	mov	ax,[di-32-2]
	or	[reg_fl],ax

rr5:	
	call skipcomma
	jmp	rr3		;if done

rr6:
	cmp [machine],byte 3
    jb  no386_5
    cmp al,'E'
    jnz no386_5
    lodsb
    and al,TOUPPER
    xchg al,ah
    mov cx,16
	mov	di,regnames
	repne	scasw
	jne	no386_5
	mov	bx,di
	mov	di,line_out
    mov [di],byte 'E'
    inc di
    stosw
	mov	al,' '
	stosb
	sub	bx,regnames+2
	call skipcomma	;skip white spaces
	cmp	al,CR
	jne	rr1aX   	;if not end of line
	push	bx		;save bx for later
	mov	ax,[bx+regshi]
	call hexword
	mov	ax,[bx+regs]
	call hexword
    
	call getline0	;prompt for new value
	pop	bx
	cmp	al,CR
	jne	rr1aX
	jmp rr1b		;if no change required
rr1aX:
	push bx
	call getdword
    mov	cx,bx
	pop	bx
	call chkeol		;expect end of line here
	mov	[bx+regs],dx	;save new value
	mov	[bx+regshi],cx	;save new value
	cmp	bx,reg_ip - regs
	jne	rr1bX		;if not changing IP
rr1bX:
	ret
    
no386_5    
	dec	si		;back up one before flagging an error
errorj9:
	jmp	error

;	S command - search for a string of bytes.

sss:	
	mov	bx,[reg_ds]	;get search range
	xor	cx,cx
	call getrangeX	;get address range into BX:(E)DX..BX:(E)CX
	call skipcomm0
	push cx
	push dx
	call getstr		;get string of bytes
	pop	dx
	pop	cx
	sub	cx,dx		;cx = number of bytes in search range minus one
	sub	di,line_out	;di = number of bytes to look for
	dec	di			;     minus one
	sub	cx,di		;number of possible positions of string minus 1
	jb	errorj9		;if none
	call prephack	;set up for the interrupt vector hack
;	inc	cx			;cx = number of possible positions of string
	xchg dx,di		;set DI to offset
	call dohack		;do the interrupt pointer hack
sss1:				;<---- search next occurance
	mov es,bx		;set the segment
	mov	si,line_out	;si = address of search string
	lodsb			;first character in al
	repne scasb		;look for first byte
    je sss1_1
    scasb			;count in CX was cnt-1
	jne	sss3		;if we're done
sss1_1:    
	push cx
	push di
	mov	cx,dx
	repe cmpsb
	pop	di
	jne	sss2		;if not equal
	call unhack		;undo the interrupt vector hack and restore es
	push dx
	xchg si,di		;write address right after search string
    mov ax,bx
	call hexword	;4 (segment)
	mov	al,':'
	stosb			;+1=5
	lea	ax,[si-1]
	call hexword	;+4=9 (offset)
	mov	ax,LF * 256 + CR
	stosw			;+2=11
	mov	cx,11
	lea	dx,[di-11]
    push bx
    call stdout		;write to stdout
    pop bx
	pop	dx
	mov	di,si
	call dohack
sss2:
	pop	cx
	inc	cx
	loop sss1		;go back for more
sss3:
	jmp unhack

;	T command - Trace.

tt:
	mov	ah,al
    and	ah,TOUPPER
    cmp	ah,'M'
    jnz	isnotmodeset
	call skipcomma
    cmp al,CR
    jz ismodeget
    call getword
    cmp	dx,1
	jna tt_0
	jmp error    
tt_0:
	call chkeol		;expect end of line here
	mov	[tmode],dl
;	ret
ismodeget:
	mov	al,[tmode]
	add	al,'0'
    mov	[tmodev],al
	mov	di,line_out
    mov si,tmodes
    call showstring
    mov si,tmode0
    test byte [tmode],1
    jz ismg_1
    mov si,tmode1
ismg_1:
    call showstring
    call putsline
	ret
isnotmodeset:
	mov	word [lastcmd], isnotmodeset
	call parse_pt	;process arguments
    
;	Do it <CX=count> times.

tt1:	
	push cx
%if PM
	call getcsattr
%endif
	xor cx,cx
	call getcseipbyte
    cmp al,0CDh
    jnz isstdtrace
    inc cx
    call getcseipbyte
    cmp al,3
    jz  isstdtrace
    test byte [tmode], 1	;TM=1?
    jnz tt1_0
    jmp  isstdtraceX
tt1_0:    
    cmp al,1
    jz isstdtrace
    jmp emuint
isstdtrace:
%if PM    
	mov bx,[reg_ip]
    cmp bx,[dpmiwatch+0]	;catch the initial switch to protected mode
    jnz isnotdbgcode
    mov ax,[reg_cs]
    cmp ax,[dpmiwatch+2]
    jz isdpmientry
isnotdbgcode:    
%endif
	or byte [reg_fl+1],1h	;set single-step mode
    xor cx,cx
    call getcseipbyte
    cmp al,9Ch				;opcode "PUSHF"?
    jnz isnotpushf
    call run
	call cleartfonstack
    jmp tt1_2
isnotpushf:    
	call	run
tt1_2:    
	cmp	word [run_int],int1msg
	je	tt1_1
	jmp	tt2		;if some other interrupt
tt1_1:    
	call dumpregs
tt1_3:    
	pop	cx
	loop tt1
	ret

cleartfonstack:
	push es
    mov es,[reg_ss]
%if PM
    call ispm
    jnz ctos_0
	cmp byte [machine],3
    jb ctos_0
    cpu 386
	mov bx,es
    call getattrhigh
    jz ctos_0
    mov bx,[regh_esp]
    shl ebx,16
    mov bx,[reg_sp]
    and byte [es:ebx+1],~1
    jmp ctos_1
    cpu 8086
ctos_0:    
%endif
    mov bx,[reg_sp]
    and byte [es:bx+1],~1
ctos_1:
    pop es
    ret

%if PM
isdpmientry:    
    cmp byte [bNoHook2F],0
    jz isstdhook
    mov word [reg_ip],mydpmientry	;if int 2fh is *not* hooked
    mov [reg_cs],cs
isstdhook:    
    push ss
    pop es
    call ppout	;run code until RETF
    jmp tt1_3
%endif

;--- test if memory at CS:E/IP can be written to
;--- return C if not

iswriteablecseip:    
    call getcseipbyte	;get byte at CS:EIP+CX
    mov ah,al
    xor al,0FFh
    call setcseipbyte
    jc notwriteable
    call getcseipbyte
    cmp ah,al     		;is it ROM?
    jz notwriteable
    mov al,ah
    call setcseipbyte
    clc
    ret
notwriteable:    
	stc
    ret

; an INT is to be processed (TM is 0)
; to avoid the nasty x86 bug which makes IRET
; cause a debug exception 1 instruction too late
; a breakpoint is set behind the INT

isstdtraceX:
	mov cx,2
    call iswriteablecseip	;is it ROM?
    jnc ist_0
    jmp isstdtrace			;is r/o
ist_0:    					;is writeable
    mov bx,[reg_cs]
	mov si,[reg_ip]
    
;--- now get (E)SI=(E)IP+2 
    
%if PM    
    cmp byte [machine],3
    jb isstdtrace_2
    test byte [bCSAttr],40h
    jz isstdtrace_2
    cpu 386
    push word [regh_eip]
    push si
    pop esi
    add esi,2
    jmp isstdtrace_3
    cpu 8086
isstdtrace_2:
%endif
	add si,2
isstdtrace_3:
	call pp11x				;set BP at BX:(E)SI and run debuggee
    jmp tt1_3

;--- TM==1, single-step into the INT

emuint:
    mov bl,al
%if PM
	call ispm
    jz   emuint_pm
%endif
    mov bh,0
    push ds
    xor ax,ax
    mov ds,ax
    shl bx,1		;stay 8086 compatible in real-mode!
    shl bx,1
    lds si,[bx+0]
    mov al,[si]
    mov ah,al
    xor al,0FFh
    mov [si],al
    cmp ah,[si]
    jz isrom
    mov [si],ah
    mov bx,ds
    pop ds
    call pp11x
    jmp tt1_3
isrom:
%if 0
	pop ds
    jmp isstdtrace
%else    
	mov  ax,ds
    pop  ds
	xchg si,[reg_ip]
	xchg ax,[reg_cs]
    mov  cx,[reg_fl]
    push ds
    mov  bx,[reg_sp]
    mov  ds,[reg_ss]        ;emulate an INT
    sub  bx,6
    inc  si                 ;skip INT xx
    inc  si
    mov  [bx+0],si
    mov  [bx+2],ax
	mov  [bx+4],cx
    pop  ds
    mov  [reg_sp],bx
    and  byte [reg_fl],0FCh
    jmp  tt1_1
%endif    
%if PM
emuint_pm:
    mov  ax,204h
    int  31h
    mov  bx,cx
    test bl,4			;is it a LDT selector?
    jnz  emuint_1
    jmp  isstdtrace
emuint_1:    
	mov  si,dx
    cmp  byte [dpmi32],0
    jz   emuint_pm16
    cpu 386
    mov  esi,edx
    cpu 8086
emuint_pm16:
	call pp11x
    jmp  tt1_3
    
%endif

;	Print message about unexpected interrupt, dump registers, and end
;	command.  This code is also used by the G and P commands.

tt2:	
	mov	dx,[run_int]
	call int21ah9	;print string
	cmp	dx,progtrm
	je	tt3			;if it terminated, skip the registers
	call dumpregs
tt3:	
	jmp	cmd3		;back to the start

%if PM
tt2x:				;<--- entry if exception occured inside debugger
	call unhack
	mov	dx,[run_int]
	call int21ah9	;print string
%if EXCCSIP    
	mov	dx,excloc
    call int21ah9
%endif    
    jmp cmd3
%endif    

;	U command - disassemble.

uu:
	mov word [lastcmd],lastuu	
	cmp	al,CR
	jne	uu1		;if an address was given
lastuu:    
%if PM    
    call getcsattr
    jz   uu_0
    cpu 386
    mov  ecx,[u_addr]
    add  ecx,1Fh
    jnc  uu0x
    mov  ecx,-1
uu0x:
	inc  ecx
uu3_32:    
	push ecx
	push edx
	mov	byte [disflags],0
	call disasm
	pop	ebx
	pop	ecx
	mov	eax,[u_addr]
	mov	edx,eax
	sub	eax,ecx		;current position - end
	sub	ebx,ecx		;previous position - end
	cmp	eax,ebx
	jnb	uu3_32		;if we haven't reached the goal
    ret
    cpu 8086
uu_0:
%endif    
	mov	cx,[u_addr]
	add	cx,1fh
	jnc	uu2		;if no overflow
	mov	cx,-1
	jmp	uu2

uu1:
	mov	cx,20h		;default length
	mov	bx,[reg_cs]
	call getrangeX	;get address range into bx:(e)dx
	call chkeol		;expect end of line here
	mov	[u_addr+4],bx
	mov	[u_addr],dx
%if PM
	cmp [machine],byte 3
    jb uu1_1
    cpu 386
    mov dword [u_addr],edx
    cpu 8086
uu1_1:    
%endif

;	At this point, cx holds the last address, and dx the address.

uu2:
	inc cx
uu3:
	push cx
	push dx
	mov	byte [disflags],0
	call disasm		;do it
	pop	bx
	pop	cx
	mov	ax,[u_addr]
	mov	dx,ax
	sub	ax,cx		;current position - end
	sub	bx,cx		;previous position - end
	cmp	ax,bx
	jnb	uu3		;if we haven't reached the goal
uu4:
	ret


lockdrive:
    push ax
    push bx
    push cx
    push dx
    mov bl,al
    inc bl
    mov bh,0
    mov cx,084Ah
    mov dx,0001h
    mov ax,440Dh
    int 21h
    pop dx
    pop cx
    pop bx
    pop ax
    ret
unlockdrive:
    push ax
    push bx
    push cx
    push dx
    mov bl,al
    inc bl
    mov bh,0
    mov cx,086Ah
    mov dx,0001h
    mov ax,440Dh
    int 21h
    pop dx
    pop cx
    pop bx
    pop ax
    ret


;	W command - write a program, or disk sectors, to disk.

ww:	
	call parselw	;parse L and W argument format
	jz	ww4			;if request to write program
%if NOEXTENDER
	call ispm
    jnz ww0_rm
    call isextenderavailable
    jnc ww0_rm
	mov dx,nodosext
    jmp int21ah9
ww0_rm:    
%endif
	cmp [cs:usepacket],byte 2
    jb  ww0_1
    mov dl,al		;A=0,B=1,C=2,...
    mov si,6001h	;write, assume "file data"
%if VDD
	mov ax,[hVdd]
    cmp ax,-1
    jnz callvddwrite
%endif
    inc dl			;A=1,B=2,C=3,...
    call lockdrive
    mov ax,7305h	;DS:(E)BX->packet
    stc
    int 21h			;use int 21h here, not doscall
    pushf
    call unlockdrive
    popf
    jmp ww0_2
%if VDD
callvddwrite:
	mov cx,5
    add cl,[dpmi32]
    DispatchCall
    jmp ww0_2
%endif
ww0_1:
	int	26h
ww0_2:
	mov	dx,writing
ww1:
	mov	bx,ss		;restore segment registers
	mov	ds,bx
	mov	sp,[savesp]
	mov	es,bx
	jnc	ww3		;if no error
	cmp	al,0ch
	jbe	ww2		;if in range
	mov	al,0ch
ww2:
	cbw
	shl	ax,1
	xchg si,ax
	mov	si,[si+dskerrs]
	mov	di,line_out
	call showstring
	mov	si,dx
	call showstring
	mov	si,drive
	call showstring
	call putsline
ww3:
	jmp	cmd3		;can't ret because stack is wrong

;	Write to file.  First check the file extension.
;   size of file is in client's BX:CX, 
;   default start address is DS:100h

ww4:
	mov	al,[fileext]	;get flags of file extension
	test al,EXT_EXE + EXT_HEX
	jz	ww5		;if not EXE or HEX
	mov	dx,nowhexe
	jmp	ww6

ww5:
	cmp	al,0
	jnz	ww7		;if extension exists
	mov	dx,nownull
ww6:
	jmp	ww16

;	File extension is OK; write it.  First, create the file.

ww7:
%if PM
	call ispm
    jnz ww7_1
    mov dx,nopmsupp
    call int21ah9
    ret
ww7_1:    
%endif

	mov	bp,line_out
	cmp	dh,0feh
	jb	ww8			;if dx < fe00h
	sub	dh,0feh		;dx -= 0xfe00
	add	bx,0fe0h
ww8:
	mov	[bp+10],dx	;save lower part of address in line_out+10
	mov	si,bx		;upper part goes into si
	mov	ah,3ch		;create file
	xor	cx,cx		;no attributes
	mov	dx,DTA
	call doscall
	jc	ww15		;if error
	push ax			;save file handle

;	Print message about writing.

	mov	dx,wwmsg1
	call int21ah9	;print string
	mov	ax,[reg_bx]
	cmp	ax,10h
	jb	ww9			;if not too large
	xor	ax,ax		;too large:  zero it out
ww9:
	mov	[bp+8],ax
	or	ax,ax
	jz	ww10
	add	al,90h		;convert to hex and print
	daa
	adc	al,40h
	daa
	stosb
ww10:
	mov	ax,[reg_cx]
	mov	[bp+6],ax
	call hexword
	call puts		;print size
	mov	dx,wwmsg2
	call int21ah9	;print string

;	Now write the file.  Size remaining is in line_out+6.

	pop	bx			;recover file handle
	mov	dx,[bp+10]	;address to write from is si:dx
ww11:
	mov	ax,0fe00h
	sub	ax,dx
	cmp	byte [bp+8],0
	jnz	ww12		;if more than 0fe00h bytes remaining
	cmp	ax,[bp+6]
	jb	ww12		;ditto
	mov	ax,[bp+6]
ww12:	
	xchg ax,cx		;mov cx,ax
	mov	ds,si
	mov	ah,40h		;write to file
	int 21h			;use INT, not doscall
	push ss			;restore DS
	pop	ds
	cmp	ax,cx
	jne	ww13		;if disk full
	xor	dx,dx		;next time write from xxxx:0
	add	si,0fe0h	;update segment pointer
	sub	[bp+6],cx
	lahf
	sbb	byte [bp+8],0
	jnz	ww11		;if more to go
	sahf
	jnz	ww11		;ditto
	jmp	ww14		;done

ww13:
	mov	dx,diskful
	call int21ah9	;print string
	mov	ah,41h		;unlink file
	mov	dx,DTA
    call doscall

;	Close the file.

ww14:
	mov	ah,3eh		;close file
	int	21h
	ret

;	Error opening file.  This is also called by the load command.

ww15:
	cmp	ax,2
	mov	dx,doserr2	;File not found
	je	ww16
	cmp	ax,3
	mov	dx,doserr3	;Path not found
	je	ww16
	cmp	ax,5
	mov	dx,doserr5	;Access denied
	je	ww16
	cmp	ax,8
	mov	dx,doserr8	;Insufficient memory
	je	ww16
	mov	di,wwerr1
	call hexword
	mov	dx,wwerr	;Error ____ opening file
ww16:
int21ah9:
	mov ah,9
	call doscall
    ret


;	X commands - manipulate EMS memory.
;
;	Reference:
;	  http://www.nondot.org/sabre/os/files/MemManagement/LIMEMS41.txt

xx:	cmp	al,'?'
	je	xhelp		;if a call for help
	or	al,TOLOWER
	cmp	al,'a'
	je	xa		;if XA command
	cmp	al,'d'
	je	xd		;if XD command
	cmp	al,'m'
	je	xm		;if XM command
	cmp	al,'r'
	je	xr		;if XR command
	cmp	al,'s'
	jne xx_1
	jmp xs		;if XS command
xx_1:
	jmp	error

xhelp:	mov	dx,xhelpmsg
	jmp	int21ah9	;print string and return

;	XA - Allocate EMS.

xa:	call emschk
	call skipcomma
	call getword		;get argument into DX
	call chkeol			;expect end of line here
	mov	bx,dx
	mov	ah,43h			;allocate handle
    and bx,bx
    jnz nonullcnt
    mov ax,5A00h		;use the EMS 4.0 version to alloc 0 pages
nonullcnt:    
	call emscall
	xchg ax,dx			;mov ax,dx
	mov	di,xaans1
	call hexword
	mov	dx,xaans
	jmp	int21ah9	;print string and return

;	XD - Deallocate EMS handle.

xd:	call emschk
	call skipcomma
	call getword	;get argument into DX
	call chkeol		;expect end of line here
	mov	ah,45h		;deallocate handle
	call emscall
	xchg ax,dx		;mov ax,dx
	mov	di,xdans1
	call hexword
	mov	dx,xdans
	jmp	int21ah9	;print string and return

;	XR - Reallocate EMS handle.

xr:	call emschk
	call skipcomma
	call getword		;get handle argument into DX
	mov	bx,dx
	call skipcomma
	call getword		;get count argument into DX
	call chkeol			;expect end of line here
    xchg bx,dx
	mov	ah,51h			;reallocate handle
	call	emscall
	mov	dx,xrans
	jmp	int21ah9		;print string and return

;	XM - Map EMS memory to physical page.

xm:	call emschk
	call skipcomma
	call getword	;get logical page
	mov	bx,dx		;save it in BX
	call skipcomm0
	call getbyte	;get physical page (DL)
	push dx
	call skipcomm0
	call getword	;get handle into DX
	call chkeol		;expect end of line
	pop	ax			;recover physical page into AL
	push ax
	mov	ah,44h		;function 5 - map memory
	call emscall
	mov	di,xmans1
	xchg ax,bx		;mov al,bl
	call hexbyte
	add	di,xmans2-xmans1-2
	pop	ax
	call hexbyte
	mov	dx,xmans
	jmp	int21ah9	;print string and return

;	XS - Print EMS status.

xs:
	call emschk
	lodsb
	call chkeol		;no arguments allowed

;	First print out the handles and handle sizes.  This can be done either
;	by trying all possible handles or getting a handle table.
;	The latter is preferable, if it fits in memory.

	mov	ah,4bh		;function 12 - get handle count
	call emscall
	cmp	bx,(real_end-line_out)/4
	jbe	xs3		;if we can do it by getting the table
	xor	dx,dx		;handle

xs1:	
	mov	ah,4ch		;function 13 - get handle pages
	int	67h
	cmp	ah,83h
	je	xs2		;if no such handle
	or	ah,ah
    jz  xs1_1
	jmp	ce1	;if other error
xs1_1:    
	xchg ax,bx		;mov ax,bx
	call hndlshow
xs2:
	inc	dl		;end of loop
	jnz	xs1		;if more to be done
	jmp	xs5		;done with this part

;	Get the information in tabular form.

xs3:
	mov	ah,4dh		;function 14 - get all handle pages
	mov	di,line_out
	call	emscall
    and bx,bx
    jz xs5
	mov	si,di
xs4:
	lodsw
	xchg ax,dx
	lodsw
	call hndlshow
	dec	bx
	jnz	xs4		;if more to go

xs5:	
	mov	dx,crlf
	call int21ah9	;print string

;	Next print the mappable physical address array.
;	The size of the array shouldn't be a problem.

	mov	ax,5800h	;function 25 - get mappable phys. address array
	mov	di,line_out	;address to put array
	call emscall
	mov	dx,xsnopgs
    jcxz xs7        ;NO mappable pages!
    
	mov	si,di
xs6:
    push cx
	lodsw
	mov	di,xsstr2b
	call hexword
	lodsw
	mov	di,xsstr2a
	call hexbyte
	mov	dx,xsstr2
	call int21ah9	;print string
	pop	cx			;end of loop
    test cl,1
    jz xs_nonl
	mov	dx,crlf		;blank line
	call int21ah9	;print string
xs_nonl:
	loop xs6
	mov	dx,crlf		;blank line
xs7:
	call int21ah9	;print string

;	Finally, print the cumulative totals.

	mov	ah,42h		;function 3 - get unallocated page count
	call emscall
	mov	ax,dx		;total pages available
	sub	ax,bx		;number of pages allocated
	mov	bx,xsstrpg
	call sumshow	;print the line
	mov	ah,4bh		;function 12 - get handle count
	call emscall
	xchg ax,bx		;ax = number of handles allocated
	mov	dx,0ffh		;total number of handles
	mov	bx,xsstrhd
	call sumshow	;print the line
	ret				;done

;	Call EMS

emscall:
%if PM
	call ispm
    jnz ems_rm
    cpu 286
    push word [cs:pspdbg]
    push 67h
    call intcall
    jmp ems_call_done
    cpu 8086
ems_rm:    
%endif    
	int	67h
ems_call_done:    
	cmp	ah,0
	jz	echk1		;return if OK
ce1:
	mov	al,ah
	cmp	al,8bh
	jg	ce2		;if out of range
	cbw
	mov	bx,ax
	shl	bx,1
	mov	dx,[emserrs+100h+bx]
	or	dx,dx
	jnz	ce3		;if there's a word there
ce2:
	mov	di,emserrxa
	call hexbyte
	mov	dx,emserrx
ce3:	
	jmp	prnquit		;print string and quit

;	Check for EMS

emschk:
%if PM
	call ispm
    jnz emschk1
    mov bl,67h
    mov ax,0200h
    int 31h
    mov ax,cx
	or ax,dx
    jz echk2
    jmp emschk2
emschk1:
%endif
	push es
	mov	ax,3567h	;get interrupt vector 67h
	int 21h
    mov ax,es
    pop es
    or  ax,bx
    jz  echk2
emschk2:
    mov ah,46h		;get version
;    int 67h
    call emscall
    and ah,ah
    jnz echk2
echk1:
	ret
echk2:
	mov	dx,emsnot
	jmp	prnquit		;print string and quit

;	HNDLSHOW - Print XS line giving the handle and pages allocated.
;
;	Entry	DX	Handle
;		AX	Number of pages
;
;	Exit	Line printed
;
;	Uses	ax,cl,di.

hndlshow:
	mov	di,xsstr1b
	call hexword
	mov	ax,dx
	mov	di,xsstr1a
	call hexword
	push	dx
	mov	dx,xsstr1
	call	int21ah9	;print string
	pop	dx
	ret

;	SUMSHOW - Print summary line for XS command.
;
;	Entry	AX	Number of xxxx's that have been used
;		DX	Total number of xxxx's
;		BX	Name of xxxx
;
;	Exit	String printed
;
;	Uses	AX, CX, DX, DI

sumshow:
	mov	di,xsstr3
	push di
	call trimhex
	xchg ax,dx		;mov ax,dx
	mov	di,xsstr3a
	call trimhex
	pop	dx			;mov dx,xsstr3
	call int21ah9	;print string
	mov	dx,bx
	call int21ah9	;print string
	mov	dx,xsstr4
	jmp	int21ah9	;print string and return

;	TRIMHEX - Print word without leading zeroes.
;
;	Entry	AX	Number to print
;		DI	Where to print it
;
;	Uses	AX, CX, DI.

trimhex:
	call hexword
	sub	di,4		;back up DI to start of word
	mov	cx,3
	mov	al,'0'
tx1:
	scasb
	jne	tx2		;return if not a '0'
	mov	byte [di-1],' '
	loop tx1
tx2:
	ret

;	Error handlers.

error:
	mov	cx,si
	sub	cx,line_in+4
	add	cx,[promptlen]
	cmp	cx,127
	ja	err2		;if we're really messed up
	inc	cx			;number of spaces to skip
	mov	dl,' '
err1:	
	mov	ah,2		;display output
	int	21h
	loop err1
err2:
	mov	dx,errcarat
	call int21ah9	;print string
	jmp	[errret]

;	FREEMEM - cancel child process

freemem:
	mov	[reg_cs],cs
	mov	word [reg_ip],fmem2
%if PM    
	xor ax,ax
    mov [regh_eip],ax
    mov [regh_esp],ax
%endif    
	mov	[reg_ss],ss
	push ax
	mov	[reg_sp],sp	;save sp-2
	pop	ax
	call run
	ret
fmem2:
	mov	ax,4c00h	;quit
	int	21h

;--- this is called by "run"
;--- better don't use INTs inside
;--- set debuggee's INT 23/24

setint2324:
%if PM
	call ispm
    jz	setint2324pm
%endif    
    push es
    push di
    push si
    
    xor di,di
    mov es,ax
    mov di,23h*4
    mov si,run2324
    movsw
	movsw
    movsw
    movsw

%if PM
	call hook2f
%endif

    pop si
    pop di
    pop es
    ret
%if PM    
setint2324pm:
	push si
	mov si,run2324
    mov bx,0223h
si2324_0:
	mov dx,[si+0]
    cmp [dpmi32],byte 0
    jz si2324_1
    cpu 386
    mov edx,[si+0]
    cpu 8086
si2324_1:
	mov cx,[si+4]
    mov ax,205h
    int 31h
    add si,6
    inc bl
    dec bh
    jnz si2324_0
    pop si
    ret
%endif

;	This is the routine that starts up the running program.

run:	
	call seteq		;set CS:IP to '=' address
	mov	bx,[pspdbe]
    call setpsp		;set debuggee's PSP
	call setint2324	;set debuggee's int 23/24
    
	mov	[run_sp],sp	;save stack position
	sub	sp,[spadjust]
	mov	[SPSAV],sp
	cli
    cmp [machine],byte 3
    jb  no386
    cpu 386
    mov fs,[reg_fs]
    mov gs,[reg_gs]
    mov sp,regshi
	pop ebx			;pop hiwords eax+ebx
    shrd eax,ebx,16
	pop edx			;pop hiwords ecx+edx
    shrd ecx,edx,16
    pop ebp			;pop hiwords esp+ebp
	pop edi			;pop hiwords esi+edi
    shrd esi,edi,16
    cpu 8086
no386:
	mov	sp,regs
	pop	ax
	pop	bx
	pop	cx
	pop	dx
	pop	bp		;we'll get sp later
	pop	bp
	pop	si
	pop	di
	pop	es  	;temporary load DS value into ES (to make sure it is valid)
	pop	es		;now load the true value for ES
	pop	ss
    cmp byte [machine],3
    jnc doiretd
	mov	sp,[reg_sp]	;restore program stack
	mov [bInDbg],byte 0
	push word [reg_fl]
	push word [reg_cs]
	push word [reg_ip]
    test byte [reg_fl+1],2	;IF set?
	mov ds,[reg_ds]
    jz ifcleared16
    sti				;required for ring3 protected mode if IOPL==0
ifcleared16:
	iret			;jump to program
    cpu 386
doiretd:    
    mov sp,[regh_esp]
    shl esp,16
	mov	sp,[reg_sp]	;restore program stack
	mov [bInDbg],byte 0
	push word [regh_efl]
	push word [reg_fl]
    push 0
	push word [reg_cs]
	push word [regh_eip]
	push word [reg_ip]
    test byte [reg_fl+1],2	;IF set?
	mov ds,[reg_ds]
    jz ifcleared32
    sti				;required for ring3 protected mode if IOPL==0
ifcleared32:    
	iretd
    cpu 8086

;	Interrupt 22 (program termination) handler.

int22:
	cli
	mov	word [cs:run_int],progtrm	;remember interrupt type
	mov	word [cs:lastcmd],dmycmd
%if PM
	mov sp,[cs:InDosSegm]
    mov word [cs:pInDOS+2],sp
	mov sp,[cs:SDASegm]
    mov word [cs:pSDA+2],sp
%endif
    mov sp,cs
    mov ss,sp
	jmp	intrtn1		;jump to register saving routine (sort of)

;	Interrupt 0 (divide error) handler.

intr0:	
	mov	word [cs:run_int],int0msg	;remember interrupt type
	jmp	intrtn		;jump to register saving routine

    
;	Interrupt 1 (single-step interrupt) handler.

intr1:	mov	word [cs:run_int],int1msg	;remember interrupt type
	jmp	intrtn		;jump to register saving routine
    
;	Interrupt 3 (breakpoint interrupt) handler.

intr3:
	mov	word [cs:run_int],int3msg	;remember interrupt type
    
;	Common interrupt routine.

;	Housekeeping.

intrtn:
	cli						;just in case
	pop	word [cs:reg_ip]	;recover things from stack
	pop	word [cs:reg_cs]
	pop	word [cs:reg_fl]
	mov	[cs:reg_ss],ss		;save stack position
	mov	[cs:reg_sp],sp
	cmp [cs:machine],byte 3
    jb nohiesp
    cpu 386
    shr esp,16
    mov [cs:regh_esp],sp
    cpu 8086
nohiesp:    
	mov	sp,cs				;mov ss,cs
	mov	ss,sp
	mov	sp,reg_ss
intrtn2:			;<--- entry protected-mode
	push es
	push ds
	push di
	push si
	push bp
	dec	sp			;we already saved sp
	dec	sp
	push dx
	push cx
	push bx
	push ax

	cmp [cs:machine],byte 3
    jb no386_1
    cpu 386
	mov		sp,reg_gs+2
    push	gs
    push	fs

	mov		sp,regh_edi+2

;--- regh_edi+2 is dword aligned, so DWORD pushs are safe here

    pushfd
    pop ax		;skip LoWord(EFL)
    pop word [ss:regh_efl]
    
    push 0
    pushf
    popfd		;clear HiWord(EFL) inside debugger (resets AC flag)
    
    push	edi	;save the HiWords of all general purpose registers
    pop		ax
    push	esi
    pop		ax
    shld	esi,ebp,16
    push	si
	dec	sp			;we already saved esp
	dec	sp
    push	edx
    pop		ax
    push	ecx
    pop		ax
    push	ebx
    pop		ax
    shr		eax,16
    push	ax
    cpu 8086
no386_1:

intrtn1:

;	Clean up.

	mov	sp,[cs:run_sp]	;restore running stack
	cld					;clear direction flag
	sti					;interrupts back on
    
    push ss
    pop	ds

	call getpsp
    mov  [pspdbe],bx
    
	call getint2324		;save debuggee's int 23/24, set debugger's int 23/24
    
    push ds
    pop es
	mov	bx,cs
    call setpsp				;set PSP of debugger
	and	word [reg_fl],~100h	;clear single-step interrupt

    
	mov [bInDbg],byte 1
	cmp	word [cs:run_int],progtrm
    jnz isnotterm
    mov ah,4Dh
    int 21h
    mov di,progexit
    call hexword
isnotterm:
	ret

;--- this is low-level, called by "run"
;--- prefer not to use INT 21h inside
;--- get debuggee's interrupt vectors 23/24
;--- set debugger's interrupt vectors 23/24

getint2324:
%if PM
	call ispm
    jz	getint2324pm
%endif
	push si
    push di
    push es

    push ds
    pop es
    xor di,di
    mov ds,di
    mov di,run2324
    mov si,23h*4
    push si
    movsw       ;save interrupt vector 23h
    movsw
    movsw       ;save interrupt vector 24h
    movsw
    pop di
    push es
    pop ds
    xor si,si
    mov es,si
    mov si,CCIV
    movsw
    movsw
    movsw
    movsw
    
    pop es
    pop di
    pop si
    ret
%if PM    
getint2324pm:
    push di
    mov di,run2324
    call getcurrI2324
    pop di
;    call setdbgI2324
;    ret

	cpu 286    
setdbgI2324:
	pusha
	mov si,dbg2324
    mov bx,0223h
sdbg2324_0:
	lodsw
	mov dx,ax
	mov cx,cs
    mov ax,205h
    cmp [dpmi32],byte 0
    jz sdbg2324_1
    cpu 386
    push edx
    movzx edx,dx
    int 31h
    pop edx
    jmp sdbg2324_2
    cpu 286
sdbg2324_1:
    int 31h
sdbg2324_2:    
    inc bl
    dec bh
    jnz sdbg2324_0
    popa
    ret
	cpu 8086

getcurrI2324:
    mov bx,0223h
gi2324_0:	    
    mov ax,204h
    int 31h
	mov	[di+0],dx
	mov	[di+4],cx
    cmp [dpmi32],byte 0
    jz  gi2324_1
    cpu 386
    mov [di+0],edx
    cpu 8086
gi2324_1:
    add di,6
    inc bl
    dec bh
    jnz gi2324_0
	ret


%endif

;	The next three subroutines concern the handling of INT 23 and 24.
;	These interrupt vectors are saved and restored when running the
;	child process, but are not active when DEBUG itself is running.
;	It is still useful for the programmer to be able to check where INT 23
;	and 24 point, so these values are copied into the interrupt table
;	during parts of the c, d, e, m, and s commands, so that they appear
;	to be in effect.  The e command also copies these values back.
;	Between calls to dohack and unhack, there should be no calls to DOS,
;	so that there is no possibility of these vectors being used when
;	the child process is not running.

;	PREPHACK - Set up for interrupt vector substitution.
;	Entry	es = cs

prephack:
	cmp	byte [hakstat],0
	jnz	ph_err		;if hack status error
	push di
	mov	di,sav2324	;debugger's Int2324
    call prehak1
    pop di
    ret
ph_err:
	push ax
	push dx
	mov	dx,ph_msg
	call int21ah9	;print string |||
	pop	dx
	pop	ax
	ret

prehak1:
%if PM
	call ispm
    jz prehak_pm	;nothing to do
%endif    
    push ds
    push si
	xor	si,si
	mov	ds,si
	mov	si,4*23h
	movsw
	movsw
	movsw
	movsw
    pop si
    pop ds
prehak_pm:
	ret

ph_msg	db	'Error in sequence of calls to hack.',CR,LF,'$'

;	DOHACK - Fake the interrupt vectors 23 and 24.
;	UNHACK - Restore interrupt vectors 23 and 24 to their original values.
;		It's OK to do either of these twice in a row.
;		In particular, the `s' command may do unhack twice in a row.
;	Entry	ds = cs
;	Exit	es = cs

dohack:
	push si
	mov	byte [hakstat],1
	mov	si,run2324	;debuggee's interrupt vectors
%if PM
	call ispm
    jz dohack_pm
%endif    
	jmp	hak1

%if PM
restdbgi2324:
    push ss
    pop es
	call setdbgI2324	
    ret
%endif

unhack:
	mov	byte [hakstat],0
%if PM
	call ispm
    jz restdbgi2324
%endif
	push si
	mov	si,sav2324	;debugger's interrupt vectors
hak1:
    push di
	xor	di,di
	mov	es,di
	mov	di,4*23h
	movsw
	movsw
	movsw
	movsw
    pop di
	pop si
    push cs
    pop es
hak_ex:
	ret
%if PM

;--- set debuggee's int 23/24 pmode

dohack_pm:
	cpu 286
	push ss
    pop es
	pusha
    mov bx,0223h
dhpm_0:
    mov dx,[si+0+0]
    mov cx,[si+0+4]
	mov ax,205h
    cmp [dpmi32],byte 0
    jz dhpm_1
    cpu 386
    push edx
    mov edx,[si+0+0]
    int 31h
    pop edx
    jmp dhpm_2
	cpu 286
dhpm_1:
    int 31h
dhpm_2:    
    add si,6
    inc bl
    dec bh
	jnz dhpm_0
    popa
    pop si
    ret
    cpu 8086
%endif

InDos:    
	push ds
	push si
    lds si,[pInDOS]
    cmp byte [si],0
    pop si
    pop ds
    ret


;	GETLINE - Print a prompt (address in DX, length in CX) and read a line
;	of input.
;	GETLINE0 - Same as above, but use the output line (so far), plus two
;	spaces and a colon, as a prompt.
;	GETLINE00 - Same as above, but use the output line (so far) as a prompt.
;	Entry	CX	Length of prompt (getline only)
;		DX	Address of prompt string (getline only)
;
;		DI	Address + 1 of last character in prompt (getline0 and
;			getline00 only)
;
;	Exit	AL	First nonwhite character in input line
;		SI	Address of the next character after that
;	Uses	AH,BX,CX,DX,DI

getline0:
	mov	ax,'  '		;add two spaces and a colon
	stosw
	mov	al,':'
	stosb
getline00:
	mov	dx,line_out
	mov	cx,di
	sub	cx,dx

getline:
	mov	[promptlen],cx	;save length of prompt
	call bufsetup
	pushf
    call stdout
	popf
	jc gl5		;if tty input
    mov word [lastcmd],dmycmd

;	This part reads the input line from a file (in the case of
;	`debug < file').  It is necessary to do this by hand because DOS
;	function 0ah does not handle EOF correctly otherwise.  This is
;	especially important for debug because it traps Control-C.

gl1:
	mov	cx,[bufend]
	sub	cx,si		;cx = number of valid characters
	jz gl3		;if none
gl2:
	lodsb
	cmp	al,CR
	je gl4		;if end of line
	cmp	al,LF
	je gl4		;if eol
	stosb
	loop gl2		;if there are more valid characters
gl3:
	call fillbuf
	jnc	gl1		;if we have more characters
	mov	al,LF
	cmp	di,line_in+LINE_IN_LEN
	jb	gl4
	dec	si
	dec	di
gl4:
	mov	[bufnext],si
	mov	[notatty],al
	mov	al,CR
	stosb
	mov	cx,di
	mov	dx,line_in + 2
	sub	cx,dx
    call stdout	;print out the received line
	jmp	gl6		;done

gl5:
	mov	dx,line_in
    call InDos
    jnz rawinput
	mov	ah,0ah		;buffered keyboard input
    call doscall
gl6:
	mov al,10
    call stdoutal
	mov	si,line_in + 2
	call skipwhite
	ret
stdoutal:
	push bx
    push cx
	push dx
    push ax
    mov cx,1
    mov dx,sp
    call stdout
    pop ax
    pop dx
    pop cx
    pop bx
    ret
rawinput:
	push di
    push ds
    pop es
    inc dx
    inc dx
    mov di,dx
rawnext:    
	mov ah,00h
    int 16h
    cmp al,0
    jz rawnext
    cmp al,0E0h
    jz rawnext
    cmp al,08h
    jz del_key
    cmp al,7Fh
    jz del_key
    stosb
    call stdoutal
    cmp al,0Dh
    jnz rawnext
    dec di
	sub di,dx
    mov ax,di
    mov di,dx
    mov byte [di-1],al
    dec dx
    dec dx
    pop di
    jmp gl6
del_key:
	cmp di,dx
    jz  rawnext
	dec di
    call fullbsout
    jmp rawnext
fullbsout:    
    mov al,8
    call stdoutal
    mov al,20h
    call stdoutal
    mov al,8
    jmp stdoutal
    
;	BUFSETUP - Set up buffer reading.  This just means discard an LF
;	if the last character read (as stored in `notatty') is CR.
;	Entry	DI	First available byte in input buffer
;	Exit	SI	Address of next character.
;	If the input is from a tty, then bufsetup returns with carry set.

bufsetup:
	cmp	byte [notatty],0
	jnz	bs1		;if not a tty
	stc
	ret

bs1:	mov	di,line_in+2
	mov	si,[bufnext]
	cmp	si,[bufend]
	jb	bs2		;if there's a character already
	call	fillbuf
	jc	bs4		;if eof
bs2:	cmp	byte [notatty],CR
	jne	bs3		;if nothing more to do
	cmp	byte [si],LF
	jne	bs3		;if not a line feed
	inc	si		;skip it
bs3:	clc
	ret

bs4:	jmp	qq		;quit:  we've hit an eof

;	FILLBUF - Fill input buffer.  Mostly this is an internal routine
;	for getline.
;	Entry	DI	First available byte in input buffer
;	Exit	SI	Next readable byte (i.e., equal to DI)
;		Carry flag is set if and only if there is an error (e.g., eof)
;	Uses	None.

fillbuf:
	push ax
	push bx
	push cx
	push dx
	mov	si,di		;we know this already
	mov	ah,3fh		;read from file
	xor	bx,bx
	mov	cx,line_in+LINE_IN_LEN
	mov	dx,di
	sub	cx,dx
	jz	fb1			;if no more room
	call doscall
	jc	fb1			;if error
	or	ax,ax
	jz	fb1			;if eof
	add	ax,dx
	clc
	jmp	fb2

fb1:	
	xchg ax,dx		;ax = last valid byte address + 1
	stc

fb2:
	mov	[bufend],ax
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

;	PARSECM - Parse command line for C and M commands.
;	Entry	AL			First nonwhite character of parameters
;			SI			Address of the character after that
;	Exit	DS:(E)SI	Address from first parameter
;			ES:(E)DI	Address from second parameter
;			(E)CX		Length of address range minus one

parsecm:
	call prephack
	mov	bx,[reg_ds]	;get source range
	xor	cx,cx
	call getrange	;get address range into bx:(e)dx bx:(e)cx
	push bx			;save segment first address
%if PM
	cmp byte [bAddr32],0
    jz  pc_0
    cpu 386
    sub ecx,edx
    push edx		;save offset first address
    push ecx
    jmp pc_01
    cpu 8086
pc_0:    
%endif
	sub	cx,dx		;number of bytes minus one
	push dx
	push cx
pc_01:    
	call skipcomm0
	mov	bx,[reg_ds]
%if PM
	cmp byte [bAddr32],0
    jz  pc_1
    cpu 386
	call getaddr			;get address into bx:(e)dx
    mov byte [bAddr32],1	;restore bAddr32
    pop ecx
    mov edi,ecx
    add edi,edx
    jc errorj7
    call chkeol
    mov edi,edx
    mov es,bx
    pop esi
    pop ds
    ret
    cpu 8086
pc_1:    
%endif
	call getaddr	;get destination address into bx:(e)dx
	pop	cx
	mov	di,cx
	add	di,dx
	jc	errorj7		;if it wrapped around
	call chkeol		;expect end of line
	mov	di,dx
	mov	es,bx
	pop	si
	pop	ds
	ret

errorj7:jmp	error

;	PARSELW - Parse command line for L and W commands.
;
;	Entry	AL	First nonwhite character of parameters
;		SI	Address of the character after that
;
;	Exit	If there is at most one argument (program load/write), then the
;		zero flag is set, and registers are set as follows:
;		bx:(e)dx	Transfer address
;
;		If there are more arguments (absolute disk read/write), then the
;		zero flag is clear, and registers are set as follows:
;
;		DOS versions prior to 3.31:
;		AL	Drive number
;		CX	Number of sectors to read
;		DX	Beginning logical sector number
;		DS:BX	Transfer address
;
;		Later DOS versions:
;		AL	Drive number
;		BX	Offset of packet
;		CX	0FFFFh

;usepacket:
; 0: packet is not used (int 25h/26h, cx!=FFFF)
; 1: packet is used (int 25h/26h, cx==FFFF)
; 2: packet is used (int 21h, ax=7305h, cx==FFFF)

packet	dw	0,0	;sector number
	dw	0		;number of sectors to read
	dw	0,0		;transfer address SSSS:OOOO
%if PM
	dw 0		;transfer address might be SSSS:OOOOOOOO!
%endif
usepacket db	0

parselw:
	mov	bx,[reg_cs]	;default segment
	mov	dx,100h		;default offset
	cmp	al,CR
	je	plw2		;if no arguments
	call getaddr	;get buffer address into bx:(e)dx
	call skipcomm0
	cmp	al,CR
	je	plw2		;if only one argument
	push bx			;save segment
	push dx			;save offset
	mov	bx,80h		;max number of sectors to read
	neg	dx
	jz	plw1		;if address is zero
	mov	cl,9
	shr	dx,cl		;max number of sectors which can be read
	mov	di,dx
plw1:	
	call getbyte	;get drive number (DL)
	call skipcomm0
	push dx
	add	dl,'A'
	mov	[driveno],dl
	call getdword	;get relative sector number
	call skipcomm0
	push bx			;save sector number high
	push dx			;save sector number low
	push si			;in case we find an error
	call getword	;get sector count
	dec	dx
	cmp	dx,di
	jae	errorj7		;if too many sectors
	inc	dx
	mov	cx,dx
	call chkeol		;expect end of line
	cmp	byte [usepacket],0
	jnz	plw3		;if new-style packet called for
	pop	si			;in case of error
	pop	dx			;get LoWord starting logical sector number 
	pop	bx			;get HiWord 
	or	bx,bx		;just a 16bit sector number possible
	jnz	errorj7		;if too big
	pop	ax			;drive number
	pop	bx			;transfer buffer ofs
	pop	ds			;transfer buffer seg
	or	cx,cx		;set nonzero flag
plw2:
	ret

;--- new style packet, [usepacket] != 0

plw3:	
	pop	bx			;discard si
	mov	bx,packet
	pop	word [bx+0]	;LoWord sector number
	pop	word [bx+2]	;HiWord sector number
	mov	[bx+4],cx	;number of sectors
	pop	ax			;drive number
	pop	word [bx+6]	;transfer address ofs
    pop dx
	xor	cx,cx
%if PM
	call ispm
    jnz plw3_1
	cmp byte [dpmi32],0
    jz plw3_1
    cpu 386
    mov [bx+10],dx	;save segment of transfer buffer
    movzx ebx,bx
    shr edx,16		;get HiWord(offset)
    cmp [bAddr32],byte 1
    jz plw3_1
    xor dx,dx
    cpu 8086
plw3_1:
%endif
	mov	[bx+8],dx	;transfer address seg
	dec	cx			;set nonzero flag and make cx = -1
	ret

;	PARSE_PT - Parse 'p' or 't' command.
;	Entry	AL	First character of command
;		SI	Address of next character
;	Exit	CX	Number of times to repeat
;	Uses	AH,BX,CX,DX.

parse_pt:
	call parseql	;process =addr
	call skipcomm0	;skip any white space
	mov	cx,1		;default count
	cmp	al,CR
	je	ppt1		;if no count given
	call getword
	call chkeol		;expect end of line here
	mov	cx,dx
    jcxz errorj10	;must be at least 1
ppt1:	
	call seteq		;make the = operand take effect
	ret

;	PARSEQL - Parse `=' operand for `g', 'p' and `t' commands.
;	Entry	AL	First character of command
;			SI	Address of next character
;	Exit	AL	First character beyond range
;			SI	Address of the character after that
;			eqflag	Nonzero if an `=' operand was present
;			eqladdr	Address, if one was given
;	Uses AH,BX,CX,DX.

parseql:
	mov	byte [eqflag],0	;mark `=' as absent
	cmp	al,'='
	jne	peq1		;if no `=' operand
	call skipwhite
	mov	bx,[reg_cs]	;default segment
	call getaddrX	;get the address into bx:(e)dx
	mov	[eqladdr+0],dx
%if PM
    mov word [eqladdr+2],0
	cmp byte [bAddr32],0
    jz nohighofs
    cpu 386
    mov [eqladdr],edx
    cpu 8086
nohighofs:    
%endif
	mov	[eqladdr+4],bx
	inc	byte [eqflag]
peq1:	
	ret

;	SETEQ - Copy the = arguments to their place, if appropriate.  (This
;	is not done immediately, because the 'g' command may have a syntax
;	error.)
;	Uses AX.

seteq:
	cmp	byte [eqflag],0
	jz	seq1		;if no `=' operand
	mov	ax,[eqladdr+0]
	mov	[reg_ip],ax
	mov	ax,[eqladdr+2]
	mov	[regh_eip],ax
	mov	ax,[eqladdr+4]
	mov	[reg_cs],ax
	mov	byte [eqflag],0	;clear the flag
seq1:
	ret

;--- get a valid offset for segment in BX

getofsforbx:
%if PM
	call	ispm
    jnz		gofbx_2
    cmp		byte [machine],3
    jb		gofbx_2
    cpu 386
    push	ax	;needed?
    call	getattrhigh
    pop		ax
    jz		gofbx_2
    mov		byte [bAddr32],1
    push	bx
    call	getdword
    push	bx
    push	dx
    pop		edx
    pop		bx
    ret
    cpu 8086
gofbx_2:    
%endif
	call getword
	ret

errorj10: jmp error

;--- a range is entered with the L/ength argument
;--- get a valid length for segment in BX
;--- L=0 means 64 kB (at least in 16bit mode)

getlenforbx:    
%if PM
	call ispm
    jnz	glfbx_1
    cmp byte [machine],3
    jb glfbx_1
    cpu 386
    push	ecx
    lar		ecx,ebx
    test	ecx,400000h	;is segment 32bit?
    pop		ecx
    jz		glfbx_1
    push	dx
    push	bx
    call 	getdword
    push	bx
    push	dx
    pop		ecx
    pop		bx
    pop		dx
    stc
    jecxz glfbx_2
    dec ecx
    add ecx, edx
    ret
    cpu 8086
glfbx_1:
%endif
	push dx
	call getword
	mov	cx,dx
	pop	dx
;   stc
;	jcxz glfbx_2	;0 means 64k
	dec	cx
	add	cx,dx		;C if it wraps around
glfbx_2:    
    ret

;	GETRANGE - Get address range from input line.
;    a range consists of either start and end address
;    or a start address, a 'L' and a length.
;	Entry	AL	First character of range
;		SI	Address of next character
;		BX	Default segment to use
;		CX	Default length to use (or 0 if not allowed)
;	Exit	AL	First character beyond range
;		SI	Address of the character after that
;		BX:(E)DX	First address in range
;		BX:(E)CX	Last address in range
;	Uses	AH

getrangeX:
	push cx
    call getaddrX
    jmp	getrange_1

getrange:
	push cx			;save the default length
	call getaddr	;get address into bx:(e)dx (sets bAddr32)
getrange_1:    
	push si
    call skipcomm0
    cmp al,' '
    ja gr2
	pop	si			;restore si and cx
	pop	cx
	jcxz errorj10	;if a range is mandatory
%if PM    
	cmp byte [bAddr32],0	;can be 1 only on a 80386+
    jz gr0
    cpu 386
	dec	ecx
	add	ecx,edx
	jnc	gr1			;if no wraparound
	or	ecx,-1		;go to end of segment
    jmp gr1
gr0:
%endif    
	dec	cx
	add	cx,dx
	jnc	gr1			;if no wraparound
	mov	cx,0ffffh	;go to end of segment
gr1:
	dec	si			;restore al
	lodsb
	ret
    
gr2:
	or	al,TOLOWER
	cmp	al,'l'
	je	gr3			;if a range is given
;	call skipwh0	;get next nonblank
%if PM
    cmp byte [machine],3
    jb gr2_1
    cpu 386
	push edx
	call getofsforbx
	mov	ecx,edx
	pop	edx
    cmp edx,ecx
    ja errorj2
    jmp gr4
    cpu 8086
gr2_1:
%endif
	push dx
	call getword
	mov	cx,dx
	pop	dx
	cmp	dx,cx
	ja	errorj2			;if empty range
	jmp	gr4

gr3:
	call skipcomma		;discard the 'l'
    call getlenforbx
    jc errorj2
gr4:
	add	sp,4			;discard saved cx, si
	ret


;	GETADDR - Get address from input line.
;	Entry	AL	First character of address
;		SI	Address of next character
;		BX	Default segment to use
;	Exit	AL	First character beyond address
;		SI	Address of the character after that
;		BX:(E)DX	Address found
;	Uses	AH,CX

getaddr:
	call getaddr_1
	call verifysegm	;make BX a writeable segment
    ret

%if PM==0
errorj2:jmp	error
%endif

; getaddrX differs from getaddr in that BX is not ensured
; to be writeable in pmode

getaddrX:
getaddr_1:
%if PM
	mov byte [bAddr32],0
	cmp		byte [si-1],'$' ;a real-mode segment?
    jnz 	ga1_1
	lodsb
	call	ispm
    jnz		ga1_1
    call	getword
    mov		bx,dx
    push	ax
    mov		ax,2
    int		31h
    jc		errorj2
    mov		bx,ax
    mov		dx,ax
    pop		ax
    jmp		ga1_2
errorj2:jmp	error
%endif
ga1_1:
	call	getofsforbx
ga1_2:
	push si
	call skipwh0
	cmp	al,':'
	je	ga2		;if this is a segment descriptor
	pop	si
	dec	si
	lodsb
    ret

ga2:
	pop	ax		;throw away saved si
	mov	bx,dx	;mov segment into BX
ga3:
	call skipwhite	;skip to next word
    call getofsforbx
	ret


;	GETSTR - Get string of bytes.  Put the answer in line_out.
;		Entry	AL	first character
;			SI	address of next character
;		Exit	[line_out] first byte of string
;			DI	address of last+1 byte of string
;		Uses	AX,CL,DL,SI

getstr:	
	mov	di,line_out
	cmp	al,CR
	je	errorj2		;we don't allow empty byte strings
gs1:
	cmp	al,"'"
	je	gs2		;if string
	cmp	al,'"'
	je	gs2		;ditto
	call getbyte;byte in DL
	mov	[di],dl	;store the byte
	inc	di
	jmp	gs6

gs2:
	mov	ah,al	;save quote character
gs3:
	lodsb
	cmp	al,ah
	je	gs5		;if possible end of string
	cmp	al,CR
	je	errorj2	;if end of line
gs4:
	stosb		;save character and continue
	jmp	gs3

gs5:
	lodsb
	cmp	al,ah
	je	gs4		;if doubled quote character
gs6:
	call skipcomm0	;go back for more
	cmp	al,CR
	jne	gs1		;if not done yet
	ret

issymbol:
	push ax
    push di
    push cx
    mov di,regnames
    mov cx,16
	mov ah,[si]
    and ax,TOUPPER_W
    cmp byte [si+1],'A'
    jnc maybenotasymbol
    repnz scasw
    jnz notasymbol
    xor bx,bx
	sub	di,regnames+2
getsymlow:    
	mov	dx,[regs+di]
	inc	si		;skip over second char
    clc
    pop cx
	pop	di
    pop ax
    ret
maybenotasymbol:
    cmp al,'E'
    jnz notasymbol
    mov al,[si+1]
    xchg al,ah
    and ax,TOUPPER_W
    cmp ax,"IP"
    jnz isnoteip
    mov di,14*2
    jmp iseip
isnoteip:
    mov cx,8
    repnz scasw
    jnz notasymbol
    sub di,regnames+2
iseip:    
    mov bx,[regshi+di]
    inc si
    jmp getsymlow
notasymbol:
	pop	cx
    pop di
    pop ax
    stc
    ret


;	GETDWORD - Get (hex) dword from input line.
;		Entry	AL	first character
;			SI	address of next character
;		Exit	BX:DX	dword
;			AL	first character not in the word
;			SI	address of the next character after that
;		Uses	AH,CL

getdword:
	call issymbol
    jc gd_0
    lodsb
    ret
gd_0:    
	call getnyb
	jc	errorj6		;if error
	cbw
	xchg ax,dx
	xor	bx,bx		;clear high order word
gd1: 
	lodsb
	call getnyb
	jc	gd3
	test bh,0f0h
	jnz	errorj6		;if too big
	mov	cx,4
gd2:
	shl	dx,1		;double shift left
	rcl	bx,1
	loop gd2
	or	dl,al
	jmp	gd1
gd3:	
	ret
errorj6: jmp error    

;	GETWORD - Get (hex) word from input line.
;		Entry	AL	first character
;			SI	address of next character
;		Exit	DX	word
;			AL	first character not in the word
;			SI	address of the next character after that
;		Uses	AH,CL

getword:
	push bx
	call getdword
    and bx,bx		;hiword clear?
    pop bx
	jz 	gw_0
	jmp	errorj2		;if error
gw_0:    
	ret

;	GETBYTE - Get (hex) byte from input line into DL.
;		Entry	AL	first character
;			SI	address of next character
;		Exit	DL	byte
;			AL	first character not in the word
;			SI	address of the next character after that
;		Uses	AH,CL

getbyte:
	call getword
    and dh,dh
	jz  gb_0
	jmp errorj2		;if error
gb_0:
	ret

;	GETNYB - Convert the hex character in AL into a nybble.  Return
;	carry set in case of error.

getnyb:	
	push ax
	sub	al,'0'
	cmp	al,9
	jbe	gn1		;if normal digit
	pop	ax
	push ax
	or	al,TOLOWER
	sub	al,'a'
	cmp	al,'f'-'a'
	ja	gn2		;if not a-f or A-F
	add	al,10
gn1:
	inc	sp		;normal return (first pop old AX)
	inc	sp
	clc
	ret
gn2:
	pop	ax		;error return
	stc
	ret

;	CHKEOL1 - Check for end of line.

chkeol:	
	call skipwh0
	cmp	al,CR
	jne	errorj8		;if not found
	ret

errorj8:jmp	error

;	SKIPCOMMA - Skip white space, then an optional comma, and more white
;		space.
;	SKIPCOMM0 - Same as above, but we already have the character in AL.

skipcomma:
	lodsb
skipcomm0:
	call skipwh0
	cmp	al,','
	jne	sc2		;if no comma
	push si
	call skipwhite
	cmp	al,CR
	jne	sc1		;if not end of line
	pop	si
	mov	al,','
	ret
sc1:
	add	sp,2	;pop si into nowhere
sc2:
	ret

;	SKIPALPHA - Skip alphabetic character, and then white space.

skipalpha:
	lodsb
	and	al,TOUPPER
	sub	al,'A'
	cmp	al,'Z'-'A'
	jbe	skipalpha
	dec	si
;	jmp	skipwhite	;(control falls through)

;	SKIPWHITE - Skip spaces and tabs.
;	SKIPWH0 - Same as above, but we already have the character in AL.

skipwhite:
	lodsb
skipwh0:
	cmp	al,' '
	je	skipwhite
	cmp	al,TAB
	je	skipwhite
	ret

;	IFSEP	Compare AL with separators ' ', '\t', ',', ';', '='.

ifsep:	cmp	al,' '
	je	ifs1
	cmp	al,TAB
	je	ifs1
	cmp	al,','
	je	ifs1
	cmp	al,';'
	je	ifs1
	cmp	al,'='
ifs1:	ret

;	Here is the start of the disassembly part of the program.

;	Jump table for OP_IMM, OP_RM, OP_M, OP_R_MOD, OP_MOFFS, OP_R, OP_R_ADD,
;	and OP_AX.

disjmp2	dw	dop01,dop04,dop29,dop29a,dop30,dop32,dop36,dop37

;   jump table for displaying operands
;   must be in the order of OP_M64 ..

optab
	dw	dop38,dop38a,dop38b,dop39,dop42,dop41
	dw	dop43,dop46,dop47,dop49,dop51,dop52
	dw	dop53,dop54,dop56,dop58,dop59,dop59e,dop60
    
	db	'1',0,'3',0,'DXCLSTCSDSESFSGSSS'	;simple strings

OP_STR	equ	OP_1		;first string entry

;	DISASM - Disassemble.

dis_n	dw	0		;number of bytes in instruction so far
		dw  0
preflags db	0		;flags for prefixes found so far
preused	db	0		;flags for prefixes used so far

PRESEG	equ	1		;segment prefix
PREREP	equ	2		;rep prefixes
PREREPZ	equ	4		;f3, not f2
PRELOCK	equ	8		;lock prefix
PRE32D	equ	10h		;flag for 32-bit data
PRE32A	equ	20h		;flag for 32-bit addressing
PREWAIT	equ	40h		;prefix wait (not really a prefix)
GOTREGM	equ	80h		;set if we have the reg/mem part

instru	db	0		;the main instruction byte
index	dw	0		;index of the instruction (unsqueezed)
		dw	SFPGROUP3, SFPGROUP3+1, SFPGROUP3+4
		dw	SPARSE_BASE+24h, SPARSE_BASE+26h ;obsolete-instruction values
rmsize	db	0		;<0 or 0 or >0 means mod r/m is 8 or 16 or 32
segmnt	db	0		;segment determined by prefix (or otherwise)
addrr	dw	0		;address in mod r/m byte
savesp2	dw	0		;save the stack pointer here (used in disasm)

disflags db	0		;flags for the disassembler

;--- equates for disflags:

DIS_F_REPT		equ	1	;repeat after pop ss, etc.
DIS_F_SHOW		equ	2	;show memory contents
DIS_I_SHOW		equ	4	;there are memory contents to show
DIS_I_UNUSED	equ	8	;(internal) print " (unused)"
DIS_I_SHOWSIZ	equ	10h	;(internal) always show the operand size
DIS_I_KNOWSIZ	equ	20h	;(internal) we know the operand size of instr.

disflags2 db	0		;another copy of DIS_I_KNOWSIZ

sizeloc	dw	0		;address of size words in output line

;	Jump table for a certain place.

disjmp	dw	disbad	;illegal instruction
	dw	da6			;two byte instruction
	dw	da7			;instruction group
	dw	da8			;coprocessor instruction
	dw	da9			;coprocessor instruction group
	dw	da10		;instruction prefix (including 66h/67h)

;	Table for 16-bit mod r/m addressing.  8 = BX, 4 = BP, 2 = SI, 1 = DI.

rmtab	db	8+2, 8+1, 4+2, 4+1, 2, 1, 4, 8

;	Tables of register names.
;	rgnam816/rgnam16/segrgnam/xregnam must be consecutive.

rgnam816	dw 'AL','CL','DL','BL','AH','CH','DH','BH'
rgnam16		dw 'AX','CX','DX','BX','SP','BP','SI','DI'
segrgnam	dw 'ES','CS','SS','DS','FS','GS'
xregnam		dw 'ST','MM','CR','DR','TR'
sizetcnam	dw 'BY','WO','WO','DW','QW','FL','DO','TB','SH','LO','NE','FA'
segrgaddr	dw reg_es,reg_cs,reg_ss,reg_ds

;	Tables for handling of named prefixes.

prefixlist	db	26h,2eh,36h,3eh,64h,65h	;segment prefixes (in order)
			db	9bh,0f0h,0f2h,0f3h	;WAIT,LOCK,REPNE,REPE
prefixmnem	dw	MNEM_WAIT,MNEM_LOCK,MNEM_REPNE,MNEM_REPE

disasm:
	mov	[savesp2],sp
    xor ax,ax
	mov	word [dis_n],ax
	mov	word [preflags],ax	;clear preflags and preused
%if PM	  
	mov [bCSAttr],al
	cmp byte [machine],3
	jb	disasm_1
	call ispm
	jnz disasm_1
	mov bx,[u_addr+4]
	call getattrhigh
	mov [bCSAttr],ah
	jz disasm_1
	or byte [preflags], PRE32D | PRE32A
;	or byte [preused], PRE32D | PRE32A
disasm_1:
%endif
	mov	byte [segmnt],3		;initially use DS segment
	mov	byte [rmsize],80h	;don't display any memory
	mov	word [dismach],0	;no special machine needed, so far
	call disgetbyte			;get a byte of the instruction
	cmp	al,9bh				;wait instruction (must be the first prefix)
	jne	da2					;if not

;	The wait instruction is actually a separate instruction as far as
;	the x86 is concerned, but we treat it as a prefix since there are
;	some mnemonics that incorporate it.  But it has to be treated specially
;	since you can't do, e.g., seg cs wait ... but must do wait seg cs ...
;	instead.  We'll catch it later if the wait instruction is not going to
;	be part of a shared mnemonic.

	or	byte [preflags],PREWAIT

;	If we've found a prefix, we return here for the actual instruction
;	(or another prefix).

da1:
	call disgetbyte
da2:
	mov	[instru],al	;save away the instruction
	mov	ah,0

;	Now we have the sequence number of the instruction in AX.  Look it up.

da3:
	mov	bx,ax
	mov	[index],ax	;save the compressed index
	cmp	ax,SPARSE_BASE
	jb	da4		;if it's not from the squeezed part of the table
	mov	bl,[sqztab+bx-SPARSE_BASE]
	mov	bh,0
	add	bx,SPARSE_BASE	;bx = compressed index

da4:
	mov	cl,[optypes+bx]	;cx = opcode type
	mov	ch,0
	shl	bx,1
	mov	bx,[opinfo+bx]	;bx = other info (usually the mnemonic)
	mov	si,cx
	mov	ax,bx
	mov	cl,12
	shr	ax,cl
	cmp	al,[dismach]
	jb	da5				;if a higher machine is already required
	mov	[dismach],al	;set machine type
da5:
	and	bx,0fffh		;remove the machine field
	cmp	si,OPTYPES_BASE
	jae	da13			;if this is an actual instruction
	call [disjmp+si]	;otherwise, do more specific processing
	jmp	da3				;back for more

;	Two-byte instruction.

da6:
	call disgetbyte
	mov	[instru],al
	mov	ah,0
	add	ax,SPARSE_BASE
	ret

;	Instruction group.

da7:
	call getregmem_r	;get the middle 3 bits of the R/M byte
	cbw
	add	ax,bx			;offset
	ret

;	Coprocessor instruction.

da8:
	or	byte [disflags],DIS_I_SHOWSIZ
	or	byte [dmflags],DM_COPR
	call getregmem
	cmp	al,0c0h
	jb	da7		;range 00-bfh is same as an instruction group
	mov	cl,3
	shr	al,cl		;C0h --> 18h
	sub	al,18h-8	;18h --> 8
	cbw
	add	ax,bx		;offset
	ret

;	Coprocessor instruction group.

da9:
	mov	al,[regmem]
	and	al,7
	cbw
	add	ax,bx
	ret

;	Instruction prefix.  At this point, bl = prefix bits; bh = segment

da10:	
%if 1
	mov al,bl
    and bl,~(PRE32D | PRE32A)	;these flags are XORed!
%endif
	test bl,[preflags]
	jnz	da12		;if there are duplicates
    or [preflags],bl
%if 1
    mov bl,al
    and al,PRE32D | PRE32A
    xor [preflags],al
%endif    
	test bl,PRESEG
	jz	da11		;if not a segment
	mov	[segmnt],bh	;save the segment
da11:
	pop	ax			;discard return address
	jmp	da1

da12:
	jmp	disbad		;we don't allow duplicate prefixes

;	OK.  Here we go.  This is an actual instruction.
;	First print the op mnemonic.

da13:
	push si
	lea	si,[mnlist+bx]	;offset of mnemonic
	cmp	si,mnlist+MNEM_BSWAP
	jne	da13a			;if not BSWAP
	call dischk32d
	jz	da12			;if no operand-size prefix
da13a:
	call showop			;print out the op code (at line_out+28)
	mov	word [sizeloc],0;clear out this flag
	pop	si				;recover list of operands
	add	si,oplists-OPTYPES_BASE
	cmp	byte [si],0
	je	da21			;if we're done

;	Loop over operands.  [si] = pointer to next operand type.
;	Fortunately the operands appear in the instruction in the same
;	order as they appear in the disassembly output.

da14:
	mov	byte [disflags2],0	;clear out size-related flags
	lodsb					;get the operand type
	cmp	al,OP_SIZE
	jb	da18				;if it's not size dependent
	mov	byte [disflags2],DIS_I_KNOWSIZ	;indicate variable size
	cmp	al,OP_8
	jae	da16		;if the size is fixed
	cmp	al,OP_1632
	jae	da15		;if word or dword
	mov	ah,-1
	test byte [instru],1
	jz	da17		;if byte
da15:
	or	byte [preused],PRE32D	;mark this flag as used
	mov	ah,[preflags]
	and	ah,PRE32D	;this will be >0 for dword, =0 for word
	jmp	da17		;done

da16:
	mov	ah,al		;OP_8 or OP_16 or OP_32 (we know which)
	and	ah,0f0h		;this converts ah to <0 for byte, =0 for word,
	sub	ah,OP_16	;and >0 for dword

;	Now we know the size (in ah); branch off to do the operand itself.

da17:
	mov	bl,al
	and	bx,0fh
	call [disjmp2+bx]	;print out the operand
	jmp	da20			;done with operand

;	Sizeless operands.

da18:
	cbw
	xchg ax,bx
	cmp	bl,OP_STR
	jb	da19		;if it's not a string
	mov	ax,[optab+bx-2]
	stosw
	cmp	ah,0
	jnz	da20		;if it's two characters
	dec	di
	jmp	da20		;done with operand

da19:	
	call [optab+bx-2]	;otherwise, do something else

da20:
	cmp	byte [si],0
	jz	da21		;if we're done
	mov	al,','
	stosb
	jmp	da14		;another operand

;--- operands done.
;--- now check if there's a 66/67 or WAIT prefix unused yet

da21:
	mov	al,[preused]
	not	al
	and	al,[preflags]
	jnz	da22		;if some flags remain unused
	jmp	da28		;if all flags were used
da22:
	mov	cx,N_WTAB
	mov	bx,wtab1
	mov	dx,2*N_WTAB-2
	mov	ah,PREWAIT
	test al,ah
	jnz	da23		;if there's a WAIT prefix hanging

	mov	cx,N_LTAB
	mov	bx,ltab1
	mov	dx,2*N_LTAB-2
	mov	ah,PRE32D
	test al,ah
	jnz da23		;if it's not a 32-bit prefix that's hanging

	mov	cx,N_LTABX
	mov	bx,ltab1X
	mov	dx,2*N_LTABX-2
	mov	ah,PRE32A
	test al,ah
	jnz da23		;if it's not a 32-bit prefix that's hanging

	jmp	da24

da23:
	or	[preused],ah	;mark this prefix as used
	push di
	mov	di,bx
    mov bl,ah
	mov	ax,[index]
	repne scasw
	jne	da23_1		;if not found in the list
	add	di,dx		;replace the mnemonic with the 32-bit name
	mov	si,[di]
	add	si,mnlist
	call showop		;copy op mnemonic
da23_0:    
	pop	di
	jmp	da21
da23_1:
%if PM
	test bl,PRE32A | PRE32D	;is a 66/67 prefix unhandled?
    jz disbad2
	test byte [bCSAttr],40h	;32bit code segment?
    jnz da23_0   			;then ignore those. 
%endif    
disbad2:
	jmp	disbad

da24:
	test al,PRESEG
	jz	da25		;if not because of a segment prefix
	mov	ax,[index]
	cmp	ah,0
	jnz	disbad2		;if index > 256
	push di
	mov	cx,P_LEN
	mov	di,prfxtab
	repne scasb
	pop	di
	jne	disbad2		;if it's not on the list
	mov	cx,3
	call moveover
	push di
	mov	di,line_out+MNEMONOFS
	call showseg
	mov	al,':'
	stosb
	pop	di
	or	byte [preused],PRESEG	;mark it as used
	jmp	da21

da25:
	test al,PREREP
	jz	da26		;if not a REP prefix
	and	al,PREREP+PREREPZ
	or	[preused],al
	mov	ax,[index]
	cmp	ah,0
	jnz	disbad2		;if not in the first 256 bytes
	and	al,0feh		;clear the low bit
	push di
	mov	di,replist
	mov	cx,5
	repne scasb
	mov	si,mnlist+MNEM_REP
	je	da27		;if one of the REP instructions
	inc	cx
	inc	cx
	repne scasb
	jne	disbad2		;if not one of the REPE/REPNE instructions
	mov	si,mnlist+MNEM_REPE
	test byte [preused],PREREPZ
	jnz	da27		;if REPE
	mov	si,mnlist+MNEM_REPNE
	jmp	da27		;it's REPNE

disbad3:
	jmp	disbad

da26:
	test al,PRELOCK
	jz	disbad3		;if not a lock prefix, either
	push di
	mov	ax,[index]
	mov	di,locktab
	mov	cx,N_LOCK
	repne scasw
	jne	disbad3		;if not in the approved list
	test byte [preused],PRESEG
	jz	disbad3		;if memory was not accessed
	mov	si,mnlist+MNEM_LOCK
	or	byte [preused],PRELOCK

;	Slip in another mnemonic.  SI = offset of mnemonic, what should be
;	DI is on the stack.

da27:
	pop	di
	mov	cx,8
	push si
	call moveover
	pop	si
	push di
	call showop
	pop	di
	jmp	da21

;	Done with instruction.  Erase the size indicator, if appropriate.

da28:
	mov	cx,[sizeloc]
	cmp	cx,0
	jz	da28b		;if there was no size given
	mov	al,[disflags]
	test al,DIS_I_SHOWSIZ
	jnz	da28b		;if we need to show the size
	test al,DIS_I_KNOWSIZ
	jz	da28b		;if the size is not known already
	xchg cx,di
	mov	si,di		;save old di
	mov	al,' '
da28a:
	scasb			;skip size name
	jne	da28a		;if not done yet
					;(The above is the same as repne scasb, but
					;has no effect on cx.)
	add	di,4		;skip 'PTR '
	xchg si,di
	sub	cx,si
	rep	movsb		;move the line

;	Now we're really done.  Print out the bytes on the left.

da28b:
	push di		;print start of disassembly line
	mov	di,line_out
	mov	ax,[u_addr+4]	;print address
	call hexword
	mov	al,':'
	stosb
%if PM    
    test byte [bCSAttr],40h
    jz isnot32bit
	mov	ax,[u_addr+2]
	call hexword
isnot32bit:    
%endif    
	mov	ax,[u_addr]
	call hexword
	mov	al,' '
	stosb
	mov	bx,[dis_n]
da28c:
    mov si,line_out+MNEMONOFS - 1
    sub si, di
    shr si, 1
	cmp	bx,si
	jle da29		;if it's a short instruction
    sub bx,si
    push bx
	mov	bx,si
    push di
	call disshowbytes
	call putsline
    pop cx
    pop bx
    mov di,line_out
    sub cx,di
    mov al,' '
    rep stosb
    jmp da28c
da29:
	call disshowbytes
da30:
	mov	al,' '		;pad to op code
	mov	cx,line_out+MNEMONOFS
	sub	cx,di
    jc  da30_1
	rep	stosb
da30_1:    
	pop	di
	test byte [disflags],DIS_I_UNUSED
	jz	da32		;if we don't print ` (unused)'
	mov	si,unused
	cmp	byte [di-1],' '
	jne	da31		;if there's already a space here
	inc	si
da31:
	call showstring

;	Print info. on minimal processor needed.

da32:
	push di
	mov	di,index+2
	call showmach	;show the machine type, if needed
	pop	di
	jcxz da32f		;if no message

;	Print a message on the far right.

	mov	ax,line_out+79
	sub	ax,cx
	push cx
	call tab_to		;tab out to the location
	pop	cx
	rep	movsb		;copy the string
	jmp	da32z		;done

;	Dump referenced memory location.

da32f:
	mov	al,[disflags]
	xor	al,DIS_F_SHOW + DIS_I_SHOW
	test al,DIS_F_SHOW + DIS_I_SHOW
	jnz	da32z		;if there is no memory location to show
	cmp	byte [segmnt],3
	ja	da32z		;if FS or GS
	mov	ax,line_out+79-10
	cmp	byte [rmsize],0
	jl	da32h		;if byte
	jz	da32g		;if word
	sub	ax,4
da32g:
	dec	ax
	dec	ax
da32h:
	call tab_to
	call showseg		;show segment name
	mov	al,':'
	stosb
	mov	ax,[addrr]
	call hexword		;show offset
	mov	al,'='
	stosb
	mov	al,[segmnt]	;segment number
	cbw
	shl	ax,1
	xchg ax,bx		;mov bx,ax
	mov	bx,[segrgaddr+bx] ;get address of value
    push es
	mov	es,[bx]
	mov	bx,[addrr]
    mov al,[es:bx+0]	;avoid a "mov ax,[-1]"
    mov ah,[es:bx+1]
	mov	dl,[es:bx+2]	;avoid a "mov dx,[-1]"
	mov	dh,[es:bx+3]
	pop	es
	cmp	byte [rmsize],0
	jl	da32j		;if byte
	jz	da32i		;if word
	xchg ax,dx
	call hexword
	xchg ax,dx
da32i:
	call hexword
	jmp	da32z		;done
da32j:
	call hexbyte	;display byte

da32z:
	call trimputs	;done with operand list
	mov	al,[disflags]
	test al,DIS_F_REPT
	jz	da34		;if we're not allowed to repeat ourselves
	test al,DIS_I_UNUSED
	jnz	da33		;if we printed ` (unused)'
	mov	ax,[index]
	cmp	ax,17h
	je	da33		;if it was `pop ss'
	cmp	ax,8eh
	je	da33		;if it was `mov ss,--'
	cmp	ax,0fbh
	jne	da34		;if it was not `sti'
da33:
	mov	byte [disflags],0
	jmp	disasm
da34:
	ret

;	Here are the routines for printing out the operands themselves.
;	Immediate data (OP_IMM)

dop01:
	cmp	ah,0
	jl	dop03		;if just a byte
	pushf
	test byte [disflags],DIS_I_SHOWSIZ
	jz	dop01a		;if we don't need to show the size
	call showsize
	sub	di,4		;erase "PTR "
dop01a:
	call disgetword
	popf
	jz	dop02		;if just a word
	push ax
	call disgetword	;print the high order word
	call hexword
	pop	ax
dop02:
	call hexword
	ret

dop03:
	call disgetbyte	;print immediate byte
	call hexbyte
	ret

;	MOD R/M (OP_RM)

dop04:
	call getregmem
	cmp	al,0c0h
	jb	dop05
	jmp	dop33		;if pure register reference
dop05:
	call showsize	;print out size
dop06:
	or	byte [preused],PRESEG;needed even if there's no segment override
							;because handling of LOCK prefix relies on it
	test byte [preflags],PRESEG
	jz	dop07			;if no segment override
	call showseg		;print segment name
	mov	al,':'
	stosb
dop07:
	mov	al,[regmem]
	and	al,0c7h
	or	byte [preused],PRE32A
	test byte [preflags],PRE32A
	jz	dop08
	jmp	dop18		;if 32-bit addressing
dop08:
	or	byte [disflags],DIS_I_SHOW	;we'd like to show this address
	mov	word [addrr],0	;zero out the address initially
	cmp	al,6
	xchg ax,bx		;mov bx,ax
	mov	al,'['
	stosb
	je	dop16		;if [xxxx]
	and	bx,7
	mov	bl,[rmtab+bx]
	test bl,8
	jnz	dop09		;if BX
	test bl,4
	jz	dop11		;if not BP
	mov	ax,'BP'
	mov	cx,[reg_bp]
	test byte [preflags],PRESEG
	jnz	dop10		;if segment override
	dec	byte [segmnt]	;default is now SS
	jmp	dop10

dop09:	
	mov	ax,'BX'		;BX
	mov	cx,[reg_bx]

dop10:	
	mov	[addrr],cx	;print it out, etc.
	stosw
	test bl,2+1
	jz	dop13		;if done
	mov	al,'+'
	stosb
dop11:	
	mov	ax,'SI'
	mov	cx,[reg_si]
	test bl,1
	jz	dop12		;if SI
	mov	al,'D'		;DI
	mov	cx,[reg_di]

dop12:
	add	[addrr],cx	;print it out, etc.
	stosw
dop13:
	test byte [regmem],0c0h
	jz	dop17		;if no displacement
	test byte [regmem],80h
	jnz	dop15		;if word displacement
	call disgetbyte
	cbw
	add	[addrr],ax
	cmp	al,0
	mov	ah,'+'
	jge	dop14		;if >= 0
	mov	ah,'-'
	neg	al
dop14:
	mov	[di],ah
	inc	di
	call hexbyte	;print the byte displacement
	jmp	dop17		;done

dop15:
	mov	al,'+'
	stosb
dop16:
	call disgetword
	add	[addrr],ax
	call hexword    ;print word displacement

dop17:
	mov	al,']'
	stosb
	ret

;	32-bit MOD REG R/M addressing.

dop18:
	cmp	al,5
	jne	dop19		;if not just a disp32 address
	mov	al,'['
	stosb
	call disp32		;display 32bit offset
	jmp	dop27

dop19:
	push ax
	and	al,7
	cmp	al,4
	jne	dop20		;if no SIB
	call disgetbyte	;get and save it
	mov	[sibbyte],al
dop20:
	pop	ax
	test al,80h
	jnz	dop22		;if disp32
	test al,40h
	jz	dop23		;if no disp8
	call disgetbyte
	cmp	al,0
	jge	dop21		;if >= 0
	neg	al
	mov	byte [di],'-'
	inc	di
dop21:
	call hexbyte
	jmp	dop23		;done

dop22:
	call disp32		;print disp32

dop23:
	mov	al,[regmem]
	and	al,7
	cmp	al,4
	jne	dop28		;if no SIB
	mov	al,[sibbyte]
%if 1               ;bugfix: make 'u' correctly handle [ESP],[ESP+x]
    cmp al,24h
    jnz noesp
    mov al,4
    jmp dop28
noesp:    
%endif
	and	al,7
	cmp	al,5
	jne	dop24		;if not [EBP]
	test byte [regmem],0c0h
	jnz	dop24		;if MOD != 0
	call disp32		;show 32-bit displacement instead of [EBP]
	jmp	dop25

dop24:
	mov	word [di],'[E'
	inc	di
	inc	di
	call showreg16	;show 16bit register name (number in AL)
	mov	al,']'
	stosb

dop25:
	mov	al,[sibbyte]
	shr	al,1
	shr	al,1
	shr	al,1
	and	al,7
	cmp	al,4
	je	disbad1		;if illegal
	mov	word [di],'[E'
	inc	di
	inc	di
	call showreg16
	mov	ah,[sibbyte]
	test ah,0c0h
	jz	dop27		;if SS = 0
	mov	al,'*'
	stosb
	mov	al,'2'
	test ah,80h
	jz	dop26		;if *2
	mov	al,'4'
	test ah,40h
	jz	dop26		;if *4
	mov	al,'8'
dop26:
	stosb
dop27:
	mov	al,']'
	stosb
	ret

;	32-bit addressing without SIB

dop28:
	mov	word [di],'[E'
	inc	di
	inc	di
	call showreg16
	mov	al,']'
	stosb
	ret

;	Memory-only reference (OP_M)

dop29:
	call getregmem
	cmp	al,0c0h
	jae	disbad1		;if it's a register reference
	jmp	dop05

disbad1:jmp	disbad		;this is not supposed to happen

;	Register reference from MOD R/M part (OP_R_MOD)

dop29a:
	call getregmem
	cmp	al,0c0h
	jb	disbad1		;if it's a memory reference
	jmp	dop33

;	Memory offset reference (OP_MOFFS)

dop30:
	call showsize	;print the size and save various things
	mov	al,5
	test byte [preflags],PRE32A
	jnz	dop31		;if 32-bit addressing
	inc	ax
dop31:
	mov	[regmem],al
	jmp	dop06

;	Pure register reference (OP_R)

dop32:
	call getregmem_r

dop33:
	and	al,7			;entry point for regs from MOD R/M, and others
	mov	cl,[disflags2]
	or	[disflags],cl	;if it was variable size operand, the size
						;should now be marked as known.
	cmp	ah,0
	jl	dop35			;if byte register
	jz	dop34			;if word register
dop33a:
	mov	byte [di],'E'	;enter here from OP_ECX
	inc	di
dop34:
	add	al,8
dop35:
	cbw
	shl	ax,1
	xchg ax,bx			;mov bx,ax
	mov	ax,[rgnam816+bx];get the register name
	stosw
	ret

;	Register number embedded in the instruction (OP_R_ADD)

dop36:
	mov	al,[instru]
	jmp	dop33

;	AL or AX or EAX (OP_AX)

dop37:
	mov	al,0
	jmp	dop33

;	QWORD mem (OP_M64).

dop38:
	mov	ax,'Q'		;print 'QWORD'
	jmp	dop40

;	FLOAT mem (OP_MFLOAT).

dop38a:
	mov	ax,'FL'
	stosw
	mov	al,'O'
	stosb
	mov	ax,'AT'
	jmp	dop38c

;	DOUBLE mem (OP_MDOUBLE).

dop38b:
	mov	ax,'DO'
	stosw
	mov	ax,'UB'
	stosw
	mov	ax,'LE'
dop38c:
	stosw
	call showptr
	jmp	dop42a

;	TBYTE mem (OP_M80).

dop39:
	mov	ax,0ff00h+'T'	;print 'tbyte'
dop40:
	stosb
	call getregmem
	cmp	al,0c0h
	jae	disbad1		;if it's a register reference
	and	byte [disflags],~DIS_F_SHOW	;don't show this
	jmp	dop05

;	far memory (OP_FARMEM).

dop41:
	call dischk32d
	jz	dop41a		;if not dword far
	call showdwd
	sub	di,4		;erase "PTR "
dop41a:
	mov	ax,'FA'		;store "FAR "
	stosw
	mov	ax,'R '
	stosw

;	mem (OP_MXX).

dop42:
	and	byte [disflags],~DIS_F_SHOW	;don't show this
dop42a:
	call getregmem
	cmp	al,0c0h
	jae	disbad5		;if it's a register reference
	jmp	dop06

disbad5:jmp	disbad

;	far pointer (OP_FARP).

dop43:
	call disgetword
	push ax
	call dischk32d
	jz	dop44		;if not 32-bit address
	call disgetword
	push ax
dop44:
	call disgetword
	call hexword
	mov	al,':'
	stosb
	call dischk32d
	jz	dop45		;if not 32-bit address
	pop	ax
	call hexword
dop45:
	pop	ax
	call hexword
	ret

;	8-bit relative jump (OP_REL8)

dop46:
	call disgetbyte
	cbw
	jmp	dop48

;	16/32-bit relative jump (OP_REL1632)

dop47:
	call disgetword
	call dischk32d
	jz	dop48		;if not 32-bit offset
	push ax
%if PM    
    test byte [bCSAttr],40h	;for 32bit code segments
    jnz dop47_1				;no need to display "DWORD "
%endif
	call showdwd
	sub	di,4		;erase "PTR "
dop47_1:    
	pop	dx
	call disgetword
	mov	bx,[u_addr+0]
	add	bx,[dis_n]
	add	dx,bx
	adc	ax,[u_addr+2]
	call hexword
	xchg ax,dx
	jmp	hexword		;call hexword and return

dop48:
%if PM
	test byte [bCSAttr],40h
    jnz dop48_1
%endif    
	add	ax,[u_addr]
	add	ax,[dis_n]
	jmp	hexword		;call hexword and return
%if PM
dop48_1:
	cpu 386
	movsx eax,ax
    add eax,[u_addr]
    add eax,[dis_n]
    push ax
    shr eax,16
    call hexword
    pop ax
    jmp hexword
	cpu 8086
%endif

;	Check for ST(1) (OP_1CHK).

dop49:
	pop	ax		;discard return address
	mov	al,[regmem]
	and	al,7
	cmp	al,1
	je	dop50		;if it's ST(1)
	jmp	da14		;another operand (but no comma)

dop50:
	jmp	da21		;end of list

;	ST(I) (OP_STI).

dop51:
	mov	al,[regmem]
	and	al,7
	xchg ax,bx		;mov bx,ax
	mov	ax,'ST'
	stosw			;store ST(bl)
	mov	al,'('
	stosb
	mov	ax,'0)'
	or	al,bl
	stosw
	ret

;	CRx (OP_CR).

dop52:
	mov	bx,'CR'
	call getregmem_r
	cmp	al,4
	ja	disbad4		;if too large
	jne	dop52a
	mov	byte [dismach],5	;CR4 is new to the 586
dop52a:
	cmp	word [index],SPARSE_BASE+22h
	jne	dop55		;if not MOV CRx,xx
	cmp	al,1
	jne	dop55		;if not CR1

disbad4:jmp	disbad		;can't MOV CR1,xx

;	DRx (OP_DR).

dop53:
	call getregmem_r
	mov	bx,'DR'
	mov	cx,-1		;no max or illegal value
	jmp	dop55

;	TRx (OP_TR).

dop54:
	call getregmem_r
	cmp	al,3
	jb	disbad		;if too small
	cmp	al,6
	jae	dop54a		;if TR6-7
	mov	byte [dismach],4	;TR3-5 are new to the 486
dop54a:
	mov	bx,'TR'

dop55:
	xchg ax,bx
	stosw			;store XX
	xchg ax,bx
	or al,'0'
	stosb
	ret

;	Segment register (OP_SEGREG).

dop56:
	call getregmem_r
	cmp	al,6
	jae	disbad		;if not a segment register
	cmp	al,2
	je dop57		;if SS
	and	byte [disflags],~DIS_F_REPT	;clear flag:  don't repeat
dop57:
	cmp	al,4
	jb dop57a		;if not FS or GS
	mov	byte [dismach],3;(no new 486-686 instructions involve seg regs)
dop57a:
	add	al,16
	jmp	dop35		;go print it out

;	Sign-extended immediate byte (OP_IMMS8). "push xx"

dop58:
	call disgetbyte
	cmp	al,0
	xchg ax,bx		;mov bl,al
	mov	al,'+'
	jge	dop58a		;if >= 0
	neg	bl
	mov	al,'-'
dop58a:
	stosb
	xchg ax,bx		;mov al,bl
	jmp	dop59a		;call hexbyte and return

;	Immediate byte (OP_IMM8).

dop59:
	call disgetbyte
dop59a:
	jmp	hexbyte		;call hexbyte and return

;	Show ECX if it's a 32-bit operand.

dop59e:
	call dischk32d
	jz	dop60a		;if not 32 bit instruction
	mov	al,1
	jmp	dop33a		;print out register ECX and return

;	Set flag to always show size (OP_SHOSIZ).

dop60:
	or	byte [disflags],DIS_I_SHOWSIZ
dop60a:
	pop	ax			;discard return address
	jmp	da14		;next...

disbad:
	mov	sp,[savesp2]	;pop junk off stack
	mov	word [dis_n],0
	mov	word [preflags],0 ;clear preflags and preused
	mov	byte [rmsize],80h	;don't display any memory
	mov	word [dismach],0;forget about the machine type
	and	byte [disflags],~DIS_I_SHOW	;and flags
	call disgetbyte
	mov	di,prefixlist
	mov	cx,10
	repne scasb
	je	dbad1		;if it's a named prefix
	dec	word [dis_n]
	mov	bx,MNEM_DB	;offset of 'DB' mnemonic
	mov	si,OPLIST_Z	;this says OP_8+OP_IMM
	jmp	da13

dbad1:
	or	byte [disflags],DIS_I_UNUSED	;print special flag
	mov	bx,9
	sub	bx,cx
	shl	bx,1
	cmp	bx,12
	jb	dbad2		;if SEG directive
	mov	bx,[prefixmnem+bx-12]
	mov	si,OPTYPES_BASE
	jmp	da13

dbad2:
	lea	si,[bx+OPLIST_ES]
	mov	bx,MNEM_SEG
	jmp	da13

;	GETREGMEM_R - Get the reg part of the reg/mem part of the instruction
;	Uses	CL

getregmem_r:
	call	getregmem
	mov	cl,3
	shr	al,cl
	and	al,7
	ret

;	GETREGMEM - Get the reg/mem part of the instruction

getregmem:
	test byte [preused],GOTREGM
	jnz	grm1		;if we have it already
	or	byte [preused],GOTREGM
	call disgetbyte	;get the byte
	mov	[regmem],al	;save it away

grm1:	mov	al,[regmem]
	ret

;	SHOWOP	Show the op code
;	Entry	SI	Null-terminated string containing the op mnemonic
;	Exit	DI	Address of next available byte in output line
;			(>= offset line_out + 32 due to padding)
;	Uses	AL

showop:
	mov	di,line_out+MNEMONOFS
showop1:
    lodsb
    mov ah,al
    and al,7Fh
    stosb
    and ah,ah
    jns showop1
	mov	al,' '
so1:
	stosb
	cmp	di,line_out+MNEMONOFS+8
	jb so1
	ret

;	SHOWSEG - Show the segment descriptor in SEGMNT
;	Entry	DI	Where to put it
;	Exit	DI	Updated
;	Uses	AX, BX

showseg:
	mov	al,[segmnt]	;segment number
	cbw
	shl	ax,1
	xchg	ax,bx		;mov bx,ax
	mov	ax,[segrgnam+bx] ;get register name
	stosw
	ret

;	SHOWSIZE - Print a description of the size
;	Entry	AH	>0 for DWORD, =0 for WORD, <0 for BYTE
;	Uses	AX

;	SHOWPTR - Print " PTR "
;	Uses	AX

;	SHOWDWD - Print "DWORD PTR"
;	Uses	AX

showsize:
	mov	[rmsize],ah	;save r/m size
	mov	[sizeloc],di	;save where we're putting this
	cmp	ah,0
	jge	ssz1		;if word or dword
	mov	ax,'BY'
	stosw
	mov	ax,'TE'
	jmp	ssz3

ssz1:
	je	ssz2		;if word
showdwd:
	mov	al,'D'
	stosb
ssz2:
	mov	ax,'WO'
	stosw
	mov	ax,'RD'
ssz3:
	stosw
showptr:
	mov	ax,' P'
	stosw
	mov	ax,'TR'
	stosw
	mov	al,' '
	stosb
	ret

;	DISP32 - Print 32-bit displacement for addressing modes.
;	Entry	None
;	Exit	None
;	Uses	AX

disp32:
	call disgetword
	push ax
	call disgetword
	call hexword
	pop	ax
	call hexword
	ret

;	SHOWREG16 - Show 16-bit register name.
;	Entry	AL	register number (0-7)
;	Exit	None
;	Uses	AX

showreg16:
	cbw
	shl	ax,1
	xchg ax,bx
	push ax
	mov	ax,[rgnam16+bx]
	stosw
	pop	ax
	xchg ax,bx
	ret

;	SHOWMACH - Return string "[needs math coprocessor]", etc.
;	Exit	si	Address of string
;		cx	Length of string, or 0 if not needed
;	Uses	al, di

showmach:
	mov	si,needsmsg		;candidate message
	test byte [dmflags],DM_COPR
	jz	sm1				;if not a coprocessor instruction
	mov	byte [si+9],'7'	;change message text
	mov	al,[mach_87]
	cmp	byte [has_87],0
	jnz	sm2				;if it has a coprocessor
	mov	al,[machine]
	cmp	al,[dismach]
	jb	sm3				;if we display the message
	mov	si,needsmath	;print this message instead
	mov	cx,needsmath_L
	ret

sm1:
	mov	byte [si+9],'6'	;reset message text
	mov	al,[machine]
sm2:
	cmp	al,[dismach]
	jae	sm4				;if no message (so far)
sm3:
	mov	al,[dismach]
	add	al,'0'
	mov	[si+7],al
	mov	cx,needsmsg_L	;length of the message
	ret

;	Check for obsolete instruction.

sm4:
	mov	si,obsolete	;candidate message
	mov	ax,[di-2]	;get info on this instruction
	mov	cx,5
	repne scasw
	jne	sm5			;if no matches
	mov	di,obsmach + 5 - 1
	sub	di,cx
	xor	cx,cx		;clear CX:  no message
	mov	al,[mach_87]
	cmp	al,[di]
	jle	sm5			;if this machine is OK
	mov	cx,obsolete_L
sm5:
	ret

;	DISGETBYTE - Get byte for disassembler.
;	Entry	None
;	Exit	AL	Next byte in instruction stream
;	Uses	None

disgetbyte:
	push ds
%if PM
	test byte [bCSAttr],40h
    jnz disgb_1
%endif    
	push si
	mov	si,[u_addr]
	mov	ds,[u_addr+4]
	add	si,[cs:dis_n]	;index to the right byte
	lodsb 			;get the byte
    pop si
	pop	ds
	inc	word [dis_n]	;indicate that we've gotten this byte
	ret
%if PM    
	cpu 386
disgb_1:
	push esi
	lds	esi,[u_addr]
	add	esi,[cs:dis_n]	;index to the right byte
	a32 lodsb
	pop	esi
	pop	ds
	inc	dword [dis_n]	;indicate that we've gotten this byte
    ret
    cpu 8086
%endif    

;	DISGETWORD - Get word for disassembler.
;	Entry	None
;	Exit	AX	Next word
;	Uses	None

disgetword:
	push ds
%if PM
	test byte [bCSAttr],40h
    jnz disgw_1
%endif    
	push si		;save si
	mov	si,[u_addr]
	mov	ds,[u_addr+4]
	add	si,[cs:dis_n]	;index to the right byte
	lodsw
	pop	si		;restore things
	pop	ds
	add	word [dis_n],2
	ret
%if PM 
	cpu 386
disgw_1:    
	push esi
	lds	esi,[u_addr]
	add	esi,[cs:dis_n]	;index to the right byte
	a32 lodsw
	pop	esi
	pop	ds
	add	dword [dis_n],2
	ret
    cpu 8086
%endif

;	DISSHOWBYTES - Show bytes for the disassembler.
;	Entry	BX	Number of bytes (must be > 0)
;	Exit		u_addr updated
;	Uses	BX, SI.

disshowbytes:
%if PM
	test byte [bCSAttr],40h
    jnz dissb_1
%endif
	mov	si,[u_addr]
	mov	ds,[u_addr+4]
dsb1:
	lodsb
	call hexbyte
	dec	bx
	jnz	dsb1
	push ss
	pop	ds
	mov	[u_addr],si
	ret
%if PM
	cpu 386
dissb_1:
	lds	esi,[u_addr]
dsb2:
	a32 lodsb
	call hexbyte
	dec	bx
	jnz	dsb2
	push ss
	pop	ds
	mov	[u_addr],esi
	ret
    cpu 8086
%endif    

;	MOVEOVER - Move the line to the right.
;	Entry	DI	Last address + 1 of line so far
;	Exit	CX	Number of bytes to move
;	DI	Updated
;	Uses	SI

moveover:
	cmp	word [sizeloc],0
	je	mo1		;if sizeloc not saved
	add	[sizeloc],cx

mo1:
	mov	si,di
	add	di,cx
	mov	cx,di
	sub	cx,line_out+MNEMONOFS
	push di
	std
	dec	si
	dec	di
	rep	movsb
	pop	di
	cld
	ret

;	DISCHK32D - Check for 32 bit operand size prefix (66h).

dischk32d:
	or	byte [preused],PRE32D
	test byte [preflags],PRE32D
	ret

;	DUMPREGS - Dump registers.

dumpregs:
	mov	si,regs
	mov	bx,regnames
	mov	di,line_out
	mov bp,regshi
	mov	cx,8			;print all 8 std regs (16-bit)
    test [regsdmp],byte 1
    jz no386_2
    mov cl,6			;room for 6 std regs (32-bit) only
no386_2:
	call dmpr1			;print first row
	push bx
	call trimputs
	pop	bx
	mov	di,line_out
    test [regsdmp],byte 1
    jnz no386_3
	mov	cl,4			;print 4 segment regs
	call dmpr1
    add si,2*2			;skip FS+GS
    add bx,2*2			;skip FS+GS
    jmp no386_31
no386_3:
    mov cl,2			;print rest of 32-bit std regs
	call dmpr1
    push bx
    push si
	call dmpflags	;print flags in 386 mode 
	call trimputs
    pop	si
    pop	bx
	mov	di,line_out
    mov cl,6		;print ds, es, ss, cs, fs, gs
    call dmpr1_1
    mov	bp,regh_eip
no386_31:
    mov	cl,1
    call dmpr1		;print (E)IP
	test [regsdmp],byte 1
    jnz	no386_32
	call dmpflags	;print flags in 8086 mode
no386_32:    
	call trimputs
	mov	ax,[reg_ip]
	mov	di,u_addr
	stosw
	mov	ax,[regh_eip]
	stosw
	mov	ax,[reg_cs]
	stosw
	mov	byte [disflags],DIS_F_REPT | DIS_F_SHOW
	call disasm
	mov	ax,[regh_eip]
	mov	[u_addr+2],ax
	mov	ax,[reg_ip]
	mov	[u_addr],ax
	ret
    

;	Function to print multiple register entries.

dmpr1:
	test [regsdmp],byte 1
    jnz dmpr1X
dmpr1_1:
    mov	ax,[bx]
	inc	bx
	inc	bx
	stosw
	mov	al,'='
	stosb
	lodsw
	push	cx
	call hexword
	pop	cx
	mov	al,' '
	stosb
	loop	dmpr1_1
	ret
;	Function to print multiple register entries.

dmpr1X:	
	mov al,'E'
    stosb
	mov	ax,[bx]
	inc	bx
	inc	bx
	stosw
	mov	al,'='
	stosb
    xchg bp,si
	lodsw
	push cx
	call hexword
    xchg bp,si
	lodsw
	call hexword
	pop	cx
	mov	al,' '
	stosb
	loop	dmpr1X
	ret

;   DUMPREGSF - Dump Floating Point Registers 

;fregnames 
;	db "CW", 2, "SW", 2, "TW", 2, "IP", 4, "CS", 2, "OO", 4, "OS", 2
fregnames
	db "CW", "SW", "TW"
    db "OPC=", "IP=", "DP="
dEmpty db "<empty>              "

dumpregsF:
	mov	di,line_out
	mov	si,fregnames
    sub sp,7*2+8*10
    mov bp,sp
    cpufpu 8086
    fnsave [bp]
    cpu 8086
    mov cx,3
nextfpr:    
	movsw
    mov al,'='
    stosb
    push cx
    mov ax,[bp+0]
    call hexword
    pop cx
    mov al,' '
    stosb
    add bp,2
    loop nextfpr
    
    movsw
    movsw
    mov ax,[bp+2]
    and ax,0FFFh
    call hexword
    mov al,' '
    stosb
    
    mov cx,2
nextfp:    
    push cx
    movsw
    movsb
    mov al,[bp+2+1]
    mov cl,4
    shr al,cl
    call hexnyb
    mov ax,[bp+0]
    call hexword
    mov al,' '
    stosb
    add bp,4
    pop cx
    loop nextfp
    
	call trimputs
    
    mov si,[bp-5*2]
    mov ax,[bp-6*2]	;get TOP

	mov cl,11
    shr ax, cl
    mov cl,al
    
    and cl, 7
    shl cl, 1
    ror si, cl
	mov	di,line_out
	mov cx,8
nextst:    
    mov ax,"ST"
    stosw
    mov al,8
    sub al,cl
    add al,'0'
    stosb
    mov al,'='
    stosb
    mov ax,si
    ror si, 1	;remain 8086 compatible here!
    ror si, 1
    and al,3
    cmp al,3
    jnz notinval
    push si
    push cx
	mov si, dEmpty
    mov cx, 21
    rep movsb
    pop cx
    pop si
    jmp regoutdone
notinval:
%if PM
	push cx
    
	push di
    push bp
	call FloatToStr
    add di,21
    pop cx
%else
    push cx
    mov ax,[bp+8]
    call hexword
    mov al,'.'
    stosb
    mov ax,[bp+6]
    call hexword
    mov ax,[bp+4]
    call hexword
    mov ax,[bp+2]
    call hexword
    mov ax,[bp+0]
    call hexword
    pop cx
%endif    
regoutdone:
    mov al,' '
    stosb
    test cl,1
    jz nocr
    push cx
	call trimputs
	mov	di,line_out
    pop cx
nocr:    
	add bp,10
    loop nextst
    mov bp,sp
    cpufpu 8086
    frstor [bp]
    cpu 8086
    add sp,7*2+8*10
	ret

;	DMPFLAGS - Dump flags output.

dmpflags:
	mov	bx,[reg_fl]
	mov	si,flgbits
	mov	cx,8
dmpf1:	mov	ax,[si+16]
	test	bx,[si]
	jz	dmpf2		;if not asserted
	mov	ax,[si+32]
dmpf2:	stosw
	mov	al,' '
	stosb
	inc	si
	inc	si
	loop	dmpf1
	ret

flgbits	dw	800h,400h,200h,80h,40h,10h,4,1
flgnams	dw	'NV','UP','DI','PL','NZ','NA','PO','NC'
flgnons	dw	'OV','DN','EI','NG','ZR','AC','PE','CY'

;	SHOWSTRING - Print nonempty null-terminated string.

showstring:
	lodsb
sstr1:
	stosb
	lodsb
	cmp	al,0
	jne	sstr1
	ret

;	HEXWORD - Print hex word (in AX).
;	HEXBYTE - Print hex byte (in AL).
;	HEXNYB - Print hex digit.
;	Uses	al,cl.

hexword:
	push ax
	mov	al,ah
	call hexbyte
	pop	ax

hexbyte:
	push ax
	mov	cl,4
	shr	al,cl
	call hexnyb
	pop	ax

hexnyb:
	and	al,0fh
	add	al,90h		;these four instructions change to ascii hex
	daa
	adc	al,40h
	daa
	stosb
	ret

;	TAB_TO - Space fill until reaching the column indicated by AX.
;	(Print a new line if necessary.)

tab_to:	push	ax
	sub	ax,di
	ja	tabto1		;if there's room on this line
	call	trimputs
	mov	di,line_out

tabto1:	pop	cx
	sub	cx,di
	mov	al,' '
	rep	stosb		;space fill to the right end
	ret

;	TRIMPUTS - Trim excess blanks from string and print (with CR/LF).
;	PUTSLINE - Add CR/LF to string and print it.
;	PUTS - Print string through DI.

trimputs:
	dec	di
	cmp	byte [di],' '
	je	trimputs
	inc	di

putsline:
	mov	ax,LF * 256 + CR
	stosw

puts:
	mov	cx,di
	mov	dx,line_out
	sub	cx,dx
stdout:    			;write DS:DX, size CX to STDOUT (1)
	call InDos
    jnz int29_puts
	mov	bx,1		;standard output
	mov	ah,40h		;write to file
    call doscall
    ret
int29_puts:    
	jcxz nooutput
	push si
    mov si,dx
nextchar:    
    lodsb
    mov bx,0007
    mov ah,0Eh
    int 10h
    loop nextchar
    pop si
nooutput:
    ret

gethwefl:
    cmp byte [machine],3
    jb gethwefl_1
    cpu 386
    pushfd
    pop ax
    pop word [regh_efl]
    cpu 8086
gethwefl_1:
	ret

createdummytask:

	mov di, regs
    mov cx, 16*2	;(8 std, 6 seg, ip, fl) * 2
    xor ax, ax
    rep stosw
    mov word [reg_ip],100h
    pushf
    pop word [reg_fl]
    call gethwefl

    mov ah,48h		;get largest free block
    mov bx,-1
    int 21h
    mov ah,48h		;allocate it
    int 21h
    jc  ct_done
    
    push bx
	mov	di,reg_ds	;fill segment registers ds,es,ss,cs
	stosw
	stosw
	stosw
	stosw
	call adusetup
	mov	bx,[reg_cs]	;bx:dx = where to load program
	mov	es,bx
    pop ax			;get size of memory block
    mov dx,ax
    add dx,bx
	mov	[es:ALASAP],dx
	cmp	ax,1000h
	jbe	init14		;if memory left <= 64K
	xor	ax,ax		;ax = 1000h (same thing, after shifting)
init14:
	mov	cl,4
	shl	ax,cl
	dec	ax
	dec	ax
	mov	[reg_sp],ax
	xchg ax,di		;es:di = child stack pointer
	xor	ax,ax
	stosw			;push 0 on client's stack

;	Create a PSP

	mov	ah,55h		;create child PSP
	mov	dx,es
	mov	si,[es:ALASAP]
	clc			;works around OS/2 bug
	int	21h
	mov	word [es:TPIVOFS],int22
	mov	[es:TPIVSEG],cs
    cmp byte [bInit],0
    jnz nomemtouch
    inc byte [bInit]
    mov [es:100h],byte 0C3h	;place opcode for 'RET' at CS:IP
nomemtouch:
    mov [pspdbe],es
    mov ax,es
    dec ax
    mov es,ax
    inc ax
    mov [es:0001],ax
    mov byte [es:0008],0
    push cs
    pop es
    mov bx,cs
    call setpsp
ct_done:    
    ret

%if PM

;--- hook int 2Fh if a DPMI host is found
;--- for Win9x and DosEmu host
;--- int 2Fh, ax=1687h is not hooked, however
;--- because it doesn't work. Debugging
;--- in protected-mode still may work, but
;--- the initial-switch to PM must be single-stepped
;--- modifies AX, BX, CX, DX, DI

hook2f:
    cmp word [oldi2f+2],0
    jnz hook2f_2
	mov ax,1687h			;DPMI host installed?
    int 2Fh
    and ax,ax
    jnz hook2f_2
    mov [dpmientry+0],di	;true host DPMI entry
    mov [dpmientry+2],es
    mov [dpmiwatch+0],di
    mov [dpmiwatch+2],es
	cmp byte [bNoHook2F],0	;can int 2Fh be hooked?
    jnz hook2f_2
	mov word [dpmiwatch+0],mydpmientry
	mov word [dpmiwatch+2],cs
	mov ax,352Fh
    int 21h
    mov [oldi2f+0],bx
    mov [oldi2f+2],es
    mov dx,debug2F
    mov ax,252Fh
    int 21h
hook2f_1:
	push ds
    pop es
%if DISPHOOK
	mov ax,cs
    mov di,dpmihookcsip
    call hexword
    inc di
    mov ax,mydpmientry
    call hexword
    mov dx,dpmihook
    call int21ah9
%endif
hook2f_2:
	push ds
    pop es
	ret
%endif

;	I/O buffers.  (End of permanently resident part.)

end1:
line_in		db	255,0,CR	;length = 257
line_out	equ	end1+258	;length = 1 + 263
real_end	equ	end1+258+264
staksiz		equ	200h

initcont:    
    int 21h			;free rest of DOS memory
	mov	byte [line_out-1],'0'	;initialize line_out
	call ll3		;load a program if one has been given at the command line
	jmp	cmd3		;done

				;lots of bytes follow this

;---------------------------------------
;	Debug initialization code.
;---------------------------------------

	align	2		;align on word boundary
fp_status dw	5a5ah		;FPU status word

imsg1	db	DBGNAME,' version 1.13.  Debugger.',CR,LF,CR,LF
	db	'Usage:	', DBGNAME, ' [[drive:][path]progname [arglist]]',CR,LF,CR,LF
	db	'  progname	(executable) file to debug or examine',CR,LF
	db	'  arglist	parameters given to program',CR,LF,CR,LF
	db	'For a list of debugging commands, '
	db	'run ', DBGNAME, ' and type ? at the prompt.',CR,LF,'$'

imsg2	db	'Invalid switch - '
imsg2a	db	'x',CR,LF,'$'

initcode:
	cld

	mov	[execblk+4],cs	;set up parameter block for exec command
	mov	[execblk+8],cs
	mov	[execblk+12],cs
    mov [pspdbg],es

;	Check for console input vs. input from a file or other device.

	mov	ax,4400h	;IOCTL--get info
	xor	bx,bx
	int	21h
	jc	init1		;if not device
	and	dl,81h		;check if console device
	cmp	dl,81h
	jne	init1		;if not the console input
	mov	byte [notatty],0	;it _is_ a tty

;	Check DOS version

init1:
	mov	ah,30h		;check DOS version
	int	21h
	xchg al,ah
	cmp	ax,31fh
	jb	init2		;if version < 3.3, then don't use new INT 25h method
	inc	byte [usepacket]
%if VDD
    cmp ah,5
    jnz isnotnt
    mov ax,3306h
    int 21h
    cmp bx,3205h
    jnz isnotnt
    mov si,szDebxxVdd	;DS:SI->"DEBXXVDD.DLL"
    mov bx,szDispatch	;DS:BX->"Dispatch"
    mov di,szInit		;ES:DI->"Init"
    RegisterModule
    jc init2
    mov [hVdd], ax
    jmp isntordos71
szDebxxVdd	db "DEBXXVDD.DLL",0
szDispatch	db "Dispatch",0
szInit		db "Init",0
isnotnt:
%endif
    cmp ax,070Ah
    jb  init2
isntordos71:
	inc	byte [usepacket]

;	Determine the processor type.  This is adapted from code in the
;	Pentium<tm> Family User's Manual, Volume 3:  Architecture and
;	Programming Manual, Intel Corp., 1994, Chapter 5.  That code contains
;	the following comment:

;	This program has been developed by Intel Corporation.
;	Software developers have Intel's permission to incorporate
;	this source code into your software royalty free.

;	Intel 8086 CPU check.
;	Bits 12-15 of the FLAGS register are always set on the 8086 processor.
;	Probably the 186 as well.

init2:
    push sp
    pop ax
    cmp ax,sp		
    jnz init6		;if 8086 or 80186 (can't tell them apart)

;	Intel 286 CPU check.
;	Bits 12-15 of the flags register are always clear on the
;	Intel 286 processor in real-address mode.

	mov	byte [machine],2
	pushf			;get original flags into AX
	pop	ax
	or	ax,0f000h	;try to set bits 12-15
	push ax			;save new flags value on stack
	popf			;replace current flags value
	pushf			;get new flags
	pop	ax			;store new flags in AX
	test ah,0f0h	;if bits 12-15 clear, CPU = 80286
	jz	init6		;if 80286

;	Intel 386 CPU check.
;	The AC bit, bit #18, is a new bit introduced in the EFLAGS
;	register on the Intel486 DX cpu to generate alignment faults.
;	This bit cannot be set on the Intel386 CPU.

;	It is now safe to use 32-bit opcode/operands.

	cpufpu	386

	inc	byte [machine]
	mov	bx,sp		;save current stack pointer to align
	and	sp,~3		;align stack to avoid AC fault
	pushfd			;push original EFLAGS
	pop	eax			;get original EFLAGS
	mov	ecx,eax		;save original EFLAGS in CX
	xor	eax,40000h	;flip (XOR) AC bit in EFLAGS
	push eax		;put new EFLAGS value on stack
	popfd			;replace EFLAGS value
	pushfd			;get new EFLAGS
	pop	eax			;store new EFLAGS value in EAX
	cmp	eax,ecx
	jz	init5		;if 80386 CPU

;	Intel486 DX CPU, Intel487 SX NDP, and Intel486 SX CPU check.
;	Checking for ability to set/clear ID flag (bit 21) in EFLAGS
;	which indicates the presence of a processor with the ability
;	to use the CPUID instruction.

	inc	byte [machine]	;it's a 486
	mov	eax,ecx		;get original EFLAGS
	xor	eax,200000h	;flip (XOR) ID bit in EFLAGS
	push eax		;save new EFLAGS value on stack
	popfd			;replace current EFLAGS value
	pushfd			;get new EFLAGS
	pop	eax			;store new EFLAGS in EAX
	cmp	eax,ecx		;check if it's changed
	je	init5		;if it's a 486 (can't toggle ID bit)
	push ecx
	popfd			;restore AC bit in EFLAGS first
	mov	sp,bx		;restore original stack pointer

;	Execute CPUID instruction.

	cpu	586

	xor	eax,eax		;set up input for CPUID instruction
	cpuid
	cmp	eax,1
	jl	init6		;if 1 is not a valid input value for CPUID
	xor	eax,eax		;otherwise, run CPUID with ax = 1
	inc	eax
	cpuid
	mov	al,ah
	and	al,0fh		;bits 8-11 are the model number
	cmp	al,6
	jbe	init3		;if <= 6
	mov	al,6		;if > 6, set it to 6
init3:	
	mov	[machine],al;save it
	jmp	init6		;don't restore SP

init5:
	push	ecx
	popfd			;restore AC bit in EFLAGS first
	mov	sp,bx		;restore original stack pointer

	cpufpu	8086 	;back to 1980s technology
    
;	Next determine the type of FPU in a system and set the mach_87
;	variable with the appropriate value.  All registers are used by
;	this code; none are preserved.

;	Coprocessor check.
;	The algorithm is to determine whether the floating-point
;	status and control words can be written to.  If not, no
;	coprocessor exists.  If the status and control words can be
;	written to, the correct coprocessor is then determined
;	depending on the processor ID.  The Intel386 CPU can
;	work with either an Intel 287 NDP or an Intel387 NDP.
;	The infinity of the coprocessormust be checked
;	to determine the correct coprocessor ID.

init6:
	mov	al,[machine]
	mov	[mach_87],al	;by default, set mach_87 to machine
    inc byte [has_87]
    cmp al,5			;a Pentium or above always will have a FPU
    jnc init7
    dec byte [has_87]

	fninit				;reset FP status word
	mov	word [fp_status],5a5ah	;restore temp word to nonzero value
	fnstsw	[fp_status]	;save FP status word
	mov	ax,[fp_status]	;check FP status word
	cmp	al,0
	jne	init7			;if no FPU present

	fnstcw	[fp_status]	;save FP control word
	mov	ax,[fp_status]	;check FP control word
	and	ax,103fh		;see if selected parts look OK
	cmp	ax,3fh
	jne	init7			;if no FPU present
	inc	byte [has_87]	;there's an FPU

;	If we're using a 386, check for 287 vs. 387 by checking whether
;	+infinity = -infinity.

	cmp	byte [machine],3
	jne	init7		;if not a 386
	fld1			;must use default control from FNINIT
	fldz			;form infinity
	fdivp	ST1		;1 / 0 = infinity
	fld	ST0			;form negative infinity
	fchs
	fcompp			;see if they are the same and remove them
	fstsw	[fp_status]	;look at status from FCOMPP
	mov	ax,[fp_status]
	sahf
	jnz	init7			;if they are different, then it's a 387
	dec	byte [mach_87]	;otherwise, it's a 287

;	Interpret switches and erase them from the command line.

init7:	
	mov	ax,3700h		;get switch character
	int	21h
	mov	[switchar],dl
	cmp	dl,'/'
	jne	init8
	mov	[swch1],dl
init8:
	mov	si,DTA+1
init9:
	lodsb
	cmp	al,' '
	je	init9
	cmp	al,TAB
	je	init9

;	Process the /? switch (or the [switchar]? switch).
;	If switchar != / and /? occurs, make sure nothing follows.

	cmp	al,dl
	je	init11		;if switch character
	cmp	al,'/'
	jne	init12		;if not the help switch
	mov	al,[si]
	cmp	al,'?'
	jne	init12		;if not /?
	mov	al,[si+1]
	cmp	al,' '
	je	init10		;if nothing after /?
	cmp	al,TAB
	je	init10		;ditto
	cmp	al,CR
	jne	init12		;if not end of line

;	Print a help message

init10:	mov	dx,imsg1	;command-line help message
	call	int21ah9	;print string
	int	20h		;done

;	Do the (proper) switches.

init11:	lodsb
	cmp	al,'?'
	je	init10		;if -?

;	||| Other switches may go here.

	mov	[imsg2a],al
	mov	dx,imsg2	;Invalid switch
	call	int21ah9	;print string
	mov	ax,4c01h	;Quit and return error status
	int	21h

;	Feed the remaining command line to the 'n' command.

init12:
	dec	si
	lodsb
	call nn		;process the rest of the line

;	Set up interrupt vectors.

	mov	cx,4
	mov	si,inttab
    mov di,intsave
init13:	
	lodsb
    mov ah,35h
    int 21h
    mov [di+0],bx
    mov [di+2],es
    add di,4
    xchg ax,dx
	lodsw			;get address
	xchg ax,dx
	mov	ah,25h		;set interrupt vector
	int	21h
	loop init13

	push es
%if MCB
	mov ah,52h		;get list of lists
    int 21h
    mov ax,[es:bx-2]	;start of MCBs
    mov [wMCB],ax
%endif
	mov ah,34h
    int 21h
    mov word [pInDOS+0],bx
    mov word [pInDOS+2],es
%if PM
    mov [InDosSegm],es
%endif
    pop es
    
;-- get address of DOS swappable DATA area
;-- to be used to get/set PSP and thus avoid DOS calls
;-- will not work for DOS < 3

%if USESDA
	push ds
    mov ax,5D06h
    int 21h
    mov ax,ds
    pop ds
    jc noSDA
    mov word [pSDA+0],si
    mov word [pSDA+2],ax
%if PM    
    mov [SDASegm],ax
%endif    
noSDA:
%endif


%if PM

;--- Windows 9x and DosEmu are among those hosts which handle some
;--- V86 Ints internally without first calling the interrupt chain.
;--- This causes various sorts of troubles and incompatibilities.

%if WIN9XSUPP
    mov ax,1600h	;running in a win9x dos box?
    int 2Fh
    and al,al
    jnz no2fhook
%endif
%if DOSEMU
    mov ax,0F000h
    mov es,ax
    mov di,0FFF5h
    mov si,dDosEmuDate
    mov cx,4
    repe cmpsw		;running in DosEmu?
    jz no2fhook
%endif
	jmp dpmihostchecked
no2fhook:    
    inc byte [bNoHook2F]
dpmihostchecked:
%endif

	push ds
    pop es

;	Save and modify termination address and the parent PSP field.

	mov	si,TPIVOFS
	mov	di,psp22
	movsw
	movsw
	mov	word [si-4],debug22
	mov	[si-2],cs
	mov	si,16h
	movsw
	mov	[si-2],cs

    mov [pspdbe],cs	;indicate there is no debuggee loaded yet

;	shrink DEBUG and set its stack
    
	mov	ax,real_end + staksiz + 15
	and	ax,~15		;new stack pointer
    mov sp,ax
	dec	ax
	dec	ax
	mov	[savesp],ax	;save new SP minus two (for the word we'll push)
	inc	ax
	inc	ax
	mov	cl,4
	shr	ax,cl
	mov bx,ax
    mov ah,4Ah
    jmp initcont
