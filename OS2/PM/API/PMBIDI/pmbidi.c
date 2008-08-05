#include <pmbidi.h>

VOID APIENTRY PMBIDI_Initialize(VOID);

VOID APIENTRY16 WIN16SETLANGINFO(VOID);

VOID APIENTRY16 WIN16QUERYLANGINFO(VOID);

VOID APIENTRY16 WIN16SETKBDLAYER(VOID);

VOID APIENTRY16 WIN16QUERYKBDLAYER(VOID);

VOID APIENTRY16 WIN16SETLANGVIEWER(VOID);

VOID APIENTRY16 WIN16QUERYLANGVIEWER(VOID);

VOID APIENTRY16 WIN16SETPROCESSLANGINFO(VOID);

VOID APIENTRY16 WIN16QUERYPROCESSLANGINFO(VOID);

   ULONG   APIENTRY Win32SetLangInfo     ( HWND    hwnd,
                                        ULONG   ulEffect,
                                        ULONG   ulData,
                                        ULONG   flMask,
                                        ULONG   flFlags,
                                        ULONG   ulReserved );

   ULONG   APIENTRY Win32QueryLangInfo   ( HWND    hwnd,
                                        ULONG   ulEffect,
                                        ULONG   flFlags,
                                        ULONG   ulReserved );

   ULONG   APIENTRY Win32SetKbdLayer     ( HWND hwnd,
                                        ULONG idKbdLayer,
                                        ULONG flFlags);

   ULONG   APIENTRY Win32QueryKbdLayer   ( HWND hwnd );

   HWND    APIENTRY Win32SetLangViewer   ( HAB hab,
                                        HWND hwndLangViewer,
                                        ULONG Codepage);

   HWND    APIENTRY Win32QueryLangViewer ( HAB hab,
                                        ULONG Codepage );

VOID APIENTRY Win32SetProcessLangInfo(VOID);

VOID APIENTRY Win32QueryProcessLangInfo(VOID);

VOID APIENTRY16 GPI16SETBIDIATTR(VOID);

VOID APIENTRY16 GPI16QUERYBIDIATTR(VOID);

   ULONG   APIENTRY Gpi32SetBidiAttr     ( HPS  hps,
                                        ULONG BidiAttr );
   ULONG   APIENTRY Gpi32QueryBidiAttr   ( HPS  hps );

VOID APIENTRY Bidi_QueryCp(VOID);

VOID APIENTRY16 BIDI_16QUERYCP(VOID);

VOID APIENTRY PMBIDI_Get_StdDlgLang_Setting(VOID);

VOID APIENTRY16 PMBIDI_16GET_STDDLGLANG_SETTING(VOID);

VOID APIENTRY PMBIDI_String_To_BinVal(VOID);

VOID APIENTRY PMBIDI_BinVal_To_String(VOID);

VOID APIENTRY PMBIDI_Keyword_To_BinVal(VOID);

VOID APIENTRY PMBIDI_Get_HelpBidi_Setting(VOID);

VOID APIENTRY16 NLS16CONVERTBIDISTRING(VOID);

VOID APIENTRY16 NLS16EDITSHAPE(VOID);

VOID APIENTRY16 NLS16INVERSESTRING(VOID);

VOID APIENTRY16 NLS16SHAPEBIDISTRING(VOID);

VOID APIENTRY16 NLS16CONVERTBIDINUMERICS(VOID);

VOID APIENTRY Nls32ConvertBidiString(VOID);

VOID APIENTRY Nls32EditShape(VOID);

VOID APIENTRY Nls32InverseString(VOID);

VOID APIENTRY Nls32ShapeBidiString(VOID);

VOID APIENTRY Nls32ConvertBidiNumerics(VOID);

VOID APIENTRY layout_object_create(VOID);

VOID APIENTRY layout_object_destroy(VOID);

VOID APIENTRY layout_object_transform(VOID);

VOID APIENTRY layout_object_editshape(VOID);

VOID APIENTRY layout_object_setvalues(VOID);

VOID APIENTRY layout_object_getvalues(VOID);

VOID APIENTRY16 PMBIDI_16CP_TRANSLATE_STRING(VOID);

VOID APIENTRY PMBIDI_CpTranslateString(VOID);

VOID APIENTRY Bidi_MapSrcToTrg(VOID);

VOID APIENTRY Bidi_ClassifyCodepage(VOID);

VOID APIENTRY Bidi_IsStringBidi(VOID);

VOID APIENTRY Bidi_IsStringAllBidi(VOID);

VOID APIENTRY Bidi_ReverseString(VOID);

VOID APIENTRY Bidi_LayoutConvert(VOID);

VOID APIENTRY Bidi_LayoutEdit(VOID);

VOID APIENTRY ulProcess_BidiAttr(VOID); // variable?

