/*
 *  FamilyAPI Binder
 *
 *  Copyright 2024 Yuri Prokushev
 *
 *  (c) osFree Project 2024, <https://www.osfree.org>
 *  for licence see licence.txt in root directory, or project website
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
                // panic!
              }
            }
          }
        }

        if (head.type==0xf1) break; // End of lib

        if (head.type==0x8a) // end of obj
        {
          fgetpos(f, &pos);
//          printf("%d %d\n", pos, 512-pos%512);
          fread(&buf, 1, 512-pos%512, f);
        }
      }
    }
    fclose(f);
  }
  return NULL;
}

int main(int argc, char *argv[])
{
  FILE * f;
  struct exe_hdr MZHeader;
  struct new_exe NEHeader;
  int result;
  int i;
  int MouAPI = 0;
  int KbdAPI = 0;
  int VioAPI = 0;
  int DLLAPI = 0;
  int DoscallsLIB = 0;
  char ** mods = NULL;
  char full_path[ _MAX_PATH ];
  WORD offset;
  BYTE len;
  WORD count;
  struct new_seg seg;
  int j;
  struct new_rlc rlc;
  FILE * fin;
  DWORD end;
  char buffer[1024*1024];
  size_t bytes;

  printf("osFree FamilyAPI Binder v.0.1\n\n");

  printf("BIND infile [implibs] [linklibs] [options]\n\n");

  printf("/HELP       Displays Help\n");
  printf("/M[AP]      Generates Link Map File\n");
  printf("/N[AMES]    Specifies Protected-Mode Functions\n");
  printf("/NOLOGO     Suppresses Sign-On Banner\n");
  printf("/O[UTFILE]  Specifies Name of Bound Program\n");
  printf("/?          Displays Short Help\n\n");
  
  // Check presense of base required files
  // wlink.exe, doscalls.lib/os2.lib, api.lib

  // Check wlink.exe
  _searchenv( "wlink.exe", "PATH", full_path );
  if(!full_path[0] == '\0') 
  {
    // @todo search using WATCOM env
    // Check os2.lib
    _searchenv( "os2.lib", "LIB", full_path );
    if(full_path[0] == '\0')
    {
      // Try doscalls.lib instead
      _searchenv( "doscalls.lib", "LIB", full_path );
      if(!full_path[0] == '\0')
      {
        DoscallsLIB=1;
      } else {
        printf( "Error: Unable to find nor os2.lib nor doscalls.lib file\n" );
        return 1;
      }
    }

    // Check api.lib
    _searchenv( "api.lib", "LIB", full_path );
    if(!full_path[0] == '\0') 
    {
      // Open exe for read
      if(f=fopen("attrib.exe", "rb"))
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
									  
                                          fname=findfunctionname(mods[rlc.nr_union.nr_import.nr_mod-1],rlc.nr_union.nr_import.nr_proc, DoscallsLIB?"doscalls.lib":"os2.lib");
                                          printf("%s.%d %s\n", mods[rlc.nr_union.nr_import.nr_mod-1], rlc.nr_union.nr_import.nr_proc, fname);
                                          
                                          // Collect in list
									  
                                          // If any Mou* used, then turn on Mou API
                                          if (!strncmp(fname, "MOU",3)) MouAPI=1;
                                          
                                          // If any Kbd* used, then turn on Kbd API
                                          if (!strncmp(fname, "KBD",3)) KbdAPI=1;
									  
                                          // If any Vio* used, then turn on Vio API
                                          if (!strncmp(fname, "VIO",3)) VioAPI=1;
									  
                                          // If VioRegister used, then turn full VIO API and DLL API
                                          if (!strncmp(fname, "VIOREGISTER",12))
                                          {
                                            VioAPI=2;
                                            DLLAPI=1;
                                          }
                                          // If MouRegister used, then turn full Mou API and DLL API
                                          if (!strncmp(fname, "MOUREGISTER",12))
                                          {
                                            MouAPI=2;
                                            DLLAPI=1;
                                          }
                                          // If KbdRegister used, then turn full Kbd API and DLL API
                                          if (!strncmp(fname, "KBDREGISTER",12))
                                          {
                                            KbdAPI=2;
                                            DLLAPI=1;
                                          }
                                          // If DosLoadModule used, then turn on DLL API
                                          if (!strncmp(fname, "DOSLOADMODULE",14)) DLLAPI=1;
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
                          // Check presense of subsystem  required files
                          // dll.lib, vios.lib, viof.lib, mous.lib, mouf.lib, kbds.lib, kbdf.lib
                          if (DLLAPI)
                          {
                            _searchenv( "dll.lib", "LIB", full_path );
                            if( full_path[0] == '\0' ) 
                            {
                              printf( "Error: Unable to find dll.lib file\n" );
                              return 1;
                            }
                          }
                    
                          if (VioAPI==1)
                          {
                            _searchenv( "vios.lib", "LIB", full_path );
                            if( full_path[0] == '\0' ) 
                            {
                              printf( "Error: Unable to find vios.lib file\n" );
                              return 1;
                            }
                          }
                      
                          if (VioAPI==2)
                          {
                            _searchenv( "viof.lib", "LIB", full_path );
                            if( full_path[0] == '\0' ) 
                            {
                              printf( "Error: Unable to find viof.lib file\n" );
                              return 1;
                            }
                          }
                      
                          if (MouAPI==1)
                          {
                            _searchenv( "mous.lib", "LIB", full_path );
                            if( full_path[0] == '\0' ) 
                            {
                              printf( "Error: Unable to find mous.lib file\n" );
                              return 1;
                            }
                          }
                      
                          if (MouAPI==2)
                          {
                            _searchenv( "mouf.lib", "LIB", full_path );
                            if( full_path[0] == '\0' ) 
                            {
                              printf( "Error: Unable to find mouf.lib file\n" );
                              return 1;
                            }
                          }
                      
                          if (KbdAPI==1)
                          {
                            _searchenv( "kbds.lib", "LIB", full_path );
                            if( full_path[0] == '\0' ) 
                            {
                              printf( "Error: Unable to find kbds.lib file\n" );
                              return 1;
                            }
                          }
                      
                          if (KbdAPI==2)
                          {
                            _searchenv( "kbdf.lib", "LIB", full_path );
                            if( full_path[0] == '\0' ) 
                            {
                              printf( "Error: Unable to find kbdf.lib file\n" );
                              return 1;
                            }
                          }

                          // Generate import table object file:
						  // Open file for write
						  // Write header part
						  // Write table struct part
						  // Write end part
						  // Close file
                          
                        
                          // Generate link file
                          f=fopen("bind.lnk", "w");
                          fputs("system dos\n",f);
                          fputs("name fstub.exe\n" , f); 
                          fputs("file tmp.obj\n", f);
						  if (DoscallsLIB)
						  {
                            fputs("lib doscalls.lib\n", f);
						  } else 
						  {
                            fputs("lib os2.lib\n", f);
						  }
                          fputs("lib api.lib\n", f);
                          if (DLLAPI) fputs("lib dll.lib\n", f);
                          if (VioAPI==1) fputs("lib vios.lib\n", f);
                          if (VioAPI==2) fputs("lib viof.lib\n", f);
                          if (MouAPI==1) fputs("lib mous.lib\n", f);
                          if (MouAPI==2) fputs("lib mouf.lib\n", f);
                          if (KbdAPI==1) fputs("lib kbds.lib\n", f);
                          if (KbdAPI==2) fputs("lib kbdf.lib\n", f);
                          fclose(f);
                        
                          // Call linker
                          system("wlink.exe op q @bind.lnk");
                        
                          // Change standard DOS stub to FamilyAPI stub:

						  // Open tmp.exe for write
                          f=fopen("tmp.exe", "wb");
						  // @todo check file format
						  // Open fstub.exe for read
                          fin=fopen("fstub.exe", "rb");
						  // Copy fstub.exe to tmp.exe
                          while (0 < (bytes = fread(buffer, 1, sizeof(buffer), fin)))
                            fwrite(buffer, 1, bytes, f);
						  
						  // get file end address
                          end = ftell(fin);
						  
                          // Seek to lfa_new
						  fseek(f, 0x2c, SEEK_SET);
						  // Write offset
						  fwrite(&end, 1, sizeof(DWORD), f);
						  // Seek to end of tmp.exe
						  fseek(f, 0, SEEK_END);
						  // Close fstub.exe
						  fclose(fin);
						  // Open exe for read
                          fin=fopen("attrib.exe", "rb");
						  // Read MZ header
						  fread(&MZHeader, 1, sizeof(struct exe_hdr), fin);
						  // Seek to NE
						  fseek(fin, E_LFANEW(MZHeader), SEEK_SET);
						  // Copy from NE to eof
                          while (0 < (bytes = fread(buffer, 1, sizeof(buffer), fin)))
                            fwrite(buffer, 1, bytes, f);
						  // Close exe
						  fclose(fin);
						  // Close tmp
						  fclose(f);
						  // Delete exe
						  //remove("attrib.exe");
						  // Rename tmp.exe to exe
						  rename("tmp.exe", "attrib2.exe");
                        
                          // Remove temporary files
						  //remove("bind.lnk");
						  //remove("fstub.exe");
						  //remove("tmp.obj");
						  
                          return 0; // Exit with success code
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
    } else {
      printf( "Error: Unable to find api.lib file\n" );
    }
  } else {
    printf( "Error: Unable to find wlink.exe file\n" );
  }
  return 1;  // Exit with error code
}
