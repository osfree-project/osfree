;
; Disk read routines
;

.286

.model compact

BootSeg segment at 7c0h
                 db          1e0h    dup (?)
ReadRunFAddr     dw          ?                                       ; ReadRunF function address
BootSeg ends

public read_run
public BIOS_READ_HD

_TEXT segment para public 'CODE' USE16

;
; read_run:
;
; Callable from C code.
; Reads contiguous run of sectors, can read data bigger
; than 64Kb at a time. Is a wrapper around ReadRun from bootsector.
;
; Arguments:
;
;            read_run(unsigned long start,
;                     unsigned short len,
;                     void far *buf);
;            where
;                  start is the offset (in sectors, not bytes) of starting
;                        sector from the beginning of partition.
;                  len   is the length of data in sectors
;                  buf   is a far pointer to a buffer to read data in
;

read_run proc near
           push bp
           mov  bp, sp

           push es
           pusha

           mov  ax, [bp + 2]
           mov  dx, [bp + 4]
           mov  cx, [bp + 6]

           mov  di, [bp + 8]
           mov  bx, [bp + 10]
           mov  es, bx

readloop:




           popa
           pop  es

           mov  sp, bp
           pop  bp

           ret

read_run endp

; int _pascal bios_read_hd(
;                          unsigned short dev,          /* bp + 16 */
;                          unsigned short head,         /* bp + 14 */
;                          unsigned short cyl,          /* bp + 12 */
;                          unsigned short sect,         /* bp + 10 */
;                          unsigned short nbsect,       /* bp + 8  */
;                          unsigned long  buffer        /* bp + 4  */
;                        );
BIOS_READ_HD proc near
        enter 0, 0
        push ds
        push es
        push si
        push di
        push bx

        mov dl, [bp+16]  ; dev
        mov dh, [bp+14]  ; head

        mov ax, [bp+12]  ; Cyl.
        mov ch, al
        mov cl, ah
        shl cl, 6
        or cl, [bp+10]  ; sect.

        les bx, [bp+4]  ; buf

        mov ah, 02h     ; read disk
        mov al, [bp+8]  ; nb sect.
        int 13h
        jc  short not_ok
        mov ax, 0
not_ok:
        shr ax, 8

        pop bx
        pop di
        pop si
        pop es
        pop ds
        leave
        ret 14
BIOS_READ_HD endp


_TEXT ends

      end
      