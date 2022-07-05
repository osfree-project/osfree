#include <rhbopt.h>

/* IDL compiler */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <rhbsc.h>

RHBtextfile::RHBtextfile(const char *n)
{
	fp=0;
	filename=0;

	if (n)
	{
		size_t x=strlen(n)+1;
		filename=new char[x];
		strncpy(filename,n,x);
	}
	else
	{
		fp=stdout;
	}

/*	fp=fopen(n,"w");

	if (!fp) 
	{
		fprintf(stderr,"failed to open %s for writing\n",n);
		exit(1);
	}*/
}

RHBtextfile::~RHBtextfile()
{
	if (fp)
	{
		fflush(fp);

		if (fp!=stdout)
		{
			fclose(fp);
		}
		fp=0;
	}
	if (filename)
	{
		delete []filename;
		filename=0;
	}
}

static long cr_count;

void RHBtextfile::write(const void *pn,size_t len)
{
	const char *n=(const char *)pn;

	if (!fp)
	{
		fp=fopen(filename,"w");

		if (!fp)
		{
			fprintf(stderr,"Failed to open '%s' for writing\n",filename);
			exit(1);
		}
	}

	if (len > 0)
	{
		if (fp)
		{
			fwrite(n,len,1,fp);

			if (len--)
			{
				if (*n=='\n')
				{
					cr_count++;

					if (cr_count > 4)
					{
						cr_count=0;
						printf("Bad formatting\n");
					}
				}
				else
				{
					cr_count=0;
				}

				n++;
			}
		}
	}
}
