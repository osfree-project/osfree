#include <windows.h>

static bInit = FALSE;

static BOOL
TWIN_InitSystem(void)
{
    UINT uiCompatibility;
    TWINDRVCONFIG DrvConfig;

    if (!bInit) {
        /*
         *  Protect against multiple calls and recursive calls.  Various
         *  API locations can make a call to us, in addition to the
         *  normal startup path through main().  This is so that the
         *  API's that require this initialization can be called from
         *  constructors in the OWL and MFC libraries.  The API's must
         *  in turn call us, so make sure we only initialize once.
         */
        bInit = TRUE;

        /* Initialize the filesystem code */
        InitFileSystem();

        /* this initializes loadable software devices */
        TWIN_InitLSD();

        /* this gets low-level driver connections etc. */
        uiCompatibility = GetCompatibilityFlags(0);
        DrvConfig.dwDoubleClickTime = GetTwinInt(WCP_DBLCLICK);
        DrvConfig.lpDrvCallback = &TWIN_DrvCallback;

        PrivateInitDriver(MAKELONG(DSUBSYSTEM_INIT,DSUB_INITSYSTEM),
                uiCompatibility,1,&DrvConfig);

        /* set system metrics */
        /* we should set menu height, rather than hardcode at 25 */
        /* The rule of thumb is MULDIV(fontsize,7,4)         */
        /* Where fontsize is the actual font size, not points    */
        /* SetSystemMetrics(SM_CYCAPTION,7*12/4); */

        SetSystemMetrics(SM_CXSCREEN,DrvConfig.nScreenWidth);
        SetSystemMetrics(SM_CYSCREEN,DrvConfig.nScreenHeight);
        SetSystemMetrics(SM_CXFULLSCREEN,DrvConfig.nScreenHeight);
        SetSystemMetrics(SM_CYFULLSCREEN,DrvConfig.nScreenHeight-
                                GetSystemMetrics(SM_CYCAPTION));

        /* this initializes DC cache */
        GdiInitDC();

        /* this initializes the system color table */
        InitSysColors();

        /* this initializes MFS layer */
        MFS_INIT();

        /* this initializes the binary machinery */
        TWIN_InitializeBinaryCode();
    }
    return bInit;
}

BOOL TWIN_InitDriver()
{
#ifdef DRVTAB
    if ( DrvEntryTab == NULL ) {
        if ( (DrvEntryTab = (TWINDRVSUBPROC **) DriverBootstrap()) == (TWINDRVSUBPROC **)NULL ) {
#else
    if ( DrvEntry == NULL) {
        if ( (DrvEntry = DriverBootstrap()) == NULL) {
#endif
            return FALSE;
        }

        (void)TWIN_InitSystem();
    }

    return TRUE;
}
