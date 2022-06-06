/* lvm.cli.h  */
#if 0
#include "lvm_gbls.h"

typedef struct _LVMCLI_BackEndToVIO {
  int a;
      } LVMCLI_BackEndToVIO;    
#endif      
/*****************************************************************************
 * SCCSID = src/lvm/common/include/LVM_Cli.h, lvm.cli, w45.fs32, 20000823.1 98/07/24
 *
 * IBM CONFIDENTIAL
 *
 * OCO Source Materials
 *
 * Program number (if available)
 *
 * (c) Copyright IBM Corp. 1998
 *
 * The source code for this program is not published or otherwise divested
 * of its tradesecrets, irrespective of what has been deposited with the
 * U.S. Copyright Office.
 *
 * Name: LVM_CLI.H
 *
 * Functions: LVMCLI
 *
 *              CARDINAL lvmcli( int argc, char * argv[] )
 *
 * Description: Routine called to invoke the command line interface for LVM.
 *              It expects to receive the argument list as it was received
 *              from the command line when LVM.EXE was invoked.
 *
 * Notes:
 *
 *                              Modifications
 * Date      Name          Description
 * ---------------------------------------------------------------------------
 * 6/12/98  M. McBride     Original
 *
 *****************************************************************************/

#ifndef LVM_CLI_H

#define LVM_CLI_H

#include "gbltypes.h"

//
// Possible back-end to VIO operation requests.
typedef enum
{
   NoOperation              = 0,
   DisplayMinInstallSize    = 1
} LVMCLI_VIO_Operation;

// Interface structure between LVM CLI's back end and the VIO user interface.
typedef struct _LVMCLI_BackEndToVIO
{
   LVMCLI_VIO_Operation  operation;                // request to VIO from LVM back-end
   CARDINAL32            minPartitonInstallSize;   // minimum install partition size
                                                   //  in MB (1 MB = 1024 * 1024 bytes)
   CARDINAL32            LVMError;                 // an LVM_ENGINE error, if encountered
} LVMCLI_BackEndToVIO;

LVMCLI_BackEndToVIO* lvmcli( int argc, char * argv[] );

#endif
