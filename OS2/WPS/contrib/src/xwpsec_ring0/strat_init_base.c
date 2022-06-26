
/*
 *@@sourcefile strat_init_base.c:
 *      PDD "init_base" strategy routine.
 *
 *      XWPSEC32.SYS is a base (!) device driver that implements
 *      local security together with the XWorkplace "security
 *      shell", XWPSHELL.EXE.
 *
 *      Installation:
 *
 *      1)  Copy XWPSEC32.SYS to \OS2\BOOT.
 *
 *      2)  Add the following line to CONFIG.SYS:
 *
 +          BASEDEV=XWPSEC32.SYS
 *
 *      Installation of SES is _not_ required.
 *
 *      The way this works is that on init, this driver exports
 *      a table of security callouts to the OS/2 kernel which
 *      causes the kernel to call the driver for a number of
 *      security-sensitve events, such as "open", "delete",
 *      "execute program" and so on. This is done in sec32_init_base().
 *      In this respect, the driver is a typical OS/2 security
 *      driver. We do _not_ however use any of the SES functionality
 *      dealing with subject handles, but instead implement all that
 *      ourselves.
 *
 *      In detail:
 *
 *      --  sec32_strategy.c contains our strategy routing table.
 *          This gets called from the 16-bit strategy stub in
 *          sec32_start.asm and routes the regular OS/2 request
 *          packets for "init", "open driver", "ioctl", and "close
 *          driver" to our 32-bit C implementations.
 *
 *          In other words, those regular OS/2 strategy calls are
 *          pretty rare with this driver. Open is restricted to
 *          a single process (XWPShell, or whoever else opens it),
 *          and that process uses IOCtls to control the driver.
 *
 *          See strat_ioctl.c for an introduction to the driver's
 *          ioctl interface.
 *
 *      --  sec32_callbacks.c contains our security callouts
 *          table that is passed to the OS/2 kernel on init.
 *          There we store the various callouts we implement
 *          in the callb_*.c files, e.g. the OPEN_PRE() callout.
 *          These callouts are 32-bit so no thunking is required.
 *
 *          There are two classes of callouts:
 *
 *          1)  "pre" callouts allow a security driver to
 *              authorize an event. For example, the driver
 *              can return ERROR_ACCESS_DENIED for an OPEN_PRE
 *              event.
 *
 *          2)  "post" callouts notify the driver of the
 *              completion of an event. These usually have no
 *              return code. For example, OPEN_POST notifies the
 *              driver of whether an open request succeeded.
 *              Some of those notifications are critical to the
 *              driver's operations. For example, the driver gets
 *              notifified of process creation via the
 *              EXECPGM_POST call.
 *
 *          Note that we do nothing in these callouts unless
 *          XWPShell currently has the driver open. See
 *          XWPSECIO_REGISTER for details.
 *
 *      --  sec32_contexts.c has the driver "engine", so-to-say,
 *          with logging, security contexts, and authentication
 *          of the various events.
 *
 *      Contexts in which the driver runs:
 *
 *      --  The callb_*.c callouts get called in possibly any
 *          task context of the system -- whenever some ring-3
 *          thread calls a Dos* API that is hooked.
 *
 *      --  The open/ioctl/close request packets will only
 *          work for a single process, which, per definition,
 *          is the XWorkplace security shell (XWPSHELL.EXE).
 */

/*
 *      Copyright (C) 2000-2003 Ulrich M”ller.
 *      Based on the MWDD32.SYS example sources,
 *      Copyright (C) 1995, 1996, 1997  Matthieu Willm (willm@ibm.net).
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>

#include <string.h>

#include "xwpsec32.sys\types.h"
#include "xwpsec32.sys\StackToFlat.h"
#include "xwpsec32.sys\DevHlp32.h"
#include "xwpsec32.sys\reqpkt32.h"

#include "xwpsec32.sys\xwpsec_callbacks.h"

#include "bldlevel.h"

/* ******************************************************************
 *
 *   Private definitions
 *
 ********************************************************************/

extern int _System sec32_pre_init_base(PTR16 reqpkt);

#pragma pack(1)

/*
 *@@ ddd32_parm_list:
 *      a pointer to this struct is in the INIT
 *      request packet. This allows us to retrieve
 *      the driver command line and such.
 */

struct ddd32_parm_list
{
    unsigned short cache_parm_list;        // addr of InitCache_Parameter List
    unsigned short disk_config_table;      // addr of disk_configuration table
    unsigned short init_req_vec_table;     // addr of IRQ_Vector_Table
    unsigned short cmd_line_args;          // addr of Command Line Args
    unsigned short machine_config_table;   // addr of Machine Config Info
};

#pragma pack()

extern unsigned short codeend;      // in sec32_start.asm
extern unsigned short dataend;      // in sec32_start.asm
extern PTR16          DevHelp2;

extern void code32_begin(void);     // in sec32_start.asm
extern int  data32_begin;           // in sec32_start.asm

extern char code32_end;
extern char data32_end;

extern int G_DebugPort;

void *G_TKSSBase;                   // in sec32_pre_init_base.asm
    // pointer to kernel TKSSBase value.

/* ******************************************************************
 *
 *   Global variables (DATA32)
 *
 ********************************************************************/

char    G_szBanner[] =
    "XWPSEC32.SYS: XWorkplace Security driver V" BLDLEVEL_VERSION " (" __DATE__ ") loaded\r\n"
    "(C) Copyright 2000-2003 Ulrich M”ller\r\n";

char    G_szNoSES[] = "XWPSEC32.SYS: Error initializing kernel security hooks!\r\n";

char    G_szCom1[] = "XWPSEC32.SYS: Debugging to COM port 1.\r\n";
char    G_szCom2[] = "XWPSEC32.SYS: Debugging to COM port 2.\r\n";

PTR16   DevHelp2;

/* ******************************************************************
 *
 *   Helper functions
 *
 ********************************************************************/

/*
 *@@ FixMemory:
 *      called by sec32_init_base() to lock our data
 *      and code segments into physical memory.
 *
 *      This is necessary because OS/2 locks only the
 *      first code and the first data segment of a
 *      driver, but these are the 16-bit segments, and
 *      we need the 32-bit segments locked too. So we
 *      have to do it explicitly here.
 *
 *      Returns NO_ERROR or error code.
 *
 *      Context: "init" request, ring 0.
 */

int FixMemory(void)
{
    APIRET  rc;
    ULONG   ulPgCount;
    char    abLock[12];            // receives lock handle

    // lock DGROUP into physical memory
    if (!(rc = DevHlp32_VMLock(VMDHL_LONG          // lock for more than 2 seconds
                                  | VMDHL_WRITE,    // write (set "dirty bit" in page table entries)
                                  // | VMDHL_VERIFY,
                                  // since VMDHL_NOBLOCK is not specified, this blocks
                                  // until the page is available
                               &data32_begin,
                               (ULONG)&data32_end - (ULONG)&data32_begin,
                               (PVOID)-1,
                               __StackToFlat(abLock),        // receives lock handle
                               __StackToFlat(&ulPgCount))))
    {
        // locks CODE32 into physical memory
        rc = DevHlp32_VMLock(VMDHL_LONG,
                                 // | VMDHL_VERIFY,
                             (PVOID)&code32_begin,
                             (ULONG)&code32_end - (ULONG)&code32_begin,
                             (PVOID)-1,
                             __StackToFlat(abLock),
                             __StackToFlat(&ulPgCount));
    }

    return rc;
}

/*
 *@@ InitSecurity:
 *      interfaces SES to pass our kernel hooks and
 *      get the security helpers.
 *
 *      Context: "init" request, ring 0.
 */

int InitSecurity(VOID)
{
    APIRET rc;

    // pass table of security hooks to kernel
    if (!(rc = DevHlp32_Security(DHSEC_SETIMPORT,
                                 &G_SecurityHooks))) // in sec32_callbacks.c
    {
        // retrieve security helpers
        /* if (!(rc = sec32_attach_ses((void *)&SecHlp)))
        {
            rc = DevHlp32_Security(DHSEC_GETEXPORT,
                                   &SecurityExports);
        } */
    }

    return rc;
}

/* ******************************************************************
 *
 *   Strategy INIT_BASE code
 *
 ********************************************************************/

/*
 *@@ sec32_init_base:
 *      initializes the base device driver. Since an ISS device driver
 *      is really a BASEDEV, this is similar to a BASEDEV initialization,
 *      that is, this code runs at ring 0.
 *
 *      This is called from sec32_strategy() since it's stored in
 *      driver_routing_table in sec32_strategy.c.
 *
 *      This calls sec32_pre_init_base() first (assembler code).
 *
 *      In addition to the BASEDEV initialization phase, an OS/2
 *      security driver must at least issue the following:
 *
 *      1. Call the kernel DevHlp32_Security helper to retrieve 32 bits
 *         kernel SecHlp entry points.
 *
 *      2. Call the kernel DevHlp32_Security helper to pass to the kernel
 *         the addresses of our security callbacks (to hook kernel
 *         functions such as DosOpen).
 *
 *      3. Attach to SES's sesdd32.sys and call its IDC entry point to
 *         retrieve 32 bits sesdd32.sys's SecHlp entry points.
 *
 *      Context: "init" request, ring 0.
 */

int sec32_init_base(PTR16 reqpkt)
{
    int                     fQuiet = 0;
    int                     status = 0;
    int                     rc;
    struct reqpkt_init      *pRequest;
    struct ddd32_parm_list  *pCmd;

    // run assembler init code to make DevHelp32 and __StackToFlat
    // work (sec32_pre_init_base.asm)
    if (rc = sec32_pre_init_base(reqpkt))
        // error in assembler drv32_pre_init:
        status |= STERR + ERROR_I24_QUIET_INIT_FAIL;

    // convert the 16:16 request packet pointer to FLAT
    else if (rc = DevHlp32_VirtToLin(reqpkt,
                                     __StackToFlat(&pRequest)))
        status |= STERR + ERROR_I24_QUIET_INIT_FAIL;

    // convert the 16:16 command line pointer to FLAT
    else if (rc = DevHlp32_VirtToLin(pRequest->u.input.initarg,
                                     __StackToFlat(&pCmd)))
        status |= STERR + ERROR_I24_QUIET_INIT_FAIL;
    else
    {
        // OK so far: process parameters
        CHAR    *pszParm;
        PTR16   tmpparm;
        tmpparm.seg = pRequest->u.input.initarg.seg;
        tmpparm.ofs = pCmd->cmd_line_args;

        if (rc = DevHlp32_VirtToLin(tmpparm,
                                    __StackToFlat(&pszParm)))
            status |= STERR + ERROR_I24_QUIET_INIT_FAIL;
        else
        {
            char    *tmp;
            __strupr(pszParm);
            for (tmp = __strpbrk(pszParm, "-/");
                 tmp;
                 tmp = __strpbrk(tmp, "-/"))
            {
                // go to char after "-" or "/"
                ++tmp;

                // -Q: quiet initialization?
                if (strncmp(tmp, "Q", sizeof("Q") - 1) == 0)
                {
                    fQuiet = 1;
                    continue;
                }

#define OUTPUT_COM1 0x3F8
#define OUTPUT_COM2 0x2F8

                if (strncmp(tmp, "1", sizeof("1") - 1) == 0)
                {
                    G_DebugPort = OUTPUT_COM1;
                    continue;
                }

                if (strncmp(tmp, "2", sizeof("2") - 1) == 0)
                {
                    G_DebugPort = OUTPUT_COM2;
                    continue;
                }
            }

            // now go lock 32-bit segments (see remarks there)
            if (rc = FixMemory())
                status |= STERR + ERROR_I24_QUIET_INIT_FAIL;

            // interface SES (pass kernel hooks, get helpers)
            else if (rc = InitSecurity())
            {
                // error interfacing SES:
                DevHlp32_SaveMessage(G_szNoSES, sizeof(G_szNoSES));
                status |= STERR + ERROR_I24_QUIET_INIT_FAIL;
            }
            else
            {
                // init security contexts
                ctxtInit();

                // OK, driver successfully initialized:
                // if not "-Q", put out message
                if (!fQuiet)
                {
                    // say hello
                    DevHlp32_SaveMessage(G_szBanner,
                                         sizeof(G_szBanner));

                    switch (G_DebugPort)
                    {
                        case OUTPUT_COM1:
                            kernel_printf(G_szCom1);
                            DevHlp32_SaveMessage(G_szCom1,
                                                 sizeof(G_szCom1));
                        break;

                        case OUTPUT_COM2:
                            kernel_printf(G_szCom2);
                            DevHlp32_SaveMessage(G_szCom2,
                                                 sizeof(G_szCom2));
                        break;
                    }
                }
            }
        }
    }

    // return end of code and end of CODE16 and DATA16;
    // this is normally used by 16-bit drivers so that
    // the init-code and -data can be released after
    // initialization, but since our init code is in
    // 32-bit code, this won't happen for us...
    pRequest->u.output.codeend = codeend;
            // set to offset CODE16:code16_end, see sec32_start.asm
    pRequest->u.output.dataend = dataend;
            // set to offset DATA16:data16_end, see sec32_start.asm

    return (status | STDON);
}

