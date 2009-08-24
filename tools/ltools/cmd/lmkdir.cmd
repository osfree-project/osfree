@if NOT ""%8=="" goto EightArg
@if NOT ""%7=="" goto SevenArg
@if NOT ""%6=="" goto SixArg
@if NOT ""%5=="" goto FiveArg
@if NOT ""%4=="" goto FourArg
@if NOT ""%3=="" goto ThreeArg
@if NOT ""%2=="" goto TwoArg
:OneArg
@ldir -WRITE -mkdir LDIRTMPL %1
@goto ende
:TwoArg
@ldir -WRITE -mkdir  LDIRTMPL %1 %2
@goto ende
:ThreeArg
@ldir -WRITE -mkdir  LDIRTMPL %1 %2 %3
@goto ende
:FourArg
@ldir -WRITE -mkdir  LDIRTMPL %1 %2 %3 %4
@goto ende
:FiveArg
@ldir -WRITE -mkdir  LDIRTMPL %1 %2 %3 %4 %5
@goto ende
:SixArg
@ldir -WRITE -mkdir  LDIRTMPL %1 %2 %3 %4 %5 %6
@goto ende
:SevenArg
@ldir -WRITE -mkdir  LDIRTMPL %1 %2 %3 %4 %5 %6 %7
@goto ende
:EightArg
@ldir -WRITE -mkdir  LDIRTMPL %1 %2 %3 %4 %5 %6 %7 %8
@goto ende
:ende
