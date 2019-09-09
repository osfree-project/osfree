/****************************************************************************
 
    @(#)mmddk.h	1.1 - multimedia system / device driver interface
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

**************************************************************************** */

BOOL	DriverCallback(DWORD, UINT, HANDLE, UINT, DWORD, DWORD, DWORD);

/* [DRIVER] (MMSYSTEM) Auxiliary Audio ************************************* */

#define AUXDM_GETNUMDEVS	0x0001
#define AUXDM_GETDEVCAPS	0x0002
#define AUXDM_GETVOLUME		0x0003
#define AUXDM_SETVOLUME		0x0004

/* [DRIVER] (MMSYSTEM) Midi ************************************************ */

typedef struct {
	HMIDI			hMidi;
	DWORD			dwCallback;
	DWORD			dwInstance;
} MIDIOPENDESC;
typedef MIDIOPENDESC		*PMIDIOPENDESC;
typedef MIDIOPENDESC NEAR	*NPMIDIOPENDESC;
typedef MIDIOPENDESC FAR	*LPMIDIOPENDESC;

/* [DRIVER] (MMSYSTEM) Midi In ********************************************* */

#define MIDM_ADDBUFFER		0x0005
#define MIDM_CLOSE		0x0006
#define MIDM_GETDEVCAPS		0x0007
#define MIDM_GETNUMDEVS		0x0008
#define MIDM_OPEN		0x0009
#define MIDM_RESET		0x000a
#define MIDM_START		0x000b
#define MIDM_STOP		0x000c
#define MIDM_PREPARE		0x000d
#define MIDM_UNPREPARE		0x000e

/* [DRIVER] (MMSYSTEM) Midi Out ******************************************** */

#define MODM_CLOSE		0x0010
#define MODM_DATA		0x0011
#define MODM_GETDEVCAPS		0x0012
#define MODM_GETNUMDEVS		0x0013
#define MODM_LONGDATA		0x0014
#define MODM_OPEN		0x0015
#define MODM_RESET		0x0016
#define MODM_PREPARE		0x0017
#define MODM_UNPREPARE		0x0018
#define MODM_GETVOLUME		0x0019
#define MODM_SETVOLUME		0x001a

/* [DRIVER] (MMSYSTEM) Wave ************************************************ */

typedef struct {
	HWAVE			hWave;
	const WAVEFORMAT FAR	*lpFormat;
	DWORD			dwCallback;
	DWORD			dwInstance;
} WAVEOPENDESC;
typedef WAVEOPENDESC		*PWAVEOPENDESC;
typedef WAVEOPENDESC NEAR	*NPWAVEOPENDESC;
typedef WAVEOPENDESC FAR	*LPWAVEOPENDESC;

/* [DRIVER] (MMSYSTEM) Wave In ********************************************* */

#define WIDM_ADDBUFFER		0x0020
#define WIDM_CLOSE		0x0021
#define WIDM_GETDEVCAPS		0x0022
#define WIDM_GETNUMDEVS		0x0023
#define WIDM_GETPOS		0x0024
#define WIDM_OPEN		0x0025
#define WIDM_RESET		0x0026
#define WIDM_START		0x0027
#define WIDM_STOP		0x0028
#define WIDM_PREPARE		0x0029
#define WIDM_UNPREPARE		0x002a

/* [DRIVER] (MMSYSTEM) Wave Out ******************************************** */

#define WODM_BREAKLOOP		0x0030
#define WODM_CLOSE		0x0031
#define WODM_GETDEVCAPS		0x0032
#define WODM_GETNUMDEVS		0x0033
#define WODM_GETPOS		0x0034
#define WODM_OPEN		0x0035
#define WODM_PAUSE		0x0036
#define WODM_RESET		0x0037
#define WODM_RESTART		0x0038
#define WODM_WRITE		0x0039
#define WODM_PREPARE		0x003a
#define WODM_UNPREPARE		0x003b
#define WODM_GETPITCH		0x003c
#define WODM_SETPITCH		0x003d
#define WODM_GETVOLUME		0x003e
#define WODM_SETVOLUME		0x003f
#define WODM_GETPLAYBACKRATE	0x0040
#define WODM_SETPLAYBACKRATE	0x0041

/* [DRIVER] (MMSYSTEM) Joystick ******************************************** */

#define JDD_GETNUMDEVS		0x0050
#define JDD_GETDEVCAPS		0x0051
#define JDD_GETPOS		0x0052
#define JDD_SETCALIBRATION	0x0053

typedef struct {
	UINT			wXbase;
	UINT			wXdelta;
	UINT			wYbase;
	UINT			wYdelta;
	UINT			wZbase;
	UINT			wZdelta;
} JOYCALIBRATE;
typedef JOYCALIBRATE		*PJOYCALIBRATE;
typedef JOYCALIBRATE NEAR	*NPJOYCALIBRATE;
typedef JOYCALIBRATE FAR	*LPJOYCALIBRATE;

/* [DRIVER] (MMSYSTEM) Timer *********************************************** */

#define TDD_GETNUMDEVS		0x0060
#define TDD_GETDEVCAPS		0x0061

/* [DRIVER] (MMSYSTEM) Media Control Interface ***************************** */

#define MCI_OPEN_DRIVER		0x0100
#define MCI_CLOSE_DRIVER	0x0101

	/*
	 */
#define MCI_COMMAND_HEAD	0x0001
#define MCI_RETURN		0x0002
#define MCI_FLAG		0x0003
#define MCI_INTEGER		0x0004
#define MCI_RECT		0x0005
#define MCI_STRING		0x0006
#define MCI_CONSTANT		0x0007
#define MCI_END_CONSTANT	0x0008
#define MCI_END_COMMAND		0x0009
#define MCI_END_COMMAND_LIST	0x000a

#define MAKEMCIRESOURCE(w,s)

	/*
	 */
#define MCI_FORMAT_MILLISECONDS_S	0x0001
#define MCI_FORMAT_HMS_S		0x0001
#define MCI_FORMAT_MSF_S		0x0001
#define MCI_FORMAT_FRAMES_S		0x0001
#define MCI_FORMAT_SMPTE_24_S		0x0001
#define MCI_FORMAT_SMPTE_25_S		0x0001
#define MCI_FORMAT_SMPTE_30_S		0x0001
#define MCI_FORMAT_SMPTE_30DROP_S	0x0001
#define MCI_FORMAT_BYTES_S		0x0001
#define MCI_FORMAT_SAMPLES_S		0x0001
#define MCI_FORMAT_TMSF_S		0x0001
#define MCI_VD_FORMAT_TRACK_S		0x0001
#define WAVE_FORMAT_PCM_S		0x0001
#define WAVE_MAPPER_S			0x0001
#define MCI_SEQ_MAPPER_S		0x0001
#define MCI_SEQ_FILE_S			0x0001
#define MCI_SEQ_MIDI_S			0x0001
#define MCI_SEQ_SMTPE_S			0x0001
#define MCI_SEQ_FORMAT_SONGPTR_S	0x0001
#define MCI_SEQ_NONE_S			0x0001
#define MIDIMAPPER_S			0x0001

	/*
	 */
#define MCI_RESOURCE_RETURNED		0x0001
#define MCI_RESOURCE_DRIVER		0x0002
#define MCI_COLONIZED3_RETURN		0x0004
#define MCI_COLONIZED4_RETURN		0x0004
#define MCI_INTEGER_RETURNED		0x0004

	/*
	 */
#define MCI_NO_COMMAND_TABLE		0

typedef struct {
	UINT			wDeviceID;
	LPCSTR			lpstrParams;
	UINT			wCustomCommandTable;
	UINT			wType;
} MCI_OPEN_DRIVER_PARMS;
typedef MCI_OPEN_DRIVER_PARMS		*PMCI_OPEN_DRIVER_PARMS;
typedef MCI_OPEN_DRIVER_PARMS NEAR	*NPMCI_OPEN_DRIVER_PARMS;
typedef MCI_OPEN_DRIVER_PARMS FAR	*LPMCI_OPEN_DRIVER_PARMS;

BOOL	WINAPI mciDriverNotify(HANDLE hMciDeviceCallback,
	UINT uMciDeviceID, UINT uMciDeviceStatus);
UINT	WINAPI mciDriverYield(UINT uMciDeviceID);
DWORD	WINAPI mciGetDriverData(UINT uMciDeviceID);
BOOL	WINAPI mciSetDriverData(UINT uMciDeviceID, DWORD dwMciDeviceData);
UINT	WINAPI mciLoadCommandResource(HANDLE hInstance, LPSTR lpszResourceName,
	UINT uMciDeviceType);
BOOL	WINAPI mciFreeCommandResource(UINT uMciCommandTable);

