#! /bin/sh
# the next line restarts using expectk \
exec expectk "$0" "$@"


#
# Copyright (C) 2000-2001, Sun Microsystems, Inc
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#


# Tcl/tk script: GUI interface for UQBT
#
# Cristina Cifuentes, created, 18-22 Jan 00
# GUI design by Robin Jeffries (13 Jan 00)
# Tcl/tk SpecTcl tutorial by Steve Uhler (18 Jan 00)
#
# 30 Oct 00: Cristina - integrated with latest version of uqbt  
# 07 Nov 00: Cristina - added directory options to output file commands 
# 19 Mar 01: Brian - Fixed problems with the JVM demo. Corrected paths.
# 20 Apr 01: Brian - Many changes, including the ability to type input
#                    to a translated program.
# 25 Apr 01: Brian - Removed debugging output command.
# 26 Nov 01: Cristina - Removed all references to logos

tk_setPalette #dddddd

tk appname UQBT
tk scaling 2		;# so that it looks bigger, 72x2 pixels per inch
source uqbt.ui.tcl
uqbt_ui .
wm title . "UQBT"

##
## General configuration.
## You need to have the following programs accessible from your path: 
## 		makeas (part of the UQBT distribution)  
##		dotty (part of the AT&T Graphviz distribution)  
##		wish (part of Scriptics' tcl/tk  distribution)  
##		java (if translating to JVM bytecodes) 
##

# The path to uqbt binaries
set uqbtPath .. 

# The path to the directory where this code is run from
catch {exec pwd} baseDir
puts $baseDir

# The path to the files to translate (base directory).
# Note that "dir" and "olddir" should be set to the same location. 
set dir    "[file dirname [pwd]]/test"
set olddir $dir

##
## Initialization of globals
##

set fileName ""
set translatedFileName ""
set translatedDirName ""
set runFileName ""
set sourceMachine ""
set targetMachine ""
set targetName ""
set uqbtRunWindowId 0
set cmdFP 0


## 
## Functions
##

# Launches window of given name with the contents of the given file
proc displayFile {widgetName windowName fileName} {
	pack forget $widgetName
	toplevel $widgetName
	wm title $widgetName $windowName
    frame $widgetName.dummy -width 12c -height 0.1c
	pack $widgetName.dummy -side top -fill x
	button $widgetName.quit -text "Close" -command "loseit $widgetName"
	text $widgetName.text -relief sunken -bd 2 \
        -yscrollcommand "$widgetName.scroll set" \
        -width 80 \
        -height 45
	scrollbar $widgetName.scroll -command "$widgetName.text yview"
	pack $widgetName.scroll -side right -fill y
	pack $widgetName.quit -side bottom 
	pack $widgetName.text -side left -side bottom -fill both -expand 1
	loadFile $fileName $widgetName.text
}

# Invokes displayFile() with rtl filename
proc displayRtl {widgetName windowName} {
	global translatedFileName

	set rtlName $translatedFileName  
	append rtlName ".rtl"
	displayFile $widgetName $windowName $rtlName
}

# Invokes displayFile() with hrtl filename
proc displayHrtl {widgetName windowName} {
	global translatedFileName

	set hrtlName $translatedFileName
	append hrtlName ".hrtl"
	displayFile $widgetName $windowName $hrtlName
}

# Invokes displayFile() with main.c filename
proc displayC {widgetName windowName} {
	global translatedFileName

    set idx [string last "/" $translatedFileName]
    set cName [string range $translatedFileName 0 $idx]
    append cName "main.c"
	displayFile $widgetName $windowName $cName
}

# Invokes displayFile() with Java bytecode filename
proc displayBytecode {widgetName windowName} {
	global translatedFileName

	set idx [string last "/" $translatedFileName]
	set bytecodeName [string range $translatedFileName 0 $idx]
	append bytecodeName "Uqbt.j"  
	displayFile $widgetName $windowName $bytecodeName
}

#
# Read in contents of file into given widget
#
proc loadFile {fileName widgetName} {
	$widgetName delete 1.0 end
	set f [open $fileName]
	while {! [eof $f]} {
		$widgetName insert end [read $f 1000]
	}
	close $f
}

#
# unrealises widget given as parameter
#
proc loseit widgetName {
	destroy $widgetName
}

#
# Displays a window to select a file to be opened
#
proc selectFile {} {
	global fileName
	if {$fileName != ""} closefile

	toplevel .fv
	wm minsize .fv 10 10
	wm title .fv "Select File"

	frame .fv.top 
	frame .fv.middle

	global dir
	label .fv.top.l 
	pack .fv.top.l -side left -ipady 2m -ipadx 2m

	button .fv.bottom -text "Select" -command \
		{foreach i [.fv.middle.list curselection] \
			{set dir [select [.fv.middle.list get $i]]}} 

	scrollbar .fv.middle.scroll -command ".fv.middle.list yview"
	listbox .fv.middle.list -yscroll ".fv.middle.scroll set" -relief raised \
		-height 15 -width 30 -setgrid yes 

	pack .fv.middle.list -side left -fill both -expand yes -anchor w
	pack .fv.middle.scroll -side left -fill y -expand yes -anchor w

	foreach i [ exec ls -a1 $dir | sort ] {
        .fv.middle.list insert end $i
	}

	if { $dir == "/" } {
		.fv.top.l configure -text "/"
	} else {
		.fv.top.l configure -text "$dir/"
	}

	pack .fv.top -side top -fill x -expand true
	pack .fv.middle -side top -padx 2m -fill both -expand true
	pack .fv.bottom -side top -padx 2m -pady 2m

	bind .fv.middle.list <Double-Button-1> {
		global dir
		foreach i [.fv.middle.list curselection] {
			set dir [select [.fv.middle.list get $i]]
		}
	}
}

# reset all translator options and buttons
proc resetAllOptions {} {
	global sourceMachine targetMachine

    # reset source and target machine radio buttons
    set sourceMachine s
    set targetMachine s

    resetAllButMachineOptions
}
    
# reset all options except those for the source and target machine
proc resetAllButMachineOptions {} {
    global createCFG createCallGraph createRTL createHRTL createC
    global createBytecode
    global runFileName

	# disable option buttons
    set createCFG       0
    set createCallGraph 0
    set createRTL       0
    set createHRTL      0
    set createC         0
    set createBytecode  0

	# disable display buttons
	.displayCFGWidget configure -state disabled
    .displayCallGraphWidget configure -state disabled
    .displayRTLWidget configure -state disabled
    .displayHRTLWidget configure -state disabled
    .displayCWidget configure -state disabled
    .displayBytecodeWidget configure -state disabled

    # reset run file name widget
    set runFileName ""

    # display run program and translated file command buttons
    .runProgramWidget configure -state disabled
	.translatedFileCommandWidget configure -state disabled
}
    
# sets the selected file as the current opened
proc select {file} {
	global fileName
	global dir

    if [file isdirectory "$dir/$file"] {
        if { $file == "." } {
        } else {
        if { $file == ".." } {
          set dir [file dirname "$dir"]
          if { $dir == "." } {
            set dir "/"
          }
        } else {
          if { $dir == "/" } {
            set dir "/$file"
          } else {
            set dir "$dir/$file"
          }
        }
      }
      .fv.middle.list delete 0 end
      foreach i [exec ls -a1 $dir | sort ] {
        .fv.middle.list insert end $i
      }
      if { $dir == "/" } {
        .fv.top.l configure -text "/"
      } else {
        .fv.top.l configure -text "$dir/"
      }
      return $dir
    }

    global olddir
    set olddir "$dir"
    set fileName "$dir/$file"
    
    # make the end of the text in the input program file name's entry widget
    # visible
    .entry#1 xview end

    resetAllOptions
 
    destroy .fv 
}


# symbolically closes current file    
proc closefile {} {
	destroy .labl .value
	global fileName
	set fileName ""
	global dir
	global olddir
	set dir "$olddir"
}

#
# Invokes relevant version of uqbt with appropriate command-line 
# options.  Makes the relevant generated files.  Enables the 
# display buttons on the interface. 
#
proc translate {} {
	global sourceMachine targetMachine
	global createCFG createCallGraph createRTL createHRTL createC
    global createBytecode
 	global fileName uqbtPath baseDir
    global translatedFileName translatedDirName runFileName
	set commandName "$uqbtPath/"
	set translatorName "uqbt"
	set commandOptions ""

	# determine appropriate translator
	if {$sourceMachine == "s" || $sourceMachine == "p"} {
		append translatorName $sourceMachine
	} else {
		error "Select a source machine; expecting Sparc or Pentium"
	}

	if {$targetMachine == "s" || $targetMachine == "p"} { 
		append translatorName $targetMachine
	} elseif {$targetMachine == "j"} {
        if {$sourceMachine == "p"} {
            error "Source machine for JVM translation must be Sparc"
        }
		append translatorName "s"	;# should be the machine it's running on
	} else {
		error "Select a target machine; expecting Sparc, Pentium, or Java virtual machine"
	}

	# endianness swaps
	if {$sourceMachine != $targetMachine} {
		append commandOptions "-e "
	} 

	# process options
	if {$createCFG == 1} {
		append commandOptions "-D "
	}
	if {$createCallGraph == 1} {
		append commandOptions "-G "
	}
	if {$createRTL == 1} {
		append commandOptions "-r "
	}
 	if {$createHRTL == 1} {
 		append commandOptions "-R "
 	}
    if {$targetMachine == "j"} {
		append commandOptions "-j "
	}

	# determine directory and filename of translated program
	set idx [string last "/" $fileName]
	incr idx -1
	set translatedFileName [string range $fileName 0 $idx]
	set translatedDirName $translatedFileName
	incr idx 2
	set targetName [string range $fileName $idx end]
	append translatedDirName "/$translatorName.$targetName"
	if {$targetMachine == "s" || $targetMachine == "p" } {
 	 	append translatedFileName "/$translatorName.$targetName/$targetName"
        set runFileName $translatedFileName
	} elseif {$targetMachine == "j"} {
		set firstChar [string range $targetName 0   0]
		set restName  [string range $targetName 1 end]
		set translatedFileName $firstChar
		append translatedFileName $restName
		set translatedFileName $translatedDirName/$translatedFileName

        set firstChar [string toupper $firstChar]
        set runFileName $firstChar
        append runFileName $restName
		set jvmOptions "/proj/walkabout/jdk1.3/bin/java -cp [file dirname [pwd]]/backend/jvm/runtime:. "
        set runFileName $jvmOptions$runFileName
	}

	# add output directory
	append commandOptions "-o $translatedDirName"

	# run uqbt 
	puts "$commandName$translatorName $commandOptions $fileName"
	catch {eval exec $commandName$translatorName $commandOptions $fileName} result
	puts $result

	# make/compile generated files
	puts "cd $translatedDirName"
	cd $translatedDirName
    if [catch {open /tmp/uqbt-gui-err w} fileId] {
        puts stderr "Cannot open /tmp/uqbt-gui-err: $fileId"
    } else {
        set result ""
        if {$targetMachine == "s" || $targetMachine == "p"} {
            puts "/proj/walkabout/bin/makeas"
            if {[catch {exec /proj/walkabout/bin/makeas 2> $fileId} result]} {
                puts stderr "makeas failed: $result"
            }
        } elseif {$targetMachine == "j"} {
            puts "/proj/walkabout/bin/make java"
            if {[catch {exec /proj/walkabout/bin/make java 2> $fileId} result]} {
                puts stderr "make java failed: $result"
            }
        }
        close $fileId
    }

	# enable display and entry buttons
	if {$createCFG == 1} {
 		.displayCFGWidget configure -state normal
	}
	if {$createCallGraph == 1} {
 		.displayCallGraphWidget configure -state normal
	}
	if {$createRTL == 1} { 
 		.displayRTLWidget configure -state normal
	}
 	if {$createHRTL == 1} { 
  		.displayHRTLWidget configure -state normal
 	}
	if {$createC == 1} { 
 		.displayCWidget configure -state normal
	}
	if {($createBytecode == 1) || ($targetMachine == "j")} { 
        .displayBytecodeWidget configure -state normal
	}
 	.runProgramWidget configure -state normal
	.translatedFileCommandWidget configure -state normal

    # make the end of the text in the translated file name entry widget visible
    .translatedFileCommandWidget xview end

	cd $baseDir
}


# Display program's .dot file using graphviz (dotty)
proc displayGraph {type} {
    global translatedFileName
    set dotFile $translatedFileName

	if { $type == "cfg" } {
		append dotFile ".dot"
	} else	{		;# "callGraph" 
		append dotFile ".cg.dot"
	}
	puts "dot file to be loaded = $dotFile"
	exec /proj/walkabout/bin/dotty $dotFile &
}


# Executes the program on another window
proc runProgram {command} {
	global baseDir translatedDirName cmdFP

    set t ".uqbtTranslated[incr ::uqbtRunWindowId]"
	toplevel $t
	wm title $t $command 
	frame $t.dummy -width 12c -height 0.1c
	pack $t.dummy -side top -fill x
	button $t.quit -text "Close" -command "loseit $t"
	text $t.text -relief sunken -bd 2 \
        -yscrollcommand "$t.scroll set" \
        -width 80 \
        -height 30
    $t.text tag configure userInput -foreground blue
    $t.text tag configure end -foreground red
	scrollbar $t.scroll -command "$t.text yview"
	pack $t.scroll -side right -fill y
	pack $t.quit -side bottom 
	pack $t.text -side left -side bottom -fill both -expand 1

    puts "cd $translatedDirName"
    cd $translatedDirName
    
    # note: use "exec $command" if cmd on its own or "|$command" if a pipe
#     if [catch {set result [eval exec $command]} err] {
#         $t.text insert end $err
#     } else {
#         $t.text insert end $result
#     }
#     or set fp [open "|$command" r]
    set stty_init -echo
    eval exp_spawn $command
    set cmdFP [exp_open]
    
	fconfigure $cmdFP -blocking 0 -buffering none -translation {auto cr}
	fileevent $cmdFP readable "copyToText $t.text"
    bind $t.text <Key> {handleKeyPress %W %K; break}

    # reset path
    cd $baseDir 
}

proc handleKeyPress {textW char} {
    global cmdFP
    # puts "handleKeyPress: got $char"
    if {"$char" == "Return"} {
        set char \n
    }
    $textW insert end $char userInput
    puts -nonewline $cmdFP "$char"   ;# send to command's stdin
}

# Copies output associated with fp to the given text widget
proc copyToText {textW} {
    global cmdFP
    $textW insert end [read $cmdFP]
    if {[eof $cmdFP]} {
        catch {$textW insert end "\n<<EOF>>\n" end}
        close $cmdFP
    }
}


