;
; bpb.inc: data structures
;

bios_parameters_block struc
; Standard BPB                                              ;
sect_size        dw          200h                           ; Sector size in bytes
clus_size        db          10h                            ; Cluster size in sectors
res_sectors      dw          1                              ; Reserved sectors
n_fats           db          2                              ; The number of FAT copies
root_dir_size    dw          200h                           ; Max. no of root directory entries
n_sect           dw          0                              ; No. of sectors of disk, if its size
                                                            ; is less then 32M
media_desc       db          0f8h                           ; Media descriptor byte
fat_size         dw          0xfc                           ; FAT size in sectors
track_size       dw          0x3f                           ; Number of sectors in track
heads_cnt        dw          0xff                           ; Number of surfaces
hidden_secs      dd          0x3f                           ; Number of hidden sectors
n_sect_ext       dd          0x0fbfc1                       ; Number of sectors on disk, if its
                                                            ; size exceeds 32M
; Extended BPB

disk_num         db          0x80                           ; 00 -- floppy; 80 -- hard drive
log_drive        db          0x83                              ; if (logDrive & 80h) = true, (logDrive & 7Fh) is
                                                            ; the number of logical disk (c: is 00)
marker           db          29h                            ;
vol_ser_no       dd          98db2815h                      ; Volume serial number
vol_label        db          'NO NAME    '                  ; Volume label
fs_name          db          'FAT     '                     ; File system name
bios_parameters_block ends

disk_addr_packet struc
pkt_size         db          ?
reserved         db          ?
num_blocks       dw          ?
buffer           dd          ?
starting_block   dq          ?
disk_addr_packet ends

ext_params struc
;
; bp register points here
;
force_chs        db               ?
drive            db               ?
cluster_base     dd               ?
disk_addr_pkt    disk_addr_packet <>
;
; bootsector boundary
; (at address 0x0:0x7c00)
;
jump             dw               ?
force_lba        db               ?
oemid            db               8 dup (?)
bpb              bios_parameters_block <>
ext_params ends

EXT_PARAMS_SIZE  equ (size ext_params - size bios_parameters_block - 11)

fat_directory struc
dir_name         db          11 dup (?)     ; name and extension
dir_attr         db          ?              ; attributes
dir_nt_res       db          ?              ; reserved for WinNT
dir_crt_time_ms  db          ?              ; millisecond stamp of creation time
dir_crt_time     dw          ?              ; creation time
dir_crt_date     dw          ?              ; creation date
dir_lst_acc_date dw          ?              ; last access date
dir_fst_clus_hi  dw          ?              ; 1st cluster hi word, = 0 for FAT12 and FAT16
dir_wrt_time     dw          ?              ; last write time
dir_wrt_date     dw          ?              ; last write date
dir_fst_clus_lo  dw          ?              ; 1st cluster lo word
dir_file_size    dd          ?              ; size in bytes
fat_directory ends

; attributes:
ATTR_READ_ONLY   equ 01h
ATTR_HIDDEN      equ 02h
ATTR_SYSTEM      equ 04h
ATTR_VOLUME_ID   equ 08h
ATTR_DIRECTORY   equ 10h
ATTR_ARCHIVE     equ 20h
ATTR_LONG_NAME   equ 0fh
