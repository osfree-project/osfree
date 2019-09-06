/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992  Anders Christensen <anders@pvv.unit.no>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/* huups, have to add one to length in everyting given to Str_ncatstr */

#include "rexx.h"
#include "strings.h"

#include <assert.h>
#include <stdio.h>

#include <descrip.h>
#include <rmsdef.h>
#include <ssdef.h>
#include <dvidef.h>
#include <jpidef.h>
#include <quidef.h>
#include <syidef.h>
#include <uicdef.h>
#include <libdtdef.h>
#include <jbcmsgdef.h>
#include <lnmdef.h>
#include <psldef.h>
#include <libdef.h>
#include <libdtdef.h>

#include <fab.h>
#include <nam.h>
#include <xab.h>

#define MAX_PATH_LEN 64

#define HEX_DIGIT(a) (((a)<10)?((a)+'0'):((a)-10+'A'))
#define ADD_CHAR(a,b) (a)->value[(a)->len++] = (b)
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))


typedef struct cli_block {
   short length ;
   char type ;
   char subtype ;
   short flags ;
   short TRO_cnt ;
} cli_block ;

typedef struct com_block {
   unsigned char handler ;
   unsigned char sizes ;
   unsigned char verbtype ;
   unsigned char pad ;
   short name ;
   short image ;
   short outputs ;
   short prefix ;
} com_block ;

struct fabptr {
   struct fabptr *next ;
   int num ;
   streng *name ;
   struct FAB *box ;
} ;

/* f$cvsi()      */
/* f$cvtime()    */
/* f$cvui()      */

/*
 * Values to be returned by the ACPTYPE item of sys$getdvi ... return
 * value is an index into this list of strings. Yes ... I know, this
 * is not the way to do it, but I don't know of any structure in the
 * library that hold this information. Whenever porting Regina to a new
 * version of VMS, make sure that this info is correct (check macros in
 * the file dvidef.h).
 */

#define NUM_ACP_CODES ((sizeof(acp_codes)/sizeof(char*))-1)
static const char *acp_codes[] = {
   "ILLEGAL", "F11CV1", "F11V2", "MTA", "NET", "REM"
} ;

#define NUM_JPI_MODES ((sizeof(jpi_modes)/sizeof(char*))-1)
static const char *jpi_modes[] = {
   "OTHER", "NETWORK", "BATCH", "INTERACTIVE"
} ;


#define NUM_SCH_TYPES ((sizeof(sch_types)/sizeof(char*))-1)
static const char *sch_types[] = {
   "UNKNOWN", "CEF", "COM", "COMO", "CUR", "COLPG", "FPG", "HIB", "HIBO",
   "LEF", "LEFO", "MWAIT", "PFW", "SUSP", "SUSPO"
} ;

typedef struct { /* vms_tsf: static variables of this module (thread-safe) */
   char *                  error_buffer;
   struct dsc$descriptor_s error_descr;
   int                     pid;
   struct fabptr *         fabptrs[16];
} vmf_tsd_t; /* thread-specific but only needed by this module. see
              * init_vmf
              */


/* init_vmf initializes the module.
 * Currently, we set up the thread specific data.
 * The function returns 1 on success, 0 if memory is short.
 */
int init_vmf( tsd_t *TSD )
{
   vmf_tsd_t *vt;

   if (TSD->vmf_tsd != NULL)
      return(1);

   if ((vt = TSD->vmf_tsd = MallocTSD(sizeof(vmf_tsd_t))) == NULL)
      return(0);
   memset(vt,0,sizeof(vmf_tsd_t));  /* correct for all values */
   return(1);
}

static char *select_code( const int code, const char *values[], const int max )
{
   return (char *)values[((code<1)||(code>max)) ? 0 : code] ;
}

static const char *all_privs[] = {
   "CMKRNL",   "CMEXEC",  "SYSNAM",   "GRPNAM",    "ALLSPOOL", "DETACH",
   "DIAGNOSE", "LOG_IO",  "GROUP",    "ACNT",      "PRMCEB",   "PRMMBX",
   "PSWAPM",   "SETPRI",  "SETPRV",   "TMPMBX",    "WORLD",    "MOUNT",
   "OPER",     "EXQUOTA", "NETMBX",   "VOLPRO",    "PHY_IO",   "BUGCHK",
   "PRMGBL",   "SYSGBL",  "PFNMAP",   "SHMEM",     "SYSPRV",   "BYPASS",
   "SYSLCK",   "SHARE",   "UPGRADE",  "DOWNGRADE", "GRPPRV",   "READALL",
   "",         "",        "SECURITY", ""
} ;

#define NUM_PRIVS ((sizeof(all_privs)/sizeof(char*)))


static void vms_error( const tsd_t *TSD, const int err )
{
   unsigned short length ;
   unsigned int rc ;
   vmf_tsd_t *vt;

   vt = TSD->vmf_tsd;
   if (!vt->error_buffer)
   {
      vt->error_descr.dsc$a_pointer = vt->error_buffer = MallocTSD( 256+1 ) ;
      vt->error_descr.dsc$w_length = 256 ;
      vt->error_descr.dsc$b_dtype = DSC$K_DTYPE_T ;
      vt->error_descr.dsc$b_class = DSC$K_CLASS_S ;
   }

   rc=sys$getmsg( err, &length, &vt->error_descr, NULL, NULL ) ;
   if (rc != SS$_NORMAL)
      exiterror( ERR_SYSTEM_FAILURE , 0 ) ;

   vt->error_buffer[length] = 0x00 ;
   printf( "\n" ) ;
   fprintf( stderr, "%s\n", vt->error_buffer ) ;
}


static streng *internal_id( const tsd_t *TSD, const short *id )
{
   streng *result ;

   result = Str_makeTSD( 20 ) ;
   sprintf( result->value, "(%d,%d,%d)", id[0], id[1], id[2] ) ;
   result->len = strlen( result->value ) ;
   return( result ) ;
}

static int name_to_num( const tsd_t *TSD, const streng *name )
{
   int id, rc ;
   $DESCRIPTOR( descr, "" ) ;

   descr.dsc$w_length = name->len ;
   descr.dsc$a_pointer = (char *)name->value ;
   rc = sys$asctoid( &descr, &id, NULL ) ;
   if (rc == SS$_NOSUCHID || rc == SS$_IVIDENT)
      return 0 ;
   if (rc != SS$_NORMAL)
      vms_error( TSD, rc ) ;

   return (id) ;
}


static streng *num_to_name( const tsd_t *TSD, const int num )
{
   char user[256], group[256] ;
   $DESCRIPTOR( udescr, user ) ;
   $DESCRIPTOR( gdescr, group ) ;
   streng *result ;
   short length, glength ;
   int rc, xnum, context, theid ;

   if (num == 0)
      return NULL ;

   if (!(num & 0x80000000))
   {
      xnum = num | 0x0000ffff ;
      rc = sys$idtoasc( xnum, &glength, &gdescr, NULL, NULL, NULL) ;
      if (rc == SS$_NOSUCHID)
         glength = -1 ;
      else if (rc != SS$_NORMAL)
      {
         vms_error( TSD, rc ) ;
         glength = -1 ;
      }
   }
   else
      glength = -1 ;

   rc = sys$idtoasc( num, &length, &udescr, NULL, NULL, NULL ) ;

   if (rc == SS$_NOSUCHID)
      return NULL ;
   if (rc != SS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      length = 0 ;
   }

   if (glength > -1)
   {
      result = Str_makeTSD( glength + 1 + length ) ;
      Str_ncatstrTSD( result, group, glength ) ;
      result->value[result->len++] = ',' ;
   }
   else
      result = Str_makeTSD( length ) ;

   Str_ncatstrTSD( result, user, length ) ;
   return result ;
}


static streng *get_prot( const tsd_t *TSD, int prot )
{
   char *names[] = { "SYSTEM", "OWNER", "GROUP", "WORLD" } ;
   int i ;
   streng *result ;

   result = Str_makeTSD( 50 ) ;
   for (i=0; i<4; i++)
   {
      Str_catstrTSD( result, names[i] ) ;
      if ((prot & 0x0f) != 0x0f)
      {
         /* DCL-bug: says RWED, should say RWLP */
         ADD_CHAR(result, '=') ;
         if (!(prot & 0x01)) ADD_CHAR(result, 'R') ;
         if (!(prot & 0x02)) ADD_CHAR(result, 'W') ;
         if (!(prot & 0x04)) ADD_CHAR(result, 'E') ; /* actually L */
         if (!(prot & 0x08)) ADD_CHAR(result, 'D') ; /* actually P */
      }
      ADD_CHAR( result, ',' ) ;
      ADD_CHAR( result, ' ' ) ;
      prot = prot >> 4 ;
   }
   result->len -= 2 ;
   return result ;
}

static streng *get_uic( const tsd_t *TSD, const union uicdef *uic )
{
   streng *name ;
   streng *result ;

   result = Str_makeTSD( 14 ) ;
   name = num_to_name( TSD, uic->uic$l_uic ) ;
   if (name)
   {
      ADD_CHAR( result, '[' ) ;
      Str_catTSD( result, name ) ;
      ADD_CHAR( result, ']' ) ;
   }
   else
   {
      sprintf(result->value,"[%o,%o]", uic->uic$v_group, uic->uic$v_member) ;
      result->len = strlen( result->value ) ;
   }
   return result ;
}


struct dvi_items_type {
   int type ;     /* Datatype returned from item, see DVI_ macros above */
   char *name ;   /* Parameter that identifies a particular vitem */
   int addr ;   /* Item identifyer to give to sys$getdvi */
} ;


struct item_list {
   union {
      struct {
         short code ;
         short length ;
      } norm ;
      int terminator ; } frst ;
   char *buffer ;
   int *length ;
} ;

/*
 * Here comes the code to implement the SYS$GETDVI() system service,
 * which is largely the same as the F$GETDVI() lexical function. There
 * are some minor differences, though.
 */

#define TYP_HEX    1    /* 4 byte unsigned hex integer */
#define TYP_ACP    2    /* ACP type code, or 'ILLEGAL' */
#define TYP_BOOL   3    /* 4 byte boolean integer */
#define TYP_LSTR   4    /* 64 byte character string */
#define TYP_4STR   5    /* 4 byte character string */
#define TYP_VEC    6    /* 4 byte unsigned integer */
#define TYP_INT    7    /* 4 byte signed integer */
#define TYP_UIC    8    /* 4 byte user identification code */
#define TYP_SSTR   9    /* 12 byte character string */
#define TYP_PROT  10    /* 4 byte protection mask */
#define TYP_LHEX  11    /* 64 byte binary string, interpreted as hex */
#define TYP_PRIV  12
#define TYP_TIME  13
#define TYP_MODE  14
#define TYP_SCHT  15
#define TYP_DTIM  16
#define TYP_MSTR  17
#define TYP_FLAG  18
#define TYP_TRNM  19
#define TYP_BSTR  20  /* Binary string, don't strip away ASCII zeros */

#define TYP_EXST   1 + 128  /* DVI$_EXISTS */
#define TYP_SPLD   2 + 128  /* force primary characteristics DVI$_DEVNAM */

#define TYP_SPECIFICS 1024
#define TYP_FLF      1024
#define TYP_FLS  1 + 1024
#define TYP_FMF  2 + 1024
#define TYP_JBF  3 + 1024
#define TYP_JBS  4 + 1024
#define TYP_PJR  5 + 1024
#define TYP_QUF  6 + 1024
#define TYP_QUS  7 + 1024

static streng *format_result( const tsd_t *TSD, const int type, const char *buffer, int length )
{
   streng *result ;
   int *iptr = (int *)&(buffer[0]) ;
   int i ;

   switch (type)
   {
      case TYP_INT:
      case TYP_VEC:
         result = Str_makeTSD( 12 ) ;
/*       sprintf( result->value, ((type==TYP_INT) ? "%d" : "%u"), *iptr ) ; */
         sprintf( result->value, "%d", *iptr ) ;  /* DCL-bug */
         result->len = strlen( result->value ) ;
         assert( result->len < result->max ) ;
         break ;

      case TYP_LHEX:
      {
         int i ;

         result = Str_makeTSD( length * 2 ) ;
         for (i=0; i<length; i++)
        {
            result->value[i*2] = HEX_DIGIT((buffer[length-i-1] >> 4) & 0x0f);
            result->value[i*2+1] = HEX_DIGIT(buffer[length-i-1] & 0x0f) ;
         }
         result->len = length * 2 ;
         break ;
      }

      case TYP_DTIM:
      {
         int timer = *((int*)buffer) ;
         int days, hour, min, sec, hund ;
         result = Str_makeTSD( 17 ) ;

         hund = timer % 100 ; timer /= 100 ;
         sec = timer % 60 ; timer /= 60 ;
         min = timer % 60 ; timer /= 60 ;
         hour = timer % 24 ;
         days = timer / 24 ;

         result->len = 16 ;
         sprintf( result->value, "%4d %02d:%02d:%02d.%02d",
                                         days, hour, min, sec, hund ) ;

         break ;
      }

      case TYP_TIME:
      {
         int length, rc ;
         $DESCRIPTOR( desc, "" ) ;
         result = Str_makeTSD( 50 ) ;
         desc.dsc$a_pointer = result->value ;
         desc.dsc$w_length = 50 ;

         rc = lib$format_date_time( &desc, buffer, NULL, &length, NULL ) ;
         if (rc != SS$_NORMAL)
            vms_error( TSD, rc ) ;

         result->len = length ;
         break ;
      }

      case TYP_HEX:
         if (*iptr)
         {
            result = Str_makeTSD( 9 ) ;
            sprintf( result->value, "%08X", *iptr ) ;
            result->len = strlen( result->value ) ;
         }
         else
            result = nullstringptr() ;

         assert( result->len < result->max ) ;
         break ;

      case TYP_ACP:
         result = Str_creTSD(select_code( *iptr, acp_codes, NUM_ACP_CODES )) ;
         break ;

      case TYP_SCHT:
         result = Str_creTSD(select_code( *iptr, sch_types, NUM_SCH_TYPES )) ;
         break ;

      case TYP_MODE:
         result = Str_creTSD(select_code( *iptr, jpi_modes, NUM_JPI_MODES )) ;
         break ;

      case TYP_PRIV:
      {
         result = Str_makeTSD(256) ;
         for (i=0; i<NUM_PRIVS; i++)
            if (buffer[i/8] & (1<<(i%8)))
            {
               Str_catstrTSD( result, all_privs[i] ) ;
               ADD_CHAR( result, ',' ) ;
            }
         if (result->len)
            result->len-- ;
         break ;
      }

      case TYP_BOOL:
         result = Str_creTSD( (*iptr) ? "TRUE" : "FALSE" ) ;
         break ;

      case TYP_LSTR:
      case TYP_SSTR:
      case TYP_4STR:
      case TYP_MSTR:
         for (;length && buffer[length-1]==0x00; length--) ;
      case TYP_BSTR:
      {
         result = Str_ncreTSD( buffer, length ) ;
         break ;
      }

      case TYP_UIC:
      {
         result = get_uic( TSD, ( union uicdef *)buffer ) ;
         break ;
      }

      case TYP_PROT:
      {
         result = get_prot( TSD, *iptr ) ;
         break ;
      }

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;

   }
   return result ;
}



#define DVI_XXX    1    /* Must be treated as a special case */
#define DVI_INT    2    /* 4 byte integer */
#define DVI_BOOL   3    /* 4 byte boolean */
#define DVI_LSTR   4    /* 64 byte ASCII string */
#define DVI_SSTR   5    /* 12 byte ASCII string */
#define DVI_4STR   6    /* 4 byte ASCII string */
#define DVI_HEX    7    /* 4 byte hexadecimal number */
#define DVI_ACP    8    /* 4 byte integer index into list */
#define DVI_UIC    9    /* 4 byte VMS UIC */
#define DVI_VEC   10    /* 4 byte vector of bitvalues */
#define DVI_PROT  11    /* 4 byte protection mask */
#define DVI_PRIV  12    /* 64 bit privilege mask */
#define DVI_MODE  13    /* index into list of jpi_mode_list */
#define DVI_TIME  14    /* 64 bit absolute time */
#define DVI_STR   15
#define DVI_PID   16
#define DVI_HTYP  17

#define DVI_BIN DVI_HTYP    /* More or less the same */

#define DVI_XXX_EXISTS 1


static streng *strip_nulls( streng *input )
{
   int i ;

   for (i=input->len-1; (i>=0) && isspace(input->value[i]); i--) ;
   input->len = i+1 ;
   return input ;
}


#define HEXDIG(x) ((isdigit(x))?((x)-'0'):(toupper(x)-'A'+10))

static unsigned int read_pid( const streng *hexpid )
{
   int i ;
   unsigned int sum=0 ;

   for (i=0; i<hexpid->len; i++)
      if (isxdigit(hexpid->value[i]))
         sum = sum*16 + HEXDIG( hexpid->value[i] ) ;

   return sum ;
}


streng *vms_f_directory( tsd_t *TSD, cparamboxptr parms )
{
   char buffer[ MAX_PATH_LEN ] ;
   short length ;
   int rc ;
   streng *result ;
   $DESCRIPTOR( dir, buffer ) ;

   checkparam( parms, 0, 0, "VMS_F_DIRECTORY" ) ;

   rc = sys$setddir( NULL, &length, &dir ) ;
   if (rc != RMS$_NORMAL)
      vms_error( TSD, rc ) ;

   if (length > MAX_PATH_LEN)
      exiterror( ERR_SYSTEM_FAILURE , 0 ) ;

   result = Str_makeTSD( length ) ;
   result = Str_ncatstrTSD( result, buffer, length ) ;
   return (result) ;
}


/* f$edit() */
/* f$element() */
/* f$environment()   --- not sure how to handle this */
/* f$extract() */
/* f$fao() */

/*
streng *vms_f_file_attributes( tsd_t *TSD, cparamboxptr parms )
{
   checkparam( parms, 2, 2, "VMS_F_FILE_ATTRIBUTES" ) ;
}
*/

struct dvi_items_type dvi_items[] =
{
   { TYP_HEX,   "ACPPID",         DVI$_ACPPID          },
   { TYP_ACP,   "ACPTYPE",        DVI$_ACPTYPE         },
   { TYP_BOOL,  "ALL",            DVI$_ALL             },
   { TYP_LSTR,  "ALLDEVNAM",      DVI$_ALLDEVNAM       },
   { TYP_INT,   "ALLOCLASS",      DVI$_ALLOCLASS       },
   { TYP_BOOL,  "ALT_HOST_AVAIL", DVI$_ALT_HOST_AVAIL  },
   { TYP_LSTR,  "ALT_HOST_NAME",  DVI$_ALT_HOST_NAME   },
   { TYP_4STR,  "ALT_HOST_TYPE",  DVI$_ALT_HOST_TYPE   },
   { TYP_BOOL,  "AVL",            DVI$_AVL             },
   { TYP_BOOL,  "CCL",            DVI$_CCL             },
   { TYP_INT,   "CLUSTER",        DVI$_CLUSTER         },
   { TYP_BOOL,  "CONCEALED",      DVI$_CONCEALED       }, /* undoc'ed */
   { TYP_INT,   "CYLINDERS",      DVI$_CYLINDERS       },
   { TYP_INT,   "DEVBUFSIZ",      DVI$_DEVBUFSIZ       },
   { TYP_VEC,   "DEVCHAR",        DVI$_DEVCHAR         },
   { TYP_VEC,   "DEVCHAR2",       DVI$_DEVCHAR2        },
   { TYP_INT,   "DEVCLASS",       DVI$_DEVCLASS        },
   { TYP_VEC,   "DEVDEPEND",      DVI$_DEVDEPEND       },
   { TYP_VEC,   "DEVDEPEND2",     DVI$_DEVDEPEND2      },
   { TYP_LHEX,  "DEVLOCKNAM",     DVI$_DEVLOCKNAM      },
   { TYP_LSTR,  "DEVNAM",         DVI$_DEVNAM          },
   { TYP_VEC,   "DEVSTS",         DVI$_DEVSTS          },
   { TYP_INT,   "DEVTYPE",        DVI$_DEVTYPE         },
   { TYP_BOOL,  "DIR",            DVI$_DIR             },
 /*  DVI$_DISPLAY_DEVNAM refered to in SS, not in LexFuncs */
   { TYP_BOOL,  "DMT",            DVI$_DMT             },
   { TYP_BOOL,  "DUA",            DVI$_DUA             },
   { TYP_BOOL,  "ELG",            DVI$_ELG             },
   { TYP_INT,   "ERRCNT",         DVI$_ERRCNT          },
   { TYP_EXST,  "EXISTS",         DVI$_DIR             },
   { TYP_BOOL,  "FOD",            DVI$_FOD             },
   { TYP_BOOL,  "FOR",            DVI$_FOR             },
   { TYP_INT,   "FREEBLOCKS",     DVI$_FREEBLOCKS      },
   { TYP_LSTR,  "FULLDEVNAM",     DVI$_FULLDEVNAM      },
   { TYP_BOOL,  "GEN",            DVI$_GEN             },
   { TYP_BOOL,  "HOST_AVAIL",     DVI$_HOST_AVAIL      },
   { TYP_INT,   "HOST_COUNT",     DVI$_HOST_COUNT      },
   { TYP_LSTR,  "HOST_NAME",      DVI$_HOST_NAME       },
   { TYP_4STR,  "HOST_TYPE",      DVI$_HOST_TYPE       },
   { TYP_BOOL,  "IDV",            DVI$_IDV             },
   { TYP_HEX,   "LOCKID",         DVI$_LOCKID          },
   { TYP_LSTR,  "LOGVOLNAM",      DVI$_LOGVOLNAM       },
   { TYP_INT,   "MAXBLOCK",       DVI$_MAXBLOCK        },
   { TYP_INT,   "MAXFILES",       DVI$_MAXFILES        },
   { TYP_BOOL,  "MBX",            DVI$_MBX             },
   { TYP_VEC,   "MEDIA_ID",       DVI$_MEDIA_ID        },
   { TYP_LSTR , "MEDIA_NAME",     DVI$_MEDIA_NAME      },
   { TYP_LSTR , "MEDIA_TYPE",     DVI$_MEDIA_TYPE      },
   { TYP_BOOL,  "MNT",            DVI$_MNT             },
   { TYP_INT,   "MOUNTCNT",       DVI$_MOUNTCNT        },
 /*  DVI$_MSCP_UNIT_NUMBER refered to in SS, not in LexFuncs */
   { TYP_BOOL,  "NET",            DVI$_NET             },
   { TYP_LSTR , "NEXTDEVNAM",     DVI$_NEXTDEVNAM      },
   { TYP_BOOL,  "ODV",            DVI$_ODV             },
   { TYP_INT,   "OPCNT",          DVI$_OPCNT           },
   { TYP_BOOL,  "OPR",            DVI$_OPR             },
   { TYP_UIC,   "OWNUIC",         DVI$_OWNUIC          },
   { TYP_HEX,   "PID",            DVI$_PID             },
   { TYP_BOOL,  "RCK",            DVI$_RCK             },
   { TYP_BOOL,  "RCT",            DVI$_RCT             },
   { TYP_BOOL,  "REC",            DVI$_REC             },
   { TYP_INT,   "RECSIZ",         DVI$_RECSIZ          },
   { TYP_INT,   "REFCNT",         DVI$_REFCNT          },
   { TYP_BOOL,  "REMOTE_DEVICE",  DVI$_REMOTE_DEVICE   },
   { TYP_BOOL,  "RND",            DVI$_RND             },
   { TYP_LSTR,  "ROOTDEVNAM",     DVI$_ROOTDEVNAM      },
   { TYP_BOOL,  "RTM",            DVI$_RTM             },
   { TYP_BOOL,  "SDI",            DVI$_SDI             },
   { TYP_INT,   "SECTORS",        DVI$_SECTORS         },
   { TYP_VEC,   "SERIALNUM",      DVI$_SERIALNUM       },
   { TYP_BOOL,  "SERVED_DEVICE",  DVI$_SERVED_DEVICE   },
   { TYP_BOOL,  "SHR",            DVI$_SHR             },
   { TYP_BOOL,  "SPL",            DVI$_SPL             },
   { TYP_SPLD,  "SPLDEVNAM",      DVI$_DEVNAM          },
   { TYP_BOOL,  "SQD",            DVI$_SQD             },
   { TYP_VEC,   "STS",            DVI$_STS             },
   { TYP_BOOL,  "SWL",            DVI$_SWL             },
   { TYP_INT,   "TRACKS",         DVI$_TRACKS          },
   { TYP_INT,   "TRANSCNT",       DVI$_TRANSCNT        },
   { TYP_BOOL,  "TRM",            DVI$_TRM             },
   { TYP_LSTR,  "TT_ACCPORNAM",   DVI$_TT_ACCPORNAM    },
   { TYP_BOOL,  "TT_ALTYPEAHD",   DVI$_TT_ALTYPEAHD    },
   { TYP_BOOL,  "TT_ANSICRT",     DVI$_TT_ANSICRT      },
   { TYP_BOOL,  "TT_APP_KEYPAD",  DVI$_TT_APP_KEYPAD   },
   { TYP_BOOL,  "TT_AUTOBAUD",    DVI$_TT_AUTOBAUD     },
   { TYP_BOOL,  "TT_AVO",         DVI$_TT_AVO          },
   { TYP_BOOL,  "TT_BLOCK",       DVI$_TT_BLOCK        },
   { TYP_BOOL,  "TT_BRDCSTMBX",   DVI$_TT_BRDCSTMBX    },
   { TYP_BOOL,  "TT_CRFILL",      DVI$_TT_CRFILL       },
   { TYP_BOOL,  "TT_DECCRT",      DVI$_TT_DECCRT       },
   { TYP_BOOL,  "TT_DECCRT2",     DVI$_TT_DECCRT2      },
   { TYP_BOOL,  "TT_DIALUP",      DVI$_TT_DIALUP       },
   { TYP_BOOL,  "TT_DISCONNECT",  DVI$_TT_DISCONNECT   },
   { TYP_BOOL,  "TT_DMA",         DVI$_TT_DMA          },
   { TYP_BOOL,  "TT_DRCS",        DVI$_TT_DRCS         },
   { TYP_BOOL,  "TT_EDIT",        DVI$_TT_EDIT         },
   { TYP_BOOL,  "TT_EDITING",     DVI$_TT_EDITING      },
   { TYP_BOOL,  "TT_EIGHTBIT",    DVI$_TT_EIGHTBIT     },
   { TYP_BOOL,  "TT_ESCAPE",      DVI$_TT_ESCAPE       },
   { TYP_BOOL,  "TT_FALLBACK",    DVI$_TT_FALLBACK     },
   { TYP_BOOL,  "TT_HALFDUP",     DVI$_TT_HALFDUP      },
   { TYP_BOOL,  "TT_HANGUP",      DVI$_TT_HANGUP       },
   { TYP_BOOL,  "TT_HOSTSYNC",    DVI$_TT_HOSTSYNC     },
   { TYP_BOOL,  "TT_INSERT",      DVI$_TT_INSERT       },
   { TYP_BOOL,  "TT_LFFILL",      DVI$_TT_LFFILL       },
   { TYP_BOOL,  "TT_LOCALECHO",   DVI$_TT_LOCALECHO    },
   { TYP_BOOL,  "TT_LOWER",       DVI$_TT_LOWER        },
   { TYP_BOOL,  "TT_MBXDSABL",    DVI$_TT_MBXDSABL     },
   { TYP_BOOL,  "TT_MECHFORM",    DVI$_TT_MECHFORM     },
   { TYP_BOOL,  "TT_MECHTAB",     DVI$_TT_MECHTAB      },
   { TYP_BOOL,  "TT_MODEM",       DVI$_TT_MODEM        },
   { TYP_BOOL,  "TT_MODHANGUP",   DVI$_TT_MODHANGUP    },
   { TYP_BOOL,  "TT_NOBRDCST",    DVI$_TT_NOBRDCST     },
   { TYP_BOOL,  "TT_NOECHO",      DVI$_TT_NOECHO       },
   { TYP_BOOL,  "TT_NOTYPEAHD",   DVI$_TT_NOTYPEAHD    },
   { TYP_BOOL,  "TT_OPER",        DVI$_TT_OPER         },
   { TYP_INT,   "TT_PAGE",        DVI$_TT_PAGE         },
   { TYP_BOOL,  "TT_PASTHRU",     DVI$_TT_PASTHRU      },
   { TYP_LSTR,  "TT_PHYDEVNAM",   DVI$_TT_PHYDEVNAM    },
   { TYP_BOOL,  "TT_PRINTER",     DVI$_TT_PRINTER      },
   { TYP_BOOL,  "TT_READSYNC",    DVI$_TT_READSYNC     },
   { TYP_BOOL,  "TT_REGIS",       DVI$_TT_REGIS        },
   { TYP_BOOL,  "TT_REMOTE",      DVI$_TT_REMOTE       },
   { TYP_BOOL,  "TT_SCOPE",       DVI$_TT_SCOPE        },
   { TYP_BOOL,  "TT_SECURE",      DVI$_TT_SECURE       },
   { TYP_BOOL,  "TT_SETSPEED",    DVI$_TT_SETSPEED     },
   { TYP_BOOL,  "TT_SIXEL",       DVI$_TT_SIXEL        },
   { TYP_BOOL,  "TT_SYSPWD",      DVI$_TT_SYSPWD       },
   { TYP_BOOL,  "TT_TTSYNC",      DVI$_TT_TTSYNC       },
   { TYP_BOOL,  "TT_WRAP",        DVI$_TT_WRAP         },
   { TYP_INT,   "UNIT",           DVI$_UNIT            },
   { TYP_INT,   "VOLCOUNT",       DVI$_VOLCOUNT        },
   { TYP_SSTR,  "VOLNAM",         DVI$_VOLNAM          },
   { TYP_INT,   "VOLNUMBER",      DVI$_VOLNUMBER       },
   { TYP_BOOL,  "VOLSETMEM",      DVI$_VOLSETMEM       },
   { TYP_PROT,  "VPROT",          DVI$_VPROT           },
   { TYP_BOOL,  "WCK",            DVI$_WCK             },
} ;



static struct dvi_items_type *item_info(
     const streng *name, const struct dvi_items_type *xlist, int size )
{
   int top, bot, mid, tmp ;
   const char *poss, *cptr ;

   top = size / sizeof( struct dvi_items_type ) - 1 ;
   bot = 0 ;

   for ( ; bot<=top; )
   {
      mid = (top+bot)/2 ;

      cptr = name->value ;
      poss = (const char *) xlist[mid].name ;
      for (tmp=name->len; tmp--; )
         if (toupper(*(cptr++))!=(*(poss++))) break ;

      if (tmp==(-1))
         tmp = - *poss ;
      else
         tmp = toupper(*(cptr-1)) - *(poss-1) ;

      if (tmp<0)
         top = mid - 1 ;
      else if (tmp)
         bot = mid + 1 ;
      else
         return (struct dvi_items_type *)&(xlist[mid]) ;
   }
   return NULL ;
}


/*
 * Why do I use sys$getdviw() instead of lib$getdvi ... because Digital
 * fucked up the implementation of lib$getdvi(). Problem: When secondary
 * characteristics are chosen (1 is added to item-code, ... or item-code
 * is or'ed with DVI$C_SECONDARY), lib$getdvi interprets the result as
 * numeric in all cases, and never do any dataconversion, except from
 * converting everything to decimal integer. Ergo, lib$getdvi is utterly
 * useless for obtaining non-numeric info about secondary devices.
 * When (if!) they fix it, undefining the LIB$GETDVI_BUG should make the
 * code far simpler, and also much more compatible.
 */


streng *vms_f_getdvi( tsd_t *TSD, cparamboxptr parms )
{
   char *buffer="", buffer1[64], buffer2[64] ;
   int spooled, slength=4, rc, itemcode, length ;
   short length1, length2 ;
   struct dvi_items_type *ptr ;
   int item[12], type ;
   struct dsc$descriptor_s name ;
   struct dsc$descriptor_s dir = {
       sizeof(buffer)-1, DSC$K_DTYPE_T, DSC$K_CLASS_S, buffer } ;

   checkparam( parms, 2, 2, "VMS_F_GETDVI" ) ;

   ptr = item_info( parms->next->value, dvi_items, sizeof( dvi_items)) ;
   if (!ptr)
      exiterror( ERR_INCORRECT_CALL , 0 ) ;

   name.dsc$w_length = Str_len( parms->value ) ;
   name.dsc$b_dtype = DSC$K_DTYPE_T ;
   name.dsc$b_class = DSC$K_CLASS_S ;
   name.dsc$a_pointer = parms->value->value ;

   item[0] = 64 + ((ptr->addr) << 16) ;
   item[1] = (int)buffer1 ;
   item[2] = (int)&length1 ;
   item[3] = 64 + ((ptr->addr | DVI$C_SECONDARY) << 16) ;
   item[4] = (int)buffer2 ;
   item[5] = (int)&length2 ;
   item[6] = 4 + ((DVI$_SPL | DVI$C_SECONDARY) << 16) ;
   item[7] = (int)&spooled ;
   item[8] = (int)&slength ;
   item[9] = item[10] = item[11] = 0 ;


   rc = sys$getdviw( NULL, NULL, &name, &item, NULL, NULL, NULL, NULL ) ;

   if (ptr->type == TYP_SPLD)
   {
      spooled = 0 ;
      type = TYP_LSTR ;
   }
   else
      type = ptr->type ;

   buffer = (spooled) ? buffer2 : buffer1 ;
   length = (spooled) ? length2 : length1 ;

   if (type == TYP_EXST)
   {
      if (rc == SS$_NOSUCHDEV) return Str_creTSD( "FALSE" ) ;
      if (rc == SS$_NORMAL) return Str_creTSD( "TRUE" ) ;
   }

   if (rc != SS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return Str_creTSD("") ;
   }

   return format_result( TSD, type, buffer, length ) ;
}


static const struct dvi_items_type jpi_items[] =
{
   { TYP_LSTR,  "ACCOUNT",      JPI$_ACCOUNT    },
   { TYP_INT,   "APTCNT",       JPI$_APTCNT     },
   { TYP_INT,   "ASTACT",       JPI$_ASTACT     },
   { TYP_INT,   "ASTCNT",       JPI$_ASTCNT     },
   { TYP_INT,   "ASTEN",        JPI$_ASTEN      },
   { TYP_INT,   "ASTLM",        JPI$_ASTLM      },
   { TYP_INT,   "AUTHPRI",      JPI$_AUTHPRI    },
   { TYP_PRIV,  "AUTHPRIV",     JPI$_AUTHPRIV   },
   { TYP_INT,   "BIOCNT",       JPI$_BIOCNT     },
   { TYP_INT,   "BIOLM",        JPI$_BIOLM      },
   { TYP_INT,   "BUFIO",        JPI$_BUFIO      },
   { TYP_INT,   "BYTCNT",       JPI$_BYTCNT     },
   { TYP_INT,   "BYTLM",        JPI$_BYTLM      },
   { TYP_LSTR,  "CLINAME",      JPI$_CLINAME    },
   { TYP_INT,   "CPULIM",       JPI$_CPULIM     },
   { TYP_INT,   "CPUTIM",       JPI$_CPUTIM     },
   { TYP_PRIV,  "CURPRIV",      JPI$_CURPRIV    },
   { TYP_INT,   "DFPFC",        JPI$_DFPFC      },
   { TYP_INT,   "DFWSCNT",      JPI$_DFWSCNT    },
   { TYP_INT,   "DIOCNT",       JPI$_DIOCNT     },
   { TYP_INT,   "DIOLM",        JPI$_DIOLM      },
   { TYP_INT,   "DIRIO",        JPI$_DIRIO      },
   { TYP_INT,   "EFCS",         JPI$_EFCS       },
   { TYP_INT,   "EFCU",         JPI$_EFCU       },
   { TYP_INT,   "EFWM",         JPI$_EFWM       },
   { TYP_INT,   "ENQCNT",       JPI$_ENQCNT     },
   { TYP_INT,   "ENQLM",        JPI$_ENQLM      },
   { TYP_HEX,   "EXCVEC",       JPI$_EXCVEC     },
   { TYP_INT,   "FILCNT",       JPI$_FILCNT     },
   { TYP_INT,   "FILLM",        JPI$_FILLM      },
   { TYP_HEX,   "FINALEXC",     JPI$_FINALEXC   },
   { TYP_HEX,   "FREP0VA",      JPI$_FREP0VA    },
   { TYP_HEX,   "FREP1VA",      JPI$_FREP1VA    },
   { TYP_INT,   "FREPTECNT",    JPI$_FREPTECNT  },
   { TYP_INT,   "GPGCNT",       JPI$_GPGCNT     },
   { TYP_INT,   "GRP",          JPI$_GRP        },
   { TYP_INT,   "IMAGECOUNT",   JPI$_IMAGECOUNT },
   { TYP_LSTR,  "IMAGNAME",     JPI$_IMAGNAME   },
   { TYP_PRIV,  "IMAGPRIV",     JPI$_IMAGPRIV   },
   { TYP_INT,   "JOBPRCCNT",    JPI$_JOBPRCCNT  },
   { TYP_TIME,  "LOGINTIM",     JPI$_LOGINTIM   },
   { TYP_HEX,   "MASTER_PID",   JPI$_MASTER_PID },
   { TYP_INT,   "MEM",          JPI$_MEM        },
   { TYP_MODE,  "MODE",         JPI$_MODE       },
   { TYP_INT,   "MSGMASK",      JPI$_MSGMASK    },
   { TYP_HEX,   "OWNER",        JPI$_OWNER      },
   { TYP_INT,   "PAGEFLTS",     JPI$_PAGEFLTS   },
   { TYP_INT,   "PAGFILCNT",    JPI$_PAGFILCNT  },
   { TYP_HEX,   "PAGFILLOC",    JPI$_PAGFILLOC  },
   { TYP_INT,   "PGFLQUOTA",    JPI$_PGFLQUOTA  },
   { TYP_VEC,   "PHDFLAGS",     JPI$_PHDFLAGS   },
   { TYP_HEX,   "PID",          JPI$_PID        },
   { TYP_INT,   "PPGCNT",       JPI$_PPGCNT     },
   { TYP_INT,   "PRCCNT",       JPI$_PRCCNT     },
   { TYP_INT,   "PRCLM",        JPI$_PRCLM      },
   { TYP_LSTR,  "PRCNAM",       JPI$_PRCNAM     },
   { TYP_INT,   "PRIB",         JPI$_PRIB       },
   { TYP_PRIV,  "PROCPRIV",     JPI$_PROCPRIV   },
   { TYP_INT,   "SITESPEC",     JPI$_SITESPEC   },
   { TYP_SCHT,  "STATE",        JPI$_STATE      },
   { TYP_INT,   "STS",          JPI$_STS        },
   { TYP_HEX,   "SWPFILLOC",    JPI$_SWPFILLOC  },
   { TYP_SSTR,  "TERMINAL",     JPI$_TERMINAL   },
   { TYP_INT,   "TMBU",         JPI$_TMBU       },
   { TYP_INT,   "TQCNT",        JPI$_TQCNT      },
   { TYP_INT,   "TQLM",         JPI$_TQLM       },
   { TYP_UIC,   "UIC",          JPI$_UIC        },
   { TYP_SSTR,  "USERNAME",     JPI$_USERNAME   },
   { TYP_INT,   "VIRTPEAK",     JPI$_VIRTPEAK   },
   { TYP_INT,   "VOLUMES",      JPI$_VOLUMES    },
   { TYP_INT,   "WSAUTH",       JPI$_WSAUTH     },
   { TYP_INT,   "WSAUTHEXT",    JPI$_WSAUTHEXT  },
   { TYP_INT,   "WSEXTENT",     JPI$_WSEXTENT   },
   { TYP_INT,   "WSPEAK",       JPI$_WSPEAK     },
   { TYP_INT,   "WSQUOTA",      JPI$_WSQUOTA    },
   { TYP_INT,   "WSSIZE",       JPI$_WSSIZE     },
} ;


streng *vms_f_getjpi( tsd_t *TSD, cparamboxptr parms )
{
   char buffer[64] ;
   int item[6] ;
   short length=0 ;
   int rc, pid, *pidaddr ;
   struct dvi_items_type *ptr ;
   struct dsc$descriptor_s dir = {
       sizeof(buffer)-1, DSC$K_DTYPE_T, DSC$K_CLASS_S, buffer } ;

   checkparam( parms, 2, 2, "VMS_F_GETJPI" ) ;

   ptr = item_info( parms->next->value, jpi_items, sizeof(jpi_items)) ;
   if (!ptr)
      exiterror( ERR_INCORRECT_CALL , 0 ) ;

   if ((!parms->value) || (!parms->value->len))
      pidaddr = NULL ;
   else
   {
      pid = read_pid( parms->value ) ;
      pidaddr = &pid ;
   }

   item[0] = 64 + ( ptr->addr << 16 ) ;
   item[1] = (int)buffer ;
   item[2] = (int)&length ;
   item[3] = item[4] = item[5] = 0 ;

   rc = sys$getjpiw( NULL, pidaddr, NULL, &item, NULL, NULL, NULL ) ;

   if (rc != SS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return Str_creTSD("") ;
   }

   return format_result( TSD, ptr->type, buffer, length ) ;
}

/*
 * Warning, the sequence of these records *must* match the macros
 * given below (CARAC, ENTRY, ENTRY), which is used in initializing
 * the array leg_items
 */
static const struct dvi_items_type qui_funcs[] = {
   { 0x70, "CANCEL_OPERATION",       QUI$_CANCEL_OPERATION },
   { 0x02, "DISPLAY_CHARACTERISTIC", QUI$_DISPLAY_CHARACTERISTIC },
   { 0x00, "DISPLAY_ENTRY",          QUI$_DISPLAY_ENTRY },
   { 0x20, "DISPLAY_FILE",           QUI$_DISPLAY_FILE },
   { 0x02, "DISPLAY_FORM",           QUI$_DISPLAY_FORM },
   { 0x20, "DISPLAY_JOB",            QUI$_DISPLAY_JOB },
   { 0x02, "DISPLAY_QUEUE",          QUI$_DISPLAY_QUEUE },
   { 0x42, "TRANSLATE_QUEUE",        QUI$_TRANSLATE_QUEUE },
} ;

/*
static const char char qui_chars[] = {
   0x70, 0x02, 0x00, 0x20, 0x02, 0x20, 0x02, 0x42
} ;

static const int qui_func_codes[] = {
   QUI$_CANCEL_OPERATION, QUI$_DISPLAY_CHARACTERISTIC, QUI$_DISPLAY_ENTRY,
   QUI$_DISPLAY_FILE, QUI$_DISPLAY_FORM, QUI$_DISPLAY_JOB,
   QUI$_DISPLAY_QUEUE, QUI$_TRANSLATE_QUEUE
} ;
*/

static const int qui_spec_values[] = {
   QUI$_FILE_FLAGS, QUI$_FILE_STATUS, QUI$_FORM_FLAGS, QUI$_JOB_FLAGS,
   QUI$_JOB_STATUS, QUI$_PENDING_JOB_REASON, QUI$_QUEUE_FLAGS,
   QUI$_QUEUE_STATUS
} ;

#define CHARAC  0x02  /* display_characteristics */
#define ENTRY   0x04  /* diskplay_entry */
#define FILE    0x08  /* display_file */
#define FORM    0x10  /* display_form */
#define JOB     0x20  /* display_job */
#define QUEUE   0x40  /* display_queue */
#define TRANS   0x80  /* translate_queue */

static const char leg_items[] = {
                  ENTRY + JOB, /* ACCOUNT_NAME */
                  ENTRY + JOB, /* AFTER_TIME */
                        QUEUE, /* ASSIGNED_QUEUE_NAME */
                        QUEUE, /* BASE_PRIORITY */
          ENTRY + JOB + QUEUE, /* CHARACTERISTICS */
                       CHARAC, /* CHARACTERISTIC_NAME */
                       CHARAC, /* CHARACTERISTIC_NUMBER */
                  ENTRY + JOB, /* CHECKPOINT_DATA */
                  ENTRY + JOB, /* CLI */
                  ENTRY + JOB, /* COMPLETE_BLOCKS */
                  ENTRY + JOB, /* CONDITION_VECTOR */
                        QUEUE, /* CPU_DEFAULT */
          ENTRY + JOB + QUEUE, /* CPU_LIMIT */
                        QUEUE, /* DEFAULT_FORM_NAME */
                        QUEUE, /* DEFAULT_FORM_STOCK */
                        QUEUE, /* DEVICE_NAME */
                  ENTRY + JOB, /* ENTRY_NUMBER */
                        QUEUE, /* EXECUTING_JOB_COUNT */
                         FILE, /* FILE_BURST */
                         FILE, /* FILE_CHECKPOINTED */
                         FILE, /* FILE_COPIES */
                         FILE, /* FILE_COPIES_DONE */
                         FILE, /* FILE_DELETE */
                         FILE, /* FILE_DOUBLE_SPACE */
                         FILE, /* FILE_EXECUTING */
                         FILE, /* FILE_FLAG */
                         FILE, /* FILE_FLAGS */
                         FILE, /* FILE_IDENTIFICATION */
                         FILE, /* FILE_PAGE_HEADER */
                         FILE, /* FILE_PAGINATE */
                         FILE, /* FILE_PASSALL */
                         FILE, /* FILE_SETUP_MODULES */
                         FILE, /* FILE_SPECIFICATION */
                         FILE, /* FILE_STATUS */
                         FILE, /* FILE_TRAILER */
                         FILE, /* FIRST_PAGE */
                         FORM, /* FORM_DESCRIPTION */
                         FORM, /* FORM_FLAGS */
                         FORM, /* FORM_LENGTH */
                         FORM, /* FORM_MARGIN_BOTTOM */
                         FORM, /* FORM_MARGIN_LEFT */
                         FORM, /* FORM_MARGIN_RIGHT */
                         FORM, /* FORM_MARGIN_TOP */
   FORM + ENTRY + JOB + QUEUE, /* FORM_NAME */
                         FORM, /* FORM_NUMBER */
                         FORM, /* FORM_SETUP_MODULES */
                         FORM, /* FORM_SHEET_FEED */
   FORM + ENTRY + JOB + QUEUE, /* FORM_STOCK */
                         FORM, /* FORM_TRUNCATE */
                         FORM, /* FORM_WIDTH */
                         FORM, /* FORM_WRAP */
                        QUEUE, /* GENERIC_TARGET */
                        QUEUE, /* HOLDING_JOB_COUNT */
                  ENTRY + JOB, /* INTERVENING_BLOCKS */
            /* see comment in vms.rexx about intervening jobs */
            /* ENTRY + */ JOB, /* INTERVENING_JOBS */
                  ENTRY + JOB, /* JOB_ABORTING */
                  ENTRY + JOB, /* JOB_COPIES */
                  ENTRY + JOB, /* JOB_COPIES_DONE */
                  ENTRY + JOB, /* JOB_CPU_LIMIT */
                  ENTRY + JOB, /* JOB_EXECUTING */
                  ENTRY + JOB, /* JOB_FILE_BURST */
                  ENTRY + JOB, /* JOB_FILE_BURST_ONE */
                  ENTRY + JOB, /* JOB_FILE_FLAG */
                  ENTRY + JOB, /* JOB_FILE_FLAG_ONE */
                  ENTRY + JOB, /* JOB_FILE_PAGINATE */
                  ENTRY + JOB, /* JOB_FILE_TRAILER */
                  ENTRY + JOB, /* JOB_FILE_TRAILER_ONE */
                  ENTRY + JOB, /* JOB_FLAGS */
                  ENTRY + JOB, /* JOB_HOLDING */
                  ENTRY + JOB, /* JOB_INACCESSIBLE */
                        QUEUE, /* JOB_LIMIT */
                  ENTRY + JOB, /* JOB_LOG_DELETE */
                  ENTRY + JOB, /* JOB_LOG_NULL */
                  ENTRY + JOB, /* JOB_LOG_SPOOL */
                  ENTRY + JOB, /* JOB_LOWERCASE */
                  ENTRY + JOB, /* JOB_NAME */
                  ENTRY + JOB, /* JOB_NOTIFY */
                  ENTRY + JOB, /* JOB_PENDING */
                  ENTRY + JOB, /* JOB_PID */
                  ENTRY + JOB, /* JOB_REFUSED */
                        QUEUE, /* JOB_RESET_MODULES */
                  ENTRY + JOB, /* JOB_RESTART */
                  ENTRY + JOB, /* JOB_RETAINED */
                  ENTRY + JOB, /* JOB_SIZE */
                        QUEUE, /* JOB_SIZE_MAXIMUM */
                        QUEUE, /* JOB_SIZE_MINIMUM */
                  ENTRY + JOB, /* JOB_STARTING */
                  ENTRY + JOB, /* JOB_STATUS */
                  ENTRY + JOB, /* JOB_SUSPENDED */
                  ENTRY + JOB, /* JOB_TIMED_RELEASE */
                  ENTRY + JOB, /* JOB_WSDEFAULT */
                  ENTRY + JOB, /* JOB_WSEXTENT */
                  ENTRY + JOB, /* JOB_WSQUOTA */
                         FILE, /* LAST_PAGE */
                        QUEUE, /* LIBRARY_SPECIFICATION */
                  ENTRY + JOB, /* LOG_QUEUE */
                  ENTRY + JOB, /* LOG_SPECIFICATION */
                  ENTRY + JOB, /* NOTE */
                  ENTRY + JOB, /* OPERATOR_REQUEST */
                        QUEUE, /* OWNER_UIC */
                         FORM, /* PAGE_SETUP_MODULES */
                  ENTRY + JOB, /* PARAMETER_1 */
                  ENTRY + JOB, /* PARAMETER_2 */
                  ENTRY + JOB, /* PARAMETER_3 */
                  ENTRY + JOB, /* PARAMETER_4 */
                  ENTRY + JOB, /* PARAMETER_5 */
                  ENTRY + JOB, /* PARAMETER_6 */
                  ENTRY + JOB, /* PARAMETER_7 */
                  ENTRY + JOB, /* PARAMETER_8 */
                        QUEUE, /* PENDING_JOB_BLOCK_COUNT */
                        QUEUE, /* PENDING_JOB_COUNT */
                  ENTRY + JOB, /* PENDING_JOB_REASON */
                  ENTRY + JOB, /* PEND_CHAR_MISMATCH */
                  ENTRY + JOB, /* PEND_JOB_SIZE_MAX */
                  ENTRY + JOB, /* PEND_JOB_SIZE_MIN */
                  ENTRY + JOB, /* PEND_LOWERCASE_MISMATCH */
                  ENTRY + JOB, /* PEND_NO_ACCESS */
                  ENTRY + JOB, /* PEND_QUEUE_BUSY */
                  ENTRY + JOB, /* PEND_QUEUE_STATE */
                  ENTRY + JOB, /* PEND_STOCK_MISMATCH */
                  ENTRY + JOB, /* PRIORITY */
                        QUEUE, /* PROCESSOR */
                        QUEUE, /* PROTECTION */
                        QUEUE, /* QUEUE_DESCRIPTION */
                        QUEUE, /* QUEUE_ACL_SPECIFIED */
                        QUEUE, /* QUEUE_ALIGNING */
                        QUEUE, /* QUEUE_BATCH */
                        QUEUE, /* QUEUE_CLOSED */
                        QUEUE, /* QUEUE_CPU_DEFAULT */
                        QUEUE, /* QUEUE_CPU_LIMIT */
                        QUEUE, /* QUEUE_FILE_BURST */
                        QUEUE, /* QUEUE_FILE_BURST_ONE */
                        QUEUE, /* QUEUE_FILE_FLAG */
                        QUEUE, /* QUEUE_FILE_FLAG_ONE */
                        QUEUE, /* QUEUE_FILE_PAGINATE */
                        QUEUE, /* QUEUE_FILE_TRAILER */
                        QUEUE, /* QUEUE_FILE_TRAILER_ONE */
                        QUEUE, /* QUEUE_FLAGS */
                        QUEUE, /* QUEUE_GENERIC */
                        QUEUE, /* QUEUE_GENERIC_SELECTION */
                        QUEUE, /* QUEUE_IDLE */
                        QUEUE, /* QUEUE_JOB_BURST */
                        QUEUE, /* QUEUE_JOB_FLAG */
                        QUEUE, /* QUEUE_JOB_SIZE_SCHED */
                        QUEUE, /* QUEUE_JOB_TRAILER */
                        QUEUE, /* QUEUE_LOWERCASE */
  TRANS + ENTRY + JOB + QUEUE, /* QUEUE_NAME */
                        QUEUE, /* QUEUE_PAUSED */
                        QUEUE, /* QUEUE_PAUSING */
                        QUEUE, /* QUEUE_PRINTER */
                        QUEUE, /* QUEUE_RECORD_BLOCKING */
                        QUEUE, /* QUEUE_REMOTE */
                        QUEUE, /* QUEUE_RESETTING */
                        QUEUE, /* QUEUE_RESUMING */
                        QUEUE, /* QUEUE_RETAIN_ALL */
                        QUEUE, /* QUEUE_RETAIN_ERROR */
                        QUEUE, /* QUEUE_SERVER */
                        QUEUE, /* QUEUE_STALLED */
                        QUEUE, /* QUEUE_STARTING */
                        QUEUE, /* QUEUE_STATUS */
                        QUEUE, /* QUEUE_STOPPED */
                        QUEUE, /* QUEUE_STOPPING */
                        QUEUE, /* QUEUE_SWAP */
                        QUEUE, /* QUEUE_TERMINAL */
                        QUEUE, /* QUEUE_UNAVAILABLE */
                        QUEUE, /* QUEUE_WSDEFAULT */
                        QUEUE, /* QUEUE_WSEXTENT */
                        QUEUE, /* QUEUE_WSQUOTA */
                  ENTRY + JOB, /* REQUEUE_QUEUE_NAME */
                          JOB, /* RESTART_QUEUE_NAME */
                        QUEUE, /* RETAINED_JOB_COUNT */
                        QUEUE, /* SCSNODE_NAME */
                  ENTRY + JOB, /* SUBMISSION_TIME */
                        QUEUE, /* TIMED_RELEASE_JOB_COUNT */
                  ENTRY + JOB, /* UIC */
                  ENTRY + JOB, /* USERNAME */
          ENTRY + JOB + QUEUE, /* WSDEFAULT */
          ENTRY + JOB + QUEUE, /* WSEXTENT */
          ENTRY + JOB + QUEUE, /* WSQUOTA */
} ;


static const struct dvi_items_type qui_items[] = {
   { TYP_LSTR, "ACCOUNT_NAME",          QUI$_ACCOUNT_NAME               },
   { TYP_TIME, "AFTER_TIME",            QUI$_AFTER_TIME                 },
   { TYP_LSTR, "ASSIGNED_QUEUE_NAME",   QUI$_ASSIGNED_QUEUE_NAME        },
   { TYP_INT,  "BASE_PRIORITY",         QUI$_BASE_PRIORITY              },
   { TYP_LSTR, "CHARACTERISTICS",       QUI$_CHARACTERISTICS            },
   { TYP_LSTR, "CHARACTERISTIC_NAME",   QUI$_CHARACTERISTIC_NAME        },
   { TYP_INT,  "CHARACTERISTIC_NUMBER", QUI$_CHARACTERISTIC_NUMBER      },
   { TYP_LSTR, "CHECKPOINT_DATA",       QUI$_CHECKPOINT_DATA            },
   { TYP_LSTR, "CLI",                   QUI$_CLI                        },
   { TYP_INT,  "COMPLETED_BLOCKS",      QUI$_COMPLETED_BLOCKS           },
   { TYP_HEX,  "CONDITION_VECTOR",      QUI$_CONDITION_VECTOR           },
   { TYP_DTIM, "CPU_DEFAULT",           QUI$_CPU_DEFAULT                },
   { TYP_DTIM, "CPU_LIMIT",             QUI$_CPU_LIMIT                  },
   { TYP_LSTR, "DEFAULT_FORM_NAME",     QUI$_DEFAULT_FORM_NAME          },
   { TYP_LSTR, "DEFAULT_FORM_STOCK",    QUI$_DEFAULT_FORM_STOCK         },
   { TYP_LSTR, "DEVICE_NAME",           QUI$_DEVICE_NAME                },
   { TYP_INT,  "ENTRY_NUMBER",          QUI$_ENTRY_NUMBER               },
   { TYP_INT,  "EXECUTING_JOB_COUNT",   QUI$_EXECUTING_JOB_COUNT        },
   { TYP_FLF,  "FILE_BURST",            QUI$M_FILE_BURST                },
   { TYP_FLS,  "FILE_CHECKPOINTED",     QUI$M_FILE_CHECKPOINTED         },
   { TYP_INT,  "FILE_COPIES",           QUI$_FILE_COPIES                },
   { TYP_INT,  "FILE_COPIES_DONE",      QUI$_FILE_COPIES_DONE           },
   { TYP_FLF,  "FILE_DELETE",           QUI$M_FILE_DELETE               },
   { TYP_FLF,  "FILE_DOUBLE_SPACE",     QUI$M_FILE_DOUBLE_SPACE         },
   { TYP_FLS,  "FILE_EXECUTING",        QUI$M_FILE_EXECUTING            },
   { TYP_FLF,  "FILE_FLAG",             QUI$M_FILE_FLAG                 },
   { TYP_INT,  "FILE_FLAGS",            QUI$_FILE_FLAGS                 },
   { TYP_LSTR, "FILE_IDENTIFICATION",   QUI$_FILE_IDENTIFICATION        },
   { TYP_FLF,  "FILE_PAGE_HEADER",      QUI$M_FILE_PAGE_HEADER          },
   { TYP_FLF,  "FILE_PAGINATE",         QUI$M_FILE_PAGINATE             },
   { TYP_FLF,  "FILE_PASSALL",          QUI$M_FILE_PASSALL              },
   { TYP_LSTR, "FILE_SETUP_MODULES",    QUI$_FILE_SETUP_MODULES         },
   { TYP_LSTR, "FILE_SPECIFICATION",    QUI$_FILE_SPECIFICATION         },
   { TYP_INT,  "FILE_STATUS",           QUI$_FILE_STATUS                },
   { TYP_FLF,  "FILE_TRAILER",          QUI$M_FILE_TRAILER              },
   { TYP_INT,  "FIRST_PAGE",            QUI$_FIRST_PAGE                 },
   { TYP_LSTR, "FORM_DESCRIPTION",      QUI$_FORM_DESCRIPTION           },
   { TYP_INT,  "FORM_FLAGS",            QUI$_FORM_FLAGS                 },
   { TYP_INT,  "FORM_LENGTH",           QUI$_FORM_LENGTH                },
   { TYP_INT,  "FORM_MARGIN_BOTTOM",    QUI$_FORM_MARGIN_BOTTOM         },
   { TYP_INT,  "FORM_MARGIN_LEFT",      QUI$_FORM_MARGIN_LEFT           },
   { TYP_INT,  "FORM_MARGIN_RIGHT",     QUI$_FORM_MARGIN_RIGHT          },
   { TYP_INT,  "FORM_MARGIN_TOP",       QUI$_FORM_MARGIN_TOP            },
   { TYP_LSTR, "FORM_NAME",             QUI$_FORM_NAME                  },
   { TYP_INT,  "FORM_NUMBER",           QUI$_FORM_NUMBER                },
   { TYP_LSTR, "FORM_SETUP_MODULES",    QUI$_FORM_SETUP_MODULES         },
   { TYP_FMF,  "FORM_SHEET_FEED",       QUI$M_FORM_SHEET_FEED           },
   { TYP_LSTR, "FORM_STOCK",            QUI$_FORM_STOCK                 },
   { TYP_FMF,  "FORM_TRUNCATE",         QUI$M_FORM_TRUNCATE             },
   { TYP_INT,  "FORM_WIDTH",            QUI$_FORM_WIDTH                 },
   { TYP_FMF,  "FORM_WRAP",             QUI$M_FORM_WRAP                 },
   { TYP_LSTR, "GENERIC_TARGET",        QUI$_GENERIC_TARGET             },
   { TYP_INT,  "HOLDING_JOB_COUNT",     QUI$_HOLDING_JOB_COUNT          },
   { TYP_INT,  "INTERVENING_BLOCKS",    QUI$_INTERVENING_BLOCKS         },
   { TYP_INT,  "INTERVENING_JOBS",      QUI$_INTERVENING_JOBS           },
   { TYP_JBS,  "JOB_ABORTING",          QUI$M_JOB_ABORTING              },
   { TYP_INT,  "JOB_COPIES",            QUI$_JOB_COPIES                 },
   { TYP_INT,  "JOB_COPIES_DONE",       QUI$_JOB_COPIES_DONE            },
   { TYP_JBF,  "JOB_CPU_LIMIT",         QUI$M_JOB_CPU_LIMIT             },
   { TYP_JBS,  "JOB_EXECUTING",         QUI$M_JOB_EXECUTING             },
   { TYP_JBF,  "JOB_FILE_BURST",        QUI$M_JOB_FILE_BURST            },
   { TYP_JBF,  "JOB_FILE_BURST_ONE",    QUI$M_JOB_FILE_BURST_ONE        },
   { TYP_JBF,  "JOB_FILE_FLAG",         QUI$M_JOB_FILE_FLAG             },
   { TYP_JBF,  "JOB_FILE_FLAG_ONE",     QUI$M_JOB_FILE_FLAG_ONE         },
   { TYP_JBF,  "JOB_FILE_PAGINATE",     QUI$M_JOB_FILE_PAGINATE         },
   { TYP_JBF,  "JOB_FILE_TRAILER",      QUI$M_JOB_FILE_TRAILER          },
   { TYP_JBF,  "JOB_FILE_TRAILER_ONE",  QUI$M_JOB_FILE_TRAILER_ONE      },
   { TYP_INT,  "JOB_FLAGS",             QUI$_JOB_FLAGS                  },
   { TYP_JBS,  "JOB_HOLDING",           QUI$M_JOB_HOLDING               },
   { TYP_JBS,  "JOB_INACCESSIBLE",      QUI$M_JOB_INACCESSIBLE          },
   { TYP_INT,  "JOB_LIMIT",             QUI$_JOB_LIMIT                  },
   { TYP_JBF,  "JOB_LOG_DELETE",        QUI$M_JOB_LOG_DELETE            },
   { TYP_JBF,  "JOB_LOG_NULL",          QUI$M_JOB_LOG_NULL              },
   { TYP_JBF,  "JOB_LOG_SPOOL",         QUI$M_JOB_LOG_SPOOL             },
   { TYP_JBF,  "JOB_LOWERCASE",         QUI$M_JOB_LOWERCASE             },
   { TYP_LSTR, "JOB_NAME",              QUI$_JOB_NAME                   },
   { TYP_JBF,  "JOB_NOTIFY",            QUI$M_JOB_NOTIFY                },
   { TYP_JBS,  "JOB_PENDING",           QUI$M_JOB_PENDING               },
   { TYP_HEX,  "JOB_PID",               QUI$_JOB_PID                    },
   { TYP_JBS,  "JOB_REFUSED",           QUI$M_JOB_REFUSED               },
   { TYP_LSTR, "JOB_RESET_MODULES",     QUI$_JOB_RESET_MODULES          },
   { TYP_JBF,  "JOB_RESTART",           QUI$M_JOB_RESTART               },
   { TYP_JBS,  "JOB_RETAINED",          QUI$M_JOB_RETAINED              },
   { TYP_INT,  "JOB_SIZE",              QUI$_JOB_SIZE                   },
   { TYP_INT,  "JOB_SIZE_MAXIMUM",      QUI$_JOB_SIZE_MAXIMUM           },
   { TYP_INT,  "JOB_SIZE_MINIMUM",      QUI$_JOB_SIZE_MINIMUM           },
   { TYP_JBS,  "JOB_STARTING",          QUI$M_JOB_STARTING              },
   { TYP_INT,  "JOB_STATUS",            QUI$_JOB_STATUS                 },
   { TYP_JBS,  "JOB_SUSPENDED",         QUI$M_JOB_SUSPENDED             },
   { TYP_JBS,  "JOB_TIMED_RELEASE",     QUI$M_JOB_TIMED_RELEASE         },
   { TYP_JBF,  "JOB_WSDEFAULT",         QUI$M_JOB_WSDEFAULT             },
   { TYP_JBF,  "JOB_WSEXTENT",          QUI$M_JOB_WSEXTENT              },
   { TYP_JBF,  "JOB_WSQUOTA",           QUI$M_JOB_WSQUOTA               },
   { TYP_INT,  "LAST_PAGE",             QUI$_LAST_PAGE                  },
   { TYP_LSTR, "LIBRARY_SPECIFICATION", QUI$_LIBRARY_SPECIFICATION      },
   { TYP_LSTR, "LOG_QUEUE",             QUI$_LOG_QUEUE                  },
   { TYP_LSTR, "LOG_SPECIFICATION",     QUI$_LOG_SPECIFICATION          },
   { TYP_LSTR, "NOTE",                  QUI$_NOTE                       },
   { TYP_LSTR, "OPERATOR_REQUEST",      QUI$_OPERATOR_REQUEST           },
   { TYP_UIC,  "OWNER_UIC",             QUI$_OWNER_UIC                  },
   { TYP_LSTR, "PAGE_SETUP_MODULES",    QUI$_PAGE_SETUP_MODULES         },
   { TYP_LSTR, "PARAMETER_1",           QUI$_PARAMETER_1                },
   { TYP_LSTR, "PARAMETER_2",           QUI$_PARAMETER_2                },
   { TYP_LSTR, "PARAMETER_3",           QUI$_PARAMETER_3                },
   { TYP_LSTR, "PARAMETER_4",           QUI$_PARAMETER_4                },
   { TYP_LSTR, "PARAMETER_5",           QUI$_PARAMETER_5                },
   { TYP_LSTR, "PARAMETER_6",           QUI$_PARAMETER_6                },
   { TYP_LSTR, "PARAMETER_7",           QUI$_PARAMETER_7                },
   { TYP_LSTR, "PARAMETER_8",           QUI$_PARAMETER_8                },
   { TYP_INT,  "PENDING_JOB_BLOCK_COUNT",QUI$_PENDING_JOB_BLOCK_COUNT   },
   { TYP_INT,  "PENDING_JOB_COUNT",     QUI$_PENDING_JOB_COUNT          },
   { TYP_INT,  "PENDING_JOB_REASON",    QUI$_PENDING_JOB_REASON         },
   { TYP_PJR,  "PEND_CHAR_MISMATCH",    QUI$M_PEND_CHAR_MISMATCH        },
   { TYP_PJR,  "PEND_JOB_SIZE_MAX",     QUI$M_PEND_JOB_SIZE_MAX         },
   { TYP_PJR,  "PEND_JOB_SIZE_MIN",     QUI$M_PEND_JOB_SIZE_MIN         },
   { TYP_PJR,  "PEND_LOWERCASE_MISMATCH",QUI$M_PEND_LOWERCASE_MISMATCH  },
   { TYP_PJR,  "PEND_NO_ACCESS",        QUI$M_PEND_NO_ACCESS            },
   { TYP_PJR,  "PEND_QUEUE_BUSY",       QUI$M_PEND_QUEUE_BUSY           },
   { TYP_PJR,  "PEND_QUEUE_STATE",      QUI$M_PEND_QUEUE_STATE          },
   { TYP_PJR,  "PEND_STOCK_MISMATCH",   QUI$M_PEND_STOCK_MISMATCH       },
   { TYP_INT,  "PRIORITY",              QUI$_PRIORITY                   },
   { TYP_LSTR, "PROCESSOR",             QUI$_PROCESSOR                  },
   { TYP_PROT, "PROTECTION",            QUI$_PROTECTION                 },
   { TYP_QUF,  "QUEUE_ACL_SPECIFIED",   QUI$M_QUEUE_ACL_SPECIFIED       },
   { TYP_QUS,  "QUEUE_ALIGNING",        QUI$M_QUEUE_ALIGNING            },
   { TYP_QUF,  "QUEUE_BATCH",           QUI$M_QUEUE_BATCH               },
   { TYP_QUS,  "QUEUE_CLOSED",          QUI$M_QUEUE_CLOSED              },
   { TYP_QUF,  "QUEUE_CPU_DEFAULT",     QUI$M_QUEUE_CPU_DEFAULT         },
   { TYP_QUF,  "QUEUE_CPU_LIMIT",       QUI$M_QUEUE_CPU_LIMIT           },
   { TYP_MSTR, "QUEUE_DESCRIPTION",     QUI$_QUEUE_DESCRIPTION          },
   { TYP_QUF,  "QUEUE_FILE_BURST",      QUI$M_QUEUE_FILE_BURST          },
   { TYP_QUF,  "QUEUE_FILE_BURST_ONE",  QUI$M_QUEUE_FILE_BURST_ONE      },
   { TYP_QUF,  "QUEUE_FILE_FLAG",       QUI$M_QUEUE_FILE_FLAG           },
   { TYP_QUF,  "QUEUE_FILE_FLAG_ONE",   QUI$M_QUEUE_FILE_FLAG_ONE       },
   { TYP_QUF,  "QUEUE_FILE_PAGINATE",   QUI$M_QUEUE_FILE_PAGINATE       },
   { TYP_QUF,  "QUEUE_FILE_TRAILER",    QUI$M_QUEUE_FILE_TRAILER        },
   { TYP_QUF,  "QUEUE_FILE_TRAILER_ONE",QUI$M_QUEUE_FILE_TRAILER_ONE    },
   { TYP_INT,  "QUEUE_FLAGS",           QUI$_QUEUE_FLAGS                },
   { TYP_QUF,  "QUEUE_GENERIC",         QUI$M_QUEUE_GENERIC             },
   { TYP_QUF,  "QUEUE_GENERIC_SELECTION",QUI$M_QUEUE_GENERIC_SELECTION  },
   { TYP_QUS,  "QUEUE_IDLE",            QUI$M_QUEUE_IDLE                },
   { TYP_QUF,  "QUEUE_JOB_BURST",       QUI$M_QUEUE_JOB_BURST           },
   { TYP_QUF,  "QUEUE_JOB_FLAG",        QUI$M_QUEUE_JOB_FLAG            },
   { TYP_QUF,  "QUEUE_JOB_SIZE_SCHED",  QUI$M_QUEUE_JOB_SIZE_SCHED      },
   { TYP_QUF,  "QUEUE_JOB_TRAILER",     QUI$M_QUEUE_JOB_TRAILER         },
   { TYP_QUS,  "QUEUE_LOWERCASE",       QUI$M_QUEUE_LOWERCASE           },
   { TYP_LSTR, "QUEUE_NAME",            QUI$_QUEUE_NAME                 },
   { TYP_QUS,  "QUEUE_PAUSED",          QUI$M_QUEUE_PAUSED              },
   { TYP_QUS,  "QUEUE_PAUSING",         QUI$M_QUEUE_PAUSING             },
   { TYP_QUF,  "QUEUE_PRINTER",         QUI$M_QUEUE_PRINTER             },
   { TYP_QUF,  "QUEUE_RECORD_BLOCKING", QUI$M_QUEUE_RECORD_BLOCKING     },
   { TYP_QUS,  "QUEUE_REMOTE",          QUI$M_QUEUE_REMOTE              },
   { TYP_QUS,  "QUEUE_RESETTING",       QUI$M_QUEUE_RESETTING           },
   { TYP_QUS,  "QUEUE_RESUMING",        QUI$M_QUEUE_RESUMING            },
   { TYP_QUF,  "QUEUE_RETAIN_ALL",      QUI$M_QUEUE_RETAIN_ALL          },
   { TYP_QUF,  "QUEUE_RETAIN_ERROR",    QUI$M_QUEUE_RETAIN_ERROR        },
   { TYP_QUS,  "QUEUE_SERVER",          QUI$M_QUEUE_SERVER              },
   { TYP_QUS,  "QUEUE_STALLED",         QUI$M_QUEUE_STALLED             },
   { TYP_QUS,  "QUEUE_STARTING",        QUI$M_QUEUE_STARTING            },
   { TYP_INT,  "QUEUE_STATUS",          QUI$_QUEUE_STATUS               },
   { TYP_QUS,  "QUEUE_STOPPED",         QUI$M_QUEUE_STOPPED             },
   { TYP_QUS,  "QUEUE_STOPPING",        QUI$M_QUEUE_STOPPING            },
   { TYP_QUF,  "QUEUE_SWAP",            QUI$M_QUEUE_SWAP                },
   { TYP_QUF,  "QUEUE_TERMINAL",        QUI$M_QUEUE_TERMINAL            },
   { TYP_QUS,  "QUEUE_UNAVAILABLE",     QUI$M_QUEUE_UNAVAILABLE         },
   { TYP_QUF,  "QUEUE_WSDEFAULT",       QUI$M_QUEUE_WSDEFAULT           },
   { TYP_QUF,  "QUEUE_WSEXTENT",        QUI$M_QUEUE_WSEXTENT            },
   { TYP_QUF,  "QUEUE_WSQUOTA",         QUI$M_QUEUE_WSQUOTA             },
   { TYP_LSTR, "REQUEUE_QUEUE_NAME",    QUI$_REQUEUE_QUEUE_NAME         },
   { TYP_LSTR, "RESTART_QUEUE_NAME",    QUI$_RESTART_QUEUE_NAME         },
   { TYP_INT,  "RETAINED_JOB_COUNT",    QUI$_RETAINED_JOB_COUNT         },
   { TYP_LSTR, "SCSNODE_NAME",          QUI$_SCSNODE_NAME               },
   { TYP_TIME, "SUBMISSION_TIME",       QUI$_SUBMISSION_TIME            },
   { TYP_INT,  "TIMED_RELEASE_JOB_COUNT",QUI$_TIMED_RELEASE_JOB_COUNT   },
   { TYP_LSTR, "UIC",                   QUI$_UIC                        },
   { TYP_LSTR, "USERNAME",              QUI$_USERNAME                   },
   { TYP_INT,  "WSDEFAULT",             QUI$_WSDEFAULT                  },
   { TYP_INT,  "WSEXTENT",              QUI$_WSEXTENT                   },
   { TYP_INT,  "WSQUOTA",               QUI$_WSQUOTA                    },
} ;

static const struct dvi_items_type qui_flags[] = {
   { TYP_INT, "ALL_JOBS",               QUI$M_SEARCH_ALL_JOBS           },
   { TYP_INT, "BATCH",                  QUI$M_SEARCH_BATCH              },
   { TYP_INT, "EXECUTING_JOBS",         QUI$M_SEARCH_EXECUTING_JOBS     },
   { TYP_INT, "FREEZE_CONTEXT",         QUI$M_SEARCH_FREEZE_CONTEXT     },
   { TYP_INT, "GENERIC",                QUI$M_SEARCH_GENERIC            },
   { TYP_INT, "HOLDING_JOBS",           QUI$M_SEARCH_HOLDING_JOBS       },
   { TYP_INT, "PENDING_JOBS",           QUI$M_SEARCH_PENDING_JOBS       },
   { TYP_INT, "PRINTER",                QUI$M_SEARCH_PRINTER            },
   { TYP_INT, "RETAINED_JOBS",          QUI$M_SEARCH_RETAINED_JOBS      },
   { TYP_INT, "SERVER",                 QUI$M_SEARCH_SERVER             },
   { TYP_INT, "SYMBIONT",               QUI$M_SEARCH_SYMBIONT           },
   { TYP_INT, "TERMINAL",               QUI$M_SEARCH_TERMINAL           },
   { TYP_INT, "THIS_JOB",               QUI$M_SEARCH_THIS_JOB           },
   { TYP_INT, "TIMED_RELEASE_JOBS",     QUI$M_SEARCH_TIMED_RELEASE_JOBS },
   { TYP_INT, "WILDCARD",               QUI$M_SEARCH_WILDCARD           },
} ;

static const int qui_stats[] = {
        QUI$_FILE_FLAGS,                QUI$_FILE_STATUS,
        QUI$_FORM_FLAGS,                QUI$_JOB_FLAGS,
        QUI$_JOB_STATUS,                QUI$_PENDING_JOB_REASON,
        QUI$_QUEUE_FLAGS,               QUI$_QUEUE_STATUS,
} ;


streng *vms_f_getqui( tsd_t *TSD, cparamboxptr parms )
{
   short length, func ;
   int flags, i, item_value, objnum, rc, usenum, item_mask ;
   char buffer[256] ;
   int items[21], cnt=0, *vector ;
   int search_flags=0, search_length=4, search_number[10], search_nlength ;
   cparamboxptr tmp ;
   int ioblk[2] ;
   streng *item, *objid ;
   struct dvi_items_type *ptr, *item_ptr ;
   $DESCRIPTOR( objdescr, "" ) ;
   $DESCRIPTOR( resdescr, buffer ) ;

   if (!parms->value)
      exiterror( ERR_INCORRECT_CALL , 0 ) ;

/*
 * First, find the function we are to perform, that is the first parameter
 * in the call to f$getqui().
 */
   if (!(ptr=item_info( parms->value, qui_funcs, sizeof(qui_funcs))))
      exiterror( ERR_INCORRECT_CALL , 0 )  ;

/*
 * Depending on the function chosen, check that the parameters are legal
 * for than function. I.e. all parameters that must be specified exists,
 * and no illegal parameters are specified.
 */
   tmp = parms->next ;
   for (i=0; i<3; i++)
   {
      if (((ptr->type >> i) & 0x01) && ((!tmp) || (!tmp->value)))
         exiterror( ERR_INCORRECT_CALL , 0 ) ;

      if (((ptr->type >> i) & 0x10) && ((tmp) && (tmp->value)))
         exiterror( ERR_INCORRECT_CALL , 0 ) ;

      if (tmp) tmp = tmp->next ;
   }

   tmp = parms->next ;
   if (objid = (tmp && tmp->next) ? ((tmp=tmp->next)->value) : NULL )
   {
      if (usenum=myisnumber(TSD, objid))
      {
         items[cnt++] = 4 + ( QUI$_SEARCH_NUMBER << 16 ) ;
         items[cnt++] = (int)&(search_number[0]) ;
         items[cnt++] = (int)NULL /* &search_nlength  */ ;
         search_number[0] = atozpos(TSD, objid, "VMS_F_GETQUI", 0 ) ;
         search_nlength = 0 ;
         length = 0 ;
      }
      else
      {
         items[cnt++] = objid->len + ( QUI$_SEARCH_NAME << 16 ) ;
         items[cnt++] = (int)&(objid->value[0]) ;
         items[cnt++] = (int)&search_nlength ;
         search_nlength = objid->len ;
      }
   }

/*
 * Now, find the item for which we are to retrieve information. If the
 * type-specified indicates that this is part of a vector which must
 * be split up, then save som vital information.
 */
   item = (tmp=parms->next) ? (tmp->value) : NULL ;
   if (item)
   {
      item_ptr = item_info( item, qui_items, sizeof( qui_items )) ;
      if (!item_ptr)
         exiterror( ERR_INCORRECT_CALL , 0 ) ;

      if (item_ptr->type >= TYP_SPECIFICS)
      {
         item_value = qui_stats[ item_ptr->type - TYP_SPECIFICS ] ;
         item_mask = item_ptr->addr ;
      }
      else
         item_value = item_ptr->addr ;

      items[cnt++] = 256 + ( item_value << 16 ) ;
      items[cnt++] = (int)buffer ;
      items[cnt++] = (int)&length ;
      vector = (int *)buffer ;

      if (!(leg_items[item_ptr - qui_items] & (1 << (ptr-qui_funcs))))
          exiterror( ERR_INCORRECT_CALL , 0 ) ;
   }
   else
      item_ptr = NULL ;

   items[cnt++] = 0 ;
   items[cnt++] = 0 ;
   items[cnt++] = 0 ;

   func = ptr->addr ;
   ioblk[0] = ioblk[1] = 0 ;

   rc = sys$getquiw( NULL, func, NULL, &items, ioblk, NULL, NULL ) ;

   if ((rc==SS$_NORMAL) && ((ioblk[0]==JBC$_NOSUCHJOB) ||
          (ioblk[0]==JBC$_NOMOREQUE) || (ioblk[0]==JBC$_NOQUECTX)))
      return nullstringptr() ;

   if (rc != SS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return nullstringptr() ;
   }

   if (!item_ptr)
      return nullstringptr() ;

   if (ioblk[0] != JBC$_NORMAL)
   {
      vms_error( TSD, ioblk[0] ) ;
      return nullstringptr() ;
   }

   if ( item_ptr->type >= TYP_SPECIFICS)
      return Str_creTSD( (*vector & item_ptr->addr) ? "TRUE" : "FALSE" ) ;

   return format_result( TSD, item_ptr->type, buffer, length ) ;
}



static const struct dvi_items_type syi_items[] = {
   { TYP_INT,  "ACTIVECPU_CNT",         SYI$_ACTIVECPU_CNT              },
   { TYP_INT,  "ARCHFLAG",              SYI$_ARCHFLAG                   },
   { TYP_INT,  "AVAILCPU_CNT",          SYI$_AVAILCPU_CNT               },
   { TYP_TIME, "BOOTTIME",              SYI$_BOOTTIME                   },
   { TYP_BOOL, "CHARACTER_EMULATED",    SYI$_CHARACTER_EMULATED         },
/* { TYP_INT,  "CLUSTER_EVOTES",        SYI$_CLUSTER_EVOTES             }, */
   { TYP_LHEX, "CLUSTER_FSYSID",        SYI$_CLUSTER_FSYSID             },
   { TYP_TIME, "CLUSTER_FTIME",         SYI$_CLUSTER_FTIME              },
   { TYP_BOOL, "CLUSTER_MEMBER",        SYI$_CLUSTER_MEMBER             },
   { TYP_INT,  "CLUSTER_NODES",         SYI$_CLUSTER_NODES              },
   { TYP_INT,  "CLUSTER_QUORUM",        SYI$_CLUSTER_QUORUM             },
   { TYP_INT,  "CLUSTER_VOTES",         SYI$_CLUSTER_VOTES              },
   { TYP_INT,  "CONTIG_GBLPAGES",       SYI$_CONTIG_GBLPAGES            },
   { TYP_INT,  "CPU",                   SYI$_CPU                        },
   { TYP_BOOL, "DECIMAL_EMULATED",      SYI$_DECIMAL_EMULATED           },
   { TYP_BOOL, "D_FLOAT_EMULATED",      SYI$_D_FLOAT_EMULATED           },
   { TYP_INT,  "ERRORLOGBUFFERS",       SYI$_ERRORLOGBUFFERS            },
   { TYP_INT,  "FREE_GBLPAGES",         SYI$_FREE_GBLPAGES              },
   { TYP_INT,  "FREE_GBLSECTS",         SYI$_FREE_GBLSECTS              },
   { TYP_BOOL, "F_FLOAT_EMULATED",      SYI$_F_FLOAT_EMULATED           },
   { TYP_BOOL, "G_FLOAT_EMULATED",      SYI$_G_FLOAT_EMULATED           },
   { TYP_INT,  "HW_MODEL",              SYI$_HW_MODEL                   },
   { TYP_LSTR, "HW_NAME",               SYI$_HW_NAME                    },
   { TYP_BOOL, "H_FLOAT_EMULATED",      SYI$_H_FLOAT_EMULATED           },
   { TYP_LSTR, "NODENAME",              SYI$_NODENAME                   },
   { TYP_INT,  "NODE_AREA",             SYI$_NODE_AREA                  },
   { TYP_LHEX, "NODE_CSID",             SYI$_NODE_CSID                  },
   { TYP_INT,  "NODE_EVOTES",           SYI$_NODE_EVOTES                },
   { TYP_LSTR, "NODE_HWTYPE",           SYI$_NODE_HWTYPE                },
   { TYP_LHEX, "NODE_HWVERS",           SYI$_NODE_HWVERS                },
   { TYP_INT,  "NODE_NUMBER",           SYI$_NODE_NUMBER                },
   { TYP_INT,  "NODE_QUORUM",           SYI$_NODE_QUORUM                },
   { TYP_LHEX, "NODE_SWINCARN",         SYI$_NODE_SWINCARN              },
   { TYP_LSTR, "NODE_SWTYPE",           SYI$_NODE_SWTYPE                },
   { TYP_LSTR, "NODE_SWVERS",           SYI$_NODE_SWVERS                },
   { TYP_LHEX, "NODE_SYSTEMID",         SYI$_NODE_SYSTEMID              },
   { TYP_INT,  "NODE_VOTES",            SYI$_NODE_VOTES                 },
   { TYP_INT,  "PAGEFILE_FREE",         SYI$_PAGEFILE_FREE              },
   { TYP_INT,  "PAGEFILE_PAGE",         SYI$_PAGEFILE_PAGE              },
   { TYP_BOOL, "SCS_EXISTS",            SYI$_SCS_EXISTS                 },
   { TYP_INT,  "SID",                   SYI$_SID                        },
   { TYP_INT,  "SWAPFILE_FREE",         SYI$_SWAPFILE_FREE              },
   { TYP_INT,  "SWAPFILE_PAGE",         SYI$_SWAPFILE_PAGE              },
   { TYP_LSTR, "VERSION",               SYI$_VERSION                    },
   { TYP_INT,  "XCPU",                  SYI$_XCPU                       },
   { TYP_INT,  "XSID",                  SYI$_XSID                       },
} ;

streng *vms_f_getsyi( tsd_t *TSD, cparamboxptr parms )
{
   char buffer[64] ;
   int length=0, rc, item[6] ;
   struct dvi_items_type *ptr ;
   struct dsc$descriptor_s name, *namep=NULL ;
   struct dsc$descriptor_s dir = {
       sizeof(buffer)-1, DSC$K_DTYPE_T, DSC$K_CLASS_S, buffer } ;

   checkparam( parms, 1, 2, "VMS_F_GETSYI" ) ;

   ptr = item_info( parms->value, syi_items, sizeof( syi_items)) ;
   if (!ptr)
      exiterror( ERR_INCORRECT_CALL , 0 ) ;

   item[0] = 64 + (ptr->addr << 16) ;
   item[1] = (int)buffer ;
   item[2] = (int)&length ;
   item[3] = item[4] = item[5] = 0 ;

   if (parms->next && parms->next->value)
   {
      namep = &name ;
      name.dsc$w_length = Str_len( parms->value ) ;
      name.dsc$b_dtype = DSC$K_DTYPE_T ;
      name.dsc$b_class = DSC$K_CLASS_S ;
      name.dsc$a_pointer = parms->value->value ;
   }

   rc = sys$getsyiw( NULL, NULL, namep, &item[0], NULL, NULL, NULL ) ;

   if (rc != SS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return Str_creTSD("") ;
   }

   return format_result( TSD, ptr->type, buffer, length ) ;
}



streng *vms_f_identifier( tsd_t *TSD, cparamboxptr parms )
{
   streng *in, *type, *result ;
   int id=0 ;

   checkparam( parms, 2, 2, "VMS_F_IDENTIFIER" ) ;

   type = parms->next->value ;

   if (type->len != 14)
      exiterror( ERR_INCORRECT_CALL , 0 ) ;

   if (!strncmp(type->value, "NAME_TO_NUMBER", 14))
      result = int_to_streng( TSD, name_to_num( TSD, parms->value )) ;
   else if (!strncmp(type->value, "NUMBER_TO_NAME", 14))
   {
      result = num_to_name( TSD, atozpos( TSD, parms->value, "VMS_F_IDENTIFIER", 1 )) ;
      if (!result)
         result = nullstringptr() ;
   }
   else
      exiterror( ERR_INCORRECT_CALL , 0 ) ;

   return result ;
}



streng *vms_f_message( tsd_t *TSD, cparamboxptr parms )
{
   char buffer[256] ;
   $DESCRIPTOR( name, buffer ) ;
   int length, rc, errmsg ;

   checkparam( parms, 1, 1, "VMS_F_MESSAGE" ) ;
   errmsg = atopos( TSD, parms->value, "VMS_F_MESSAGE", 1 ) ;

   rc = sys$getmsg( errmsg, &length, &name, NULL, NULL ) ;

   if ((rc != SS$_NORMAL) && (rc != SS$_MSGNOTFND))
      vms_error( TSD, rc ) ;

   return Str_ncatstrTSD( Str_makeTSD(length), buffer, length ) ;
}


streng *vms_f_mode( tsd_t *TSD, cparamboxptr parms )
{
   char buffer[256] ;
   $DESCRIPTOR( descr, buffer ) ;
   int item = JPI$_MODE, length, rc ;

   rc = lib$getjpi( &item, NULL, NULL, NULL, &descr, &length ) ;

   if (rc != SS$_NORMAL)
      vms_error( TSD, rc ) ;

   return Str_ncatstrTSD( Str_makeTSD(length), buffer, length ) ;
}


streng *vms_f_pid( tsd_t *TSD, cparamboxptr parms )
{
   short length ;
   int *pidp=NULL, rc, buffer ;
   int pid;
   unsigned int items[6] ;
   const streng *Pid ;
   vmf_tsd_t *vt;
   char *str;
   streng *val = NULL ;

   vt = TSD->vmf_tsd;
   checkparam( parms, 1, 1, "VMS_F_PID" ) ;

   items[0] = ( JPI$_PID << 16 ) + 4 ;
   items[1] = (unsigned int)&buffer ;
   items[2] = (unsigned int)&length ;
   items[3] = 0 ;
   items[4] = 0 ;
   items[5] = 0 ;

   Pid = getvalue( TSD, parms->value, -1 ) ;

   if (Pid->len)
   {
      str = str_of( TSD, val ) ;
      sscanf( str, "%x", &pid ) ;
      FreeTSD( str ) ;
   }
   else
      pid = -1 ;

   do {
      rc = sys$getjpiw( NULL, &pid, NULL, &items, NULL, NULL, NULL ) ;
      }
   while (rc == SS$_NOPRIV) ;

   if ((rc != SS$_NORMAL) && (rc != SS$_NOMOREPROC))
      vms_error( TSD, rc ) ;

   sprintf( (val=Str_makeTSD(10))->value, "%08x", pid ) ;
   val->len = 8 ;
   setvalue( TSD, parms->value, val, -1 ) ;

   if (rc == SS$_NOMOREPROC)
      return nullstringptr() ;

   assert( length==4 ) ;
   sprintf( (val=Str_makeTSD(10))->value, "%08x", buffer ) ;
   val->len = 8 ;

   return val ;
}


#define MAX_PRIVS (sizeof(all_privs)/sizeof(char*))

static streng *map_privs( const tsd_t *TSD, const int *vector )
{
   int i ;
   char *ptr, buffer[512] ;

   *(ptr=buffer) = 0x00 ;
   for (i=0; i<MAX_PRIVS; i++)
      if ((vector[i/32] >> (i%32)) & 0x01)
      {
         strcat( ptr, all_privs[i] ) ;
         ptr += strlen(all_privs[i]) ;
         strcat( ptr++, "," ) ;
      }

   if (ptr>buffer)
      *(--ptr) = 0x00 ;

   return Str_ncatstrTSD( Str_makeTSD(ptr-buffer), buffer, (ptr-buffer)) ;
}

static int extract_privs( int *vector, const streng *privs )
{
   int max_priv, negate, i ;
   const char *ptr, *eptr, *tptr, *lptr ;

   max_priv = MAX_PRIVS ;

   eptr = Str_end( privs ) ;
   for (ptr=privs->value; ptr<eptr; ptr=(++lptr) )
   {
      for (; isspace(*ptr) && ptr<eptr; ptr++ ) ;
      for (lptr=ptr; (lptr<eptr) && (*lptr!=','); lptr++) ;
      for (tptr=lptr; isspace(*(tptr-1)) && tptr>=ptr; tptr-- ) ;
      if (tptr-ptr<3)
         return 1 ;

      negate = ((*ptr=='N') && (*(ptr+1)=='O')) * 2 ;
      for (i=0; i<max_priv; i++)
         if ((!strncmp(ptr+negate,all_privs[i],tptr-ptr-negate)) &&
                                (all_privs[i][tptr-ptr-negate] == 0x00))
         {
            if (negate)
               vector[2+i/32] |= (1 << (i%32)) ;
            else
               vector[i/32] |= (1 << (i%32)) ;
            break ;
         }

      if (i==max_priv)
         return 1 ;
   }
   return 0 ;
}


streng *vms_f_privilege( tsd_t *TSD, cparamboxptr parms )
{
   int privbits[4], privs[2] ;
   int rc ;
   char *ptr, *eptr, *tptr ;

   checkparam( parms, 1, 1, "VMS_F_PRIVILEGE" ) ;
   extract_privs( privbits, parms->value ) ;

   rc = lib$getjpi( &JPI$_PROCPRIV, NULL, NULL, &privs, NULL, NULL ) ;
   if (rc != SS$_NORMAL)
      vms_error( TSD, rc ) ;

   return Str_creTSD(
            (((privbits[0] & ~privs[0]) | ( privbits[2] & privs[0] )) ||
             ((privbits[1] & ~privs[1]) | ( privbits[3] & privs[1] ))) ?
                     "FALSE" : "TRUE" ) ;
}



streng *vms_f_process( tsd_t *TSD, cparamboxptr parms )
{
   int rc, length ;
   char buffer[64] ;
   $DESCRIPTOR( descr, buffer ) ;

   checkparam( parms, 0, 0, "VMS_F_PROCESS" ) ;
   rc = lib$getjpi( &JPI$_PRCNAM, NULL, NULL, NULL, &descr, &length ) ;

   if ( rc != SS$_NORMAL)
      vms_error( TSD, rc ) ;

   return Str_ncatstrTSD( Str_makeTSD(length), buffer, length ) ;
}


streng *vms_f_string( tsd_t *TSD, cparamboxptr parms )
{
   checkparam( parms, 1, 1, "VMS_F_STRING" ) ;

/*   return str_norm( TSD, parms->value ) ;   / * if it existed */
   return Str_dupTSD(parms->value) ;
}



#define DAT_TIME_LEN 23

streng *vms_f_time( tsd_t *TSD, cparamboxptr parms )
{
   int rc ;
   char buffer[32] ;
   $DESCRIPTOR( descr, buffer ) ;

   checkparam( parms, 0, 0, "VMS_F_TIME" ) ;

   rc = lib$date_time( &descr ) ;
   if (rc != SS$_NORMAL)
      vms_error( TSD, rc ) ;

   return Str_ncatstrTSD( Str_makeTSD(DAT_TIME_LEN+1), buffer, DAT_TIME_LEN) ;
}


streng *vms_f_setprv( tsd_t *TSD, cparamboxptr parms )
{
   int privbits[4], old[2] ;
   int rc ;

   checkparam( parms, 1, 1, "VMS_F_SETPRV" ) ;

   extract_privs( privbits, parms->value ) ;
   rc = sys$setprv( 0, &privbits[0], 0, &old ) ;
   if (rc != SS$_NORMAL)
      vms_error( TSD, rc ) ;

   rc = sys$setprv( 1, &privbits[2], 0, NULL ) ;
   if (rc != SS$_NORMAL)
      vms_error( TSD, rc ) ;

   return map_privs( TSD, old ) ;
}


streng *vms_f_user( tsd_t *TSD, cparamboxptr parms )
{
   int item[6], rc ;
   short length ;
   union uicdef uic ;

   checkparam( parms, 0, 0, "VMS_F_USER" ) ;

   item[0] = 4 + ( JPI$_UIC << 16 ) ;
   item[1] = (int)&uic ;
   item[2] = (int)&length ;
   item[3] = item[4] = item[5] = 0 ;

   rc = sys$getjpi( NULL, NULL, NULL, item, NULL, NULL, NULL ) ;

   if ((rc != SS$_NORMAL) || (length != 4))
   {
      vms_error( TSD, rc ) ;
      return nullstringptr() ;
   }

   return get_uic( TSD, &uic ) ;
}


streng *vms_f_locate( tsd_t *TSD, cparamboxptr parms )
{
   int res ;

   checkparam( parms, 2, 2, "VMS_F_LOCATE" ) ;
   res = bmstrstr( parms->next->value, 0, parms->value, 0 ) ;
   if (res==(-1))
      res = parms->next->value->len + 1 ;

   return int_to_streng( TSD, res ) ;
}


streng *vms_f_length( tsd_t *TSD, cparamboxptr parms )
{
   checkparam( parms, 1, 1, "VMS_F_LENGTH" ) ;
   return int_to_streng( TSD, parms->value->len ) ;
}


streng *vms_f_integer( tsd_t *TSD, cparamboxptr parms )
{
   checkparam( parms, 1, 1, "VMS_F_INTEGER" ) ;
   return int_to_streng( TSD, myatol( TSD, parms->value )) ;
}


static const struct dvi_items_type trnlnm_cases[] = {
   { 1, "CASE_BLIND",     LNM$M_CASE_BLIND   },
   { 0, "CASE_SENSITIVE", LNM$M_CASE_BLIND   },
} ;

static const struct dvi_items_type trnlnm_modes[] = {
   { 0,        "EXECUTIVE",   PSL$C_EXEC      },
   { 0,        "KERNEL",      PSL$C_KERNEL    },
   { 0,        "SUPERVISOR",  PSL$C_SUPER     },
   { 0,        "USER",        PSL$C_USER      },
} ;

static const struct dvi_items_type trnlnm_list[] = {
   { TYP_TRNM, "ACCESS_MODE", LNM$_ACMODE     },
   { TYP_FLAG, "CONCEALED",   LNM$M_CONCEALED },
   { TYP_FLAG, "CONFINE",     LNM$M_CONFINE   },
   { TYP_FLAG, "CRELOG",      LNM$M_CRELOG    },
   { TYP_INT,  "LENGTH",      LNM$_LENGTH     },
   { TYP_INT,  "MAX_INDEX",   LNM$_MAX_INDEX  },
   { TYP_FLAG, "NO_ALIAS",    LNM$M_NO_ALIAS  },
   { TYP_FLAG, "TABLE",       LNM$M_TABLE     },
   { TYP_LSTR, "TABLE_NAME",  LNM$_TABLE      },
   { TYP_FLAG, "TERMINAL",    LNM$M_TERMINAL  },
   { TYP_BSTR, "VALUE",       LNM$_STRING     },
} ;

streng *vms_f_trnlnm( tsd_t *TSD, cparamboxptr parms )
{
   char buffer[256] ;
   $DESCRIPTOR( lognam, "" ) ;
   $DESCRIPTOR( tabnam, "LNM$DCL_LOGICAL" ) ;
   short length ;
   int attr=0, item=LNM$_STRING, rc, cnt=0, index ;
   unsigned char mode ;
   int attribs, lattribs ;
   struct dvi_items_type *item_ptr ;
   cparamboxptr ptr ;
   int items[20] ;

   checkparam( parms, 1, 6, "VMS_F_TRNLNM" ) ;

   ptr = parms ;
   lognam.dsc$a_pointer = ptr->value->value ;
   lognam.dsc$w_length = ptr->value->len ;

   if (ptr) ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      tabnam.dsc$a_pointer = ptr->value->value ;
      tabnam.dsc$w_length = ptr->value->len ;
   }

   if (ptr) ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      index = atozpos( TSD, ptr->value, "VMS_F_TRNLNM", 0 ) ;
      if (index<0 || index>127)
         exiterror( ERR_INCORRECT_CALL , 0 ) ;

      items[cnt++] = 4 + ( LNM$_INDEX << 16 ) ;
      items[cnt++] = (int)&index ;
      items[cnt++] = 0 ;
   }

   if (ptr) ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      item_ptr = item_info( ptr->value, trnlnm_modes, sizeof( trnlnm_modes)) ;
      if (!item_ptr)
         exiterror( ERR_INCORRECT_CALL , 0 ) ;

      mode = item_ptr->addr ;
   }
   else
      mode = PSL$C_USER ;

   if (ptr) ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      item_ptr = item_info( ptr->value, trnlnm_cases, sizeof( trnlnm_cases)) ;
      if (!item_ptr)
         exiterror( ERR_INCORRECT_CALL , 0 ) ;
/*
 * Digital says that bit zero is used, and LNM$M_CASE_BLIND points to
 * that bit, but LNM$M_CASE_BLIND is (1<<25). My guess is that there
 * is (yet another) f*ckup in DEC's documentation, so I hardcode the
 * value.
 */
/*    attr = ( item_ptr->type << item_ptr->addr ) ; */  /* don't work */
      attr = ( item_ptr->type << 0 ) ;
   }

   if (ptr) ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      item_ptr = item_info( ptr->value, trnlnm_list, sizeof(trnlnm_list)) ;
      if (!item_ptr)
         exiterror( ERR_INCORRECT_CALL , 0 ) ;

      if (item_ptr->type == TYP_FLAG)
         item = (LNM$_ATTRIBUTES) ;
      else
         item = item_ptr->addr ;
   }
   else
      item_ptr = 0 ;

   items[cnt++] = 256 + ( item << 16 ) ;
   items[cnt++] = (int)buffer ;
   items[cnt++] = (int)&length ;

   items[cnt++] = 0 ;
   items[cnt++] = 0 ;
   items[cnt++] = 0 ;

   rc = sys$trnlnm( &attr, &tabnam, &lognam, &mode, items ) ;

   if (rc== SS$_NOLOGNAM)
      return nullstringptr() ;

   if (rc != SS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return nullstringptr() ;
   }

   if (buffer[0]==0x1b && buffer[1]==0x00 &&
       ((item_ptr && item_ptr->addr==LNM$_STRING) || (!item_ptr)))
      return format_result( TSD, TYP_LSTR, &buffer[4], length-4) ;

   if (item_ptr && item_ptr->type == TYP_TRNM)
   {
      for (cnt=0; cnt<sizeof(trnlnm_modes)/sizeof(struct dvi_items_type);cnt++)
         if (trnlnm_modes[cnt].addr == (*((unsigned char*)buffer)))
            return Str_creTSD( trnlnm_modes[cnt].name ) ;
      exiterror( ERR_SYSTEM_FAILURE , 0 ) ;
   }

   if (item_ptr && item_ptr->type == TYP_FLAG)
      return Str_creTSD( (*((int*)buffer) & item_ptr->addr) ? "TRUE" : "FALSE" ) ;

   if (item_ptr)
      return format_result( TSD, item_ptr->type, buffer, length ) ;
   else
      return format_result( TSD, TYP_BSTR, buffer, length ) ;
}



streng *vms_f_logical( tsd_t *TSD, cparamboxptr parms )
{
   checkparam( parms, 1, 1, "VMS_F_LOGICAL" ) ;
   return vms_f_trnlnm( TSD, parms ) ;
}



static const struct dvi_items_type parse_types[] = {
   { 0, "NO_CONCEAL",  NAM$M_NOCONCEAL },
   { 0, "SYNTAX_ONLY", NAM$M_SYNCHK    },
} ;


#define PARSE_EVERYTHING 0x00
#define PARSE_DEVICE     0x01
#define PARSE_DIRECTORY  0x02
#define PARSE_NAME       0x04
#define PARSE_NODE       0x08
#define PARSE_TYPE       0x10
#define PARSE_VERSION    0x20

static const struct dvi_items_type parse_fields[] = {
   { 0, "DEVICE",    PARSE_DEVICE    },
   { 0, "DIRECTORY", PARSE_DIRECTORY },
   { 0, "NAME",      PARSE_NAME      },
   { 0, "NODE",      PARSE_NODE      },
   { 0, "TYPE",      PARSE_TYPE      },
   { 0, "VERSION",   PARSE_VERSION   },
} ;



streng *vms_f_parse( tsd_t *TSD, cparamboxptr parms )
{
   char relb[256], expb[256], relb2[256], expb2[256] ;
   int clen, rc, fields ;
   char *cptr ;
   struct dvi_items_type *item ;
   cparamboxptr ptr ;
   streng *result ;
   struct FAB fab, relfab ;
   struct NAM nam, relnam ;

   checkparam( parms, 1, 5, "VMS_F_PARSE" ) ;
   ptr = parms ;

   memcpy( &fab, &cc$rms_fab, sizeof(struct FAB)) ;
   memcpy( &nam, &cc$rms_nam, sizeof(struct NAM)) ;

   fab.fab$l_fna = ptr->value->value ;
   fab.fab$b_fns = ptr->value->len ;

/*
   nam.nam$l_rsa = buffer ;
   nam.nam$b_rss = sizeof(buffer)-1 ;
 */
   nam.nam$l_esa = expb ;
   nam.nam$b_ess = sizeof(expb)-1 ;

   fab.fab$w_ifi = 0 ;
   fab.fab$l_fop &= ~(FAB$M_OFP) ;
   fab.fab$l_nam = &nam ;

   ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      fab.fab$l_dna = ptr->value->value ;
      fab.fab$b_dns = ptr->value->len ;
   }

   if (ptr) ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      memcpy( &relfab, &cc$rms_fab, sizeof(struct FAB)) ;
      memcpy( &relnam, &cc$rms_nam, sizeof(struct NAM)) ;
      relnam.nam$l_rsa = ptr->value->value ;
      relnam.nam$b_rsl = ptr->value->len ;
      relnam.nam$b_rss = ptr->value->len ;

      nam.nam$l_rlf = &relnam ;
   }

   if (ptr) ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      item = item_info( ptr->value, parse_fields, sizeof(parse_fields)) ;
      if (!item)
         exiterror( ERR_INCORRECT_CALL , 0 ) ;
      fields = item->addr ;
   }
   else
      fields = PARSE_EVERYTHING ;

   if (ptr) ptr=ptr->next ;
   if (ptr && ptr->value)
   {
      item = item_info( ptr->value, parse_types, sizeof(parse_types)) ;
      if (!item)
         exiterror( ERR_INCORRECT_CALL , 0 ) ;
      nam.nam$b_nop |= item->addr ;
   }

   rc = sys$parse( &fab, NULL, NULL ) ;

   if ((rc==RMS$_SYN) || (rc==RMS$_DEV) || (rc==RMS$_DNF) || (rc==RMS$_DIR) ||
       (rc==RMS$_NOD))
      return nullstringptr() ;

   if (rc != RMS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return nullstringptr() ;
   }

   switch( fields )
   {
      case PARSE_EVERYTHING:
         cptr = nam.nam$l_esa ;  clen = nam.nam$b_esl ;  break ;

      case PARSE_DEVICE:
         cptr = nam.nam$l_dev ;  clen = nam.nam$b_dev ;  break ;

      case PARSE_DIRECTORY:
         cptr = nam.nam$l_dir ;  clen = nam.nam$b_dir ;  break ;

      case PARSE_NAME:
         cptr = nam.nam$l_name ; clen = nam.nam$b_name ; break ;

      case PARSE_NODE:
         cptr = nam.nam$l_node ; clen = nam.nam$b_node ; break ;

      case PARSE_TYPE:
         cptr = nam.nam$l_type ; clen = nam.nam$b_type ; break ;

      case PARSE_VERSION:
         cptr = nam.nam$l_ver ;  clen = nam.nam$b_ver ;  break ;

      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
   }

   result = Str_makeTSD( clen ) ;
   memcpy( result->value, cptr, result->len=clen ) ;

   return result ;
}



streng *vms_f_search( tsd_t *TSD, cparamboxptr parms )
{
   streng *name, *result ;
   int context, rc, search ;
   struct fabptr *fptr ;
   vmf_tsd_t *vt;

   vt = TSD->vmf_tsd;
   checkparam( parms, 1, 2, "VMS_F_SEARCH" ) ;

   name = parms->value ;
   context = (parms->next && parms->next->value) ?
                      atopos(TSD, parms->next->value, "VMS_F_SEARCH", 2 ) : 0 ;

   search = (context/16) ;
   for (fptr=vt->fabptrs[search]; fptr && fptr->num!=context; fptr=fptr->next) ;
   if (!fptr)
   {

      fptr = MallocTSD( sizeof(struct fabptr)) ;
      fptr->num = context ;
      fptr->next = vt->fabptrs[search] ;
      vt->fabptrs[search] = fptr ;
      fptr->box = MallocTSD( sizeof(struct FAB)) ;
      memcpy( fptr->box, &cc$rms_fab, sizeof(struct FAB)) ;
      fptr->box->fab$l_nam = MallocTSD( sizeof(struct NAM)) ;
      memcpy( fptr->box->fab$l_nam, &cc$rms_nam, sizeof(struct NAM)) ;
      fptr->box->fab$l_nam->nam$l_esa = MallocTSD( 256 ) ;
      fptr->box->fab$l_nam->nam$b_ess = 255 ;
      fptr->box->fab$l_nam->nam$l_rsa = MallocTSD( 256 ) ;
      fptr->box->fab$l_nam->nam$b_rss = 255 ;
      fptr->box->fab$l_nam->nam$b_rsl = 0 ;
      fptr->box->fab$l_fna = NULL ;
      fptr->box->fab$b_fns = 0 ;
   }

   if (context==0 && ((name->len!=fptr->box->fab$b_fns) ||
                      memcmp(name->value, fptr->box->fab$l_fna, name->len )))
      fptr->box->fab$l_nam->nam$b_rsl = 0 ;

   if (fptr->box->fab$l_nam->nam$b_rsl == 0)
   {
/*      fptr->box->fab$l_dna = NULL ;
      fptr->box->fab$b_dns = 0 ; */
      fptr->name = Str_dupTSD( name ) ;
      fptr->box->fab$l_fna = fptr->name->value ;
      fptr->box->fab$b_fns = fptr->name->len ;
/*      fptr->box->fab$l_fop |= FAB$M_OFP ; */
      fptr->box->fab$w_ifi = 0 ;
/*      fptr->box->fab$l_nam->nam$b_nop = NAM$M_PWD ;
      fptr->box->fab$l_nam->nam$l_rlf = NULL ; */

      rc = sys$parse( fptr->box, NULL, NULL ) ;

      if (rc != RMS$_NORMAL)
      {
         vms_error( TSD, rc ) ;
         return nullstringptr() ;
      }
   }

   rc = sys$search( fptr->box, NULL, NULL ) ;
   if (rc == RMS$_NMF)
      return nullstringptr() ;

   if (rc != RMS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return nullstringptr() ;
   }

   result = Str_makeTSD( fptr->box->fab$l_nam->nam$b_rsl ) ;
   result->len = fptr->box->fab$l_nam->nam$b_rsl ;
   memcpy( result->value, fptr->box->fab$l_nam->nam$l_rsa, result->len ) ;

   return result ;
}



streng *vms_f_type( tsd_t *TSD, cparamboxptr parms )
{
   checkparam( parms, 1, 1, "VMS_F_TYPE" ) ;
   return Str_creTSD(myisinteger( parms->value ) ? "INTEGER" : "STRING" ) ;
}


static streng *boolean( const tsd_t *TSD, const int param )
{
   return Str_creTSD( param ? "TRUE" : "FALSE" ) ;
}

#define FIL_ALQ 1
#define FIL_BDT 2
#define FIL_BKS 3
#define FIL_BLS 4
#define FIL_CBT 5
#define FIL_CDT 6
#define FIL_CTG 7
#define FIL_DEQ 8
#define FIL_DID 9
#define FIL_DVI 10
#define FIL_EDT 11
#define FIL_EOF 12
#define FIL_FID 13
#define FIL_FSZ 14
#define FIL_GRP 15
#define FIL_KNOWN 16
#define FIL_MBM 17
#define FIL_MRN 101
#define FIL_MRS 18
#define FIL_NOA 19
#define FIL_NOK 20
#define FIL_ORG 21
#define FIL_PRO 22
#define FIL_PVN 23
#define FIL_RAT 24
#define FIL_RCK 25
#define FIL_RDT 26
#define FIL_RFM 27
#define FIL_RVN 28
#define FIL_UIC 29
#define FIL_WCK 30


static const struct dvi_items_type file_attribs[] = {
   { TYP_INT,  "ALQ",   FIL_ALQ },
   { TYP_INT,  "BDT",   FIL_BDT },
   { TYP_INT,  "BKS",   FIL_BKS },
   { TYP_INT,  "BLS",   FIL_BLS },
   { TYP_INT,  "CBT",   FIL_CBT },
   { TYP_INT,  "CDT",   FIL_CDT },
   { TYP_INT,  "CTG",   FIL_CTG },
   { TYP_INT,  "DEQ",   FIL_DEQ },
   { TYP_INT,  "DID",   FIL_DID },
   { TYP_INT,  "DVI",   FIL_DVI },
   { TYP_INT,  "EDT",   FIL_EDT },
   { TYP_INT,  "EOF",   FIL_EOF },
   { TYP_INT,  "FID",   FIL_FID },
   { TYP_INT,  "FSZ",   FIL_FSZ },
   { TYP_INT,  "GRP",   FIL_GRP },
   { TYP_INT,  "KNOWN", FIL_KNOWN },
   { TYP_INT,  "MBM",   FIL_MBM   },
   { TYP_INT,  "MRN",   FIL_MRN   },
   { TYP_INT,  "MRS",   FIL_MRS   },
   { TYP_INT,  "NOA",   FIL_NOA   },
   { TYP_INT,  "NOK",   FIL_NOK   },
   { TYP_INT,  "ORG",   FIL_ORG   },
   { TYP_INT,  "PRO",   FIL_PRO   },
   { TYP_INT,  "PVN",   FIL_PVN   },
   { TYP_INT,  "RAT",   FIL_RAT   },
   { TYP_INT,  "RCK",   FIL_RCK   },
   { TYP_INT,  "RDT",   FIL_RDT   },
   { TYP_INT,  "RFM",   FIL_RFM   },
   { TYP_INT,  "RVN",   FIL_RVN   },
   { TYP_INT,  "UIC",   FIL_UIC   },
   { TYP_INT,  "WCK",   FIL_WCK   },
} ;

streng *vms_f_file_attributes( tsd_t *TSD, cparamboxptr parms )
{
   struct dvi_items_type *item ;
   int rc, rc2, tmp ;
   char temp_space[256] ;
   streng *res ;
   struct FAB fab ;
   struct NAM nam ;
   struct XABALL xaball ;
   struct XABDAT xabdat ;
   struct XABPRO xabpro ;
   struct XABSUM xabsum ;
   struct XABFHC xabfhc ;

   checkparam( parms, 2, 2, "VMS_F_FILE_ATTRIBUTES" ) ;
   item = item_info( parms->next->value, file_attribs, sizeof(file_attribs)) ;

   memcpy( &fab, &cc$rms_fab, sizeof( struct FAB )) ;
   memcpy( &nam, &cc$rms_nam, sizeof( struct NAM )) ;
   memcpy( &xaball, &cc$rms_xaball, sizeof( struct XABALL )) ;
   memcpy( &xabdat, &cc$rms_xabdat, sizeof( struct XABDAT )) ;
   memcpy( &xabpro, &cc$rms_xabpro, sizeof( struct XABPRO )) ;
   memcpy( &xabsum, &cc$rms_xabsum, sizeof( struct XABSUM )) ;
   memcpy( &xabfhc, &cc$rms_xabfhc ,sizeof( struct XABFHC )) ;

   fab.fab$l_fna = parms->value->value ;
   fab.fab$b_fns = parms->value->len ;
   fab.fab$l_nam = &nam ;

   fab.fab$l_xab = &xabdat ;
   xabdat.xab$l_nxt = &xabpro ;
   xabpro.xab$l_nxt = &xabsum ;
   xabsum.xab$l_nxt = &xabfhc ;
/*   xaball.xab$l_next = &xabdat ; */

   if (item->addr==FIL_KNOWN)
   {
      /* This field is undocumented in 'The Grey Wall', I spent quite
       * some time trying to find this ... sigh. Also note that the
       * return code RMS$_KFF is an Digital internal code.
       */
      fab.fab$l_fop |= FAB$M_KFO ;
      fab.fab$l_ctx = 0 ;

      nam.nam$b_nop |= NAM$M_NOCONCEAL ;
      nam.nam$l_esa = temp_space ;
      nam.nam$b_ess = 255 ;
   }

   rc = sys$open( &fab, NULL, NULL ) ;

   if (item->addr==FIL_KNOWN)
   {
      if (rc==RMS$_NORMAL || rc==RMS$_KFF)
      {
         /* OK, we ought to check the rc from sys$close() ... */
         sys$close( &fab, NULL, NULL ) ;
         return Str_creTSD( (fab.fab$l_ctx) ? "TRUE" : "FALSE" ) ;
      }
   }
   if (rc != RMS$_FNF)
   {
      if (rc != RMS$_NORMAL)
      {
         vms_error( TSD, rc ) ;
         return nullstringptr() ;
      }
   }
   else
      return nullstringptr() ;

#define fr(a,b,c) format_result(TSD,a,b,c)
   switch (item->addr)
   {
      case FIL_ALQ:  res = int_to_streng( TSD, fab.fab$l_alq ); break ;
      case FIL_BDT:  res = fr( TYP_TIME, (const char *)xabdat.xab$q_bdt, 8 ); break ;
      case FIL_BKS:  res = int_to_streng( TSD, fab.fab$b_bks ); break ;
      case FIL_BLS:  res = int_to_streng( TSD, fab.fab$w_bls ); break ;
      case FIL_CBT:  res = boolean( TSD, fab.fab$l_fop & FAB$M_CBT ); break ;
      case FIL_CDT:  res = fr( TYP_TIME, (const char *)xabdat.xab$q_cdt, 8 ); break ;
      case FIL_CTG:  res = boolean( TSD, fab.fab$l_fop & FAB$M_CTG ); break ;
      case FIL_DEQ:  res = int_to_streng( TSD,    fab.fab$w_deq ); break ;
      case FIL_DID:  res = internal_id( TSD, (const short *)nam.nam$w_did ); break ;
      case FIL_DVI:
        res = Str_makeTSD( nam.nam$t_dvi[0] ) ;
        memcpy( res->value, &(nam.nam$t_dvi[1]), res->len=nam.nam$t_dvi[0] ) ;
        break ;
      case FIL_EDT:  res = fr( TYP_TIME, (const char *)xabdat.xab$q_edt, 8 ); break ;
      case FIL_EOF:
         res = int_to_streng( TSD, xabfhc.xab$l_ebk - (xabfhc.xab$w_ffb==0));
         break ;
      case FIL_FID:  res = internal_id( TSD, (const short *)nam.nam$w_fid ); break ;
      case FIL_FSZ:  res = int_to_streng( TSD,    fab.fab$b_fsz ); break ;
      case FIL_KNOWN: res = nullstringptr() ; /* must be nonexistent */
         break ;
      case FIL_GRP:  res = int_to_streng( TSD, xabpro.xab$w_grp ); break ;
      case FIL_MBM:  res = int_to_streng( TSD, xabpro.xab$w_mbm ); break ;
      case FIL_MRN:  res = int_to_streng( TSD,    fab.fab$l_mrn ); break ;
      case FIL_MRS:  res = int_to_streng( TSD,    fab.fab$w_mrs ); break ;
      case FIL_NOA:  res = int_to_streng( TSD, xabsum.xab$b_noa ); break ;
      case FIL_NOK:  res = int_to_streng( TSD, xabsum.xab$b_nok ); break ;
      case FIL_ORG:
         switch (xabfhc.xab$b_rfo & 48 )   /* magic number! */
         {
            case FAB$C_IDX: res = Str_creTSD( "IDX" ) ; break ;
            case FAB$C_REL: res = Str_creTSD( "REL" ) ; break ;
            case FAB$C_SEQ: res = Str_creTSD( "SEQ" ) ; break ;
            default: exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         }
         break ;
      case FIL_PRO:  res = get_prot( TSD, tmp=xabpro.xab$w_pro ); break ;
      case FIL_PVN:  res = int_to_streng( TSD, xabsum.xab$w_pvn ); break ;
      case FIL_RAT:
         if (fab.fab$b_rat & FAB$M_BLK)
            res = Str_creTSD( "" ) ;
         else if (fab.fab$b_rat & FAB$M_CR)
            res = Str_creTSD( "CR" ) ;
         else if (fab.fab$b_rat & FAB$M_FTN)
            res = Str_creTSD( "FTN" ) ;
         else if (fab.fab$b_rat & FAB$M_PRN)
            res = Str_creTSD( "PRN" ) ;
         else
            res = nullstringptr() ;
         break ;
      case FIL_RCK:  res = boolean( TSD, fab.fab$l_fop & FAB$M_RCK ); break ;
      case FIL_RDT:  res = fr( TYP_TIME, (const char *)xabdat.xab$q_rdt, 8 ); break ;
/*      case FIL_RDT:  res = fr( TYP_TIME, &(xabdat.xab$q_rdt), 8 ); break ; */
      case FIL_RFM:
         switch (xabfhc.xab$b_rfo & 15 ) /* magic number! */
         {
            case FAB$C_VAR: res = Str_creTSD( "VAR" ) ; break ;
            case FAB$C_FIX: res = Str_creTSD( "FIX" ) ; break ;
            case FAB$C_VFC: res = Str_creTSD( "VFC" ) ; break ;
            case FAB$C_UDF: res = Str_creTSD( "UDF" ) ; break ;
            case FAB$C_STM: res = Str_creTSD( "STM" ) ; break ;
            case FAB$C_STMLF: res = Str_creTSD( "STMLF" ) ; break ;
            case FAB$C_STMCR: res = Str_creTSD( "STMCR" ) ; break ;
            default: exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
         }
         break ;
      case FIL_RVN:  res = int_to_streng( TSD, xabdat.xab$w_rvn ); break ;
      case FIL_UIC:  res = get_uic( TSD, ( union uicdef *)&(xabpro.xab$l_uic) ); break ;
      case FIL_WCK:  res = boolean( TSD, fab.fab$l_fop & FAB$M_WCK ); break ;
      default:
         exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
   }

   if (rc == RMS$_NORMAL)
   {
      rc = sys$close( &fab, NULL, NULL ) ;
      if (rc != RMS$_NORMAL )
      {
         vms_error( TSD, rc ) ;
         return nullstringptr() ;
      }
   }
   return res ;
}


streng *vms_f_extract( tsd_t *TSD, cparamboxptr parms )
{
   int start, length ;
   streng *result, *string ;

   checkparam( parms, 3, 3, "VMS_F_EXTRACT" ) ;
   start = atozpos( TSD, parms->value, "VMS_F_EXTRACT", 1 ) ;
   length = atozpos( TSD, (parms=parms->next)->value, "VMS_F_EXTRACT", 2 ) ;
   string = parms->next->value ;

   if (start>string->len)
      start = string->len ;

   if (length > string->len - start)
      length = (string->len - start) ;

   result = Str_makeTSD( length ) ;
   memcpy( result->value, string->value+start, length ) ;
   result->len = length ;

   return result ;
}

streng *vms_f_element( tsd_t *TSD, cparamboxptr parms )
{
   int number, count ;
   streng *string, *result ;
   char delim, *cptr, *cend, *cmax ;

   checkparam( parms, 3, 3, "VMS_F_ELEMENT" ) ;

   number = atozpos( TSD, parms->value, "VMS_F_ELEMENT", 1 ) ;
   delim = getonechar( TSD, (parms=parms->next)->value, "VMS_F_ELEMENT", 2) ;
   string = parms->next->value ;

   cptr = string->value ;
   cend = cptr + string->len ;
   for (count=0;count<number && cptr<cend;)
      if (*(cptr++)==delim) count++ ;

   if (count<number)
   {
      result = Str_makeTSD( 1 ) ;
      result->len = 1 ;
      result->value[0] = delim ;
   }
   else
   {
      for (cmax=cptr; *cmax!=delim && cmax<cend; cmax++) ;
      result = Str_makeTSD( cmax - cptr ) ;
      result->len = cmax - cptr ;
      memcpy( result->value, cptr, cmax-cptr ) ;
   }

   return result ;
}


static streng *convert_bin( tsd_t *TSD, cparamboxptr parms, const int issigned, const char *bif )
{
   int start, length, obyte, obit, count, bit=0 ;
   streng *string, *result, *temp ;

   checkparam( parms, 3, 3, bif ) ;

   start = atozpos( TSD, parms->value, bif, 1 ) ;
   length = atozpos( TSD, parms->next->value, bif, 2 ) ;
   string = parms->next->next->value ;

   if (issigned)
   {
      start++ ;
      length-- ;
   }

   if ((start+length > string->len*8) || length<0)
      exiterror( ERR_INCORRECT_CALL , 0 ) ;

   temp = Str_makeTSD((start+length)/8 + 2) ;
   obyte = (start+length)/8 + 1 ;
   temp->len = obyte + 1 ;
   obit = 7 ;
   for (count=0; count<=obyte; temp->value[count++] = 0x00) ;

   for (count=start+length-1; count>=start; count--)
   {
      bit = (string->value[count/8] >> (7-(count%8))) & 1 ;
      temp->value[obyte] |= bit << (7-obit--) ;
      if (obit<0)
      {
         obit = 7 ;
         obyte-- ;
      }
   }

   if (issigned)
      bit = (string->value[count/8] >> (7-(count%8))) & 1 ;

   if (issigned && bit)
      for (;obyte>=0;)
      {
         temp->value[obyte] |= 1 << (7-obit--) ;
         if (obit<0)
         {
            obit = 7 ;
            obyte-- ;
         }
      }

   result = str_digitize( TSD, temp, 0, 1 ) ;
   FreeTSD( temp ) ;
   return result ;
}



streng *vms_f_cvui( tsd_t *TSD, cparamboxptr parms )
{
   return convert_bin( TSD, parms, 0, "VMS_F_CVUI" ) ;
}

streng *vms_f_cvsi( tsd_t *TSD, cparamboxptr parms )
{
   return convert_bin( TSD, parms, 1, "VMS_F_CVSI" ) ;
}


static const char *vms_weekdays[] = { "Monday", "Tuesday", "Wednesday",
                                      "Thursday", "Friday", "Saturday",
                                      "Sunday" } ;
static const char *vms_months[] = { "", "JAN", "FEB", "MAR", "APR", "MAY",
                                        "JUN", "JUL", "AUG", "SEP", "OCT",
                                        "NOV", "DEC" } ;

enum outs { absolute, comparison, delta } ;
enum funcs { year, month, day, hour, minute, second, hundredth,
               weekday, time_part, date_part, datetime } ;


static char *read_abs_time( char *ptr, char *end, short *times )
{
   int cnt, increment, rc ;
   char *tmp ;

   rc = sys$numtim( times, NULL ) ;

   if (ptr>=end) exiterror( ERR_INCORRECT_CALL, 0 ) ;
   if (*ptr=='-')
   {
      ptr++ ;
      goto abs_hours ;
   }

   if (*ptr=='+')
      return ptr ;

   if (isspace(*ptr))
      return ptr ;

   if (*ptr==':')
   {
      ptr++ ;
      goto abs_minutes ;
   }

   if (!isdigit(*ptr))
   {
      if (ptr+3>=end ) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      for (cnt=1; cnt<=12; cnt++)
         if (!memcmp(ptr,vms_months[cnt],3))
         {
            ptr += 3 ;
            times[month] = cnt ;
            if (ptr>=end)
               return ptr ;
            else if (*ptr==':')
            {
               ptr++ ;
               goto abs_hours ;
            }
            else if (*ptr=='-')
            {
               ptr++ ;
               goto abs_years ;
            }
            else
               return ptr ;
         }
      exiterror( ERR_INCORRECT_CALL , 0 ) ;
   }
   else
   {
      for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      if (ptr>=end || isspace(*ptr) || *ptr==':')
      {
         if (ptr<end && *ptr==':') ptr++ ;
         if (cnt>23) exiterror( ERR_INCORRECT_CALL, 0 ) ;
         times[hour] = cnt ;
         goto abs_minutes ;
      }
      else if (*ptr=='-')
      {
         ptr++ ;
         times[day] = cnt ;
         goto abs_months ;
      }
      else
        return ptr ;
   }


   abs_months:
   if (ptr<end && isalpha(*ptr))
   {
      if (ptr+3>=end) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      for (cnt=1; cnt<=12; cnt++)
         if (!memcmp(ptr,vms_months[cnt],3))
         {
            ptr += 3 ;
            times[month] = cnt ;

            if (ptr>=end)
               return ptr ;
            else if (*ptr==':')
            {
               ptr++ ;
               goto abs_hours ;
            }
            else if (*ptr=='-')
            {
               ptr++ ;
               goto abs_years ;
            }
            else
               return ptr ;
         }
      exiterror( ERR_INCORRECT_CALL , 0 ) ;
   }
   else if (ptr>=end || isspace(*ptr))
      return ptr ;
   else if (*ptr=='-')
   {
      ptr++ ;
      goto abs_years ;
   }
   else if (*ptr==':')
   {
      ptr++ ;
      goto abs_hours ;
   }
   else
      exiterror( ERR_INCORRECT_CALL , 0 ) ;


   abs_years:
   if (ptr<end && isdigit(*ptr))
   {
      for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      if (cnt>9999) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[year] = cnt ;
      if (ptr<end && *ptr==':')
      {
         ptr++ ;
         goto abs_hours ;
      }
      else
         return ptr ;
   }
   else if (ptr<end && *ptr==':')
   {
      ptr++ ;
      goto abs_hours ;
   }
   else
      return ptr ;


   abs_hours:
   if (ptr<end && isdigit(*ptr))
   {
      for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      if (cnt>23) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[hour] = cnt ;
      if (ptr<end && *ptr==':')
      {
         ptr++ ;
         goto abs_minutes ;
      }
      else
         return ptr ;
   }
   else if (ptr<end && *ptr==':')
   {
      ptr++ ;
      goto abs_minutes ;
   }
   else
      return ptr ;


   abs_minutes:
   if (ptr<end && isdigit(*ptr))
   {
      for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      if (cnt>59) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[minute] = cnt ;
      if (ptr<end && *ptr==':')
      {
         ptr++ ;
         goto abs_seconds ;
      }
      else
         return ptr ;
   }
   else if (ptr<end && *ptr==':')
   {
      ptr++ ;
      goto abs_seconds ;
   }
   else
      return ptr ;


   abs_seconds:
   if (ptr<end && isdigit(*ptr))
   {
      for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      if (cnt>59) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[second] = cnt ;
      if (ptr<end && *ptr=='.')
      {
         ptr++ ;
         goto abs_hundredths ;
      }
      else
         return ptr ;
   }
   else if (ptr<end && *ptr=='.')
   {
      ptr++ ;
      goto abs_hundredths ;
   }
   else
      return ptr ;


   abs_hundredths:
   if (ptr<end && isdigit(*ptr))
   {
      tmp = ptr ;
      for (cnt=0; ptr<end && ptr<tmp+2 && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      increment = (ptr<end && isdigit(*ptr) && (*ptr-'0'>=5)) ;
      for (;ptr<end && isdigit(*ptr); ptr++) ;
      times[hundredth] = cnt + increment ;
      return ptr ;
   }
   else
      return ptr ;
}


static char *read_delta_time( char *ptr, char *end, short *times )
{
   int cnt, increment ;
   char *tmp ;

   for (cnt=0; cnt<7; times[cnt++]=0) ;

   if (ptr>=end) exiterror( ERR_INCORRECT_CALL, 0 ) ;
   if (*ptr=='-')
   {
      ptr++ ;
      goto delta_hours ;
   }

   if (*ptr==':')
   {
      ptr++ ;
      goto delta_minutes ;
   }

   if (!isdigit( *ptr )) exiterror( ERR_INCORRECT_CALL, 0 ) ;
   for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
      cnt = cnt*10 + *ptr-'0' ;

   if (ptr>=end || isspace(*ptr) || *ptr==':')
   {
      if (ptr<end && *ptr==':') ptr++ ;
      if (cnt>23) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[hour] = cnt ;
      goto delta_minutes ;
   }
   else
   {
      if (*ptr!='-') exiterror( ERR_INCORRECT_CALL, 0 ) ;
      ptr++ ;
      if (cnt>9999) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[day] = cnt ;
      goto delta_hours ;
   }

   delta_hours:
   if (ptr<end && isdigit(*ptr))
   {
      for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      if (cnt>23) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[hour] = cnt ;
      if (ptr<end && *ptr==':')
      {
         ptr++ ;
         goto delta_minutes ;
      }
      else
         return ptr ;
   }
   else if (ptr<end && *ptr==':')
   {
      ptr++ ;
      goto delta_minutes ;
   }
   else
      return ptr ;


   delta_minutes:
   if (ptr<end && isdigit(*ptr))
   {
      for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      if (cnt>59) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[minute] = cnt ;
      if (ptr<end && *ptr==':')
      {
         ptr++ ;
         goto delta_seconds ;
      }
      else
         return ptr ;
   }
   else if (ptr<end && *ptr==':')
   {
      ptr++ ;
      goto delta_seconds ;
   }
   else
      return ptr ;


   delta_seconds:
   if (ptr<end && isdigit(*ptr))
   {
      for (cnt=0; ptr<end && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      if (cnt>59) exiterror( ERR_INCORRECT_CALL, 0 ) ;
      times[second] = cnt ;
      if (ptr<end && *ptr=='.')
      {
         ptr++ ;
         goto delta_hundredths ;
      }
      else
         return ptr ;
   }
   else if (ptr<end && *ptr=='.')
   {
      ptr++ ;
      goto delta_hundredths ;
   }
   else
      return ptr ;


   delta_hundredths:
   if (ptr<end && isdigit(*ptr))
   {
      tmp = ptr ;
      for (cnt=0; ptr<end && ptr<tmp+2 && isdigit(*ptr); ptr++)
         cnt = cnt*10 + *ptr-'0' ;

      increment = (ptr<end && isdigit(*ptr) && (*ptr-'0'>=5)) ;
      for (;ptr<end && isdigit(*ptr); ptr++) ;
      times[hundredth] = cnt + increment ;
      return ptr ;
   }
   else
      return ptr ;
}


streng *vms_f_cvtime( tsd_t *TSD, cparamboxptr parms )
{
   streng *item=NULL, *input=NULL, *output=NULL, *result ;
   int rc, res, cnt, abs=0 ;
   short times[7], timearray[7], btime[4] ;
   char *cptr, *cend, *ctmp, *cptr2 ;
   $DESCRIPTOR( timbuf, "" ) ;
   enum funcs func ;
   enum outs out ;

   checkparam( parms, 0, 3, "VMS_F_CVTIME" ) ;
   func = datetime ;
   out = comparison ;

   input = parms->value ;
   if (parms->next)
   {
      output = parms->next->value ;
      if (parms->next->next)
         item = parms->next->next->value ;
   }

   if (item)
   {
      for (cnt=0; cnt<item->len; cnt++)
         item->value[cnt] = toupper(item->value[cnt]) ;

      if (item->len==4 && !memcmp(item->value, "YEAR", 4))
         func = year ;
      else if (item->len==5 && !memcmp(item->value, "MONTH", 5))
         func = month ;
      else if (item->len==8 && !memcmp(item->value, "DATETIME", 8))
         func = datetime ;
      else if (item->len==3 && !memcmp(item->value, "DAY", 3))
         func = day ;
      else if (item->len==4 && !memcmp(item->value, "DATE", 4))
         func = date_part ;
      else if (item->len==4 && !memcmp(item->value, "TIME", 4))
         func = time_part ;
      else if (item->len==4 && !memcmp(item->value, "HOUR", 4))
         func = hour ;
      else if (item->len==6 && !memcmp(item->value, "SECOND", 6))
         func = second ;
      else if (item->len==6 && !memcmp(item->value, "MINUTE", 6))
         func = minute ;
      else if (item->len==9 && !memcmp(item->value, "HUNDREDTH", 9))
         func = hundredth ;
      else if (item->len==7 && !memcmp(item->value, "WEEKDAY", 7))
         func = weekday ;
      else
         exiterror( ERR_INCORRECT_CALL , 0 ) ;
   }

   if (output)
   {
      for (cnt=0; cnt<output->len; cnt++)
         output->value[cnt] = toupper(output->value[cnt]) ;

      if (output->len==5 && !memcmp(output->value, "DELTA", 5))
         out = delta ;
      else if (output->len==10 && !memcmp(output->value, "COMPARISON", 10))
         abs = 0 ;
      else if (output->len==8 && !memcmp(output->value, "ABSOLUTE", 8))
         abs = 1 ;
      else
         exiterror( ERR_INCORRECT_CALL , 0 ) ;
   }

   if (out==delta)
      if (func==year  || func==month || func==weekday)
         exiterror( ERR_INCORRECT_CALL , 0 ) ;

   if (input)
   {
      short atime[4], dtime[4], xtime[4], ttimes[7] = {0,0,0,0,0,0,1} ;
      int rc2, increment ;

      lib$cvt_vectim( ttimes, xtime ) ;
      cptr = input->value ;
      cend = cptr + input->len ;

      for (ctmp=cptr;ctmp<cend;ctmp++)
         *ctmp = toupper(*ctmp) ;

      for (;isspace(*cptr);cptr++) ; /* strip leading spaces */
      if (out!=delta)
      {
         if (cptr<cend && *cptr!='-')
         {
            cptr = read_abs_time( cptr, cend, times ) ;
            if ((increment=(times[hundredth]==100)))
               times[hundredth] -= 1 ; ;

            rc = lib$cvt_vectim( times, btime ) ;
            if (increment)
            {
               lib$add_times( xtime, btime, dtime ) ;
               btime[0] = dtime[0] ;
               btime[1] = dtime[1] ;
               btime[2] = dtime[2] ;
               btime[3] = dtime[3] ;
            }
         }
         else
         {
            rc = sys$gettim( btime ) ;
         }

         if (cptr<cend && (*cptr=='-' || *cptr=='+'))
         {
            char oper = *cptr ;
            cptr2 = read_delta_time( ++cptr, cend, times ) ;
            if ((increment=(times[6]==100)))
               times[6] -= 1 ;

            rc2 = lib$cvt_vectim( times, dtime ) ;
            if (increment)
            {
               lib$add_times( dtime, xtime, atime ) ;
               dtime[0] = atime[0] ;
               dtime[1] = atime[1] ;
               dtime[2] = atime[2] ;
               dtime[3] = atime[3] ;
            }

            if (oper=='+')
               rc = lib$add_times( btime, dtime, atime ) ;
            else
               rc = lib$sub_times( btime, dtime, atime ) ;

            btime[0] = atime[0] ;
            btime[1] = atime[1] ;
            btime[2] = atime[2] ;
            btime[3] = atime[3] ;
         }
      }
      else
      {
         cptr = read_delta_time( cptr, cend, times ) ;
         if ((increment=(times[6]==100)))
            times[6] -= 1 ;

         rc = lib$cvt_vectim( times, &btime ) ;
         if (increment)
         {
            lib$add_times( xtime, btime, atime ) ;
            btime[0] = atime[0] ;
            btime[1] = atime[1] ;
            btime[2] = atime[2] ;
            btime[3] = atime[3] ;
         }
      }
   }
   else
      rc = sys$gettim( &btime ) ;

   if (rc!=SS$_NORMAL && rc!=LIB$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return nullstringptr() ;
   }

   rc = sys$numtim( timearray, &btime ) ;
   if (rc!=SS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
      return nullstringptr() ;
   }

   switch (func)
   {
      case year:
         result = Str_makeTSD( 5 ) ;
         sprintf( result->value, ((abs) ? "%04d" : "%d"), timearray[func]);
         result->len = strlen( result->value ) ;
         break ;

      case hour:
      case minute:
      case second:
      case hundredth:
         abs = 0 ;
      case day:
         result = Str_makeTSD( 3 ) ;
         sprintf( result->value, ((abs) ? "%d" : "%02d"), timearray[func]);
         result->len = strlen( result->value ) ;
         break ;

      case month:
         if (abs)
            result = Str_creTSD( vms_months[ func ]) ;
         else
         {
            result = Str_makeTSD( 3 ) ;
            sprintf( result->value, "%02d", timearray[month]) ;
            result->len = 2 ;
         }
         break ;

      case time_part:
         result = Str_makeTSD( 12 ) ;
         sprintf(result->value, "%02d:%02d:%02d.%02d", timearray[hour],
              timearray[minute], timearray[second], timearray[hundredth]) ;
         result->len = 11 ;
         break ;

      case date_part:
         result = Str_makeTSD( 12 ) ;
         if (out==delta)
            sprintf( result->value, "%d", timearray[day] ) ;
         else if (abs)
            sprintf( result->value, "%d-%s-%d", timearray[day],
                vms_months[timearray[month]], timearray[year] ) ;
         else
            sprintf( result->value, "%04d-%02d-%02d", timearray[year],
                timearray[month], timearray[day] ) ;

         result->len = strlen( result->value ) ;
         break ;

      case datetime:
         result = Str_makeTSD( 24 ) ;
         if (out==delta)
            sprintf( result->value, "%d %02d:%02d:%02d.%02d",
                   timearray[day], timearray[hour], timearray[minute],
                   timearray[second], timearray[hundredth] ) ;
         else if (abs)
            sprintf( result->value, "%d-%s-%d %02d:%02d:%02d.%02d",
                   timearray[day], vms_months[timearray[month]],
                   timearray[year], timearray[hour], timearray[minute],
                   timearray[second], timearray[hundredth] ) ;
         else
            sprintf( result->value, "%04d-%02d-%02d %02d:%02d:%02d.%02d",
                   timearray[year], timearray[month], timearray[day],
                   timearray[hour], timearray[minute], timearray[second],
                   timearray[hundredth] ) ;
         result->len = strlen( result->value ) ;
         break ;

      case weekday:
      {
         int op=LIB$K_DAY_OF_WEEK, res ;
         rc = lib$cvt_from_internal_time( &op, &res, &btime ) ;
         if (rc!=LIB$_NORMAL)
         {
            vms_error( TSD, rc ) ;
            return nullstringptr() ;
         }
         result = Str_creTSD( vms_weekdays[res-1] ) ;
         break ;
      }

      default: exiterror( ERR_INTERPRETER_FAILURE, 1, __FILE__, __LINE__, "" )  ;
   }

   return result ;
}


streng *vms_f_fao( tsd_t *TSD, cparamboxptr parms )
{
   void *prmlst[30] = {NULL} ;
   int i, cnt, paran, rc, pcnt=0, icnt=0 ;
   int int_list[30], dcnt=0, xper ;
   struct dsc$descriptor_s dscs[15] ;
   cparamboxptr p ;
   char buffer[512], *cstart, *cptr, *cend ;
   streng *result ;
   $DESCRIPTOR( ctrl, "" ) ;
   $DESCRIPTOR( outbuf, buffer ) ;
   short outlen ;

   if (parms->value==NULL)
      exiterror( ERR_INCORRECT_CALL , 0 ) ;

   ctrl.dsc$a_pointer = parms->value->value ;
   ctrl.dsc$w_length = parms->value->len ;

   cptr = cstart = parms->value->value ;
   cend = cptr + parms->value->len ;

   p = parms->next ;

   for (cptr=cstart; cptr<cend; cptr++)
   {
      if (*cptr!='!') continue ;

      if (*(++cptr)=='#')
      {
         cptr++ ;
         if (!p || !p->value)
            exiterror( ERR_INCORRECT_CALL , 0 ) ;

         cnt = atopos( TSD, p->value, "VMS_F_FAO", pcnt ) ;
         prmlst[pcnt++] = int_list + icnt ;
         int_list[icnt++] = cnt ;
         p = p->next ;
      }
      else if (!isdigit(*cptr))
         cnt = 1 ;
      else
         for (cnt=0;cptr<cend && isdigit(*cptr); cptr++)
            cnt = cnt*10 + *cptr-'0' ;

      paran = 0 ;
      if (cptr<cend && *cptr=='(')
      {
         paran = 1 ;
         cptr++ ;
         if (*cptr=='#')
         {
            if (!p || !p->value)
               exiterror( ERR_INCORRECT_CALL , 0 ) ;

            prmlst[pcnt++] = int_list + icnt ;
            int_list[icnt++] = atopos( TSD, p->value, "VMS_F_FAO", 0 ) ;
            p = p->next ;
         }
         else
            for (;cptr<cend && isdigit(*cptr); cptr++ ) ;
      }

      if (cptr<cend)
      {
         xper = toupper(*cptr) ;
         if (xper=='O' || xper=='X' || xper=='Z' || xper=='U' || xper=='S')
         {
            cptr++ ;
            xper = toupper(*cptr) ;
            if (xper!='B' && xper!='W' && xper!='L')
               exiterror( ERR_INCORRECT_CALL , 0 ) ;

            for (i=0; i<cnt; i++)
            {
               if (!p || !p->value)
                  exiterror( ERR_INCORRECT_CALL , 0 ) ;

               prmlst[pcnt++] = (void *)myatol( TSD, p->value ) ;
               p = p->next ;
            }
         }
         else if (toupper(*cptr)=='A')
         {
            cptr++ ;
            if (cptr<cend && toupper(*cptr)!='S')
               exiterror( ERR_INCORRECT_CALL , 0 ) ;

            for (i=0; i<cnt; i++ )
            {
               if (!p || !p->value)
                  exiterror( ERR_INCORRECT_CALL , 0 ) ;

               dscs[dcnt].dsc$b_class = DSC$K_CLASS_S ;
               dscs[dcnt].dsc$b_dtype = DSC$K_DTYPE_T ;
               dscs[dcnt].dsc$a_pointer = p->value->value ;
               dscs[dcnt].dsc$w_length = p->value->len ;
               prmlst[pcnt++] = &(dscs[dcnt++]) ;
               p = p->next ;
            }
         }
      }
      else
         exiterror( ERR_INCORRECT_CALL , 0 ) ;

      if (paran)
         if (cptr<cend-1 && *(++cptr)!=')')
             exiterror( ERR_INCORRECT_CALL , 0 ) ;
   }

   rc = sys$faol( &ctrl, &outlen, &outbuf, prmlst ) ;
   if (rc!=SS$_NORMAL)
   {
      vms_error( TSD, rc ) ;
/*      return nullstringptr() ; */
   }

   result = Str_makeTSD( outlen ) ;
   result->len = outlen ;
   memcpy( result->value, buffer, outlen ) ;

   return result ;
}
