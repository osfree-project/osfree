/* testing f$directory */

trace off 
/* signal here */
/* ===== f$directory() =============================================== */
call notify 'F$DIRECTORY'
'show default >LIFO'
parse pull . dev':'dir .
if (dir \== f$directory()) then
   call complain "f$directory return incorrect answer"


/* ===== f$identifier() ============================================== */
call notify 'F$IDENTIFIER'

users = 'system operator field'
do while users \= ''
   parse var users user users
   call tell user

   'write sys$output f$identifier( "'user'", "NAME_TO_NUMBER" ) >LIFO'
   parse pull number  .
   if number \== f$identifier( user, "NAME_TO_NUMBER") then 
      call complain 'built-in f$identifier dont convert to number correctly'

   'write sys$output f$identifier( 'number', "NUMBER_TO_NAME" ) >LIFO'
   parse pull name .
   if name \= f$identifier( number, "NUMBER_TO_NAME" ) then 
      call complain 'built-in f$identifier dont convert to name correctly'
   end


/* ===== f$getdvi ==================================================== */
call notify 'F$GETDVI'
/* 
 * Notice that you can never expect all of these to *always* be the 
 * same. After all, we are fetching information about devices in use, 
 * so of course the values will change. Consequently, use some 
 * intelligence when interpreting the results from this test.
 */

items = 'acppid acptype all alldevnam alloclass alt_host_avail',
   'alt_host_name alt_host_type avl ccl cluster concealed cylinders',
   'devbufsiz devchar devchar2 devclass devdepend devdepend2 devlocknam',
   'devnam devsts devtype dir dmt dua elg errcnt exists fod for freeblocks',
   'fulldevnam gen host_avail host_count host_name host_type idv lockid',
   'logvolnam maxblock maxfiles mbx media_id media_name media_type mnt',
   'mountcnt net nextdevnam odv opcnt opr ownuic pid rck rct rec recsiz',
   'refcnt remote_device rnd rootdevnam rtm sdi sectors serialnum',
   'served_device shr spl spldevnam sqd sts swl tracks transcnt trm',
   'tt_accpornam tt_altypeahd tt_ansicrt tt_app_keypad tt_autobaud tt_avo',
   'tt_block tt_brdcstmbx tt_crfill tt_deccrt tt_deccrt2 tt_dialup',
   'tt_disconnect tt_dma tt_drcs tt_edit tt_editing tt_eightbit tt_escape',
   'tt_fallback tt_halfdup tt_hangup tt_hostsync tt_insert tt_lffill',
   'tt_localecho tt_lower tt_mbxdsabl tt_mechform tt_mechtab tt_modem',
   'tt_modhangup tt_nobrdcst tt_noecho tt_notypeahd tt_oper tt_page',
   'tt_pasthru tt_phydevnam tt_printer tt_readsync tt_regis tt_remote',
   'tt_scope tt_secure tt_setspeed tt_sixel tt_ttsync tt_syspwd tt_wrap',
   'unit volcount volnam volnumber volsetmem vprot wck'

/* add your own devices to run the test on, as it suits you */
/*
 *  'sh log sys$command >LIFO'
 *  parse pull line '=' '"' sys_command '"' .
 *  devs = 'sys$login' f$getjpi("",'TERMINAL') 'tape: _RUNIT$TXC1:'
 */

devs = ''
'show device >LIFO'
do queued()
   parse pull dev ':' rest
   if rest \= '' then
      devs = devs dev
   end
  
do xx=1 for 3 while devs \= '' 
   parse var devs dev devs 
   call tell dev

   titems = items 
   do while titems \= ''
      parse var titems item titems
      queue 'write sys$output f$getdvi( "'dev'", "'item'")'
      end

   'LIFO> set def nl: >FIFO'

   titems = items
   do queued()
      parse var titems item titems   
      call blink item
      parse pull vms_result 
      rexx_result = f$getdvi( dev, translate(item) ) 

      if (item == 'opcnt') | (item=='transcnt') | (item=='refcnt') then do
         if abs(rexx_result-vms_result)*15 < rexx_result then
            iterate  /* or something like that .... */
         end

      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says <'vms_result'>' ||,
                                              ' and rexx says <'rexx_result'>.'
      end
   end


/* ===== f$getjpi ================================================ */
call notify 'F$GETJPI'
items = 'account aptcnt astact astcnt asten astlm authpri authpriv biocnt',
   'biolm bufio bytcnt bytlm cliname cpulim cputim curpriv dfpfc dfwscnt',
   'diocnt diolm dirio efcs efcu efwm enqcnt enqlm excvec filcnt fillm',
   'finalexc frep0va frep1va freptecnt gpgcnt grp imagecount imagname',
   'imagpriv jobprccnt logintim master_pid mem mode msgmask owner',
   'pageflts pagfilcnt pagfilloc pgflquota phdflags pid ppgcnt prccnt',
   'prclm prcnam prib procpriv sitespec state sts swpfilloc terminal',
   'tmbu tqcnt tqlm uic username virtpeak volumes wsauth wsauthext',
   'wsextent wspeak wsquota wssize'

numeric digits 12 

pid = ''
procs = ''
do until proc=='' 
   proc = f$pid('PID')
   if f$getjpi(proc,"PRCNAM") == f$process() then
      iterate 

   procs = procs proc 
   end

do while procs\='' 
   parse var procs proc procs 
   proc = translate(proc)
   call tell proc

   titems = items 
   do while titems \= ''
      parse var titems item titems
      queue 'write sys$output f$getjpi( "'proc'", "'item'")'
      end

   'LIFO> set def nl: >FIFO'

   titems = items
   myproc = translate(f$getjpi('','PID'))
   do queued()
      parse var titems item titems   
      call blink item 
      parse pull vms_result 
      rexx_result = f$getjpi( proc, translate(item) ) 

      if (datatype(rexx_result,'num') & datatype(vms_result,'num')) then do
         if abs(rexx_result-vms_result) < rexx_result*0.15 then
            iterate
    
         if (translate(right(item,3))=='CNT') then
            if abs(rexx_result - vms_result)<4 then 
               iterate
         end

      if (proc = myproc) & (item='finalexc' | item='excvec' | item='state' |,
          item='efcu') then
         iterate

      if (vms_result\==rexx_result) & (item=='state') &,
           (vms_result=='LEF' | rexx_result=='LEF') then iterate

      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'

      end
   end



/* ===== f$getsyi ================================================ */
call notify 'F$GETSYI'
items = 'archflag boottime character_emulated cpu decimal_emulated',
    'd_float_emulated errorlogbuffers f_float_emulated g_float_emulated',
    'pagefile_free pagefile_page sid swapfile_free swapfile_page',
    'version activecpu_cnt availcpu_cnt cluster_fsysid cluster_ftime ',
    'cluster_member cluster_nodes cluster_quorum cluster_votes',
    'contig_gblpages free_gblpages free_gblsects hw_model hw_name',
    'nodename node_area node_csid node_hwtype node_hwvers node_number',
    'node_quorum node_swincarn node_swtype node_swvers node_systemid',
    'node_votes scs_exists'

devs = ''
do until devs = '' 
   parse var devs dev devs 

   titems = items 
   do while titems \= ''
      parse var titems item titems
      if dev \= '' then
         queue 'write sys$output f$getsyi( "'item'", "'dev'")'
      else
         queue 'write sys$output f$getsyi( "'item'" )'
      end

   'LIFO> set def nl: >FIFO'

   titems = items
   do queued()
      parse var titems item titems   
      call blink item
      parse pull vms_result 
      if dev = '' then 
         rexx_result = f$getsyi( translate(item) ) 
      else
         rexx_result = f$getsyi( translate(item), dev ) 

      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'

      end
   end


/* ===== f$getqui ================================================ */
call notify 'F$GETQUI'
items = 'account_name after_time assigned_queue_name base_priority',
   'characteristic_name characterstics charactersistic_number',
   'checkpoint_data cli completed_blocks condiiton_vector cpu_default',
   'cpu_limit default_form_name default_form_stock device_name',
   'entry_number execution_job_count file_burst file_checkpointed',
   'file_copies file_copies_done file_delete file_double_space',
   'file_executing file_flag file_flags file_identification',
   'file_page_header file_paginate file_passall file_setup_modules',
   'file_spacification file_status file_trailer first_page',
   'form_description form_flags form_length form_margin_bottom',
   'form_margin_left form_margin_right form_margin_top form_name',
   'forma_number form_setup_modules form_sheet_feed form_stock',
   'form_truncate worm_width form_wrap generic_target holding_job_count',
   'intervening_blocks intervening_jobs job_aborting job_copies',
   'job_copies_done job_cpu_limit job_executing job_file_burst',
   'job_file_burst_one job_file_flag job_file_flag_one',
   'job_file_paginate job_file_trailer job_file_trailer_one job_flags',
   'job_holding job_inaccessible job_limit job_log_delete job_log_null',
   'jb_log_spool job_lowercase job_name job_notify job_pending job_pid',
   'job_refused job_reset_modules job_restart job_retained job_size',
   'job_size_maximum job_size_minimum job_starting job_status',
   'job_suspended job_timed_release job_wsdefault job_wsextent',
   'job_wsquota last_page library_specification log_queue note',
   'operator_request owner_uic page_Setup_modules parameter_1',
   'parameter_2 parameter_3 parameter_4 parameter_5 parameter_6',
   'parameter_7 parameter_8 pending_job_block_count pending_job_count',
   'pending_job_reason pedn_char_mismatch pend_hob_size_max',
   'pend_job_size_min pend_lowercase_mismatch pend_no_access',
   'pend_queue_busy pend_queue_state pend_stock_mismatch priority',
   'processor protection queue_acl_specified queue_aligning',
   'queue_closed queue_cpu_default queue_cpu_limit queue_file_burst',
   'queue_file_bust_one queue_file_flag queue_file_flag_one',
   'queue_file_paginate queue_file_trailer queue_file_trailer_one',
   'queue_flags queue_generic queue_generic_selection queue_idle',
   'queue_job_burst queue_job_flag queue_job_size_sched',
   'queue_job_trailer queue_lowercase queue_name queue_paused',
   'queue_printer queue_record_blocking queue_remote queue_resetting',
   'queue_resuming queue_retain_All queue_retain_error queue_server',
   'queue_stalled queue_starting queue_status queue_stopped queue_swap',
   'queue_terminal queue_unavailable queue_wsdefault queue_wsextent',
   'queue_wsquota requeue_queue_name restart_queue_name',
   'retained_job_count scsnode_name submission_time',
   'timed_release_job_count uic username wsdefault wsextent wsquota'

char_items = 'characteristic_name characteristic_number'

trans_items = 'queue_name'

entry_items = 'account_name after_time characteristics checkpoint_data',
   'cli completed_blocks condition_vector cpu_limit entry_number',
   'form_name form_stock', /* intervening_blocks intervening_jobs*/ 
   'job_aborting job_copies job_cpu_limit job_copies_done job_executing',
   'job_file_burst job_file_burst_one job_file_flag job_file_flag_one',
   'job_file_paginate job_file_trailer job_file_trailer_one job_flags',
   'job_holding job_inaccessible job_log_delete job_log_null',
   'job_log_spool job_lowercase job_name job_notify job_pending',
   'job_pid job_refused job_restart job_retained job_size job_starting',
   'job_suspended job_timed_release job_wsdefault job_wsextent',
   'job_wsquota log_queue log_specification note operator_request',
   'parameter_1 parameter_2 parameter_3 parameter_4 parameter_5',
   'parameter_6 parameter_7 parameter_8 pend_char_mismatch',
   'pend_job_size_max pend_job_size_min pend_lowercase_mismatch',
   'pend_no_access pend_queue_busy pend_queue_state pend_stock_mismatch',
   'priority queue_name requeue_queue_name'

file_items = 'file_copies file_checkpointed file_copies_done file_burst',
   'file_delete file_double_space file_executing file_flag file_flags',
   'file_identification file_page_header file_paginate file_passall',
   'file_setup_modules file_specification file_trailer first_page',
   'last_page'

form_items = 'form_description form_length form_margin_bottom',
   'form_margin_left form_margin_right form_margin_top form_name',
   'form_number form_setup_modules form_sheet_feed form_stock',
   'form_truncate form_width form_wrap page_setup_modules'

job_items = 'account_name after_time characteristics checkpoint_Data',
   'cli completed_blocks condition_vector cpu_limit entry_number',
   'form_name form_stock intervening_blocks' /*intervening_jobs*/ 'job_copies',
   'job_copies_done job_flags job_cpu_limit job_file_burst',
   'job_File_burst_one job_file_flag job_File_flag_one',
   'job_file_paginate job_File_trailer job_file_Trailer_one',
   'job_log_delte job_log_null job_log_spool job_lowercase job_notify',
   'job_restart job_wsdefault job_wsextent job_wsquota job_name job_pid',
   'job_sizejob_status job_aborting job_executing job_holdeing',
   'job_inaccessible job_pending job_refused job_retained job_starting',
   'job_suspended job_Timed_release log_queue log_specification note',
   'operator_request parameter_1 parameter_2 parameter_3 parameter_4',
   'parameter_5 parameter_6 parameter_7 parameter_8 pending job_reason',
   'pend_char_mismatch pend_job_size_max pend_job_size_min',
   'pend_lowercase_mismatch pend_no_access pend_queue_busy',
   'pend_queue_state pend_stock_mismatch priority queue_name',
   'requeue_queue_name restart_queue_name'

/*
 * The item 'intervening_job' is meaningful only for output execution
 * queues, according to the documentation. It seems to be sensitive to
 * what sort of object it is called for and triggers an error if it 
 * don't like what is sees. I'll comment it in again if someone can
 * tell me what a 'output execution queue' is, and how I determine 
 * whether a given queue is such.
 */


queue_items = 'assigned_queue_name base_priority characteristics',
   'cpu_default cpu_limit default_form_name default_form_stock',
   'device_name executing_job_count form_name form_stock generic_target',
   'holding_job_count job_limit job_reset_modules job_size_maximum',
   'job_size_minimum library_specification owner_uic',
   'pending_job_block_count pending_job_count processor protection',
   'queue_description queue_flags queue_acl_specified queue_batch',
   'queue_cpu_default queue_cpu_limit queue_file_burst_one',
   'queue_file_flag queue_file_flag_one queue_file_paginate',
   'queue_file_trailer queue_file_trailer_one queue_generic',
   'queue_generic_selection queue_job_flag queue_job_size_sched',
   'queue_job_trailer queue_printer queue_record_blocking',
   'queue_retain_all queue_retain_error queue_swap queue_terminal',
   'queue_wsdefault queue_wsextent queue_wsquota retained_job_count',
   'scsnode_name'

funcs = 'job entry file form job queue'
names = ''

call notify '__display_queue'
'show queue >FIFO'
do queued() 
   parse pull . keyw name',' . 
   if keyw \== 'queue' then
      iterate
   
   names = names name 
   end

do while names \= ''
   parse var names name names 

   call tell name
   titems = queue_items 
   do while titems \= ''
      parse var titems item titems
      queue 'write sys$output f$getqui( "DISPLAY_QUEUE", "'item'", "'name'")'
      end

   'LIFO> set def nl: >FIFO'

   titems = queue_items
   do queued()
      parse var titems item titems   
      call blink item 
      parse pull vms_result 
      rexx_result = f$getqui( 'DISPLAY_QUEUE', translate(item), name )
      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'
      end
   end


/*====  form ===== */
call notify '__display_form'
'show queue/form >FIFO'

do queued() 
   parse pull name . '('keyw'=' . ')' number . 
   if keyw \== 'stock' then
      iterate
   
   names = names name number
   end

do while names \= ''
   parse var names name number names 

   call tell name
   titems = form_items 
   do while titems \= ''
      parse var titems item titems
      queue 'write sys$output f$getqui( "DISPLAY_FORM", "'item'", "'name'")'
      end

   'LIFO> set def nl: >FIFO'

   titems = form_items
   do queued()
      parse var titems item titems   
      call blink item 
      parse pull vms_result 
      rexx_result = f$getqui( 'DISPLAY_FORM', translate(item), name )
      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'

      rexx_result = f$getqui( 'DISPLAY_FORM', translate(item), number )
      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'
      end
   end


/* ===== entry ===== */

call notify '__display_entry'
'show queue/all >FIFO'
do queued() 
   parse pull name next entry .
   if datatype(entry)\=='NUM' then
      iterate
   
   names = names entry 
   end


do while names \= ''
   parse var names name names 

   call tell name
   titems = entry_items 
   do while titems \= ''
      parse var titems item titems
/*      queue 'write sys$output f$getqui( "DISPLAY_ENTRY", "'item'", "'name'")' */
      queue 'write sys$output f$getqui( "DISPLAY_ENTRY", "'item'", "'name'")'
      end

   'LIFO> set def nl: >FIFO'

   titems = entry_items
   do queued()
      parse var titems item titems   
      call blink item  
      parse pull vms_result 
      rexx_result = f$getqui( 'DISPLAY_ENTRY', translate(item), name )
      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'
      end
   end


/* ===== translate queue ===== */

call notify '__translate_queue'
'show queue >FIFO'
do queued() 
   parse pull line
   if (pos('>',line)=0) then iterate
   parse value reverse(line) with '>'name .
   names = names reverse(name)
   end


do while names \= ''
   parse var names name names 

   call tell name
   titems = trans_items 
   do while titems \= ''
      parse var titems item titems
      queue 'write sys$output f$getqui( "TRANSLATE_QUEUE", "'item'", "'name'")'
      end

   'LIFO> set def nl: >FIFO'

   titems = trans_items
   do queued()
      parse var titems item titems   
      call blink item 
      parse pull vms_result 
      rexx_result = f$getqui( 'TRANSLATE_QUEUE', translate(item), name )
      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'
      end
   end


/* ===== display_characteristic ===== */
char_items = 'characteristic_name characteristic_number'
call notify '__display_characteristic'
'show queue/char >FIFO'
do queued() 
   parse pull name num . 
   if datatype(num)\='NUM' then
      iterate
   
   names = names name num  
   end

do while names \= ''
   parse var names name num names 

   call tell name
   titems = char_items 
   do while titems \= ''
      parse var titems item titems
      queue 'write sys$output f$getqui( "DISPLAY_CHARACTERISTIC", "'item'", "'name'")'
      end

   'LIFO> set def nl: >FIFO'

   titems = char_items
   do queued()
      parse var titems item titems   
      call blink item 
      parse pull vms_result 
      if (vms_result=='') then
          iterate

      rexx_result = f$getqui( 'DISPLAY_CHARACTERISTIC', translate(item), name )
      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'

      rexx_result = f$getqui( 'DISPLAY_CHARACTERISTIC', translate(item), num )
      if (vms_result \== rexx_result) then
         call complain 'For item='item', DCL says 'vms_result ||,
                                              ' and rexx says' rexx_result'.'
      end
   end


call f$getqui "CANCEL_OPERATION"
do forever
   qname = f$getqui('display_queue', 'queue_name', '*') 
   if qname=='' then leave 
   do forever
      jobname = f$getqui('display_job', 'job_name')
      if jobname=='' then leave
      call tell jobname
      do forever
         filename = f$getqui('display_file', 'job_file_identification')
         if filename=='' then leave
         call blink filename
         end
      end
   end


/* ===== f$user() ===== */

call notify 'F$USER'
'write sys$output f$user() >LIFO'
parse pull line
if line\==f$user() then
   call complain 'Builtin f$user() does not seem to work'



/* ===== f$locate() ===== */
call notify 'F$LOCATE'
string.1 = 'This is a short line of text' ; loc.1 = 'is'    
string.2 = 'This is a short line of text' ; loc.2 = ' is'   
string.3 = 'This is a short line of text' ; loc.3 = 'This'  
string.4 = 'This is a short line of text' ; loc.4 = 'this'  
string.5 = 'This is a short line of text' ; loc.5 = 'text'  
string.6 = 'This is a short line of text' ; loc.6 = 'foo'   

do i=1 to 6
   push 'write sys$output f$locate("'string.i'", "'loc'")'
   end

'LIFO> set def nl: >LIFO'
do i=1 to 6
   parse pull line
   call tell i
   if line \== f$locate(string.i, loc.1 ) then
      call complain 'Builtin f$locate() does not seem to work'
   end


/* ===== f$length() ===== */

call notify 'F$LENGTH'
if f$length("Testing testing") \== 15 then
   call complain 'Builtin f$length() does not work for non-empty strings'

if f$length("") \== 0 then
   call complain 'Builtin f$length() does not work for empty strings'



/* ===== f$integer() ===== */

call notify 'F$INTEGER'
if f$integer('- 3' +10) \== '7' then 
   call complain 'Builtin f$integer() does not work'

if f$integer('- 3' -14) \== '-17' then 
   call complain 'Builtin f$integer() does not work'

if f$integer('  0  ') \== '0' then 
   call complain 'Builtin f$integer() does not work'



/* ===== f$trnlnm() ===== */
call notify 'F$TRNLNM'

'show logical/full >FIFO'


tables = ''
logicals. = ''
parse pull junk
values. = ''
index. = 0
namestab. = ''
numlogs. = 0 
allnames. = 0
parse value 0 with concealed. 1 terminal. 1 no_alias. 1 confine. 1 crelog.


call notify '__reading'
do while queued() > 0 
   parse pull '('tablename')'
   do forever
      parse pull line
      if left(line,3)=='  "' then do
         push line
         leave
         end
      end

   tables = tables tablename
   logs = 0 ;

   call tell tablename
   do mid=1 while queued() > 0
      parse pull line 
      if line='' then leave
      if left(line,1)='(' then do
         push line
         leave
         end

      parse var line '"' name '"' modes '= "' value '"' flags 1 '=' rest
      parse var modes '[' modes ']'
      parse var modes accmode','lmodes 
      parse var flags '[' flags ']'

      call blink name

      if accmode=='super' then accmode='supervisor'
      if accmode=='exec' then accmode='executive'
      allnames.accmode.name = 1 
      namestab.name = namestab.name tablename
 
      logicals.logs.tablename = name
      accmodes.logs.tablename = accmode
      logs = logs + 1

/* 
 * Note: there might be some problems here, although it is probably of 
 * academic interest. If table 'A$TABLE.0' contains the logical symbol
 * 'FOOBAR', and table 'A$TABLE' contains the logical symbol '0.FOOBAR',
 * the two of them will not be separable.
 */

      push ' =' || rest
      do while queued() > 0
         parse pull line
         if line='' then leave mid
         parse var line before '= "' value '"' '['flags']'
         if before\='' then do
            push line 
            iterate mid
            end

         if flags \== '' & lmodes\== '' then
            modes = lmodes','flags
         else
            modes = lmodes || flags

         cnt = index.accmode.tablename.name
         term = accmode"."tablename"."name

         do while modes\==''
            parse var modes mode','modes
            select
               when mode=='terminal' then terminal.cnt.term = 1
               when mode=='concealed' then concealed.cnt.term = 1
               when mode=='no_alias' then no_alias.cnt.term = 1
               when mode=='confine' then confine.cnt.term = 1
               when mode=='crelog' then crelog.cnt.term = 1
               otherwise
                  say "Unknown mode ("mode") detected, I'm confused..."
               end
            end

         values.accmode.tablename.cnt.name = value 
         index.accmode.tablename.name = index.accmode.tablename.name + 1 
         end
      end

   numlogs.tablename = logs - 1
   end


/* 
 * I really have no idea of what the 'table' item of f$trnlnm is 
 * supposed to do. I far as I know, it always return false. ???
 */


call notify '__bacics'

ttables = tables
do while ttables\=''
   parse var ttables table ttables 
   call tell table

   do log=0 to numlogs.table
      name = logicals.log.table
      mode = accmodes.log.table
      call blink name

/*
 * We have to ignore some of the values. Since the 'sh log' command was 
 * exeuted by LIB$SPAWN with output 'piped' (mailboxed!) to another 
 * process, the output will ofcourse be somewhat different.
 */
      if name=='TT' | name=='SYS$OUTPUT' | name=='SYS$ERROR' then iterate
      if name=='SYS$INPUT' | name=='SYS$COMMAND' then iterate

      max_ind = f$trnlnm(name,table,,mode,,'max_index')
      if max_ind='' & pos('.',name)>0 then 
         iterate

      if datatype(max_ind)\=='NUM' then do
         call complain 'Heeeeelp, what is happening??? name='name
         iterate 
         end

      if index.mode.table.name\==f$trnlnm(name,table,,mode,,'max_index')+1 then
         call complain 'Wrong number of indexes in logical names', 
            '(tab='table'/nam='name'/mod='mode')',
            'rexx='f$trnlnm(name,table,,mode,,'max_index'),
            'dcl='index.mode.table.name

      do cnt=0 to index.mode.table.name-1
         if length(values.mode.table.cnt.name) \==,
                                 f$trnlnm(name,table,cnt,mode,,'length') then
            call complain 'Mismatching length of locical name',
               '(tab='table'/nam='name'/cnt='cnt'/mod='mode')',
               'rexx=<'f$trnlnm(name,table,cnt,mode,,'length')'>',
               'dcl=<'length(values.mode.table.cnt.name)'>'

         xname = f$trnlnm(name,table,cnt,mode) 
         xyz = translate(xname,,xrange('7e'x,'1f'x),'.')
         if values.mode.table.cnt.name \== xyz then
            call complain 'Mismatching values in logical names',
               '(tab='table'/nam='name'/cnt='cnt'/mod='mode')',
               'rexx=<'xyz'>' 'dcl=<'values.mode.table.cnt.name'>'

         flags = 'terminal concealed confine crelog no_alias'
         do while flags\==''
            parse var flags flag flags
            if \(value(flag'.cnt.mode.table.name') &&,
                  f$trnlnm(name,table,cnt,mode,,flag)=='FALSE') then
               call complain 'Boolean info about logical is incorrect',
                  '(tab='table'/nam='name'/cnt='cnt'/mod='mode'/bool='flag')',
                  'dcl='value(flag'.cnt.mode.table.name'),
                  'rexx='f$trnlnm(name,table,cnt,mode,,flag) 
            end
         end
      end
   end



nextcheck:

call notify '__reversed'

ttables = tables
do while ttables\=''
   parse var ttables table ttables 
   call tell table 

   do log=0 to numlogs.table
      name = logicals.log.table
      mode = accmodes.log.table
      call blink name

      if name=='SYS$ERROR' | name=='SYS$OUTPUT' | name=='TT' then
         iterate

      if pos('.',name)>0 then iterate

      modes = 'user supervisor executive kernel none'
      do while modes\=='' 
         parse var modes accmode modes
         if accmode=='none' then 
            call complain "Oops, I can't find any access modes at all for",
                 'logical name' name

         if allnames.accmode.name then do
            if hepp(f$trnlnm(name,,,,,'access_mode')) == translate(accmode) then do
               tab = hepp(f$trnlnm(name,,,,,'table_name'))
               if (hepp(f$trnlnm(name,,,,,'value')) \== ,
                                               values.accmode.tab.0.name) then
                  call complain "Default mapping of logicals is incorrect",
                       '(tab='tab',nam='name',mod='accmode')'
               if (f$trnlnm(name,,,,,'value') \== f$logical(name) |,
                      hepp(f$logical(name)) \== values.accmode.tab.0.name) then
                  call complain "Default mapping of logicals is incorrect",
                       '(tab='tab',nam='name',mod='accmode')'
               leave
               end
/*          else
               call complain "Can't find correct logical table",
                                  '(tab='table',nam='name') dcl='accmode,
                                  'rexx='f$trnlnm(name,,,,,'access_mode') */
            end
         end
      end
   end

signal after_hepp

hepp: return translate(arg(1),,xrange('7e'x,'1f'x),'.')


after_hepp:


/* ===== f$parse() ===== */
call notify 'F$PARSE'

spec.1 = 'nod dev dir.dir fil typ ver'

i_node = 'foo'
i_device = 'foo'
i_directory = 'foo'
i_name = 'foo'
i_type = 'foo'
i_version = 'foo'

max_cnt = 400

'sh def >LIFO'
parse pull line
line = strip(translate( line, '<>', '[]' ))
parse var line defdev':<'defdir'>'

do cnt=1 to max_cnt
   def = filespec( 'DEF' ) 
   rel = filespec( 'REL' )
   fil = filespec( 'FIL' ) 

   answer = translate(calc_file( fil, def, rel, defdev, defdir), '<>', '[]')
   call blink answer
   if def=='' & rel=='' then 
      rexx = translate(f$parse(fil,,,, 'syntax_only'), '<>', '[]') 
   else if def=='' then
      rexx = translate(f$parse(fil,, rel,, 'syntax_only'), '<>', '[]') 
   else if rel=='' then 
      rexx = translate(f$parse(fil, def,,, 'syntax_only'), '<>', '[]') 
   else
      rexx = translate(f$parse(fil, def, rel,, 'syntax_only'), '<>', '[]') 

   if answer \== rexx then do
      'write sys$output f$parse("'fil'","'def'","'rel'",,"SYNTAX_ONLY") >LIFO'
      parse pull dcl
      call complain 'f$parse() doesn''t work, rexx='rexx', correct='answer,
               'fil='fil', def='def', rel='rel', dcl='dcl
      end
   
   items = translate('node device directory name type version')
   do while items\==''
      parse var items item items
      if value('I_'item) \==,
            translate(f$parse(fil,def,rel,item,'syntax_only'),'<>','[]') then
         call complain 'f$parse returns incorrect answer for item='item,
             'rexx='f$parse(fil,def,rel,item,'syntax_only'),
             'correct='value('I_'item)
      end
   end


   res = f$parse("foo::bar::bar:bar:",,,,'syntax_only')
   if res\=='' then 
      call complain "f$parse() doesn't trap syntax errors in filename"

   res = f$parse("foo","foo::bar::bar:bar:",,,'syntax_only')
   if res\=='' then 
      call complain "f$parse() doesn't trap syntax errors in default spec"

   res = f$parse("foo",,"foo::bar::bar:bar:",,'syntax_only')
   if res\=='' then 
      call complain "f$parse() doesn't trap syntax errors in related spec"

   if f$parse('foo::bar')\=='FOO::BAR.;' then 
      call complain "Something is wrong with f$parse()"

   if f$parse('foo:bar')\=='' then 
      call complain "Something is wrong with f$parse()"

   if f$parse('<This_is_a_very_very_long_name_for_a_dir_>foo.bar')\=='' then 
      call complain "Long directory name not detected"

   if f$parse('<foobar>foo.bar')\=='' then 
      call complain "Didn't trap non-existing directory"

   if f$parse('sys$sysroot:bar')\=='' then 
      call complain "Didn't trap logical name expanding to two directories"

   if f$parse('sys$login:foo.bar.bar')\=='' then 
      call complain "Didn't trap syntax error in file name"

   if f$parse('sys$login:fo#()+| $!~`''"\<>bar')\=='' then 
      call complain "Didn't trap syntax error in file name"

   if f$parse('sys$sysroot:bar')\=='' then 
      call complain "Didn't trap loop in logical name definition"

   if f$parse('long_node_name::bar')\=='' then 
      call complain "Didn't trap syntax error in DECnet node name"

   if f$parse('sys$login:foo.bar&$#<>\+~.')\=='' then 
      call complain "Didn't trap syntax error in file name"

   if f$parse('sys$login:foo.bar;bar')\=='' then 
      call complain "Didn't trap syntax error in file name"

signal after_parse


filespec: procedure 
   pref = arg(1)
   result = ''
   if random(,100)>80 then result = pref || 'NODE::'
   if random(,100)>60 then result = result || pref || 'DEV:'
   if random(,100)>60 then result = result || '<' || pref || 'DIR>'
   if random(,100)>50 then result = result || pref || 'FILE'
   if random(,100)>70 then result = result || '.' || pref || 'TYP'
   if random(,100)>85 then result = result || ';' || c2d(left(pref,1))
   return result 


/* 
 * The algorithm for this parsing has been found empirically. I believe
 * that there must be a bug in VMS in sys$parse, because the behavior 
 * of that function is so 'interesting' for special cases.
 */
calc_file: procedure expose i_node i_directory i_device i_name i_type i_version
   fil = arg(1) 
   def = arg(2)
   rel = arg(3)

   defdev = arg(4)
   defdir = arg(5) 

   miss_nod = 1
   miss_dev = 1
   miss_dir = 1 
   miss_fil = 1
   miss_typ = 1
   miss_ver = 1

   if pos("::",fil)>0 then do
      parse var fil nod"::"fil; miss_nod = 0 ; end

   if pos(":",fil)>0 then do
      parse var fil dev":"fil; miss_dev = 0 ; end

   if pos("<",fil)>0 then do
      parse var fil '<'dir'>'fil; miss_dir = 0 ; end 

   if pos(";",fil)>0 then do
      parse var fil fil';'ver; miss_ver = 0 ; end

   if pos('.',fil)>0 then do
      parse var fil fil'.'typ; miss_typ = 0 ; end

   if fil\=='' then miss_fil = 0
   got_a_node = \miss_nod

   if pos("::",def)>0 then do
      parse var def tnod"::"def
      rest = miss_dev + miss_dir + miss_fil + miss_typ + miss_ver 
      if miss_nod & rest>0 then do 
         nod=tnod; miss_nod=0; end; end

   if pos(":",def)>0 then do
      parse var def tdev":"def
      if miss_dev then do; dev=tdev; miss_dev=0; end; end

   if pos("<",def)>0 then do
      parse var def '<'tdir'>'def
      if miss_dir then do; dir=tdir; miss_dir=0; end; end

   if pos(";",def)>0 then do
      parse var def def";"tver
      if miss_ver then do; ver=tver; miss_ver=0; end; end

   if pos(".",def)>0 then do
      parse var def def'.'ttyp
      if miss_typ then do; typ=ttyp; miss_typ=0; end; end

   if miss_fil & def\=='' then do; fil=def; miss_fil=0; end

   if pos("::",rel)>0 then do
      parse var rel tnod"::"rel
      rest = miss_dev + miss_dir + miss_fil + miss_typ + miss_ver 
      if miss_nod & rest>0 then do 
         nod=tnod; miss_nod=0; end; end

   if pos(":",rel)>0 then do
      parse var rel tdev":"rel
      if miss_dev & (\got_a_node) then do; dev=tdev; miss_dev=0; end; end

   if pos("<",rel)>0 then do
      parse var rel '<'tdir'>'rel
      if got_a_node then tdir=''
      if miss_dir then do; 
         dir=tdir; miss_dir=0; end; 
      end

   if pos(';',rel)>0 then 
      parse var rel rel';'

   if pos(".",rel)>0 then do
      parse var rel rel'.'ttyp
      if miss_typ then do; typ=ttyp; miss_typ=0; end; end

   if miss_fil & rel\=='' then do; fil=rel; miss_fil=0; end

   if miss_dir & miss_nod then do
      miss_dir = 0 
      dir = defdir
      end

   if miss_dev & miss_nod then do
      miss_dev = 0
      dev = defdev
      end

   result = ''
   if \miss_nod then result = nod'::'
   if \miss_dev then result = result || dev || ":"
   if \miss_dir then result = result || '<' || dir || '>'
   if \miss_fil then result = result || fil
   if miss_typ then typ = '' 
   if miss_ver then ver = ''

   if miss_nod then i_node=''; else i_node=nod'::'
   if miss_dev then i_device=''; else i_device=dev':'
   if miss_dir then i_directory=''; else i_directory='<'dir'>'
   if miss_fil then i_name=''; else i_name=fil
   if miss_typ then i_type='.'; else i_type='.'typ
   if miss_ver then i_version=';'; else i_version=';'ver

   result = result || '.' || typ || ';' || ver
   return result 


after_parse:


/* ===== f$search() ===== */

call notify 'F$SEARCH'

'dir/size/ver=1000 >LIFO'
drop filnam. filtyp. filver. isfile.
pull 
pull
do cnt=1 to queued()-3
   parse pull filnam.cnt '.' filtyp.cnt ';' filver.cnt . 1 line .
   isfile.line = 1 
   end

cnt = cnt - 1  /* cnt incremented after last iteration */

types. = 0 
types = ''
do c=1 to cnt
   type = filtyp.c
   types.type = types.type + 1 
   if types.type=1 then
      types = types filtyp.c
   end

do while types\='' 
   parse var types type types 
   call tell type
   do forever 
      file = f$search( '*.'type';*' ) 
      call blink file 
      parse value translate(file,'<>','[]') with . '>' file .
      if file=='' then leave
      if isfile.file\==1 then
         call complain 'f$search() finds nonexistent file' isfile.file
      end
   end


hfile = f$search( "*.h", 1 ) 
cfile = f$search( "*.c", 2 ) 
do forever
   if (hfile\=='') then do; hfile=f$search( "*.h", 1 ); end
   if (cfile\=='') then do; cfile=f$search( "*.c", 2 ); end
   if (cfile=='' & hfile=='') then leave
   end


/* ===== f$type() ===== */
call notify 'F$TYPE'

if f$type('asdf')\=='STRING' then 
   call complain 'Believes that "asdf" is not a STRING'

if f$type('')\=='STRING' then 
   call complain 'Believes that "" is not a STRING'

if f$type('4')\=='INTEGER' then 
   call complain 'Believes that "4" is not a STRING'

if f$type(' + 3 ')\=='INTEGER' then 
   call complain 'Believes that " + 3 " is not a STRING'

if f$type('  - 123123  ')\=='INTEGER' then 
   call complain 'Believes that "  -  123123  " is not a STRING'



/* ===== f$file_attributes() ===== */
call notify 'F$FILE_ATTRIBUTES'


files = ''
/* Treat 'known' as a special case */
attribs = 'alq bdt bks bls cbt cdt ctg deq did dvi edt eof fid fsz grp',
          'mbm mrn mrs noa nok org pro pvn rat rck rdt rfm rvn uic wck'


'dir/size >LIFO'
pull
pull
do queued()-3
   parse pull file .
   files = files file
   end
pull
pull
pull
files = space(files)

do while files\==''
   parse var files file files
   if (file='' | file='Directory') then iterate
   call blink file

   attrs = attribs
   do while attrs\==''
      parse var attrs attr attrs
      push 'write sys$output f$file_attrib("'file'", "'attr'")'
      end
   'LIFO> set prompt="" >LIFO'

   attrs = attribs
   do while attrs\==''
      parse var attrs attr attrs
      parse pull outcome
      if (outcome\==f$file_attributes(file, attr)) then do
         call complain "F$file_attributes didn't work for" attr', rexx='||,
               f$file_attributes(file,attr)', dcl='outcome
         end
      end
   end

/*
 * Then we process known. Strategy for testing, is to do the following 
 * steps:
 *
 *  1) Check all files named ".EXE" in sys$system and sys$library
 *  2) Check that they are not fooled by concealed logicals
 */

call notify '__known_files'

dirs = 'SYS$SYSTEM SYS$LIBRARY' 
do while dirs\=''
   parse var dirs dir dirs
   call tell dir
   'dir/size' dir':*.exe >LIFO' 
   inc = 1 
   do queued()
      parse pull line
      if (left(line,1)==' ') then do
         prevline = line 
         iterate
         end      

      if left(line,5)=='Total' then iterate
      if left(line,5)=='Grand' then iterate
      if left(line,9)=='Directory' then iterate
      if words(line)=1 then line = line prevline
      if word(line,2)='no' then iterate
      
      parse var line files.inc ';'
      inc = inc + 1 
      end

   do i=1 to inc-1
      push 'write sys$output f$file_attributes("'dir':'files.i'","known")'
      end

   'LIFO> set def nl: >LIFO'

   do i=1 to inc-1
      parse pull line      
      if (left(line,1) == '%') then do
         parse pull junk 
         iterate
         end

      call blink files.i
      if line \== f$file_attributes(dir':'files.i,'known') then
         call complain 'F$FILE_ATTRIBUTES differs for' files.i 'rexx=' ||,
                f$file_attributes(dir':'files.i,'known'),
                'dcl='line
      end
   end

/*
 * We assume that sys$system:show exists, and is a known image (if not, 
 * you should seriously consider changing your system administrator)
 */


oldplaces = ''
place = sys$system
call notify '__recursive'
do until place==''
   if (pos(":",place)=0) then 
      place = place || ":"

   found = f$file_attributes(place"SHOW.EXE","KNOWN")
   if found\=='TRUE' then 
      if oldplaces='' & did_find=0 then
          call complain 'Didn''t known that' place 'equals sys$system'
      else do
          parse var oldplaces place oldplaces
          iterate 
          end

    parse var place place ":" "[" olddirs "]"
    place = translate(f$trnlnm( place ), "[]", "<>" ) 
    if place=='' then do
       if found=='TRUE' & did_find=0 then
	  did_find = 1 

       if oldplaces \='' then do
	  parse var oldplaces place oldplaces 
	  iterate
	  end
       leave 
       end

    call blink place 
    parse var place rplace ":" "[" xdirs "]"

    topp = f$trnlnm(rplace,,,,,'MAX_INDEX')
    if topp='' then topp=0 
    do i=1 to topp
        xplace = f$trnlnm(rplace,,i)
        if right(xplace,1)==':' & xdirs\='' then
           xplace = xplace || '['xdirs']'
	else if right(xplace,2)=='.]' & xdirs\='' then
	   xplace = left(xplace,length(place)-2) || '.' || xdirs || "]"
	else if xdirs\='' then
	   xplace = xplace || ":[" || xdirs || "]"
	 oldplaces = oldplaces xplace
       end

    if right(place,2)=='.]' & olddirs\='' then
       place = left(place,length(place)-2) || '.' || olddirs || "]"
    else if olddirs\='' then
       place = place || ":[" || olddirs || "]"

    end

 if did_find=0 then
    call complain 'Didn''t find real file for SHOW.EXE'


 /* ===== f$extract ===== */
 call notify 'F$EXTRACT'
    if f$extract(3,4,'')\=='' then
       call complain 'Didn''t handle f$extract correctly'

    if f$extract(3,4,'foobar')\=='bar' then
       call complain 'Didn''t handle f$extract correctly'

    if f$extract(3,2,'foobar')\=='ba' then
       call complain 'Didn''t handle f$extract correctly'


 /* ===== f$element ===== */
 call notify 'F$ELEMENT'
    if f$element(0,'/','foo/bar/baz')\=='foo' then
       call complain 'Didn''t handle f$element correctly'

    if f$element(1,'/','foo/bar/baz')\=='bar' then
       call complain 'Didn''t handle f$element correctly'

    if f$element(2,'/','foo/bar/baz')\=='baz' then
       call complain 'Didn''t handle f$element correctly'

    if f$element(3,'/','foo/bar/baz')\=='/' then
       call complain 'Didn''t handle f$element correctly'

    if f$element(0,'/','/foo/bar/baz')\=='' then
       call complain 'Didn''t handle f$element correctly'

    if f$element(1,'/','/foo/bar/baz')\=='foo' then
       call complain 'Didn''t handle f$element correctly'

    if f$element(2,'/','/foo//bar/baz')\=='' then
       call complain 'Didn''t handle f$element correctly'

    if f$element(10,'/','')\=='/' then
       call complain 'Didn''t handle f$element correctly'


 /* ===== f$edit ===== */
 call notify 'F$EDIT'
    if f$edit('foo bar', 'upcase') \== 'FOO BAR' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit('FOO BAR', 'lowerCASE') \== 'foo bar' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit('Foo bAR', 'upcase,lowercase') \== 'fOO Bar' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit('foo! bar', 'uncomment') \== 'foo' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit(' foo  bar ', 'trim') \== 'foo  bar' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit(' foo  bar ', 'collapse') \== 'foobar' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit(' foo  bar ', 'compress') \== ' foo bar ' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit(' foo  bar ', 'trim,compress') \== 'foo bar' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit('foo 	 bar', 'compress') \== 'foo bar' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit('foo ! bar', 'uncomment') \== 'foo ' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit('foo ! bar', 'uncomment,trim') \== 'foo' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit('foo!bar', 'uncomment') \== 'foo' then
       call complain 'Didn''t handle f$edit correctly'

    if f$edit(' foo bar ', 'collapse,compress') \== 'foobar' then
       call complain 'Didn''t handle f$edit correctly'



 /* ===== F$CVUI ===== */
 call notify 'F$CVUI'
    if f$cvui(2,4,'deadbeef'x)\=='7' then 
       call complain 'Didn''t handle f$cvui correctly'

    if f$cvui(0,0,'deadbeef'x)\=='0' then 
       call complain 'Didn''t handle f$cvui correctly'

    if f$cvui(0,0,'')\=='0' then 
       call complain 'Didn''t handle f$cvui correctly'

    if f$cvui(0,4,'deadbeef'x)\=='13' then 
       call complain 'Didn''t handle f$cvui correctly'

    if f$cvui(2,0,'deadbeef'x)\=='0' then 
       call complain 'Didn''t handle f$cvui correctly'


 /* ===== F$CVSI ===== */
 call notify 'F$CVSI'
    if f$cvsi(2,4,'deadbeef'x)\=='7' then 
       call complain 'Didn''t handle f$cvsi correctly'

    if f$cvsi(0,7,'deadbeef'x)\=='-17' then 
       call complain 'Didn''t handle f$cvsi correctly'

    if f$cvsi(0,1,'0'b)\=='0' then 
       call complain 'Didn''t handle f$cvsi correctly'

    if f$cvsi(0,4,'deadbeef'x)\=='-3' then 
       call complain 'Didn''t handle f$cvsi correctly'

    if f$cvsi(2,1,'deadbeef'x)\=='0' then 
       call complain 'Didn''t handle f$cvsi correctly'



 start_here:

 /* ===== f$cvtime() ===== */
 call notify 'F$CVTIME'
    if (f$cvtime(,,'year')\==word(date(),3)) then 
       call complain 'Didn''t handle simple f$cvtime for output of year'

    if (f$cvtime(,,'month')\==substr(date('e'),4,2)) then 
       call complain 'Didn''t handle simple f$cvtime for output of month'

    if (f$cvtime(,,'day')\==substr(date('e'),1,2)) then 
       call complain 'Didn''t handle simple f$cvtime for output of day'

    if (f$cvtime(,,'hour')\==substr(time(),1,2)) then 
       call complain 'Didn''t handle simple f$cvtime for output of hour'

    if (f$cvtime(,,'minute')\==substr(time(),4,2)) then 
       call complain 'Didn''t handle simple f$cvtime for output of minute'

    if (f$cvtime(,,'second')\==substr(time(),7,2)) then 
       call complain 'Didn''t handle simple f$cvtime for output of second'

    vax = f$cvtime(,,'hundredth')
    if (length(vax)\=2 | datatype(vax)\=='NUM') then
       call complain 'Didn''t handle simple f$cvtime for output of hundredth'

    if (f$cvtime(,,'weekday')\==date('w')) then 
       call complain 'Didn''t handle simple f$cvtime for output of weekday'

    if (f$cvtime(,,'date')\==insert('-',insert('-',date('s'),6),4)) then 
       call complain 'Didn''t handle simple f$cvtime for output of date'

    /* don't check the hundredth of seconds */
    vax = f$cvtime(,,'time')
    rexx = left(time('l'),11)
    if length(vax)\=length(rexx) |,
	    left(rexx,length(rexx)-2)\==left(vax,length(vax)-2) then
       call complain 'Didn''t handle simple f$cvtime for output of time'

    vax = f$cvtime(,,'datetime')
    rexx = insert('-',insert('-',date('s'),6),4) left(time('l'),11)
    if length(vax)\=length(rexx) | ,
	    left(rexx,length(rexx)-2)\==left(vax,length(vax)-2) then
       call complain 'Didn''t handle simple f$cvtime for output of datetime'



    abs = 'absolute'
    if (f$cvtime(,abs,'year')\==word(date(),3)+0) then 
       call complain 'Didn''t handle simple f$cvtime for output of year'

    if (f$cvtime(,abs,'month')\==translate(left(date('m'),3))) then 
       call complain 'Didn''t handle simple f$cvtime for output of month'

    if (f$cvtime(,abs,'day')\==substr(date('e'),1,2)+0) then 
       call complain 'Didn''t handle simple f$cvtime for output of day'

    if (f$cvtime(,abs,'hour')\==substr(time(),1,2)) then 
       call complain 'Didn''t handle simple f$cvtime for output of hour'

    if (f$cvtime(,abs,'minute')\==substr(time(),4,2)) then 
       call complain 'Didn''t handle simple f$cvtime for output of minute'

    if (f$cvtime(,abs,'second')\==substr(time(),7,2)) then 
       call complain 'Didn''t handle simple f$cvtime for output of second'

    vax = f$cvtime(,abs,'hundredth')
    if (length(vax)\=2 | datatype(vax)\=='NUM') then
       call complain 'Didn''t handle simple f$cvtime for output of hundredth'

    if (f$cvtime(,abs,'weekday')\==date('w')) then 
       call complain 'Didn''t handle simple f$cvtime for output of weekday'

    if (f$cvtime(,abs,'date')\==translate(translate(date()),'-',' ')) then 
       call complain 'Didn''t handle simple f$cvtime for output of date'

    /* don't check the hundredth of seconds */
    vax = f$cvtime(,abs,'time')
    rexx = left(time('l'),11)
    if length(vax)\=length(rexx) |,
	    left(rexx,length(rexx)-2)\==left(vax,length(vax)-2) then
       call complain 'Didn''t handle simple f$cvtime for output of time'

    vax = f$cvtime(,abs,'datetime')
    rexx = translate(translate(date(),'-',' ')) left(time('l'),11)
    if length(vax)\=length(rexx) | ,
	    left(rexx,length(rexx)-2)\==left(vax,length(vax)-2) then
       call complain 'Didn''t handle simple f$cvtime for output of datetime'



    tim = '33-07:09:06.34'
    del = 'delta'
    if (f$cvtime(tim,del,'day')\=='33') then 
       call complain 'Didn''t handle simple f$cvtime for output of day'

    if (f$cvtime(tim,del,'hour')\=='07') then 
       call complain 'Didn''t handle simple f$cvtime for output of hour'

    if (f$cvtime(tim,del,'minute')\=='09') then 
       call complain 'Didn''t handle simple f$cvtime for output of minute'

    if (f$cvtime(tim,del,'second')\=='06') then 
       call complain 'Didn''t handle simple f$cvtime for output of second'

    if f$cvtime(tim,del,'hundredth')\=='34' then
       call complain 'Didn''t handle simple f$cvtime for output of hundredth'

    if (f$cvtime(tim,del,'date')\=='33') then 
       call complain 'Didn''t handle simple f$cvtime for output of date'

    if f$cvtime(tim,del,'time')\=='07:09:06.34' then 
       call complain 'Didn''t handle simple f$cvtime for output of time'

    if f$cvtime(tim,del,'datetime')\=='33 07:09:06.34' then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'


    abs = 'absolute'
    tim = '12-JAN-1993:13:23:34.45'
    if (f$cvtime(tim,abs,'year')\=='1993') then 
       call complain 'Didn''t handle simple f$cvtime for output of year'

    if (f$cvtime(tim,abs,'month')\=='JAN') then 
       call complain 'Didn''t handle simple f$cvtime for output of month'

    if (f$cvtime(tim,abs,'day')\=='12') then 
       call complain 'Didn''t handle simple f$cvtime for output of day'

    if (f$cvtime(tim,abs,'hour')\=='13') then 
       call complain 'Didn''t handle simple f$cvtime for output of hour'

    if (f$cvtime(tim,abs,'minute')\=='23') then 
       call complain 'Didn''t handle simple f$cvtime for output of minute'

    if (f$cvtime(tim,abs,'second')\=='34') then 
       call complain 'Didn''t handle simple f$cvtime for output of second'

    if (f$cvtime(tim,abs,'hundredth')\=='45') then 
       call complain 'Didn''t handle simple f$cvtime for output of hundredth'

    if (f$cvtime(tim,abs,'date')\=='12-JAN-1993') then 
       call complain 'Didn''t handle simple f$cvtime for output of date'

    if (f$cvtime(tim,abs,'time')\=='13:23:34.45') then 
       call complain 'Didn''t handle simple f$cvtime for output of time'

    if (f$cvtime(tim,abs,'datetime')\=='12-JAN-1993 13:23:34.45') then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'

    date = translate(translate(date()),'-',' ')
    if (f$cvtime('::34.56',abs)\==date left(time(),5)':34.56') then 
       call complain 'Didn''t handle simple f$cvtime for output of time'

    vax = f$cvtime('DEC::34',abs)
    if left(vax,length(vax)-3) \== word(date(),1)'-DEC-'word(date(),3),
					      overlay('34',time(),4) then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'

    vax = f$cvtime('25-:14',abs)
    if left(vax,length(vax)-3)\==,
	    '25-'translate(word(date(),2))'-'word(date(),3),
	    '14:'substr(time(),4) then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'

    if f$cvtime('+:5',abs,'minute')\=(substr(time(),4,2)+5)//60 then
       call complain 'Didn''t handle simple f$cvtime for output of datetime'

    if f$cvtime('-1-00',abs,'day')\==word(date(),1)-1 then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'

    if f$cvtime('3:-1-10',abs,'hour')\=='17' then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'

    if f$cvtime('-1-',abs,'day')\==word(date(),1)-1 then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'

    if f$cvtime('-1',abs,'day')\==word(date(),1) then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'

    if f$cvtime('-3-:30',abs,'day')\==word(date(),1)-3 then 
       call complain 'Didn''t handle simple f$cvtime for output of datetime'


    if f$cvtime("10:59:59.999","comparison",'time')\=='11:00:00.00' then
       call complain 'Didn''t round comparison time properly'

    if f$cvtime("10:59:59.999","absolute",'time')\=='11:00:00.00' then
       call complain 'Didn''t round absolute time properly'

    if f$cvtime("10:59:59.999","delta",'time')\=='11:00:00.00' then
       call complain 'Didn''t round delta time properly'


 /* ===== f$fao ===== */
 call notify 'F$FAO'

    test = f$fao("Foo !3(10UB) bar", 123, -234, 345 ) 
    if test\=='Foo' format(123,10) || format(-234+256,10) ||,
				       format(345//256,10) 'bar' then
       call complain "f$fao doesn't handle !UB correctly"

    test = f$fao("Foo !3(10UW) bar", 123, -234, 345 ) 
    if test\=="Foo" format(123,10) || format(-234+(2**16),10) ||,
				  format(345,10) 'bar' then
       call complain "f$fao doesn't handle !UW correctly"

    numeric digits 16
    test = f$fao("Foo !3(12UL) bar", 123, -234, 345 ) 
    if test\=="Foo" format(123,12) || format(4294967062,12) ||,
				  format(345,12) 'bar' then
       call complain "f$fao doesn't handle !UW correctly"

    test = f$fao( "One !SW !SB !2(5AS)!2(2UB) !XB !UL !OB",,
		 2, 3, 'four', 'five', -250, 7, 8, 9, 8 )
    if test\== "One 2 3 four five  6 7 08 9 010" then
       call complain "Mixed f$fao didn't work"



exit 

    'sh dev sys$login: /full >LIFO'
    disk_dev = ''
    do queued()
       parse pull line 
       disk_dev = disk_dev line
       end



 say 'hallo'
 exit 0



 syntax:
    say 'oops ... feil for item='item
    signal on syntax 
 signal restart

    'sh dev sys$login: /full >LIFO'
    disk_dev = ''
    do queued()
       parse pull line 
       disk_dev = disk_dev line
       end



 say 'hallo'
 exit 0



 complain: procedure expose global. sigl
    trace off 
    say ' ...ouch...'
    say 'Tripped in line' sigl':' 'ARG'(1)'.'
    global.point = 0 
 return


 blink: procedure expose global.
    parse arg word .
    if global.point=0 then do
       call charout ,global.indent' '
       global.point = length(global.indent' ')
       global.blink = 0
       end

    if (global.point + length(word) + 1) > global.max then do
       call charout ,copies('082008'x,global.blink)
       say
       call charout ,global.indent word
       global.point = length(global.indent)
       end
    else
       call charout , copies('082008'x,global.blink) || ' 'word

    global.blink = length(word)+1
    return


 tell: procedure expose global.
    parse arg word .
    if global.point=0 then do
       call charout ,global.indent' '
       written = length(global.indent' ')
       global.blink = 0
       end

    call blink ''
    global.point = global.point + length(word) + 2 
    if global.point>75 then do
       say ','
       global.point = length(global.indent word)
       call charout , global.indent word
       end
    else
       call charout , word', '
    return

 notify: procedure expose global.
    parse arg word .
    if datatype(global.point)\='NUM' then do
       global.indent = '      '
       global.point = 0 
       global.max = 79
       global.blink = 0 
       end

    call blink ''
    if global.point\=0 then
       say

    call charout ,translate(word)' ==> '
    global.point = length(word' ==> ')
    return

