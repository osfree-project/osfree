// File: error.cc
// Desc: Error message code
// Created: 17th June 1998 Mike

/*==============================================================================
 * $Revision: 1.9 $
 * Dependencies.
 *============================================================================*/

#include "global.h"
/*==============================================================================
 * FUNCTION:      error
 * OVERVIEW:      We may decide to get fancy about this at some stage. For now
 *                at least, all error messages should funnel through this
 *                function.
 * PARAMETERS:    sMsg - an error message
 * RETURNS:       <nothing>
 *============================================================================*/
void error(const string& sMsg)
{
	cerr << "Error: " << sMsg << endl << flush;
}

/*==============================================================================
 * FUNCTION:      warning
 * OVERVIEW:      We may decide to get fancy about this at some stage. For now
 *                at least, all warning messages should funnel through this
 *                function.
 * PARAMETERS:    sMsg - a warning message
 * RETURNS:       <nothing>
 *============================================================================*/
void warning(const string& sMsg)
{
	cerr << "Warning: " << sMsg << endl << flush;
}

