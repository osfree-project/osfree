extern "C" {

//#include <strnlen.h>

#define INCL_DOSMODULEMGR
#include <os2.h>

#include <stdio.h>
#include <stdarg.h>

//#include "dl.h"

// DosLogWrite ordinal
#define LOG_ORD 1112

extern "C" void APIENTRY
_db_print(const char *format,...);

typedef APIRET APIENTRY (*logwrt_t)(PSZ s);

logwrt_t DosLogWrite = 0;

void log_init(void)
{
  long __syscall (*pfn)(void);
  APIRET  rc;
  HMODULE handle;

  /* now check if DosLogWrite is available */
  rc = DosQueryModuleHandle("DOSCALLS", &handle);

  if (rc) return;

  rc = DosQueryProcAddr(handle, LOG_ORD, 0, (PFN *)&pfn);

  if (rc != 182) // ERROR_INVALID_ORDINAL
    DosLogWrite = (logwrt_t)pfn;
}

void log(const char *fmt, ...)
{
  va_list arg_ptr;
  char buf[1024];

  va_start(arg_ptr, fmt);
  vsprintf(buf, fmt, arg_ptr);
  va_end(arg_ptr);

  _db_print(buf);
}


APIRET unimplemented(char *func)
{
  log("%s is not yet implemented!\n", func);
  return 0;
}


APIRET APIENTRY ShePIInitialise(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProfileInt(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProfileString(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinWriteProfileString(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProfileSize(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProfileData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinWriteProfileData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PrfChangeWriteThru(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PMExecRegister(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShePIInitIniFiles(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SheConvert190ProgramListTo300(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PrfCreateGroup(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PrfQueryProgramHandle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PrfQueryProgramCategory(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SheProfileHook(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SheInitializeIniFile(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShpPI16LockIniFile(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShpPI16Shutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EnablePMPre(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGreEntry(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreInitialize(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry3(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry4(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry5(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry6(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry7(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry8(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry9(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreEntry10(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GetDriverInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SetDriverInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PostDeviceModes(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY BitmapLimitWarning(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry3(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry4(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry5(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry6(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry7(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry8(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry9(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PmWinGreEntry10(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SSAllocSeg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SSAllocHuge(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SSFreeSeg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SSFlushSegs(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY RamSemRequest16(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY RamSemClear16(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLockWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryWindowLockCount(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY OldWinQueryPointerInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryHungProcess(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLoadStringTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLoadVkeyGlyphXlatetbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLoadCharXlatetbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetVkeyGlyphXlatetbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryVkeyGlyphXlatetbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinVkeyToScanID(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProcessCP(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinCreateHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinDestroyHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinAvailMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinAllocMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinReallocMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinFreeMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLockHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetLastError(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinCatch(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinThrow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FsrSemExit(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FsrSemCheck(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY AabGetReg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY AabSetReg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ValidateSelector(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY LoadModuleHandle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GetScreenSelector(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY IdentifyCodeSelector(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY _WinSetErrorInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinTerminateSystem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinDefQueueProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinVisRegionNotify(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryKbdLayout(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetKbdLayout(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinBuildPtrHandle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DumWinCreateHelpInstance(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DimWinDestroyHelpInstance(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DumWinAssociateHelpInstance(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DimWinCreateHelpTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DumWinLoadHelpTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DumWinQueryHelpInstance(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DummyHelpEntry(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GetPMDDCodeSelector(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryPwnd(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinEnumVKeyScanIDs(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLoadTranslationTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetKbdLayout2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DebugInputEnable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DebugInput(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DebugOutput(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DebugCurPos(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DebugCls(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DebugRedirect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLangsupWindowProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLangsupDlgProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinPortHoleUtil(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinCreateFarHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinDestroyFarHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinAllocFarMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinFreeFarMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinReallocFarMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetQueueProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FarFlushBufHook(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY fnBadAppDlgProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InputGetDeviceInfo16(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY lpfnIconWndProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY lpfnShellWndProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SheStartLongProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SheSystemShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SheUpdateIniFile(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShlLoadPublicFonts(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShlUnloadPublicFonts(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShpLockProgramList(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShpUnlockProgramList(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY StartStartUpDotCmd(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY StartSystemExecutables(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinCancelMode(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinGetMinWindowViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinInitializeSystem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLockStartInput(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinResetSystem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinScreenSwitch(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinTerminateShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinAddProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinAddToGroup(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinCallHelpHook(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinCanTerminate(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinChangeProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinCplRegister(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinCreateGroup(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinDebugBreak(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinDestroyAniPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinDestroyGroup(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinEndProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinEndSession(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinEndWindowSession(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinExplodeWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinForceMenuRedraw(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinFormatFrame(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinGetFrameTreePPs(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinGetFrameTreePPSize(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinGetScanState(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinHAPPfromPID(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinHSWITCHfromHAPP(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinInitializePL(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinInitSession(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinInitSessionMgr(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinInitSwEntry(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinInstStartApp(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinIsShuttingDown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinJiggleMouse(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLoadAniPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinLockInput(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinMenuIndexFromID(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinNoShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinPMFileRegister(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryAccelTableValue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryCharXlatetbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryDefinition(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryExtIDFocus(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryFileExtOpts(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryMenuData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProgramTitles(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProgramType(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProgramUse(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQuerySendMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryState(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryWindowSGID(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryWorkplaceWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueueFromID(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinRegisterSeamlessProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinRegisterWindowDestroy(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinRemoveFromGroup(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinRemoveProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinReplyMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSeamlessBox(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSemEnter(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSemLeave(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSemQuery(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSendQueueMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetAccelTableValue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetAniPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetAtomTableOwner(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetCharXlatetbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetExtIDFocus(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetFgndWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetFileExtOpts(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetFrameTreePPs(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetMenuData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetScanStateTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetSwEntry(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetSysClipbrdViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetTitle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetTitleAndIcon(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetWindowDC(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinStopProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinStpRegister(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSwitchToProgram2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSwitchToTaskManager(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSyncStartPgm(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSystemShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinThreadAssocQueue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinTrackSeamless(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinTranslateChar(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinVkeyGlyphCharXlate(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinWakeThread(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetTitleAndHwndIcon(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinTrackWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetLangInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryLangInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetKbdLayer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryKbdLayer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetLangViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryLangViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinPourEvent(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetMsgQimInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryMsgQimInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Devmode_Call_16(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Devname_Call_16(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch16Gate10(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch16Gate3(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch16Gate4(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch16Gate5(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch16Gate6(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch16Gate7(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch16Gate8(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch16Gate9(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Enable_Call_16(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FsrSemEnter(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FsrSemEnterP(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FsrSemLeave(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PFN10R2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PFN4R2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PFN5R2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PFN6R2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PFN7R2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PFN8R2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PFN9R2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DeleteBitmap16Call(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Do16CLI(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Do16STI(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GRE16Callback(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Dispatch32Gate(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GRE16CallForwardReturn(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FirePSProbe(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY AAB32GETREG(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY AAB32SETREG(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY AbnormalProcessExit(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DEBUG32CLS(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DEBUG32CURPOS(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DEBUG32INPUT(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DEBUG32OUTPUT(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DestroyTimers(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EDITAllocSpace(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditAutoSize(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditBackSpace(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditChangeFocus(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditChangeSelection(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditChar(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditCheckScroll(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditClear(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditCopy(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditCreate(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditCreateCursor(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditCreatePassword(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditCut(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditCXFromICH(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditDrawSubString(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditGetFlags(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditGetWindowRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditICHFromPt(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditNotify(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditPaint(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditPaste(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditPtFromICH(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditQueryPresParams(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditReplace(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditSetCursorPos(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditSetFirstIndex(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditSetInsertMode(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditSetSelection(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditSetText(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditSetWindowParams(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditShowChange(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditStopMouseTracking(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditTimerScroll(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditTrackMouse(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditVirtualKey(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EditWMChar(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Far32LockupHook(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FarCallHook(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GetDriverInfo32(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gpi32LoadBitmap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry10(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry3(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry4(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry5(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry6(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry7(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry8(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Gre32Entry9(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GetIconWindowsEXE(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GetIconWindowsICO(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry10(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry3(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry4(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry5(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry6(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry7(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry8(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InnerGre32Entry9(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InputGetDeviceInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PM32ExecRegister(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PMReleaseMutexSem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PMRequestMutexSem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PMRequestMutexSemSetWait(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PostDeviceModes32(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32AddProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32ChangeProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32ChangeWriteThru(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32CloseProfile(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32CreateGroup(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32DestroyGroup(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32OpenProfile(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryDefinition(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryProfile(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryProfileData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryProfileInt(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryProfileSize(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryProfileString(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryProgramCategory(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryProgramHandle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32QueryProgramTitles(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32RemoveProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32Reset(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32WriteProfileData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Prf32WriteProfileString(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PrfSuspendWriteToDisk(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PrfWriteToDisk(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY RamSemClear32(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY RamSemRequest32(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SetDriverInfo32(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY She32PIInitIniFiles(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY She32SystemShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShpPILockIniFile(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShpPIShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SLEC_InitPen(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SSAllocMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SSFreeMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Start32SystemExecutables(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY StartTimer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY SetDeviceSurface(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ThkDos16SetFgnd(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY TidCurrent(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32AddAtom(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32AddSwitchEntry(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32Alarm(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32BeginEnumWindows(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32BeginPaint(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32BroadcastMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32BuildPtrHandle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CalcFrameRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CallHelpHook(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CallMsgFilter(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CancelShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ChangeSwitchEntry(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CheckInput(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CloseClipbrd(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CompareStrings(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CopyAccelTable(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32CopyRect(void)


APIRET APIENTRY Win32CpTranslateChar(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CpTranslateString(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CreateAccelTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CreateAtomTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CreateCursor(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CreateDlg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CreateFrameControls(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CreateMenu(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32CreateMsgQueue(void);


APIRET APIENTRY Win32CreatePointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32CreatePointerIndirect(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32CreateStdWindow(void);


APIRET APIENTRY Win32CreateSwitchEntry(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32CreateWindow(void);


APIRET APIENTRY Win32DdeInitiate(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DdePostMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DdeRespond(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DebugBreak(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DefDlgProc(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32DefWindowProc(void);


APIRET APIENTRY Win32DeleteAtom(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32DeleteLibrary(void);


//APIRET APIENTRY Win32DeleteProcedure(void);


APIRET APIENTRY Win32DestroyAccelTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DestroyAtomTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DestroyCursor(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32DestroyMsgQueue(void);


APIRET APIENTRY Win32DestroyPointer(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32DestroyWindow(void);


APIRET APIENTRY Win32DismissDlg(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32DispatchMsg(void);


APIRET APIENTRY Win32DlgBox(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DrawBitmap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DrawBorder(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DrawPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32DrawText(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32EmptyClipbrd(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32EnablePhysInput(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32EnableWindow(void);


//APIRET APIENTRY Win32EnableWindowUpdate(void);


APIRET APIENTRY Win32EndEnumWindows(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32EndPaint(void);


APIRET APIENTRY Win32EndProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32EnumClipbrdFmts(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32EnumDlgItem(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32EqualRect(void);


APIRET APIENTRY Win32ExcludeUpdateRegion(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ExplodeWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32FillRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32FindAtom(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32FlashWindow(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32FocusChange(void);


//APIRET APIENTRY Win32FreeErrorInfo(void);


APIRET APIENTRY Win32GetClipPS(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetCurrentTime(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetDlgMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetErasePS(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32GetErrorInfo(void);


APIRET APIENTRY Win32GetFrameTreePPs(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetFrameTreePPSize(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetKeyState(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32GetLastError(void);


APIRET APIENTRY Win32GetMaxPosition(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetMinPosition(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32GetMsg(void);


APIRET APIENTRY Win32GetNextWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetPhysKeyState(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32GetPS(void);


APIRET APIENTRY Win32GetScreenPS(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetSysBitmap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32HAPPfromPID(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32HSWITCHfromHAPP(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32InflateRect(void);


//APIRET APIENTRY Win32Initialize(void);


APIRET APIENTRY Win32InSendMsg(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32IntersectRect(void);


APIRET APIENTRY Win32InvalidateRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32InvalidateRegion(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32InvertRect(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32IsChild(void);


APIRET APIENTRY Win32IsPhysInputEnabled(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32IsRectEmpty(void);


APIRET APIENTRY Win32IsThreadActive(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32IsWindow(void);


//APIRET APIENTRY Win32IsWindowEnabled(void);


APIRET APIENTRY Win32IsWindowShowing(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32IsWindowVisible(void);


APIRET APIENTRY Win32JiggleMouse(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LoadAccelTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LoadCharXlatetbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LoadDlg(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32LoadLibrary(void);


APIRET APIENTRY Win32LoadMenu(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LoadMessage(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LoadPointer(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32LoadProcedure(void);


APIRET APIENTRY Win32LoadString(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LockInput(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LockPointerUpdate(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LockVisRegions(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LockWindowUpdate(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32MakePoints(void);


//APIRET APIENTRY Win32MakeRect(void);


APIRET APIENTRY Win32MapDlgPoints(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32MapWindowPoints(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32MessageBox(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32MoveCursor(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32MsgMuxSemWait(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32MsgSemWait(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32MultWindowFromIDs(void);


APIRET APIENTRY Win32NextChar(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32OffsetRect(void);


APIRET APIENTRY Win32OpenClipbrd(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32OpenWindowDC(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32PeekMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32PopupMenu(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32PostMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32PostQueueMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32PrevChar(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ProcessDlg(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32PtInRect(void);


APIRET APIENTRY Win32QueryAccelTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryAccelTableValue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryActiveWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryAnchorBlock(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryAtomLength(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryAtomName(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryAtomUsage(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryCapture(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClassInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClassName(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClassThunkProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClipbrdData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClipbrdFmtInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClipbrdOwner(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClipbrdViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryCp(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryCpList(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryCursorInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryDesktopBkgnd(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryDesktopWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryDlgItemShort(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryDlgItemText(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryDlgItemTextLength(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryFocus(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryMsgPos(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryMsgTime(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryObjectWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryPaletteMode(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryPointerInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryPointerPos(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryPresParam(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryProcessCP(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryQueueInfo(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryQueueStatus(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySendMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySessionTitle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySwitchEntry(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySwitchHandle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySwitchList(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySysColor(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySysModalWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySysPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySysPointerData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySystemAtomTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QuerySysValue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryTaskSizePos(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryTaskTitle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryUpdateRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryUpdateRegion(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryVersion(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryVisibleRegion(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32QueryWindow(void);


//APIRET APIENTRY Win32QueryWindowDC(void);


APIRET APIENTRY Win32QueryWindowModel(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32QueryWindowPos(void);


APIRET APIENTRY Win32QueryWindowProcess(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryWindowPtr(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32QueryWindowRect(void);


//APIRET APIENTRY Win32QueryWindowText(void);


//APIRET APIENTRY Win32QueryWindowTextLength(void);


APIRET APIENTRY Win32QueryWindowThunkProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryWindowULong(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryWindowUShort(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueueFromID(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32RealizePalette(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32RegisterClass(void);


APIRET APIENTRY Win32RegisterPen(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32RegisterSeamlessProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32RegisterUserDatatype(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32RegisterUserMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ReleaseErasePS(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ReleaseHook(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32ReleasePS(void);


APIRET APIENTRY Win32RemovePresParam(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32RemoveSwitchEntry(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ReplyMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32RequestMutexSem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SaveWindowPos(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ScrollWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SeamlessBox(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SemEnter(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SemLeave(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SemQuery(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SendDlgItemMsg(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SendMsg(void);


APIRET APIENTRY Win32SetAccelTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetAccelTableValue(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SetActiveWindow(void);


APIRET APIENTRY Win32SetCapture(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetCharXlateTbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetClassMsgInterest(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetClassThunkProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetClipbrdData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetClipbrdOwner(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetClipbrdViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetCp(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetDesktopBkgnd(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetDlgItemShort(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetDlgItemText(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetErrorInfo(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SetFocus(void);


APIRET APIENTRY Win32SetFrameTreePPs(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetHook(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetKeyboardStateTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetMsgInterest(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetMsgMode(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SetMultWindowPos(void);


//APIRET APIENTRY Win32SetOwner(void);


APIRET APIENTRY WinSetPaletteMode(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SetParent(void);


APIRET APIENTRY Win32SetPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetPointerOwner(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetPointerPos(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetPresParam(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SetRect(void);


//APIRET APIENTRY Win32SetRectEmpty(void);


APIRET APIENTRY Win32SetSynchroMode(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetSysClipbrdViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetSysColors(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetSysFont(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetSysModalWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetSysPointerData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetSysValue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetVisibleRegionNotify(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetWindowBits(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SetWindowPos(void);


APIRET APIENTRY Win32SetWindowPtr(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SetWindowText(void);


APIRET APIENTRY Win32SetWindowThunkProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetWindowULong(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetWindowUShort(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ShowCursor(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ShowPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ShowTrackRect(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32ShowWindow(void);


APIRET APIENTRY Win32StartApp(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32StartTimer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32StopTimer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32StretchPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SubclassWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SubstituteStrings(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32SubtractRect(void);


APIRET APIENTRY Win32SwitchProgramRegister(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SwitchToProgram(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SwitchToProgram2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SystemShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32Terminate(void);


APIRET APIENTRY Win32TerminateApp(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ThreadAssocQueue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32TrackRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32TrackSeamless(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32TranslateAccel(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32UnionRect(void);


APIRET APIENTRY Win32UpdateWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32Upper(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32UpperChar(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ValidateRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32ValidateRegion(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32VisRegionNotify(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32WaitEventSem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32WaitMsg(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32WaitMuxWaitSem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32WakeThread(void)
{
    return unimplemented(__FUNCTION__);
}


//APIRET APIENTRY Win32WindowFromDC(void);


//APIRET APIENTRY Win32WindowFromID(void);


APIRET APIENTRY Win32WindowFromPoint(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinAugmentPointer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinMspAlloc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinMspFree(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinMspInit(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinProcessHotKey(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinProfControl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXAllocFill(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXAllocMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXCreateHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXDestroyHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXFreeMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXIsMemInHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXReallocMem(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PMHeapIsMemInChain(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32NoShutdown(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinDrawTabbedText(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryPointerClipRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetPointerClipRect(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32WaitMsgTimeout(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32IsWindowOfClass(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetAtomTableOwner(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXQueryMemSize(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetClipRegion(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClipRegion(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32AddClipbrdViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32RemoveClipbrdViewer(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryClipbrdViewerChain(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32IncrementAtomUsage(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxQueryOpenClipbrdWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryDaxData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetDaxData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinEnumProperties(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryProperty(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinRemoveProperty(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetProperty(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinEnumClassProperties(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryClassProperty(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinRemoveClassProperty(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetClassProperty(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryHeapFlags(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetKbdLayout(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetDAXDefWndProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PostEscape32(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32GetAFCValue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetAFCValue(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32LoadVkeyGlyphXlateTbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SetVkeyGlyphXlateTbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryVkeyGlyphXlateTbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32TrackWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32SemEnterTimeout(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetDesktopWorkArea(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryDesktopWorkArea(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryControlColors(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinSetControlColors(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY CtlQueryColor(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32TranslateChar2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32WaitForIdleThread(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinXXCreateHeap2(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQuerySharedHeapData(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryActiveOtherWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32EnumVkeyScanIDs(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryKbdLayout(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY WinQueryCpType(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY Win32QueryCharXlatetbl(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY InitGameFrameProc(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreNLS(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ValidateDCHandle(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY HookMan(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY CCHSetHailing(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY EnableCoachMsgHook(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FCallGRECoach(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GRE32CallForward(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY GreMemptrs(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY FPen(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY ShellWindow(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PMSemaphores(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PHandleTable(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY PSharedHeap(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY VPAULDefaultColors(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxCreateCaret(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxDestroyCaret(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxGetCaretBlinkTime(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxHideCaret(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxSetCaretBlinkTime(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxShowCaret(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxShowCursor(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxSwapMouseButton(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY LookupAtom(void)
{
    return unimplemented(__FUNCTION__);
}


APIRET APIENTRY DaxOpenClipbrd(void)
{
    return unimplemented(__FUNCTION__);
}

} // extern "C"

