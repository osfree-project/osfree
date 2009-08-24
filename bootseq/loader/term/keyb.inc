;
; keyb.inc:
; Keyboard defines
;

; The size of the key map.
KEY_MAP_SIZE	equ      128

K_RDWR		equ	 0x60     ; keyboard data & cmds (read/write)
K_STATUS	equ      0x64     ; keyboard status
K_CMD		equ      0x64     ; keybd ctlr command (write-only)

K_OBUF_FUL	equ      0x01     ; output buffer full
K_IBUF_FUL	equ      0x02     ; input buffer full

KC_CMD_WIN	equ      0xd0     ; read  output port
KC_CMD_WOUT	equ      0xd1     ; write output port
KB_OUTPUT_MASK	equ	 0xdd     ; enable output buffer full interrupt
                                  ; enable data line
                                  ; enable clock line
KB_A20_ENABLE	equ	 0x02

; Codes for getchar.
KEY_LEFT	equ        0x4B00
KEY_RIGHT	equ        0x4D00
KEY_UP		equ        0x4800
KEY_DOWN	equ        0x5000
KEY_IC		equ        0x5200 ; insert char
KEY_DC		equ        0x5300 ; delete char
KEY_BACKSPACE	equ	   0x0008
KEY_HOME	equ        0x4700
KEY_END		equ        0x4F00
KEY_NPAGE	equ        0x5100
KEY_PPAGE	equ        0x4900
A_NORMAL	equ        0x7
A_REVERSE	equ	   0x70

ACS_ULCORNER	equ        '+'
ACS_URCORNER	equ        '+'
ACS_LLCORNER	equ        '+'
ACS_LRCORNER	equ        '+'
ACS_HLINE	equ        '-'
ACS_VLINE	equ        '|'
ACS_LARROW	equ        '<'
ACS_RARROW	equ        '>'
ACS_UARROW	equ        '^'
ACS_DARROW	equ        'v'

; Special graphics characters for IBM displays.
DISP_UL		equ        218
DISP_UR		equ        191
DISP_LL		equ        192
DISP_LR		equ        217
DISP_HORIZ	equ        196
DISP_VERT	equ        179
DISP_LEFT	equ        0x1b
DISP_RIGHT	equ        0x1a
DISP_UP		equ        0x18
DISP_DOWN	equ        0x19
