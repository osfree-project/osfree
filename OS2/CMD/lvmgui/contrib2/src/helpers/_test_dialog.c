#define OS2EMX_PLAIN_CHAR
    // this is needed for "os2emx.h"; if this is defined,
    // emx will define PSZ as _signed_ char, otherwise
    // as unsigned char

#define INCL_DOSNLS
#define INCL_DOSERRORS

#define INCL_WINWINDOWMGR
#define INCL_WINFRAMEMGR
#define INCL_WINDIALOGS
#define INCL_WININPUT
#define INCL_WINSTATICS
#define INCL_WINBUTTONS
#define INCL_WINLISTBOXES
#define INCL_WINENTRYFIELDS
#define INCL_WINMLE
#define INCL_WINSYS
#define INCL_WINCOUNTRY

#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#define INCL_GPILCIDS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "setup.h"                      // code generation and debugging options

#include "K:\projects\cvs\xworkplace\include\dlgids.h"

#include "helpers\comctl.h"
#include "helpers\dialog.h"
#include "helpers\gpih.h"
#include "helpers\linklist.h"
#include "helpers\standards.h"
#include "helpers\stringh.h"
#include "helpers\textview.h"
#include "helpers\textv_html.h"
#include "helpers\winh.h"
#include "helpers\xstring.h"
#include "helpers\tree.h"
#include "helpers\tmsgfile.h"

#define TEXT_WIDTH  120
#define EF_WIDTH    200
#define EF_HEIGHT   -1

#define ID_TEXTVIEW     1000

#define TEXT_AND_ENTRYFIELD(varname, str) \
            varname ## Text = CONTROLDEF_TEXT(str, -1, TEXT_WIDTH, -1), \
            varname ## EF = CONTROLDEF_ENTRYFIELD_RO(NULL, -1, EF_WIDTH, EF_HEIGHT)

// #define TRY_FILEOPS 1

CONTROLDEF
    G_OKButton = CONTROLDEF_DEFPUSHBUTTON(
                            "~OK",
                            DID_OK,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    G_CancelButton = CONTROLDEF_PUSHBUTTON(
                            "~Cancel",
                            DID_CANCEL,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    G_UndoButton = CONTROLDEF_PUSHBUTTON(
                            "Undo",
                            DID_UNDO,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    G_DefaultButton = CONTROLDEF_PUSHBUTTON(
                            "~Default",
                            DID_DEFAULT,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    G_HelpButton = CONTROLDEF_HELPPUSHBUTTON(
                            "~Help",
                            DID_HELP,
                            STD_BUTTON_WIDTH,
                            STD_BUTTON_HEIGHT),
    G_Spacing = CONTROLDEF_TEXT(
                            NULL,
                            -1,
                            8,
                            1);

#ifndef TRY_FILEOPS

CONTROLDEF
    Expl1 = CONTROLDEF_TEXT("green is column box", -1, -1, -1),
    Expl2 = CONTROLDEF_TEXT("red is column content", -1, -1, -1),
    Expl3 = CONTROLDEF_TEXT("blue is table box", -1, -1, -1),
    DlgUnits = CONTROLDEF_TEXT(NULL, -1, -1, -1),
    Checkbox = CONTROLDEF_AUTOCHECKBOX("Test Checkbox", -1, -1, -1),
    Checkbox1 = CONTROLDEF_AUTOCHECKBOX("CB1", -1, -1, -1),
    Checkbox2 = CONTROLDEF_AUTOCHECKBOX("Test Checkbox 2 longer", -1, -1, -1),
    Checkbox3 = CONTROLDEF_AUTOCHECKBOX("asd”flijwet ", -1, -1, -1),
    Checkbox4 = CONTROLDEF_AUTOCHECKBOX("blah blah", -1, -1, -1),
    Checkbox5 = CONTROLDEF_AUTOCHECKBOX("whatever", -1, -1, -1),
    Checkbox6 = CONTROLDEF_AUTOCHECKBOX("another test checkbox", -1, -1, -1),
    Checkbox7 = CONTROLDEF_AUTOCHECKBOX("more whatever", -1, -1, -1),
    Checkbox8 = CONTROLDEF_AUTOCHECKBOX("yet another test checkbox", -1, -1, -1),
    MainGroup = CONTROLDEF_GROUP(
                        "Group",
                        -1,
                        -1,
                        -1),
    OKButton = CONTROLDEF_DEFPUSHBUTTON(
                        "Auto-size ~OK button",
                        DID_OK,
                        -1,
                        -1),
    FixedOKButton = CONTROLDEF_DEFPUSHBUTTON(
                        "~OK",
                        DID_OK,
                        STD_BUTTON_WIDTH,
                        STD_BUTTON_HEIGHT),
    CancelButton = CONTROLDEF_PUSHBUTTON(
                        "~Cancel",
                        DID_CANCEL,
                        -1,
                        -1),
    Text1 = CONTROLDEF_TEXT_WORDBREAK(
                        "This table's width is determined by the combined width "
                            "of the OK and Cancel buttons.",
                        -1,
                        -100),
    Text2 = CONTROLDEF_TEXT_WORDBREAK(
                        "widebutton is as wide as those two buttons.",
                        -1,
                        -100),
    WideButton = CONTROLDEF_PUSHBUTTON(
                        "widebutton",
                        DID_CANCEL,
                        -100,
                        50),
    Text3 = CONTROLDEF_TEXT_WORDBREAK(
                        "The following two are 60% and 40% of the table width.",
                        -1,
                        -100),
    HalfWideButton1 = CONTROLDEF_PUSHBUTTON(
                        "half",
                        DID_CANCEL,
                        -60,
                        50),
    HalfWideButton2 = CONTROLDEF_PUSHBUTTON(
                        "half",
                        DID_CANCEL,
                        -40,
                        50),
    Text4 = CONTROLDEF_TEXT_WORDBREAK(
                        "highbutton is 50% of the row width.",
                        -1,
                        -100),
    HighButton = CONTROLDEF_PUSHBUTTON(
                        "highbutton",
                        DID_CANCEL,
                        100,
                        -50),
    Text400 = CONTROLDEF_TEXT_WORDBREAK(
                        "This text is 400 pixels wide. The entry field below should be too.",
                        -1,
                        400),
    TextAuto = CONTROLDEF_TEXT(
                        "Autosize text.",
                        -1,
                        -1,
                        -1),
    RemainderEF = CONTROLDEF_ENTRYFIELD(
                        "Entry field that fills remaining space.",
                        -1,
                        SZL_REMAINDER,
                        SZL_AUTOSIZE),
    Button100 = CONTROLDEF_PUSHBUTTON(
                        "100 pixels?",
                        -1,
                        100,
                        SZL_AUTOSIZE),
    TEXT_AND_ENTRYFIELD(SummaryName,
                        "~Name:"),
    EF200 = CONTROLDEF_ENTRYFIELD(
                        "200 pixels?",
                        -1,
                        200,
                        30),
    MLE = CONTROLDEF_MLE(
                        "200 pixels?",
                        -1,
                        200,
                        30),
    TestGroup1 = CONTROLDEF_GROUP(
                        "group fixed at 400 + (2 * COMMON_SPACING)",
                        -1,
                        400 + (2 * COMMON_SPACING),
                        -1),
    TestGroup2 = CONTROLDEF_GROUP(
                        "auto size group",
                        -1,
                        -1,
                        -1),
    TestGroup3 = CONTROLDEF_GROUP(
                        "Auto-size group with columns aligned",
                        -1,
                        -1,
                        -1),
    TestGroup4 = CONTROLDEF_GROUP(
                        "Group that inherits size from parent table",
                        -1,
                        -1,
                        -1),
    StaticFullWidth = CONTROLDEF_TEXT(
                        "This static has a width of 100%.",
                        -1,
                        -100,
                        SZL_AUTOSIZE),
    StaticMLFullWidth = CONTROLDEF_TEXT_WORDBREAK(
                        "This multi-line static has a width of 100% and breaks properly "
                        "over several lines still. Its height is determined automatically.",
                        -1,
                        -100),
    StaticHalfWidth = CONTROLDEF_TEXT(
                        "This static has a width of 50%.",
                        -1,
                        -50,
                        SZL_AUTOSIZE),
    DropDown = CONTROLDEF_DROPDOWN(
                        -1,
                        -100,
                        200
                        ),
    TextView =
#if 1
        CONTROLDEF_XTEXTVIEW_HTML(NULL, ID_TEXTVIEW, -100, NULL)
#else
        {
            WC_XTEXTVIEW,
            NULL,
            WS_VISIBLE | XS_FULLSCROLL | XS_WORDWRAP | XS_HTML,
            ID_TEXTVIEW,
            CTL_COMMON_FONT,
            0,
            {-100, 100},
            COMMON_SPACING,
            NULL
        }
#endif
    ;

#define ICON_WIDTH          40
#define BUTTON_WIDTH        45
#define GROUPS_WIDTH       175
#undef EF_HEIGHT
#define EF_HEIGHT           25
#define HOTKEY_EF_WIDTH     50

STATIC const CONTROLDEF
    TitleGroup = CONTROLDEF_GROUP("T~itle",
                            ID_XSDI_ICON_TITLE_TEXT,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
    TitleEF = CONTROLDEF_MLE(
                            NULL,
                            ID_XSDI_ICON_TITLE_EF,
                            -100, // GROUPS_WIDTH - 2 * COMMON_SPACING,
                            MAKE_SQUARE_CY(EF_HEIGHT)),
    IconGroup = CONTROLDEF_GROUP(
                            "Icon",
                            ID_XSDI_ICON_GROUP,
                            GROUPS_WIDTH,
                            -1),
    IconStatic =
        {
            WC_STATIC,
            NULL,
            WS_VISIBLE | SS_TEXT | DT_LEFT | DT_VCENTER | DT_MNEMONIC,
            ID_XSDI_ICON_STATIC,
            CTL_COMMON_FONT,
            {ICON_WIDTH, MAKE_SQUARE_CY(ICON_WIDTH)},
            COMMON_SPACING
        },
    IconExplanationText = CONTROLDEF_TEXT_WORDBREAK(
                            "Drag any icon to the rectangle on the left, or find an "
                            "icon file using the \"Browse\" button.",
                            ID_XSDI_ICON_EXPLANATION_TXT,
                            -100),      // use table width
    IconEditButton = CONTROLDEF_PUSHBUTTON(
                            "~Edit icon",
                            ID_XSDI_ICON_EDIT_BUTTON,
                            -1,
                            STD_BUTTON_HEIGHT),
    IconBrowseButton = CONTROLDEF_PUSHBUTTON(
                            "Bro~wse...",
                            DID_BROWSE,
                            -1,
                            STD_BUTTON_HEIGHT),
    IconResetButton = CONTROLDEF_PUSHBUTTON(
                            "Reset icon",
                            ID_XSDI_ICON_RESET_BUTTON,
                            -1,
                            STD_BUTTON_HEIGHT),
    ExtrasGroup = CONTROLDEF_GROUP(
                            "Extras",
                            ID_XSDI_ICON_EXTRAS_GROUP,
                            GROUPS_WIDTH,
                            -1),
    HotkeyText = CONTROLDEF_TEXT(
                            "Object hot~key:",
                            ID_XSDI_ICON_HOTKEY_TEXT,
                            -1,
                            -1),
    HotkeyEF = CONTROLDEF_ENTRYFIELD(
                            NULL,
                            ID_XSDI_ICON_HOTKEY_EF,
                            HOTKEY_EF_WIDTH,
                            -1),
    HotkeyClearButton = CONTROLDEF_PUSHBUTTON(
                            "~Clear",
                            ID_XSDI_ICON_HOTKEY_CLEAR,
                            -1,
                            STD_BUTTON_HEIGHT),
    HotkeySetButton = CONTROLDEF_PUSHBUTTON(
                            "~Set",
                            ID_XSDI_ICON_HOTKEY_SET,
                            -1,
                            2 * STD_BUTTON_HEIGHT),
    LockPositionCB = CONTROLDEF_AUTOCHECKBOX(
                            "~Lock in place",
                            ID_XSDI_ICON_LOCKPOSITION_CB,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
    TemplateCB = CONTROLDEF_AUTOCHECKBOX(
                            "~Template",
                            ID_XSDI_ICON_TEMPLATE_CB,
                            SZL_AUTOSIZE,
                            SZL_AUTOSIZE),
    DetailsButton = CONTROLDEF_PUSHBUTTON(
                            "Det~ails...",
                            DID_DETAILS,
                            -1,
                            STD_BUTTON_HEIGHT);

/*
DLGHITEM dlgTest[] =
    {
        START_TABLE,            // root table, required
            START_ROW(ROW_VALIGN_CENTER),
                START_GROUP_TABLE_EXT(&TitleGroup, TABLE_INHERIT_SIZE),
                    START_ROW(0),
                        CONTROL_DEF(&TitleEF),
                END_TABLE,
            START_ROW(ROW_VALIGN_CENTER),
                START_GROUP_TABLE_EXT(&ExtrasGroup, TABLE_INHERIT_SIZE),
                    START_ROW(0),
                        START_ROW(ROW_VALIGN_CENTER),
                            CONTROL_DEF(&HotkeyText),
                            CONTROL_DEF(&HotkeyEF),
                            CONTROL_DEF(&HotkeySetButton),
                            CONTROL_DEF(&HotkeyClearButton),
                        START_ROW(ROW_VALIGN_CENTER),
                            CONTROL_DEF(&TemplateCB),
                            CONTROL_DEF(&LockPositionCB),
                END_TABLE,
            START_ROW(ROW_VALIGN_CENTER),
                START_GROUP_TABLE_EXT(&IconGroup, TABLE_INHERIT_SIZE),
                    START_ROW(0),
                        CONTROL_DEF(&IconStatic),
                    START_TABLE,
                        START_ROW(0),
                            CONTROL_DEF(&IconExplanationText),
                        START_ROW(0),
                            CONTROL_DEF(&IconEditButton),
                            CONTROL_DEF(&IconBrowseButton),
                            CONTROL_DEF(&IconResetButton),
                    END_TABLE,
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&DetailsButton),
            START_ROW(0),
                CONTROL_DEF(&G_UndoButton),         // common.c
                CONTROL_DEF(&G_DefaultButton),      // common.c
                CONTROL_DEF(&G_HelpButton),         // common.c
        END_TABLE
    };
*/

DLGHITEM dlgTest[] =
    {
        START_TABLE,
            START_ROW(0),
#if 1
                START_GROUP_TABLE_ALIGN(&TestGroup3),
#else
                START_TABLE_ALIGN,
#endif
                    START_ROW(0),
                        CONTROL_DEF(&Checkbox1),
                        CONTROL_DEF(&Checkbox2),
                    START_ROW(0),
                        CONTROL_DEF(&Checkbox3),
                        CONTROL_DEF(&Checkbox4),
                    START_ROW(0),
                        CONTROL_DEF(&Checkbox5),
                        CONTROL_DEF(&Checkbox6),
                    START_ROW(0),
                        CONTROL_DEF(&Checkbox7),
                        CONTROL_DEF(&Checkbox8),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&DropDown),
#if 1
            START_ROW(0),
                START_GROUP_TABLE(&TestGroup2),
                    START_ROW(0),
                        CONTROL_DEF(&Expl1),
                        CONTROL_DEF(&Expl2),
                        CONTROL_DEF(&Expl3),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&DlgUnits),
#endif
            START_ROW(0),
                START_GROUP_TABLE_EXT(&TestGroup4, TABLE_INHERIT_SIZE),
                    START_ROW(0),
                        CONTROL_DEF(&StaticFullWidth),
                    START_ROW(0),
                        CONTROL_DEF(&StaticMLFullWidth),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&RemainderEF),
                    START_ROW(ROW_VALIGN_CENTER),
                        CONTROL_DEF(&TextAuto),
                        CONTROL_DEF(&RemainderEF),
                        CONTROL_DEF(&RemainderEF),
                END_TABLE,
            START_ROW(0),
                CONTROL_DEF(&OKButton),
        END_TABLE
    };

#else // TRY_FILEOPS

#define CX_COL1             60
#define CX_COLDATE          40
#define CX_COLTIME          40
#define CX_COLSIZE          50

#define CX_INTRO   (CX_COL1 + CX_COLDATE + CX_COLTIME + CX_COLSIZE + 6 * COMMON_SPACING)

STATIC CONTROLDEF
    ClashIntro = CONTROLDEF_TEXT_WORDBREAK("An object with the title blah already exists in the folder blup.",
                ID_XFDI_CLASH_TXT1, -100),
    ClashOldObject = CONTROLDEF_TEXT("old object", ID_XFDI_CLASH_OLDOBJECT, CX_COL1, SZL_AUTOSIZE),
    ClashDateOld = CONTROLDEF_TEXT("99.99.9999 ", ID_XFDI_CLASH_DATEOLD, SZL_AUTOSIZE, SZL_AUTOSIZE),
    ClashTimeOld = CONTROLDEF_TEXT("99:99:99 ", ID_XFDI_CLASH_TIMEOLD, SZL_AUTOSIZE, SZL_AUTOSIZE),
    ClashSizeOld = CONTROLDEF_TEXT_RIGHT("old size", ID_XFDI_CLASH_SIZEOLD, CX_COLSIZE, SZL_AUTOSIZE),
    ClashNewObject = CONTROLDEF_TEXT("new object", ID_XFDI_CLASH_NEWOBJECT, CX_COL1, SZL_AUTOSIZE),
    ClashDateNew = CONTROLDEF_TEXT("99.99.9999 ", ID_XFDI_CLASH_DATENEW, SZL_AUTOSIZE, SZL_AUTOSIZE),
    ClashTimeNew = CONTROLDEF_TEXT("99:99:99 ", ID_XFDI_CLASH_TIMENEW, SZL_AUTOSIZE, SZL_AUTOSIZE),
    ClashSizeNew = CONTROLDEF_TEXT_RIGHT("new size", ID_XFDI_CLASH_SIZENEW, CX_COLSIZE, SZL_AUTOSIZE),

    ClashReplaceRadio = CONTROLDEF_FIRST_AUTORADIO("replace", ID_XFDI_CLASH_REPLACE, -60, SZL_AUTOSIZE),
    ClashRenameNewRadio = CONTROLDEF_NEXT_AUTORADIO("rename new", ID_XFDI_CLASH_RENAMENEW, -60, SZL_AUTOSIZE),
    ClashRenameNewEF = CONTROLDEF_ENTRYFIELD("M", ID_XFDI_CLASH_RENAMENEWTXT, -40, SZL_AUTOSIZE),
    ClashRenameOldRadio = CONTROLDEF_NEXT_AUTORADIO("rename old", ID_XFDI_CLASH_RENAMEOLD, -60, SZL_AUTOSIZE),
    ClashRenameOldEF = CONTROLDEF_ENTRYFIELD("M", ID_XFDI_CLASH_RENAMEOLDTXT, -40, SZL_AUTOSIZE);

STATIC const DLGHITEM dlgTest[] =
    {
        START_TABLE,
            START_ROW(0),
                CONTROL_DEF(&ClashIntro),
            START_ROW(0),
                CONTROL_DEF(&ClashOldObject),
                CONTROL_DEF(&ClashDateOld),
                CONTROL_DEF(&ClashTimeOld),
                CONTROL_DEF(&ClashSizeOld),
            START_ROW(0),
                CONTROL_DEF(&ClashNewObject),
                CONTROL_DEF(&ClashDateNew),
                CONTROL_DEF(&ClashTimeNew),
                CONTROL_DEF(&ClashSizeNew),
            START_ROW(0),
                CONTROL_DEF(&ClashReplaceRadio),
            START_ROW(0),
                CONTROL_DEF(&ClashRenameNewRadio),
                CONTROL_DEF(&ClashRenameNewEF),
            START_ROW(0),
                CONTROL_DEF(&ClashRenameOldRadio),
                CONTROL_DEF(&ClashRenameOldEF),
            START_ROW(0),
                CONTROL_DEF(&G_OKButton),
                CONTROL_DEF(&G_CancelButton),
                CONTROL_DEF(&G_HelpButton),
        END_TABLE
    };

#endif // TRY_FILEOPS

MRESULT EXPENTRY fnwpTest(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    MRESULT mrc = 0;

    switch (msg)
    {
        case WM_CONTROL:
            if (    (SHORT1FROMMP(mp1) == ID_TEXTVIEW)
                 && (SHORT2FROMMP(mp1) == TXVN_LINK)
               )
            {
                WinMessageBox(HWND_DESKTOP,
                              hwnd,
                              (PSZ)mp2,
                              "Anchor clicked",
                              0,
                              MB_OK | MB_MOVEABLE);
            }
        break;

        default:
            mrc = WinDefDlgProc(hwnd, msg, mp1, mp2);
    }

    return mrc;
}

/*
 *@@ XWPENTITY:
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 */

typedef struct _XWPENTITY
{
    PCSZ    pcszEntity;
    PCSZ    *ppcszString;
} XWPENTITY, *PXWPENTITY;

typedef const struct _XWPENTITY *PCXWPENTITY;

STATIC CHAR     G_szCopyright[5] = "";

STATIC PCSZ     G_pcszBldlevel = "BLDLEVEL_VERSION",
                G_pcszBldDate = __DATE__,
                G_pcszNewLine = "\n",
                G_pcszNBSP = "\xFF",      // non-breaking space
                G_pcszContactUser = "CONTACT_ADDRESS_USER",
                G_pcszContactDev = "CONTACT_ADDRESS_DEVEL",
                G_pcszCopyChar = "\xB8",       // in codepage 850
                G_pcszCopyright = G_szCopyright,
                ENTITY_XWORKPLACE = "XWorkplace";

STATIC BOOL     G_fEntitiesHacked = FALSE;

STATIC const XWPENTITY G_aEntities[] =
    {
        "&copy;", &G_pcszCopyright,
        "&xwp;", &ENTITY_XWORKPLACE,
        "&version;", &G_pcszBldlevel,
        "&date;", &G_pcszBldDate,
        "&nl;", &G_pcszNewLine,
        "&nbsp;", &G_pcszNBSP,
        "&contact-user;", &G_pcszContactUser,
        "&contact-dev;", &G_pcszContactDev,
    };

STATIC PTMFMSGFILE      G_pXWPMsgFile = NULL;        // V0.9.16 (2001-10-08) [umoeller]

/*
 *@@ cmnInitEntities:
 *      called from initMain to initialize NLS-dependent
 *      parts of the entities.
 *
 *@@added V0.9.20 (2002-08-10) [umoeller]
 */

VOID cmnInitEntities(HAB hab)
{
    // get the current process codepage for the WPS
    ULONG acp[8];       // fixed V0.9.19 (2002-04-14) [umoeller], this needs an array
    ULONG cb = 0;
    APIRET arcCP;
    if (arcCP = DosQueryCp(sizeof(acp),
                           acp,
                           &cb))
        acp[0] = 437;

    if (acp[0] == 850)
        strcpy(G_szCopyright, G_pcszCopyChar);
    else
    {
        WinCpTranslateString(hab,
                             850,
                             (PSZ)G_pcszCopyChar,
                             acp[0],
                             sizeof(G_szCopyright),
                             G_szCopyright);
        if (G_szCopyright[0] == '\xFF')
            strcpy(G_szCopyright, "(C)");
    }

}

/*
 *@@ ReplaceEntities:
 *
 *@@added V0.9.16 (2001-09-29) [umoeller]
 */

STATIC ULONG ReplaceEntities(PXSTRING pstr)
{
    ULONG ul,
          rc = 0;

    for (ul = 0;
         ul < ARRAYITEMCOUNT(G_aEntities);
         ul++)
    {
        ULONG ulOfs = 0;
        PCXWPENTITY pThis = &G_aEntities[ul];
        while (xstrFindReplaceC(pstr,
                                &ulOfs,
                                pThis->pcszEntity,
                                *(pThis->ppcszString)))
            rc++;
    }

    return rc;
}

APIRET cmnGetMessageExt(PCSZ *pTable,     // in: replacement PSZ table or NULL
                        ULONG ulTable,     // in: size of that table or 0
                        PXSTRING pstr,     // in/out: string
                        PCSZ pcszMsgID)    // in: msg ID to retrieve
{
    APIRET  arc = NO_ERROR;
    BOOL fLocked = FALSE;

    if (!G_pXWPMsgFile)
    {
        // first call:
        // go load the XWP message file
        arc = tmfOpenMessageFile("I:\\eCS\\ewps\\help\\xfldr001.tmf",
                                 &G_pXWPMsgFile);
    }

    if (!arc)
    {
        arc = tmfGetMessage(G_pXWPMsgFile,
                            pcszMsgID,
                            pstr,
                            pTable,
                            ulTable);

        #ifdef DEBUG_LANGCODES
            _Pmpf(("  tmfGetMessage rc: %d", arc));
        #endif

        if (!arc)
            ReplaceEntities(pstr);
        else
        {
            CHAR sz[500];
            sprintf(sz,
                    "Message %s not found in %s, rc = %d",
                    pcszMsgID,
                    "I:\\eCS\\ewps\\help\\xfldr001.tmf",
                    arc);
            xstrcpy(pstr, sz, 0);
        }
    }

    return arc;
}

APIRET cmnGetMessage(PCSZ *pTable,     // in: replacement PSZ table or NULL
                     ULONG ulTable,     // in: size of that table or 0
                     PXSTRING pstr,     // in/out: string
                     ULONG ulMsgNumber) // in: msg number to retrieve
{
    CHAR szMessageName[40];
    // create string message identifier from ulMsgNumber
    sprintf(szMessageName, "XFL%04d", ulMsgNumber);

    return cmnGetMessageExt(pTable, ulTable, pstr, szMessageName);
}

int main (int argc, char *argv[])
{
    HAB             hab;
    HMQ             hmq;
    APIRET          arc;
    HWND            hwndDlg;
    XSTRING         strInfo;

    POINTL          ptl1024 = {100, 100};
    CHAR            sz[100];

    DosBeep(1000, 10);

    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(hab, 0);

    cmnInitEntities(hab);

    txvRegisterTextView(hab);

    xstrInit(&strInfo, 0);
    cmnGetMessage(NULL, 0,
                  &strInfo,
                  140);
/*
#if 0
    txvStripLinefeeds(&strInfo.psz, 4);
#else
    txvConvertFromHTML(&strInfo.psz,
                       NULL,
                       NULL,
                       NULL);
#endif
*/

#ifndef TRY_FILEOPS
    TextView.pcszText = strInfo.psz;

    WinMapDlgPoints(NULLHANDLE,
                    &ptl1024,
                    1,
                    TRUE);

    sprintf(sz, "Dialog units are (%d/%d)", ptl1024.x, ptl1024.y);
    DlgUnits.pcszText = sz;
#endif

    if (!(arc = dlghCreateDlg(&hwndDlg,
                              NULLHANDLE,
                              FCF_TITLEBAR | FCF_SYSMENU | FCF_DLGBORDER | FCF_NOBYTEALIGN,
                              fnwpTest,
                              "Test dialog",
                              dlgTest,
                              ARRAYITEMCOUNT(dlgTest),
                              NULL,
                              "9.WarpSans")))
    {
        winhCenterWindow(hwndDlg);
        WinProcessDlg(hwndDlg);
        WinDestroyWindow(hwndDlg);
    }

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);

    return 0;
}
