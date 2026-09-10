;/*!
;   @file
;
;   @ingroup fapi
;
;   @brief Family API implementation
;
;   (c) osFree Project 2018-2022, <http://www.osFree.org>
;   for licence see licence.txt in root directory, or project website
;
;   This is Family API implementation for DOS, used with BIND tools
;   to link required API
;
;   @author Yuri Prokushev (yuri.prokushev@gmail.com)
;
;*/
;
; System Anchor Structure
;
; This is main system structure which uset to access to all system data.
; It is similar to BURGERMASTER structure of Windows system.
;

SASSTRC			struc
SAS_signature		DD	?	; 'SAS '
SAS_tables_data		DW	?	; offset to tables section
SAS_flat_sel		DW	?	; FLAT selector for kernel data
SAS_config_data		DW	?	; offset to configuration section
SAS_dd_data		DW	?	; offset to device driver section
SAS_vm_data		DW	?	; offset to Virtual Memory section
SAS_task_data		DW	?	; offset to Tasking section
SAS_RAS_data		DW	?	; offset to RAS section
SAS_file_data		DW	?	; offset to File System section
SAS_info_data		DW	?	; offset to infoseg section
SASSTRC			ends

SASRAS			struc
SAS_RAS_STDA_p		DW	?	; selector for System Trace Data Area (STDA)
SAS_RAS_STDA_r		DD	?	; segment for System Trace Data Area (STDA)
SAS_RAS_event_mask	DD	?	; offset for trace event mask
SASRAS			ends

SAS_info_section	struc
SAS_info_global		DW	?	; selector for global info seg
SAS_info_local		DD	?	; address of curtask local infoseg
SAS_info_localRM	DD	?	; address of DOS task's infoseg
SAS_info_CDIB		DW	?	; selector for Codepage Data
SAS_info_section	ends

SAS_tables_section	struc
SAS_tbl_GDT		DW	?	; selector for GDT
SAS_tbl_LDT		DW	?	; selector for LDT
SAS_tbl_IDT		DW	?	; selector for IDT
SAS_tbl_GDTPOOL		DW	?	; selector for GDTPOOL
SAS_tables_section	ends

SAS_config_section	struc
SAS_config_table	DW	?	; offset for Device Configuration Table (DevConfigTbl)
SAS_config_section	ends

SAS_dd_section		struc
SAS_dd_bimodal_chain	DW	?	; offset for the first bimodal device driver's device header
SAS_dd_real_chain	DW	?	; offset for the address of the first real mode device driver's device header
SAS_dd_DPB_segment	DW	?	; selector for Drive Parameter Block (DPB) segment
SAS_dd_CDA_anchor_p	DW	?	; selector for ABIOS protected mode Common Data Area
SAS_dd_CDA_anchor_r	DW	?	; segment for ABIOS real mode Common Data Area
SAS_dd_FSC		DW	?	; selector for FSC
SAS_dd_section		ends

