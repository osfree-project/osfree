/* debugger.h
 */

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include "windows.h"
#include "DPMI.h"
#include "toolhelp.h"
#include "Resources.h"
#include "Module.h"

#define PTR_SEG_TO_LIN(a)	GetAddress(HIWORD(a), LOWORD(a))
#define PTR_SEG_OFF_TO_LIN(s,o)	GetAddress((s), (o))


void TWIN_print_address(unsigned int , FILE * , int );

#endif /* DEBUGGER_H */
