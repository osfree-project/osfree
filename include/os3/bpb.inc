;
;  BPB structure
;
;  (c) osFree project
;  valerius 2007, Jan 13
;

BPB struc
   jump        db     3 dup(?)
   sysid       db     8 dup(?)
   ; Standard BPB
   secsize     dw     ?
   secperclu   db     ?
   ressecs     dw     ?
   nfats       db     ?
   rootdirsize dw     ?
   nsec        dw     ?
   media       db     ?
   fatsize     dw     ?
   secpertrack dw     ?
   heads       dw     ?
   hiddensec   dd     ?
   nsec_ext    dd     ?
   ; Extended BPB
   diskno      db     ?
   logdrive    db     ?
   marker      db     ?
   volserno    dd     ?
   vollabel    db     11 dup (?)
   fsname      db     8  dup (?)
BPB ends
