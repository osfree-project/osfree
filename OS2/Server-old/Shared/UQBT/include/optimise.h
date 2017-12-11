/*
 * Copyright (C) 2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       optimise.h
 * OVERVIEW:   Header file for JVM-related optimisations.
 *
 * Copyright (C) 2000, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.3 $
 * 30 Aug 00 - Brian: Created file based on Sameer's unimplemented optimise.h.
*/

#ifndef _OPTIMISE_H_
#define _OPTIMISE_H_

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "options.h"

/*==============================================================================
 * Procedures.
 *============================================================================*/

// Not currently called.
void optimise(UserProc* proc);

#endif

