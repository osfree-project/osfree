
/*
 *@@sourcefile iconmgr.c:
 *      miscellaneous icon manager code.
 *
 *      This file is new with V0.9.20. The code used
 *      to be in icons.c before and hasn't changed.
 *
 *      Function prefix for this file:
 *      --  icm*
 *
 *@@added V0.9.16 (2001-10-01) [umoeller]
 *@@header "filesys\icons.h"
 */

/*
 *      Copyright (C) 2001-2013 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#pragma strings(readonly)

/*
 *  Suggested #include order:
 *  1)  os2.h
 *  2)  C library headers
 *  3)  setup.h (code generation and debugging options)
 *  4)  headers in helpers\
 *  5)  at least one SOM implementation header (*.ih)
 *  6)  dlgids.h, headers in shared\ (as needed)
 *  7)  headers in implementation dirs (e.g. filesys\, as needed)
 *  8)  #pragma hdrstop and then more SOM headers which crash with precompiled headers
 */

#define INCL_DOSPROCESS
#define INCL_DOSEXCEPTIONS
#define INCL_DOSSEMAPHORES
#define INCL_DOSRESOURCES
#define INCL_DOSERRORS

#define INCL_WINPOINTERS
#define INCL_WINSYS
#define INCL_WINPROGRAMLIST     // needed for PROGDETAILS, wppgm.h

#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#include <os2.h>

// C library headers
#include <stdio.h>
#include <setjmp.h>             // needed for except.h
#include <assert.h>             // needed for except.h

// generic headers
#include "setup.h"                      // code generation and debugging options

// headers in /helpers
#include "helpers\dosh.h"               // Control Program helper routines
#include "helpers\except.h"             // exception handling
#include "helpers\exeh.h"               // executable helpers
#include "helpers\linklist.h"           // linked list helper routines
#include "helpers\standards.h"          // some standard macros
#include "helpers\threads.h"            // thread helpers

// SOM headers which don't crash with prec. header files
#include "xfldr.ih"
#include "xfobj.ih"

// XWorkplace implementation headers
#include "dlgids.h"                     // all the IDs that are shared with NLS
#include "shared\common.h"              // the majestic XWorkplace include file
#include "shared\errors.h"              // private XWorkplace error codes
#include "shared\kernel.h"              // XWorkplace Kernel
#include "shared\wpsh.h"                // some pseudo-SOM functions (WPS helper routines)

// headers in /hook

#include "filesys\filesys.h"            // various file-system object implementation code
#include "filesys\icons.h"              // icons handling
#include "filesys\object.h"             // XFldObject implementation

// other SOM headers

#pragma hdrstop
#include <wpdataf.h>

/* ******************************************************************
 *
 *   Global variables
 *
 ********************************************************************/

static HMTX         G_hmtxIconShares = NULLHANDLE;

static HMTX         G_hmtxLazyIcons = NULLHANDLE;
static LINKLIST     G_llLazyIcons;
static HEV          G_hevLazyIcons = NULLHANDLE;
static THREADINFO   G_tiLazyIcons = {0};

/* ******************************************************************
 *
 *   Icon sharing
 *
 ********************************************************************/

/*
 *@@ icomLockIconShares:
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

BOOL icomLockIconShares(VOID)
{
    if (G_hmtxIconShares)
        // return !DosRequestMutexSem(G_hmtxIconShares, SEM_INDEFINITE_WAIT);
        if (!DosRequestMutexSem(G_hmtxIconShares, 2000))
            return TRUE;
        else
        {
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "Cannot request icon shares mutex.");
            return FALSE;
        }

    // first call:
    return !DosCreateMutexSem(NULL,
                              &G_hmtxIconShares,
                              0,
                              TRUE);
}

/*
 *@@ icomUnlockIconShares:
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

VOID icomUnlockIconShares(VOID)
{
    DosReleaseMutexSem(G_hmtxIconShares);
}

/*
 *@@ icomShareIcon:
 *      returns the current icon of somSelf and registers pobjClient
 *      as a current user of the icon so that pobjClient can be
 *      forced to refresh its icon if somSelf's icon changes.
 *
 *      This was added for finally getting icons from associations
 *      right. Practically, this function only gets called on WPProgram
 *      and WPProgramFile objects, but instead of having to implement
 *      it twice, I added it to XFldObject as the only common
 *      ancestor.
 *
 *      XFldObject now has a list in its instance data to allow
 *      for tracking who is sharing icons with whom. For this, I
 *      chose the "server" and "client" terminology: the server
 *      object (typically, a program object) shares its icon with
 *      possibly many clients (typically, data files).
 *
 *      To avoid circular references and endless loops, it is
 *      prohibited that an object be both server and client.
 *
 *      In detail, this does the following:
 *
 *      1)  Calls wpQueryIcon(somSelf) to get the current icon of
 *          the server. This will go through the class-specific
 *          overhead to determine somSelf's current (program) icon.
 *
 *      2)  If the client is already a client of some other server,
 *          it is detached from that server's tree first.
 *
 *      3)  Adds pobjClient to the list of clients using this icon
 *          in somSelf's instance data, if it's not a client of
 *          ourselves already. (Otherwise we'll just leave it
 *          there.)
 *
 *      4)  If fMakeGlobal is TRUE and the icon has not yet been
 *          made global (i.e. pobjClient is the first requestor),
 *          we tell PM to put the icon in shared memory.
 *
 *      Cleanup happens automatically:
 *
 *      --  The client is automatically removed from the private
 *          list when he goes dormant. This works because the
 *          caller is an XFldObject too and we can thus do the
 *          cleanup behind his back as well.
 *
 *      --  When the server goes dormant and still has clients
 *          (for example, if a program object with associations
 *          is deleted), all the clients' icons are set to
 *          NULLHANDLE to force a re-query.
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

HPOINTER icomShareIcon(WPObject *somSelf,       // in: server object
                       WPObject *pobjClient,    // in: client object
                       BOOL fMakeGlobal)
{
    XFldObjectData *somThis = XFldObjectGetData(somSelf);
    HPOINTER    hptrIcon;
    BOOL        fLocked = FALSE;
    PCSZ        p1, p2;

    // now, since we are supposed to become a server now,
    // MAKE SURE THAT WE ARE NOT A CLIENT OURSELVES, or
    // we can get into circular references and have a
    // never-ending mess
    if (_pobjIconServer)
    {
        cmnLog(__FILE__, __LINE__, __FUNCTION__,
               "Detected circular icon reference for pobjClient 0x%lX [%s] with server 0x%lX [%s]",
               pobjClient,
               (p1 = _wpQueryTitle(pobjClient)) ? p1 : "NULL",
               somSelf,
               (p2 = _wpQueryTitle(somSelf)) ? p2 : "NULL");

        return NULLHANDLE;
    }

    // get our own icon (class-dependent, can go through
    // a lot of overhead for getting executable icons etc.)
    if (hptrIcon = _wpQueryIcon(somSelf))
    {
        // alright, got it: then go hack up the client/server data

        XFldObjectData *somClient = XFldObjectGetData(pobjClient);

        TRY_LOUD(excpt1)
        {
            if (fLocked = icomLockIconShares())
            {
                // check if the object was already using a shared
                // icon:
                // -- if it's already a client of somSelf, then
                //    the object is already in our tree
                // -- if it was sharing the icon from someone else,
                //    we must remove the client from _that_ server first
                if (    (somClient->pobjIconServer)
                     && (somClient->pobjIconServer != somSelf)
                   )
                {
                    PMPF_ICONREPLACEMENTS(("Un-sharing pobjClient 0x%lX [%s] from previous server 0x%lX [%s]",
                           pobjClient,
                           (p1 = _wpQueryTitle(pobjClient)) ? p1 : "NULL",
                           somSelf,
                           (p2 = _wpQueryTitle(somSelf)) ? p2 : "NULL"));

                    icomUnShareIcon(somClient->pobjIconServer,
                                    pobjClient);
                }

                // add the client to our own tree
                // if it's not already our client
                if (somClient->pobjIconServer != somSelf)
                {
                    if (_pLastIconClient)
                    {
                        // list is not empty: append to tail

                        // store new client in the previously last object
                        XFldObjectData *somLast = XFldObjectGetData(_pLastIconClient);
                        somLast->pNextClient = pobjClient;
                        // store that object as the previous in the new client
                        somClient->pPreviousClient = _pLastIconClient;
                        // terminate list with the new client
                        somClient->pNextClient = NULL;
                        // and make new client the last one now
                        _pLastIconClient = pobjClient;

                        // raise list count
                        ++(_cIconClients);
                    }
                    else
                    {
                        // list is empty: insert as first
                        _pFirstIconClient
                            = _pLastIconClient
                            = pobjClient;

                        somClient->pNextClient
                            = somClient->pPreviousClient
                            = NULL;

                        _cIconClients = 1;
                    }

                    // lock ourselves once so that we won't go dormant
                    _wpLockObject(somSelf);

                    // and store ourselves as the new server of the client
                    somClient->pobjIconServer = somSelf;
                }
            }
        }
        CATCH(excpt1)
        {
        } END_CATCH();

        if (fLocked)
        {
            icomUnlockIconShares();
            fLocked = FALSE;
        }

        if (    (fMakeGlobal)
             // && (fLocked = !_wpRequestObjectMutexSem(somSelf, SEM_INDEFINITE_WAIT))
                // it is sufficient to do this once
             && (!(_flObject & OBJFL_GLOBALICON))
           )
        {
            _flObject |= OBJFL_GLOBALICON;
            // _wpReleaseObjectMutexSem(somSelf);

            WinSetPointerOwner(hptrIcon,
                               0,           // pid
                               FALSE);      // do not allow people to destroy this
        }

    }

    return hptrIcon;
}

/*
 *@@ icomUnShareIcon:
 *
 *      Notes:
 *
 *      --  The caller must hold the icon shares mutex. This
 *          is different from icomShareIcon.
 *
 *      --  There's no exception handling in this function.
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

VOID icomUnShareIcon(WPObject *pobjServer,      // in: icon server object
                     WPObject *pobjClient)      // in: icon client object
{
    BOOL fLocked = FALSE;
    XFldObjectData *somServer = XFldObjectGetData(pobjServer);
    XFldObjectData *somClient = XFldObjectGetData(pobjClient);
    XFldObjectData *somThat;

    if (somServer->cIconClients)
    {
        if (somServer->pFirstIconClient == pobjClient)
            // item to be removed is first: adjust first
            somServer->pFirstIconClient = somClient->pNextClient;     // can be NULL

        if (somServer->pLastIconClient == pobjClient)
            // item to be removed is last: adjust last
            somServer->pLastIconClient = somClient->pPreviousClient;  // can be NULL

        if (somClient->pPreviousClient)
        {
            // we have a previous object: make that point to
            // our next object, taking us out from the middle
            somThat = XFldObjectGetData(somClient->pPreviousClient);
            somThat->pNextClient = somClient->pNextClient;
        }

        if (somClient->pNextClient)
        {
            // we have a next object: make that point to
            // our previous object, taking us out from the middle
            somThat = XFldObjectGetData(somClient->pNextClient);
            somThat->pPreviousClient = somClient->pPreviousClient;
        }

        somClient->pobjIconServer
            = somClient->pPreviousClient
            = somClient->pNextClient
            = NULL;

        // decrease list count
        (somServer->cIconClients)--;

        if (    (!somServer->cIconClients)      // was this the last icon?
             // && (fLocked = !_wpRequestObjectMutexSem(pobjServer, SEM_INDEFINITE_WAIT))
                // it is sufficient to do this once
             && (somServer->flObject & OBJFL_GLOBALICON)
           )
        {
            somServer->flObject &= ~OBJFL_GLOBALICON;

            // _wpReleaseObjectMutexSem(pobjServer);

            WinSetPointerOwner(_wpQueryIcon(pobjServer),
                               G_pidWPS,
                               TRUE);
        }

        // unlock the server once,
        // it was locked by us previously
        _wpUnlockObject(pobjServer);
    }
}

/* ******************************************************************
 *
 *   Lazy icons
 *
 ********************************************************************/

STATIC void _Optlink fntLazyIcons(PTHREADINFO ptiMyself);

/*
 *@@ LockLazyIcons:
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 *@@changed V1.0.4 (2005-04-11) [pr]: More error checking
 */

STATIC BOOL LockLazyIcons(VOID)
{
    if (G_hmtxLazyIcons)
        return !DosRequestMutexSem(G_hmtxLazyIcons, SEM_INDEFINITE_WAIT);

    if (!DosCreateMutexSem(NULL,
                           &G_hmtxLazyIcons,
                           0,
                           TRUE))
    {
        if (!DosCreateEventSem(NULL,
                               &G_hevLazyIcons,
                               0,
                               FALSE))     // "reset" (not posted)
        {
            lstInit(&G_llLazyIcons, FALSE);
            thrCreate(&G_tiLazyIcons,
                      fntLazyIcons,
                      NULL,
                      "LazyIcons",
                      THRF_PMMSGQUEUE | THRF_WAIT,
                      0);
            return TRUE;
        }
        else
        {
            DosReleaseMutexSem(G_hmtxLazyIcons);
            DosCloseMutexSem(G_hmtxLazyIcons);
            G_hmtxLazyIcons = NULLHANDLE;
            return FALSE;
        }
    }

    return FALSE;
}

/*
 *@@ UnlockLazyIcons:
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

STATIC VOID UnlockLazyIcons(VOID)
{
    DosReleaseMutexSem(G_hmtxLazyIcons);
}

/*
 *@@ fntLazyIcons:
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 *@@changed V1.0.1 (2003-01-27) [umoeller]: now calling new _xwpLazyLoadIcon method
 *@@changed V1.0.10 (2013-03-16) [pr]: removed junk and ignore crashes @@fixes 336
 */

STATIC void _Optlink fntLazyIcons(PTHREADINFO ptiMyself)
{
    BOOL    fLocked = FALSE;
    BOOL    fDummy = 1;

    TRY_LOUD(excpt1)
    {
        while (!ptiMyself->fExit)
        {
            PLISTNODE   pNode;
            ULONG       flOwnerDraw;

            DosWaitEventSem(G_hevLazyIcons, SEM_INDEFINITE_WAIT);

            // refresh owner draw setting
            flOwnerDraw = cmnQuerySetting(sflOwnerDrawIcons);

            if (fLocked = LockLazyIcons())
            {
                ULONG       ulPosted;
                DosResetEventSem(G_hevLazyIcons, &ulPosted);

                while (    (fLocked)
                        && (!ptiMyself->fExit)
                      )
                {
                    WPObject    *pDataFile = NULL;

                    // take one object off the list; make this
                    // list FIFO by taking the object off
                    // the tail of the list
                    if (pNode = lstQueryLastNode(&G_llLazyIcons))
                    {
                        pDataFile = (WPObject*)pNode->pItemData;
                        lstRemoveNode(&G_llLazyIcons,
                                      pNode);
                    }

                    UnlockLazyIcons();
                    fLocked = FALSE;

                    if (!pDataFile)
                        break;
                    else
                    {
                        TRY_QUIET(excpt2)
                        {
                            _xwpLazyLoadIcon(pDataFile,
                                             flOwnerDraw,
                                             &ptiMyself->fExit);
                                    // replaced V1.0.1 (2003-01-25) [umoeller]
                        }
                        CATCH(excpt2)
                        {
                        } END_CATCH();
                    }

                    // grab the next item on the list; only if
                    // there's nothing left, sleep on the event
                    // semaphore again

                    fLocked = LockLazyIcons();
                } // while (fLocked)
            } // if (fLocked = LockLazyIcons())
        } // while (!ptiMyself->fExit)
    }
    CATCH(excpt1)
    {
    } END_CATCH();

    if (fLocked)
        UnlockLazyIcons();

    // V1.0.4 (2005-04-10) [pr]: Cleanup, otherwise we don't restart when we crash
    DosCloseEventSem(G_hevLazyIcons);
    G_hevLazyIcons = NULLHANDLE;
    DosCloseMutexSem(G_hmtxLazyIcons);
    G_hmtxLazyIcons = NULLHANDLE;
}

/*
 *@@ icomQueueLazyIcon:
 *      adds the given data file to the queue of
 *      icons to be loaded lazily.
 *
 *@@added V0.9.20 (2002-07-25) [umoeller]
 */

BOOL icomQueueLazyIcon(WPDataFile *somSelf)
{
    BOOL fLocked = FALSE,
         brc = FALSE;

    PMPF_ICONREPLACEMENTS(("[%s]", _wpQueryTitle(somSelf)));

    if (fLocked = LockLazyIcons())
    {
        if (lstAppendItem(&G_llLazyIcons,
                          somSelf))
        {
            // set a flag so that XFldObject::wpSetIconHandle knows
            // that the new icon that will come in soon comes from us
            _xwpModifyFlags(somSelf,
                            OBJFL_LAZYLOADINGICON,
                            OBJFL_LAZYLOADINGICON);

            DosPostEventSem(G_hevLazyIcons);
            brc = TRUE;
        }
        else
            cmnLog(__FILE__, __LINE__, __FUNCTION__,
                   "lstAppendItem failed for %s",
                   _wpQueryTitle(somSelf));

        UnlockLazyIcons();
    }

    return brc;
}

/*
 *@@ icomUnqueueLazyIcon:
 *      removes the given data file from the queue of
 *      icons to be loaded lazily.
 *
 *@@added V1.0.10 (2013-03-15) [pr]: @@fixes 336
 */

VOID icomUnqueueLazyIcon(WPDataFile *somSelf)
{
    if (LockLazyIcons())
    {
        lstRemoveItem(&G_llLazyIcons,
                      somSelf);
        UnlockLazyIcons();
    }
}

/* ******************************************************************
 *
 *   Object icon management
 *
 ********************************************************************/

/*
 *@@ icomRunReplacement:
 *      returns TRUE if either turbo folders or
 *      extended associations are enabled.
 *
 *      Used mainly by XWPProgramFile and
 *      XWPProgram.
 *
 *@@added V0.9.18 (2002-03-16) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

BOOL icomRunReplacement(VOID)
{
    BOOL        fRunReplacement = FALSE;

    // turbo folders enabled?
#ifndef __NOTURBOFOLDERS__
    fRunReplacement = cmnQuerySetting(sfTurboFolders);
#endif

#ifndef __NOICONREPLACEMENTS__
    if (!fRunReplacement)
        if (cmnQuerySetting(sfIconReplacements))
            fRunReplacement = TRUE;
#endif

    return fRunReplacement;
}

/*
 *@@ icomClsQueryMaxAnimationIcons:
 *      evil helper for calling wpclsQueryMaxAnimationIcons,
 *      which is undocumented.
 *
 *      See xfTP_wpclsQueryMaxAnimationIcons.
 *
 *      Note that this calls a class method, so pass in a
 *      class object.
 *
 *@@added V0.9.19 (2002-06-15) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

ULONG icomClsQueryMaxAnimationIcons(M_WPObject *somSelf)
{
    return _wpclsQueryMaxAnimationIcons(somSelf);
}

/*
 *@@ icomQueryIconN:
 *      evil helper for calling wpQueryIcon
 *      or wpQueryIconN, which is undocumented.
 *
 *      Returns 0 on errors.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

HPOINTER icomQueryIconN(WPObject *pobj,    // in: object
                        ULONG ulIndex)     // in: animation index or 0 for regular icon
{
    if (!ulIndex)
        return _wpQueryIcon(pobj);

    // index specified: this better be a folder, and we better be Warp 4
    if (    (G_fIsWarp4)
         && (_somIsA(pobj, _WPFolder))
       )
    {
        return _wpQueryIconN(pobj, ulIndex);
    }

    return 0;
}

/*
 *@@ icomQueryIconDataN:
 *      evil helper for calling wpQueryIconData
 *      or wpQueryIconDataN, which is undocumented.
 *
 *      Returns 0 on errors.
 *
 *      The "query icon data" methods return
 *      an ICONINFO structure. This only sometimes
 *      contains the "icon data" (which is why the
 *      method name is very misleading), but sometimes
 *      instructions for the caller about the icon
 *      format:
 *
 *      --  With ICON_RESOURCE, the caller will
 *          find the icon data in the specified
 *          icon resource.
 *
 *      --  With ICON_FILE, the caller will have
 *          to load an icon file.
 *
 *      --  Only with ICON_DATA, the icon data is
 *          actually returned. This is usually
 *          returned if the object has a user icon
 *          set in OS2.INI (for abstracts) or
 *          in the .ICONx EA (for fs objects)
 *
 *      As a result, icomLoadIconData was added
 *      which actually loads the icon data if
 *      the format isn't ICON_DATA already.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

ULONG icomQueryIconDataN(WPObject *pobj,    // in: object
                         ULONG ulIndex,     // in: animation index or 0 for regular icon
                         PICONINFO pData)   // in: icon data buffer or NULL for "query size"
{
    if (!ulIndex)
        return _wpQueryIconData(pobj, pData);

    // index specified: this better be a folder, and we better be Warp 4
    if (    (G_fIsWarp4)
         && (_somIsA(pobj, _WPFolder))
       )
    {
        return _wpQueryIconDataN(pobj, pData, ulIndex);
    }

    return 0;
}

/*
 *@@ icomSetIconDataN:
 *      evil helper for calling wpSetIconData
 *      or wpSetIconDataN, which is undocumented.
 *
 *      While wpSetIcon only temporarily changes
 *      an object's icon (which is then returned
 *      by wpQueryIcon), wpSetIconData stores a
 *      new persistent icon as well. For abstracts,
 *      this will go into OS2.INI, for fs objects
 *      into the object's EAs.
 *
 *      Note that as a special hack, wpSetIconData
 *      supports the ICON_CLEAR "format", which
 *      resets the icon data to the class's default
 *      icon (or something else for special classes
 *      like WPDisk, WPDataFile or WPProgram).
 *
 *      Returns 0 on errors.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

BOOL icomSetIconDataN(WPObject *pobj,    // in: object
                      ULONG ulIndex,     // in: animation index or 0 for regular icon
                      PICONINFO pData)   // in: icon data to set (requried)
{
    if (!ulIndex)
        return _wpSetIconData(pobj, pData);

    // index specified: this better be a folder, and we better be Warp 4
    if (    (G_fIsWarp4)
         && (_somIsA(pobj, _WPFolder))
       )
    {
        return _wpSetIconDataN(pobj, pData, ulIndex);
    }

    return 0;
}

/*
 *@@ icoClsQueryIconN:
 *      evil helper for calling wpclsQueryIconN,
 *      which is undocumented.
 *
 *      WARNING: This always builds a new HPOINTER,
 *      so the caller should use WinDestroyPointer
 *      to avoid resource leaks.
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

HPOINTER icoClsQueryIconN(SOMClass *pClassObject,
                          ULONG ulIndex)
{
    if (!ulIndex)
        return _wpclsQueryIcon(pClassObject);

    // index specified: this better be a folder, and we better be Warp 4
    if (    (G_fIsWarp4)
         && (_somDescendedFrom(pClassObject, _WPFolder))
       )
    {
        return _wpclsQueryIconN(pClassObject, ulIndex);
    }

    return 0;
}

/*
 *@@ icomLoadIconData:
 *      retrieves the ICONINFO for the specified
 *      object and animation index in a new buffer.
 *
 *      If ulIndex == 0, this retrieves the standard
 *      icon. Otherwise this returns the animation icon.
 *      Even though the WPS always uses this stupid
 *      index with the icon method calls, I don't think
 *      any index besides 1 is actually supported.
 *
 *      If this returns NO_ERROR, the given PICONINFO*
 *      will receive a pointer to a newly allocated
 *      ICONINFO buffer whose format is always ICON_DATA.
 *      This will properly load an icon resource if the
 *      object has the icon format set to ICON_RESOURCE
 *      or ICON_FILE.
 *
 *      If NO_ERROR is returned, the caller must free()
 *      this pointer.
 *
 *      Otherwise this might return the following errors:
 *
 *      --  ERROR_NO_DATA: icon format not understood.
 *
 *      --  ERROR_FILE_NOT_FOUND: icon file doesn't exist.
 *
 *      plus those of doshMalloc and DosGetResource, such as
 *      ERROR_NOT_ENOUGH_MEMORY.
 *
 *      This is ICONINFO:
 *
 +      typedef struct _ICONINFO {
 +        ULONG       cb;           // Length of the ICONINFO structure.
 +        ULONG       fFormat;      // Indicates where the icon resides.
 +        PSZ         pszFileName;  // Name of the file containing icon data (ICON_FILE)
 +        HMODULE     hmod;         // Module containing the icon resource (ICON_RESOURCE)
 +        ULONG       resid;        // Identity of the icon resource (ICON_RESOURCE)
 +        ULONG       cbIconData;   // Length of the icon data in bytes (ICON_DATA)
 +        PVOID       pIconData;    // Pointer to the buffer containing icon data (ICON_DATA)
 +      } ICONINFO;
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 *@@changed V0.9.18 (2002-03-19) [umoeller]: no longer recursing, which didn't work anyway
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

APIRET icomLoadIconData(WPObject *pobj,             // in: object whose icon to query
                        ULONG ulIndex,              // in: animation index or 0 for regular icon
                        PICONINFO *ppIconInfo)      // out: ICONINFO allocated via _wpAllocMem
{
    APIRET arc = NO_ERROR;
    PICONINFO pData = NULL;
    ULONG cbIconInfo;

    arc = ERROR_NO_DATA;     // whatever, this shouldn't fail

    *ppIconInfo = NULL;

    // find out how much memory the object needs for this
    if (    (cbIconInfo = icomQueryIconDataN(pobj,
                                             ulIndex,
                                             NULL))        // query size
                                // if this fails, arc is still ERROR_NO_DATA
            // allocate the memory
         && (pData = doshMalloc(cbIconInfo, &arc))
       )
    {
        PMPF_ICONREPLACEMENTS(("allocated %d bytes", cbIconInfo));

        // ask the object again
        if (icomQueryIconDataN(pobj,
                               ulIndex,
                               pData))
        {
            PMPF_ICONREPLACEMENTS(("   got %d bytes data", cbIconInfo));

            // get the icon data depending on the format
            switch (pData->fFormat)
            {
                case ICON_RESOURCE:
                {
                    ULONG   cbResource;
                    PVOID   pvResourceTemp;

                    PMPF_ICONREPLACEMENTS(("   ICON_RESOURCE 0x%lX, %d", pData->hmod, pData->resid));

                    // object has specified icon resource:
                    // load resource data...
                    if (    (!(arc = DosQueryResourceSize(pData->hmod,
                                                          RT_POINTER,
                                                          pData->resid,
                                                          &cbResource)))
                       )
                    {
                        if (!cbResource)
                            arc = ERROR_NO_DATA;
                        else if (!(arc = DosGetResource(pData->hmod,
                                                        RT_POINTER,
                                                        pData->resid,
                                                        &pvResourceTemp)))
                        {
                            // loaded OK:
                            // return a new ICONINFO then
                            PICONINFO pData2;
                            ULONG     cb2 = sizeof(ICONINFO) + cbResource;
                            if (pData2 = doshMalloc(cb2, &arc))
                            {
                                // point icon data to after ICONINFO struct
                                PBYTE pbIconData = (PBYTE)pData2 + sizeof(ICONINFO);
                                pData2->cb = sizeof(ICONINFO);
                                pData2->fFormat = ICON_DATA;
                                pData2->cbIconData = cbResource;
                                pData2->pIconData = pbIconData;
                                // copy icon data there
                                memcpy(pbIconData, pvResourceTemp, cbResource);

                                // and return the new struct
                                *ppIconInfo = pData2;
                            }

                            // in any case, free the original
                            free(pData);
                            pData = NULL;       // do not free again below

                            DosFreeResource(pvResourceTemp);
                        }
                    }
                }
                break;

                case ICON_DATA:

                    PMPF_ICONREPLACEMENTS(("   ICON_DATA"));

                    // this is OK, no conversion needed
                    *ppIconInfo = pData;
                    arc = NO_ERROR;
                break;

                case ICON_FILE:
                {
                    WPFileSystem *pfs;

                    PMPF_ICONREPLACEMENTS(("   ICON_FILE \"%s\"", pData->pszFileName));

                    if (    (pData->pszFileName)
                         && (pfs = _wpclsQueryObjectFromPath(_WPFileSystem,
                                                             pData->pszFileName))
                       )
                    {
                        ULONG cbRequired;
                        if (!(cbRequired = _wpQueryIconData(pfs, NULL)))
                            arc = ERROR_NO_DATA;
                        else
                        {
                            PICONINFO pData2;
                            if (pData2 = doshMalloc(cbRequired, &arc))
                            {
                                if (!_wpQueryIconData(pfs, pData2))
                                {
                                    arc = ERROR_NO_DATA;
                                    free(pData2);
                                }
                                else
                                {
                                    free(pData);
                                    *ppIconInfo = pData2;
                                    arc = NO_ERROR;
                                }
                            }
                        }

                        _wpUnlockObject(pfs);
                    }
                    else
                        arc = ERROR_FILE_NOT_FOUND;
                }
                break;

                default:
                    // any other format:
                    PMPF_ICONREPLACEMENTS(("    invalid format %d", pData->fFormat));

                    arc = ERROR_INVALID_DATA;
            } // end switch (pData->Format)
        } // end if (_wpQueryIconData(pobj, pData))
        else
            arc = ERROR_NO_DATA;

        if (arc && pData)
            free(pData);
    }

    PMPF_ICONREPLACEMENTS(("returning %d", arc));

    return arc;
}

/*
 *@@ icomCopyIconFromObject:
 *      sets a new persistent icon for somSelf
 *      by copying the icon data from pobjSource.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

APIRET icomCopyIconFromObject(WPObject *somSelf,       // in: target
                              WPObject *pobjSource,    // in: source
                              ULONG ulIndex)           // in: animation index or 0 for regular icon
{
    APIRET arc = NO_ERROR;

    if (pobjSource = _xwpResolveIfLink(pobjSource))
    {
        PICONINFO pData;
        if (arc = icomLoadIconData(pobjSource, ulIndex, &pData))
            // error loading that icon:
            // if we're trying to load an animation icon
            // (i.e. ulIndex != 0), try loading index 0 and
            // set that on the animation icon... the user
            // might be trying to drag a regular icon on
            // the animation page
            // V0.9.16 (2001-12-08) [umoeller]
            arc = icomLoadIconData(pobjSource, 0, &pData);

        if (!arc)
        {
            // now set this icon for the target object
            icomSetIconDataN(somSelf, ulIndex, pData);
            free(pData);

            // the standard WPS behavior is that
            // if a folder icon is copied onto the
            // standard icon of a folder, the animation
            // icon is copied too... so check:
            if (    (_somIsA(somSelf, _WPFolder))
                 && (_somIsA(pobjSource, _WPFolder))
                 // but don't do this if we're setting the
                 // animation icon explicitly
                 && (!ulIndex)
               )
            {
                // alright, copy animation icon too,
                // but if this fails, don't return an error
                if (!icomLoadIconData(pobjSource, 1, &pData))
                {
                    icomSetIconDataN(somSelf, 1, pData);
                    free(pData);
                }
            }
        }
    }
    else
        arc = ERROR_FILE_NOT_FOUND;

    return arc;
}

/*
 *@@ icomResetIcon:
 *      resets an object's icon to its default.
 *
 *      See the explanations about ICON_CLEAR
 *      with icomSetIconDataN.
 *
 *@@added V0.9.16 (2001-10-15) [umoeller]
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

VOID icomResetIcon(WPObject *somSelf,
                   ULONG ulIndex)
{
    ICONINFO Data;
    memset(&Data, 0, sizeof(Data));
    Data.fFormat = ICON_CLEAR;
    icomSetIconDataN(somSelf,
                     ulIndex,
                     &Data);
}

/*
 *@@ icomIsUsingDefaultIcon:
 *      returns TRUE if the object is using
 *      a default icon for the specified
 *      animation index.
 *
 *      This call is potentially expensive so
 *      don't use it excessively.
 *
 *@@added V0.9.16 (2001-10-19) [umoeller]
 *@@changed V0.9.18 (2002-03-19) [umoeller]: fixed disappearing animation icon
 *@@changed V0.9.20 (2002-07-25) [umoeller]: renamed prefix
 */

BOOL icomIsUsingDefaultIcon(WPObject *pobj,
                            ULONG ulAnimationIndex)
{
    if (ulAnimationIndex)
    {
        // caller wants animation icon checked:
        // compare this object's icon to the class's
        // default animation icon
        BOOL brc = FALSE;
        HPOINTER hptrClass = icoClsQueryIconN(_somGetClass(pobj),
                                              ulAnimationIndex);
        if (    hptrClass
             && (icomQueryIconN(pobj,
                                ulAnimationIndex)
                 == hptrClass)
           )
            brc = TRUE;

#ifndef __NOICONREPLACEMENTS__
        // only destroy the pointer if we're not
        // running our replacement because then
        // it is shared
        // V0.9.18 (2002-03-19) [umoeller]
        if (!cmnQuerySetting(sfIconReplacements))
#endif
            WinDestroyPointer(hptrClass);

        return brc;
    }

    // caller wants regular icon checked:
    // do NOT compare it to the class default icon
    // but check the object style instead (there are
    // many default icons in the WPS which are _not_
    // class default icons, e.g. WPProgram and WPDataFile
    // association icons)
    return ((0 == (_wpQueryStyle(pobj)
                            & OBJSTYLE_NOTDEFAULTICON)));
}


