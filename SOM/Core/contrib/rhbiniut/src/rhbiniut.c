/**************************************************************************
 *
 *  Copyright 2012, Roger Brown
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rhbiniut.h>

struct rhbiniData
{
	struct rhbinifile iniFile;
	size_t length,lineCount,stanzaCount;
	size_t *stanzas;
	size_t *lines;
	int usage;
	char *data;
};


static int rhbini_QueryInterface(struct rhbinifile *pSelf,const void *iid,void **ppv)
{
	if (pSelf && iid && ppv)
	{
		*ppv=NULL;
	}

	return -1;
}

static int rhbini_AddRef(struct rhbinifile *pSelf)
{
    struct rhbiniData *pThis=(void *)pSelf;
    
    pThis->usage++;
    
	return 0;
}

static int rhbini_Release(struct rhbinifile *pSelf)
{
    int val=0;
    struct rhbiniData *pThis=(void *)pSelf;
    
    val=--(pThis->usage);
    
    if (!val)
    {
        if (pThis->stanzas) free(pThis->stanzas);
        if (pThis->lines) free(pThis->lines);
        if (pThis->data) free(pThis->data);
        free(pThis);
    }
    
	return 0;
}

static const char *rhbini_GetValue(struct rhbinifile *pSelf,const char *name,const char *stanza)
{
    struct rhbiniData *pThis=(void *)pSelf;
    const char *value=NULL;
    size_t i=0;
    
    while (i < pThis->stanzaCount)
    {
        size_t line=pThis->stanzas[i++];
        
        if (!strcmp(stanza,pThis->data+pThis->lines[line]))
        {
            size_t endOfStanza=pThis->lineCount;
            size_t nameLen=strlen(name);
            
            if (i < pThis->stanzaCount)
            {
                endOfStanza=pThis->stanzas[i];
            }
            
            line++;
            
            while (line < endOfStanza)
            {
                const char *p=pThis->data+pThis->lines[line];
                
                while ((*p==' ')||(*p=='\t')) p++;
                
                if ((!memcmp(p,name,nameLen))&&(p[nameLen]=='='))
                {
                    value=p+nameLen+1;
                    
                    break;
                }
                
                line++;
            }
            
            break;
        }
    }
    
	return value;
}

static struct rhbinifileVtbl rhbiniVtbl={rhbini_QueryInterface,rhbini_AddRef,rhbini_Release,rhbini_GetValue};

static size_t countLines(size_t *pLines,char *p,size_t len)
{
    size_t n=0,o=0;
    
    while (o < len)
    {
        /* get past white space */
        
        while ((o < len)&&((p[o]==' ')||(p[o]=='\t')))
        {
            o++;
        }
    
        /* if not empty or comment then add as legitimate line */
        
        if ((p[o]!=';')&&(p[o]!='\n')&&(p[0]!='#'))
        {
            n++;
        
            if (pLines)
            {
                *pLines++=o;
            }
        }
        
        /* find end of line */
    
        while (o < len)
        {
            char c=p[o++];
        
            if ((c=='\\')&&(o<len))
            {
                c=p[o++];
            }
            else
            {
                if (c=='\n')
                {
                    c=0;
                
                    p[o-1]=c;
                }
            }
        
            if (!c)
            {
                break;
            }
        }
    }
    
    n++;
    
    if (pLines)
    {
        *pLines++=o;
    }
        
    return n;
}

static size_t countStanzas(struct rhbiniData *f)
{
    size_t n=0,i=0,c=f->lineCount;
    size_t *l=f->lines;
    size_t *s=f->stanzas;
    const char *d=f->data;
    
    while (i < c)
    {
        if (d[l[i]]=='[')
        {
            if (s)
            {
                *s++=i;
            }
            
            n++;
        }
        
        i++;
    }
    
    return n;
}

struct rhbinifileCreation
{
	struct rhbiniData *iniFile;
	FILE *fp;
};

RHBOPT_cleanup_begin(rhbinifile_cleanup,pv)

struct rhbinifileCreation *data=pv;

	if (data->fp) 
	{
		fclose(data->fp);

		data->fp=NULL;
	}

	if (data->iniFile)
	{
		struct rhbinifile *result=&(data->iniFile->iniFile);
		data->iniFile=NULL;

		result->lpVtbl->Release(result);

	}

RHBOPT_cleanup_end

struct rhbinifile *rhbinifile(const char *path)
{
	struct rhbinifile *result=NULL;
	struct rhbinifileCreation data={NULL,NULL};

	RHBOPT_cleanup_push(rhbinifile_cleanup,&data);

	data.fp=fopen(path,"r");

	if (data.fp)
	{
		long len=0,zero=0;
		fseek(data.fp,zero,SEEK_END);
		len=ftell(data.fp);
		fseek(data.fp,zero,SEEK_SET);

		if (len > 2)
		{
			struct rhbiniData *iniFile=malloc(sizeof(*iniFile)+len);
	
			data.iniFile=iniFile;

			iniFile->iniFile.lpVtbl=&rhbiniVtbl;
			iniFile->stanzas=NULL;
			iniFile->usage=1;
            iniFile->lines=NULL;
            iniFile->lineCount=0;
            iniFile->stanzaCount=0;
            iniFile->data=malloc(len+1);
			iniFile->length=fread(iniFile->data,1,len,data.fp);
            iniFile->data[iniFile->length]=0;
            
            iniFile->lineCount=countLines(NULL,iniFile->data,iniFile->length);
            iniFile->lines=calloc(iniFile->lineCount,sizeof(iniFile->lines[0]));
            iniFile->lineCount=countLines(iniFile->lines,iniFile->data,iniFile->length);

#if 0
            {
                size_t i=0;
                while (i < iniFile->lineCount)
                {
                    size_t off=iniFile->lines[i];
                    const char *p=iniFile->data+off;
                    
                    printf("L=%ld O=%ld %s\n",i,off,p);
                    
                    i++;
                }
            }
#endif

            iniFile->stanzaCount=countStanzas(iniFile);
            
            if (iniFile->stanzaCount)
            {
                iniFile->stanzas=calloc(iniFile->stanzaCount,sizeof(iniFile->stanzas[0]));
                
                iniFile->stanzaCount=countStanzas(iniFile);
            }

#if 0            
            {
                size_t i=0;
                
                while (i < iniFile->stanzaCount)
                {
                    size_t line=iniFile->stanzas[i];
                    const char *p=iniFile->data+iniFile->lines[line];
                    printf("%ld %s\n",i,p);
                    i++;
                }
            }
#endif
		}

		if (data.iniFile)
		{
			result=&(data.iniFile->iniFile);
			data.iniFile=NULL;
		}
	}

	RHBOPT_cleanup_pop();

	return result;
}



