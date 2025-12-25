
PROJ = somwm35i
TRGT = $(PROJ).dll
srcfiles = $(p)somwm35i$(e)
DEST = os2tk45$(SEP)som$(SEP)common$(SEP)dll

DLL=1

#	TCCalloc.1, &
#     TCFree.                                       2, &
#     TCMalloc.                                     3, &
#     TCRealloc.                                    4, &
#     _DLL_InitTerm.                                5, &
#     get_double_alignment.                         6, &
#     somVaBuf_add.                                 7, &
#     somVaBuf_create.                              8, &
#     somVaBuf_destroy.                             9, &
#     somVaBuf_get_valist.                         10, &

EXPORTS = _ctype=sr_ctype, &
  stderr=srstderr, &
  stdin=srstdin, &
  stdout=srstdout, &
  _environ=sr_environ, &
  _CRT_init=_sr_CRT_init, &
  _CRT_term=_sr_CRT_term, &
  _abort_in_progress=_sr_abort_in_progress, &
  _exception_dllinit=_sr_exception_dllinit, &
  _exception_procinit=_sr_exception_procinit, &
  _fpninit=_sr_fpninit, &
  _fpreset=_sr_fpreset, &
  _fscanfieee=_sr_fscanfieee, &
  atexit=_sratexit, &
  atoi=_sratoi, &
  fflush=_srfflush, &
  fopen=_srfopen, &
  _matherr=_sr_matherr, &
  _fprintfieee=_sr_fprintfieee, &
  _sprintfieee=_sr_sprintfieee, &
  _printfieee=_sr_printfieee, &
  _sscanfieee=_sr_sscanfieee, &
  _putenv=_sr_putenv, &
  getenv=_srgetenv, &
  _terminate=_sr_terminate, &
  _PrintErrMsg=_sr_PrintErrMsg, &
  _SysFindFirst=_sr_SysFindFirst, &
  _SysFindNext=_sr_SysFindNext, &
  _SysFindClose=_sr_SysFindClose, &
  fclose=_srfclose, &
  feof=_srfeof, &
  fgetc=_srfgetc, &
  fgets=_srfgets, &
  fputs=_srfputs, &
  fputc=_srfputc, &
  fread=_srfread, &
  fseek=_srfseek, &
  fwrite=_srfwrite, &
  remove=_srremove, &
  rename=_srrename, &
  rewind=_srrewind, &
  strcmp=_srstrcmp, &
  strstr=_srstrstr, &
  strlen=_srstrlen, &
  memmove=_srmemmove, &
  memset=_srmemset, &
  strcpy=_srstrcpy, &
  strncmp=_srstrncmp, &
  strncpy=_srstrncpy, &
  strrchr=_srstrrchr, &
  strtok=_srstrtok, &
  tolower=_srtolower, &
  memcpy=_srmemcpy, &
  strcat=_srstrcat, &
  strdup=_srstrdup, &
  strpbrk=_srstrpbrk, &
  malloc=_srmalloc, &
  free=_srfree, &
  exit=_srexit, &
  isalnum=_srisalnum, &
  isalpha=_srisalpha, &
  perror=_srperror, &
  setlocale=_srsetlocale, &
  signal=_srsignal, &
  strchr=_srstrchr, &
  strtoul=_srstrtoul, &
  system=_srsystem, &
  tmpnam=_srtmpnam, &
  _EXE_Exception=_sr_EXE_Exception, &
  _fltused=_sr_fltused, &
  _atofieee = _sratofieee


!include $(%ROOT)SOM/common.mk

!ifdef HOSTBLD
OS2TK = $(%OS2TK)$(SEP)
!else
OS2TK = $(%ROOT)build$(SEP)bin$(SEP)os2tk45$(SEP)
!endif

ADD_COPT = -s -6s $(ADD_COPT) -i=$(OS2TK)$(SEP)som$(SEP)include -i=$(PORT_BASE)somwm35i$(SEP)include

.c: $(PORT_BASE)somwm35i$(SEP)src

.h: $(PORT_BASE)somwm35i$(SEP)include

