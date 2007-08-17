/***********************************************************************/
/* PRINT.C - Printing related functions                                */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

static char RCSid[] = "$Id: print.c,v 1.5 2001/12/18 08:23:27 mark Exp $";

#include <the.h>
#include <proto.h>

#if !defined(WIN32)
# ifdef HAVE_PROTO
static void print_shadow_line(FILE *,CHARTYPE *,LINETYPE);
# else
static void print_shadow_line( );
# endif
#endif

#ifdef WIN32
/*
 * Because curses implementations use MOUSE_MOVED and it clashes with
 * <windows.h> on Windows platforms, we have to undef it here to avoid
 * conflict.
 */
#undef MOUSE_MOVED
# include <windows.h>
static HFONT hOldFont;
static PRINTDLG pd;
static LOGFONT lf;
static DWORD dwCPI;
static DEVMODE *dm;
static HANDLE hPrinter=0;
static DWORD dwCharExtent=0;
static BOOL bLineWrapping=TRUE;
static BOOL documentON=FALSE;
static BOOL defaultprinter=TRUE;
static int ask,debug,printit,background;
static char work[255];
static char *ini;
static int offsetx, offsety, savelpi, savecpi, orient;
static int row=0;
static int column=0;
static int page=0;
static int color=0;

static HFONT SetFont( );
static void PrepareDC( );
static void PageHeader( int );
static void NextLine( );
static void NextPage( );
static void SetCPI( DWORD );
static void SetOrient( int );
static void StartDocument( void );
static void EndDocument( void );
static void print_shadow_line(CHARTYPE *,LINETYPE );

/*****************************************************************************
 * Select a fixed font into the printer DC. Since the DC is scaled for rows
 * and columns, we set the font size to 1x1 always.
 * This function returns a handle to the previous selected font.
*****************************************************************************/
static HFONT SetFont( void )
{
   // Create font
   lf.lfWidth = lf.lfHeight = 1;
   return ( HFONT )SelectObject( pd.hDC, CreateFontIndirect( &lf ) );
}

/*****************************************************************************
 * Prepare the printer DC for output. This function uses the specified CPI
 * values to scale the DC, using MM_ANISOTROPIC mapping mode.
*****************************************************************************/
static void PrepareDC( void )
{
   int lpxlX = GetDeviceCaps( pd.hDC, LOGPIXELSX );
   int lpxlY = GetDeviceCaps( pd.hDC, LOGPIXELSY );

   SetMapMode( pd.hDC, MM_ANISOTROPIC );
   SetViewportExtEx( pd.hDC, lpxlX, lpxlY, NULL );
   SetWindowExtEx( pd.hDC, LOWORD( dwCPI ), HIWORD( dwCPI ), NULL );
   SetViewportOrgEx( pd.hDC, offsetx, offsety, NULL );

   SetBkMode( pd.hDC, TRANSPARENT );
   SetTextColor( pd.hDC, color );
}

/*****************************************************************************
 End document
 Done on PRINT CLOSE
*****************************************************************************/
static void EndDocument(void)
{
   DeleteObject( SelectObject( pd.hDC, hOldFont ) );
   if (defaultprinter)
   {
     EndPage( pd.hDC );
     EndDoc( pd.hDC );
   }
   else
   {
     EndPagePrinter( hPrinter );
     EndDocPrinter( hPrinter );
   }
   documentON = FALSE;
}

/*****************************************************************************
 * Print a page header if wanted...
*****************************************************************************/
static void PageHeader( int pn )
{
}

/*****************************************************************************
 * Advance by one line. I crossed page boundary, go to next page
*****************************************************************************/
static void NextLine( void )
{
  row++;
  column = 0;
  if ( row == HIWORD( dwCharExtent ) )
     NextPage();
}

/*****************************************************************************
 * Go to next page
*****************************************************************************/
static void NextPage( void )
{
   DeleteObject( SelectObject( pd.hDC, hOldFont ) );
   if (defaultprinter)
   {
     EndPage( pd.hDC );
     StartPage( pd.hDC );
   }
   else
   {
     EndPagePrinter( hPrinter );
     StartPagePrinter( hPrinter );
   }
   hOldFont = SetFont( );
   PrepareDC( );
   column = row = 0;
   page++;
   PageHeader( page+1 );
}
/*****************************************************************************
 * Assign new CPI and LPI values
*****************************************************************************/
static void SetCPI( DWORD cpi )
{
  if ( documentON )
     EndDocument();
  dwCPI = cpi;
}

/*****************************************************************************
 * Start document
 * Done on first write to the spooler
*****************************************************************************/
static void StartDocument( void )
{
   char docname[255];
   DOCINFO di;
   DOC_INFO_1 di1;

   strcpy( docname, (DEFCHAR*)CURRENT_FILE->fpath );
   strcat( docname, (DEFCHAR*)CURRENT_FILE->fname );
   if (defaultprinter)
   {
      memset( &di, 0, sizeof ( DOCINFO ) );
      di.cbSize = sizeof( DOCINFO );
      di.lpszDocName = docname;
   }
   else
   {
      memset( &di1, 0, sizeof ( DOC_INFO_1 ) );
      di1.pDocName = docname;
   }

   if ( pd.hDC )
   {
      // If there`s still an open document, we have to close it first
      if ( documentON )
         EndDocument();

      // Re-calculate page extent in characters
      dwCharExtent = MAKELONG(
         ( ( GetDeviceCaps( pd.hDC, HORZRES ) / GetDeviceCaps( pd.hDC, LOGPIXELSX ) ) * LOWORD( dwCPI ) ),
         ( ( GetDeviceCaps( pd.hDC, VERTRES ) / GetDeviceCaps( pd.hDC, LOGPIXELSY ) ) * HIWORD( dwCPI ) ) );

      // Open a new document and start at its first page
      if (defaultprinter)
      {
         StartDoc( pd.hDC, &di );
         StartPage( pd.hDC );
      }
      else
      {
         StartDocPrinter( hPrinter, 1, (LPBYTE)&di1 );
         StartPagePrinter( hPrinter );
      }
      row = column = page = 0;
      PageHeader( page+1 );
      hOldFont = SetFont( );
      PrepareDC( );
      documentON = TRUE;
   }
}

/*****************************************************************************
 * Select a fixed font into the printer DC. Since the DC is scaled for rows
 * and columns, we set the font size to 1x1 always.
 * This function returns a handle to the previous selected font.
*****************************************************************************/
static void SetOrient( int or )
{
   dm = (DEVMODE*)GlobalLock( pd.hDevMode );
   dm->dmOrientation = orient = or;
   dm->dmFields |= DM_ORIENTATION;
   ResetDC( pd.hDC, dm );
   GlobalUnlock( dm );
}

/*****************************************************************************
 * Startup the printing interface
 * Done at start of program
*****************************************************************************/
void StartTextPrnt(void)
{
   memset( &pd, 0, sizeof(PRINTDLG) );
   pd.lStructSize = sizeof(PRINTDLG);

   memset( &lf, 0, sizeof(LOGFONT) );

   strcpy( lf.lfFaceName, "LinePrinter BM" );
   offsetx = 450;
   offsety = 200;
   savelpi = 8;
   savecpi = 16;
   SetCPI( MAKELONG(savecpi,savelpi) );
   lf.lfWidth = lf.lfHeight = 1;
   lf.lfWeight = 400;
   lf.lfPitchAndFamily = FF_MODERN|FIXED_PITCH;
   // Get a DC handle to the system`s default printer. using
   // the PD_RETURNDEFAULT flag ensures that no dialog is opened.
   pd.Flags = PD_RETURNDC | PD_RETURNDEFAULT | PD_NOSELECTION;
   PrintDlg(&pd);
   if ( pd.hDevMode == NULL )
   {
      display_error(78,"Unable to initialise default printer",FALSE);
      return;
   }
   SetOrient( DMORIENT_PORTRAIT );
}

/*****************************************************************************
 * Stop the printing interface
 * Done at end of program
*****************************************************************************/
void StopTextPrnt( void )
{
   if ( pd.hDC )
   {
      if ( documentON )
         EndDocument();
      if ( pd.hDevMode )
      {
         GlobalUnlock( pd.hDevMode );
         GlobalFree( pd.hDevMode );
      }
      if ( pd.hDevNames )
         GlobalFree( pd.hDevNames );
      if ( pd.hDC )
         DeleteDC( pd.hDC );
   }
   if (!defaultprinter)
      ClosePrinter( hPrinter );
}

/*****************************************************************************
 * Write a string to the current document
*****************************************************************************/
void WriteString(char* sz, int len)
{
   int tabn = (int)CURRENT_FILE->tabsout_num;
   int i=0;
   char *p=NULL;
   char ch=0;
   DWORD dwNumWritten=0;

   if ( pd.hDC == NULL )
      return;
   if ( !documentON )
      StartDocument( );
   for (i = 0,p = sz; i < len; p++, i++)
   {
      ch = *p;
      switch ( ch )
      {
         case '\n':                             // Line-feed
            NextLine();
            break;

         case '\f' :                             // Form-feed
            NextPage();
            break;

         case '\r' :                             // Carriage-return
            column = 0;
            break;

         case '\t' :                             // Tab
            column += tabn - (column % tabn);
            if ( bLineWrapping 
            && column >= LOWORD(dwCharExtent) )
               NextLine();
            break;
         default :
            if ( isprint(ch) )
            {
               if (defaultprinter)
                  TextOut(pd.hDC, column++, row, &ch, 1);
               else
                  WritePrinter( hPrinter, p, 1, &dwNumWritten );
               // Wrap line
               if ( bLineWrapping 
               && column >= LOWORD(dwCharExtent) )
                  NextLine();
            }
            break;
      } // End switch
   }
}
#endif

#ifndef MSWIN
/***********************************************************************/
#ifdef HAVE_PROTO
void print_line(bool close_spooler,LINETYPE true_line,LINETYPE num_lines,
                short pagesize,CHARTYPE *text,CHARTYPE *line_term,short target_type)
#else
void print_line(close_spooler,true_line,num_lines,pagesize,text,line_term,target_type)
bool close_spooler;
LINETYPE true_line,num_lines;
short pagesize;
CHARTYPE *text;
CHARTYPE *line_term;
short target_type;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 static bool spooler_open=FALSE;
 static FILE *pp;
 short rc=RC_OK;
 LINETYPE j=0L;
 LINE *curr=NULL;
 short line_number=0;
 LINETYPE num_excluded=0L;
 LINETYPE num_actual_lines=0L;
 LINETYPE abs_num_lines=(num_lines < 0L ? -num_lines : num_lines);
 short direction=(num_lines < 0L ? DIRECTION_BACKWARD : DIRECTION_FORWARD);
 unsigned short y=0,x=0;
 bool lines_based_on_scope=(target_type==TARGET_BLOCK_CURRENT)?FALSE:TRUE;
 LINETYPE start=0L,end=0L,len=0L;
 CHARTYPE *ptr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("print.c:   print_line");

 if (close_spooler)
   {
    if (spooler_open)
      {
       spooler_open = FALSE;
#if defined(WIN32)
       EndDocument();
#elif defined(UNIX)
       pclose(pp);
#else
       fclose(pp);
#endif
       TRACE_RETURN();
       return;
      }
   }

#if defined(WIN32)
 if (!spooler_open)
   {
    StartDocument();
    spooler_open = TRUE;
   }
#elif defined(UNIX)
 if (!spooler_open)
   {
    pp = popen((DEFCHAR *)spooler_name,"w");
    if (pp == NULL)
     {
      TRACE_RETURN();
      return;
     }
    spooler_open = TRUE;
   }
#else
 if (!spooler_open)
   {
    pp = fopen((DEFCHAR *)spooler_name,"ab");
    if (pp == NULL)
     {
      TRACE_RETURN();
      return;
     }
    spooler_open = TRUE;
   }
#endif

 if (num_lines == 0L)
   {
#if defined(WIN32)
    WriteString(text,strlen(text));
    WriteString(line_term,strlen(line_term));
#else
    fprintf(pp,"%s%s",text,line_term);
#endif
    TRACE_RETURN();
    return;
   }
/*---------------------------------------------------------------------*/
/* Once we get here, we are to print lines from the file.              */
/*---------------------------------------------------------------------*/
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 if (curses_started)
   {
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
       getyx(CURRENT_WINDOW_FILEAREA,y,x);
    else
       getyx(CURRENT_WINDOW,y,x);
   }
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
 for (j=0L,num_actual_lines=0L;;j++)
   {
    if (lines_based_on_scope)
      {
       if (num_actual_lines == abs_num_lines)
          break;
      }
    else
      {
       if (abs_num_lines == j)
          break;
      }
    rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(j*direction),curr);
    switch(rc)
      {
       case LINE_SHADOW:
            num_excluded++;
            break;
       case LINE_TOF:
       case LINE_EOF:
            num_actual_lines++;
            break;
       default:
            if (num_excluded != 0)
              {
#if defined(WIN32)
               print_shadow_line(line_term,num_excluded);
#else
               print_shadow_line(pp,line_term,num_excluded);
#endif
               num_excluded = 0L;
              }
            switch(target_type)
              {
               case TARGET_BLOCK_CURRENT:
                    switch(MARK_VIEW->mark_type)
                      {
                       case M_LINE:
                            start = 0;
                            end = (curr->length)-1;
                            len = end - start + 1L;
                            ptr = curr->line;
                            break;
                       case M_BOX:
                       case M_WORD:
                       case M_COLUMN:
                            pre_process_line(CURRENT_VIEW,true_line+(LINETYPE)(j*direction),curr);
                            start = MARK_VIEW->mark_start_col - 1;
                            end = MARK_VIEW->mark_end_col - 1;
                            len = end - start + 1L;
                            ptr = rec+start;
                            break;
                       case M_STREAM:
                       case M_CUA:
                            pre_process_line(CURRENT_VIEW,true_line+(LINETYPE)(j*direction),curr);
                            start = 0;
                            end = (curr->length)-1;
                            if (true_line+(LINETYPE)(j*direction) == MARK_VIEW->mark_start_line)
                               start = MARK_VIEW->mark_start_col - 1;
                            if (true_line+(LINETYPE)(j*direction) == MARK_VIEW->mark_end_line)
                               end = MARK_VIEW->mark_end_col - 1;
                            len = end - start + 1L;
                            ptr = curr->line+start;
                            break;
                      }
                    break;
               default:
                    if (curr->length == 0)
                       len = 0L;
                    else
                    {
                       start = (LINETYPE)CURRENT_VIEW->zone_start-1;
                       end = (LINETYPE)min((curr->length)-1,CURRENT_VIEW->zone_end-1);
                       ptr = curr->line+start;
                       if (start > end)
                          len = 0L;
                       else
                          len = end - start + 1L;
                    }
                    break;
              }
#if defined(WIN32)
            WriteString(ptr,len);
            WriteString(line_term,strlen(line_term));
#else
            fwrite((DEFCHAR*)ptr,sizeof(CHARTYPE),len,pp);
            fprintf(pp,"%s",line_term);
#endif
            line_number++;
            if (line_number == pagesize
            && pagesize != 0)
              {
#if defined(WIN32)
               WriteString("\f",1);
#else
               fputc('\f',pp);
#endif
               line_number = 0;
              }
            num_actual_lines++;
            break;
      }
/*---------------------------------------------------------------------*/
/* Proceed to the next record, even if the current record not in scope.*/
/*---------------------------------------------------------------------*/
    if (direction == DIRECTION_BACKWARD)
       curr = curr->prev;
    else
       curr = curr->next;
     if (curr == NULL)
        break;
   }
/*---------------------------------------------------------------------*/
/* If we have a shadow line remaining, print it...                     */
/*---------------------------------------------------------------------*/
 if (num_excluded != 0)
   {
#if defined(WIN32)
    print_shadow_line(line_term,num_excluded);
#else
    print_shadow_line(pp,line_term,num_excluded);
#endif
    num_excluded = 0L;
   }
/*---------------------------------------------------------------------*/
/* If STAY is OFF, change the current and focus lines by the number    */
/* of lines calculated from the target.                                */
/*---------------------------------------------------------------------*/
 if (!CURRENT_VIEW->stay)                               /* stay is off */
   {
    CURRENT_VIEW->focus_line = min(CURRENT_VIEW->focus_line+num_lines-1L,CURRENT_FILE->number_lines+1L);
    CURRENT_VIEW->current_line = min(CURRENT_VIEW->current_line+num_lines-1L,CURRENT_FILE->number_lines+1L);
   }
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 build_screen(current_screen);
 display_screen(current_screen);
 if (curses_started)
   {
    y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                               CURRENT_VIEW->current_row);
    if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
       wmove(CURRENT_WINDOW_FILEAREA,y,x);
    else
       wmove(CURRENT_WINDOW,y,x);
   }
 TRACE_RETURN();
 return;
}

/***********************************************************************/
#ifdef HAVE_PROTO
static void make_shadow_line(char *buf,LINETYPE num_excluded, int width)
#else
static void make_shadow_line(buf,num_excluded,width)
char *buf;
LINETYPE num_excluded;
int width;
#endif
/***********************************************************************/
{
#define LINES_NOT_DISPLAYED " line(s) not displayed "
/*--------------------------- local data ------------------------------*/
 int numlen=0,first=0;
 char numbuf[33]; /* 10 + length of LINES_NOT_DISPLAYED */
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("print.c:   make_shadow_line");
 numlen = sprintf(numbuf," %ld%s",num_excluded,LINES_NOT_DISPLAYED);
 if (numlen > width)
    numlen = width;
 /* distribute pad characters */
 first = (width - numlen) >> 1;
 memset(buf,'-',first);
 buf += first;
 memcpy(buf,numbuf,numlen);
 buf += numlen;
 /* fill up to end */
 memset(buf,'-',width - first - numlen);
 /* terminate string */
 buf[width - first - numlen] = '\0';
 TRACE_RETURN();
 return;
}

#if defined(WIN32)
/***********************************************************************/
#ifdef HAVE_PROTO
static void print_shadow_line(CHARTYPE *line_term,LINETYPE num_excluded)
#else
static void print_shadow_line(line_term,num_excluded)
CHARTYPE *line_term;
LINETYPE num_excluded;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register int width=0;
 char buf[512];
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("print.c:   print_shadow_line");
 if (CURRENT_VIEW->shadow)
   {
    width = min(sizeof(buf)-1,CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
    make_shadow_line(buf,num_excluded,width);
    WriteString(buf,width);
    WriteString(line_term,strlen(line_term));
   }
 TRACE_RETURN();
 return;
}
#else
/***********************************************************************/
#ifdef HAVE_PROTO
static void print_shadow_line(FILE *pp,CHARTYPE *line_term,LINETYPE num_excluded)
#else
static void print_shadow_line(pp,line_term,num_excluded)
FILE *pp;
CHARTYPE *line_term;
LINETYPE num_excluded;
#endif
/***********************************************************************/
{
   register int width=0;
   char buf[512];

   TRACE_FUNCTION("print.c:   print_shadow_line");
   if (CURRENT_VIEW->shadow)
   {
      width = min(sizeof(buf)-1,CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
      make_shadow_line(buf,num_excluded,width);
      fwrite(buf,width,1,pp);
      fputs((DEFCHAR *)line_term,pp);
   }
   TRACE_RETURN();
   return;
}
#endif

/***********************************************************************/
#ifdef HAVE_PROTO
short setprintername(char *pn)
#else
short setprintername(pn)
char *pn;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("print.c:   setprintername");
#ifdef WIN32
   if ( hPrinter )
   {
      ClosePrinter( hPrinter );
      hPrinter = 0;
   }
   if ( stricmp( pn, "default" ) != 0 )
   {
      if ( OpenPrinter( pn, &hPrinter, NULL ) == 0 )
      {
         char buf[25];
         display_error(78,"no access to printer",FALSE);
         sprintf(buf,"Win32 error: %d",GetLastError());
         display_error(78,buf,FALSE);
         rc = RC_INVALID_OPERAND;
      }
      defaultprinter = FALSE;
   }
   if ( documentON )
      EndDocument();
#endif

   TRACE_RETURN();
   return (rc);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short setfontcpi(int cpi)
#else
short setfontcpi(cpi)
int cpi;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("print.c:   setfontcpi");
#ifdef WIN32
   SetCPI(MAKELONG(cpi,savelpi));
#endif
   TRACE_RETURN();
   return (RC_OK);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short setfontlpi(int lpi)
#else
short setfontlpi(lpi)
int lpi;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("print.c:   setfontlpi");
#ifdef WIN32
   SetCPI(MAKELONG(savecpi,lpi));
#endif
   TRACE_RETURN();
   return (RC_OK);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short setpagesize(int fs)
#else
short setpagesize(ps)
int ps;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("print.c:   setpagesize");
   TRACE_RETURN();
   return (RC_OK);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short setfontname(char *font)
#else
short setfontname(font)
char *font;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("print.c:   setfontname");
   TRACE_RETURN();
   return (RC_OK);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short setorient(char or)
#else
short setorient(or)
char or;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("print.c:   setorient");
#ifdef WIN32
   if (or == 'L') 
      SetOrient(DMORIENT_LANDSCAPE);
   else
      SetOrient(DMORIENT_PORTRAIT);
#endif
   TRACE_RETURN();
   return (RC_OK);
}
#endif
