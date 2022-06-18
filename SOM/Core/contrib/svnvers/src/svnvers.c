/**************************************************************************
 *
 *  Copyright 2010, Roger Brown
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

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int needQuote(const char *q)
{
	while (*q)
	{
		char c=*q++;

		if (c <= ' ') return 1;
	}

	return 0;
}

void catQuote(char *p,const char *q,size_t n)
{
	int b=needQuote(q);

	if (*p)
	{
		strncat(p," ",n);
	}

	if (b) strncat(p,"\"",n);

	strncat(p,q,n);

	if (b) strncat(p,"\"",n);
}

static const char label[]="Revision:";

int main(int argc,char **argv)
{
	if (argc < 4)
	{
		fprintf(stderr,"usage: %s svn-app file name\n",argv[0]);

		return 1;
	}
	else
	{
		char *svn=argv[1];
		char *file=argv[2];
		char *stem=argv[3];
		long l_version=0;
		struct stat s;
		char dirname[1024];
		int rc=0;

		if (stat(file,&s))
		{
			perror(file);
			return 1;
		}

		strncpy(dirname,file,sizeof(dirname));

		switch(s.st_mode&S_IFMT)
		{
		case S_IFDIR:
			break;
		case S_IFREG:
			{
				size_t i=strlen(dirname);

				while (i--)
				{
					if ((dirname[i]=='\\')||(dirname[i]=='/'))
					{
						dirname[i]=0;
						break;
					}
				}
			}
			break;
		default:
			fprintf(stderr,"%s not a regular file\n",file);
			return 1;
		}

		strncat(dirname,"\\.svn",sizeof(dirname));

		if (!stat(dirname,&s))
		{
			char buf[1024];

			buf[0]=0;

			catQuote(buf,svn,sizeof(buf));
			catQuote(buf,"info",sizeof(buf));
			catQuote(buf,file,sizeof(buf));

			{
				size_t k=strlen(label);
				FILE *fp=_popen(buf,"r");

				if (!fp) 
				{
					perror(buf);

					return 1;
				}

				while (fgets(buf,sizeof(buf),fp))
				{
					size_t i=strlen(buf);
					while (i--)
					{
						if (buf[i]>' ') break;
						buf[i]=0;
					}
					i=strlen(buf);
		
					if ((i > k)&&(!l_version))
					{
						if (!memcmp(label,buf,k))
						{
							const char *p=buf+k;

							while (*p && (*p<=' '))
							{
								p++;
							}

							l_version=atol(p);
		
						}
					}
				}	

				rc=_pclose(fp);
			}
		}

		if (!rc)
		{
			unsigned short low=(unsigned short)l_version,high=(unsigned short)(l_version>>16);

			printf("#define %s_SVN_VER           \".%d.%d\"\n",stem,high,low);
			printf("#define %s_SVN_VER_NUM(x,y)  x,y,%d,%d\n",stem,high,low);
		}

		return rc;
	}

	return 0;
}
