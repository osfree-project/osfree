.**********************************************************
.*
.* IPF Tagging Sample
.*
.**********************************************************
.*
:userdoc.
:docprof toc=12 ctrlarea=none.
:title.IPF Tagging Sample
.*
.**********************************************************
.*
:h1.Introduction
:p.
This file contains sample tagging to show the various tagging
techniques available in IPF.  It has intentionally been kept
very simple, with very little text in each panel, to make it
easier to focus on the tagging and the results of the tagging.
To learn more about IPF and tagging, see the online
"IPF Programming Guide and Reference."
.*
.**********************************************************
.*
.* DEFAULT WINDOW EXAMPLE
.*
.**********************************************************
.*
:h1.Default Window Example
:p.
Default Window Example.
:p.
Select the "My First Window" heading from the Contents window
to see the effect of the sample tagging.
.*
:h2 res=001.My First Window
:p.
Here is the text for the first window.
This is a
:link reftype=hd res=002.hypertext link:elink.
to the second window.
.*
:h2 res=002.My Second Window
:p.
Here is the text for the second window.
This is a
:link reftype=hd res=001.hypertext link:elink.
to the first window.
.*
.**********************************************************
.*
.* HEADING LEVELS EXAMPLE
.*
.**********************************************************
.*
:h1.Heading Levels Example
:p.
Heading Levels Example.
:p.
Select the "Second-Level Heading" heading from the Contents window
to see the effect of the sample tagging.
.*
:h2 res=026.Second-Level Heading
:p.
This window is defined by an :hp2.h2.:ehp2. tag.
.*
:h2 res=027.Another Second-Level Heading
:p.
This window is defined by an :hp2.h2.:ehp2. tag.
:p.
.*
:h3 res=028.Third-Level Heading
:p.
Because the docprof tag at the beginning of the file specifies
that only heading levels 1 and 2 may have contents window
entries (toc=12), the preceding "Third-Level Heading" title
of an :hp2.h3:ehp2. tag and this text following it become part
of the window defined by the preceding :hp2.h2:ehp2. tag.
.*
:h2 res=029 toc=123.Yet Another Second-Level Heading
:p.
The :hp2.h2.:ehp2. tag for this window contains
a :hp2.toc=123:ehp2. specification.
.*
:h3 res=030.Third-Level Heading
:p.
Because the toc=123 in the preceding :hp2.h2:ehp2. tag
overrides the toc=12 in the docprof tag, this :hp2.h3.:ehp2.
tag defines a new window and creates a contents entry.
.*
.**********************************************************
.*
.* ORIGIN AND SIZE WINDOW EXAMPLE
.*
.**********************************************************
.*
:h1.Origin and Size Window Example
:p.
Origin and Size Window Example.
:p.
Select the "Primary Window" heading from the Contents window
to see the effects of the sample tagging.
.*
:h2 res=003
    x=left y=bottom width=50% height=100%.
Primary Window
:p.
Here is the text for the primary window.  This is a
:link reftype=hd res=004.hypertext link:elink.
to the secondary window.
.*
:h2 res=004
    x=right y=bottom width=50% height=100%.
Secondary Window
:p.
Here is the text for the secondary window.  This is a
:link reftype=hd res=003.hypertext link:elink.
to the primary window.
.*
.**********************************************************
.*
.* WINDOW GROUP NUMBER EXAMPLE
.*
.**********************************************************
.*
:h1.Window Group Number Example
:p.
Window Group Number Example.
:p.
Select the "My First Window" heading from the Contents window
to see the effects of the sample tagging.
.*
:h2 res=005
    x=left y=bottom width=50% height=100%
    group=1.
My First Window
:p.
Here is the text for the first window.
This is a
:link reftype=hd res=006.hypertext link:elink.
to the second window.
:p.
This is a
:link reftype=hd res=007.hypertext link:elink.
to the third window.
.*
:h2 res=006
    x=right y=top width=50% height=100%
    group=2.
My Second Window
:p.
Here is the text for the second window.
This is a
:link reftype=hd res=005.hypertext link:elink.
to the first window.
:p.
This is a
:link reftype=hd res=007.hypertext link:elink.
to the third window.
.*
:h2 res=007
    x=right y=top width=50% height=100%
    group=2.
My Third Window
:p.
Here is the text for the third window.
This is a
:link reftype=hd res=005.hypertext link:elink.
to the first window.
:p.
This is a
:link reftype=hd res=006.hypertext link:elink.
to the second window.
.*
.**********************************************************
.*
.* AUTOMATIC WINDOWS EXAMPLE
.*
.**********************************************************
.*
:h1.Automatic Windows Example
:p.
Automatic Windows Example.
:p.
Select the "Example 1" heading from the Contents window
to see the effects of the sample tagging.
.*
.* AUTOMATIC WINDOWS EXAMPLE ONE
.*
:h2 res=008
    x=left y=top width=25% height=100%
    scroll=none group=1 clear.
Example 1
:p.
This is Window 1.
:p.
This window has three automatic links to
Windows 2, 3, and 4.
:link reftype=hd res=009
      vpx=25% vpy=top vpcx=25% vpcy=100%
      group=2 auto dependent.
:link reftype=hd res=010
      vpx=50% vpy=top vpcx=25% vpcy=100%
      group=3 auto dependent.
:link reftype=hd res=011
      vpx=75% vpy=top vpcx=25% vpcy=100%
      group=4 auto dependent.
.*
:h2 res=009
    x=25% y=top width=25% height=100%
    scroll=none hide.
Window 2
:p.
This is  Window 2.
.*
:h2 res=010
    x=50% y=top width=25% height=100%
    scroll=none hide.
Window 3
:p.
This is Window 3.
.*
:h2 res=011
    x=75% y=top width=25% height=100%
    scroll=none hide.
Window 4
:p.
This is Window 4.
.*
.* AUTOMATIC WINDOWS EXAMPLE TWO
.*
:h2 res=012
    x=left y=top width=25% height=100%
    scroll=none group=1 clear.
Example 2
:p.
This is Window 1.
:p.
This window has an automatic link to
Window 2.
:link reftype=hd res=013
      vpx=25% vpy=top vpcx=25% vpcy=100%
      group=2 auto dependent.
.*
:h1 res=013
    x=25% y=top width=25% height=100%
    scroll=none hide.
Window 2
:p.
This is Window 2.
:p.
This window has an automatic link to
Window 3.
:link reftype=hd res=014
      vpx=50% vpy=top vpcx=25% vpcy=100%
      group=3 auto dependent.
.*
:h1 res=014
    x=50% y=top width=25% height=100%
    scroll=none hide.
Window 3
:p.
This is Window 3.
:p.
This paragraph contains a
:link reftype=hd res=015
      vpx=75% vpy=top vpcx=25% vpcy=100%
      group=4 dependent.hypertext link:elink.
to Window 4.
.*
:h1 res=015
    x=75% y=top width=25% height=100%
    scroll=none hide.
Window 4
:p.
This is Window 4.
.*
.**********************************************************
.*
.* SPLIT WINDOWS EXAMPLE
.*
.**********************************************************
.*
:h1.Split Windows Example
:p.
Split Windows Example.
:p.
Select the "Primary Window A" heading from the Contents window
to see the effects of the sample tagging.
.*
.* Parent window "A" and two child windows
.*
:h2 res=016 scroll=none clear.
Primary Window A
.*
:link reftype=hd res=017 auto split group=10
      vpx=left vpy=top vpcx=50% vpcy=100%
      rules=border scroll=none titlebar=none.
:link reftype=hd res=018 auto split group=11
      vpx=right vpy=top vpcx=50% vpcy=100%
      rules=border scroll=none titlebar=none.
.*
:h2 res=017 hide nosearch noprint.Dummy
:p.
This secondary window contains hypertext links
to the adjacent secondary window.
:p.
Select one:
:sl compact.
:li.:link reftype=hd res=018 split group=11
          vpx=right vpy=top vpcx=50% vpcy=100%
          rules=border scroll=none titlebar=none.
Ducks:elink.
:li.:link reftype=hd res=019 split group=11
          vpx=right vpy=top vpcx=50% vpcy=100%
          rules=border scroll=none titlebar=none.
World:elink.
:esl.
.*
:h2 res=018 hide nosearch noprint.Dummy
:artwork name='ducks.bmp' fit.
.*
:h2 res=019 hide nosearch noprint.Dummy
:artwork name='world.bmp' fit.
.*
.* Parent window "B" and three child windows
.*
:h2 res=022 scroll=none titlebar=none rules=none clear.
Primary Window B
:link reftype=hd res=023 auto split group=10
      vpx=left vpy=top vpcx=40% vpcy=100%
      scroll=none titlebar=none rules=none.
:link reftype=hd res=024 auto split group=11
      vpx=right vpy=top vpcx=60% vpcy=20%
      scroll=none titlebar=none rules=none.
:link reftype=hd res=025 auto split group=12
      vpx=right vpy=bottom vpcx=60% vpcy=80%
      scroll=none titlebar=none rules=none.
.*
:h2 res=023 hide nosearch noprint.Dummy
:lm margin=5.
:rm margin=2.
:p.
:font facename='Tms Rmn' size=24x12.
:color bc=green.:hp2.TREES LOVE IPF:ehp2.
:color bc=cyan.:hp3.TREES LOVE IPF:ehp3.
:color bc=green.:hp4.TREES LOVE IPF:ehp4.
:color bc=cyan.:hp5.TREES LOVE IPF:ehp5.
:color bc=green.:hp6.TREES LOVE IPF:ehp6.
:color bc=cyan.:hp7.TREES LOVE IPF:ehp7.
:color bc=green.:hp8.TREES LOVE IPF:ehp8.
:color bc=cyan.:hp9.TREES LOVE IPF:ehp9.
:color bc=green.:hp2.TREES LOVE IPF:ehp2.
.*
:h2 res=024 hide nosearch noprint.Dummy
:p.&rbl.
.*
:h2 res=025 hide rules=none nosearch noprint.Dummy
:rm margin=3.
:font facename='Helv' size=18x9.
:p.
The Information Presentation Facility (IPF) is a set of tools
that supports the design and development of an online help
facility that is accessed by users of your application.
:p.
IPF also supports the design and development of online
information that may be viewed independently of an application.
These files are compiled with the /INF parameter of the IPF
compiler, and they are viewed by entering the name of the
compiled file as a parameter of the VIEW program.
.*
.**********************************************************
.*
.* HYPERGRAPHIC LINK EXAMPLE
.*
.**********************************************************
.*
:h1.Hypergraphic Link Example
:p.
Hypergraphic Link Example.
:p.
Select the "Hypergraphic Link" heading from the Contents window
to see the effects of the sample tagging.
.*
:h2.Hypergraphic Link
:p.
This is an example of a hypergraphic link.  Select the
"shuttle" bit map graphic and get ready for a
walk on the moon.
:artwork name='shuttle.bmp' align=center.
:artlink.
:link reftype=hd res=1153 dependent.
:eartlink.
.*
:h2 res=1153 clear hide.The Moonwalk Bit Map
:artwork name='moonwalk.bmp'.
:p.
To return to Earth, press the Esc key.
.*
.**********************************************************
.*
.* ACVIEWPORT TAG EXAMPLE
.*
.**********************************************************
.*
:h1.Application-Controlled Window Example
:p.
Application-Controlled Window Example.
:p.
Select the "IPF-Controlled Window" heading from the Contents window
to see the effects of the sample tagging.
.*
:h2 x=left y=top width=45% height=100%
    scroll=none titlebar=both clear group=1.
IPF-Controlled Window
:link reftype=hd res=1173
      vpx=45% vpy=bottom vpcx=55% vpcy=100%
      scroll=none auto dependent group=2.
:p.
Notice that as the text that follows describes the use of a
mouse, the panel on the right contains an &colon.acviewport
tag, which enables IPF.DLL to graphically demonstrate the
use of a mouse.
:p.
You move the pointer (usually an arrow) so that it points at
the objects and actions you want to select.
:p.
If you run out of room to slide the mouse, lift it up, put it
back down, and slide it again.
:p.
The left&hyphen.hand mouse button (or mouse button 1) is
usually used to select objects on the screen.
:p.
The right&hyphen.hand mouse button (or mouse button 2) is
usually used to :hp1.drag:ehp1. or move objects around the
screen.
.*
:h2 res=1173.Application-Controlled Window
.*
:acviewport dll='ipf' objectname='IPFMain' objectid=1
vpx=right vpy=top vpcx=55% vpcy=100%.
:euserdoc.
.*
.**********************************************************
