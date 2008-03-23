/*
 *         mzloader
 *   Copyright (C) 1995 Wayne Meissner
 *   Copyright (C) 1996  Martin von Löwis
 *
 * Profile functions taken from profile.c
 *
 * Copyright 1993 Miguel de Icaza
 * Copyright 1996 Alexandre Julliard
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef	int	BOOL;
typedef int INT;

/*
 * Old MZ header for DOS programs.  Actually just a couple of fields
 * from it, so that we can find the start of the NE header.
 */
struct mz_header_s
{
    WORD mz_magic;         /* MZ Header signature */
    BYTE dont_care[0x3a];  /* MZ Header stuff */
    WORD ne_offset;        /* Offset to extended header */
};

/*
 * This is the Windows executable (NE) header.
 */
struct ne_header_s
{
    WORD  ne_magic;             /* 00 NE signature 'NE' */
    BYTE  linker_version;   /* 02 Linker version number */
    BYTE  linker_revision;  /* 03 Linker revision number */
    WORD  entry_tab_offset; /* 04 Offset to entry table relative to NE */
    WORD  entry_tab_length; /* 06 Length of entry table in bytes */
    DWORD reserved1;        /* 08 Reserved by Microsoft */
    WORD  format_flags;         /* 0c Flags about segments in this file */
    WORD  auto_data_seg;    /* 0e Automatic data segment number */
    WORD  local_heap_length;    /* 10 Initial size of local heap */
    WORD  stack_length;         /* 12 Initial size of stack */
    WORD  ip;           /* 14 Initial IP */
    WORD  cs;           /* 16 Initial CS */
    WORD  sp;           /* 18 Initial SP */
    WORD  ss;           /* 1a Initial SS */
    WORD  n_segment_tab;    /* 1c # of entries in segment table */
    WORD  n_mod_ref_tab;    /* 1e # of entries in module reference tab. */
    WORD  nrname_tab_length;    /* 20 Length of nonresident-name table     */
    WORD  segment_tab_offset;   /* 22 Offset to segment table */
    WORD  resource_tab_offset;  /* 24 Offset to resource table */
    WORD  rname_tab_offset; /* 26 Offset to resident-name table */
    WORD  moduleref_tab_offset; /* 28 Offset to module reference table */
    WORD  iname_tab_offset; /* 2a Offset to imported name table */
    DWORD nrname_tab_offset;    /* 2c Offset to nonresident-name table */
    WORD  n_mov_entry_points;   /* 30 # of movable entry points */
    WORD  align_shift_count;    /* 32 Logical sector alignment shift count */
    WORD  n_resource_seg;   /* 34 # of resource segments */
    BYTE  operating_system; /* 36 Flags indicating target OS */
    BYTE  additional_flags; /* 37 Additional information flags */
    WORD  fastload_offset;  /* 38 Offset to fast load area */
    WORD  fastload_length;  /* 3a Length of fast load area */
    WORD  reserved2;        /* 3c Reserved by Microsoft */
    WORD  expect_version;   /* 3e Expected Windows version number */
};


#define MZ_SIGNATURE  ('M' | ('Z' << 8))
#define NE_SIGNATURE  ('N' | ('E' << 8))
#define PE_SIGNATURE  ('P' | ('E' << 8))

#define FALSE	0
#define TRUE	1

#define min(a,b) ((a) <= (b) ? (a) : (b))

static char * def_wine_args[]={ "wine" ,NULL };
static char WinePath[PATH_MAX];
static char DosEmuPath[PATH_MAX];
static char RcOptions[256];
static char **RcCommand;

int is_nepe(FILE *f)
{
	struct mz_header_s mz_header;
	struct ne_header_s ne_header;
	fseek(f,0,SEEK_SET);
	if(fread(&mz_header,1,sizeof(mz_header),f)!=sizeof(mz_header))
		return 0;
	if(mz_header.mz_magic != MZ_SIGNATURE)
		return 0;
	fseek(f,mz_header.ne_offset,SEEK_SET);
	if(fread(&ne_header,1,sizeof(ne_header),f)!=sizeof(ne_header))
		return 0;
	if(ne_header.ne_magic == PE_SIGNATURE) return 1;
	if(ne_header.ne_magic == NE_SIGNATURE) return 1;
	return 0;
}

char *combine_command_line(int argc,char* argv[])
{
	int i,len;
	char *result;
	
	for(i=0,len=0;i<argc;i++)
		len+=strlen(argv[i])+1;
	result=(char*)malloc(len+1);
	result[0]='\0';
	for(i=0;i<argc;)
	{
		strcat(result,argv[i++]);
		if(i < argc)
			strcat(result," ");
	}
	return result;
}

char **split_command_line(char *in)
{
	char **ret,*it;
	int count;
	for(count=2,it=in;(it=strchr(it,' '));count++)
		it=it+1;
	ret=malloc(sizeof(char*)*count);
	for(count=0,it=in;it;count++)
	{
		ret[count]=it;
		it=strchr(it,' ');
		if(it)
			*(it++)='\0';
	}
	ret[count]='\0';
	return ret;
}

void exec_wine(int argc,char *argv[])
{
	int argc_out;
	char **argv_out;
	char **wine_command_line;
	char *prog_command_line;
	int cl,i;
	wine_command_line = RcCommand ? RcCommand : def_wine_args;

	for(cl=0;wine_command_line[cl];cl++)
		/*Nothing*/;
	prog_command_line=combine_command_line(argc,argv);

	argc_out=cl+1;
	argv_out=(char**)malloc(sizeof(char*)*(argc_out+1));

	for(i=0;i<cl;i++)
		argv_out[i]=wine_command_line[i];
	argv_out[i]=prog_command_line;
	argv_out[i+1]=0;

	execvp(WinePath,argv_out);
}

void exec_dosemu(int argc,char **argv)
{
	execlp (DosEmuPath,    /* dosemu itself */
		argv[0],  /* so the dos program shows up in the ps listing */
		"-d",     /* detach to a new terminal */
		"-E",     /* dos execute program switch */
		argv[0],  /* the prog to execute */
		(char *)NULL);
}

/********************************************************************
 *   profile functions
 */

#define MZRC_GLOBAL	"/usr/local/etc/mzrc"

typedef struct tagPROFILEKEY
{
    char                  *name;
    char                  *value;
    struct tagPROFILEKEY  *next;
} PROFILEKEY;

typedef struct tagPROFILESECTION
{
    char                       *name;
    struct tagPROFILEKEY       *key;
    struct tagPROFILESECTION   *next;
} PROFILESECTION; 


typedef struct
{
    int              changed;
    PROFILESECTION  *section;
    char            *dos_name;
} PROFILE;


/* Cached profile file */
static PROFILE CurProfile = { FALSE, NULL, NULL };

/* wine.ini profile content */
static PROFILESECTION *WineProfile;

#define PROFILE_MAX_LINE_LEN   1024

/* Wine profile name in $HOME directory; must begin with slash */
static const char PROFILE_MzRcName[] = "/.mzrc";

/* Check for comments in profile */
#define IS_ENTRY_COMMENT(str)  ((str)[0] == ';')


/***********************************************************************
 *           PROFILE_CopyEntry
 *
 * Copy the content of an entry into a buffer, removing quotes, and possibly
 * translating environment variables.
 */
static void PROFILE_CopyEntry( char *buffer, const char *value, int len,
                               int handle_env )
{
    char quote = '\0';
    const char *p;

    if ((*value == '\'') || (*value == '\"'))
    {
        if (value[1] && (value[strlen(value)-1] == *value)) quote = *value++;
    }

    if (!handle_env)
    {
        strncpy( buffer, value, len );
        if (quote && (len >= strlen(value))) buffer[strlen(buffer)-1] = '\0';
        return;
    }

    for (p = value; (*p && (len > 1)); *buffer++ = *p++, len-- )
    {
        if ((*p == '$') && (p[1] == '{'))
        {
            char env_val[1024];
            const char *env_p;
            const char *p2 = strchr( p, '}' );
            if (!p2) continue;  /* ignore it */
            strncpy( env_val, p + 2, min( sizeof(env_val), (int)(p2-p)-1 ) );
            if ((env_p = getenv( env_val )) != NULL)
            {
                strncpy( buffer, env_p, len );
                buffer += strlen( buffer );
                len -= strlen( buffer );
            }
            p = p2 + 1;
        }
    }
    *buffer = '\0';
}


/***********************************************************************
 *           PROFILE_Free
 *
 * Free a profile tree.
 */
static void PROFILE_Free( PROFILESECTION *section )
{
    PROFILESECTION *next_section;
    PROFILEKEY *key, *next_key;

    for ( ; section; section = next_section)
    {
        if (section->name) free( section->name );
        for (key = section->key; key; key = next_key)
        {
            next_key = key->next;
            if (key->name) free( key->name );
            if (key->value) free( key->value );
            free( key );
        }
        next_section = section->next;
        free( section );
    }
}


/***********************************************************************
 *           PROFILE_Load
 *
 * Load a profile tree from a file.
 */
static PROFILESECTION *PROFILE_Load( FILE *file )
{
    char buffer[PROFILE_MAX_LINE_LEN];
    char *p, *p2;
    int line = 0;
    PROFILESECTION *section, *first_section;
    PROFILESECTION **prev_section;
    PROFILEKEY *key, **prev_key;

    first_section = (PROFILESECTION *)malloc( sizeof(*section) );
    first_section->name = NULL;
    first_section->key  = NULL;
    first_section->next = NULL;
    prev_section = &first_section->next;
    prev_key     = &first_section->key;

    while (fgets( buffer, PROFILE_MAX_LINE_LEN, file ))
    {
        line++;
        p = buffer + strlen(buffer) - 1;
        while ((p > buffer) && ((*p == '\n') || isspace(*p))) *p-- = '\0';
        p = buffer;
        while (*p && isspace(*p)) p++;
        if (*p == '[')  /* section start */
        {
            if (!(p2 = strrchr( p, ']' )))
            {
                fprintf( stderr, "PROFILE_Load: Invalid section header at line %d: '%s'\n",
                         line, p );
            }
            else
            {
                *p2 = '\0';
                p++;
                section = (PROFILESECTION *)malloc( sizeof(*section));
                section->name = strdup( p );
                section->key  = NULL;
                section->next = NULL;
                *prev_section = section;
                prev_section  = &section->next;
                prev_key      = &section->key;
                continue;
            }
        }
        if ((p2 = strchr( p, '=' )) != NULL)
        {
            char *p3 = p2 - 1;
            while ((p3 > p) && isspace(*p3)) *p3-- = '\0';
            *p2++ = '\0';
            while (*p2 && isspace(*p2)) p2++;
        }
        key = (PROFILEKEY *)malloc( sizeof(*key) );
        key->name  = strdup( p );
        key->value = p2 ? strdup( p2 ) : NULL;
        key->next  = NULL;
        *prev_key  = key;
        prev_key = &key->next;
    }
    return first_section;
}


/***********************************************************************
 *           PROFILE_Find
 *
 * Find a key in a profile tree, optionally creating it.
 */
static PROFILEKEY *PROFILE_Find( PROFILESECTION **section,
                                 const char *section_name,
                                 const char *key_name, int create )
{
    while (*section)
    {
        if ((*section)->name && !strcasecmp( (*section)->name, section_name ))
        {
            PROFILEKEY **key = &(*section)->key;
            while (*key)
            {
                if (!strcasecmp( (*key)->name, key_name )) return *key;
                key = &(*key)->next;
            }
            if (!create) return NULL;
            *key = (PROFILEKEY *)malloc( sizeof(PROFILEKEY) );
            (*key)->name  = strdup( key_name );
            (*key)->value = NULL;
            (*key)->next  = NULL;
            return *key;
        }
        section = &(*section)->next;
    }
    if (!create) return NULL;
    *section = (PROFILESECTION *)malloc( sizeof(PROFILESECTION) );
    (*section)->name = strdup(section_name);
    (*section)->next = NULL;
    (*section)->key  = (PROFILEKEY *)malloc( sizeof(PROFILEKEY) );
    (*section)->key->name  = strdup( key_name );
    (*section)->key->value = NULL;
    (*section)->key->next  = NULL;
    return (*section)->key;
}


/***********************************************************************
 *           PROFILE_Open
 *
 * Open a profile file, checking the cached file first.
 */
static BOOL PROFILE_Open( const char *filename )
{
    char buffer[PATH_MAX];
    const char *dos_name, *unix_name;
    char *newdos_name, *p;
    FILE *file = NULL;

	dos_name = filename;

    if (CurProfile.dos_name && !strcmp( dos_name, CurProfile.dos_name ))
    {
        return TRUE;
    }

    /* Flush the previous profile */

    newdos_name = strdup( dos_name );
    PROFILE_Free( CurProfile.section );
    if (CurProfile.dos_name) free( CurProfile.dos_name );
    CurProfile.section   = NULL;
    CurProfile.dos_name  = newdos_name;

    /* Try to open the profile file, first in $HOME/.wine */

    /* FIXME: this will need a more general solution */
    if ((p = getenv( "HOME" )) != NULL)
    {
        strcpy( buffer, p );
        strcat( buffer, "/.wine/" );
        p = buffer + strlen(buffer);
        strcpy( p, strrchr( newdos_name, '\\' ) + 1 );
        file = fopen( buffer, "r" );
    }

    if (!file && (unix_name = dos_name))
    {
        file = fopen( unix_name, "r" );
    }

    if (file)
    {
        CurProfile.section = PROFILE_Load( file );
        fclose( file );
    }
    else
        fprintf( stderr, "Warning: profile file %s not found\n", newdos_name );
    return TRUE;
}


/***********************************************************************
 *           PROFILE_GetSection
 *
 * Enumerate all the keys of a section.
 */
static INT PROFILE_GetSection( PROFILESECTION *section,
                               const char *section_name,
                               char *buffer, INT len, int handle_env )
{
    PROFILEKEY *key;
    while (section)
    {
        if (section->name && !strcasecmp( section->name, section_name ))
        {
            INT oldlen = len;
            for (key = section->key; key; key = key->next)
            {
                if (len <= 2) break;
                if (IS_ENTRY_COMMENT(key->name)) continue;  /* Skip comments */
                PROFILE_CopyEntry( buffer, key->name, len - 1, handle_env );
                len -= strlen(buffer) + 1;
                buffer += strlen(buffer) + 1;
            }
            *buffer = '\0';
            return oldlen - len + 1;
        }
        section = section->next;
    }
    buffer[0] = buffer[1] = '\0';
    return 2;
}


/***********************************************************************
 *           PROFILE_GetString
 *
 * Get a profile string.
 */
static INT PROFILE_GetString( const char *section, const char *key_name,
                              const char *def_val, char *buffer, INT len )
{
    PROFILEKEY *key = NULL;

    if (!def_val) def_val = "";
    if (key_name)
    {
        key = PROFILE_Find( &CurProfile.section, section, key_name, FALSE );
        PROFILE_CopyEntry( buffer, (key && key->value) ? key->value : def_val,
                           len, FALSE );
        return strlen( buffer );
    }
    return PROFILE_GetSection(CurProfile.section, section, buffer, len, FALSE);
}


/***********************************************************************
 *           PROFILE_GetMzRcString
 *
 * Get a config string from the wine.ini file.
 */
int PROFILE_GetMzRcString( const char *section, const char *key_name,
                              const char *def, char *buffer, int len )
{
    if (key_name)
    {
        PROFILEKEY *key = PROFILE_Find(&WineProfile, section, key_name, FALSE);
        PROFILE_CopyEntry( buffer, (key && key->value) ? key->value : def,
                           len, TRUE );
        return strlen( buffer );
    }
    return PROFILE_GetSection( WineProfile, section, buffer, len, TRUE );
}


/***********************************************************************
 *           PROFILE_LoadMzRc
 *
 * Load the wine.ini file.
 */
int PROFILE_LoadMzRc(void)
{
    char buffer[PATH_MAX];
    const char *p;
    FILE *f;

    if ((p = getenv( "HOME" )) != NULL)
    {
        strncpy( buffer, p, PATH_MAX - sizeof(PROFILE_MzRcName) );
        strcat( buffer, PROFILE_MzRcName );
        if ((f = fopen( buffer, "r" )) != NULL)
        {
            WineProfile = PROFILE_Load( f );
            fclose( f );
            return 1;
        }
    }
    else fprintf( stderr, "Warning: could not get $HOME value for config file.\n" );

    /* Try global file */

    if ((f = fopen( MZRC_GLOBAL, "r" )) != NULL)
    {
        WineProfile = PROFILE_Load( f );
        fclose( f );
        return 1;
    }
    fprintf( stderr, "Can't open configuration file %s or $HOME%s\n",
             MZRC_GLOBAL, PROFILE_MzRcName );
    return 0;
}


/************************************************************************
 *           main
 */

int main (int argc, char *argv[]) {
	FILE *f;
	char name[255],*it;
	PROFILE_LoadMzRc();
	PROFILE_GetMzRcString("emulators","wine","wine",WinePath,sizeof(WinePath));
	PROFILE_GetMzRcString("emulators","dos","dos",DosEmuPath,sizeof(DosEmuPath));
	/* Get program name */
	it = strrchr(argv[0],'/');
	it = it ? (it+1) : argv[0];
	strcpy(name,it);
	it = strrchr(name, '.');
	if(it) *it= '\0';
	PROFILE_GetMzRcString(name,"command","",RcOptions,sizeof(RcOptions));
	if(RcOptions[0])
	{
		RcCommand = split_command_line(RcOptions);
		if(strcmp(RcCommand[0],"wine")==0)
			exec_wine(argc,argv);
		else
			exec_dosemu(argc,argv);
	}
	f=fopen(argv[0],"r");
	if(!f)
	{
		perror("mzloader");
		return 1;
	}
	if(is_nepe(f))
		exec_wine(argc,argv);
	else
		exec_dosemu(argc,argv);
	return 1;
}
