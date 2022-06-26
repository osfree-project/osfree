
#ifndef AMPTRDEU_RCH
#define AMPTRDEU_RCH

#define __TITLE__                                "Animierter Mauszeiger fÅr OS/2"

//
//      Text for dialog controls
//

// Note: String-concatenation does not work for Strings with a leading "~"
#define IDT_IDTXT_NBPAGE                         "Mauszeiger"
#define IDT_IDTXT_GROUPBOX                       __TITLE__
#define IDT_IDTXT_FIND                           "Suchen"
#define IDT_IDTXT_LOADSET                        "Satz laden"
#define IDT_IDTXT_USEFORALL                      "FÅr alle Zeiger verwenden"
#define IDT_IDTXT_ANIMATEONLOAD                  "Animation aktivieren nach Laden"
#define IDT_IDTXT_ANIMATE                        "Animieren"
#define IDT_IDTXT_DEMO                           "Demo"
#define IDT_IDTXT_SETTINGS                       "Einstellungen"

#define IDT_IDDLG_DLG_CNRSETTINGS_230            "Einstellungen Animation"
#define IDT_IDDLG_DLG_CNRSETTINGS                "Einstellungen Animation"
#define IDT_IDDLG_GB_ANIMATIONPATH               "Pfad fÅr Animationsdateien"
#define IDT_IDDLG_GB_FRAMELENGTH                 "Standardwert Bilddauer"
#define IDT_IDDLG_ST_VALUE_MS                    "Wert in ms.:"
#define IDT_IDDLG_CB_USEFORALL                   "~" IDT_IDTXT_USEFORALL
#define IDT_IDDLG_GB_LOAD                        "Laden"
#define IDT_IDDLG_CB_ANIMATEONLOAD               "~" IDT_IDTXT_ANIMATEONLOAD
#define IDT_IDDLG_GB_HIDEPOINTER                 "Zeiger verbergen"
#define IDT_IDDLG_CB_HIDEPOINTER                 "Zeiger verbergen"
#define IDT_IDDLG_GB_HIDEPOINTERDELAY            "verzîgerungswert fÅr Zeiger verbergen"
#define IDT_IDDLG_GB_DRAGSETTYPE                 "Standard Datetyp fÅr Zeiger"
#define IDT_IDDLG_GB_DRAGPTRTYPE                 "Default filetype fÅr Zeigersatz"
#define IDT_IDDLG_GB_INITDELAY                   "Verzîgerung fÅr Initialisierung der Animation"
#define IDT_IDDLG_ST_VALUE_S                     "Wert in s.:"

//
// strings for the "Pointers" page
//

#define IDT_IDTAB_NBPAGE                         "Mauszei~ger"
#define IDT_IDDLG_PB_OK                          "OK"
#define IDT_IDDLG_PB_CANCEL                      "Abbruch"
#define IDT_IDDLG_PB_EDIT                        "Editieren..."
#define IDT_IDDLG_PB_FIND                        IDT_IDTXT_FIND "..."
#define IDT_IDDLG_PB_LOAD                        IDT_IDTXT_LOADSET "..."
#define IDT_IDDLG_PB_UNDO                        "~Widerrufen"
#define IDT_IDDLG_PB_DEFAULT                     "~Standard"
#define IDT_IDDLG_PB_HELP                        "Hilfe"
#define IDT_IDDLG_PB_CLOSE                       "~Close"
#define IDT_IDDLG_PB_RETURN                      "~ZurÅck"

#define IDT_IDTAB_NBANIMATION                    "~Animation"
#define IDT_IDTAB_NBHIDE                         "~Verbergen"
#define IDT_IDTAB_NBDRAGDROP                     "~Ziehen&ögergeben"
#define IDT_IDTAB_NBINIT                         "~Initialisierung"


#define IDT_IDDLG_DLG_ABOUT                      "Produktinformation"
#define IDT_IDDLG_DLG_ABOUT_TITLE                __TITLE__
#define IDT_IDDLG_DLG_ABOUT_VERSION              "Version " BLDLEVEL_VERSION
#define IDT_IDDLG_DLG_ABOUT_AUTHOR               "von " __AUTHOR__  " " __YEAR__
#define IDT_IDDLG_DLG_ABOUT_RIGHTS               "Alle Rechte vorbehalten."
#define IDT_IDDLG_DLG_ABOUT_NLS                  "National Language Support"
#define IDT_IDDLG_DLG_ABOUT_TRANSLATOR           "von Christian Langanke"
#define IDT_IDDLG_DLG_ABOUT_SENDTO               "Senden Sie Email an:"
#define IDT_IDDLG_DLG_ABOUT_EMAILADR             __EMAIL__

#define IDT_IDDLG_DLG_LOADSET                    "Bitte wÑhlen Sie aus dieser Liste aus:"
#define IDT_IDDLG_ST_FILTER                      "Anzuzeigende Resourcetypen auswÑhlen:"

#define IDT_IDMEN_ITEM_SETTINGS_230              "~Einstellungen"
#define IDT_IDMEN_ITEM_SETTINGS                  "~Einstellungen"
#define IDT_IDMEN_ITEM_HELP                      "~Hilfe"
#define IDT_IDMEN_ITEM_HELP_INDEX                "Hilfe~index"
#define IDT_IDMEN_ITEM_HELP_GENERAL              "~Erweiterte Hilfe"
#define IDT_IDMEN_ITEM_HELP_USING                "Hilfe fÅr ~Hilfefunktion"
#define IDT_IDMEN_ITEM_HELP_KEYS                 "Hilfe fÅr ~Tasten"
#define IDT_IDMEN_ITEM_HELP_ABOUT                "~Produktinformation"
#define IDT_IDMEN_ITEM_EDIT                      "~Editieren..."
#define IDT_IDMEN_ITEM_FIND                      "S~uchen..."
#define IDT_IDMEN_ITEM_SAVEAS                    "~Sichern unter..."
#define IDT_IDMEN_ITEM_DEFAULT                   "S~tandard"
#define IDT_IDMEN_ITEM_ANIMATE                   "~Animieren"

#define IDT_IDMEN_FOLDER_SETTINGS_230            IDT_IDMEN_ITEM_SETTINGS_230
#define IDT_IDMEN_FOLDER_SETTINGS                IDT_IDMEN_ITEM_SETTINGS
#define IDT_IDMEN_FOLDER_VIEW                    "~Anzeige"
#define IDT_IDMEN_FOLDER_VIEW_ICON               "~Symbolanzeige"
#define IDT_IDMEN_FOLDER_VIEW_DETAIL             "~Detailanzeige"
#define IDT_IDMEN_FOLDER_HELP                    IDT_IDMEN_ITEM_HELP
#define IDT_IDMEN_FOLDER_HELP_INDEX              IDT_IDMEN_ITEM_HELP_INDEX
#define IDT_IDMEN_FOLDER_HELP_GENERAL            IDT_IDMEN_ITEM_HELP_GENERAL
#define IDT_IDMEN_FOLDER_HELP_USING              IDT_IDMEN_ITEM_HELP_USING
#define IDT_IDMEN_FOLDER_HELP_KEYS               IDT_IDMEN_ITEM_HELP_KEYS
#define IDT_IDMEN_FOLDER_HELP_ABOUT              IDT_IDMEN_ITEM_HELP_ABOUT
#define IDT_IDMEN_FOLDER_FIND                    "Sa~tz laden..."
#define IDT_IDMEN_FOLDER_SAVEAS                  "Sichern ~unter..."
#define IDT_IDMEN_FOLDER_DEFAULT                 IDT_IDMEN_ITEM_DEFAULT
#define IDT_IDMEN_FOLDER_HIDEPOINTER             "~Zeiger verbergen"
#define IDT_IDMEN_FOLDER_BLACKWHITE              "Sch~warzwei·"
#define IDT_IDMEN_FOLDER_DEMO                    "~Demo"
#define IDT_IDMEN_FOLDER_ANIMATE                 "A~nimieren"

#define IDT_IDSTR_FILETYPE_DEFAULT               "Standard"
#define IDT_IDSTR_FILETYPE_POINTER               "OS/2 Zeiger"
#define IDT_IDSTR_FILETYPE_POINTERSET            "OS/2 Zeigersatz"
#define IDT_IDSTR_FILETYPE_CURSOR                "Win Cursor"
#define IDT_IDSTR_FILETYPE_CURSORSET             "Win Cursorsatz"
#define IDT_IDSTR_FILETYPE_ANIMOUSE              "AniMouse"
#define IDT_IDSTR_FILETYPE_ANIMOUSESET           "AniMouse Satz"
#define IDT_IDSTR_FILETYPE_ANMFILE               "AniMouse Scriptdatei"
#define IDT_IDSTR_FILETYPE_WINANIMATION          "Win Animation"
#define IDT_IDSTR_FILETYPE_WINANIMATIONSET       "Win Animationssatz"
#define IDT_IDSTR_FILETYPE_ANIMATIONSETDIR       "Animationssatz Verzeichnis"
#define IDT_IDSTR_FILETYPE_ALL                   "<Alle Typen>"

#define IDT_IDSTR_POINTER_ARROW                  "Pfeil"
#define IDT_IDSTR_POINTER_TEXT                   "Text"
#define IDT_IDSTR_POINTER_WAIT                   "Warten"
#define IDT_IDSTR_POINTER_SIZENWSE               "Grî·e NWSO"
#define IDT_IDSTR_POINTER_SIZEWE                 "Grî·e WO"
#define IDT_IDSTR_POINTER_MOVE                   "Verschieben"
#define IDT_IDSTR_POINTER_SIZENESW               "Grî·e NOSW"
#define IDT_IDSTR_POINTER_SIZENS                 "Grî·e NS"
#define IDT_IDSTR_POINTER_ILLEGAL                "UngÅltig"

#define IDT_IDSTR_TITLE_ICON                     "Symbol"
#define IDT_IDSTR_TITLE_NAME                     "Name"
#define IDT_IDSTR_TITLE_STATUS                   "Status"
#define IDT_IDSTR_TITLE_ANIMATIONTYPE            "Animationstyp"
#define IDT_IDSTR_TITLE_POINTER                  "Zeiger"
#define IDT_IDSTR_TITLE_ANIMATIONNAME            "Animationsname"
#define IDT_IDSTR_TITLE_FRAMERATE                "BildlÑngen"
#define IDT_IDSTR_TITLE_INFONAME                 "Name der Animation"
#define IDT_IDSTR_TITLE_INFOARTIST               "Author der Animation"
#define IDT_IDSTR_STATUS_ON                      "ein"
#define IDT_IDSTR_STATUS_OFF                     "aus"

//
//      Text for messages
//

#define IDT_IDMSG_TITLE_ERROR                    "Fehler"
#define IDT_IDMSG_ANIMATIONPATH_NOT_FOUND        "Der angegebene Pfad fÅr Animationsdateien existiert nicht."
#define IDT_IDMSG_TITLENOTFOUND                  "Suchergebnis"
#define IDT_IDMSG_MSGNOTFOUND                    "Der " IDT_IDDLG_GB_ANIMATIONPATH " enthÑlt keine Animationsressourcen."

//
//      some system related text
//

#define IDT_IDTXT_INFORMATION                    "Information"
#define IDT_IDTXT_SYSTEM_SETUP                   "Systemkonfiguration"
#define IDT_IDTXT_MOUSE_OBJECT                   "Mausobjekt"
#define IDT_IDTXT_ICON_EDITOR                    "Symbol Editor"
#define IDT_IDTXT_SELECTIVE_INSTALL              "Selektives Installieren"
#define IDT_IDTXT_OPTIONAL_SYSTEM_UTILITIES      "Systemdienstprogramme"
#define IDT_IDTXT_LINK_OBJECT_MODULES            "Objektmodule verbinden"

//
// General/Icon page replacement (see General/Icon page of any Desktop object of your OS/2 !)
//

// In english this page is called:
// - WARP 3: "General"
// - WARP 4: "Icon"
#define IDT_IDTAB_NBGENERAL_230                  "~General"
#define IDT_IDTAB_NBGENERAL                      "~Icon"
#define IDT_IDDLG_ST_TITLE                       "Title:"
#define IDT_IDDLG_ST_CONTENTS                    "Current icon"
#define IDT_IDDLG_ST_CONTENTS_ANIMATION          "Current animation"
#define IDT_IDDLG_CB_TEMPLATE                    "~Template"
#define IDT_IDDLG_CB_LOCKPOS                     "~Lock position"

//
// Strings for the context menus of fileclasses
//

#define IDT_IDMEN_CONVERT_SUBMENU                "### Umwandeln in..."

#define IDT_IDMEN_CONVERT_POINTER                "OS/2 Zeiger"
#define IDT_IDMEN_CONVERT_CURSOR                 "Win Cursor"
#define IDT_IDMEN_CONVERT_WINANIMATION           "Win Animation"
#define IDT_IDMEN_CONVERT_ANIMOUSE               "Animouse Resource DLL"

//
// strings for the Edit Animation dialog
//

#define IDT_IDDLG_DLG_EDITANIMATION              "Animation Editor"
#define IDT_IDDLG_ST_SHOWFOR                     "Show frame for"
#define IDT_IDDLG_GB_PREVIEW                     "Preview"
#define IDT_IDDLG_GB_ANIMINFO                    "Animation Info"
//      IDT_IDDLG_ST_ANIMNAME
//      IDT_IDDLG_ST_ANIMARTIST

//
// Strings for the frame set container
//

#define IDT_IDSTR_TITLE_IMAGE                    "Image"
//      IDT_IDSTR_TITLE_FRAMERATE
#define IDT_IDSTR_SELECTED_NONE                  " - No frame selected -"
#define IDT_IDSTR_SELECTED_FRAME                 " Frame %u of %u selected"
#define IDT_IDSTR_SELECTED_FRAMES                " %u Frames selected"
#define IDT_IDSTR_UNIT_MILLISECONDS              "ms."
#define IDT_IDSTR_UNIT_JIFFIES                   "Jiffies"

#define IDT_IDMEN_AE_FILE                        "~File"
#define IDT_IDMEN_AE_FILE_NEW                    "~New"
#define IDT_IDMEN_AE_FILE_OPEN                   "~Open..."
#define IDT_IDMEN_AE_FILE_SAVE                   "~Save"
#define IDT_IDMEN_AE_FILE_SAVEAS                 "Save ~as..."
#define IDT_IDMEN_AE_FILE_IMPORT                 "~Import frame..."
#define IDT_IDMEN_AE_FILE_EXIT                   "E~xit"
#define IDT_IDMEN_AE_EDIT                        "~Edit"
#define IDT_IDMEN_AE_EDIT_COPY                   "~Copy"
#define IDT_IDMEN_AE_EDIT_CUT                    "Cu~t"
#define IDT_IDMEN_AE_EDIT_PASTE                  "Pa~ste"
#define IDT_IDMEN_AE_EDIT_DELETE                 "~Delete"
#define IDT_IDMEN_AE_EDIT_SELECTALL              "Select ~all"
#define IDT_IDMEN_AE_EDIT_DESELECTALL            "D~eselect all"
#define IDT_IDMEN_AE_PALETTE                     "~Palette"
#define IDT_IDMEN_AE_PALETTE_OPEN                "~Open"
#define IDT_IDMEN_AE_PALETTE_SAVE                "~Save"
#define IDT_IDMEN_AE_PALETTE_SAVEAS              "Save ~as..."
#define IDT_IDMEN_AE_PALETTE_COPY                "~Copy"
#define IDT_IDMEN_AE_PALETTE_PASTE               "~Paste"
#define IDT_IDMEN_AE_OPTION                      "~Options"
#define IDT_IDMEN_AE_OPTION_UNIT                 "Change Unit"
#define IDT_IDMEN_AE_OPTION_UNIT_MS              "milliseconds"
#define IDT_IDMEN_AE_OPTION_UNIT_JIF             "Jiffies"
#define IDT_IDMEN_AE_HELP                        IDT_IDMEN_ITEM_HELP
#define IDT_IDMEN_AE_HELP_INDEX                  IDT_IDMEN_ITEM_HELP_INDEX
#define IDT_IDMEN_AE_HELP_GENERAL                IDT_IDMEN_ITEM_HELP_GENERAL
#define IDT_IDMEN_AE_HELP_USING                  IDT_IDMEN_ITEM_HELP_USING
#define IDT_IDMEN_AE_HELP_KEYS                   IDT_IDMEN_ITEM_HELP_KEYS
#define IDT_IDMEN_AE_ABOUT                       IDT_IDMEN_ITEM_HELP_ABOUT

//
//      Text for online help panel titles
//

#define IDH_IDPNL_MAIN                           __TITLE__

#define IDH_IDPNL_OVERVIEW                       "öberblick"

#define IDH_IDPNL_REVIEWS                        "Testberichte"

#define IDH_IDPNL_PREREQUISITES                  "Voraussetzungen"

#define IDH_IDPNL_LEGAL                          "Copyright &amp. Co."
#define IDH_IDPNL_LEGAL_COPYRIGHT                "Copyright"
#define IDH_IDPNL_LEGAL_LICENSE                  "freeware Lizenz"
#define IDH_IDPNL_LEGAL_PROMOTEOS2               "öber OS/2"
#define IDH_IDPNL_LEGAL_DISCLAIMER               "GewÑhrleistungsausschlu·"
#define IDH_IDPNL_LEGAL_AUTHOR                   "Der Autor"

#define IDH_IDPNL_USAGE                          "Benutzung"
#define IDH_IDPNL_USAGE_OS2POINTER               "OS/2 Mauszeiger"
#define IDH_IDPNL_USAGE_ANIMATION                "Mauszeigeranimationen"
#define IDH_IDPNL_USAGE_RESTYPES                 "UnterstÅtzte Ressourcetypen"
#define IDH_IDPNL_USAGE_RESTYPES_POINTER         "OS/2 Zeiger Datei"
#define IDH_IDPNL_USAGE_RESTYPES_CURSOR          "Win* Cursor Datei"
#define IDH_IDPNL_USAGE_RESTYPES_WINANIMATION    "Win* Animationsdatei"
#define IDH_IDPNL_USAGE_RESTYPES_ANIMOUSE        "AniMouse Ressource DLL Datei"
#define IDH_IDPNL_USAGE_RESTYPES_ANMFILE         "AniMouse Scriptdatei"
#define IDH_IDPNL_USAGE_RESTYPES_SET             "Animationssatz"
#define IDH_IDPNL_USAGE_RESTYPES_SET_POINTER     "OS/2 Zeigersatz"
#define IDH_IDPNL_USAGE_RESTYPES_SET_CURSOR      "Win Cursorsatz"
#define IDH_IDPNL_USAGE_RESTYPES_SET_WINANIMATION "Win Animationssatz"
#define IDH_IDPNL_USAGE_RESTYPES_SET_ANIMOUSE    "AniMouse Animationssatz"
#define IDH_IDPNL_USAGE_STATICPTR                "Statischer Zeiger"
#define IDH_IDPNL_USAGE_NBPAGE                   "&IDT_IDTXT_NBPAGE. Seite"
#define IDH_IDPNL_USAGE_NBPAGE_CNRSETTINGS       "Einstellungen des Mauszeiger Containers"
#define IDH_IDPNL_USAGE_NBPAGE_CNRLOADSET        "Animationssatz laden"
#define IDH_IDPNL_USAGE_SETTINGS_STRINGS         "Mausobjekt Einstellungsbefehle"
#define IDH_IDPNL_USAGE_REXX_SAMPLES             "REXX Beispielprogramme"
#define IDH_IDPNL_USAGE_ENVIRONMENT_VARIABLES    "Umgebungsvariablen"
#define IDH_IDPNL_USAGE_MAKEAND                  "AniMouse Resource DLL Dateien erzeugen"
#define IDH_IDPNL_USAGE_MAKEAND_SCRIPT           "AniMouse Scriptdatei Format"
#define IDH_IDPNL_USAGE_SUPPLIEDANIMATIONS       "Mitgelieferte Animationen"
#define IDH_IDPNL_USAGE_WHEREANIMATIONS          "Wo bekommt man weitere/neue Animationen ?"

#define IDH_IDPNL_HOW                            "Wie kann ich ... ?"
#define IDH_IDPNL_HOW__BASETEXT                  "Wie kann ich "
#define IDH_IDPNL_HOW__BASEDOT                   "... "
#define IDH_IDPNL_HOW_LOADSET                    "eine Animationsressource laden ?"
#define IDH_IDPNL_HOW_ACTIVATE                   "eine Animation (de-)aktivieren ?"
#define IDH_IDPNL_HOW_DEMO                       "die Demofunktion (de-)aktivieren ?"
#define IDH_IDPNL_HOW_SETTINGS                   "Animationseinstellungen Ñndern ?"
#define IDH_IDPNL_HOW_DELAYINITANIMATION         "die Initialisierung der Animation verzîgern ?"
#define IDH_IDPNL_HOW_USEREXX                    "&__TITLE__. mit REXX konfigurieren ?"
#define IDH_IDPNL_HOW_MAKEAND                    "eine AniMouse Ressource DLL erzeugen ?"

#define IDH_IDPNL_LIMITATIONS                    "EinschrÑnkungen"

#define IDH_IDPNL_REVISIONS                      "Revisionshistorie"
#define IDH_IDPNL_REVISION_HISTORY_100           "Version 1.00"
#define IDH_IDPNL_REVISION_HISTORY_101           "Version 1.01"
#define IDH_IDPNL_FILE_ID_DIZ                    "file_id.diz"

#define IDH_IDPNL_CREDITS                        "Danksagungen"
#define IDH_IDPNL_CREDITS_GENERAL                "Generelle Danksagungen"
#define IDH_IDPNL_CREDITS_TRANSLATORS            "öbersetzer"
#define IDH_IDPNL_CREDITS_ANIMATIONS             "Ersteller von Animationen"

#define IDH_IDPNL_TRADEMARKS                     "Warenzeichen"

#endif // AMPTRDEU_RCH

