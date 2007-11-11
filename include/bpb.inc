;
;  BPB structure
;
;  (c) osFree project
;  valerius 2007, Jan 13
;

;BPB struc
;   jump        db     3 dup(?)
;   sysid       db     8 dup(?)
;   ; Standard BPB
;   secsize     dw     ?
;   secperclu   db     ?
;   ressecs     dw     ?
;   nfats       db     ?
;   rootdirsize dw     ?
;   nsec        dw     ?
;   media       db     ?
;   fatsize     dw     ?
;   secpertrack dw     ?
;   heads       dw     ?
;   hiddensec   dd     ?
;   nsec_ext    dd     ?
;   ; Extended BPB
;   diskno      db     ?
;   logdrive    db     ?
;   marker      db     ?
;   volserno    dd     ?
;   vollabel    db     11 dup (?)
;   fsname      db     8  dup (?)
;BPB ends

bios_parameters_block struc
; Standard BPB                                              ;
sect_size        dw          ?                              ; Sector size in bytes
clus_size        db          ?                              ; Cluster size in sectors
res_sectors      dw          ?                              ; Reserved sectors
n_fats           db          ?                              ; The number of FAT copies
root_dir_size    dw          ?                              ; Max. no of root directory entries
n_sect           dw          ?                              ; No. of sectors of disk, if its size
                                                            ; is less then 32M
media_desc       db          ?                              ; Media descriptor byte
fat_size         dw          ?                              ; FAT size in sectors
track_size       dw          ?                              ; Number of sectors in track
heads_cnt        dw          ?                              ; Number of surfaces
hidden_secs      dd          ?                              ; Number of hidden sectors
n_sect_ext       dd          ?                              ; Number of sectors on disk, if its
                                                            ; size exceeds 32M
; Extended BPB

disk_num         db          ?                              ; 00 -- floppy; 80 -- hard drive
log_drive        db          ?                              ; if (logDrive & 80h) = true, (logDrive & 7Fh) is
                                                            ; the number of logical disk (c: is 00)
marker           db          ?                              ;
vol_ser_no       dd          ?                              ; Volume serial number
vol_label        db          11 dup (?)                     ; Volume label
fs_name          db          8  dup (?)                     ; File system name
bios_parameters_block ends
