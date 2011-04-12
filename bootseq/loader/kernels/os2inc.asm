;
; os2inc.asm:
; include binary files
;

public rel_start
public ufsd_start
public ufsd_size
public mfsd_start
public mfsd_size

.386p

_TEXT    segment dword public 'CODE'  use32

ufsd_start:
; here uFSD image begins
include ufsd.inc
; here it ends
; its size
ufsd_size          dd $ - ufsd_start

mfsd_start:
; here mFSD image begins
include mfsd.inc
; here it ends
; its size
mfsd_size          dd $ - mfsd_start

rel_start:
; here uFSD reloc. info begins
include urel.inc
; here it ends
; its size
rel_size           dd $ - rel_start

_TEXT    ends

         end
