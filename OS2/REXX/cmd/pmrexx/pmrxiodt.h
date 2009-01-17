/*static char *SCCSID = "@(#)pmrxiodt.h	6.2 92/03/09";*/
/*static char *SCCSID = "@(#)pmrxiodt.h	6.2 92/03/09";                     */
/******************************************************************************/
/* Global values for common uses...                                           */
/******************************************************************************/

#define  ID_NULL        -1
#define  ENTER          1
#define  ESCAPE         2
#define  HELP           3
#define  DELIMITER_SIZE 5
#define  RXHOST_WIN     0x1100

/* Start of string space:                                                     */

#define  WINDOW_TEXT        0x01
#define  MLE_CREATE_ERROR   0x02
#define  MLE_IMPORT_ERROR   0x03
#define  MLE_IMEXPORT_ERROR 0x04
#define  INPUT_TITLE        0x05
#define  RXHB_MSGBOXHDR     0x06
#define  HELP_ERR           0x07
#define  NO_HELP_MGR        0x08
#define  PMREXX_HELPFILENAME 0x09
#define  PMREXX_HELP_TITLE  0x0a
#define  ERROR_HELPASSOC    0x0b
#define  ERROR_HELPCREATE   0x0c
#define  INIT_FAILED        0x0d

/* ClipBoard Errors Headings                                                  */

#define  RXIOH_ERR_CLIPBRD  0x10
#define  RXIOH_ERR_UNDO     0x11
#define  RXIOH_ERR_CUT      0x12
#define  RXIOH_ERR_DELETE   0x13
#define  RXIOH_ERR_PASTE    0x14
#define  RXIOH_ERR_COPY     0x15
#define  RX_ALLOCERR        0x16
#define  SAMPLE_TEXT        0x17
#define  RXIOH_TITLE        0x18
#define  RXIOH_WARNING      0x19

/* Help Defines                                                               */

#define  RXHH_HELPHELP    1
#define  FONT_HELP        2
#define  PMREXX_HELPTABLE 3

/******************************************************************************/
/* String Defines                                                             */
/******************************************************************************/

#define  RXIOH_ERRORTITLE   0x2FC
#define  RXIOH_NOTIFICATION 0x2FD
#define  RXIOH_CRITICAL     0x2FF
#define  SELECTED_TEXT      0x8110
#define  NORMAL_TEXT        0x8111
#define  RXIOHM_UNDO        0x8118
#define  FONT_TITLE         274
#define  RX_KEYSHELP        9001
#define  SEARCH_TITLE       9002
#define  SEARCH_ERROR       9003

