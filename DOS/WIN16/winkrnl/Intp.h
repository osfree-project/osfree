/*
	@(#)Intp.h	2.3
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*/
 
#define ADDbfrm		0x00
#define ADDwfrm		0x01
#define ADDbtrm		0x02
#define ADDwtrm		0x03
#define ADDbia		0x04
#define ADDwia		0x05
#define PUSHes		0x06
#define POPes		0x07
#define ORbfrm		0x08
#define ORwfrm		0x09
#define ORbtrm		0x0a
#define ORwtrm		0x0b
#define ORbi		0x0c
#define ORwi		0x0d
#define PUSHcs		0x0e

#define TWO_BYTE_ESC	0x0f
#define VER		0xf00
#define LAR		0xf02
#define LSL		0xf03

#define ADCbfrm		0x10
#define ADCwfrm		0x11
#define ADCbtrm		0x12
#define ADCwtrm		0x13
#define ADCbi		0x14
#define ADCwi		0x15
#define PUSHss		0x16
#define POPss		0x17
#define SBBbfrm		0x18
#define SBBwfrm		0x19
#define SBBbtrm		0x1a
#define SBBwtrm		0x1b
#define SBBbi		0x1c
#define SBBwi		0x1d
#define PUSHds		0x1e
#define POPds		0x1f

#define ANDbfrm		0x20
#define ANDwfrm		0x21
#define ANDbtrm		0x22
#define ANDwtrm		0x23
#define ANDbi		0x24
#define ANDwi		0x25
#define SEGes		0x26
#define DAA		0x27
#define SUBbfrm		0x28
#define SUBwfrm		0x29
#define SUBbtrm		0x2a
#define SUBwtrm		0x2b
#define SUBbi		0x2c
#define SUBwi		0x2d
#define SEGcs		0x2e
#define DAS		0x2f

#define XORbfrm		0x30
#define XORwfrm		0x31
#define XORbtrm		0x32
#define XORwtrm		0x33
#define XORbi		0x34
#define XORwi		0x35
#define SEGss		0x36
#define AAA		0x37
#define CMPbfrm		0x38
#define CMPwfrm		0x39
#define CMPbtrm		0x3a
#define CMPwtrm		0x3b
#define CMPbi		0x3c
#define CMPwi		0x3d
#define SEGds		0x3e
#define AAS		0x3f

#define INCax		0x40
#define INCcx		0x41
#define INCdx		0x42
#define INCbx		0x43
#define INCsp		0x44
#define INCbp		0x45
#define INCsi		0x46
#define INCdi		0x47
#define DECax		0x48
#define DECcx		0x49
#define DECdx		0x4a
#define DECbx		0x4b
#define DECsp		0x4c
#define DECbp		0x4d
#define DECsi		0x4e
#define DECdi		0x4f

#define PUSHax		0x50
#define PUSHcx		0x51
#define PUSHdx		0x52
#define PUSHbx		0x53
#define PUSHsp		0x54
#define PUSHbp		0x55
#define PUSHsi		0x56
#define PUSHdi		0x57
#define POPax		0x58
#define POPcx		0x59
#define POPdx		0x5a
#define POPbx		0x5b
#define POPsp		0x5c
#define POPbp		0x5d
#define POPsi		0x5e
#define POPdi		0x5f

#define PUSHA		0x60
#define POPA		0x61
/*#define 0x62		0x62
 *#define 0x63		0x63
 *#define 0x64		0x64
 *#define 0x65		0x65
 *#define 0x66		0x66
 *#define 0x67		0x67 */
#define PUSHwi		0x68
#define IMULwrm		0x69 
#define PUSHbi		0x6a
#define IMULbrm		0x6b
/*#define 0x6c		0x6c
 *#define 0x6d		0x6d
 *#define 0x6e		0x6e
 *#define 0x6f		0x6f
 */
#define JO		0x70
#define JNO		0x71
#define JB_JNAE		0x72
#define JNB_JAE		0x73
#define JE_JZ		0x74
#define JNE_JNZ		0x75
#define JBE_JNA		0x76
#define JNBE_JA		0x77
#define JS		0x78
#define JNS		0x79
#define JP_JPE		0x7a
#define JNP_JPO		0x7b
#define JL_JNGE		0x7c
#define JNL_JGE		0x7d
#define JLE_JNG		0x7e
#define JNLE_JG		0x7f

#define IMMEDbrm	0x80
#define IMMEDwrm	0x81
#define IMMEDbrm2	0x82
#define IMMEDisrm	0x83
#define TESTbrm		0x84
#define TESTwrm		0x85
#define XCHGbrm		0x86
#define XCHGwrm		0x87
#define MOVbfrm		0x88
#define MOVwfrm		0x89
#define MOVbtrm		0x8a
#define MOVwtrm		0x8b
#define MOVsrtrm	0x8c
#define LEA			0x8d
#define MOVsrfrm	0x8e
#define POPrm		0x8f

#define NOP		0x90
#define XCHGcx		0x91
#define XCHGdx		0x92
#define XCHGbx		0x93
#define XCHGsp		0x94
#define XCHGbp		0x95
#define XCHGsi		0x96
#define XCHGdi		0x97
#define CBW		0x98
#define CWD		0x99
#define CALLld		0x9a
#define WAIT		0x9b
#define PUSHF		0x9c
#define POPF		0x9d
#define SAHF		0x9e
#define LAHF		0x9f

#define MOVmal		0xa0
#define MOVmax		0xa1
#define MOValm		0xa2
#define MOVaxm		0xa3
#define MOVSb		0xa4
#define MOVSw		0xa5
#define CMPSb		0xa6
#define CMPSw		0xa7
#define	TESTbi		0xa8
#define TESTwi		0xa9
#define STOSb		0xaa
#define STOSw		0xab
#define LODSb		0xac
#define LODSw		0xad
#define SCASb		0xae
#define SCASw		0xaf

#define MOVial		0xb0
#define MOVicl		0xb1
#define MOVidl		0xb2
#define MOVibl		0xb3
#define MOViah		0xb4
#define MOVich		0xb5
#define MOVidh		0xb6
#define MOVibh		0xb7
#define MOViax		0xb8
#define MOVicx		0xb9
#define MOVidx		0xba
#define MOVibx		0xbb
#define MOVisp		0xbc
#define MOVibp		0xbd
#define MOVisi		0xbe
#define MOVidi		0xbf

#define SHIFTbi		0xc0
#define SHIFTwi		0xc1 
#define RETisp		0xc2
#define RET		0xc3
#define LES		0xc4
#define LDS		0xc5
#define MOVbirm		0xc6
#define MOVwirm		0xc7
#define ENTER		0xc8
#define LEAVE		0xc9 
#define RETlisp		0xca
#define RETl		0xcb
#define INT3		0xcc
#define INT		0xcd
#define INTO		0xce
#define IRET		0xcf

#define SHIFTb		0xd0
#define SHIFTw		0xd1
#define SHIFTbv		0xd2
#define SHIFTwv		0xd3
#define AAM		0xd4
#define AAD		0xd5
/*#define 0xd6		0xd6 */
#define XLAT		0xd7
#define ESC0		0xd8
#define ESC1		0xd9
#define ESC2		0xda
#define ESC3		0xdb
#define ESC4		0xdc
#define ESC5		0xdd
#define ESC6		0xde
#define ESC7		0xdf

#define LOOPNZ_LOOPNE	0xe0
#define LOOPZ_LOOPE	0xe1
#define LOOP		0xe2
#define JCXZ		0xe3
#define INb			0xe4
#define INw			0xe5
#define OUTb		0xe6
#define OUTw		0xe7
#define CALLd		0xe8
#define JMPd		0xe9
#define JMPld		0xea
#define JMPsid		0xeb
#define INvb		0xec
#define INvw		0xed
#define OUTvb		0xee
#define OUTvw		0xef

#define LOCK		0xf0
#define REPNE		0xf2
#define REP		0xf3
#define HLT		0xf4
#define CMC		0xf5
#define GRPlbrm 	0xf6
#define GRPlwrm 	0xf7
#define CLC		0xf8
#define STC		0xf9
#define CLI		0xfa
#define STI		0xfb
#define CLD		0xfc
#define STD		0xfd
#define GRP2brm		0xfe
#define GRP2wrm		0xff

#ifdef FP87 /* [ */
#define FADDm32r_sti	((ESC0<<3) | (0x0 & 0x38) >> 3)
#define FMULm32r_sti	((ESC0<<3) | (0x8 & 0x38) >> 3)
#define FCOMm32r_sti	((ESC0<<3) | (0x10 /*or 0xd0*/ & 0x38) >> 3)
#define FCOMPm32r_sti	((ESC0<<3) | (0x18 /*or 0xd8*/ & 0x38) >> 3)
#define FSUBm32r_sti	((ESC0<<3) | (0x20 /*or 0xe0*/ & 0x38) >> 3)
#define FSUBRm32r_sti	((ESC0<<3) | (0x28 /*or 0xe8*/ & 0x38) >> 3)
#define FDIVm32r_sti	((ESC0<<3) | (0x30 /*or 0xf0*/ & 0x38) >> 3)
#define FDIVRm32r_sti	((ESC0<<3) | (0x38 /*or 0xf8*/ & 0x38) >> 3)
#define FLDm32r_sti  	((ESC1<<3) | (0x0 & 0x38) >> 3)
#define FXCH        	((ESC1<<3) | (0xc8 & 0x38) >> 3)
#define FSTm32r_FNOP	((ESC1<<3) | (0x10 /*or 0xd0*/ & 0x38) >> 3)
#define FSTPm32r    	((ESC1<<3) | (0x18 & 0x38) >> 3)
#define FD9SLASH4 		((ESC1<<3) | (0x20 /*or 0xe0-0xe5*/ & 0x38) >> 3)
#define    FLDENV 		0x20
#define    FCHS 		0xe0
#define    FABS 		0xe1
#define    FTST 		0xe4
#define    FXAM 		0xe5
#define FLDCONST		((ESC1<<3) | (0xe8 /*or 0xe9-0xee*/ & 0x38) >> 3)
#define    FLDCW		0x28
#define    FLD1 		0xe8
#define    FLDL2T		0xe9
#define    FLDL2E		0xea
#define    FLDPI		0xeb
#define    FLDLG2		0xec
#define    FLDLN2		0xed
#define    FLDZ 		0xee
#define FD9SLASH6		((ESC1<<3) | (0x30 /*or 0xf0-0xf7*/ & 0x38) >> 3)
#define    F2XM1 		0xf0
#define    FYL2X 		0xf1
#define    FPTAN 		0xf2
#define    FPATAN 		0xf3
#define    FXTRACT 		0xf4
#define    FPREM1 		0xf5
#define    FDECSTP		0xf6
#define    FINCSTP		0xf7
#define FD9SLASH7		((ESC1<<3) | (0x38 /*or 0xf8-0xff*/ & 0x38) >> 3)
#define    FPREM 		0xf8
#define    FYL2XP1 		0xf9
#define    FSQRT 		0xfa
#define    FSINCOS 		0xfb
#define    FRNDINT 		0xfc
#define    FSCALE 		0xfd
#define    FSIN			0xfe
#define    FCOS			0xff
#define FADDm32i		((ESC2<<3) | (0x0 & 0x38) >> 3)
#define FMULm32i		((ESC2<<3) | (0x8 & 0x38) >> 3)
#define FICOMm32i		((ESC2<<3) | (0x10 & 0x38) >> 3)
#define FICOMPm32i		((ESC2<<3) | (0x18 & 0x38) >> 3)
#define FISUBm32i		((ESC2<<3) | (0x20 & 0x38) >> 3)
#define FISUBRm32i_FUCOMPPst1	((ESC2<<3) | (0x28 /*or 0xe9*/ & 0x38) >> 3)
#define FIDIVm32i		((ESC2<<3) | (0x30 & 0x38) >> 3)
#define FIDIVRm32i		((ESC2<<3) | (0x38 & 0x38) >> 3)
#define FILDm32i		((ESC3<<3) | (0x0 & 0x38) >> 3)
#define FISTm32i		((ESC3<<3) | (0x10 & 0x38) >> 3)
#define FISTPm32i		((ESC3<<3) | (0x18 & 0x38) >> 3)
#define FRSTORm94B_FINIT_FCLEX	((ESC3<<3) | (0x20 /*or 0xe3-2*/ & 0x38) >> 3)
#define FLDm80r  		((ESC3<<3) | (0x28 & 0x38) >> 3)
#define FSTPm80r  		((ESC3<<3) | (0x38 & 0x38) >> 3)
#define FADDm64r_tosti	((ESC4<<3) | (0x0 & 0x38) >> 3)
#define FMULm64r_tosti	((ESC4<<3) | (0x8 & 0x38) >> 3)
#define FCOMm64r     	((ESC4<<3) | (0x10 & 0x38) >> 3)
#define FCOMPm64r    	((ESC4<<3) | (0x18 & 0x38) >> 3)
#define FSUBm64r_FSUBRfromsti ((ESC4<<3) | (0x20 /*or 0xe0*/ & 0x38) >> 3)
#define FSUBRm64r_FSUBfromsti ((ESC4<<3) | (0x28 /*or 0xe8*/ & 0x38) >> 3)
#define FDIVm64r_FDIVRtosti   ((ESC4<<3) | (0x30 /*or 0xf0*/ & 0x38) >> 3)
#define FDIVRm64r_FDIVtosti   ((ESC4<<3) | (0x38 /*or 0xf8*/ & 0x38) >> 3)
#define FLDm64r_FFREE         ((ESC5<<3) | (0x0 /*or 0xc0*/ & 0x38) >> 3)
#define FSTm64r_sti           ((ESC5<<3) | (0x10 /*or 0xd0+i*/ & 0x38) >> 3)
#define FSTPm64r_sti          ((ESC5<<3) | (0x18 /*or 0xd8+i*/ & 0x38) >> 3)
#define FUCOMsti              ((ESC5<<3) | (0x20 /*or 0xe0+i*/ & 0x38) >> 3)
#define FUCOMPsti             ((ESC5<<3) | (0x28 /*or 0xe8+i*/ & 0x38) >> 3)
#define FSAVEm94B             ((ESC5<<3) | (0x30 & 0x38) >> 3)
#define FSTSWm16i             ((ESC5<<3) | (0x38 & 0x38) >> 3)
#define FADDm16i_tostipop     ((ESC6<<3) | (0x0 & 0x38) >> 3)
#define FMULm16i_tostipop     ((ESC6<<3) | (0x8 & 0x38) >> 3)
#define FICOMm16i             ((ESC6<<3) | (0x10 & 0x38) >> 3)
#define FICOMPm16i_FCOMPPst1  ((ESC6<<3) | (0x18 /*or 0xd9*/ & 0x38) >> 3)
#define FISUBm16i_FSUBRPfromsti ((ESC6<<3) | (0x20 /*or 0xe0*/ & 0x38) >> 3)
#define FISUBRm16i_FSUBPfromsti ((ESC6<<3) | (0x28 /*or 0xe8*/ & 0x38) >> 3)
#define FIDIVm16i_FDIVRPtosti ((ESC6<<3) | (0x30 /*or 0xf0*/ & 0x38) >> 3)
#define FIDIVRm16i_FDIVPtosti ((ESC6<<3) | (0x38 /*or 0xf8*/ & 0x38) >> 3)
#define FILDm16i		((ESC7<<3) | (0x0 & 0x38) >> 3)
#define FISTm16i		((ESC7<<3) | (0x10 & 0x38) >> 3)
#define FISTPm16i		((ESC7<<3) | (0x18 & 0x38) >> 3)
#define FBLDm80dec_FSTSWax		((ESC7<<3) | (0x20 /*or 0xe0*/ & 0x38) >> 3)
#define FILDm64i		((ESC7<<3) | (0x28 & 0x38) >> 3)
#define FBSTPm80dec		((ESC7<<3) | (0x30 & 0x38) >> 3)
#define FISTPm64i		((ESC7<<3) | (0x38 & 0x38) >> 3)
#endif	/* ] */

#define MOD(x)		(x >> 6)
#define RG(x)		((WORD)(x & 0x38) >> 3)
#define RM(x)		(x & 0x7)
#define MOD_MEM		(mod != 3)
#define MOD_REG		(mod == 3)

#define CARRY_FLAG		0x0001
#define PARITY_FLAG		0x0004
#define AUX_CARRY_FLAG		0x0010
#define ZERO_FLAG		0x0040
#define SIGN_FLAG		0x0080
#define TRAP_FLAG		0x0100
#define INTERRUPT_FLAG		0x0200
#define DIRECTION_FLAG		0x0400
#define OVERFLOW_FLAG		0x0800

#define IS_CF_SET (BOOL)(flags & CARRY_FLAG)
#define IS_PF_SET (BOOL)((WORD)(flags & PARITY_FLAG) >> 2)
#define IS_AF_SET (BOOL)((WORD)(flags & AUX_CARRY_FLAG) >> 4)
#define IS_ZF_SET (BOOL)((WORD)(flags & ZERO_FLAG) >> 6)
#define IS_SF_SET (BOOL)((WORD)(flags & SIGN_FLAG) >> 7)
#define IS_TF_SET (BOOL)((WORD)(flags & TRAP_FLAG) >> 8)
#define IS_IF_SET (BOOL)((WORD)(flags & INTERRUPT_FLAG) >> 9)
#define IS_DF_SET (BOOL)((WORD)(flags & DIRECTION_FLAG) >> 10)
#define IS_OF_SET (BOOL)((WORD)(flags & OVERFLOW_FLAG) >> 11)

#define CLEAR_CF flags &= (~ CARRY_FLAG)
#define CLEAR_PF flags &= (~ PARITY_FLAG)
#define CLEAR_AF flags &= (~ AUX_CARRY_FLAG)
#define CLEAR_ZF flags &= (~ ZERO_FLAG)
#define CLEAR_SF flags &= (~ SIGN_FLAG)
#define CLEAR_TF flags &= (~ TRAP_FLAG)
#define CLEAR_IF flags &= (~ INTERRUPT_FLAG)
#define CLEAR_DF flags &= (~ DIRECTION_FLAG)
#define CLEAR_OF flags &= (~ OVERFLOW_FLAG)
#define CLEAR_CF_AF flags &= (~(CARRY_FLAG | AUX_CARRY_FLAG))

#define SET_CF flags |= CARRY_FLAG
#define SET_PF flags |= PARITY_FLAG
#define SET_AF flags |= AUX_CARRY_FLAG
#define SET_ZF flags |= ZERO_FLAG
#define SET_SF flags |= SIGN_FLAG
#define SET_TF flags |= TRAP_FLAG
#define SET_IF flags |= INTERRUPT_FLAG
#define SET_DF flags |= DIRECTION_FLAG
#define SET_OF flags |= OVERFLOW_FLAG
#define SET_CF_AF flags |= (CARRY_FLAG | AUX_CARRY_FLAG)

#define ROL	0
#define ROR	1
#define RCL	2
#define RCR	3
#define SHL	4
#define SHR	5
#define SAR	7

#define ADD	0
#define OR	1
#define ADC	2
#define SBB	3
#define AND	4
#define SUB	5
#define XOR	6
#define CMP	7

#define INC	0
#define DEC	1

#define TEST	0
#define NOT	2
#define NEG	3
#define MUL	4
#define IMUL	5
#define DIV	6
#define IDIV	7

#define AL r.h.al
#define AH r.h.ah
#define AX r.x.ax
#define BL r.h.bl
#define BH r.h.bh
#define BX r.x.bx
#define CL r.h.cl
#define CH r.h.ch
#define CX r.x.cx
#define DL r.h.dl
#define DH r.h.dh
#define DX r.x.dx
#define SP r.x.sp
#define BP r.x.bp
#define SI r.x.si
#define DI r.x.di
#define ES (((es - MEM_base) >> 4) & 0xffff)
#define CS (((cs - MEM_base) >> 4) & 0xffff)
#define SS (((ss - MEM_base) >> 4) & 0xffff)
#define DS (((ds - MEM_base) >> 4) & 0xffff)

#define REGB(reg)	r.regh[regs_map[reg]]
#define REGW(reg)	r.regx[reg]

#define RAX     0
#define RCX     1
#define RDX     2
#define RBX     3

#ifndef X386 
#define RAH     0
#define RAL     1
#define RCH     2
#define rCL     3
#define RDH     4
#define RDL     5
#define RBH     6
#define RBL     7
#else
#define RAH     1
#define RAL     0
#define RCH     3
#define rCL     2
#define RDH     5
#define RDL     4
#define RBH     7
#define RBL     6
#endif

#define SIGNb (temp & 0x80)
#define SIGNw ((temp & 0x8000) >> 8)
#define CARRYb ((temp >> 8) & 0x1)
#define CARRYw ((temp >> 16) & 0x1)
#define ZEROb ((LOBYTE(temp) == 0) ? 0x40 : 0)
#define ZEROw ((LOWORD(temp) == 0) ? 0x40 : 0)
#define AUXCARRY 0
#define OVFb(x)		((x & 0x180) >> 7)
#define OVFw(x)		((x & 0x18000) >> 15)
#define OVFLb ((((x = OVFb(temp)) == 1) || (x == 2)) ? 0x800 : 0)
#define OVFLw ((((x = OVFw(temp)) == 1) || (x == 2)) ? 0x800 : 0)
#define OVFLOWb(t) (((!(temp ^ t)) & (!((temp - t) ^ t)) << 4) & 0x800)
#define OVFLOWw(t) (((!(temp ^ t)) & (!((temp - t) ^ t)) >> 4) & 0x800)
#define PARITY (parity [LOBYTE(temp)])

#define AT_EXECUTE	1
#define AT_READ		2
#define AT_WRITE	3

#define ERRORLOG(s,o,m,r) fprintf(stderr,"Error accessing selector %x, offset %x, mode %x, errorcode %x\n",s,o,m,r)

#define UINT_86		2
#define INT_86		2
#define LP_86		4

#define	SIGNED		signed
#define JUMP(lp)	lp += (SIGNED char)(*lp); 

#define SET_SEGREG(lp,sel) { \
	WORD wFlags; \
	char prnt_buf[40]; \
	if ((sel >> 3) == 0) \
	    lp = 0; \
	else { \
	    wFlags = GetSelectorFlags(sel); \
	    if (!(wFlags & DF_PRESENT)) { \
	        if (!LoadSegment(sel)) { \
		    sprintf(prnt_buf, \
		        "INTP32: failed to load selector %x\n",sel); \
		    printf("%s\n",prnt_buf); \
		    FatalAppExit(0,prnt_buf); \
	        } \
	    } \
	    lp = GetPhysicalAddress(sel); \
	} \
}

#define FillEnv {env->reg.ax = AX; \
	env->reg.bx = BX; \
	env->reg.cx = CX; \
	env->reg.dx = DX; \
	env->reg.sp = (REGISTER)(ss + SP); \
	env->reg.bp = (REGISTER)(ss + BP); \
	env->reg.si = SI; \
	env->reg.di = DI; \
	env->reg.ss = _ss; \
	env->reg.ds = _ds; \
	env->reg.es = _es; \
	env->trans_addr = (BINADDR)MAKELONG(_ip,_sel); \
	env->return_addr = (BINADDR)MAKELONG(GETWORD(&ss[SP]),GETWORD(&ss[SP+2])); \
	env->reg.flags = flags; }

#define GetEnv {AX = env->reg.ax; \
	BX = env->reg.bx; \
	CX = env->reg.cx; \
	DX = env->reg.dx; \
	SI = env->reg.si; \
	DI = env->reg.di; \
	_ss = env->reg.ss; \
	ss = GetPhysicalAddress(_ss); \
	_ds = env->reg.ds; \
	ds = GetPhysicalAddress(_ds); \
	_es = env->reg.es; \
	es = GetPhysicalAddress(_es); \
	SP = (WORD)(env->reg.sp-(DWORD)ss); \
	BP = (WORD)(env->reg.bp-(DWORD)ss); \
	if (env->active) {	\
	    _cs = HIWORD(env->return_addr); \
	    cs = GetPhysicalAddress(_cs); \
	    ip = LOWORD(env->return_addr); \
	    env->active = 0L; \
	} \
	flags = (WORD)env->reg.flags; }

#define PUSHWORD(w) {SP -= 2; PUTWORD(&ss[SP],w);}

#define POPWORD(w) {w = GETWORD(&ss[SP]);SP +=2;}

struct _WORDREGS {
	unsigned short ax;
	unsigned short cx;
	unsigned short dx;
	unsigned short bx;
	unsigned short sp;
	unsigned short bp;
	unsigned short si;
	unsigned short di;
	};

#ifndef X386
struct _BYTEREGS {
	unsigned char ah,al;
	unsigned char ch,cl;
	unsigned char dh,dl;
	unsigned char bh,bl;
	unsigned char sph,spl;
	unsigned char bph,bpl;
	unsigned char sih,sil;
	unsigned char dih,dil;
	};
#else
struct _BYTEREGS {
	unsigned char al,ah;
	unsigned char cl,ch;
	unsigned char dl,dh;
	unsigned char bl,bh;
	unsigned char spl,sph;
	unsigned char bpl,bph;
	unsigned char sil,sih;
	unsigned char dil,dih;
	};
#endif

union _REGS {
	struct _WORDREGS x;
	struct _BYTEREGS h;
	unsigned short regx[4];
	unsigned char regh[8];
	};

