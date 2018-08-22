#include <stdio.h>
#include <string.h>
#include <io.h>
#include <stdarg.h>
#include <time.h>

#ifdef __WATCOMC__
#include <direct.h> /* below: Turbo C dir.h values as comments */
#define MAXPATH  _MAX_PATH  /* 80 */
#define MAXDRIVE _MAX_DRIVE /*  3 */
#define MAXDIR   _MAX_DIR   /* 66 */
#define MAXFILE  _MAX_FNAME /*  9 */
#define MAXEXT   _MAX_EXT   /*  5 */
#define FA_RDONLY _A_RDONLY
#define FA_ARCH _A_ARCH
#define FA_SYSTEM _A_SYSTEM
#define FA_HIDDEN _A_HIDDEN
#define FA_DIREC _A_SUBDIR
#define FA_LABEL _A_VOLID

#define date dosdate_t
#define da_year year
#define da_mon month
#define da_day day
#define ti_hour hour
#define ti_min minute
#define ti_sec second
#define ti_hund hsecond
#define time dostime_t
#define getdate _dos_getdate
#define gettime _dos_gettime
#define setdate _dos_setdate
#define settime _dos_settime

#define ff_name name
#define ff_attrib attrib
#define ff_reserved reserved
#define ff_ftime wr_time
#define ff_date wr_date
#define ff_fsize size

#define r_ax w.ax
#define r_bx w.bx
#define r_cx w.cx
#define r_dx w.dx
#define r_bp w.bp
#define r_si w.si
#define r_di w.di
#define r_ds w.ds
#define r_es w.es
#define r_flags w.flags

#define findfirst(x,y,z) _dos_findfirst(x,z,y)
#define findnext _dos_findnext
#define ffblk find_t

#define setverify(value) bdos(0x2e,0,(value))
#define getverify() bdos(0x54,0,0)&0xff

#define getdfree _dos_getdiskfree
#define dfree diskfree_t
#define df_avail avail_clusters
#define df_sclus sectors_per_cluster
#define df_bsec bytes_per_sector

#define DOS_GETFTIME _dos_getftime
#define DOS_SETFTIME _dos_setftime
#define FAR _far

struct ftime /* As defined by Borland C */
{
    unsigned    ft_tsec  : 5;   /* Two second interval */
    unsigned    ft_min   : 6;   /* Minutes */
    unsigned    ft_hour  : 5;   /* Hours */
    unsigned    ft_day   : 5;   /* Days */
    unsigned    ft_month : 4;   /* Months */
    unsigned    ft_year  : 7;   /* Year */
};

#else
#include <dir.h>
#endif


#ifdef __WATCOMC__ /* "dosdate_t" instead of Borland style "date" ... */
#define THEDATE struct dosdate_t /* in dos.h */
#define da_year year
#define da_mon month
#define da_day day
#define ti_hour hour
#define ti_minute minute
#define ti_sec second
#define ti_hund hsecond
#else
#define THEDATE struct date
#endif

/*-------------------------------------------------------------------------*/
/* COMPILER SPECIFICA                                                      */
/*-------------------------------------------------------------------------*/
#if __DJGPP__
  #include <unistd.h>

  /* disable DJGPP filename globbing, so wildcards are handled the DOS-way */
  char **__crt0_glob_function(char *arg) {
    return 0;
  }

  /* convert DJGPP's _fixpath function into Borland C's _fullpath */
  #define _fullpath(buffer, path, buflen) _fixpath(path, buffer)

  /* directory separator is the UNIX-slash */
  #define DIR_SEPARATOR "/"
#else
  /* directory separator is the DOS-backslash */
  #define DIR_SEPARATOR "\\"

  /* constants for "access" function */
  /* (equal with DJGPP but with different values!) */
  #define F_OK 0  /* exists */
  #define R_OK 4  /* readable */
  #define W_OK 2  /* writeable */
  #define X_OK 1  /* executable */
#endif

int getftime (int handle, struct ftime *ftimep); /* Get a file's time */
int setftime (int handle, struct ftime *ftimep); /* Set a file's time */


/*-------------------------------------------------------------------------*/
/* Works like function strcpy() but stops copying characters into          */
/* destination when the specified maximum number of characters (including  */
/* the terminating null character) is reached to prevent bounds violation. */
/*-------------------------------------------------------------------------*/
char *strmcpy(char *dest,
              const char *src,
              const unsigned int maxlen) {
  unsigned int i, tmp_maxlen;

  tmp_maxlen = maxlen - 1;
  i = 0;
  while ((src[i] != '\0') &&
         (i < tmp_maxlen)) {
    dest[i] = src[i];
    i++;
  }
  dest[i] = '\0';

  return dest;
}


/*-------------------------------------------------------------------------*/
/* Works like function strcat() but stops copying characters into          */
/* destination when the specified maximum number of characters (including  */
/* the terminating null character) is reached to prevent bounds violation. */
/*-------------------------------------------------------------------------*/
char *strmcat(char *dest,
              const char *src,
              const unsigned int maxlen) {
  unsigned int i, tmp_maxlen;
  char *src_ptr;


  tmp_maxlen = maxlen - 1;
  src_ptr = (char *)src;
  i = strlen(dest);
  while ((*src_ptr != '\0') &&
         (i < tmp_maxlen)) {
    dest[i] = *src_ptr;
    src_ptr++;
    i++;
  }
  dest[i] = '\0';

  return dest;
}


/*-------------------------------------------------------------------------*/
/* Converts an US date string ("M/D/Y" or "M-D-Y") into a date structure.  */
/* A two digit year may be specified.                                      */
/*                                                                         */
/* return value:                                                           */
/*  0   date was converted successfully                                    */
/* -1   date is invalid and was not converted                              */
/*-------------------------------------------------------------------------*/
int strtodate(const char *string,
              THEDATE *dt) {
  char *ptr;

  ptr = (char *)string;
  if(isdigit(*ptr)) {
    dt->da_mon = atoi(ptr);
    while (isdigit(*ptr)) ++ptr;
    if ((*ptr != '/') && (*ptr != '-')) {
      return -1;
    }
    ptr++;
    dt->da_day = atoi(ptr);
    while (isdigit(*ptr)) ++ptr;
    if((*ptr != '/') && (*ptr != '-')) {
      return -1;
    }
    ptr++;
    dt->da_year = atoi(ptr);
  }

  if (dt->da_year < 80) {
    dt->da_year = dt->da_year + 2000;
  }
  else if (dt->da_year < 100) {
    dt->da_year = dt->da_year + 1900;
  }

  return 0;
}


/*-------------------------------------------------------------------------*/
/* Checks, if the specified date is valid.                                 */
/*                                                                         */
/* return value:                                                           */
/*  0   date is invalid                                                    */
/* -1   date is valid                                                      */
/*-------------------------------------------------------------------------*/
int datevalid(THEDATE * dt) {
  if ((dt->da_mon < 1) || (dt->da_mon > 12) ||
      (dt->da_day < 1) || (dt->da_day > 31)) {
    return 0;
  }

  switch (dt->da_mon) {
    case 2:
      if ((dt->da_year % 4 == 0) &&
          ((dt->da_year % 100 != 0) || (dt->da_year % 400 == 0))) {
        if (dt->da_day > 29) {
          return 0;
        }
      } else {
        if (dt->da_day > 28) {
          return 0;
        }
      }
      break;
    case 4:
    case 6:
    case 9:
    case 11:
      if (dt->da_day > 30) {
        return 0;
      }
      break;
  }

  return -1;
}


/*-------------------------------------------------------------------------*/
/* Splits the program arguments into file and switch arguments.            */
/*-------------------------------------------------------------------------*/
void classify_args(const char argc,
                   const char **argv,
                   char *fileargc,
                   char **fileargv,
                   char *switchargc,
                   char **switchargv) {
  char i,
       *argptr;


  *fileargc = 0;
  *switchargc = 0;
  for (i = 1; i < argc; i++) {
    argptr = (char *)argv[i];
    if (argptr[0] == '/' || argptr[0] == '-') {
      /* first character of parameter is '/' or '-' -> switch argument */
      switchargv[*switchargc] = argptr + 1;
      *switchargc = *switchargc + 1;
    }
    else {
      /* file argument */
      fileargv[*fileargc] = argptr;
      *fileargc = *fileargc + 1;
    }
  }
}


/*-------------------------------------------------------------------------*/
/* Appends a trailing directory separator to the path, but only if it is   */
/* missing.                                                                */
/*-------------------------------------------------------------------------*/
char *cat_separator(char *path) {
  int length;


  length = strlen(path);
  if (path[length - 1] != *DIR_SEPARATOR) {
    path[length] = *DIR_SEPARATOR;
    path[length + 1] = '\0';
  }

  return path;
}


/*-------------------------------------------------------------------------*/
/* Checks if the specified path is valid. The pathname may contain a       */
/* trailing directory separator.                                           */
/*-------------------------------------------------------------------------*/
int dir_exists(const char *path) {
  char tmp_path[MAXPATH],
       i;
  int attrib;


  strmcpy(tmp_path, path, sizeof(tmp_path));
  i = strlen(tmp_path);
  if (i < 3) {
    /* root directory needs trailing backspace -> add it */
    strmcat(tmp_path, DIR_SEPARATOR, sizeof(tmp_path));
  }
  else if (i > 3) {
    /* remove trailing backspace if necessary */
    i = i - 1;
    if (tmp_path[i] == *DIR_SEPARATOR) {
      tmp_path[i] = '\0';
    }
  }

  attrib = _chmod(tmp_path, 0);
  if (attrib == -1 ||
      (attrib & FA_DIREC) == 0) {
    return 0;
  }

  return -1;
}


/*-------------------------------------------------------------------------*/
/* Writes the specified message on the screen and waits for a key input    */
/* only accepting the specified ones. The first character of a key message */
/* is used as input key (eg: "yes" -> 'Y', "no" -> 'N').                   */
/* MODIFIED 9/2005: returns 1/2/3/4 if 1st/2nd/3rd/4th option chosen       */
/*                                                                         */
/* parameters:                                                             */
/*   msg             message text                                          */
/*   yes             text for answer "yes"                                 */
/*   no              text for answer "no"                                  */
/*   overwrite_all   optional; text for answer "overwrite_all" or NULL     */
/*   skip_all        optional; text for answer "skip_all" or NULL          */
/*                                                                         */
/* examples:                                                               */
/*   char ch;                                                              */
/*   ch = confirm("Overwrite file", "Yes", "No", NULL, NULL);              */
/*   ch = confirm("Overwrite file", "Yes", "No", "All", NULL);             */
/*   ch = confirm("Overwrite file", "Yes", "No",                           */
/*                "Overwrite all", "Skip all");                            */
/*-------------------------------------------------------------------------*/
char confirm(const char *msg,
             const char *yes,
             const char *no,
             const char *overwrite_all,
             const char *skip_all) {
  char msg_tmp[128],
       ch;


  strmcpy(msg_tmp, msg, sizeof(msg_tmp));
  strmcat(msg_tmp, " (", sizeof(msg_tmp));
  strmcat(msg_tmp, yes, sizeof(msg_tmp));
  strmcat(msg_tmp, "/", sizeof(msg_tmp));
  strmcat(msg_tmp, no, sizeof(msg_tmp));
  if (overwrite_all != NULL &&
      overwrite_all[0] != '\0') {
    strmcat(msg_tmp, "/", sizeof(msg_tmp));
    strmcat(msg_tmp, overwrite_all, sizeof(msg_tmp));
  }
  if (skip_all != NULL &&
      skip_all[0] != '\0') {
    strmcat(msg_tmp, "/", sizeof(msg_tmp));
    strmcat(msg_tmp, skip_all, sizeof(msg_tmp));
  }
  strmcat(msg_tmp, ")? ", sizeof(msg_tmp));

  do {
    printf(msg_tmp);
    scanf("%c", &ch);
    fflush(stdin);
    ch = toupper(ch);
    if (ch == yes[0]) { return 1; }
    if (ch == no[0]) { return 2; }
    if ((overwrite_all != NULL) && (ch == overwrite_all[0])) { return 3; }
    if ((skip_all != NULL) && (ch == skip_all[0])) { return 4; }
  } while (1);

  /* never reached: return 0; */
}


/*-------------------------------------------------------------------------*/
/* Copies the source into the destination file including file attributes   */
/* and timestamp.                                                          */
/*-------------------------------------------------------------------------*/
void copy_file(const char *src_filename,
               const char *dest_filename,
               const char return_on_error) {
  FILE *src_file,
       *dest_file;
  char buffer[16384];
  unsigned int buffersize;
  int readsize,
      fileattrib;
  struct ftime filetime;


  /* open source file */
  src_file = fopen(src_filename, "rb");
  if (src_file == NULL) {
    printf("%s - %s\n", catgets(cat, 25, 0, "Cannot open source file"), src_filename);
    if (return_on_error) {
      return;
    }
    else {
      exit(30);
    }
  }

  /* open destination file */
  dest_file = fopen(dest_filename, "wb");
  if (dest_file == NULL) {
    printf("%s - %s\n", catgets(cat, 25, 1, "Cannot create destination file"), dest_filename);
    fclose(src_file);
    if (return_on_error) {
      return;
    }
    else {
      exit(29);
    }
  }

  /* copy file data */
  buffersize = sizeof(buffer);
  readsize = fread(buffer, sizeof(char), buffersize, src_file);
  while (readsize > 0) {
    if (fwrite(buffer, sizeof(char), readsize, dest_file) != readsize) {
      printf("%s - %s\n", catgets(cat, 25, 2, "Write error on destination file"), dest_filename);
      fclose(src_file);
      fclose(dest_file);
      if (return_on_error) {
        return;
      }
      else {
        exit(29);
      }
    }
    readsize = fread(buffer, sizeof(char), buffersize, src_file);
  }

  /* copy file timestamp */
  getftime(fileno(src_file), &filetime);
  setftime(fileno(dest_file), &filetime);

  /* close files */
  fclose(src_file);
  fclose(dest_file);

  /* copy file attributes */
  fileattrib = _chmod(src_filename, 0);
  _chmod(dest_filename,  fileattrib);
}

int getftime (int handle, struct ftime *ftimep) /* Get a file's time */
{
      int retval = 0;
      union
      {
            struct
            {
                  unsigned time;
                  unsigned date;
            } msc_time;
            struct ftime bc_time;
      } FTIME;

      if (0 == (retval = DOS_GETFTIME(handle, (unsigned int *)&FTIME.msc_time.date,
            (unsigned int *)&FTIME.msc_time.time)))
      {
            *ftimep = FTIME.bc_time;
      }
      return retval;
}

int setftime (int handle, struct ftime *ftimep) /* Set a file's time */
{
      union
      {
            struct
            {
                  unsigned time;
                  unsigned date;
            } msc_time;
            struct ftime bc_time;
      } FTIME;

      FTIME.bc_time = *ftimep;

      return DOS_SETFTIME(handle, FTIME.msc_time.date, FTIME.msc_time.time);
}

#if __WATCOMC__ < 1280
int _chmod(const char *filename, int func, ...) /* Get/set file attributes */
{
        va_list l;
        int attributes;
        unsigned attr;
        va_start(l, func);
        attributes = va_arg(l, int);
        va_end(l);
        if (func == 0)
        {
                if (_dos_getfileattr(filename, &attr) != 0) return -1;
                return attr;
        }
        if (_dos_setfileattr(filename, attributes) != 0) return -1;
        return 0;
}
#endif
unsigned long dostounix (struct date *d, struct time *t)
{
  struct tm tm;

  if (!d || !t)
     return (0);

  tm.tm_year  = d->da_year - 1900;
  tm.tm_mon   = d->da_mon - 1;
  tm.tm_mday  = d->da_day;
  tm.tm_isdst = 0;
  tm.tm_hour  = t->ti_hour;
  tm.tm_min   = t->ti_min;
  tm.tm_sec   = t->ti_sec;
  return mktime (&tm);
}
