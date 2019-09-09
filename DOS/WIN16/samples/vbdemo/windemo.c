
#include <stdlib.h>
#include <stdio.h>
#include "windows.h"
#include "dlfcn.h"
#include "ModTable.h"

/*************************************************************
 *
 *	LIBENTRY structure
 *		TWIN_LibEntry	function to initialize the library
 *		GetProcAddress  function to return other functions
 *		TWIN_LibExit    function to shutdown library
 *		GetInstanceFromModule	utility function
 *		GetModuleHandle         utility function
 *
 **************************************************************/ 

typedef struct {
	int       (*TWIN_LibEntry)(char *,ENTRYTAB *,MODULEDSCR *);
	FARPROC   (*GetProcAddress)(int,char *);
	BOOL      (*TWIN_LibExit)(int,DWORD);	
	HINSTANCE (*GetInstanceFromModule)(HANDLE);
	HANDLE    (*GetModuleHandle)(char *);
} LIBENTRY;

/************************************************
 * 
 * Private to file
 *
 ************************************************/
static LIBENTRY ltl;	/* libentry table	*/
static int      lte;	/* libentry errorcode   */

/************************************************
 *
 * This function is the primary linkage to TWIN
 * This function returns a table of pointers
 * necessary to access functions in the library.
 *
 * This function will Initialize the library if
 * desired.
 *
 * The function returns a structure of pointers
 * to functions, that allow access to any function
 * in the library.
 *
 *************************************************/

/*
 * note: ideally the libraries below should be passed
 *	 in an array, so that they are NOT hard-coded
 */

LIBENTRY *
LoadTwinLibrary(int init)
{
	void  *hso;
	char  *f = "libtwin.so";
	char  *x = "/usr/X11R6/lib/libX11.so";
	char  *m = "libm.so";

	hso = dlopen(m,RTLD_LAZY|RTLD_GLOBAL);
	if(hso == 0) {
		printf("error %s\n",x);
		lte = -1;
		return (LIBENTRY *) 0;
	}

	hso = dlopen(x,RTLD_LAZY|RTLD_GLOBAL);
	if(hso == 0) {
		printf("error %s\n",x);
		lte = -2;
		return (LIBENTRY *) 0;
	}
	hso = dlopen(f,RTLD_LAZY|RTLD_GLOBAL);
	if(hso == 0) {
		printf("error %s\n",f);
		lte = -3;
		return (LIBENTRY *) 0;
	}

	/**********************************************************
 	 *
 	 *	Get Basic Required Functions for later use
 	 *
	 **********************************************************/

	ltl.TWIN_LibEntry  = dlsym(hso,"TWIN_LibEntry");
	ltl.GetProcAddress = dlsym(hso,"GetProcAddress");
	ltl.TWIN_LibExit   = dlsym(hso,"TWIN_LibExit");
	ltl.GetModuleHandle       = dlsym(hso,"GetModuleHandle");
	ltl.GetInstanceFromModule = dlsym(hso,"GetInstanceFromModule");

	/***********************************************************
 	 * 
	 * 	Initialize the library, to 'safe' defaults
	 *	command string, library initializers, and MDT
	 *
	 **********************************************************/

	if(init)
		(*ltl.TWIN_LibEntry)(0,0,0);	

	return &ltl;
}

int
main(int argc, char **argv)
{
	//int (*lpf)();
	FARPROC lpf;
	HMODULE hModule;
	HINSTANCE hInstance;
	LIBENTRY *ltl;

	/****************************************
 	 *
 	 *	Load the twin library, and return
 	 * 	the LIBENTRY table.
	 *
 	 ***************************************/

	if ((ltl = LoadTwinLibrary(1)) == 0) {
		printf("TWIN Library not loaded.\n");
	}

	/**********************************************
	 *
	 * 	Demonstration of usage
	 *	Get hModule and hInstance of KERNEL
	 *	Use GetProcAddress to get pointer to
	 *	"GetWinFlags" function
	 *
	 **********************************************/

	hModule   = (*ltl->GetModuleHandle)("KERNEL");	
	hInstance = (*ltl->GetInstanceFromModule)(hModule);

	lpf = (*ltl->GetProcAddress)(hInstance,"GetWinFlags");	

	printf("lpf=%p\n", lpf);

	/************************************************
	 *
 	 * 	If we got it, call it
	 * 
	 *	Note: we don't yet have a task, hTask, or
	 *	      hInstance for ourselves, so some 
	 *	      functions cannot be called.  This
	 *	      probably includes any Window functions.
	 *
	 *	Note: you need to know the library a function
	 *	      is in, although you can iterate through
	 *	      all loaded modules.
	 *
	 *	Note: we probably could create a 'stub' WinMain
	 *	      and use it to create a task/instance pair
	 *	      and use setjmp/longjmp to get back 
	 *
	 ***********************************************/

	if(lpf)
		printf("DWORD=%x\n", (*lpf)());

	return 0;
}

