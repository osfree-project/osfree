/*
 *  This file was generated by the SOM Compiler and Emitter Framework.
 *  Generated using template emitter:
 *      SOM Emitter emitctm: 2.23.1.9
 */

#ifndef SOM_Module_wpdisk_Source
#define SOM_Module_wpdisk_Source
#endif
#define WPDisk_Class_Source
#define M_WPDisk_Class_Source

#define INCL_PMWP
#include <os2.h>

#include "wpdisk.ih"


SOM_Scope ULONG  SOMLINK dsk_wpAddDiskDetailsPage(WPDisk *somSelf, 
                                                  HWND hwndNotebook)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpAddDiskDetailsPage");

    /* Return statement to be customized: */
    return;
}

SOM_Scope ULONG  SOMLINK dsk_wpQueryDriveLockStatus(WPDisk *somSelf, 
                                                    PULONG pulLockStatus, 
                                                    PULONG pulLockCount)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpQueryDriveLockStatus");

    /* Return statement to be customized: */
    return;
}

SOM_Scope ULONG  SOMLINK dsk_wpEjectDisk(WPDisk *somSelf)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpEjectDisk");

    /* Return statement to be customized: */
    return;
}

SOM_Scope ULONG  SOMLINK dsk_wpLockDrive(WPDisk *somSelf, BOOL fLock)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpLockDrive");

    /* Return statement to be customized: */
    return;
}

SOM_Scope ULONG  SOMLINK dsk_wpQueryLogicalDrive(WPDisk *somSelf)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpQueryLogicalDrive");

    /* Return statement to be customized: */
    return;
}

SOM_Scope WPRootFolder*  SOMLINK dsk_wpQueryRootFolder(WPDisk *somSelf)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpQueryRootFolder");

    /* Return statement to be customized: */
    return;
}

SOM_Scope BOOL  SOMLINK dsk_wpSetCorrectDiskIcon(WPDisk *somSelf)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpSetCorrectDiskIcon");

    /* Return statement to be customized: */
    return;
}

SOM_Scope BOOL  SOMLINK dsk_wpSetLogicalDrive(WPDisk *somSelf, 
                                              ULONG ulLogicalDrive)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpSetLogicalDrive");

    /* Return statement to be customized: */
    return;
}

SOM_Scope BOOL  SOMLINK dsk_wpSetDiskInfo(WPDisk *somSelf)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpSetDiskInfo");

    /* Return statement to be customized: */
    return;
}

SOM_Scope void  SOMLINK dsk_wpTouchDisk(WPDisk *somSelf)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpTouchDisk");

}

SOM_Scope BOOL  SOMLINK dsk_wpSetup(WPDisk *somSelf, PSZ pszSetupString)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpSetup");

    return (WPDisk_parent_WPAbstract_wpSetup(somSelf, pszSetupString));
}

SOM_Scope BOOL  SOMLINK dsk_wpSaveState(WPDisk *somSelf)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpSaveState");

    return (WPDisk_parent_WPAbstract_wpSaveState(somSelf));
}

SOM_Scope BOOL  SOMLINK dsk_wpRestoreState(WPDisk *somSelf, ULONG ulReserved)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpRestoreState");

    return (WPDisk_parent_WPAbstract_wpRestoreState(somSelf, 
                                                    ulReserved));
}

SOM_Scope BOOL  SOMLINK dsk_wpAddSettingsPages(WPDisk *somSelf, 
                                               HWND hwndNotebook)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpAddSettingsPages");

    return (WPDisk_parent_WPAbstract_wpAddSettingsPages(somSelf, 
                                                        hwndNotebook));
}

SOM_Scope BOOL  SOMLINK dsk_wpMenuItemHelpSelected(WPDisk *somSelf, 
                                                   ULONG MenuId)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpMenuItemHelpSelected");

    return (WPDisk_parent_WPAbstract_wpMenuItemHelpSelected(somSelf, 
                                                            MenuId));
}

SOM_Scope BOOL  SOMLINK dsk_wpMenuItemSelected(WPDisk *somSelf, 
                                               HWND hwndFrame, 
                                               ULONG ulMenuId)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpMenuItemSelected");

    return (WPDisk_parent_WPAbstract_wpMenuItemSelected(somSelf, 
                                                        hwndFrame, 
                                                        ulMenuId));
}

SOM_Scope BOOL  SOMLINK dsk_wpSetTitle(WPDisk *somSelf, PSZ pszNewTitle)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpSetTitle");

    return (WPDisk_parent_WPAbstract_wpSetTitle(somSelf, pszNewTitle));
}

SOM_Scope HWND  SOMLINK dsk_wpOpen(WPDisk *somSelf, HWND hwndCnr, 
                                   ULONG ulView, ULONG param)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpOpen");

    return (WPDisk_parent_WPAbstract_wpOpen(somSelf, hwndCnr, 
                                            ulView, param));
}

SOM_Scope BOOL  SOMLINK dsk_wpQueryDefaultHelp(WPDisk *somSelf, 
                                               PULONG pHelpPanelId, 
                                               PSZ HelpLibrary)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpQueryDefaultHelp");

    return (WPDisk_parent_WPAbstract_wpQueryDefaultHelp(somSelf, 
                                                        pHelpPanelId, 
                                                        HelpLibrary));
}

SOM_Scope MRESULT  SOMLINK dsk_wpDragOver(WPDisk *somSelf, HWND hwndCnr, 
                                          PDRAGINFO pdrgInfo)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpDragOver");

    return (WPDisk_parent_WPAbstract_wpDragOver(somSelf, hwndCnr, 
                                                pdrgInfo));
}

SOM_Scope MRESULT  SOMLINK dsk_wpDrop(WPDisk *somSelf, HWND hwndCnr, 
                                      PDRAGINFO pdrgInfo, PDRAGITEM pdrgItem)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpDrop");

    return (WPDisk_parent_WPAbstract_wpDrop(somSelf, hwndCnr, 
                                            pdrgInfo, pdrgItem));
}

SOM_Scope ULONG  SOMLINK dsk_wpQueryIconData(WPDisk *somSelf, 
                                             PICONINFO pIconInfo)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpQueryIconData");

    return (WPDisk_parent_WPAbstract_wpQueryIconData(somSelf, 
                                                     pIconInfo));
}

SOM_Scope BOOL  SOMLINK dsk_wpSetIconData(WPDisk *somSelf, PICONINFO pIconInfo)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpSetIconData");

    return (WPDisk_parent_WPAbstract_wpSetIconData(somSelf, pIconInfo));
}

SOM_Scope HWND  SOMLINK dsk_wpViewObject(WPDisk *somSelf, HWND hwndCnr, 
                                         ULONG ulView, ULONG param)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpViewObject");

    return (WPDisk_parent_WPAbstract_wpViewObject(somSelf, hwndCnr, 
                                                  ulView, param));
}

SOM_Scope void  SOMLINK dsk_wpUnInitData(WPDisk *somSelf)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpUnInitData");

    WPDisk_parent_WPAbstract_wpUnInitData(somSelf);
}
 
SOM_Scope BOOL  SOMLINK dsk_wpFilterMenu(WPDisk *somSelf,
                                         FILTERFLAGS* pFlags, 
                                         HWND hwndCnr, BOOL fMultiSelect, 
                                         ULONG ulMenuType, ULONG ulView, 
                                         ULONG ulReserved)

{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpFilterPopupMenu");

    return (WPDisk_parent_WPAbstract_wpFilterMenu(somSelf, pFlags, 
                                                      hwndCnr, 
                                                      fMultiSelect, 
                                                      ulMenuType, 
                                                      ulView, 
                                                      ulReserved));
}

SOM_Scope BOOL  SOMLINK dsk_wpModifyMenu(WPDisk *somSelf, HWND hwndMenu, 
                                         HWND hwndCnr, ULONG iPosition, 
                                         ULONG ulMenuType, ULONG ulView, 
                                         ULONG ulReserved)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpModifyMenu");

    return (WPDisk_parent_WPAbstract_wpModifyMenu(somSelf, hwndMenu, 
                                                  hwndCnr, iPosition, 
                                                  ulMenuType, 
                                                  ulView, ulReserved));
}

SOM_Scope void SOMLINK dsk_somDefaultInit(WPDisk *somSelf, somInitCtrl* ctrl)
{
    WPDiskData *somThis; /* set in BeginInitializer */
    somInitCtrl globalCtrl;
    somBooleanVector myMask;
    WPDiskMethodDebug("WPDisk","somDefaultInit");
    WPDisk_BeginInitializer_somDefaultInit;

    WPDisk_Init_WPAbstract_somDefaultInit(somSelf, ctrl);

    /*
     * local WPDisk initialization code added by programmer
     */
}


SOM_Scope void SOMLINK dsk_somDestruct(WPDisk *somSelf, octet doFree, 
                                       somDestructCtrl* ctrl)
{
    WPDiskData *somThis; /* set in BeginDestructor */
    somDestructCtrl globalCtrl;
    somBooleanVector myMask;
    WPDiskMethodDebug("WPDisk","dsk_somDestruct");
    WPDisk_BeginDestructor;

    /*
     * local WPDisk deinitialization code added by programmer
     */

    WPDisk_EndDestructor;
}


SOM_Scope ULONG  SOMLINK dsk_wpAddObjectWindowPage(WPDisk *somSelf, 
                                                   HWND hwndNotebook)
{
    /* WPDiskData *somThis = WPDiskGetData(somSelf); */
    WPDiskMethodDebug("WPDisk","dsk_wpAddObjectWindowPage");

    return (WPDisk_parent_WPAbstract_wpAddObjectWindowPage(somSelf, 
                                                           hwndNotebook));
}


SOM_Scope BOOL  SOMLINK dskM_wpclsQueryDefaultHelp(M_WPDisk *somSelf, 
                                                   PULONG pHelpPanelId, 
                                                   PSZ pszHelpLibrary)
{
    /* M_WPDiskData *somThis = M_WPDiskGetData(somSelf); */
    M_WPDiskMethodDebug("M_WPDisk","dskM_wpclsQueryDefaultHelp");

    return (M_WPDisk_parent_M_WPAbstract_wpclsQueryDefaultHelp(somSelf, 
                                                               pHelpPanelId, 
                                                               pszHelpLibrary));
}

SOM_Scope ULONG  SOMLINK dskM_wpclsQueryIconData(M_WPDisk *somSelf, 
                                                 PICONINFO pIconInfo)
{
    /* M_WPDiskData *somThis = M_WPDiskGetData(somSelf); */
    M_WPDiskMethodDebug("M_WPDisk","dskM_wpclsQueryIconData");

    return (M_WPDisk_parent_M_WPAbstract_wpclsQueryIconData(somSelf, 
                                                            pIconInfo));
}

SOM_Scope ULONG  SOMLINK dskM_wpclsQueryStyle(M_WPDisk *somSelf)
{
    /* M_WPDiskData *somThis = M_WPDiskGetData(somSelf); */
    M_WPDiskMethodDebug("M_WPDisk","dskM_wpclsQueryStyle");

    return (M_WPDisk_parent_M_WPAbstract_wpclsQueryStyle(somSelf));
}

SOM_Scope PSZ  SOMLINK dskM_wpclsQueryTitle(M_WPDisk *somSelf)
{
    /* M_WPDiskData *somThis = M_WPDiskGetData(somSelf); */
    M_WPDiskMethodDebug("M_WPDisk","dskM_wpclsQueryTitle");

    return (M_WPDisk_parent_M_WPAbstract_wpclsQueryTitle(somSelf));
}

SOM_Scope ULONG  SOMLINK dskM_wpclsQueryDefaultView(M_WPDisk *somSelf)
{
    /* M_WPDiskData *somThis = M_WPDiskGetData(somSelf); */
    M_WPDiskMethodDebug("M_WPDisk","dskM_wpclsQueryDefaultView");

    return (M_WPDisk_parent_M_WPAbstract_wpclsQueryDefaultView(somSelf));
}

SOM_Scope BOOL  SOMLINK dskM_wpclsQuerySettingsPageSize(M_WPDisk *somSelf, 
                                                        PSIZEL pSizl)
{
    /* M_WPDiskData *somThis = M_WPDiskGetData(somSelf); */
    M_WPDiskMethodDebug("M_WPDisk","dskM_wpclsQuerySettingsPageSize");

    return (M_WPDisk_parent_M_WPAbstract_wpclsQuerySettingsPageSize(somSelf, 
                                                                    pSizl));
}

