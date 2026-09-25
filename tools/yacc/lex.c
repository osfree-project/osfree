/****************************************************************
 * Token-reader for Bison's input parser,
 * Copyright (C) 1984, 1986, 1989, 1992 Free Software Foundation, Inc.
 *
 * This file is part of Bison, the GNU Compiler Compiler.
 *
 * Bison is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Bison is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bison; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ****************************************************************/

/*!
 *  @file lex.c
 *  @brief Token reader for Bison's input parser.
 *
 *  Implements lex(), the entry point called from reader.c. It returns
 *  one of the token type codes defined in lex.h. When an identifier is
 *  seen, the code IDENTIFIER is returned and the name is looked up in
 *  the symbol table using symtab.c; symval is set to a pointer to the
 *  entry found.
 *
 *  @copyright Copyright (C) 1984, 1986, 1989, 1992 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */


/*
   lex is the entry point.  It is called from reader.c.
   It returns one of the token-type codes defined in lex.h.
   When an identifier is seen, the code IDENTIFIER is returned
   and the name is looked up in the symbol table using symtab.c;
   symval is set to a pointer to the entry found.  */

#include <stdio.h>
#include "system.h"
#include "files.h"
#include "getopt.h"		/* for optarg */
#include "symtab.h"
#include "lex.h"
#include "alloc.h"

/* flags set by % directives */
extern int definesflag;    	/*!< Non-zero when -d was given. */
extern int toknumflag;   	/*!< Non-zero when -k was given. */
extern int noparserflag;	/*!< Non-zero when -n was given. */
extern int fixed_outfiles;  	/*!< Non-zero when -y was given. */
extern int nolinesflag;    	/*!< Non-zero when -l was given. */
extern int rawtoknumflag;	/*!< Non-zero when -r was given. */
extern int verboseflag;	/*!< Non-zero when -v was given. */
extern int debugflag;  		/*!< Non-zero when -t was given. */
extern char *spec_name_prefix; 	/*!< Name prefix set by -p. */
extern char *spec_file_prefix;	/*!< File prefix set by -b. */
/*spec_outfile is declared in files.h, for -o */

extern int lineno;          /*!< Current input line number. */
extern int translations;    /*!< Non-zero if the token translation table is used. */

/*!
 *  @brief Initializes the token buffer.
 */
void init_lex PARAMS((void));

/*!
 *  @brief Doubles the size of the token buffer.
 *
 *  @param[in] p Current position inside the buffer.
 *
 *  @return New position inside the reallocated buffer.
 */
char *grow_token_buffer PARAMS((char *));

/*!
 *  @brief Skips whitespace and comments on the input.
 *
 *  @return The first non-whitespace character.
 */
int skip_white_space PARAMS((void));

/*!
 *  @brief Calls getc(), terminating on unexpected end of file.
 *
 *  @param[in] f Input stream.
 *
 *  @return The character read.
 */
int safegetc PARAMS((FILE *));

/*!
 *  @brief Reads one literal character and appends it to the token buffer.
 *
 *  Processes backslash escapes, appends the canonical text of the
 *  character to the buffer pointed to by @p pp, stores its code in
 *  @p pcode, and returns 1 unless the character is an unescaped
 *  @p term or a newline.
 *
 *  @param[in,out] pp    Pointer to the buffer position.
 *  @param[out]    pcode Character code.
 *  @param[in]     term  Terminating character.
 *
 *  @return Non-zero if the character is not the terminator.
 */
int literalchar PARAMS((char **, int *, char));

/*!
 *  @brief Pushes a token back for the next call to lex().
 *
 *  @param[in] token Token to push back.
 */
void unlex PARAMS((int));

/*!
 *  @brief Reads the next token from the input.
 *
 *  @return The next token code.
 */
int lex PARAMS((void));

/*!
 *  @brief Parses a token that starts with a percent sign.
 *
 *  @return The token code corresponding to the directive.
 *  @retval NOOP The directive only set a flag; no token is emitted.
 */
int parse_percent_token PARAMS((void));

/* functions from main.c */
/*!
 *  @brief Returns a printable form of a character code.
 *
 *  @param[in] c Character code.
 *
 *  @return Pointer to a static buffer with the printable form.
 */
extern char *printable_version PARAMS((int));

/*!
 *  @brief Prints a fatal error and exits.
 *
 *  @param[in] s Message.
 */
extern void fatal PARAMS((char *));

/*!
 *  @brief Prints a warning.
 *
 *  @param[in] s Message.
 */
extern void warn PARAMS((char *));

/*!
 *  @brief Prints a warning with an integer argument.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  Integer argument.
 */
extern void warni PARAMS((char *, int));

/*!
 *  @brief Prints a warning with one string argument.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  String argument.
 */
extern void warns PARAMS((char *, char *));

/*!< Buffer for storing the current token. */
char *token_buffer;

/*!< Allocated size of token_buffer, not including space for terminator. */
int maxtoken;

bucket *symval;     /*!< Symbol table entry of the last identifier read. */
int numval;         /*!< Numeric value of the last NUMBER token read. */

static int unlexed;		/*!< Token to be reread, or -1. */
static bucket *unlexed_symval;	/*!< Symbol value saved with the pushed-back token. */


/*!
 *  @brief Initializes the token buffer.
 */
void
init_lex (void)
{
  maxtoken = 100;
  token_buffer = NEW2 (maxtoken + 1, char);
  unlexed = -1;
}


/*!
 *  @brief Doubles the size of the token buffer.
 *
 *  @param[in] p Current position inside the buffer.
 *
 *  @return New position inside the reallocated buffer.
 */
char *
grow_token_buffer (char *p)
{
  int offset = p - token_buffer;
  maxtoken *= 2;
  token_buffer = (char *) xrealloc(token_buffer, maxtoken + 1);
  return token_buffer + offset;
}


/*!
 *  @brief Skips whitespace and comments on the input.
 *
 *  @return The first non-whitespace character.
 */
int
skip_white_space (void)
{
  register int c;
  register int inside;

  c = getc(finput);

  for (;;)
    {
      int cplus_comment;

      switch (c)
	{
	case '/':
	  c = getc(finput);
	  if (c != '*' && c != '/')
	    {
	      warn(_("unexpected `/' found and ignored"));
	      break;
	    }
	  cplus_comment = (c == '/');

	  c = getc(finput);

	  inside = 1;
	  while (inside)
	    {
	      if (!cplus_comment && c == '*')
		{
		  while (c == '*')
		    c = getc(finput);

		  if (c == '/')
		    {
		      inside = 0;
		      c = getc(finput);
		    }
		}
	      else if (c == '\n')
		{
		  lineno++;
		  if (cplus_comment)
		    inside = 0;
		  c = getc(finput);
		}
	      else if (c == EOF)
		fatal(_("unterminated comment"));
	      else
		c = getc(finput);
	    }

	  break;

	case '\n':
	  lineno++;

	case ' ':
	case '\t':
	case '\f':
	  c = getc(finput);
	  break;

	default:
	  return (c);
	}
    }
}

/*!
 *  @brief Calls getc(), terminating on unexpected end of file.
 *
 *  Do a getc, but give error message if EOF encountered.
 *
 *  @param[in] f Input stream.
 *
 *  @return The character read.
 */
int
safegetc (FILE *f)
{
  register int c = getc(f);
  if (c == EOF)
    fatal(_("Unexpected end of file"));
  return c;
}

/*!
 *  @brief Reads one literal character and appends it to the token buffer.
 *
 *  Read one literal character from finput. process \ escapes. Append
 *  the normalized string version of the char to @p pp. Assign the
 *  character code to @p pcode. Return 1 unless the character is an
 *  unescaped @p term or \n; report error for \n.
 *
 *  @param[in,out] pp    Pointer to the buffer position.
 *  @param[out]    pcode Character code.
 *  @param[in]     term  Terminating character.
 *
 *  @return Non-zero if the character is not the terminator.
 */
int
literalchar (char **pp, int *pcode, char term)
{
  register int c;
  register char *p;
  register int code;
  int wasquote = 0;

  c = safegetc(finput);
  if (c == '\n')
    {
      warn(_("unescaped newline in constant"));
      ungetc(c, finput);
      code = '?';
      wasquote = 1;
    }
  else if (c != '\\')
    {
      code = c;
      if (c == term)
	wasquote = 1;
    }
  else
    {
      c = safegetc(finput);
      if (c == 't')  code = '\t';
      else if (c == 'n')  code = '\n';
      else if (c == 'a')  code = '\007';
      else if (c == 'r')  code = '\r';
      else if (c == 'f')  code = '\f';
      else if (c == 'b')  code = '\b';
      else if (c == 'v')  code = '\013';
      else if (c == '\\')  code = '\\';
      else if (c == '\'')  code = '\'';
      else if (c == '\"')  code = '\"';
      else if (c <= '7' && c >= '0')
	{
	  code = 0;
	  while (c <= '7' && c >= '0')
	    {
	      code = (code * 8) + (c - '0');
	      if (code >= 256 || code < 0)
		{
		  warni(_("octal value outside range 0...255: `\\%o'"), code);
		  code &= 0xFF;
		  break;
		}
	      c = safegetc(finput);
	    }
	  ungetc(c, finput);
	}
      else if (c == 'x')
	{
	  c = safegetc(finput);
	  code = 0;
	  while (1)
	    {
	      if (c >= '0' && c <= '9')
		code *= 16,  code += c - '0';
	      else if (c >= 'a' && c <= 'f')
		code *= 16,  code += c - 'a' + 10;
	      else if (c >= 'A' && c <= 'F')
		code *= 16,  code += c - 'A' + 10;
	      else
		break;
	      if (code >= 256 || code<0)
		{
		  warni(_("hexadecimal value above 255: `\\x%x'"), code);
		  code &= 0xFF;
		  break;
		}
	      c = safegetc(finput);
	    }
	  ungetc(c, finput);
	}
      else
	{
	  warns (_("unknown escape sequence: `\\' followed by `%s'"),
		 printable_version(c));
	  code = '?';
	}
    } /* has \ */

  /* now fill token_buffer with the canonical name for this character
     as a literal token.  Do not use what the user typed,
     so that `\012' and `\n' can be interchangeable.  */

  p = *pp;
  if (code == term && wasquote)
    *p++ = code;
  else if (code == '\\')  {*p++ = '\\'; *p++ = '\\';}
  else if (code == '\'')  {*p++ = '\\'; *p++ = '\'';}
  else if (code == '\"')  {*p++ = '\\'; *p++ = '\"';}
  else if (code >= 040 && code < 0177)
    *p++ = code;
  else if (code == '\t')  {*p++ = '\\'; *p++ = 't';}
  else if (code == '\n')  {*p++ = '\\'; *p++ = 'n';}
  else if (code == '\r')  {*p++ = '\\'; *p++ = 'r';}
  else if (code == '\v')  {*p++ = '\\'; *p++ = 'v';}
  else if (code == '\b')  {*p++ = '\\'; *p++ = 'b';}
  else if (code == '\f')  {*p++ = '\\'; *p++ = 'f';}
  else
    {
      *p++ = '\\';
      *p++ = code / 0100 + '0';
      *p++ = ((code / 010) & 07) + '0';
      *p++ = (code & 07) + '0';
    }
  *pp = p;
  *pcode = code;
  return  ! wasquote;
}


/*!
 *  @brief Pushes a token back for the next call to lex().
 *
 *  @param[in] token Token to push back.
 */
void
unlex (int token)
{
  unlexed = token;
  unlexed_symval = symval;
}


/*!
 *  @brief Reads the next token from the input.
 *
 *  @return The next token code.
 */
int
lex (void)
{
  register int c;
  char *p;

  if (unlexed >= 0)
    {
      symval = unlexed_symval;
      c = unlexed;
      unlexed = -1;
      return (c);
    }

  c = skip_white_space();
  *token_buffer = c;	/* for error messages (token buffer always valid) */
  token_buffer[1] = 0;

  switch (c)
    {
    case EOF:
      strcpy(token_buffer, "EOF");
      return (ENDFILE);

    case 'A':  case 'B':  case 'C':  case 'D':  case 'E':
    case 'F':  case 'G':  case 'H':  case 'I':  case 'J':
    case 'K':  case 'L':  case 'M':  case 'N':  case 'O':
    case 'P':  case 'Q':  case 'R':  case 'S':  case 'T':
    case 'U':  case 'V':  case 'W':  case 'X':  case 'Y':
    case 'Z':
    case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    case 'p':  case 'q':  case 'r':  case 's':  case 't':
    case 'u':  case 'v':  case 'w':  case 'x':  case 'y':
    case 'z':
    case '.':  case '_':
      p = token_buffer;
      while (isalnum(c) || c == '_' || c == '.')
	{
	  if (p == token_buffer + maxtoken)
	    p = grow_token_buffer(p);

	  *p++ = c;
	  c = getc(finput);
	}

      *p = 0;
      ungetc(c, finput);
      symval = getsym(token_buffer);
      return (IDENTIFIER);

    case '0':  case '1':  case '2':  case '3':  case '4':
    case '5':  case '6':  case '7':  case '8':  case '9':
      {
	numval = 0;

	p = token_buffer;
	while (isdigit(c))
	  {
	    if (p == token_buffer + maxtoken)
	      p = grow_token_buffer(p);

	    *p++ = c;
	    numval = numval*10 + c - '0';
	    c = getc(finput);
	  }
	*p = 0;
	ungetc(c, finput);
	return (NUMBER);
      }

    case '\'':

      /* parse the literal token and compute character code in  code  */

      translations = -1;
      {
	int code, discode;
	char discard[10], *dp;

	p = token_buffer;
	*p++ = '\'';
	literalchar(&p, &code, '\'');

	c = getc(finput);
	if (c != '\'')
	  {
	    warn(_("use \"...\" for multi-character literal tokens"));
	    while (1)
	      {
		dp = discard;
		if (! literalchar(&dp, &discode, '\''))
		  break;
	      }
	  }
	*p++ = '\'';
	*p = 0;
	symval = getsym(token_buffer);
	symval->class = STOKEN;
	if (! symval->user_token_number)
	  symval->user_token_number = code;
	return (IDENTIFIER);
      }

    case '\"':

      /* parse the literal string token and treat as an identifier */

      translations = -1;
      {
	int code;	/* ignored here */
	p = token_buffer;
	*p++ = '\"';
	while (literalchar(&p, &code, '\"'))  /* read up to and including " */
	  {
	    if (p >= token_buffer + maxtoken - 4)
	      p = grow_token_buffer(p);
	  }
	*p = 0;

	symval = getsym(token_buffer);
	symval->class = STOKEN;

	return (IDENTIFIER);
      }

    case ',':
      return (COMMA);

    case ':':
      return (COLON);

    case ';':
      return (SEMICOLON);

    case '|':
      return (BAR);

    case '{':
      return (LEFT_CURLY);

    case '=':
      do
	{
	  c = getc(finput);
	  if (c == '\n') lineno++;
	}
      while(c==' ' || c=='\n' || c=='\t');

      if (c == '{')
	{
	  strcpy(token_buffer, "={");
	  return(LEFT_CURLY);
	}
      else
	{
	  ungetc(c, finput);
	  return(ILLEGAL);
	}

    case '<':
      p = token_buffer;
      c = getc(finput);
      while (c != '>')
	{
	  if (c == EOF)
	    fatal(_("unterminated type name at end of file"));
	  if (c == '\n')
	    {
	      warn(_("unterminated type name"));
	      ungetc(c, finput);
	      break;
	    }

	  if (p == token_buffer + maxtoken)
	    p = grow_token_buffer(p);

	  *p++ = c;
	  c = getc(finput);
	}
      *p = 0;
      return (TYPENAME);


    case '%':
      return (parse_percent_token());

    default:
      return (ILLEGAL);
    }
}

/*!
 *  @brief Table describing the % directives accepted by the reader.
 *
 *  The following table dictates the action taken for the various
 *  % directives. A setflag value causes the named flag to be set. A
 *  retval action returns the code.
 */
struct percent_table_struct {
	char *name;         /*!< Directive name without the leading percent sign. */
	void *setflag;      /*!< Pointer to a flag variable, or NULL. */
	int retval;         /*!< Code to return, or NOOP/SETOPT. */
} percent_table[] =
{
  {"token", NULL, TOKEN},
  {"term", NULL, TOKEN},
  {"nterm", NULL, NTERM},
  {"type", NULL, TYPE},
  {"guard", NULL, GUARD},
  {"union", NULL, UNION},
  {"expect", NULL, EXPECT},
  {"thong", NULL, THONG},
  {"start", NULL, START},
  {"left", NULL, LEFT},
  {"right", NULL, RIGHT},
  {"nonassoc", NULL, NONASSOC},
  {"binary", NULL, NONASSOC},
  {"semantic_parser", NULL, SEMANTIC_PARSER},
  {"pure_parser", NULL, PURE_PARSER},
  {"prec", NULL, PREC},

  {"no_lines", &nolinesflag, NOOP}, /* -l */
  {"raw", &rawtoknumflag, NOOP}, /* -r */
  {"token_table", &toknumflag, NOOP}, /* -k */

#if 0
  /* These can be utilized after main is reoganized so
     open_files() is deferred 'til after read_declarations().
     But %{ and %union both put information into files
     that have to be opened before read_declarations().
     */
  {"yacc", &fixed_outfiles, NOOP}, /* -y */
  {"fixed_output_files", &fixed_outfiles, NOOP}, /* -y */
  {"defines", &definesflag, NOOP}, /* -d */
  {"no_parser", &noparserflag, NOOP}, /* -n */
  {"output_file", &spec_outfile, SETOPT}, /* -o */
  {"file_prefix", &spec_file_prefix, SETOPT}, /* -b */
  {"name_prefix", &spec_name_prefix, SETOPT}, /* -p */

  /* These would be acceptable, but they do not affect processing */
  {"verbose", &verboseflag, NOOP}, /* -v */
  {"debug", &debugflag, NOOP},	/* -t */
  /*	{"help", <print usage stmt>, NOOP},*/	/* -h */
  /*	{"version", <print version number> ,  NOOP},*/	/* -V */
#endif

  {NULL, NULL, ILLEGAL}
};

/*!
 *  @brief Parses a token that starts with a percent sign.
 *
 *  Parse a token which starts with %. Assumes the % has already been
 *  read and discarded.
 *
 *  @return The token code corresponding to the directive.
 *  @retval NOOP The directive only set a flag; no token is emitted.
 */
int
parse_percent_token (void)
{
  register int c;
  register char *p;
  register struct percent_table_struct *tx;

  p = token_buffer;
  c = getc(finput);
  *p++ = '%';
  *p++ = c;	/* for error msg */
  *p = 0;

  switch (c)
    {
    case '%':
      return (TWO_PERCENTS);

    case '{':
      return (PERCENT_LEFT_CURLY);

    case '<':
      return (LEFT);

    case '>':
      return (RIGHT);

    case '2':
      return (NONASSOC);

    case '0':
      return (TOKEN);

    case '=':
      return (PREC);
    }
  if (!isalpha(c))
    return (ILLEGAL);

  p = token_buffer;
  *p++ = '%';
  while (isalpha(c) || c == '_' || c == '-')
    {
      if (p == token_buffer + maxtoken)
	p = grow_token_buffer(p);

      if (c == '-') c = '_';
      *p++ = c;
      c = getc(finput);
    }

  ungetc(c, finput);

  *p = 0;

  /* table lookup % directive */
  for (tx = percent_table; tx->name; tx++)
    if (strcmp(token_buffer+1, tx->name) == 0)
      break;
  if (tx->retval == SETOPT)
    {
      *((char **)(tx->setflag)) = optarg;
      return NOOP;
    }
  if (tx->setflag)
    {
      *((int *)(tx->setflag)) = 1;
      return NOOP;
    }
  return tx->retval;
}
