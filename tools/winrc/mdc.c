/*    
	mdc.c	1.24
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "rc.h"
#include <stdlib.h>

#if !defined(macintosh)
#include <malloc.h>
#else
char *strdup(const char *s);
#endif

void DoLibraryInfo();

/* What exactly is end-of-line? */
#ifdef MSDOS
#define EOLCHARS "\r\n"
#elif defined(_MAC) || defined(macintosh)
#define EOLCHARS "\r"
#else
#define EOLCHARS "\n"
#endif

char *get_string(char *str, int len);

extern void open_modulefiles(void);
static int stripcomments (char *);
static int leftjustify(char *);

/***********************************************************************/

extern int build_app_module;
static char **modules_to_init = NULL;
static int n_modules_to_init = 0;
static char filename[256];
static char *description  = NULL;
char *modulename = "module";

static int   IsLibrary    = 0;
static int   bResourceOnly = 0;
static int   bSegmentTable = 0;

static time_t now;	        /* Hold current time for time stamp of .def.c */

extern char in_name[];
extern char cpp_module_name[];
extern char libentry_name[];

/***********************************************************************/

#define KEEP			1     /* Added to clarify arguments to
					 parse(). */
#define DUMP			0     /* Added to clarify arguments to
					 parse(). */
#define CURRENT			0     /* Added to clarify arguments to
	    				 parse(). */
#define STRIPLEADING		0     /* Added to clarify arguments to
					 parse(). */
#define NOSTRIPLEADING		10    /* Added to clarify arguments to
					 parse(). */

/***********************************************************************/

#define NONUMBER		-1    /* Index number for entry table
					 point with no number in the
					 .def file. */

/***********************************************************************/

/* Table ordinals used to represent matched keywords in the .def file. */
#define 	MD_LIB	1
#define 	MD_DESC	2
#define 	MD_EXE 	3
#define 	MD_CODE 4
#define 	MD_DATA 5
#define 	MD_HEAP 6
#define 	MD_SEGMENTS  7
#define 	MD_EXP  8
#define 	MD_NAME 9
#define 	MD_RESOURCE 10

/* Table to lookup and match keywords in the .def file. */
struct
{
  char *command;
  int	code;
} ModDefTable[] =
  {
    { "library",     MD_LIB },
    { "name",        MD_NAME },		
    { "description", MD_DESC },
    { "exetype",     MD_EXE },
    { "code",        MD_CODE },
    { "data",        MD_DATA },
    { "heapsize",    MD_HEAP },
    { "segments",    MD_SEGMENTS },
    { "exports",     MD_EXP },
    { "resource",    MD_RESOURCE },
    { 0, 0}
  };


/***********************************************************************/

void doheader(void)
/* Output the header for the .def.c. */
{
  COUTPUT(("\n/*************************************\n"));
  COUTPUT((" * File: %s\n",filename));
  COUTPUT((" * Date: %s",ctime(&now)));
  COUTPUT((" **************************************/\n")); 
  COUTPUT(("\n"));

  COUTPUT(("#include \"ModTable.h\"\n"));
  COUTPUT(("#include \"Log.h\"\n"));
  COUTPUT(("\n"));


} /* doheader */


char *
parse(char *cp,int code)
/* To break up a string and "feed" it to the caller field by field.
   The remaining string survives through subsequent calls. To access
   last string, give CURRENT to cp. Else, give a new string to override
   the stored value. A code of KEEP will store the remaining string
   untouched. A code of DUMP will nullify it. */
{
  static char *p;		/* allow p to survive over several 
				   calls to parse. */
  char *t;
  
  if(cp)
    p = (char *)strtok(cp,EOLCHARS);

  /* remove all leading tabs and spaces */
  while(p && *p && isspace(*p))
    p++;
  t = p;

  /* terminate this string... */
  while(code && p && *p)
    {
      if(isspace(*p))
	{
	  /* where we start next... */
	  *p++ = 0;
	  break;
	}
      p++;
    }
  if(code == 0)
    p = 0;
  if(t && !strcmp(t, EOLCHARS))
    t = 0;
  return t;
} /* parse */

/* Main procedure for parsing, storing, and writing out information
   from a .def to a .def.c. */

static int   max = 0;		
static char **functions;	/* Actual array allocated at runtime. */

void
DoLibraryInfo()
{
  int n;
  char entry_name[255];
  char class_name[255];
  int i,j,k;
  int cpp_exists;

  if (build_app_module)
    strcpy(entry_name, "TWIN_AppEntry_");
  else
    strcpy(entry_name, "TWIN_LibEntry_");

  strcpy(class_name, "TWIN_InitClass_");
#if !defined(macintosh)
  if (!build_app_module)
  {
    /*
     *  Entry point name includes "lib" for Unix platforms.
     */
    strcat(entry_name, "lib");
    strcat(class_name, "lib");
  }
#endif

  if (strlen(cpp_module_name) > 0)
    cpp_exists = 1;
  else
    cpp_exists = 0;
  i = strlen(entry_name);
  j = 0;
  k = strlen(class_name);
  while (modulename[j] != 0)
  {
    if (isupper(modulename[j]))
	entry_name[i] = tolower(modulename[j]);
    else
        entry_name[i] = modulename[j];
    class_name[k] = entry_name[i];
    i++;
    j++;
    k++;
  }
  entry_name[i] = 0;
  class_name[k] = 0;

  COUTPUT(("static MODULETAB ModuleDescriptorTable = {\n"));
  COUTPUT(("\tMODULE_SYSTEM, &mod_dscr\n};\n\n"));
 
  /*
   *  Do some platform-specific init code here, for those platforms
   *  that need it.  Do -not- do this if we are also creating a
   *  C++ init module.
   */ 
#if defined(linux)
  if (!cpp_exists && !build_app_module)
  {
    COUTPUT(("LIBENTRY_DECL(%s);\n",entry_name));
#ifdef LATER
    COUTPUT(("void %s()", entry_name));
    COUTPUT((" __attribute__ ((section(\".init\")));\n"));
#endif
  }
#endif
  COUTPUT(("HINSTANCE    InternalLoadDLL(MODULETAB *);\n"));
  COUTPUT(("\n"));
#if defined (macintosh)
  COUTPUT(("short\n"));
#else
  COUTPUT(("void\n"));
#endif
  COUTPUT(("%s()\n", entry_name));
  COUTPUT(("{\n"));
  COUTPUT(("\tLOGSTR((LF_CONSOLE,\"LoadLibrary: %%s \\n\",\n"));
  COUTPUT(("\t       ModuleDescriptorTable.dscr->name));\n"));
#if defined(linux)
  /*
   * On linux, -Bsymbolic does not do the right thing for
   * for symbols stored as initialized data, so we have to
   * make sure we set the functions dynamically before using
   * the table.
   */
  COUTPUT(("\tentry_tab_default[0].fun = %s;\n", libentry_name));
  for(n = 1;n<max+1;n++)
    if(functions[n] && (strlen(functions[n]) != 0))
      COUTPUT(("\tentry_tab_default[%d].fun = %s;\n", n, functions[n]));
#endif
  COUTPUT(("\tInternalLoadDLL(&ModuleDescriptorTable);\n"));
#if defined (macintosh)
  COUTPUT(("return(0);\n"));
#endif
  COUTPUT(("}\n"));

  EOUTPUT(("%s\n", entry_name));

  /*
   *  Now write out the C++ init code, if a C++ module has been requested.
   *  This creates a dummy class with a single static global instance
   *  of that class.  The constructor will call our init routine, which
   *  will initialize us when the library is loaded, due to global
   *  constructors being called.
   */
  if (cpp_exists)
  {
    CPPOUTPUT(("extern \"C\" void %s();\n", entry_name));
    CPPOUTPUT(("class %s\n", class_name));
    CPPOUTPUT(("{\n"));
    CPPOUTPUT(("public:\n"));
    CPPOUTPUT(("    %s();\n", class_name));
    CPPOUTPUT(("};\n"));
    CPPOUTPUT(("static %s TWIN_dummy;\n", class_name));
    CPPOUTPUT(("%s::%s()\n", class_name, class_name));
    CPPOUTPUT(("{\n"));
    CPPOUTPUT(("    %s();\n", entry_name));
    CPPOUTPUT(("}\n"));
  }
}

void
DoApplication()
{

  COUTPUT(("\n"));
  COUTPUT(("extern int LoadTwinModDscr(int,char **, ENTRYTAB *,MODULEDSCR *);\n"));
  COUTPUT(("extern ENTRYTAB TWIN_LibInitTable[];\n"));
  COUTPUT(("\n"));

  {
    COUTPUT(("int\nmain(int argc, char **argv)\n"));
    COUTPUT(("{\n"));
    COUTPUT(("\tint    rc;\n"));
    COUTPUT(("\textern BOOL bIsXWin;\n"));
    COUTPUT(("\textern BOOL bIsBinaryApp;\n"));
    COUTPUT(("\n"));
    COUTPUT(("\tbIsBinaryApp = bIsXWin;\n"));
    COUTPUT(("\n"));
    COUTPUT(("\trc = LoadTwinModDscr(argc,argv,TWIN_LibInitTable,&mod_dscr);\n"));
    COUTPUT(("\n"));
    COUTPUT(("\treturn ExitWindows(rc==WM_QUIT?0:rc,0);\n"));

    COUTPUT(("}\n"));
  }
  
  COUTPUT(("\n"));
}

static   int   currentnum = 0;	

struct table_point
{
    int index;
    char *name;
    struct table_point *next;
};

static struct table_point *entrytable=NULL;

void
parse_module()
{
static   int   havecomment = 0;  
static   int   havekeyword = 0; 
static   int   valid_entry = 0;  
static   char cmdline[256];
static   char *cp,*sp;
static   int   n,code;

  /* Structure list to hold entry table to allow for dynamic
     allocation of entry table space. */
	struct table_point *tempentry;

  n = 0;
  
  open_modulefiles();
  
  /* Go into loop to search for keywords and then fill in the 
     appropriate information given by those keywords. */
  /* NOTE: havekeyword || is used to be able to bypass
     string reads if exiting from EXPORT section
     due to a left-justified *ALPHABETIC* character. */
  /* NOTE: Left-justified *ALPHABETIC* character supposed to
     always cause keyword lookup and break from
     whatever mode (i.e. EXPORT). */

  while (havekeyword  || (cp = get_string(cmdline,256))) {
    
    havekeyword=0;		/* Reset havekeyword. */
    sp = (char *)parse(cp,KEEP);

    if(sp == NULL)
      continue;

    code = 0;
    for(n=0;ModDefTable[n].command;n++)
      if(strcasecmp(ModDefTable[n].command,sp) == 0)
	{
	  code = ModDefTable[n].code;
	  break;
	}
    if(code)
      {
	switch(code)
	  {
	    /* Fill in module name. */
	  case MD_NAME:
	    sp = parse(CURRENT,DUMP);
	    if(sp && strlen(sp))
 	    	modulename = strdup(sp);
	    break;

	  case MD_LIB:
	    n = 0;
	    IsLibrary++;
	    sp = parse(CURRENT,DUMP);
	    if(sp && strlen(sp))
 	    	modulename = strdup(sp);
	    break;

	    /* Fill in library description. */
	  case MD_DESC:
	    n = 0;
	    /* get the whole string... */
	    sp = parse(CURRENT,DUMP);
	    if(sp && strlen(sp))
	      {
		if(*sp == '\'')
		  {
		    sp++;
		    cp = (char *) strrchr(sp,'\'');
		    if(cp)
		      *cp = 0;
		  }
		if(description)
		  WARNING(("Overriding module description string (was %s, now %s)",sp,description));
		else
		  description = (char *) strdup(sp);
		break;
	      }
	    break;

	  /* ideally make this like exports... */
	  case MD_SEGMENTS:
	    bSegmentTable = 1;
	    break;

	  case MD_RESOURCE:
	    bResourceOnly = 1;
	    sp = parse(CURRENT,DUMP);
	    if(sp && strlen(sp))
 	    	strcpy(tbl_name,sp);
	    break;

	    /* Start parsing EXPORT section. */
	  case MD_EXP:
	    max = 0;
	    while ((cp = get_string(cmdline,256)))
	      {
		if(leftjustify(cmdline))
		  {
		    havekeyword=1;
		    break;
		  }
		valid_entry = 1;
		havecomment = 0;
		sp = parse(cmdline,KEEP);
		cp = parse(CURRENT,DUMP);
		/* Take care of comments */
		havecomment=stripcomments(sp);	
		if(sp && *sp=='\000') 
		  valid_entry=0;
		/* Get entry point number.  A no-number point gets NONUMBER value. */
		if(cp && *cp == '@' && !havecomment)
		  n = atoi(cp+1);
		else 
		  if(cp)
		    n = NONUMBER;
		  else
		    valid_entry=0;
		if(valid_entry)
		  {
		    if(n > max)
		      max = n;
		    /* Increment entry point number count. */
		    currentnum++;
		    /* Allocate space for another entry and fill in info. */
		    tempentry=(struct table_point *)malloc(sizeof(struct table_point));
		    tempentry->index=n;
		    tempentry->name=(char *) strdup(sp);
		    tempentry->next=entrytable;
		    entrytable = tempentry;
		  }
	      }
	    
	    break;
	  default:
	    continue;
	  }
      }
  }

}

void
print_module()
{
  int n;
  struct table_point *tempentry;
  int   index;
 
  if(bResourceOnly) 
	return;

  /* Create new array out of linked list of entry points. */
  /* Check that there will be sufficient space in final array to
     contain both numbered and unnumbered entry points. */

  if (currentnum + 1 > max)
    max = currentnum + 1;
  functions=(char **)calloc(max+1,sizeof(char *));


  /* Set the default entry point name for libraries. */
  if ( *libentry_name == '\0' )
	strcpy(libentry_name, "LibMain");

  if (build_app_module)
      strcpy(libentry_name, "LibMainStub");

  /* Initialize table. */
  for(n=0;n<=max;n++)
    functions[n]=NULL;
  
  tempentry=entrytable;	/* Save copy of entrytable head before
			   traversing. */
  
  /* Fill in numbered entries. */
  while(entrytable)
    {
      if((entrytable->index)>0)
	functions[entrytable->index]= entrytable->name;
      entrytable=entrytable->next;
    }
  
  entrytable=tempentry;	/* Restore head of list. */
  
  /* Fill in non-numbered entries. */
  while(entrytable)
    {
      if((entrytable->index)<0)
	{
	  n=max;
	  while(functions[n])
	    n--;
	  functions[n]=entrytable->name;
	}
      entrytable=entrytable->next;
    }

  if (build_app_module) {
    n=max;
    while(functions[n])
      n--;
    functions[n]=malloc(strlen(modulename) + 9);
    strcpy(functions[n], modulename);
    strcat(functions[n], "_WinMain");
  }

  /* Print the .def.c header. */
  time(&now);
  
  doheader();
  
  /* Print out prototypes for entry points. */
  for(n=1;n<max+1;n++)
    if(functions[n] && strlen(functions[n]))
      COUTPUT(("extern long int %s(); \n",functions[n]));

  /* Print out actual entry point table. */
  if (IsLibrary) {
  	COUTPUT(("\n#include \"twindll.h\"\n\n"));
	COUTPUT(("long int %s();\n", libentry_name));
  }
  else if (build_app_module) {
  	COUTPUT(("\n#include \"twindll.h\"\n\n"));
	COUTPUT(("long int %s() {return 0;}\n", libentry_name));
  } else
	COUTPUT(("int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);\n"));

  COUTPUT(("\nstatic ENTRYTAB entry_tab_default[] = \n{\n"));

  if (!description)
      description = modulename?modulename:"TWIN Application";
  if (build_app_module)
      description = libentry_name;

  COUTPUT(("\t/* 000 */ { \"%s\",0,0,%s},\n", 
	description,
	(IsLibrary||build_app_module)?libentry_name:"(void *)WinMain"));

  index = 8;
  for(n = 1;n<max+1;n++)
    {
      if((functions[n] == 0) || (strlen(functions[n]) == 0))
	COUTPUT(("\t/* %3.3x */\t{ \"\",0,0,0 },\n",n));
      else
	{
	  COUTPUT(("\t/* %3.3x */\t{ \"%s\",0x%4.4x,0x%4.4x,%s },\n",
		   n,functions[n],8,index,functions[n]));
	  EOUTPUT(("%s\n", functions[n]));
	}
      index += 8;
    }
  COUTPUT(("\t/* end */ { 0, 0, 0, 0 }\n};\n\n"));
  
  
  if(bSegmentTable)
  	COUTPUT(("extern SEGTAB %s%s[];\n",
		"SegmentTable",modulename?modulename:""));
  /* Print out remaining "standard" .def.c stuff. */
  COUTPUT(("static MODULEDSCR mod_dscr = \n"));
  COUTPUT(("{    \"%s\",\n",modulename?modulename:""));
  COUTPUT(("\tentry_tab_default,\n"));

  /* put out a segment table, with option modulename */
  if(bSegmentTable)
  	COUTPUT(("\t%s%s,\n","SegmentTable",modulename?modulename:""));
  else
  	COUTPUT(("\t0,\n"));
  COUTPUT(("\t(long *) &hsmt_resource_table\n};\n\n"));
  
  if(IsLibrary||build_app_module) 
	DoLibraryInfo();
  else {
	DoApplication();
  }

  /* Clean up allocated memory. */
  for(n=0;n<=max;n++)
    free(functions[n]);
  entrytable=tempentry;
  while(entrytable)
    {
      tempentry=entrytable->next;
      free(entrytable);	
      entrytable=tempentry;
    }
} /* domodule */

static int stripcomments(char *thestring)
/* Strip off comments designated by a semicolon. */
{
  int hadcomment = 0;
  
  while(thestring && *thestring)
    {
      if(*thestring==';') 
	{
	  *thestring='\000';
	  hadcomment=1;
	}
      thestring++; 
    }
  return hadcomment;
}

static int leftjustify(char *thestring)
/* Check for left-justification of ALPHABETIC character. */
{
  return isalpha((int) *thestring);
}

void define_used_module(char *modulename)
/* Explicitly call initialization functions for module named here */
{
    int idx;
    
    if (!modulename)
	return;
    
    idx = n_modules_to_init++;
    if (n_modules_to_init == 1)
    {
	modules_to_init = malloc(sizeof(*modules_to_init));
    }
    else
    {
	int new_size = n_modules_to_init * sizeof(*modules_to_init);
	modules_to_init = realloc(modules_to_init, new_size);
    }

    modules_to_init[idx] = strdup(modulename);
}

