# $Id: makefile,v 1.3 2004/08/21 14:49:22 prokushev Exp $
#
# Makefile for common shared libs

PROJ = libflac
!define DEB

!ifdef DEB
defines= -dVERSION="1.0.4" -dFLAC__CPU_IA32 -dFLAC__NO_NASM -dFLAC__OVERFLOW_DETECT -dSTRICT -d_DEBUG
!else
defines= -dVERSION="1.0.4" -dFLAC__CPU_IA32 -dFLAC__NO_NASM -dSTRICT -dNODEBUG -dFLAC__ALIGN_MALLOC_DATA
!endif

ADD_COPT = $(defines) -i=$(MYDIR)include

srcfiles = &
 $(p)bitbuffer$(e) &
 $(p)bitmath$(e) &
 $(p)cpu$(e) &
 $(p)crc$(e) &
 $(p)file_decoder$(e) &
 $(p)file_encoder$(e) &
 $(p)fixed$(e) &
 $(p)format$(e) &
 $(p)lpc$(e) &
 $(p)md5$(e) &
 $(p)memory$(e) &
 $(p)metadata_iterators$(e) &
 $(p)metadata_object$(e) &
 $(p)seekable_stream_decoder$(e) &
 $(p)seekable_stream_encoder$(e) &
 $(p)stream_decoder$(e) &
 $(p)stream_encoder$(e) &
 $(p)stream_encoder_framing$(e)

!include $(%ROOT)/mk/libsos2.mk

TARGETS  = $(PATH)$(PROJ).lib

$(PATH)$(PROJ).lib: $(OBJS)
 @$(MAKE) $(MAKEOPT) library=$(PATH)$(PROJ).lib library

qwe: .symbolic
 @echo $(defines)
