@echo off
set model=s
set lng=english
set compiler=mc
set freedos=d:\freedos
set mcdir=c:\mc

set fdsrc=%freedos%\src
set fdbin=%fdsrc%\bin
set fdlib=%fdsrc%\lib\%compiler%
set fdinc=%fdsrc%\include

cc foreach.c -fop _MICROC_=1
%fdbin%\msgcomp %fdinc%\%lng% %lng% comp.rsp lib.rsp yerror.h
.\foreach.com comp.rsp cc %%name: -flop _MICROC_=1
lib.exe /SCW msg.lib @lib.rsp, msg.lst
ASM.exe /ml /DNDEBUG asgn_tsr.asm;
VAL.exe /COM asgn_tsr.obj , asgn_tsr.com;
%fdbin%\Bin2c module asgn_tsr.com >asgn_asc.c
CC.com assign.c -flpo NDEBUG=1 _MICROC_=1
echo msg %fdlib%\s_%lng% %fdlib%\Suppl_s %mcdir%\Mclib >link.rsp
VAL.exe /COM /NCI %MCDIR%\Pc86rl_t assign, assign.com, NUL, @link.rsp
