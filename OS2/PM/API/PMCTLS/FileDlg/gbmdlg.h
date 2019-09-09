/*

gbmdlg.h - Interface to GBM File Dialog

*/

#ifndef _GBM_DLG_H_
  #define _GBM_DLG_H_

#define	L_GBM_OPTIONS 500

#ifdef __cplusplus
  extern "C"
  {
#endif

/* Enforce compatibility to old binary interface by      */
/* aligning data in public structs to 4 byte boundaries. */
/* This allows internal data alignment with optimum size */
/* for best processor specific optimization.             */
#pragma pack(4)

typedef struct _GBMFILEDLG
	{
	FILEDLG fild;
	CHAR szOptions [L_GBM_OPTIONS+1];
	} GBMFILEDLG;

/* Enable compiler default packing. */
#pragma pack()

#ifndef _GBMDLG

HWND _System GbmFileDlg(HWND hwndP, HWND hwndO, GBMFILEDLG *pgbmfild);
MRESULT _System GbmDefFileDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif

#endif

