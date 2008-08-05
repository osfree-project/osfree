;
; struc.inc:
; structure definitions
;

;
; segment descriptor structure
;
desc struc
ds_limit          dw   ?      ; low two bytes of limit
ds_baselo         dw   ?
ds_basehi1        db   ?
ds_acclo          db   ?
ds_acchi          db   ?      ; low byte contains high nibble of limit
ds_basehi2        db   ?
desc ends

;
; GDTR descriptor
;
gdtr struc
g_limit dw    ?                              ; limit
g_base  dd    ?                              ; address
gdtr ends
