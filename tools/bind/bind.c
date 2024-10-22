/*!
 *  @brief FamilyAPI Binder
 *
 *  (c) osFree Project 2024, <https://www.osfree.org>
 *  for licence see licence.txt in root directory, or project website
 *
 *  @author Yuri Prokushev <yuri.prokushev@gmail.com>
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef __UNIX__
#include <stat.h>
#else
#include <direct.h>
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef HANDLE
#define HANDLE WORD
#endif

#ifndef HGLOBAL
#define HGLOBAL HANDLE
#endif

#include "newexe.h"
#include "omf.h"

char func[255];
typedef struct _apientry {
	char mod[9];				/*!< Module name */
	WORD ord;					/*!< Function ordinal */
	char func[21];				/*!< Function name */
	DWORD offset;	  			/*!< Offset of pointer to fixup */
	struct _apientry * next;	/*!< Next entry */
} apientry;

typedef struct _opts {
	int quiet;					/*!< Quiet mode */
	int logo;					/*!< Show logo */
	char outfile[_MAX_PATH];	/*!< Output filename */
	char infile[_MAX_PATH];		/*!< Input filename */
	char mapfile[_MAX_PATH];	/*!< MAP filename */
	char libpath[_MAX_PATH];	/*!< Additional library search path */
	int map;					/*!< Generate MAP file for DOS stub */
	int dosformat;				/*!< Options in DOS format (not UNIX) */
	int DoscallsLIB;			/*!< Use DOSCALLS.LIB instead of OS2.LIB */
	int MouAPI;
	int KbdAPI;
	int VioAPI;
	int DLLAPI;
} opts;

apientry * apiroot;
opts options;

/*! @brief Output usage help information */
void printhlp(void);
/*! @brief Check required environment to run */
int check_environment(void);
/*! @brief Add imported function info to linked list */
int addtolist(char * mod, char * func);
/*! @brief Bind stub file to NE executable */
int bind(char * fname);
/*! @brief Find function name in lib by ordinal and module name */
char * findfunctionname(char * module, WORD ordinal, char * lib);
/*! @brief Search library in lib paths */
int searchlib(char * libname, char * fullpath);
/*! @brief Concatecate path separator */
char * addpathsep(char * buf);
/*! @brief Create and return temporary directory */
char * mktmpdir(char *tmpdir);
/*! @brief Create imptable object file */
void generate_imptable(void);
/*! @brief Generate lnk file */
void generate_lnk(void);


/*! @brief Concatecate path separator */
char * addpathsep(char * buf)
{
#if defined(_WIN32) || defined(__OS2__) || defined(__DOS__)
    return strncat(buf,"\\", 1);
#else
    return strncat(buf,"/", 1);
#endif
}

/*! @brief Create and return temporary directory */
char * mktmpdir(char *tmpdir)
{
    char name2[_MAX_PATH];
    tmpnam(name2);
#if defined(_WIN32) || defined(__OS2__) || defined(__DOS__)
    tmpdir = getenv("TMP");
#else
    tmpdir = "/tmp";
#endif
	addpathsep(tmpdir);
    strcat(tmpdir,&name2);
	mkdir(tmpdir);
	return addpathsep(tmpdir);
}

/*! @brief Generate lnk file */
void generate_lnk(void)
{
	FILE * f;
                            // Generate link file
                            f=fopen("bind.lnk", "w");
                            fputs("system dos\n",f);
                            fputs("name fstub.exe\n" , f); 
                            fputs("file tmp.obj\n", f);
                            if (options.map) 
							{
								fputs("op m=", f);
								fputs(options.mapfile, f);
								fputs("\n", f);
							}
                            /*if (DoscallsLIB)
                            {
                                fputs("lib doscalls.lib\n", f);
                            } else 
                            {
                                fputs("lib os2.lib\n", f);
                            }*/
                            fputs("lib api.lib\n", f);
                            if (options.DLLAPI) fputs("lib dll.lib\n", f);
                            if (options.VioAPI==1) fputs("lib vios.lib\n", f);
                            if (options.VioAPI==2) fputs("lib viof.lib\n", f);
                            if (options.MouAPI==1) fputs("lib mous.lib\n", f);
                            if (options.MouAPI==2) fputs("lib mouf.lib\n", f);
                            if (options.KbdAPI==1) fputs("lib kbds.lib\n", f);
                            if (options.KbdAPI==2) fputs("lib kbdf.lib\n", f);
                            fclose(f);
}

/*! @brief Create imptable object file */
void generate_imptable(void)
{
	FILE * f;
	BYTE hdr[0xef]={
		// THEADR
		0x80,
		0x27, 0x00,
		0x25,
		'D',':','\\','o','s','f',
		'r','e','e','\\','d','u',
		'a','l','\\','f','a','p',
		'i','\\','l','o','a','d',
		'e','r','\\','i','m','p',
		't','a','b','l','e','.','c',
		0xca,
		// COMMENT
		0x88,
		0x03, 0x00,
		// CodeView debug info
		0x80,
		0xa1,
		0x54,
		// COMMENT
		0x88,
		0x08, 0x00,
		// Watcom options
		0x80,
		0x9b,
		0x30, 0x73, 0x4F, 0x65, 0x64, 0x9A, 
		// COMMENT
		0x88, 
		0x2D, 0x00, 
		// Borland Dependency
		0x80, 
		0xE9,
		0x8C, 0x4C, 0x50, 0x59, 0x25, 
		'D', ':', '\\', 'o', 's', 'f',
		'r', 'e', 'e', '\\', 'd', 'u',
		'a', 'l', '\\', 'f', 'a', 'p', 
		'i', '\\', 'l', 'o', 'a', 'd',
		'e', 'r', '\\', 'i', 'm', 'p',
		't', 'a', 'b', 'l', 'e', '.', 'c',
		0xD2, 
		// COMMENT
		0x88, 
		0x03, 0x00, 
		// Borland Dependency
		0x80, 0xE9,
		0x0C, 
		// LNAMES
		0x96, 
		0x21, 0x00, 
		0x00, 
		0x04, 'C', 'O', 'D', 'E', 
		0x04, 'D', 'A', 'T', 'A', 
		0x03, 'B', 'S', 'S', 
		0x03, 'T', 'L', 'S', 
		0x06, 'D', 'G', 'R', 'O', 'U', 'P', 
		0x05, '_', 'T', 'E', 'X', 'T', 
		0xAB, 
		// SEGDEF
		0x98, 
		0x07, 0x00, 
		0x28, 0x00, 0x00, 0x07, 0x02, 0x01, 
		0x2F, 
		// COMMENT
		0x88,
		0x05, 0x00, 
		// Optimize far call
		0x80, 0xFE, 0x4F, 0x01, 
		0xA5, 
		// LNAMES
		0x96, 
		0x07, 0x00, 
		0x05, 'C', 'O', 'N', 'S', 'T', 
		0xD7, 
		// SEGDEF
		0x98, 
		0x07, 0x00, 
		0x48, 0x00, 0x00, 0x08, 0x03, 0x01, 
		0x0D, 
		// LNAMES
		0x96, 
		0x08, 0x00, 
		0x06, 'C', 'O', 'N', 'S', 'T', '2', 
		0xA3, 
		// SEGDEF
		0x98, 
		0x07, 0x00, 
		0x48, 0x00, 0x00, 0x09, 0x03, 0x01, 
		0x0C,
		// LNAMES
		0x96, 
		0x07, 0x00, 
		0x05, '_', 'D', 'A', 'T', 'A', 
		0xE5, 
		// SEGDEF
		0x98, 
		0x07, 0x00, 
		0x48, 0x44, 0x00, 0x0A, 0x03, 0x01, 
		0xC7, 
		// GRPDEF
		0x9A, 
		0x08, 0x00, 
		0x06, 0xFF, 0x02, 0xFF, 0x03, 0xFF, 0x04, 
		0x52
	};

	BYTE ftr[48]={
		// PUBDEF
		0x90, 
		0x10, 0x00,
		0x01, 0x04, 
		0x09, '_', 'i', 'm', 'p', 't', 'a', 'b', 'l', 'e', 
		0x00, 0x00, 0x00, 
		0xA5,
		// COMMENT
		0x88, 
		0x0A, 0x00, 
		// Default library
		0x80, 0x9F, 'm', 'a', 't', 'h', '8', '7', 's', 
		0xC3, 
		// COMMENT
		0x88, 
		0x08, 0x00,
		// Default library
		0x80, 0x9F, 'e', 'm', 'u', '8', '7', 
		0x9B, 
		// MODEND
		0x8A, 
		0x02, 0x00, 
		0x00, 0x74								  
	};

	BYTE buf[1024];

	apientry * current=apiroot;
	BYTE * curbuf=&buf[3];

    // Generate import table object file:
	
    // Open file for write
    f=fopen("tmp.obj", "wb");
    // Write header part
	fwrite(hdr, 1, sizeof(hdr), f);
	
    // Write table struct part:
	
	// Generate EXDEF object
	memset(buf, 0, sizeof(buf));
	buf[0]=0x8c;
	buf[1]=1; //size of chksum
	buf[2]=0;

		while (current)
		{
			// 1 byte len + string size bytes + 1 byte zero
			*((WORD *)&buf[1])=*((WORD *)&buf[1])+strlen(current->func)+1+1;
			*curbuf=strlen(current->func);
			curbuf++;
			strcpy(curbuf,current->func);
			curbuf=curbuf+strlen(current->func);
			*curbuf=0; // index (always 0)
			curbuf++;
			current=current->next;
		}
	    // buf[(curbuf-&buf)]=0xa4; Checksum
        fwrite(buf, 1, (curbuf-&buf)+1, f);
	
	// Generate LEDATA object (actual import table)
	memset(buf, 0, sizeof(buf));
	buf[0]=0xa0;
	buf[1]=0x4; // Checksum+segmet+offset
	buf[2]=0x00;
	buf[3]=0x04; // Segment
	buf[4]=0x00; // offset
	buf[5]=0x00;
	{
		apientry * current=apiroot;

		while (current)
		{
			// copy modname
			strcpy(&buf[*((WORD *)&buf[1])+2],current->mod);
			// copy funcname
			strcpy(&buf[*((WORD *)&buf[1])+2+9],current->func);
			// 9 modname 21 funcname 4 far pointer
			*((WORD *)&buf[1])=*((WORD *)&buf[1])+9+21+4;
			// Store offset for fixup
			current->offset=*((WORD *)&buf[1])-4-4;
			current=current->next;
		}
	    // buf[*((WORD *)&buf[1])+2]=0xe4; checksum
        fwrite(&buf, 1, *((WORD *)&buf[1])+3, f);
	}
	fseek(f, 0xde, SEEK_SET);
	{
		WORD a=*((WORD *)&buf[1])-1;
	    fwrite(&a,1, 2,f);
	}
	fseek(f, 0, SEEK_END);
	
	// Generate FIXUPP object
	memset(buf, 0, sizeof(buf));
	buf[0]=0x9c;
	buf[1]=0x1;  // Checksum
	buf[2]=0x00;
	{
		apientry * current=apiroot;
		int i=1;

		while (current)
		{
			buf[*((WORD *)&buf[1])+2]=0xCC+((current->offset & 0xFF00) >> 8);
			buf[*((WORD *)&buf[1])+2+1]=current->offset & 0xFF;
			buf[*((WORD *)&buf[1])+2+2]=0x56;
			buf[*((WORD *)&buf[1])+2+3]=i;
			i++;
			*((WORD *)&buf[1])=*((WORD *)&buf[1])+4;
			current=current->next;
		}
	    // buf[*((WORD *)&buf[1])+2]=0x80; Checksum
        fwrite(&buf, 1, *((WORD *)&buf[1])+3, f);
	}

    // Write end part
	fwrite(ftr, 1, sizeof(ftr), f);
    // Close file
	fclose(f);
}

int addtolist(char * mod, char * func)
{
	apientry * current;
	
	if (apiroot)
	{
		current=apiroot;
		
		// Search is exists
		while (current)
		{
			// Exit if found
			if ((!strcmp(current->mod, mod))&&(!strcmp(current->func, func))) return 0;
			// next entry
			if (current->next) current=current->next; else break;
		}

		current->next=malloc(sizeof(apientry));
		current=current->next;
		memset(current, 0, sizeof(apientry));
		strcpy(current->mod, mod);
		strcpy(current->func, func);
	} else {
		apiroot=malloc(sizeof(apientry));
		memset(apiroot, 0, sizeof(apientry));
		strcpy(apiroot->mod, mod);
		strcpy(apiroot->func, func);
	};
	return 0;
}

int bind(char * fname)
{
  FILE * f;
  FILE * fin;
  long end, pos;
  signed long delta;
  char buffer[1024];
  size_t bytes;
  struct exe_hdr MZHeader;
  struct new_exe NEHeader;
  int rc=1;
  struct new_seg seg;
  int i;
  char tmpdir[_MAX_PATH];
  char tmpexe[_MAX_PATH];
  
  strncpy(tmpdir, mktmpdir(tmpdir), sizeof(tmpdir));
  strncat(tmpexe, tmpdir, sizeof(tmpexe));
  strncat(tmpexe, "tmp.exe", sizeof(tmpexe));

  // Open tmp.exe for write
  if (f=fopen(tmpexe, "wb"))
  {
    // @todo check file format
    // Open fstub.exe for read
    if (fin=fopen("fstub.exe", "rb"))
    {
      // Copy fstub.exe to tmp.exe
      while (0 < (bytes = fread(buffer, 1, sizeof(buffer), fin)))
        fwrite(buffer, 1, bytes, f);

      // Close fstub.exe
      fclose(fin);

      // Remove temporary file
      remove("fstub.exe");

            // Open exe for read
            if (fin=fopen(fname, "rb"))
            {
              // Read MZ header
              if (fread(&MZHeader, 1, sizeof(struct exe_hdr), fin)==sizeof(struct exe_hdr))
              {
                // Seek to NE
                if (!fseek(fin, E_LFANEW(MZHeader), SEEK_SET))
                {
				  // Read NE
                  fread(&NEHeader, 1, sizeof(NEHeader), fin);

				  // Pad to ne_align
				  fgetpos(f, &pos);
				  memset(buffer, 0, sizeof(buffer));
				  fwrite(buffer, 1, ((pos>>NEHeader.ne_align+1)<<NEHeader.ne_align)-pos, f);

				  fgetpos(f, &end);
		          fseek(f, 0x3c, SEEK_SET);
                  // Write offset
                  fwrite(&end, 1, sizeof(DWORD), f);
                  // Seek to end of tmp.exe
                  fseek(f, end, SEEK_SET);
				  // Delta for segment start offset
                  delta=(E_LFANEW(MZHeader)>>NEHeader.ne_align)-(end>>NEHeader.ne_align);
				  //printf("%d %d %d\n", E_LFANEW(MZHeader), end, delta);
                  // Seek to start of NE
                  fseek(fin, E_LFANEW(MZHeader), SEEK_SET);

                  // Copy from NE to eof
                  while (0 < (bytes = fread(buffer, 1, sizeof(buffer), fin)))
                    fwrite(buffer, 1, bytes, f);

				  
				  // Fix segment table for new offsets
                  fseek(fin, E_LFANEW(MZHeader), SEEK_SET);
                  for (i = NE_CSEG(NEHeader); i > 0; i--)
                  {
                    // Seek segment table entry
                    fseek(fin, E_LFANEW(MZHeader)+NE_SEGTAB(NEHeader)+(NE_CSEG(NEHeader)-i)*sizeof(struct new_seg), SEEK_SET);
                    // Read segment table entry
                    fread(&seg, 1, sizeof(struct new_seg), fin);
                    //printf("%d\n", seg.ns_sector);
					if (seg.ns_sector)
					{
					  // Set new segment offset
					  seg.ns_sector=seg.ns_sector-delta;

                      // Seek segment table entry
                      fseek(f, end+NE_SEGTAB(NEHeader)+(NE_CSEG(NEHeader)-i)*sizeof(struct new_seg), SEEK_SET);
                      // Write segment table entry
                      fwrite(&seg, 1, sizeof(struct new_seg), f);
					}
				  }
				  
				  // Fix non-resident name table offset
				  fseek(f, end+0x2c, SEEK_SET);
				  delta=NEHeader.ne_nrestab-(E_LFANEW(MZHeader)-end);
				  fwrite(&delta, 1, 4, f);
			  
                  // Close exe
                  fclose(fin);
                  // Close tmp
                  fclose(f);
                  // Delete exe 
				  // @todo: if infile and outfile differs, then don't delete infile
                  if (!remove(options.outfile))
                  {
                    // Rename tmp.exe to exe
                    if (!rename(tmpexe, options.outfile))
                    {
                      // Success
	                  rc=0;
                    } else {
                      printf( "Error: File rename\n" );
                    }
                  } else {
                    printf( "Error: Delete file\n" );
                  }
                } else {
                  printf( "Error: Seek input file\n" );
                }
              } else {
                printf( "Error: Read input file\n" );
              }
            } else {
              printf( "Error: Open input file\n" );
            }
    } else {
      printf( "Error: Open output file\n" );
    }
  } else {
    printf( "Error: Open input file\n" );
  }
  return rc;
}

char * findfunctionname(char * module, WORD ordinal, char * lib)
{
  FILE * f;
  omf_record_header head;
  int result;
  char buf[512];
  int i;
  long pos;
  char mod[255];
  WORD ord;
  
  // Open os2.lib/doscalls.lib for read
  if(f=fopen(lib, "rb"))
  {
    for (;1;)
    {
      if ( (result = fread(&head, 1, sizeof(head), f)) == sizeof(head) )
      {
        //printf ("type=%02x length=%d\n", head.type, head.length);
        fread(&buf, 1, head.length, f);

        if (head.type==0x88) // comment
        {
          if (buf[1]==0xa0) // comment class
          {
            if (buf[2]==0x01) // impdef
            {
              if (buf[3]!=0)
              {
                memcpy(&func, &buf[5], buf[4]);   // function name
                func[buf[4]]=0;
                memcpy(&mod, &buf[5+buf[4]+1], buf[5+buf[4]]); // module name
                mod[buf[5+buf[4]]]=0;
                ord=buf[5+buf[4]+1+buf[5+buf[4]]]+0xff*buf[5+buf[4]+1+buf[5+buf[4]]+1];                       // ordinal
                if ((!strcmp(mod, module)) && (ord==ordinal)) 
                {
                  fclose(f);

                  return func;//printf("%s %s %d\n", func, mod, ord);
                }
              } else {
                printf("panic!\n");
              }
            }
          }
        }

        if (head.type==0xf1) break; // End of lib

        if (head.type==0x8a) // end of obj
        {
          fgetpos(f, &pos);
          if ((16-pos%16)!=16) fread(&buf, 1, 16-pos%16, f);
        }
      }
    }
    fclose(f);
  }
  return NULL;
}

/*! @brief Search library in lib paths */
/* First search in current directory. If not found, then search
 * in /LIBPATH paths. If not found, then search in LIB paths. */
int searchlib(char * libname, char * fullpath)
{
  _searchenv(libname, ".", fullpath);
  if(fullpath[0] == '\0')
  {
	_searchenv(libname, options.libpath, fullpath);
	if(fullpath[0] == '\0')
	{
	  _searchenv(libname, "LIB", fullpath);
	}
  }
  return (fullpath[0] == '\0')?0:1;
}

/*! @brief Check required environment to run */
int check_environment(void)
{
  char full_path[ _MAX_PATH ];

  // Check presense of required files
  // wlink.exe, doscalls.lib/os2.lib, api.lib etc.

  // Check presense of WATCOM environment variable
  if (!getenv("WATCOM"))
  {
	  printf( "Error: WATCOM environment variable not set\n" );
	  return 1;
  }

  // Check presense of wlink.lnk in path
  _searchenv( "wlink.lnk", "PATH", full_path );
  if(full_path[0] == '\0') 
  {
	  printf( "Error: Unable to find wlink.lnk file\n" );
	  return 1;
  }

  // Check wlink.exe
  _searchenv( "wlink.exe", "PATH", full_path );
  if(full_path[0] == '\0') 
  {
	  printf( "Error: Unable to find wlink.exe file\n" );
	  return 1;
  }

  // Check os2.lib
  if(!searchlib( "os2.lib", full_path))
  {
    // Try doscalls.lib instead
    if (searchlib( "doscalls.lib", full_path)) options.DoscallsLIB=1;
  }

  if (full_path[0] == '\0')
  {
      printf( "Error: Unable to find nor os2.lib nor doscalls.lib file\n" );
	  return 1;
  }

  // Check api.lib
  if(!searchlib( "api.lib", full_path ))
  {
	  printf( "Error: Unable to find api.lib file\n" );
	  return 1;
  }

  // Check presense of subsystem  required files
  // dll.lib, vios.lib, viof.lib, mous.lib, mouf.lib, kbds.lib, kbdf.lib
    if (!searchlib( "dll.lib", full_path ))
    {
      printf( "Error: Unable to find dll.lib file\n" );
      return 1;
    }
  
    if (!searchlib( "vios.lib", full_path ))
    {
      printf( "Error: Unable to find vios.lib file\n" );
      return 1;
    }
  
    if (!searchlib( "viof.lib", full_path ))
    {
      printf( "Error: Unable to find viof.lib file\n" );
      return 1;
    }
  
    if (!searchlib( "mous.lib", full_path ))
    {
      printf( "Error: Unable to find mous.lib file\n" );
      return 1;
    }
  
    if (!searchlib( "mouf.lib", full_path ))
    {
      printf( "Error: Unable to find mouf.lib file\n" );
      return 1;
    }
  
    if (!searchlib( "kbds.lib", full_path ))
    {
      printf( "Error: Unable to find kbds.lib file\n" );
      return 1;
    }
  
    if (!searchlib( "kbdf.lib", full_path ))
    {
      printf( "Error: Unable to find kbdf.lib file\n" );
      return 1;
    }

    if (!searchlib( "apilmr.obj", full_path ))
    {
      printf( "Error: Unable to find apilmr.obj file\n" );
      return 1;
    }

	return 0;
}

/*! @brief Output usage help information */
void printhlp(void)
{
  printf("BIND infile [implibs] [linklibs] [options]\n\n");

  printf("/HELP       Displays Help\n");
  printf("/?          Displays Help\n\n");
  printf("/L[IBPATH]  Add to library search path\n");
  printf("/M[AP]      Generates Link Map File\n");
  printf("/N[AMES]    Specifies Protected-Mode Functions\n");
  printf("/NOLOGO     Suppresses Sign-On Banner\n");
  printf("/O[UTFILE]  Specifies Name of Bound Program\n");
  printf("/Q          Quiet\n");
}

int main(int argc, char *argv[])
{
  FILE * f;
  struct exe_hdr MZHeader;
  struct new_exe NEHeader;
  int result;
  int i;
  char ** mods = NULL;
  char full_path[ _MAX_PATH ];
  WORD offset;
  BYTE len;
  WORD count;
  struct new_seg seg;
  int j;
  struct new_rlc rlc;
  int rc=1; // Error exit code by default
  signed char ch;

	// Configure initial options
	options.quiet=0;
	options.logo=1;
	options.outfile[0]=0;;
	options.infile[0]=0;;
	options.mapfile[0]=0;;
	options.libpath[0]=0;;
	options.map=0;
	options.dosformat=0;
	options.DoscallsLIB=0;
	options.MouAPI = 0;
	options.KbdAPI = 0;
	options.VioAPI = 0;
	options.DLLAPI = 0;


  if (check_environment())
  {
	  printf("Environment not configured");
	  return 1;
  };

    // no args - print usage and exit
    if (argc == 1)
    {
        printhlp();
        exit(1);
    }

#ifndef __UNIX__
    if ((*argv[1] != '-') && (*argv[1] != '/')) // first arg prefix - or / ?
#else
    if (*argv[1] != '-') // first arg prefix -  ?
#endif
    {
		optind++;
        strncpy(options.infile, argv[optind], sizeof(options.infile)-1);
		options.dosformat=1;
    }

    // Get program arguments using getopt()
    while ((ch = getopt(argc, argv, "?h:H:m:M:n:N:q:Q:o:O")) != -1)
    {
        switch (ch)
        {
        case 'l':
        case 'L':
            if (!strnicmp(optarg, "IBPATH", 6))
            {
				strcpy(options.libpath, argv[optind]);
            } else
			{
				strcpy(options.libpath, optarg);
			}
            break;

        case 'm':
        case 'M':
		    // todo: default mapfile using input filename
			// todo: default extenwion is a BM
			// todo: check is it really filename or next option
            if (!strnicmp(optarg, "AP", 2))
            {
				strcpy(options.mapfile, argv[optind]);
            } else
			{
				strcpy(options.mapfile, optarg);
			}
            options.map=1;
            break;

        case 'n':
        case 'N':
            if (!strncmp(_strupr(optarg), "OLOGO", 5))
            {
                options.logo=0;
                break;
            }

			// todo: add NAMES support and /NAMES @list support
            if (!strncmp(_strupr(optarg), "AMES", 4))
            {
                printf("NAMES\n");
                break;
            }
			exit(1);


        case 'q':
        case 'Q':
            if (!strncmp(_strupr(optarg), "UIET", 4))
            {
                options.quiet=1;
                break;
            }
            if (!strlen(optarg))
			{
				options.quiet=1;
                break;
			}
            exit(1);

        case 'o':
        case 'O':
            if (!strnicmp(optarg, "UTFILE", 6))
            {
				strcpy(options.outfile, argv[optind]);
            } else
			{
				strcpy(options.outfile, optarg);
			}
            break;

        case 'h':
        case 'H':
            if (!strncmp(_strupr(optarg), "ELP", 3))
            {
                printhlp();
                exit(0);
            }

        case '?':
            printhlp();
            exit(0);

        default:
            printf("Unknown option\n");
            exit(1);
        }
    }

  if (options.logo&&!options.quiet) printf("osFree FamilyAPI Binder v.0.9\n\n");

    // check for input file - getopt compatable cmd line
    // we either have in/out files or it is error
    if ((argc == optind) && !options.dosformat)
	{
        printf("BIND: no input file\n");
		exit(1);
	}

    // if dosformat is false then using new format
    // so we need to get input file and maybe the output file
    if (!options.dosformat)
    {
        strncpy(options.infile, argv[optind], sizeof(options.infile)-1);
        if (!strlen(options.outfile)) strncpy(options.outfile, argv[optind], sizeof(options.outfile)-1);
        optind++;
	}
  
        // Open exe for read
        if(f=fopen(options.infile, "rb"))
        {
          // Read old Executable header
          if (fread(&MZHeader, 1, sizeof(MZHeader), f) == sizeof(MZHeader))
          {
            // Check MZ Header magic
            if (E_MAGIC(MZHeader) == EMAGIC)
            {
              // Seek New Executable header
              if (!fseek(f, E_LFANEW(MZHeader), SEEK_SET))
              {
                // Read New Executable header
                if (fread(&NEHeader, 1, sizeof(NEHeader), f) == sizeof(NEHeader))
                {
                  // Check NE Header magic
                  if (NE_MAGIC(NEHeader) == NEMAGIC)
                  {
                    /* check for OS/2 program */
                    if (NE_EXETYP(NEHeader) == NE_OS2)
                    {
                      // seek to Module table
                      if (!fseek(f, E_LFANEW(MZHeader)+NE_MODTAB(NEHeader), SEEK_SET))
                      {
                        //Allocate memory for mod table
                        if (mods=(char**)malloc(NE_CMOD(NEHeader)*sizeof(char*)))
                        {
                          // Read mod table
                          for (i=NE_CMOD(NEHeader); i >0 ; i-- )
                          {
                            // Seek to module table
                            if (!fseek(f, E_LFANEW(MZHeader)+NE_MODTAB(NEHeader)+2*(NE_CMOD(NEHeader)-i), SEEK_SET))
                            {
                              // Read offset of module name
                              if (fread(&offset, 1, sizeof(WORD), f)==sizeof(WORD))
                              {
                                // Seek to module name
                                if (!fseek(f, E_LFANEW(MZHeader)+NE_IMPTAB(NEHeader)+offset, SEEK_SET))
                                {
                                  // Read module name length
                                  if (fread(&len, 1, sizeof(BYTE), f)==sizeof(BYTE))
                                  {
                                    // Allocate memory for module name
                                    if (mods[NE_CMOD(NEHeader)-i]=malloc(len+1))
                                    {
                                      // Read module name
                                      if (fread(mods[NE_CMOD(NEHeader)-i], 1, len, f)==len)
                                      {
                                        // Convert to ASCIIZ
                                        mods[NE_CMOD(NEHeader)-i][len]=0;
                                        //printf("%d %s\n", i, mods[NE_CMOD(NEHeader)-i]);
                                      } else {
                                        printf( "Error: Read name\n" );
                                        return 1;
                                      }
                                    } else {
                                      printf( "Error: Allocate name memory\n" );
                                      return 1;
                                    }
                                  } else {
                                    printf( "Error: Read name length\n" );
                                    return 1;
                                  }
                                } else {
                                  printf( "Error: Seek to name table\n" );
                                  return 1;
                                }
                              } else {
                                printf( "Error: Read module name offset\n" );
                                return 1;
                              }
                            } else {
                              printf( "Error: Seek to module table\n" );
                              return 1;
                            }
                          }
	  

                          // Now read segments fixup tables and build list of imported functions
                          for (i = NE_CSEG(NEHeader); i > 0; i--)
                          {
                            // Seek segment table entry
                            if (!fseek(f, E_LFANEW(MZHeader)+NE_SEGTAB(NEHeader)+(NE_CSEG(NEHeader)-i)*sizeof(struct new_seg), SEEK_SET))
                            {
                              // Read segment table entry
                              if (fread(&seg, 1, sizeof(struct new_seg), f)==sizeof(struct new_seg))
                              {
                                // Seek relocation table
                                if (!fseek(f, (seg.ns_sector<<NEHeader.ne_align)+(seg.ns_cbseg?seg.ns_cbseg:0x10000), SEEK_SET))
                                {
                                  // Read relocation table size
                                  if (fread(&count, 1, sizeof(WORD), f)==sizeof(WORD))
                                  {
                                    //printf("%d pos=%d %d\n", i, (seg.ns_sector<<NEHeader.ne_align)+(seg.ns_cbseg?seg.ns_cbseg:0x10000), count);
                                    for (j = 0; j < count; j++)
                                    {
                                      // Read relocation table entry
                                      if (fread(&rlc, 1, sizeof(struct new_rlc), f)==sizeof(struct new_rlc))
                                      {
                                        if (((rlc.nr_flags & NRRTYP)==NRRORD) || ((rlc.nr_flags & NRRTYP)==NRRNAM))
                                        {
                                          if ((rlc.nr_flags & NRRTYP)==NRRNAM)
                                          {
                                            // not supported yet...
                                            printf("Panic!\n");
                                            return 1;
                                          } else {
                                            char * fname;
	  								  
                                            fname=findfunctionname(mods[rlc.nr_union.nr_import.nr_mod-1],rlc.nr_union.nr_import.nr_proc, options.DoscallsLIB?"doscalls.lib":"os2.lib");
                                            //printf("%s.%d %s\n", mods[rlc.nr_union.nr_import.nr_mod-1], rlc.nr_union.nr_import.nr_proc, fname);
                                            
                                            // Collect in list
											addtolist(mods[rlc.nr_union.nr_import.nr_mod-1], fname);
	  								  
                                            // If any Mou* used, then turn on Mou API
                                            if (!strncmp(fname, "MOU",3)) options.MouAPI=1;
                                            
                                            // If any Kbd* used, then turn on Kbd API
                                            if (!strncmp(fname, "KBD",3)) options.KbdAPI=1;
	  								  
                                            // If any Vio* used, then turn on Vio API
                                            if (!strncmp(fname, "VIO",3)) options.VioAPI=1;
	  								  
                                            // If VioRegister used, then turn full VIO API and DLL API
                                            if (!strncmp(fname, "VIOREGISTER",12))
                                            {
                                              options.VioAPI=2;
                                              options.DLLAPI=1;
                                            }
                                            // If MouRegister used, then turn full Mou API and DLL API
                                            if (!strncmp(fname, "MOUREGISTER",12))
                                            {
                                              options.MouAPI=2;
                                              options.DLLAPI=1;
                                            }
                                            // If KbdRegister used, then turn full Kbd API and DLL API
                                            if (!strncmp(fname, "KBDREGISTER",12))
                                            {
                                              options.KbdAPI=2;
                                              options.DLLAPI=1;
                                            }
                                            // If DosLoadModule used, then turn on DLL API
                                            if (!strncmp(fname, "DOSLOADMODULE",14)) options.DLLAPI=1;
                                          }
                                        }       
                                      } else 
                                      {
                                        printf( "Error: Read relocation table entry\n" );
                                        return 1;
                                      }
                                    }
                                  } else 
                                  {
                                    printf( "Error: Read relocation table size\n" );
                                    return 1;
                                  }
                                } else 
                                {
                                  printf( "Error: Seek relocation table\n" );
                                  return 1;
                                }
                              } else 
                            {
                                printf( "Error: Read segment table entry\n" );
                                return 1;
                              }
                            } else 
                            {
                              printf( "Error: Seek segment table entry\n" );
                              return 1;
                            }
                          }
        

                          if (!fclose(f))
                          {
							  // Generate temporary imptable.obj
							  generate_imptable();
							  // Generate LNK file
							  generate_lnk();
							  // Call linker
                              system("wlink.exe op q op fullh @bind.lnk");

                              // remove temporary files
                              remove("bind.lnk");
							  //return 0;
                              remove("tmp.obj");

                              // Change standard DOS stub to FamilyAPI stub:
						      rc=bind(options.infile);

                              // Exit
                          } else {
                            printf("Error: Close file\n");
                          }
                          free(mods);
                        } else {
                          printf( "Error: Memory allocate\n");
                        }
                      } else {
                        printf( "Error: Seek module reference table\n");
                      }
                    } else {
                      printf( "Error: Target OS not OS/2\n");
                    }
                  } else {
                    printf( "Error: Bad NE header\n");
                  }
                } else {
                  printf( "Error: Read NE Header\n");
                }
              } else {
                printf( "Error: Seek to NE header\n");
              }
            } else {
              printf( "Error: Bad MZ header\n");
            }
          } else {
            printf( "Error: Read MZ Header\n");
          }
	  	fclose(f);
        } else {
          printf( "Error: File open\n");
        }
  return rc;  // Exit with error code
}
