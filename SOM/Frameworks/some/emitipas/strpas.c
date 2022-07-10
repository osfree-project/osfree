#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

#define LINE_LENGTH	70

char *passtrIDLConst(char *pcBuf, char *pcConst)
{
  char		*p, ch, lch;
  char		*dst;
  char		code = 1, lcode;
  unsigned long	chrCnt = 0;
  unsigned long	multiline = 0;

  if ( *pcConst != '\'' && *pcConst != '"' )
    return strcpy(pcBuf,pcConst);

  pcConst++;
  dst = pcBuf;

  for(;;)
  {
    ch = *(pcConst++);
    if ( (ch=='\'' || ch=='\"') && !*pcConst )
      break;    
    if ( !ch ) break;

    if ( ch == '\'' )
      *(dst++) = '\'';
    else if ( ch == '\\' )
    {
      ch = *pcConst;
      lcode = code;
      code = 0;
      switch ( ch )
      {
        case 'r':
          code = 0x0D;
          pcConst++;
          break;
        case 'n':
          code = 0x0A;
          pcConst++;
          break;
        case 't':
          code = 0x09;
          pcConst++;
          break;
        case '\\':
          if ( lcode )
            *(dst++) = '\'';
          *(dst++) = '\\';
          pcConst++;
          code = 0;
          continue;
        default:
          code = strtol(pcConst, &p, 0);
          if ( pcConst == p )
          {
            code = lcode;
            continue;
          }
          else
            pcConst = p;
      }

      if ( code )
      {
        if ( !lcode )
          *(dst++) = '\'';
        dst += sprintf( dst, "#$%.2X", code);
        chrCnt += 5;
        continue;
      }
    }

    if ( chrCnt >= LINE_LENGTH )
    {
      chrCnt = 0;
      if ( !code ) *(dst++) = '\'';
      if ( !multiline )
      {
        memmove(pcBuf+3,pcBuf,dst-pcBuf);
        memcpy(pcBuf,"\n  ",3);
        dst += 3;
      }
      memcpy(dst,"+\n  \'",5);
      dst += 5;
      code = 0;
      multiline = 1;
    }
    else
      if ( code )
      {
        *(dst++) = '\'';
        code = 0;
        chrCnt++;
      }
    *(dst++) = ch;
    chrCnt++;
    lch = ch;
  }

  if ( !code && ( lch != '\'' ) )
  {
    if ( lch == '"' ) dst--;
    *(unsigned short *)(dst) = '\0\'';
  }
  else
    *dst = '\0';
  return pcBuf;
}
