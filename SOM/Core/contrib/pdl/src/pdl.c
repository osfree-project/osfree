/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static short _output_stack[256];
static short _output_stack_index;

#define is_output   1
#define is_private  2

static char out_dir[1024];
static FILE *fp_out;
static char out_file[1024];

/*
static short output_stack_value(void)
{
	return _output_stack[_output_stack_index];
}
*/

static void output_stack_push(short v)
{
	_output_stack[_output_stack_index++]=v;
}

static short output_stack_pop(void)
{
	return _output_stack[--_output_stack_index];
}

static char *first_non_white(char *p)
{
	if (p)
	{
		while (*p)
		{
			switch (*p)
			{
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				p++;
				break;
			default:
				return p;
			}
		}
	}

	return 0;
}

int is_only_private(char *p)
{
	p=first_non_white(p);

	if (p)
	{
		char *q="__PRIVATE__";
		size_t i=strlen(q);

		if (strlen(p) >= i)
		{
			if (!memcmp(p,q,i))
			{
				p+=i;

				if (!first_non_white(p))
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

static int do_if(char *p)
{
	if (is_only_private(p))
	{
		output_stack_push(is_private);

		return 0;
	}

	output_stack_push(is_output);

	return 1;
}

static int do_ifdef(char *p)
{
	if (is_only_private(p))
	{
		output_stack_push(is_private);

		return 0;
	}

	output_stack_push(is_output);

	return 1;
}

static int do_output(void)
{
	short i=_output_stack_index;

	while (i--)
	{
		short s=_output_stack[i];

		if ((s & is_output)==0)
		{
			return 0;
		}
	}

	return 1;
}

static int do_else(void)
{
	int i=do_output();
	short s=output_stack_pop();

	if (s & is_private)
	{
		s^=is_output;

		output_stack_push(s);

		return i;
	}

	output_stack_push(s);

	return i;
}

static int do_endif(void)
{
	int i=do_output();
	short s=output_stack_pop();

	if (s & is_private) return 0;

	return i;
}

static int do_ifndef(char *p)
{
	if (is_only_private(p))
	{
		output_stack_push(is_output|is_private);

		return 0;
	}
	else
	{
		output_stack_push(is_output);
	}

	return 1;
}

/* this returns 1 if should not emit output */

static int do_command(char *p)
{
	int i;
	char buf[256];

	p++;
	p=first_non_white(p);

	if (!p) return 1;

	i=0;

	while (p[i])
	{
		if ((p[i]==' ')||(p[i]=='\t')||(p[i]=='\n')||(p[i]=='\r'))
		{
			break;
		}
		i++;
	}

	memcpy(buf,p,i);
	buf[i]=0;

	p+=i;
	p=first_non_white(p);

	if (!strcmp(buf,"if")) 
	{
		return do_if(p);
	}

	if (!strcmp(buf,"ifdef")) 
	{
		return do_ifdef(p);
	}

	if (!strcmp(buf,"else")) 
	{
		return do_else();
	}

	if (!strcmp(buf,"ifndef")) 
	{
		return do_ifndef(p);
	}

	if (!strcmp(buf,"endif")) 
	{
		return do_endif();
	}

	return 1;
}

static void output_line(char *line)
{
	if (!fp_out)
	{
		fp_out=fopen(out_file,"w");

		if (!fp_out)
		{
			fprintf(stderr,"failed to open '%s'\n",out_file);
			fflush(stderr);
			exit(1);
		}
	}
	if (line[0]) fprintf(fp_out,"%s",line);
}

static void do_line(char *line)
{
	char *p;

	p=first_non_white(line);

	if (p)
	{
		if (*p)
		{
			if (*p=='#')
			{
				if (!do_command(p))
				{
					return;
				}
			}
		}
	}

	if (do_output())
	{
		output_line(line);
	}
}

static void do_filter(FILE *in)
{
	char buf[2048];

	while (fgets(buf,sizeof(buf),in))
	{
		do_line(buf);
	}
}

static void get_filestem(char *p,char *name,size_t namelen)
{
char *q=p+strlen(p);

	while (q > p)
	{
		q--;

		if ((*q=='\\')||(*q=='/')||(*q==':'))
		{
			q++;
			strncpy(name,q,namelen);
			return;
		}
	}

	strncpy(name,p,namelen);
}

int main(int argc,char **argv)
{
	int i=1;

	while (i < argc)
	{
		char *p=argv[i++];

		if ((!strcmp(p,"-d"))||
			(!strcmp(p,"-o")))
		{
			if (fp_out)
			{
				if (fp_out!=stdout)
				{
					fclose(fp_out);
				}

				fp_out=NULL;
			}

			if (!p) 
			{
				fprintf(stderr,"no outdir specified\n");
				return 1;
			}

			p=argv[i++];
			strncpy(out_dir,p,sizeof(out_dir));
		}
		else
		{
			FILE *fp_in=NULL;

			if (!strcmp(out_dir,"-"))
			{
				fp_out=stdout;
			}
			else
			{
				char filestem[256];
				fp_out=0;

				get_filestem(p,filestem,sizeof(filestem));

				strncpy(out_file,out_dir,sizeof(out_file));

				if (out_file[0])
				{
					char *q=out_file+strlen(out_file)-1;

					if ((*q=='/')||(*q=='\\')||(*q==':')) *q=0;

#ifdef _PLATFORM_MACINTOSH_
					strncat(out_file,":",sizeof(out_file)-1);
#else
	#ifdef _WIN32
					strncat(out_file,"\\",sizeof(out_file)-1);
	#else
					strncat(out_file,"/",sizeof(out_file)-1);
	#endif
#endif
				}

				strncat(out_file,filestem,sizeof(out_file)-1);
			}

			if (!strcmp(p,"-"))
			{
				fp_in=stdin;
			}
			else
			{
				fp_in=fopen(p,"r");
			}

			if (!fp_in) 
			{
				fprintf(stderr,"failed to open file '%s'\n",p);

				return 1;
			}

			do_filter(fp_in);

			if (fp_in!=stdin)
			{
				fclose(fp_in);
			}

			if (fp_out) 
			{
				if (fp_out != stdout)
				{
					fclose(fp_out);

					fp_out=NULL;
				}

/*				printf("Generated public IDL file '%s'\n",out_file);*/
				out_file[0]=0;
			}
		}
	}

	return 0;
}
