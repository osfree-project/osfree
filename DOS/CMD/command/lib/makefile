#
# A Makefile for ATTRIB
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

#32_BITS  = 1

#defines object file names in format objname.$(O)
OBJS = tcc2wat.$(O) absfile.$(O) almemblk.$(O) alprmblk.$(O) alsysblk.$(O) app_get.$(O) &
        app_set.$(O) beep_l.$(O) beep_n.$(O) brk_get.$(O) brk_set.$(O) cbreak.$(O) &
        cbs.$(O) cd_dir.$(O) cgetch.$(O) cgettime.$(O) chgctxt.$(O) chgdrv.$(O) &
        chgenv.$(O) chgenvc.$(O) chgenvr.$(O) clrline.$(O) cmdinput.$(O) comfile.$(O) &
        compfile.$(O) critend.$(O) critrchk.$(O) ctxt.$(O) ctxt_adr.$(O) ctxt_as.$(O) &
        ctxt_chg.$(O) ctxt_clr.$(O) ctxt_get.$(O) ctxt_inf.$(O) ctxt_mk.$(O) &
        ctxt_mkb.$(O) ctxt_mkn.$(O) ctxt_pop.$(O) ctxt_psh.$(O) ctxt_rnu.$(O) &
        ctxt_set.$(O) ctxt_ss.$(O) ctxt_vw.$(O) curdatel.$(O) curtime.$(O) cwd.$(O) &
        dateget.$(O) dateset.$(O) dbg_c.$(O) dbg_mem.$(O) dbg_prnt.$(O) dbg_s.$(O) &
        dbg_sn.$(O) devopen.$(O) dfn_err.$(O) dispcnt.$(O) dispexit.$(O) drvnum.$(O) &
        efct_001.$(O) exec.$(O) exec1.$(O) farread.$(O) fcompl1.$(O) fcompl2.$(O) &
        fdattr.$(O) fdevopen.$(O) fdsattr.$(O) fillcomp.$(O) find.$(O) freep.$(O) &
        frsysblk.$(O) fstpcpy.$(O) gallstr.$(O) get1mcb.$(O) getenv.$(O) goxy.$(O) &
        grabfcom.$(O) gumblink.$(O) hdlrctxt.$(O) hist_get.$(O) hist_set.$(O) &
        inputdos.$(O) is_empty.$(O) is_fnamc.$(O) is_fnstr.$(O) is_pchr.$(O) &
        isadev.$(O) keyprsd.$(O) kswap_c.$(O) lastdget.$(O) lastdset.$(O) leadopt.$(O) &
        lowexec.$(O) ltrimcl.$(O) ltrimsp.$(O) lwr1wd.$(O) match.$(O) messages.$(O) &
        mk_rddir.$(O) mktmpfil.$(O) msg_dflt.$(O) msg_dps.$(O) msg_fstr.$(O) &
        msg_get.$(O) msg_gpt.$(O) msg_mkey.$(O) msg_prmp.$(O) mux_ae.$(O) nls.$(O) &
        nls_date.$(O) nls_time.$(O) num_fmt.$(O) onoff.$(O) openf.$(O) optsb.$(O) &
        optsi.$(O) optss.$(O) parsenum.$(O) pr_date.$(O) pr_prmpt.$(O) pr_time.$(O) &
        prprompt.$(O) readcmd.$(O) realnum.$(O) res.$(O) res_r.$(O) res_vald.$(O) &
        res_w.$(O) resfile.$(O) rmtmpfil.$(O) rtrimcl.$(O) rtrimsp.$(O) salloc.$(O) &
        samefile.$(O) scancmd.$(O) scanopt.$(O) session.$(O) showcmds.$(O) skqwd.$(O) &
        spfnam.$(O) split.$(O) sumblink.$(O) tempfile.$(O) timeget.$(O) timeset.$(O) &
        tmpnam.$(O) trimcl.$(O) trimsp.$(O) truepath.$(O) truncate.$(O) txtlend.$(O) &
        unquote.$(O) vcgetch.$(O) vcgetstr.$(O) where.$(O) &
err1.$(O) &
err10.$(O) &
err11.$(O) &
err12.$(O) &
err13.$(O) &
err14.$(O) &
err15.$(O) &
err16.$(O) &
err17.$(O) &
err18.$(O) &
err19.$(O) &
err2.$(O) &
err20.$(O) &
err21.$(O) &
err22.$(O) &
err23.$(O) &
err24.$(O) &
err25.$(O) &
err26.$(O) &
err27.$(O) &
err28.$(O) &
err29.$(O) &
err3.$(O) &
err30.$(O) &
err31.$(O) &
err32.$(O) &
err33.$(O) &
err34.$(O) &
err35.$(O) &
err36.$(O) &
err37.$(O) &
err38.$(O) &
err39.$(O) &
err4.$(O) &
err40.$(O) &
err41.$(O) &
err42.$(O) &
err43.$(O) &
err44.$(O) &
err45.$(O) &
err46.$(O) &
err47.$(O) &
err48.$(O) &
err49.$(O) &
err5.$(O) &
err50.$(O) &
err51.$(O) &
err52.$(O) &
err53.$(O) &
err54.$(O) &
err55.$(O) &
err56.$(O) &
err57.$(O) &
err58.$(O) &
err59.$(O) &
err6.$(O) &
err60.$(O) &
err61.$(O) &
err62.$(O) &
err63.$(O) &
err64.$(O) &
err65.$(O) &
err66.$(O) &
err67.$(O) &
err68.$(O) &
err69.$(O) &
err7.$(O) &
err70.$(O) &
err71.$(O) &
err72.$(O) &
err73.$(O) &
err74.$(O) &
err75.$(O) &
err76.$(O) &
err77.$(O) &
err78.$(O) &
err79.$(O) &
err8.$(O) &
err80.$(O) &
err81.$(O) &
err82.$(O) &
err83.$(O) &
err84.$(O) &
err85.$(O) &
err86.$(O) &
err9.$(O)

# defines additional options for C compiler

ADD_COPT = -i=..$(SEP)suppl

!include $(%ROOT)/mk/all.mk


all: lib.lib

lib.lib: $(OBJS)
 $(SAY) Creating library $^@
 -@if exist $@ @del $@
 @$(LIB) $(LIBOPT) $@ +$< $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).exe $(FILESDIR)$(SEP)dos
