/**************************************************************************
 *
 *  Copyright 1998-2010, Roger Brown
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

#include <rhbopt.h>
/* pre-processor */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rhbsc.h>

RHBStream::RHBStream()
{
}

RHBStream::~RHBStream()
{
}

RHBFile::RHBFile(int _argc,char **_argv,FILE *fpIn) :
argc(_argc),
argv(_argv),
fp(NULL)
{
	un_got=0;
	filename[0]=0;
	line_number=1;

	if (argc)
	{
		const char *name=*argv++;
		argc--;

		fp=fopen(name,"r");

		if (fp)
		{
			strncpy(filename,name,sizeof(filename));
		}
		else
		{
			fprintf(stderr,"failed to open \"%s\"\n",name);
			exit(1);
		}
	}
	else
	{
		fp=fpIn;
	}

	incl=0;

#ifdef _WIN32
	if (!fp) bomb("missing file");
#endif

/*	if (fp) printf("--%s--\n",name);*/
}

RHBPreprocessor::RHBPreprocessor()
{
	scope=0;
	error_count=0;
	emit_root=RHBshould_emit_default;
	lText=0;
	stream_level=0;
	in_text=0;
	includes=new RHBinclude_list(this,"include");
}

int RHBPreprocessor::get_next_token(char *buf)
{
	char c;

	while (currStream())
	{
		c=currStream()->get_char();

		if (lText)
		{
			if (isalnum(c) || (c=='_'))
			{
				text[lText]=c;
				lText++;
			}
			else
			{
				currStream()->un_get(c);
				break;
			}
		}
		else
		{
			if (isalnum(c) || (c=='_'))
			{
				text[lText]=c;
				lText++;
			}
			else
			{
				if ((c=='*')||
					(c==';')||
					(c==':')||
					(c==',')||
					(c=='.')||
					(c=='=')||
					(c=='{')||
					(c=='}')||
					(c=='[')||
					(c==']')||
					(c=='>')||
					(c=='<')||
					(c=='(')||
					(c==')')||
					(c=='+')||
					(c=='&')||
					(c=='|')||
					(c=='!')||
					(c=='-'))
				{
					text[lText]=c;
					lText++;

					if (c==':') /* this was a change!! */
					{
						c=currStream()->get_char();

						if (c==':')
						{
							text[lText]=c;
							lText++;
						}
						else
						{
							currStream()->un_get(c);
						}
					}

					break;
				}
				else
				{
					if (c=='#')
					{
						/* need special case here */

						text[lText++]=c;
					}
					else
					{
						if (c=='\\')
						{
							if (in_text)
							{
								text[lText++]=c;
							}
							else
							{
								do
								{
									c=currStream()->get_char();

								} while ((c!=0)&&(c!='\n')&&(c!='\r'));

								currStream()->un_get(c);
							}
						}
						else
						{
							if ((c==0x22)||(c==0x27))
							{
								int escape=0;

								text[lText++]=c;

								in_text++;

								do
								{
									char d=currStream()->get_char();

									if (!escape)
									{
										if (d=='\\')
										{
											escape=1;
										}
										else
										{
											if (d==c)
											{
												in_text--;
											}

											text[lText++]=d;
										}
									}
									else
									{
										escape=0;
										text[lText++]=d;
									}


								} while (in_text);

								break;
							}
							else
							{
								if ((c=='/')&&(currStream()))
								{
									char c2=currStream()->get_char();

									if (c2=='/')
									{
										while (currStream())
										{
											c=currStream()->get_char();

											if ((c=='\r')||(c=='\n')) break;
										}
									}
									else
									{
										currStream()->un_get(c2);

										text[lText]=c;
										lText++;

										break;
									}
								}
								else
								{
									if ((c==' ')||
										(c=='\t')||
										(c=='\r')||
										(c=='\n')||
										(c==0))
									{
									}
									else
									{
#ifdef _WIN32
										bomb("unexpected character");
#endif
										fprintf(stderr,"unexpected character %d\n",c);
										exit(1);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	memcpy(buf,text,lText);
	buf[lText]=0;

	int l=(int)lText;
	lText=0;

	return l;
}

char RHBFile::get_char()
{
	if (un_got)
	{
		char c=un_got;

		un_got=0;
#ifdef DEBUG_PP
		printf("[%c]",c);
#endif
		return c;
	}

	while (fp)
	{
		char c=0;

		if (fread(&c,1,1,fp))
		{
			if ((c=='\n')||(c=='\r'))
			{
				line_number++;
			}

#ifdef DEBUG_PP
			printf("%c",c);

			if (c=='|')
			{
				printf("<|>");
			}
#endif

			return c;
		}
	
		if (fp!=stdin)
		{
			fclose(fp);
		}

		fp=0;

		if (argc)
		{
			const char *name=*argv++;
			argc--;
			fp=fopen(name,"r");

			if (fp)
			{
				strncpy(filename,name,sizeof(filename));
			}
			else
			{
				fprintf(stderr,"failed to open \"%s\"\n",name);
				exit(1);
			}
		}
	}

	return 0;
}

void RHBFile::un_get(char c)
{
	un_got=c;
}

int RHBFile::is_eof()
{
	if (un_got) return 0;
	if (fp) return 0;
	return 1;
}

char RHBPreprocessor::get_non_whitespace()
{
	char c;

	c=currStream()->get_char();

	switch (c)
	{
		case '\r':
		case '\n':
		case '\t':
		case ' ':
			c=0;
	}
		
	return c;
}

int RHBPreprocessor::get_token(char *buf,size_t buflen)
{
	char b[1024]={0};
#ifdef _WIN32
	if (buflen==sizeof(buf))
	{
		bomb("maximum buffer size");
	}
#endif

	buf[0]=0;

	do
	{
		long l=get_next_token(b);

		if (!l)
		{
			return 0;
		}

		if (b[0]!='#')
		{
			strncpy(buf,b,buflen);

			return (int)l;
		}
		else
		{
			char *p=&b[1];

			if ((!strcmp(p,"line")) || !*p)
			{
				char linenumber[256]={0};
				char filename[1024]={0};
				char optarg[64]={0};
				char *fp=NULL;
				int argc=0;
				char quote=0;

				b[0]=0;

				/* filename within quotes,
				   if has <> when make filename empty 
				   only up to end of line */

				while (currStream())
				{
					int c=currStream()->get_char();

					if (c==EOF) break;

					if ((c=='\r')||(c=='\n'))
					{
						break;
					}

					if (quote)
					{
						if (c==quote)
						{
							quote=0;
							*fp=0;
							fp=NULL;
						}
						else
						{
							*fp++=c;
						}
					}
					else
					{
						if ((c=='\t')||(c==' '))
						{
							if (fp)
							{
								*fp=0;
								fp=NULL;
							}
						}
						else
						{
							if (fp)
							{
								*fp++=c;
							}
							else
							{
								switch (argc)
								{
								case 0: 
									fp=linenumber; 
									break;
								case 1: 
									fp=filename; 
									break;
								case 2: 
									fp=optarg; 
									break;
								default: 
									break;
								}

								argc++;

								if (fp)
								{
									if ((c=='\"')||(c=='\''))
									{
										quote=c;
									}
									else
									{
										*fp++=c;
									}
								}
							}
						}
					}
				}

				if (fp) 
				{
					*fp++=0;
				}

				switch (filename[0]) 
				{
				case '<':
					filename[0]=0;
					break;
				case 0:
					if (currStream())
					{
						RHBFile *f2=currStream()->is_file();
						if (f2)
						{
							const char *p=f2->_get_filename();
							if (p)
							{
								strncpy(filename,p,sizeof(filename));
							}
						}
					}
					break;
				default:
					break;
				}

				if (argc > 1)
				{
#if defined(_WIN32) && 0
					char *p=filename;

					while (*p)
					{
						if (*p=='~') 
						{
							fprintf(stderr,"tilde found in filename\n");
/*							bomb("tilde found");
							break;*/
						}
						p++;
					}

					fprintf(stderr,"#### line \"%s\" file \"%s\" opt=\"%s\" ####\n",
							linenumber,
							filename,
							optarg);
#endif

					RHBinclude_file *current=NULL;

					if (currStream())
					{
						current=currStream()->File();
					}

					RHBinclude_file *f=includes->included(this,filename,current);

					if (currStream())
					{
						RHBFile *f2=currStream()->is_file();

						if (f2)
						{
							f2->incl=f;
							f2->update_file_line(filename,atol(linenumber));
						}
					}
				}
				else
				{
#ifdef _WIN32
					fprintf(stderr,"#### line \"%s\" ####\n",linenumber);
#endif
					if (currStream())
					{
						RHBFile *f2=currStream()->is_file();

						if (f2)
						{
							f2->update_file_line(NULL,atol(linenumber));
						}
					}
				}
			}
			else
			{
				if (!strcmp(p,"pragma"))
				{
					strncpy(buf,b,buflen);

					return (int)l;
				}
				else
				{
					while (currStream())
					{
						int c=currStream()->get_char();

						if (c==EOF) break;

						if ((c=='\r')||(c=='\n'))
						{
							break;
						}
					}
				}
			}
		}

	} while (this);

	return 0;
}

void RHBPreprocessor::pragma_clear()
{
	while (currStream())
	{
		int c=currStream()->get_char();

		if (c==EOF) break;

		if ((c=='\r')||(c=='\n'))
		{
			break;
		}
	}
}

void RHBPreprocessor::err(const char *x,const char *y)
{
	error_count++;
	if (stream_level)
	{
		streamStack[stream_level-1]->print_identity(stderr);
	}
	fprintf(stderr,"Error, '%s' near '%s'\n",x,y);

#ifdef _WIN32
	bomb("error");
#endif

	exit(1);
}

int RHBFile::GetLine()
{
	return line_number;
}

int RHBStream::GetLine()
{
	return 0;
}

RHBStream * RHBPreprocessor::currStream()
{
	while (stream_level)
	{
		RHBStream *s=streamStack[stream_level-1];

		if (!s->is_eof()) return s;
		
		stream_level--;

		streamStack[stream_level]=NULL;
	}

	return 0;
}


void RHBPreprocessor::push_stream(RHBStream *str)
{
	streamStack[stream_level]=str;
	stream_level++;
}

void RHBFile::print_identity(FILE *z)
{
	fprintf(z,"file \042%s\042; line %d\n",filename,line_number);
}

int RHBPreprocessor::CurrentLine()
{
	int i=stream_level;

	while (i--)
	{
		if (streamStack[i]->File())
		{
			int j=streamStack[i]->GetLine();

			if (j) return j;
		}
	}

	return 0;
}

RHBinclude_file * RHBPreprocessor::CurrentFile()
{
	int i=stream_level;

	while (i--)
	{
		RHBinclude_file *f=streamStack[i]->File();

		if (f) return f;
	}

	return 0;
}

RHBinclude_file * RHBFile::File()
{
	return incl;
}

RHBFile *RHBFile::is_file()
{
	return this;
}

void RHBFile::update_file_line(const char *f,int n)
{
	if (f)
	{
		strncpy(filename,f,sizeof(filename));
	}

	if (n>0)
	{
		line_number=n;
	}
}

