/* File produces I/O Procedures for all GBM formats */

#include <os2.h>

#define INCL_MMIOOS2
#include <os2me.h>

#include <gbm.h>

struct STR_SAVED_TOKENS_ {
        char *str_next_saved_tokens;
        char str_ch_saved_tokens;
};

typedef struct STR_SAVED_TOKENS_ STR_SAVED_TOKENS;

static char *nxtToken = 0;              /* pointer to previous scanned string */
static char ch;                                         /* previous token delimiter */

char *StrTokenize(char *str, const char * const token)
{

        if(!str) {
                if((str = nxtToken) == 0                /* nothing to do */
                 || (*str++ = ch) == 0)               /* end of string reached */
                        return( nxtToken = 0);
        }

        if(!token || !*token)                   /* assume all the string */
                nxtToken = 0;
        else {
                nxtToken = str - 1;
                while(!strchr(token, *++nxtToken));
                ch = *nxtToken;
                *nxtToken = 0;
        }

        return( str);
}

void StrTokSave(STR_SAVED_TOKENS *st)
{

        if(st) {
                st->str_next_saved_tokens = nxtToken;
                st->str_ch_saved_tokens = ch;
        }

}

void StrTokRestore(STR_SAVED_TOKENS *st)
{

        if(st) {
                nxtToken = st->str_next_saved_tokens;
                ch = st->str_ch_saved_tokens;
        }

}


#define StrTokStop() (void)StrTokenize(0, 0)


void main(void)
{
  int n_ft, ft;
  GBMFT gbmft;
  char *p;
  STR_SAVED_TOKENS st;

  gbm_query_n_filetypes(&n_ft);

  for ( ft = 0; ft < n_ft; ft++ )
  {
    gbm_query_filetype(ft, &gbmft);

    StrTokSave(&st);
    if((p = StrTokenize((char*)gbmft.extensions, " ")) != 0) do if(*p)
    {
      printf("LONG EXPENTRY %sproc(PVOID pmmioStr,USHORT usMsg,LONG lParam1,LONG lParam2)", p);
      printf("{\n");
      printf("  switch (usMsg)\n");
      printf("  {\n");
      printf("    case MMIOM_GETFORMATNAME:\n");
      printf("    {\n");
      printf("      memcpy((PSZ)lParam1, \"%s (%s)\", %d);\n", gbmft.long_name, p, strlen(gbmft.long_name)+strlen(p)+4);
      printf("      return(%d);\n", strlen(gbmft.long_name)+strlen(p)+4);
      printf("    }\n");
      printf("    case MMIOM_GETFORMATINFO:\n");
      printf("    {\n");
      printf("      PMMFORMATINFO pmmformatinfo;\n");
      printf("      pmmformatinfo = (PMMFORMATINFO) lParam1;\n");
      printf("      pmmformatinfo->ulStructLen  = sizeof (MMFORMATINFO);\n");
      printf("      pmmformatinfo->fccIOProc    = 0x%x;\n", mmioStringToFOURCC(p, MMIO_TOUPPER ));
      printf("      pmmformatinfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;\n");
      printf("      pmmformatinfo->ulMediaType  = MMIO_MEDIATYPE_IMAGE;\n");
      printf("      pmmformatinfo->ulFlags      = MMIO_CANREADTRANSLATED|MMIO_CANSEEKTRANSLATED");
      if (gbmft.flags & (GBM_FT_W1|GBM_FT_W4|GBM_FT_W8|GBM_FT_W24|GBM_FT_W32|GBM_FT_W48|GBM_FT_W64))
        printf("|MMIO_CANWRITETRANSLATED");
      printf(";\n      strcpy ((PSZ) pmmformatinfo->szDefaultFormatExt, \"%s\");\n",p);
      printf("      pmmformatinfo->ulCodePage = MMIO_DEFAULT_CODE_PAGE;\n");
      printf("      pmmformatinfo->ulLanguage = MMIO_LC_US_ENGLISH;\n");
      printf("      pmmformatinfo->lNameLength=%d;\n", strlen(gbmft.long_name)+strlen(p)+4);
      printf("      return (MMIO_SUCCESS);\n");
      printf("    }\n");
      printf("    default:\n");
      printf("    {\n");
      printf("      return IOProc_Entry(pmmioStr,usMsg,lParam1,lParam2);\n");
      printf("    }\n");
      printf("  }\n");
      printf("}\n");
    } while((p = StrTokenize(0, " ")) != 0);
    StrTokRestore(&st);
  }
}
