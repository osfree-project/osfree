#ifndef _Fs_globals_hpp_
#define _Fs_globals_hpp_
/*
 $Id: Fs_globals.hpp,v 1.3 2002/10/23 14:29:52 evgen2 Exp $
*/
/* Fs_globals.hpp */
/* ver 0.00 20.08.2002       */

#include <time.h>
#include <sys/time.h>
#include "F_def.hpp"
#include "FreePMs.hpp"
#include "Fs_WND.hpp"

/*+---------------------------------+*/
/*| Global server variables         |*/
/*+---------------------------------+*/

extern class F_WND_List  _WndList;
extern class _FreePM_HAB  _shab;

extern void fatal(const char *message);

#endif
