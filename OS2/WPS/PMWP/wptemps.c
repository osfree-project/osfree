/*
 *  This file was generated by the SOM Compiler and Emitter Framework.
 *  Generated using template emitter:
 *      SOM Emitter emitctm: 2.23.1.9
 */

#ifndef SOM_Module_wptemps_Source
#define SOM_Module_wptemps_Source
#endif
#define WPTemplates_Class_Source
#define M_WPTemplates_Class_Source

#define INCL_PMWP
#include <os2.h>

#include "wptemps.ih"


SOM_Scope BOOL  SOMLINK tfd_wpPopulate(WPTemplates *somSelf, 
                                       ULONG ulReserved, PSZ pszPath, 
                                       BOOL fFoldersOnly)
{
    /* WPTemplatesData *somThis = WPTemplatesGetData(somSelf); */
    WPTemplatesMethodDebug("WPTemplates","tfd_wpPopulate");

    return (WPTemplates_parent_WPFolder_wpPopulate(somSelf, ulReserved, 
                                                   pszPath, fFoldersOnly));
}

SOM_Scope BOOL  SOMLINK tfd_wpSetup(WPTemplates *somSelf, PSZ pszSetupString)
{
    /* WPTemplatesData *somThis = WPTemplatesGetData(somSelf); */
    WPTemplatesMethodDebug("WPTemplates","tfd_wpSetup");

    return (WPTemplates_parent_WPFolder_wpSetup(somSelf, pszSetupString));
}

SOM_Scope void SOMLINK tfd_somDefaultInit(WPTemplates *somSelf, 
                                          somInitCtrl* ctrl)
{
    WPTemplatesData *somThis; /* set in BeginInitializer */
    somInitCtrl globalCtrl;
    somBooleanVector myMask;
    WPTemplatesMethodDebug("WPTemplates","somDefaultInit");
    WPTemplates_BeginInitializer_somDefaultInit;

    WPTemplates_Init_WPFolder_somDefaultInit(somSelf, ctrl);

    /*
     * local WPTemplates initialization code added by programmer
     */
}


SOM_Scope void SOMLINK tfd_somDestruct(WPTemplates *somSelf, 
                                       octet doFree, somDestructCtrl* ctrl)
{
    WPTemplatesData *somThis; /* set in BeginDestructor */
    somDestructCtrl globalCtrl;
    somBooleanVector myMask;
    WPTemplatesMethodDebug("WPTemplates","tfd_somDestruct");
    WPTemplates_BeginDestructor;

    /*
     * local WPTemplates deinitialization code added by programmer
     */

    WPTemplates_EndDestructor;
}



SOM_Scope PSZ  SOMLINK tfdM_wpclsQueryTitle(M_WPTemplates *somSelf)
{
    /* M_WPTemplatesData *somThis = M_WPTemplatesGetData(somSelf); */
    M_WPTemplatesMethodDebug("M_WPTemplates","tfdM_wpclsQueryTitle");

    return (M_WPTemplates_parent_M_WPFolder_wpclsQueryTitle(somSelf));
}

SOM_Scope ULONG  SOMLINK tfdM_wpclsQueryIconData(M_WPTemplates *somSelf, 
                                                 PICONINFO pIconInfo)
{
    /* M_WPTemplatesData *somThis = M_WPTemplatesGetData(somSelf); */
    M_WPTemplatesMethodDebug("M_WPTemplates","tfdM_wpclsQueryIconData");

    return (M_WPTemplates_parent_M_WPFolder_wpclsQueryIconData(somSelf, 
                                                               pIconInfo));
}

SOM_Scope ULONG  SOMLINK tfdM_wpclsQueryIconDataN(M_WPTemplates *somSelf, 
                                                  ICONINFO* pIconInfo, 
                                                  ULONG ulIconIndex)
{
    /* M_WPTemplatesData *somThis = M_WPTemplatesGetData(somSelf); */
    M_WPTemplatesMethodDebug("M_WPTemplates","tfdM_wpclsQueryIconDataN");

    return (M_WPTemplates_parent_M_WPFolder_wpclsQueryIconDataN(somSelf, 
                                                                pIconInfo, 
                                                                ulIconIndex));
}

SOM_Scope ULONG  SOMLINK tfdM_wpclsQueryStyle(M_WPTemplates *somSelf)
{
    /* M_WPTemplatesData *somThis = M_WPTemplatesGetData(somSelf); */
    M_WPTemplatesMethodDebug("M_WPTemplates","tfdM_wpclsQueryStyle");

    return (M_WPTemplates_parent_M_WPFolder_wpclsQueryStyle(somSelf));
}

