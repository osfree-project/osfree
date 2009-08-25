/*
 * Copyright (C) 2000, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       jvm.h
 * OVERVIEW:   The header file for the JVM backend.
 *
 * Copyright (C) 2000, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision: 1.3 $
 * 30 Aug 00 - Brian: Created file based on Sameer's jvm.h.
*/

#ifndef _JVM_H_
#define _JVM_H_

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "options.h"

/*==============================================================================
 * Procedures.
 *============================================================================*/

// The main translation function which is called from driver.cc
void translate2j(UserProc* proc);

#endif /*_JVM_H_*/
