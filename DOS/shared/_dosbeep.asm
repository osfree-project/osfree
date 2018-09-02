; Code based on https://stackoverflow.com/questions/43996835/assembly-8086-dosbox-how-to-produce-beep-sound

BITS 16

ORG 100h

__start__:

 ;Setup
 call beep_setup

 ;Sample beep of ~2sec
 mov ax, 2000
 mov bx, 36
 call beep_play

 ;Wait for input
 xor ax, ax
 int 16h

 ;Tear down
 call beep_teardown

 mov ax, 4c00h
 int 21h

 ;-------------------------------------------------

 ;
 ;Setup the beep ISR
 ;

 beep_setup:
  push es
  push ax

  xor ax, ax
  mov es, ax

  ;Save the original ISR
  mov ax, WORD [es: TIMER_INT * 4]
  mov WORD [cs:original_timer_isr], ax
  mov ax, WORD [es: TIMER_INT * 4 + 2]
  mov WORD [cs:original_timer_isr + 2], ax

  ;Setup the new ISR

  cli
  mov ax, beep_isr
  mov WORD [es: TIMER_INT * 4], ax
  mov ax, cs
  mov WORD [es: TIMER_INT * 4 + 2], ax
  sti

  pop ax
  pop es
  ret 


 ;
 ;Tear down the beep ISR
 ;

 beep_teardown:
  push es
  push ax

  call beep_stop

  xor ax, ax
  mov es, ax

  ;Restore the old ISR

  cli
  mov ax, WORD [cs:original_timer_isr]
  mov WORD [es: TIMER_INT * 4], ax
  mov ax, WORD [cs:original_timer_isr + 2]
  mov WORD [es: TIMER_INT * 4 + 2], ax
  sti

  pop ax
  pop es
  ret 

 ;
 ;Beep ISR
 ;
 beep_isr:
  cmp BYTE [cs:sound_playing], 0
  je _bi_end

  cmp WORD [cs:sound_counter], 0
  je _bi_stop

  dec WORD [cs:sound_counter]

 jmp _bi_end

_bi_stop:
  call beep_stop

_bi_end:
  ;Chain
  jmp FAR [cs:original_timer_isr]

 ;
 ;Stop beep
 ;
 beep_stop:
  push ax

  ;Stop the sound

  in al, 61h
  and al, 0fch    ;Clear bit 0 (PIT to speaker) and bit 1 (Speaker enable)
  out 61h, al

  ;Disable countdown

  mov BYTE [cs:sound_playing], 0

  pop ax
  ret

 ;
 ;Beep
 ;
 ;AX = 1193180 / frequency
 ;BX = duration in 18.2th of sec
 beep_play:
  push ax
  push dx

  mov dx, ax

  mov al, 0b6h
  out 43h, al

  mov ax, dx
  out 42h, al
  mov al, ah
  out 42h, al


  ;Set the countdown
  mov WORD [cs:sound_counter], bx

  ;Start the sound

  in al, 61h
  or al, 3h    ;Set bit 0 (PIT to speaker) and bit 1 (Speaker enable)
  out 61h, al


  ;Start the countdown

  mov BYTE [cs:sound_playing], 1

  pop dx
  pop ax
  ret

 ;Keep these in the code segment
 sound_playing      db  0
 sound_counter      dw  0
 original_timer_isr     dd  0

 TIMER_INT      EQU     1ch
