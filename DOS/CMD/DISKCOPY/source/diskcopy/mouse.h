/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be

 */

#ifndef MOUSE_H_
#define MOUSE_H_

#define LEFTBUTTON   1
#define RIGHTBUTTON  2
#define MIDDLEBUTTON 4

#define LEFTBUTTONACCENT   0
#define RIGHTBUTTONACCENT  1
#define MIDDLEBUTTONACCENT 2

#define SOTWARECURSOR   0
#define HARDWARECURSOR  1

/* Mouse.asm */

int InitMouse (void);
int MousePresent (void);
void ShowMouse (void);
void HideMouse (void);
int WhereMouse (int *x, int *y);
void MouseGotoXY (int x, int y);
int CountButtonPresses (int button, int *releases, int *x, int *y);
int CountButtonReleases (int button, int *releases, int *x, int *y);
void MouseWindow (int x1, int y1, int x2, int y2);
void DefineTextMouseCursor (int type, int andmask, int ormask);
void GetMouseMoved (int *distx, int *disty);
void SetLightPenOn (void);
void SetLightPenOff (void);
void SetMickey (int hm, int vm);
void CloseMouse (void);

/* Himouse.c -- Only in defrag */

#ifndef HI_TECH_C

void ClearMouse (void);
int MousePressed (int button);
int MouseReleased (int button);
int PressedInRange (int x1, int y1, int x2, int y2);
int ReleasedInRange (int x1, int y1, int x2, int y2);
int AnyButtonPressed (void);

int GetPressedX ();
int GetPressedY ();
int GetReleasedX ();
int GetReleasedY ();

void LockMouse ();
void UnLockMouse ();

#endif

#endif
