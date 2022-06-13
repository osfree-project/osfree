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

#ifdef _WIN32
#	define USE_PROTO
#	define HAVE_SNPRINTF
#	define snprintf _snprintf
#	ifdef _WIN32_WCE
	static FILE *popen(const char *f,const char *m) { return NULL; }
	static int pclose(FILE *f) { return 0; }
#	define getenv(x)   (NULL)
#	else
	static FILE *popen(const char *f,const char *m) { return _popen(f,m);}
	static int pclose(FILE *f) { return _pclose(f); }
#endif
#else
#	define USE_PROTO
#	include <popen.h>
#endif

typedef const char *const_char_ptr;

/****************************************************************************
 * because we are linked to the file, it will tell us where it is!   
 */

static const char *config2_c;
static const char *config2_cpp;

/*********************************************************************************
 * Variable                 Windows								OS/2
 * ===============================================================================
 * CONFIG_H				../../include/$(PLATFORM)/$(BUILDTYPE)/config.h
 * CONFIG_HPP			../../include/$(PLATFORM)/$(BUILDTYPE)/config.hpp
 * CONFIG_CC_EP		cl.exe /EP /nologo							lcc.exe
 * CONFIG_CXX_EP	cl.exe /EP /nologo /I../src					lcc.exe -I../src
 * CONFIG_CC		cl.exe /nologo								bcc.exe -w -g1				
 * CONFIG_CXX		cl.exe /nologo								bcc.exe -I../src
 * CONFIG_LINKOPTS	/Fea.out /link /INCREMENTAL:NO /PDB:NONE	-ea.out
 * CONFIG_LOG		config.log									config.log
 */

static const char *sz_config_h="config.h";
static const char *sz_config_hpp="config.hpp";
static const char *sz_config_log="config.log";

static const char *cc_e_p="cpp";
static const char *cxx_e_p="cpp -I../src";
static const char *cc="cc";
static const char *cxx="CC -I../src";
static const char *link_opts=NULL;

static struct {
	const_char_ptr name;
	const_char_ptr *value;
} envmap[]={
	{"CONFIG_LOG",&sz_config_log},
	{"CONFIG_H",&sz_config_h},
	{"CONFIG_HPP",&sz_config_hpp},
	{"CONFIG_CC_EP",&cc_e_p},
	{"CONFIG_CXX_EP",&cxx_e_p},
	{"CONFIG_CC",&cc},
	{"CONFIG_CXX",&cxx},
	{"CONFIG2_C",&config2_c},
	{"CONFIG2_CPP",&config2_cpp},
	{"CONFIG_LINKOPTS",&link_opts}
};

struct option
{
	struct option *next;
	char *name;
	char *value;
	char buf[1];
};

static int read_opt
#ifdef USE_PROTO
(struct option **h,char *buf)
#else
(h,buf) struct option **h; char *buf;
#endif
{
	int namelen=0,textlen=0;
	char *nameptr=NULL;
	char *textptr=NULL;
	struct option *opt=NULL;

	while (*buf)
	{
		if ((*buf==' ')||(*buf=='\t')) buf++;
		else break;
	}
	if (!*buf) return 0;
	nameptr=buf;
	while (buf[namelen] > ' ')
	{
		namelen++;
	}
	buf+=namelen;
	while (*buf)
	{
		if ((*buf==' ')||(*buf=='\t')) buf++;
		else break;
	}
	if (*buf!='\"') return 0;
	textptr=++buf;
	while ((buf[textlen]!='\"')&&(buf[textlen]))
	{
		textlen++;
	}
	opt=(struct option *)malloc(sizeof(*opt)+textlen+namelen);
	opt->next=NULL;
	opt->name=opt->buf;
	opt->value=opt->buf+namelen+1;
	memcpy(opt->name,nameptr,namelen);
	opt->name[namelen]=0;
	memcpy(opt->value,textptr,textlen);
	opt->value[textlen]=0;
	if (*h)
	{
		struct option *p=*h;
		while (p->next) p=p->next;
		p->next=opt;
	}
	else
	{
		*h=opt;
	}
	return 0;
}

struct config
{
	FILE *fpLog;
	const char *config_h;
	struct option *opt_list;
	int cplusplus;
};

static int config_opt
#ifdef USE_PROTO
(struct config *cfg,struct option *opt,int argc,char **argv)
#else
(cfg,opt,argc,argv)
struct config *cfg;
struct option *opt;
int argc;
char **argv;
#endif
{
	int rc=1;
	char buf[16384];
	FILE *fp=NULL;

#ifdef HAVE_SNPRINTF
	snprintf(buf,sizeof(buf),"%s -DTRY_%s %s",
		cfg->cplusplus ? cxx : cc,
		opt->name,
		cfg->cplusplus ? "-DHAVE_CONFIG_HPP" : "-DHAVE_CONFIG_H");
#else
	sprintf(buf,"%s -DTRY_%s %s",
		cfg->cplusplus ? cxx : cc,
		opt->name,
		cfg->cplusplus ? "-DHAVE_CONFIG_HPP" : "-DHAVE_CONFIG_H");
#endif

#ifdef CC_LIBPATH
	strcat(buf," /LIBPATH:");
	strcat(buf,CC_LIBPATH);
#endif

	if (argc > 1)
	{
		int i=1;

		while (i < argc)
		{
			strncat(buf," ",sizeof(buf));
			strncat(buf,argv[i],sizeof(buf));
			i++;
		}
	}

	strcat(buf," ");
	strcat(buf,cfg->cplusplus ? config2_cpp : config2_c);

	if (link_opts && link_opts[0])
	{
		strncat(buf," ",sizeof(buf));
		strncat(buf,link_opts,sizeof(buf));
	}

	printf("%s ...",opt->value);

	if (cfg->fpLog) fprintf(cfg->fpLog,"%s\n",buf);

#if 0
	printf("exec(%s)",buf);
#endif

	fp=popen(buf,"r");

	if (fp)
	{
		while (fgets(buf,sizeof(buf),fp))
		{
			const char *p="INVALID_OPTION";
			int i=strlen(p);
			int j=strlen(buf)-i;
			if (j > 0)
			{
				while (j--)
				{
					if (!memcmp(buf+j,p,i))
					{
						fprintf(stderr,"%s -> %s",
							opt->name,
							buf);

						if (cfg->fpLog)
						{
							fprintf(cfg->fpLog,"%s -> %s",opt->name,buf);
							fclose(cfg->fpLog);
						}

						exit(1);
					}
				}
			}
			if (cfg->fpLog) fprintf(cfg->fpLog,"%s",buf);
		}

		rc=pclose(fp);
	}

	if (rc)
	{
		printf(" no\n");
	}
	else
	{
		printf(" %s\n",opt->name);

		fp=fopen(cfg->config_h,"a");

		if (fp)
		{
			fprintf(fp,"#ifndef %s\n",opt->name);
			fprintf(fp,"#	define %s 1\n",opt->name);
			fprintf(fp,"#endif\n");
			fclose(fp);
		}
		else
		{
			fprintf(stderr,"failed to open %s for writing\n",cfg->config_h);
			fflush(stderr);
			rc=1;
		}
	}

	return rc;
}

static int config
#ifdef USE_PROTO
(struct config *cfg,int argc,char **argv)
#else
(cfg,argc,argv)
struct config *cfg;
int argc;
char **argv;
#endif
{
char buf[16384];
FILE *fp=NULL;

#ifdef HAVE_SNPRINTF
	snprintf(buf,sizeof(buf),
		"%s -DLIST_OPTIONS %s",
		cfg->cplusplus ? cxx_e_p : cc_e_p,
		cfg->cplusplus ? config2_cpp : config2_c);
#else
	sprintf(buf,
		"%s -DLIST_OPTIONS %s",
		cfg->cplusplus ? cxx_e_p : cc_e_p,
		cfg->cplusplus ? config2_cpp : config2_c);
#endif

#if 0
	fprintf(stderr,"popen: %s\n",buf);
	fflush(stderr);
#endif

	fp=popen(buf,"r");

	if (fp)
	{
		while (fgets(buf,sizeof(buf),fp))
		{
			read_opt(&cfg->opt_list,buf);
		}
		pclose(fp);
	}

	if (cfg->opt_list)
	{
		struct option *opt=cfg->opt_list;

		fp=fopen(cfg->config_h,"w");
		if (fp)	fclose(fp);

		while (opt)
		{
			config_opt(cfg,opt,argc,argv);

			opt=opt->next;
		}
	}

	while (cfg->opt_list)
	{
		struct option *o=cfg->opt_list;
		cfg->opt_list=cfg->opt_list->next;
		free((char *)o);
	}

	return 0;
}

#ifdef USE_PROTO
int main(int argc,char **argv)
#else
int main(argc,argv)
int argc;
char **argv;
#endif /* USE_PROTO */
{
struct config cfg_data={NULL,NULL,NULL,0};
int i=sizeof(envmap)/sizeof(envmap[0]);

	while (i--)
	{
		const char *p=getenv(envmap[i].name);

		if (p && p[0])
		{
			envmap[i].value[0]=p;
		}
	}

	if (!config2_c)
	{
		fprintf(stderr,"No CONFIG2_C\n");
		return 1;
	}

	if (!config2_cpp)
	{
		fprintf(stderr,"No CONFIG2_CPP\n");
		return 1;
	}

	cfg_data.fpLog=fopen(sz_config_log,"w");
	cfg_data.opt_list=NULL;
	cfg_data.config_h=sz_config_h;

	config(&cfg_data,argc,argv);

	cfg_data.config_h=sz_config_hpp;

	cfg_data.cplusplus=1;

	config(&cfg_data,argc,argv);

	if (cfg_data.fpLog) fclose(cfg_data.fpLog);

	return 0;
}
