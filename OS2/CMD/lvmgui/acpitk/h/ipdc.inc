; ipdc.inc -- keep this file synced with ipdc.h
;
; Inter PSD Device communication module
;
; Copyright (c) 2012 Mensys, BV
;
; Author: David Azarewicz <david@88watts.net>
;
; All Rights Reserved

ACPI_IOCTL_CATEGORY         equ 88h

; Special function
READ_WRITE_EMBEDDED         equ 80h
; Public & export function
EVALUATE_FUNCTION           equ 81h
GETSYSTEMIFO_FUNCTION       equ 82h
GETTABLEHEADER_FUNCTION     equ 83h
GETTABLE_FUNCTION           equ 84h
WALKNAMESPACE_FUNCTION      equ 85h
GETNAME_FUNCTION            equ 86h
GETOBJECTINFO_FUNCTION      equ 87h
RSGETMETHODDATA_FUNCTION    equ 88h
GETTYPE_FUNCTION            equ 89h
GETPARENT_FUNCTION          equ 8Ah
GETNEXTOBJECT_FUNCTION      equ 8Bh
WAIT_BUTTON                 equ 8Ch
GET_DRIVER_VERSION          equ 8Dh
ACPI_ENTER_SLEEP            equ 8Eh
GETTIMER_FUNCTION           equ 8Fh
GETTIMERRESOLUTION_FUNCTION equ 90h
PMTIMER_UPTIME_FUNCTION     equ 91h
SET_THROTTLING_C_STATE      equ 92h
GET_THROTTLING              equ 93h
GET_STATISTICS              equ 94h
SET_CPU_POWER_STATE         equ 95h
ENTER_EVENT_THREAD_FUNCTION equ 96h
WAIT_NOTIFY_FUNCTION        equ 97h
WAIT_EMBEDDED_FUNCTION      equ 98h
FIND_PCI_DEVICE             equ 99h
GETHANDLE_FUNCTION          equ 9Ah
ACPI_INTERNAL_FUNCTION      equ 9Bh
READ_WRITE_PCI              equ 9Ch
NOTIFY_DEVICE_DRIVERS       equ 9Dh
MEMORY_TYPE_SYNC            equ 9Eh
GET_EXPORTED_FUNCTION_TABLE equ 9Fh
; Function numbers above 0x00ff can only be called via the 32 bit interface
; Set event flag
SET_POWERBUTTON_FLAG        equ 100h
SET_SLEEPBUTTON_FLAG        equ 101h
; Function for drivers only
GET_FUNCTION_ADDRESS        equ 211h
REGISTERED_NOTIFY           equ 213h


