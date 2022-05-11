;
; This is stub for Windows 1.01 logo
; Supported up to Win 3.xx
;
; Logo API
;
; All code compiled as raw binary with offset 0.
;
; First 4 bytes must contain 'LOGO' signature
; Entry table consist of 2 entry points:
;  ShowLogo - Switch display to graphics mode, show logo and return last required byte of logo code in AX
;  HideLogo - Switch display back to original mode
;

.8086
			include	video.inc

			org	0h

			db	'LOGO'
			jmp	near ptr Init
;			jmp	near ptr Done
Done:			@SetMode [CurrentVideoMode]
			retf
CurrentVideoMode	db	?
;--------------------------------------------------------
DeadSpace:		
Init:			push	ds
			push	cs
			pop	ds
			@GetMode
		        mov     [CurrentVideoMode],al
			@SetMode 2
			lea	dx, Hello
			mov	ah,09H
			int	21H
			lea	ax, DeadSpace
			pop	ds
			retf

Hello			db	10,13,10,13
			db	"                             ,.=:^!^!t3Z3z.,             ",10,13
			db	"                            :tt:::tt333EE3               ",10,13
			db	"                            Et:::ztt33EEE  @Ee.,      ..,",10,13
			db	"                           ;tt:::tt333EE7 ;EEEEEEttttt33#",10,13
			db	"                          :Et:::zt333EEQ. SEEEEEttttt33QL",10,13
			db	"                          it::::tt333EEF @EEEEEEttttt33F ",10,13
			db	"                         ;3=*^```'*4EEV :EEEEEEttttt33@. ",10,13
			db	"                         ,.=::::it=., ` @EEEEEEtttz33QF  ",10,13
			db	"                        ;::::::::zt33)   '4EEEtttji3P*   ",10,13
			db	"                       :t::::::::tt33.:Z3z..  `` ,..g.   ",10,13
			db	"                       i::::::::zt33F AEEEtttt::::ztF    ",10,13
			db	"                      ;:::::::::t33V ;EEEttttt::::t3     ",10,13
			db	"                      E::::::::zt33L @EEEtttt::::z3F     ",10,13
			db	"                     {3=*^```'*4E3) ;EEEtttt:::::tZ`     ",10,13
			db	"                                 ` :EEEEtttt::::z7       ",10,13
			db	"                                     'VEzjt:;;z>*`       ",10,13,10,13,10,13
			db	"                              Windows loading...",10,13,10,13
			db	"                Text Logo Example by Yuri Prokushev (C) 2022$"
;https://github.com/nijikokun/WinScreeny/blob/master/screeny
;$f1         ,.=:^!^!t3Z3z.,                
;$f1        :tt:::tt333EE3                  
;$f1        Et:::ztt33EEE  $f2@Ee.,      ..,   
;$f1       ;tt:::tt333EE7 $f2;EEEEEEttttt33#   
;$f1      :Et:::zt333EEQ.$f2 SEEEEEttttt33QL   
;$f1      it::::tt333EEF $f2@EEEEEEttttt33F    
;$f1     ;3=*^\`\`\`'*4EEV $f2:EEEEEEttttt33@. 
;$f4     ,.=::::it=., $f1\` $f2@EEEEEEtttz33QF 
;$f4    ;::::::::zt33)   $f2'4EEEtttji3P*      
;$f4   :t::::::::tt33.$f3:Z3z..  $f2\`\` $f3,..g.     
;$f4   i::::::::zt33F$f3 AEEEtttt::::ztF      
;$f4  ;:::::::::t33V $f3;EEEttttt::::t3       
;$f4  E::::::::zt33L $f3@EEEtttt::::z3F       
;$f4 {3=*^\`\`\`'*4E3) $f3;EEEtttt:::::tZ\`   
;$f4             \` $f3:EEEEtttt::::z7        
;$f3                 $f3'VEzjt:;;z>*\`        


			end
