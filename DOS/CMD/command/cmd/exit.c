/* $Id: exit.c 1291 2006-09-05 01:44:33Z blairdude $
 *  EXIT -- exits current instance of FreeCOM or leave all batch files
 *
 * set the exitflag to true
 */

#include "../config.h"

#include "../include/command.h"

int internal_exit (char * rest) { (void)rest; exitflag = 1; return 0; }
