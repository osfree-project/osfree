
src\xwpsec_ring0 has the code for the ring-0 part of the
XWorkplace Installable Security Subsystem (ISS).

This is an SES base device driver. Aside from some 16-bit
thunking code written in assembler, it is a 32-bit device
driver, and can be compiled using VAC 3.08.

IBM ALP is needed for the assembler files. I now force
ALP into MASM 5.10 compatibility mode, so this should work
with newer ALPs as well (V1.0.1 (2003-01-07) [umoeller]).

All this code compiles into the XWPSEC32.SYS driver file,
which forms the ring-0 part of XWorkplace Security.
See strat_init_base.c for an introduction.

Ulrich M”ller (2000-11-28)

