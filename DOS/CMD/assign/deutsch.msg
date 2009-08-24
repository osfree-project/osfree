$Id: DEUTSCH.MSG 1.2 1997/01/27 08:09:37 ska Rel ska $
$Locker: ska $ $Name:  $	$State: Rel $

    Meldungsdefinitionsdatei lokaler Meldungen in Deutsch
    Copyright (C) 1995  Steffen Kaiser

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

>>>>>>I_locMsgFile
$Id: DEUTSCH.MSG 1.2 1997/01/27 08:09:37 ska Rel ska $
>>>>>>E_hlpScreen 127
%-%normal%ASSIGN: Ersetzen eines Laufwerksbuchstaben durch einen anderen.

Aufruf: %s [/%-%hell%SHADOW%-%normal%] /%-%hell%STATUS%-%normal%
or: 	[/%-%hell%SHADOW%-%normal%] [/{%-%hell%mu%-%normal%}] [{%-%hell%lw1%-%normal%[:]=%-%hell%lw2[:]%-%normal%}]

Options:
	%-%hell%SHADOW%-%normal%: Verhindert einen positiven Installationcheck.
	%-%hell%STATUS%-%normal%: Gibt alle ersetzten Laufwerksbuchstaben aus.
		Falls ASSIGN noch nicht geladen ist, wird eine Meldung angezeigt,
		ASSIGN jedoch nicht geladen.
	%-%hell%M%-%normal%: Verschiebt den residenten Teil in den UMB, falls m”glich.
	%-%hell%U%-%normal%: Entfernt ASSIGN wieder.

%-%hell%lw1%-%normal%: Zu ersetzenden Laufwerksbuchstaben.
%-%hell%lw2%-%normal%: Neuer L‘ufwerksbuchstabe.

Die Doppelbunkte ':' sind optional, zwischen den Laufwerksbuchstaben,
den Doppelpunkten und dem Gleichheitszeichen '=' drfen sich keine
Leerzeichen befinden.

>>>>>>E_assignData 20
Datensegment nicht lokalisierbar
>>>>>>W_mayhapInstalled
ASSIGN ist m”glicherweise nicht geladen
>>>>>>W_notInstalled
ASSIGN wurde noch nicht installiert
>>>>>>M_assignment
%c: => %c:

>>>>>>E_mcbChain 40
MCB Kette zerst”rt
>>>>>>E_loadedModule 41
Das Modul %s wurde nicht mit dieser Version geladen
>>>>>>E_notOnTop 42
Einige Interruptvektoren wurden von anderen
Programmen ge„ndert. Sie mssen erst jene entfernen
>>>>>>E_releaseBlock 42
Der Speicherbereich konnte nicht freigegeben werden
>>>>>>M_installByte
Das Installationcheckbyte enth„lt 0%02x
>>>>>>M_inRecurs
Das Rekursionscheckbyte enth„lt 0%02x
>>>>>>E_recursion 43
Das Modul befindet sich in einer Rekursionschleife.
Entfernung nicht m”glich
>>>>>>E_lowDOSVers 60
DOS Version zu klein. DOS 3 erforderlich
>>>>>>W_highDOSVers
DOS Version hoch. Fehlfunktionen m”glich
