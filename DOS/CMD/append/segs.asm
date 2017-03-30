extern start :near

_TEXT segment word public 'CODE' use16

	  org 100h
          ;.code
EXE_startcode:
          jmp start
          db 80h dup(0)		; Last 128 bytes of cmdline

_TEXT ends

;_DATA segment word public 'DATA' use16
;
;	  ;section .data
;          ;.data
;EXE_startdata:
;
;_DATA ends

;_BSS  segment word public 'BSS' use16
;
;	  ;section .bss
;          ;.bss
;EXE_startbss:
;
;_BSS  ends

;STACK segment word public 'STACK' use16

;EXE_stack 512
;EXE_realstacksize equ 512
;EXE_stack_size equ EXE_bogusstacksize ; defeat EQU in EXE_end

          ;.stack         
;EXE_startstack:
;
;STACK ends

;DGROUP group _TEXT ;, _DATA, _BSS, STACK

       end EXE_startcode
