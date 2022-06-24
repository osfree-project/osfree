
; 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
; services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
; (device drivers and installable file system drivers).
; Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



MWDD32_IOCTL_CATEGORY           equ 0F0h

;/*
; * Function 1 : init FSD
; * input :
; *     parm - magic number (dword)
; *     data - FLAT  address of fs32_init()
; *            16:16 pointer to FS_INIT parameters
; * output :
; *     parm - magic number               (dword)
; *     data - return code of fs32_init() (dword)
; */

MWDD32_IOCTL_FUNCTION_INIT_FSD  equ 041h
INIT_FSD_MAGIC_IN               equ 0AAAAAAAAh
INIT_FSD_MAGIC_OUT              equ 0BBBBBBBBh

mwdd32_ioctl_int_fsd_parm       struc
        magic   dd (?)                                  ; magic number for sanity checks
mwdd32_ioctl_int_fsd_parm       ends

mwdd32_ioctl_int_fsd_parm_input         struc
        fs32_init_ptr       dd (?)                      ; FLAT address of fs32_init()
        fs32_init_parms_ofs dw (?)                      ; offset  of fs32_init() parameter block pointer
        fs32_init_parms_seg dw (?)                      ; segment of fs32_init() parameter block pointer
    pTKSSBase           dd (?)
    pDevHelp32          dd (?)
mwdd32_ioctl_int_fsd_parm_input         ends

mwdd32_ioctl_int_fsd_parm_output        struc
        fs32_init_rc        dd (?)                      ; return code from fs32_init()
mwdd32_ioctl_int_fsd_parm_output        ends



;/*
; * Function 2 : init DEVICE
; * input :
; *     parm - magic number (dword)
; *     data - FLAT  address of device_init()
; *            16:16 address of INIT request packet
; *            FLAT  address of kernel _TKSSBase pointer
; *            FLAT  address of mwdd32.sys entry points structure
; * output :
; *     parm - magic number                 (dword)
; *     data - return code of device_init() (dword)
; */

MWDD32_IOCTL_FUNCTION_INIT_DEVICE  equ 042h
INIT_DEVICE_MAGIC_IN               equ 0AAAAAAAAh
INIT_DEVICE_MAGIC_OUT              equ 0BBBBBBBBh

mwdd32_ioctl_init_device_parm       struc
        b_magic   dd (?)                                    ; magic number for sanity checks
mwdd32_ioctl_init_device_parm       ends

mwdd32_ioctl_init_device_parm_input         struc
        b_device_init_ptr       dd (?)                      ; FLAT address of device_init()
        b_pReqPkt               dd (?)                      ; 16:16 address of INIT request packet
    b_pTKSSBase             dd (?)              ; FLAT address of _TKSSBase pointer
    b_pDevHelp32            dd (?)              ; FLAT address of mwdd32.sys entry points struct
mwdd32_ioctl_init_device_parm_input         ends

mwdd32_ioctl_init_device_parm_output        struc
        b_device_init_rc        dd (?)                      ; return code from device_init()
mwdd32_ioctl_init_device_parm_output        ends
