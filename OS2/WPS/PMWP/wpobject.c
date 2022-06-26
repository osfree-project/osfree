#define INCL_WINSTDDRAG
#define INCL_WINSTDCONTAINER
#include <os2.h>
#include <som.h>
#include "wpobject.ih"

SOM_Scope ULONG  SOMLINK obj_wpAddObjectGeneralPage(WPObject *somSelf, HWND hwndNotebook)
{
    WPObjectData *somThis = WPObjectGetData(somSelf);
    WPObjectMethodDebug("WPObject","xl_SetKeys");

    return 0;
}

#if 0

    ULONG wpAddObjectGeneralPage2(in HWND hwndNotebook);

    ULONG wpAddObjectWindowPage(in HWND hwndNotebook);

    BOOL wpAddSettingsPages(in HWND hwndNotebook);

    BOOL wpAddToObjUseList(in PUSEITEM pUseItem);

    PBYTE wpAllocMem(in ULONG cbBytes,
                     in PULONG prc);

    BOOL32 wpAppendObject(in WPObject targetObject,
                          in BOOL32 fMove);

    BOOL wpAssertObjectMutexSem();

    BOOL wpClose();

    PMINIRECORDCORE wpCnrInsertObject(in HWND hwndCnr,
                                      in PPOINTL pptlIcon,
                                      in PMINIRECORDCORE preccParent,
                                      in PRECORDINSERT pRecInsert);

    BOOL wpCnrRemoveObject(in HWND hwndCnr);

    BOOL wpCnrSetEmphasis(in ULONG ulEmphasisAttr,
                          in BOOL fTurnOn);

    ULONG wpConfirmDelete(in ULONG fConfirmations);

    ULONG wpConfirmObjectTitle(in WPFolder Folder,
                               inout WPObject ppDuplicate,
                               in PSZ pszTitle,
                               in ULONG cbTitle,
                               in ULONG menuID);

    VOID wpCopiedFromTemplate();

    WPObject wpCopyObject(in WPFolder Folder,
                          in BOOL fLock);

    WPObject wpCreateAnother(in PSZ pszTitle,
                             in PSZ pszSetupEnv,
                             in WPFolder Folder);

    WPObject wpCreateFromTemplate(in WPFolder folder,
                                  in BOOL fLock);

    WPObject wpCreateShadowObject(in WPFolder Folder,
                                  in BOOL fLock);

    ULONG wpDelete(in ULONG fConfirmations);

    BOOL wpDeleteFromObjUseList(in PUSEITEM pUseItem);

    BOOL wpDisplayHelp(in ULONG HelpPanelId,
                       in PSZ HelpLibrary);

    HWND   wpDisplayMenu(in HWND hwndOwner, in HWND hwndClient,
                         inout POINTL ptlPopupPt, in ULONG ulMenuType,
                         in ULONG ulReserved);

    BOOL wpDoesObjectMatch(in PVOID pvoidExtendedCriteria);

    MRESULT wpDraggedOverObject(in WPObject DraggedOverObject);

    MRESULT wpDragOver(in HWND hwndCnr,
                       in PDRAGINFO pdrgInfo);

    MRESULT wpDrop(in HWND hwndCnr,
                   in PDRAGINFO pdrgInfo,
                   in PDRAGITEM pdrgItem);

    BOOL wpDroppedOnObject(in WPObject DroppedOnObject);

    MRESULT wpEndConversation(in ULONG ulItemID,
                              in ULONG flResult);

    ULONG wpFilterPopupMenu(in ULONG ulFlags,
                            in HWND hwndCnr,
                            in BOOL fMultiSelect);

    PUSEITEM wpFindUseItem(in ULONG type,
                           in PUSEITEM pCurrentItem);

    BOOL wpFormatDragItem(in PDRAGITEM pdrgItem);

    BOOL wpFree();

    BOOL wpFreeMem(in PBYTE pByte);

    BOOL wpHide();

    VOID wpInitData();

    BOOL wpInsertPopupMenuItems(in HWND hwndMenu,
                                in ULONG iPosition,
                                in HMODULE hmod,
                                in ULONG MenuID,
                                in ULONG SubMenuID);

    ULONG wpInsertSettingsPage(in HWND hwndNotebook,
                               in PPAGEINFO ppageinfo);

    BOOL wpMenuItemHelpSelected(in ULONG MenuId);

    BOOL wpMenuItemSelected(in HWND hwndFrame,
                            in ULONG ulMenuId);

    BOOL wpModifyPopupMenu(in HWND hwndMenu,
                           in HWND hwndCnr,
                           in ULONG iPosition);

    BOOL wpMoveObject(in WPFolder Folder);

    HWND wpOpen(in HWND hwndCnr,
                in ULONG ulView,
                in ULONG param);

    BOOL wpPrintObject(in PPRINTDEST pPrintDest,
                       in ULONG ulReserved);

    ULONG wpQueryConcurrentView();

    ULONG wpQueryButtonAppearance();

    ULONG wpQueryConfirmations();

    BOOL wpQueryDefaultHelp(in PULONG pHelpPanelId,
                            in PSZ HelpLibrary);

    ULONG wpQueryDefaultView();

    ULONG wpQueryDetailsData(inout PVOID ppDetailsData,
                             in PULONG pcp);

    ULONG wpQueryError();

    BOOL wpSetFolder(in WPObject container);

    WPObject wpQueryFolder();

    HPOINTER wpQueryIcon();

    ULONG wpQueryIconData(in PICONINFO pIconInfo);

    ULONG wpQueryMinWindow();

    ULONG wpQueryNameClashOptions(in ULONG menuID);

    ULONG wpQueryStyle();

    BOOL32 wpSetTaskRec(in PTASKREC pNew,
                        in PTASKREC pOld);

    PTASKREC wpFindTaskRec();

    PSZ wpQueryTitle();

    BOOL wpRegisterView(in HWND hwndFrame,
                        in PSZ pszViewTitle);

    ULONG wpReleaseObjectMutexSem();

    MRESULT wpRender(in PDRAGTRANSFER pdxfer);

    MRESULT wpRenderComplete(in PDRAGTRANSFER pdxfer,
                             in ULONG ulResult);

    BOOL32 wpReplaceObject(in WPObject targetObject,
                           in BOOL32 fMove);

    ULONG wpRequestObjectMutexSem(in ULONG ulTimeout);

    BOOL wpRestore();

    BOOL wpRestoreData(in PSZ pszClass,
                       in ULONG ulKey,
                       in PBYTE pValue,
                       in PULONG pcbValue);

    BOOL wpRestoreLong(in PSZ pszClass,
                       in ULONG ulKey,
                       in PULONG pulValue);

    BOOL wpRestoreState(in ULONG ulReserved);

    BOOL wpRestoreString(in PSZ pszClass,
                         in ULONG ulKey,
                         in PSZ pszValue,
                         in PULONG pcbValue);

    BOOL wpSaveData(in PSZ pszClass,
                    in ULONG ulKey,
                    in PBYTE pValue,
                    in ULONG cbValue);

    BOOL wpSaveImmediate();

    BOOL wpSaveDeferred();

    BOOL wpSaveLong(in PSZ pszClass,
                    in ULONG ulKey,
                    in ULONG ulValue);

    BOOL wpSaveState();

    BOOL wpSaveString(in PSZ pszClass,
                      in ULONG ulKey,
                      in PSZ pszValue);

    BOOL wpScanSetupString(in PSZ pszSetupString,
                           in PSZ pszKey,
                           in PSZ pszValue,
                           in PULONG pcbValue);

    VOID wpSetConcurrentView(in ULONG ulCCView);

    VOID wpSetButtonAppearance(in ULONG ulButtonType);

    BOOL wpSetDefaultHelp(in ULONG HelpPanelId,
                          in PSZ HelpLibrary);

    BOOL wpSetDefaultView(in ULONG ulView);

    BOOL wpSetError(in ULONG ulErrorId);

    BOOL wpSetIconHandle(in HPOINTER hptrNewIcon);

    USHORT wpQueryScreenGroupID( in USHORT usPrevSgId );

    BOOL wpSetupOnce (in PSZ pszSetupString);

    BOOL wpSetIcon(in HPOINTER hptrNewIcon);

    BOOL wpSetIconData(in PICONINFO pIconInfo);

    VOID wpSetMinWindow(in ULONG ulMinWindow);

    BOOL wpSetStyle(in ULONG ulNewStyle);

    BOOL wpModifyStyle(in ULONG ulStyleFlags,
                       in ULONG ulStyleMask);

    BOOL wpSetTitle(in PSZ pszNewTitle);

    BOOL wpSetup(in PSZ pszSetupString);

    BOOL wpSwitchTo(in ULONG View);

    VOID wpUnInitData();

    HWND wpViewObject(in HWND hwndCnr,
                      in ULONG ulView,
                      in ULONG param);

    ULONG wpQueryTrueStyle();

    HOBJECT wpQueryHandle();

    BOOL wpUnlockObject();

    VOID wpObjectReady( in ULONG ulCode, in WPObject refObject );

    BOOL wpIsObjectInitialized();

    WPObject wpCreateShadowObjectExt( in WPFolder Folder, in BOOL fLock,
                                      in PSZ pszSetup, in M_WPObject shadowClass);

    BOOL wpCnrDeleteUseItem(in HWND hwndCnr);

    BOOL wpIsDeleteable();

    PMINIRECORDCORE wpQueryCoreRecord();

    BOOL wpSetObjectID(in PSZ pszObjectID);

    PSZ wpQueryObjectID();

    BOOL wpSetDefaultIconPos(in PPOINTL pPointl);

    BOOL wpQueryDefaultIconPos(in PPOINTL pPointl);

    VOID wpCnrRefreshDetails();

    PVIEWITEM wpFindViewItem(in ULONG flViews,
                             in PVIEWITEM pCurrentItem);

    VOID wpLockObject();

    BOOL wpIsLocked();

    PULONG wpQueryContainerFlagPtr();

    ULONG wpWaitForClose(in LHANDLE lhView,
                         in ULONG ulViews,
                         in LONG lTimeOut,
                         in BOOL bAutoClose);

#ifndef __NOWARP4METHODS__
    BOOL wpFilterMenu(inout FILTERFLAGS pFlags,
                      in HWND hwndCnr,
                      in BOOL fMultiSelect,
                      in ULONG ulMenuType,
                      in ULONG ulView,
                      in ULONG ulReserved);

    BOOL wpModifyMenu(in HWND hwndMenu,
                      in HWND hwndCnr,
                      in ULONG iPosition,
                      in ULONG ulMenuType,
                      in ULONG ulView,
                      in ULONG ulReserved);

    BOOL wpInsertMenuItems(in HWND hwndMenu,
                           in ULONG iPosition,
                           in HMODULE hmod,
                           in ULONG MenuID,
                           in ULONG SubMenuID);

    BOOL wpSetMenuStyle(in ULONG ulStyle);

    ULONG wpQueryMenuStyle();
#endif

//# new non-public Warp 3 methods hacked in for XWorkplace

    BOOL wpSetNextObj(in WPObject pobjNext);

    WPObject wpQueryNextObj();

    BOOL wpMakeDormant(in BOOL fSave);

    BOOL wpDestroyObject();

    BOOL wpDeleteWindowPosKeys();

    BOOL wpSetState(in ULONG fl);

    ULONG wpQueryState();

    BOOL wpModifyState(in ULONG flFlags,
                       in ULONG flMask);

    BOOL wpIdentify(in PSZ pszIdentity);

//# The WPS class list shows __get_XXX methods for this class, so
//# there must be SOM attributes here. Since however there is no
//# correponsing __set_XXX method, I assume the WPS uses readonly
//# attributes.

    readonly attribute WPObject*    pobjNext;

#endif
