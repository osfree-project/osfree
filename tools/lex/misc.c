/* misc - miscellaneous flex routines */

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Vern Paxson.
 * 
 * The United States Government has rights in this work pursuant
 * to contract no. DE-AC03-76SF00098 between the United States
 * Department of Energy and the University of California.
 *
 * Redistribution and use in source and binary forms with or without
 * modification are permitted provided that: (1) source distributions retain
 * this entire copyright notice and comment, and (2) distributions including
 * binaries display the following acknowledgement:  ``This product includes
 * software developed by the University of California, Berkeley and its
 * contributors'' in the documentation or other materials provided with the
 * distribution and in all advertising materials mentioning features or use
 * of this software.  Neither the name of the University nor the names of
 * its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* $Header: /home/daffy/u0/vern/flex/RCS/misc.c,v 2.47 95/04/28 11:39:39 vern Exp $ */

/*!
 *  @file misc.c
 *  @brief Miscellaneous flex routines.
 *
 *  Provides error reporting, dynamic array helpers, output formatting,
 *  string utilities and the skeleton-file copier.
 */

#include "flexdef.h"


/*!
 *  @brief Adds a #define to the action file.
 *
 *  @param[in] defname Macro name.
 *  @param[in] value   Macro value.
 */
void action_define( defname, value )
char *defname;
int value;
	{
	char buf[MAXLINE];

	if ( (int) strlen( defname ) > MAXLINE / 2 )
		{
		format_pinpoint_message( _( "name \"%s\" ridiculously long" ), 
			defname );
		return;
		}

	sprintf( buf, "#define %s %d\n", defname, value );
	add_action( buf );
	}


/*!
 *  @brief Adds the given text to the stored actions.
 *
 *  @param[in] new_text Text to append.
 */
void add_action( new_text )
char *new_text;
	{
	int len = strlen( new_text );

	while ( len + action_index >= action_size - 10 /* slop */ )
		{
		int new_size = action_size * 2;

		if ( new_size <= 0 )
			/* Increase just a little, to try to avoid overflow
			 * on 16-bit machines.
			 */
			action_size += action_size / 8;
		else
			action_size = new_size;

		action_array =
			reallocate_character_array( action_array, action_size );
		}

	strcpy( &action_array[action_index], new_text );

	action_index += len;
	}


/*!
 *  @brief Allocates memory for an integer array of the given size.
 *
 *  @param[in] size         Number of elements.
 *  @param[in] element_size Size of each element.
 *
 *  @return Pointer to the allocated memory.
 */
void *allocate_array( size, element_size )
int size;
size_t element_size;
	{
	register void *mem;
	size_t num_bytes = element_size * size;

	mem = flex_alloc( num_bytes );
	if ( ! mem )
		flexfatal(
			_( "memory allocation failed in allocate_array()" ) );

	return mem;
	}


/*!
 *  @brief Tests whether a string is all lower case.
 *
 *  @param[in] str String to test.
 *
 *  @return Non-zero if every character is lower case.
 *  @retval 0 At least one character is not a lower-case letter.
 *  @retval 1 All characters are lower-case letters.
 */
int all_lower( str )
register char *str;
	{
	while ( *str )
		{
		if ( ! isascii( (Char) *str ) || ! islower( *str ) )
			return 0;
		++str;
		}

	return 1;
	}


/*!
 *  @brief Tests whether a string is all upper case.
 *
 *  @param[in] str String to test.
 *
 *  @return Non-zero if every character is upper case.
 *  @retval 0 At least one character is not an upper-case letter.
 *  @retval 1 All characters are upper-case letters.
 */
int all_upper( str )
register char *str;
	{
	while ( *str )
		{
		if ( ! isascii( (Char) *str ) || ! isupper( *str ) )
			return 0;
		++str;
		}

	return 1;
	}


/*!
 *  @brief Bubble-sorts an integer array in increasing order.
 *
 *  synopsis
 *    int v[n], n;
 *    void bubble( v, n );
 *
 *  description
 *    sorts the first n elements of array v and replaces them in
 *    increasing order.
 *
 *  passed
 *    v - the array to be sorted
 *    n - the number of elements of 'v' to be sorted
 *
 *  @param[in,out] v Array to sort.
 *  @param[in]     n Number of elements to sort.
 */
void bubble( v, n )
int v[], n;
	{
	register int i, j, k;

	for ( i = n; i > 1; --i )
		for ( j = 1; j < i; ++j )
			if ( v[j] > v[j + 1] )	/* compare */
				{
				k = v[j];	/* exchange */
				v[j] = v[j + 1];
				v[j + 1] = k;
				}
	}


/*!
 *  @brief Checks that a character is within the expected range.
 *
 *  Checks a character to make sure it's within the range we're expecting.
 *  If not, generates fatal error message and exits.
 *
 *  @param[in] c Character to check.
 */
void check_char( c )
int c;
	{
	if ( c >= CSIZE )
		lerrsf( _( "bad character '%s' detected in check_char()" ),
			readable_form( c ) );

	if ( c >= csize )
		lerrsf(
		_( "scanner requires -8 flag to use the character %s" ),
			readable_form( c ) );
	}



/*!
 *  @brief Replaces an upper-case letter with its lower-case equivalent.
 *
 *  @param[in] c Character to convert.
 *
 *  @return Lower-case equivalent, or @p c unchanged.
 */
Char clower( c )
register int c;
	{
	return (Char) ((isascii( c ) && isupper( c )) ? tolower( c ) : c);
	}


/*!
 *  @brief Returns a dynamically allocated copy of a string.
 *
 *  @param[in] str String to copy.
 *
 *  @return Newly allocated copy.
 */
char *copy_string( str )
register const char *str;
	{
	register const char *c1;
	register char *c2;
	char *copy;
	unsigned int size;

	/* find length */
	for ( c1 = str; *c1; ++c1 )
		;

	size = (c1 - str + 1) * sizeof( char );
	copy = (char *) flex_alloc( size );

	if ( copy == NULL )
		flexfatal( _( "dynamic memory failure in copy_string()" ) );

	for ( c2 = copy; (*c2++ = *str++) != 0; )
		;

	return copy;
	}


/*!
 *  @brief Returns a dynamically allocated copy of a (potentially) unsigned string.
 *
 *  @param[in] str String to copy.
 *
 *  @return Newly allocated copy.
 */
Char *copy_unsigned_string( str )
register Char *str;
	{
	register Char *c;
	Char *copy;

	/* find length */
	for ( c = str; *c; ++c )
		;

	copy = allocate_Character_array( c - str + 1 );

	for ( c = copy; (*c++ = *str++) != 0; )
		;

	return copy;
	}


/*!
 *  @brief Shell-sorts a character array in increasing order.
 *
 *  synopsis
 *
 *    Char v[n];
 *    int n, special_case_0;
 *    cshell( v, n, special_case_0 );
 *
 *  description
 *    Does a shell sort of the first n elements of array v.
 *    If special_case_0 is true, then any element equal to 0
 *    is instead assumed to have infinite weight.
 *
 *  passed
 *    v - array to be sorted
 *    n - number of elements of v to be sorted
 *
 *  @param[in,out] v              Array to sort.
 *  @param[in]     n              Number of elements to sort.
 *  @param[in]     special_case_0 Non-zero to treat 0 as infinite weight.
 */
void cshell( v, n, special_case_0 )
Char v[];
int n, special_case_0;
	{
	int gap, i, j, jg;
	Char k;

	for ( gap = n / 2; gap > 0; gap = gap / 2 )
		for ( i = gap; i < n; ++i )
			for ( j = i - gap; j >= 0; j = j - gap )
				{
				jg = j + gap;

				if ( special_case_0 )
					{
					if ( v[jg] == 0 )
						break;

					else if ( v[j] != 0 && v[j] <= v[jg] )
						break;
					}

				else if ( v[j] <= v[jg] )
					break;

				k = v[j];
				v[j] = v[jg];
				v[jg] = k;
				}
	}


/*!
 *  @brief Finishes up a block of data declarations.
 */
void dataend()
	{
	if ( datapos > 0 )
		dataflush();

	/* add terminator for initialization; { for vi */
	outn( "    } ;\n" );

	dataline = 0;
	datapos = 0;
	}


/*!
 *  @brief Flushes generated data statements.
 */
void dataflush()
	{
	outc( '\n' );

	if ( ++dataline >= NUMDATALINES )
		{
		/* Put out a blank line so that the table is grouped into
		 * large blocks that enable the user to find elements easily.
		 */
		outc( '\n' );
		dataline = 0;
		}

	/* Reset the number of characters written on the current line. */
	datapos = 0;
	}


/*!
 *  @brief Reports an error message and terminates.
 *
 *  @param[in] msg Message to print.
 */
void flexerror( msg )
const char msg[];
	{
	fprintf( stderr, "%s: %s\n", program_name, msg );
	flexend( 1 );
	}


/** @brief Reports a fatal error message and terminates.
 *  @param msg Message to print.
 */
void flexfatal( msg )
const char msg[];
	{
	fprintf( stderr, _( "%s: fatal internal error, %s\n" ),
		program_name, msg );
	exit( 1 );
	}


/** @brief Converts a hexadecimal digit string to an integer value.
 *  @param str Hexadecimal string.
 *  @return Converted value.
 */
int htoi( str )
Char str[];
	{
	unsigned int result;

	(void) sscanf( (char *) str, "%x", &result );

	return result;
	}


/*!
 *  @brief Reports an error message formatted with one integer argument.
 *
 *  @param[in] msg Format string.
 *  @param[in] arg Integer argument.
 */
void lerrif( msg, arg )
const char msg[];
int arg;
	{
	char errmsg[MAXLINE];
	(void) sprintf( errmsg, msg, arg );
	flexerror( errmsg );
	}


/** @brief Reports an error message formatted with one string argument.
 *  @param msg Format string.
 *  @param arg String argument.
 */
void lerrsf( msg, arg )
const char msg[], arg[];
	{
	char errmsg[MAXLINE];

	(void) sprintf( errmsg, msg, arg );
	flexerror( errmsg );
	}


/** @brief Emits a #line statement.
 *  @param output_file Output stream, or NULL to add to the action array.
 *  @param do_infile   Non-zero to use the input file name.
 */
void line_directive_out( output_file, do_infile )
FILE *output_file;
int do_infile;
	{
	char directive[MAXLINE], filename[MAXLINE];
	char *s1, *s2, *s3;
	static char line_fmt[] = "#line %d \"%s\"\n";

	if ( ! gen_line_dirs )
		return;

	if ( (do_infile && ! infilename) || (! do_infile && ! outfilename) )
		/* don't know the filename to use, skip */
		return;

	s1 = do_infile ? infilename : outfilename;
	s2 = filename;
	s3 = &filename[sizeof( filename ) - 2];

	while ( s2 < s3 && *s1 )
		{
		if ( *s1 == '\\' )
			/* Escape the '\' */
			*s2++ = '\\';

		*s2++ = *s1++;
		}

	*s2 = '\0';

	if ( do_infile )
		sprintf( directive, line_fmt, linenum, filename );
	else
		{
		if ( output_file == stdout )
			/* Account for the line directive itself. */
			++out_linenum;

		sprintf( directive, line_fmt, out_linenum, filename );
		}

	/* If output_file is nil then we should put the directive in
	 * the accumulated actions.
	 */
	if ( output_file )
		{
		fputs( directive, output_file );
		}
	else
		add_action( directive );
	}


/*!
 *  @brief Marks the end of the user's section 1 definitions.
 *
 *  Marks the current position in the action array as representing where
 *  the user's section 1 definitions end and the prolog begins.
 */
void mark_defs1()
	{
	defs1_offset = 0;
	action_array[action_index++] = '\0';
	action_offset = prolog_offset = action_index;
	action_array[action_index] = '\0';
	}


/*!
 *  @brief Marks the end of the action prolog.
 *
 *  Marks the current position in the action array as representing the end
 *  of the action prolog.
 */
void mark_prolog()
	{
	action_array[action_index++] = '\0';
	action_offset = action_index;
	action_array[action_index] = '\0';
	}


/*!
 *  @brief Generates a data statement for a two-dimensional array.
 *
 *  Generates a data statement initializing the current 2-D array to "value".
 *
 *  @param[in] value Value to emit.
 */
void mk2data( value )
int value;
	{
	if ( datapos >= NUMDATAITEMS )
		{
		outc( ',' );
		dataflush();
		}

	if ( datapos == 0 )
		/* Indent. */
		out( "    " );

	else
		outc( ',' );

	++datapos;

	out_dec( "%5d", value );
	}


/*!
 *  @brief Generates a data statement.
 *
 *  Generates a data statement initializing the current array element to
 *  "value".
 *
 *  @param[in] value Value to emit.
 */
void mkdata( value )
int value;
	{
	if ( datapos >= NUMDATAITEMS )
		{
		outc( ',' );
		dataflush();
		}

	if ( datapos == 0 )
		/* Indent. */
		out( "    " );
	else
		outc( ',' );

	++datapos;

	out_dec( "%5d", value );
	}


/*!
 *  @brief Returns the integer represented by a string of digits.
 *
 *  @param[in] array Digit string.
 *
 *  @return Converted value.
 */
int myctoi( array )
char array[];
	{
	int val = 0;

	(void) sscanf( array, "%d", &val );

	return val;
	}


/*!
 *  @brief Returns the character corresponding to an escape sequence.
 *
 *  @param[in] array Escape sequence text.
 *
 *  @return Decoded character.
 */
Char myesc( array )
Char array[];
	{
	Char c, esc_char;

	switch ( array[1] )
		{
		case 'b': return '\b';
		case 'f': return '\f';
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';

#if __STDC__
		case 'a': return '\a';
		case 'v': return '\v';
#else
		case 'a': return '\007';
		case 'v': return '\013';
#endif

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			{ /* \<octal> */
			int sptr = 1;

			while ( isascii( array[sptr] ) &&
				isdigit( array[sptr] ) )
				/* Don't increment inside loop control
				 * because if isdigit() is a macro it might
				 * expand into multiple increments ...
				 */
				++sptr;

			c = array[sptr];
			array[sptr] = '\0';

			esc_char = otoi( array + 1 );

			array[sptr] = c;

			return esc_char;
			}

		case 'x':
			{ /* \x<hex> */
			int sptr = 2;

			while ( isascii( array[sptr] ) &&
				isxdigit( (char) array[sptr] ) )
				/* Don't increment inside loop control
				 * because if isdigit() is a macro it might
				 * expand into multiple increments ...
				 */
				++sptr;

			c = array[sptr];
			array[sptr] = '\0';

			esc_char = htoi( array + 2 );

			array[sptr] = c;

			return esc_char;
			}

		default:
			return array[1];
		}
	}


/*!
 *  @brief Converts an octal digit string to an integer value.
 *
 *  @param[in] str Octal string.
 *
 *  @return Converted value.
 */
int otoi( str )
Char str[];
	{
	unsigned int result;

	(void) sscanf( (char *) str, "%o", &result );
	return result;
	}


/*!
 *  @brief Outputs a string, keeping track of the line count.
 *
 *  @param[in] str String to output.
 */
void out( str )
const char str[];
	{
	fputs( str, stdout );
	out_line_count( str );
	}


/** @brief Outputs a formatted string with one integer argument.
 *  @param fmt Format string.
 *  @param n   Integer argument.
 */
void out_dec( fmt, n )
const char fmt[];
int n;
	{
	printf( fmt, n );
	out_line_count( fmt );
	}


/** @brief Outputs a formatted string with two integer arguments.
 *  @param fmt Format string.
 *  @param n1  First integer argument.
 *  @param n2  Second integer argument.
 */
void out_dec2( fmt, n1, n2 )
const char fmt[];
int n1, n2;
	{
	printf( fmt, n1, n2 );
	out_line_count( fmt );
	}


/** @brief Outputs a formatted string with one unsigned argument in hex.
 *  @param fmt Format string.
 *  @param x   Unsigned argument.
 */
void out_hex( fmt, x )
const char fmt[];
unsigned int x;
	{
	printf( fmt, x );
	out_line_count( fmt );
	}


/** @brief Updates the output line count for a string.
 *  @param str String to scan.
 */
void out_line_count( str )
const char str[];
	{
	register int i;

	for ( i = 0; str[i]; ++i )
		if ( str[i] == '\n' )
			++out_linenum;
	}


/** @brief Outputs a formatted string with one string argument.
 *  @param fmt Format string.
 *  @param str String argument.
 */
void out_str( fmt, str )
const char fmt[], str[];
	{
	printf( fmt, str );
	out_line_count( fmt );
	out_line_count( str );
	}


/** @brief Outputs a formatted string with three string arguments.
 *  @param fmt Format string.
 *  @param s1  First string argument.
 *  @param s2  Second string argument.
 *  @param s3  Third string argument.
 */
void out_str3( fmt, s1, s2, s3 )
const char fmt[], s1[], s2[], s3[];
	{
	printf( fmt, s1, s2, s3 );
	out_line_count( fmt );
	out_line_count( s1 );
	out_line_count( s2 );
	out_line_count( s3 );
	}


/** @brief Outputs a formatted string with a string and an integer argument.
 *  @param fmt Format string.
 *  @param str String argument.
 *  @param n   Integer argument.
 */
void out_str_dec( fmt, str, n )
const char fmt[], str[];
int n;
	{
	printf( fmt, str, n );
	out_line_count( fmt );
	out_line_count( str );
	}


/** @brief Outputs a single character, keeping track of the line count.
 *  @param c Character to output.
 */
void outc( c )
int c;
	{
	putc( c, stdout );

	if ( c == '\n' )
		++out_linenum;
	}


/*!
 *  @brief Outputs a string followed by a newline.
 *
 *  @param[in] str String to output.
 */
void outn( str )
const char str[];
	{
	puts( str );
	out_line_count( str );
	++out_linenum;
	}


/** @brief Returns the human-readable form of a character.
 *
 *  The returned string is in static storage.
 *
 *  @param c Character code.
 *  @return Pointer to a static buffer with the printable form.
 */
char *readable_form( c )
register int c;
	{
	static char rform[10];

	if ( (c >= 0 && c < 32) || c >= 127 )
		{
		switch ( c )
			{
			case '\b': return "\\b";
			case '\f': return "\\f";
			case '\n': return "\\n";
			case '\r': return "\\r";
			case '\t': return "\\t";

#if __STDC__
			case '\a': return "\\a";
			case '\v': return "\\v";
#endif

			default:
				(void) sprintf( rform, "\\%.3o",
						(unsigned int) c );
				return rform;
			}
		}

	else if ( c == ' ' )
		return "' '";

	else
		{
		rform[0] = c;
		rform[1] = '\0';

		return rform;
		}
	}


/*!
 *  @brief Increases the size of a dynamic array.
 *
 *  @param[in] array        Array to resize.
 *  @param[in] size         New number of elements.
 *  @param[in] element_size Size of each element.
 *
 *  @return Pointer to the resized array.
 */
void *reallocate_array( array, size, element_size )
void *array;
int size;
size_t element_size;
	{
	register void *new_array;
	size_t num_bytes = element_size * size;

	new_array = flex_realloc( array, num_bytes );
	if ( ! new_array )
		flexfatal( _( "attempt to increase array size failed" ) );

	return new_array;
	}


/*!
 *  @brief Writes out one section of the skeleton file.
 *
 *  Description
 *     Copies skelfile or skel array to stdout until a line beginning with
 *     "%%" or EOF is found.
 */
void skelout()
	{
	char buf_storage[MAXLINE];
	char *buf = buf_storage;
	int do_copy = 1;

	/* Loop pulling lines either from the skelfile, if we're using
	 * one, or from the skel[] array.
	 */
	while ( skelfile ?
		(fgets( buf, MAXLINE, skelfile ) != NULL) :
		((buf = (char *) skel[skel_ind++]) != 0) )
		{ /* copy from skel array */
		if ( buf[0] == '%' )
			{ /* control line */
			switch ( buf[1] )
				{
				case '%':
					return;

				case '+':
					do_copy = C_plus_plus;
					break;

				case '-':
					do_copy = ! C_plus_plus;
					break;

				case '*':
					do_copy = 1;
					break;

				default:
					flexfatal(
					_( "bad line in skeleton file" ) );
				}
			}

		else if ( do_copy )
			{
			if ( skelfile )
				/* Skeleton file reads include final
				 * newline, skel[] array does not.
				 */
				out( buf );
			else
				outn( buf );
			}
		}
	}


/*!
 *  @brief Outputs a yy_trans_info structure.
 *
 *  Outputs the yy_trans_info structure with the two elements, element_v and
 *  element_n.  Formats the output with spaces and carriage returns.
 *
 *  @param[in] element_v First element.
 *  @param[in] element_n Second element.
 */
void transition_struct_out( element_v, element_n )
int element_v, element_n;
	{
	out_dec2( " {%4d,%4d },", element_v, element_n );

	datapos += TRANS_STRUCT_PRINT_LENGTH;

	if ( datapos >= 79 - TRANS_STRUCT_PRINT_LENGTH )
		{
		outc( '\n' );

		if ( ++dataline % 10 == 0 )
			outc( '\n' );

		datapos = 0;
		}
	}


/*!
 *  @brief Allocates memory, terminating on failure.
 *
 *  The following is only needed when building flex's parser using certain
 *  broken versions of bison.
 *
 *  @param[in] size Number of bytes to allocate.
 *
 *  @return Pointer to the allocated memory.
 */
void *yy_flex_xmalloc( size )
int size;
	{
	void *result = flex_alloc( (size_t) size );

	if ( ! result  )
		flexfatal(
			_( "memory allocation failed in yy_flex_xmalloc()" ) );

	return result;
	}


/*!
 *  @brief Sets a region of memory to 0.
 *
 *  Description
 *     Sets region_ptr[0] through region_ptr[size_in_bytes - 1] to zero.
 *
 *  @param[in] region_ptr    Start of the region.
 *  @param[in] size_in_bytes Size of the region in bytes.
 */
void zero_out( region_ptr, size_in_bytes )
char *region_ptr;
size_t size_in_bytes;
	{
	register char *rp, *rp_end;

	rp = region_ptr;
	rp_end = region_ptr + size_in_bytes;

	while ( rp < rp_end )
		*rp++ = 0;
	}
