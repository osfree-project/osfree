/*----------------------------------------------------------------------------- 
 *
 *----------------------------------------------------------------------------*/

/* Use -z option of whead on say file.exe and redirect the output to say
 * file.api. Include this file here and add modulename_api to the list of
 * in api_list. And you're all set to have import names of programs which
 * use file.exe
 */

typedef char *string;
typedef	struct entry	ENTRY;

struct entry{
	int	ordinal;
	string	name;
};

#include	"api/comm.api"
#include	"api/commdlg.api"
#include	"api/ddeml.api"
#include	"api/display.api"
#include	"api/gdi.api"
#include	"api/kernel.api"
#include	"api/keyboard.api"
#include	"api/lzexpand.api"
#include	"api/mmsystem.api"
#include	"api/mouse.api"
#include	"api/olecli.api"
#include	"api/olesvr.api"
#include	"api/shell.api"
#include	"api/sound.api"
#include	"api/system.api"
#include	"api/toolhelp.api"
#include	"api/user.api"
#include	"api/ver.api"

ENTRY *api_list[] = {
	COMM_api,
	COMMDLG_api,
	DDEML_api,
	DISPLAY_api,
	GDI_api,
	KERNEL_api,
	KEYBOARD_api,
	LZEXPAND_api,
	MMSYSTEM_api,
	MOUSE_api,
	OLECLI_api,
	OLESVR_api,
	SHELL_api,
	SOUND_api,
	SYSTEM_api,
	TOOLHELP_api,
	USER_api,
	VER_api,
	NULL
};





