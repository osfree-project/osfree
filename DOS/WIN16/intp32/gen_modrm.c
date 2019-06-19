/*    
	gen_modrm.c	1.3
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

#include <stdio.h>

char *wreg[] = {"AX","CX","DX","BX","SP","BP","SI","DI"};
char *breg[] = {"AL","CL","DL","BL","AH","CH","DH","BH"};
char *rm[] = {"BXSI","BXDI","BPSI","BPDI","SI","DI","imm16","BX"};
char *brm[] = {"BXSIimm8","BXDIimm8","BPSIimm8","BPDIimm8","SIimm8","DIimm8","BPimm8","BXimm8"};
char *wrm[] = {"BXSIimm16","BXDIimm16","BPSIimm16","BPDIimm16","SIimm16","DIimm16","BPimm16","BXimm16"};

main()
{
	int modrm, ret, is_reg;

	printf("#include \"hsw_interp.h\"\n");
	printf("#include \"mod_rm.h\"\n\n");
	printf("int\n");
	printf("hsw_modrm_sib");
	printf("(Interp_ENV *env, int sib)\n{\n");
	printf("\tswitch(sib) {\n");
	for (modrm = 0; modrm < 256; modrm++) {
		if ((modrm & 0x38) == 0x20) /* no index register */
			printf("\t\tcase %d: return (E%s);\n",modrm,wreg[modrm&7]);
		else if (modrm >> 6)
			printf("\t\tcase %d: return (E%s + (E%s << %d));\n",
				modrm, wreg[modrm&7], wreg[(modrm>>3)&7], modrm>>6);
		else 
			printf("\t\tcase %d: return (E%s + E%s);\n",
				modrm, wreg[modrm&7], wreg[(modrm>>3)&7]);
	}
	printf("\t}\n}\n");

	printf("\n\nint\n");
	printf("hsw_modrm_16_byte");
	printf("(Interp_ENV *env, unsigned char *PC, Interp_VAR *interp_var)\n{\n");
	printf("\tswitch(*(PC+1)) {\n");

	for (modrm = 0; modrm < 256; modrm++) {

	ret = 2;
	switch(modrm >> 6) {
		case 0:
			is_reg = 0;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			breg[(modrm >>3) & 0x7], rm[modrm & 0x7], modrm);
#include "modrm_00.incl"
			break;
		case 1:
			is_reg = 0;
			ret = 3;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			breg[(modrm >>3) & 0x7], brm[modrm & 0x7], modrm);
#include "modrm_01.incl"
			break;
		case 2:
			is_reg = 0;
			ret = 4;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			breg[(modrm >>3) & 0x7], wrm[modrm & 0x7], modrm);
#include "modrm_10.incl"
			break;
		case 3:
			is_reg = 1;
			printf("\t\tcase MOD_%s_EB_%s: \n\t\t\tMEM_REF = ",
				breg[(modrm >>3) & 0x7], breg[modrm & 0x7]);
			printf("&(%s);\n",breg[modrm & 0x7]);
			break;
		}
		printf("\t\t\tinterp_var->reg1 = &(%s);\n", breg[(modrm >>3) & 0x7]);
		printf("\t\t\tIS_MODE_REG = %d; return(%d);\n",is_reg, ret);
	}
	printf("\t}\n}\n");
	printf("\n\nint\n");
	printf("hsw_modrm_16_word");
	printf("(Interp_ENV *env, unsigned char *PC, Interp_VAR *interp_var)\n{\n");
	printf("\tswitch(*(PC+1)) {\n");

	for (modrm = 0; modrm < 256; modrm++) {

	ret = 2;
	switch(modrm >> 6) {
		case 0:
			is_reg = 0;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], rm[modrm & 0x7], modrm);
#include "modrm_00.incl"
			break;
		case 1:
			is_reg = 0;
			ret = 3;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], brm[modrm & 0x7], modrm);
#include "modrm_01.incl"
			break;
		case 2:
			is_reg = 0;
			ret = 4;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], wrm[modrm & 0x7], modrm);
#include "modrm_10.incl"
			break;
		case 3:
			is_reg = 1;
			printf("\t\tcase MOD_%s_EW_%s: \n\t\t\tMEM_REF = ",
				wreg[(modrm >>3) & 0x7], wreg[modrm & 0x7]);
			printf("(unsigned char *)&(%s);\n",wreg[modrm & 0x7]);
			break;
		}
		printf("\t\t\tinterp_var->reg1 = (unsigned char *)&(%s);\n", wreg[(modrm >>3) & 0x7]);
		printf("\t\t\tIS_MODE_REG = %d; return(%d);\n",is_reg, ret);
	}
	printf("\t}\n}\n");
	printf("\n\nint\n");
	printf("hsw_modrm_16_quad");
	printf("(Interp_ENV *env, unsigned char *PC, Interp_VAR *interp_var)\n{\n");
	printf("\tswitch(*(PC+1)) {\n");

	for (modrm = 0; modrm < 256; modrm++) {

	ret = 2;
	switch(modrm >> 6) {
		case 0:
			is_reg = 0;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], rm[modrm & 0x7], modrm);
#include "modrm_00.incl"
			break;
		case 1:
			is_reg = 0;
			ret = 3;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], brm[modrm & 0x7], modrm);
#include "modrm_01.incl"
			break;
		case 2:
			is_reg = 0;
			ret = 4;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], wrm[modrm & 0x7], modrm);
#include "modrm_10.incl"
			break;
		case 3:
			is_reg = 1;
			printf("\t\tcase MOD_%s_EW_%s: \n\t\t\tMEM_REF = ",
				wreg[(modrm >>3) & 0x7], wreg[modrm & 0x7]);
			printf("(unsigned char *)&(E%s);\n",wreg[modrm & 0x7]);
			break;
		}
		printf("\t\t\tinterp_var->reg1 = (unsigned char *)&(E%s);\n", wreg[(modrm >>3) & 0x7]);
		printf("\t\t\tIS_MODE_REG = %d; return(%d);\n",is_reg, ret);
	}
	printf("\t}\n}\n");
	printf("\n\nint\n");
	printf("hsw_modrm_32_byte");
	printf("(Interp_ENV *env, unsigned char *PC, Interp_VAR *interp_var)\n{\n");
	printf("\tswitch(*(PC+1)) {\n");

	for (modrm = 0; modrm < 256; modrm++) {

	ret = 2;
	switch(modrm >> 6) {
		case 0:
			is_reg = 0;
			printf("\t\tcase MOD_%s_%s: \n\t\t\t",
			breg[(modrm >>3) & 0x7], rm[modrm & 0x7], modrm);
#include "modrm_0032.incl"
			break;
		case 1:
			is_reg = 0;
			ret = 3;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			breg[(modrm >>3) & 0x7], brm[modrm & 0x7], modrm);
#include "modrm_0132.incl"
			break;
		case 2:
			is_reg = 0;
			ret = 6;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			breg[(modrm >>3) & 0x7], wrm[modrm & 0x7], modrm);
#include "modrm_1032.incl"
			break;
		case 3:
			is_reg = 1;
			printf("\t\tcase MOD_%s_EB_%s: \n\t\t\tMEM_REF = ",
				breg[(modrm >>3) & 0x7], breg[modrm & 0x7]);
			printf("&(%s);\n",breg[modrm & 0x7]);
			break;
		}
		printf("\t\t\tinterp_var->reg1 = &(%s);\n", breg[(modrm >>3) & 0x7]);
		printf("\t\t\tIS_MODE_REG = %d; return(%d);\n",is_reg, ret);
	}
	printf("\t}\n}\n");
	printf("\n\nint\n");
	printf("hsw_modrm_32_word");
	printf("(Interp_ENV *env, unsigned char *PC, Interp_VAR *interp_var)\n{\n");
	printf("\tswitch(*(PC+1)) {\n");

	for (modrm = 0; modrm < 256; modrm++) {

	ret = 2;
	switch(modrm >> 6) {
		case 0:
			is_reg = 0;
			printf("\t\tcase MOD_%s_%s: \n\t\t\t",
			wreg[(modrm >>3) & 0x7], rm[modrm & 0x7], modrm);
#include "modrm_0032.incl"
			break;
		case 1:
			is_reg = 0;
			ret = 3;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], brm[modrm & 0x7], modrm);
#include "modrm_0132.incl"
			break;
		case 2:
			is_reg = 0;
			ret = 6;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], wrm[modrm & 0x7], modrm);
#include "modrm_1032.incl"
			break;
		case 3:
			is_reg = 1;
			printf("\t\tcase MOD_%s_EW_%s: \n\t\t\tMEM_REF = ",
				wreg[(modrm >>3) & 0x7], wreg[modrm & 0x7]);
			printf("(unsigned char *)&(%s);\n",wreg[modrm & 0x7]);
			break;
		}
		printf("\t\t\tinterp_var->reg1 = (unsigned char *)&(%s);\n", wreg[(modrm >>3) & 0x7]);
		printf("\t\t\tIS_MODE_REG = %d; return(%d);\n",is_reg, ret);
	}
	printf("\t}\n}\n");
	printf("\n\nint\n");
	printf("hsw_modrm_32_quad");
	printf("(Interp_ENV *env, unsigned char *PC, Interp_VAR *interp_var)\n{\n");
	printf("\tswitch(*(PC+1)) {\n");

	for (modrm = 0; modrm < 256; modrm++) {

	ret = 2;
	switch(modrm >> 6) {
		case 0:
			is_reg = 0;
			printf("\t\tcase MOD_%s_%s: \n\t\t\t",
			wreg[(modrm >>3) & 0x7], rm[modrm & 0x7], modrm);
#include "modrm_0032.incl"
			break;
		case 1:
			is_reg = 0;
			ret = 3;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], brm[modrm & 0x7], modrm);
#include "modrm_0132.incl"
			break;
		case 2:
			is_reg = 0;
			ret = 6;
			printf("\t\tcase MOD_%s_%s: \n\t\t\tMEM_REF = ",
			wreg[(modrm >>3) & 0x7], wrm[modrm & 0x7], modrm);
#include "modrm_1032.incl"
			break;
		case 3:
			is_reg = 1;
			printf("\t\tcase MOD_%s_EW_%s: \n\t\t\tMEM_REF = ",
				wreg[(modrm >>3) & 0x7], wreg[modrm & 0x7]);
			printf("(unsigned char *)&(E%s);\n",wreg[modrm & 0x7]);
			break;
		}
		printf("\t\t\tinterp_var->reg1 = (unsigned char *)&(E%s);\n", wreg[(modrm >>3) & 0x7]);
		printf("\t\t\tIS_MODE_REG = %d; return(%d);\n",is_reg, ret);
	}
	printf("\t}\n}\n");
	exit(0);
}
