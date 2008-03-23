/*    
	Profile.c	2.30
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

 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "windows.h"
#include "Log.h"

typedef struct {
    FILE   *fp;			/* input FILE *     */
    FILE   *fpout;		/* output FILE *     */
    int     eof;		/* is fp at eof?    */
    int     newsection;		/* have we hit a new section */
    int     flag;		/* output file write options */
    char     linebuffer[512];	/* current working inputline */
    char    sectionname[128];	/* section name we want to read */
    char   *source;		/* name of input .ini file   */
    char    output[512];	/* constructed .ini.new file */
    char    *pnext;
} PROFILESTRING;

#define WRITESECTION    1
#define READWRITEALL    2

static LPSTR strtrim(LPSTR,LPINT);
static void readline(PROFILESTRING *);
static FILE * openfile(PROFILESTRING *,LPCSTR,char *);
static int InternalGetPrivateProfileString(LPCSTR,LPCSTR,LPCSTR,
			LPSTR,int,LPCSTR,BOOL);

static LPSTR
strtrim( LPSTR str, LPINT len )
{
    char *s = str;    
    int	      slen;

    if (s) {
	while (*s  &&  isspace(*s))
	    ++s;
        if (strlen(s) == 0)
		slen = 0;
	else {
		for (slen=strlen(s); slen>=0  &&  isspace(*(s-1+slen)); (slen)--);
		*(s + slen) = 0;
	}
	if (len)
		*len = slen;
    }
    return s;
}

/*******************************************************************/
static void
readline(PROFILESTRING *wp)
{
    if(wp->eof)
	return;    

    wp->newsection = 0;

    if(fgets(wp->linebuffer,512,wp->fp) == 0) {
	wp->eof++;
	wp->linebuffer[0] = 0;
	return;
    }

    wp->linebuffer[strlen(wp->linebuffer)-1] = 0;

    if(wp->linebuffer[0] == '[')
	wp->newsection++;
}


static FILE *
openfile(PROFILESTRING *wp, LPCSTR lpszFileName,char *mode)
{
	LPSTR	lpName = (LPSTR) lpszFileName;
	FILE	*fp;
	int	n;
	char	buf[_MAX_PATH];

	if ( !lpName )
		lpName = (LPSTR)"win.ini";

	if (!strcasecmp(lpName, "win.ini")
	 || !strcasecmp(lpName, "system.ini")
	 || ((strchr(lpName, '/') == NULL) && (strchr(lpName, '\\') == NULL)))
	{
		if ( !(n = GetWindowsDirectory(buf, sizeof(buf))) )
			return 0;
		buf[n]   = '/';
		buf[n+1] = '\0';
		strcat(buf, lpName);
		if ((fp = fopen(buf, mode))) {
			if ( wp  &&  wp->source ) 
				strcpy(wp->source, buf);
			return fp;
		}
	}

	if ( wp  &&  wp->source ) 
		strcpy(wp->source, lpName);

	if ( !(fp = fopen(lpName, mode))  &&  wp )
		wp->eof++;

	return fp;
}


static int
InternalGetPrivateProfileString(LPCSTR lpszSection, LPCSTR lpszEntry,
	    LPCSTR lpszDefault, LPSTR lpszReturnBuffer,
	    int cbReturnBuffer, LPCSTR lpszFileName, BOOL fNeedValues)
{
    PROFILESTRING p;
    char inputline[512];
    char *value;
    FILE *fp;
    int  cEntLen, cSectLen, cLen, count = 0;
    LPSTR lpszSectionDup,lpszEntryDup;
    char *lpszSect, *lpszTEntry, *lpszInline, *lpszValue;
    char *lpRightBracket;

    /* open lpszFileName or win.ini */
    fp = openfile(0,lpszFileName,"r");

    *lpszReturnBuffer = 0;

    if(fp == 0) {
	if(lpszDefault && *lpszDefault) {
	    count = cbReturnBuffer <= (int)strlen(lpszDefault)?
		cbReturnBuffer-1:(int)strlen(lpszDefault);
	    strncpy(lpszReturnBuffer,lpszDefault,count+1);
	} else  {
	    count = 0;
	}
	return count;
    }

    p.source   = lpszReturnBuffer;
    p.pnext    = lpszReturnBuffer;
    lpszSectionDup = (lpszSection)?
	(LPSTR)WinStrdup((LPSTR)lpszSection):(LPSTR)NULL;
    lpszSect   = strtrim( lpszSectionDup, &cSectLen );
    lpszEntryDup = (lpszEntry)?(LPSTR)WinStrdup((LPSTR)lpszEntry):(LPSTR)NULL;
    lpszTEntry = strtrim( lpszEntryDup, &cEntLen );

    while ( fgets(inputline, sizeof(inputline), fp) ) {
	if(strlen(inputline) == 0)
	    continue;
	inputline[strlen(inputline)-1] = 0;
	if(*inputline == '[') {
	    if(lpszSection == 0 || strlen(lpszSection) == 0) {
		if ((count + (int)strlen(inputline) + 1) > cbReturnBuffer)
		    break;
		strcat(p.pnext,inputline);
		p.pnext += strlen(inputline);
		*p.pnext++ = 0;
		count += strlen(inputline) + 1;
		continue;
	    }
	    if ((lpRightBracket = strchr(inputline+1,']')))
		*lpRightBracket = '\0';
	    else
		continue;
	    if(strcasecmp(inputline+1,lpszSect) == 0) {
		while(fgets(inputline,512,fp)) {
		    if ( strlen(inputline) == 0  ||
				*inputline == ';' ||
				*inputline == '#' ||
				*inputline == '\n' ||
				*inputline == '\r')
			continue;
		    inputline[strlen(inputline)-1] = 0;

		    if(*inputline == '[')
			break;

		    if (!fNeedValues) {
			value = strchr(inputline, '=');
			if(value)
			    *value++ = 0;
			lpszValue = strtrim(value, (int *)NULL);
			lpszInline = strtrim(inputline, &cLen);
		    }
		    else {
			lpszValue = 0;
			lpszInline = inputline;
			cLen = strlen(inputline);
		    }
		
		    if(lpszTEntry == 0 || strlen(lpszTEntry) == 0) {
			/* List of all entries */
			if ((count + cLen + 1) > cbReturnBuffer) 
			    cLen = cbReturnBuffer - count - 2;
			if (cLen <= 0)
			   break;
			strncpy(p.pnext,lpszInline,cLen);
			p.pnext += cLen;
			*p.pnext++ = 0;
			count += cLen+1;
			if (count < cbReturnBuffer)
			    *p.pnext   = 0;
			else
			    break;
			continue;
		    }
	    
		    if(strcasecmp(lpszInline,lpszTEntry) == 0) {
			if(lpszValue) {
			    count = min((int)strlen(lpszValue),cbReturnBuffer-1);
			    strncpy(p.pnext,lpszValue,count);
			    p.pnext += count;
			    *p.pnext++ = 0;
			    count++;
			}
			break;
		    }
		}
		break;
	    } else  continue;
	} else continue;
    }

    fclose(fp);
    if (lpszSectionDup)
	WinFree(lpszSectionDup);
    if (lpszEntryDup)
	WinFree(lpszEntryDup);

    if(strlen(p.source) == 0 && lpszDefault)
	strcpy(p.source,lpszDefault);
    return (count)?count-1:0;
}

int     WINAPI 
GetProfileString(LPCSTR sect, LPCSTR key, LPCSTR dflt, LPSTR lpreturn, int len)
{
    int	rc;
    APISTR((LF_APICALL,
	"GetProfileString(LPCSTR=%s,LPCSTR=%s,LPCSTR=%s,LPSTR=%x,int %d)\n",
	(sect)?sect:"NULL",
	(key)?key:"NULL",
	(dflt)?dflt:"NULL",
	lpreturn,
	len));
    rc = InternalGetPrivateProfileString(sect,key,dflt,lpreturn,len,0,FALSE);
    APISTR((LF_APIRET,"GetProfileString: returns int %d\n",rc));
    return rc;
}

int
GetPrivateProfileString(LPCSTR lpszSection, LPCSTR lpszEntry,
	    LPCSTR lpszDefault, LPSTR lpszReturnBuffer,
	    int cbReturnBuffer, LPCSTR lpszFileName)
{
    int rc;
    APISTR((LF_APICALL, 
	"GetPrivateProfileString(LPCSTR=%s,LPCSTR=%s,LPCSTR=%s,LPSTR=%x,int %d,LPCSTR=%s)\n",
	lpszSection?lpszSection:"NULL",
	lpszEntry?lpszEntry:"NULL",
	lpszDefault?lpszDefault:"NULL",
	lpszReturnBuffer,
	cbReturnBuffer,
	(lpszFileName)?lpszFileName:"NULL"));

    rc = InternalGetPrivateProfileString(lpszSection,lpszEntry,lpszDefault,
		lpszReturnBuffer,cbReturnBuffer,lpszFileName,FALSE);
    APISTR((LF_APIRET,"GetPrivateProfileString: returns int %d\n",rc));

    return rc;
}

UINT WINAPI
GetPrivateProfileInt(LPCSTR sect, LPCSTR key, int defaultrc, LPCSTR fp)
{
    	char abuf[80];    
	UINT rc = defaultrc; 

	APISTR((LF_APICALL, 
	 "GetPrivateProfileInt(LPCSTR=%s,LPCSTR=%s,int=%d,LPCSTR=%s)\n", 
		(sect)?sect:"NULL", 
		(key)?key:"NULL", 
		rc, 
		(fp)?fp:"NULL")); 
    
	if(InternalGetPrivateProfileString(sect,key,"",abuf,80,fp,FALSE))
		rc = strtol(abuf,0,0);

    	APISTR((LF_APIRET,"GetPrivateProfileInt: returning %d\n",rc));
    	return rc;
}

UINT    WINAPI 
GetProfileInt(LPCSTR sect, LPCSTR key, int defaultrc)
{
    	char abuf[80];    
	UINT rc = defaultrc; 

	APISTR((LF_APICALL, 
	 "GetProfileInt(LPCSTR=%s,LPCSTR=%s,int=%d)\n", 
		(sect)?sect:"NULL", 
		(key)?key:"NULL", 
		defaultrc));
    
	if(InternalGetPrivateProfileString(sect,key,"",abuf,80,0,FALSE))
		rc = strtol(abuf,0,0);

    	APISTR((LF_APIRET,"GetProfileInt: returning %d\n",rc));
    	return rc;
}

/**************************************************************************
	Win32-specific APIs
**************************************************************************/

DWORD	WINAPI
GetProfileSection(LPCSTR lpszSection, LPSTR lpBuffer, DWORD dwSize)
{
    DWORD rc;
    APISTR((LF_APICALL,
	"GetProfileSection(LPCSTR=%s,LPSTR=%x,DWORD=%x)\n",
		lpszSection?lpszSection:"NULL",
		lpBuffer,
		dwSize));
    rc = (DWORD)InternalGetPrivateProfileString(lpszSection,"","",
		lpBuffer,(int)dwSize,NULL,TRUE);
    APISTR((LF_APIRET,"GetProfileSection: returning DWORD %x\n",rc));
    return rc;
}

DWORD	WINAPI
GetPrivateProfileSection(LPCSTR lpszSection, LPSTR lpBuffer, DWORD dwSize,
			LPCSTR lpszFileName)
{
    DWORD rc;
    APISTR((LF_APICALL,
	"GetPrivateProfileSection(LPCSTR=%s,LPSTR=%s,DWORD=%x,LPCSTR=%s)\n",
	lpszSection?lpszSection:"NULL",
	lpBuffer,
	dwSize,
	lpszFileName?lpszFileName:"NULL"));
    
    rc =  (DWORD)InternalGetPrivateProfileString(lpszSection,"","",
		lpBuffer,(int)dwSize,lpszFileName,TRUE);
    APISTR((LF_APIRET,"GetPrivateProfileSection: returning DWORD %x\n",rc));
    return rc;
}

/**************************************************************************
**   Michael Dvorkin 09-JAN-1996 
**
**   when lpszString == NULL the function should remove the lpszEntry
**   from the file (for instance when we're saving drive mappings).
**************************************************************************/
BOOL WINAPI
WritePrivateProfileString(LPCSTR lpszSection, LPCSTR lpszEntry,
	      LPCSTR lpszString, LPCSTR lpszFileName)
{
    PROFILESTRING wps,*wp;
    char    sourcefile[512];
    LPSTR   lpstr;
    LPSTR   lpsection = (LPSTR)lpszString;

    APISTR((LF_APICALL,
	"WritePrivateProfileSection(LPCSTR=%s,LPSTR=%s,LPCSTR=%s,LPCSTR=%s)\n",
	lpszSection?lpszSection:"NULL",
	lpszEntry?lpszEntry:"NULL",
	lpszString?lpszString:"NULL",
	lpszFileName?lpszFileName:"NULL"));
    


    if (!lpszSection) {
     	APISTR((LF_APIRET,"WritePrivateProfileSection: returning BOOL FALSE\n"));
	return FALSE;
    }

    wp = &wps;
    wp->source = sourcefile;
    wp->eof = 0;
    /*   
    **   Open lpszFileName or win.ini (if it doesn't exist, create it)
    */
    if ((wp->fp = openfile(wp,lpszFileName,"r")) == 0) 
	if ((wp->fp = openfile(wp,lpszFileName,"a+")) == 0) {
     	    APISTR((LF_APIRET,"WritePrivateProfileSection: returning BOOL FALSE\n"));
	    return FALSE;
	}
    /* 
    **   Copy whatever file we did open to source and make a .new name 
    */
    strcpy(wp->output,wp->source);
    strcat(wp->output,".new");
    /* 
    **   Open the .new file for output
    */
    wp->fpout = fopen(wp->output,"w");
    if (wp->fpout == 0) {
     	APISTR((LF_APIRET,"WritePrivateProfileSection: returning BOOL FALSE\n"));
	return FALSE;
    }

    wp->flag = WRITESECTION | READWRITEALL;
    sprintf(wp->sectionname,"[%s]",lpszSection);
    if (lpszEntry == 0)
       wp->flag &= ~WRITESECTION;

    for (;;) 
        {
	readline(&wps);
	if (wp->eof)    
	   break;
	
	if (wp->newsection == 0) 
           {
	   fprintf(wp->fpout,"%s\n",wp->linebuffer);
	   continue;
	   }

	if (strcasecmp(wp->linebuffer,wp->sectionname) == 0) 
           {
	   if ((wp->flag & WRITESECTION) == 0) 
              {
              /*   Just skip contents of this section...
              */
	      for(;;)
                 {
		 readline(&wps);
		 if (wp->eof)    
		    break;
		 if (wp->newsection)
		    break;
		 }
	      break;
	      }
	   fprintf(wp->fpout,"[%s]\n",lpszSection);
	   wp->flag &= ~WRITESECTION;
	   for (;;) 
                {
		readline(&wps);
		if (wp->eof)
		    break;
                if (wp->linebuffer[0] == '\0')
                   continue; /* skip empty lines */
		if (wp->newsection) 
                   {
		   /*   We didn't find the entry, so write
		   **   the new values, if not truncating 
                   */
		   if (lpsection)
		      fprintf(wp->fpout,"%s=%s\n",lpszEntry,lpsection);
                   /*
		   **   Don't write it again... 
		   **   this will keep linebuffer with the
		   **   new section name intact so we can
		   **   write it out below... 
                   */
		   lpsection = 0;
		   break;
		   }
		lpstr = (LPSTR)strchr(wp->linebuffer,'=');
		if (lpstr) {
		  LPSTR keyend = lpstr;
		  int keylen = 0;
		  char key[80];
		  /* skip over whitespace */
		  while(lpstr[1] == ' ' || lpstr[1] == '\t')
		    lpstr++;
		  while(keyend[-1]==' ' || keyend[-1] == '\t')
		    keyend--;
		  keylen = (int)(keyend - wp->linebuffer);
		  strncpy(key, wp->linebuffer, keylen);
		  key[keylen] = '\0';
		  /*   
		   **   Check the entry is what we want 
		   */
		  if (strcasecmp(key, lpszEntry) == 0) 
		    {
		      /*   Check current value of entry against new value
		       **   skip the check if the new value is NULL
		       */
		      if (lpsection && strcasecmp(lpstr+1,lpsection) == 0) 
			{
			  fclose(wp->fp);
			  fclose(wp->fpout);
			  unlink(wp->output);
			  return TRUE;
			}
		      else if (lpsection == NULL)
			continue; /* "erase" this entry! */
		      /* 
		       **   We found the entry, and need to write a new line 
		       */
		      else break;
		    }
		}
		fprintf(wp->fpout,"%s\n",wp->linebuffer);
	      }
	    /*   
            **   Re-write the entry if lpsection is not null
	    **   lpsection == 0 means don't overwrite new section 
            */
	    if (lpsection)
	       sprintf(wp->linebuffer,"%s=%s",lpszEntry,lpsection);
	    break;
	    }
        /*
	**   This is only written when we are still in the desired
	**   section, but haven't found the entry we want, if we hit
	**   a new section we will have broken out above 
        */
	fprintf(wp->fpout,"%s\n",wp->linebuffer);
        }

    if (wp->flag & WRITESECTION) 
       {
       fprintf(wp->fpout,"[%s]\n",lpszSection);
       sprintf(wp->linebuffer,"%s=%s",lpszEntry,lpsection);
       }
    /*   
    **   The new section or new entry hasn't been printed yet... 
    */
    if (wp->flag & READWRITEALL) 
       {
       for(;;) 
          {
          if (strlen(wp->linebuffer))
	     fprintf(wp->fpout,"%s\n",wp->linebuffer);
	  readline(wp);
	  if (wp->eof)
	     break;
	  }
       }
    if (wp->fp) 
	fclose(wp->fp);
    fclose(wp->fpout);

    /* remove the previous file */
    unlink(wp->source);

    /* rename the temp file to the original file */
    rename(wp->output,wp->source);
    APISTR((LF_APIRET,"WritePrivateProfileSection: returning BOOL TRUE\n"));
    return TRUE;
}

BOOL WINAPI
WriteProfileString(LPCSTR lpszSection, LPCSTR lpszEntry,LPCSTR lpszString)
{
    BOOL rc;
    APISTR((LF_APICALL,
	"WriteProfileStrin(LPCSTR=%s,LPCSTR=%s,LPCSTR=%s)\n",
	lpszSection?lpszSection:"NULL",
	lpszEntry?lpszEntry:"NULL",
	lpszString?lpszString:"NULL"));
    
    rc = WritePrivateProfileString(lpszSection,lpszEntry,lpszString,0);
    APISTR((LF_APIRET,"WriteProfileSection: returning BOOL %x\n"));
    return rc;
}

/* (WIN32) Profile ********************************************************* */

DWORD WINAPI
GetPrivateProfileSectionNames(LPTSTR lpszReturnBuffer,
	DWORD nSize,
	LPCTSTR lpFileName)
{
	FILE *file;
	char buffer[BUFSIZ];
	char *ptr;
	size_t len;
	DWORD nResult;

	APISTR((LF_APICALL,
		"GetPrivateProfileSectionNames(LPTSTR=%s,DWORD=%x,LPCTSTR=%s)\n",
		lpszReturnBuffer?lpszReturnBuffer:"NULL",
		nSize,
		lpFileName?lpFileName:"NULL"));

	if ((lpszReturnBuffer == NULL)
	 || (nSize == 0)
	 || (lpFileName == NULL)
	 || ((file = openfile(NULL, lpFileName, "r")) == NULL)
	) {
     	 	APISTR((LF_APIRET,"GetPrivateProfileSection: returning DWORD %x\n",
			0));
		return (0);
	}

	*lpszReturnBuffer = '\0';
	nResult = 0;

	while (fgets(buffer, sizeof(buffer), file) != NULL)
	{
		if (*buffer != '[') continue;
		ptr = strtok(buffer + 1, "]");
		len = strlen(ptr);
		strncpy(lpszReturnBuffer + nResult, ptr, nSize - nResult);
		if (len >= nSize - nResult)
		{
			lpszReturnBuffer[nSize - 1] = '\0';
			lpszReturnBuffer[nSize - 2] = '\0';
			nResult = nSize - 2;
			break;
		}
		nResult += len + 1;
	}

	fclose(file);

     	APISTR((LF_APIRET,"GetPrivateProfileSection: returning DWORD %x\n",nResult));
	return (nResult);

}

BOOL WINAPI
WritePrivateProfileSection(LPCTSTR lpszSection, LPCTSTR lpszKeysValues,
	LPCTSTR lpszFileName)
{
	char key[BUFSIZ];
	char *value;
	char *lpstr = (char *) lpszKeysValues;

	APISTR((LF_APICALL,
		"WritePrivateProfileSection(LPCTSTR=%s,LPCTSTR=%s,LPCTSTR\%s)\n",
		lpszSection?lpszSection:"NULL",
		lpszKeysValues?lpszKeysValues:"NULL",
		lpszFileName?lpszFileName:"NULL"));

	for ( ;
	     *lpstr != '\0';
	     lpstr += strlen(lpstr) + 1)
	{
		strncpy(key, lpstr, sizeof(key));
		if ((value = strchr(key, '=')) == NULL)
			break;
		*value = '\0';		/* terminate key with '\0' */
		value += 1;
		if (!WritePrivateProfileString(lpszSection, key, value,
			lpszFileName)) {
     			APISTR((LF_APIRET,
			   "WritePrivateProfileSection: returning BOOL FALSE\n"));
			return (FALSE);
		}
	}
     	APISTR((LF_APIRET,"WritePrivateProfileSection: returning BOOL TRUE\n"));
	return (TRUE);
}

BOOL WINAPI
WriteProfileSection(LPCTSTR lpszSection, LPCTSTR lpszKeysValues)
{
	BOOL rc;
	APISTR((LF_APICALL,
		"WriteProfileSection(LPCTSTR=%s,LPCTSTR=%s)\n",
		lpszSection?lpszSection:"NULL",
		lpszKeysValues?lpszKeysValues:"NULL"));

	rc = WritePrivateProfileSection(lpszSection, lpszKeysValues,
		"wini.ini");
     	APISTR((LF_APIRET,"WriteProfileSection: returning BOOL %x\n"));
	return rc;
}

static char hex_digit[] = "0123456789abcdef";

static int decode_hex(int c)
{
	c = toupper(c);
	c -= '0';
	if (c > 9) c -= 7;
	return (c);
}

BOOL WINAPI
GetPrivateProfileStruct(LPCTSTR lpszSection,
	LPCTSTR lpszKey,
	LPVOID lpStruct,
	UINT uSizeStruct,
	LPCTSTR szFile)
{
	char valuebuf[BUFSIZ];
	char *src, *dst;

	APISTR((LF_APICALL,
	   "GetPrivateProfileStruct(LPCTSTR=%s,LPCTSTR=%s,LPVOID=%s,UINT=%x,LPCTSTR=%s)\n",
		lpszSection?lpszSection : "NULL",
		lpszKey?lpszKey : "NULL",
		lpStruct?lpStruct  : "NULL",
		uSizeStruct,
		szFile?szFile : "NULL"));

	if (!GetPrivateProfileString(lpszSection, lpszKey, "", valuebuf,
		sizeof(valuebuf), szFile)) {
     		APISTR((LF_APIRET,"GetPrivateProfileStruct: returning BOOL FALSE\n"));
		return (FALSE);
	}

	/* convert hex string to struct bytes */
	if (strlen(valuebuf) / 2 > uSizeStruct)
		return (FALSE);
	src = valuebuf;
	dst = (char *) lpStruct;
	for ( ; *src != '\0'; src += 2, dst++)
		*dst = decode_hex(src[0]) << 4 | decode_hex(src[1]);

     	APISTR((LF_APIRET,"GetPrivateProfileStruct: returning BOOL TRUE\n"));
	return (TRUE);

}

BOOL WINAPI
WritePrivateProfileStruct(LPCTSTR lpszSection,
	LPCTSTR lpszKey,
	LPVOID lpStruct,
	UINT uSizeStruct,
	LPCTSTR szFile)
{
	char valuebuf[BUFSIZ];
	char *value, *src, *dst;
	BOOL rc;

	APISTR((LF_APICALL,
	   "WritePrivateProfileStruct(LPCTSTR=%s,LPCTSTR=%s,LPVOID=%s,UINT=%x,LPCTSTR=%s)\n",
		lpszSection?lpszSection : "NULL",
		lpszKey?lpszKey : "NULL",
		lpStruct?lpStruct : "NULL",
		uSizeStruct,
		szFile?szFile : "NULL"));

	if (lpStruct == NULL)
		value = NULL;
	else
	{
		value = valuebuf;
		/* encode struct bytes as hex string */
		if (2 * uSizeStruct + 1 > sizeof(valuebuf)) {
     			APISTR((LF_APIRET,"WritePrivateProfileStruct: returning BOOL FALSE\n"));
			return (FALSE);
		}
		src = (char *) lpStruct;
		dst = valuebuf;
		for ( ; uSizeStruct-- > 0; src++, dst += 2)
		{
			dst[0] = hex_digit[(*src & 0xf0) >> 4];
			dst[1] = hex_digit[*src & 0x0f];
		}
		*dst = '\0';
	}

	rc = WritePrivateProfileString(lpszSection, lpszKey, value, szFile);

     	APISTR((LF_APIRET,"WritePrivateProfileStruct: returning BOOL %x\n",rc));

	return rc;
}

