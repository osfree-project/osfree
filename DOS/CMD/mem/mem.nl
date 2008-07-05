# fatal errors
0.0:Geheugen is op. %ld meer bytes nodig.\n
0.1:SYSTEEMGEHEUGEN TRASHED! (int 21.5803 failure)\n
0.2:UMB Corruption: Ketting haalt de top van lage RAM bij %dk niet. Laatste=0x%x.\n
0.3:De MCB ketting is beschadigd (geen Z MCB na laatste M MCB, maar %c op seg 0x%x).\n
0.4:onbekende optie: %s\nUse /? voor hulp\n
# misc messages
1.0:Een onbekend besturingssysteem
1.1:%lu bytes\n
1.2:(%s bytes)\n
1.3: (%7s bytes)\n
# memory types
2.0:Geheugentype       Totaal    Gebruikt     Vrij\n
#   ----------------  --------   --------   --------
2.1:Conventioneel
2.2:Boven
2.3:Gereserveerd
2.4:Uitgebreid (XMS)
2.5:Totaal geheugen
2.6:Totaal onder 1 MB
2.7:Totaal Vergroot (EMS)
2.8:Vrij Vergroot (EMS)
2.9:Grootste uitvoerbare programmagrootte
2.10:Grootste vrije bovengeheugenblok
2.11:%s is resident in het hoge geheugengebied (HMA).\n
# block types
3.0:
3.1:vrij
3.2:systeemcode
3.3:systeemgegevens
3.4:programma
3.5:omgeving
3.6:gegevensgebied
3.7:gereserveerd
3.8:stuurprogramma
3.9:IFS
# classify msgs
4.0:\nModules die geheugen onder 1 MB gebruiken:\n\n
4.1:  Naam           Totaal          Conventioneel      Bovengeheugen\n
#     --------  ----------------   ----------------   ----------------
4.2:SYSTEEM
4.3:Vrij
4.4:\nSegment  Grootte     Naam         Type\n
#     ------- --------  ----------  -------------
4.5:\n    Adres      Attr    Naam      Programma\n
#      -----------  ------ ----------  ----------
# EMS stuff
5.0:EMS INTERNE FOUT.\n
5.1:  EMS stuurprogramma niet geinstalleerd in het systeem.\n
5.2:\nEMS stuurprogramma versie
5.3:EMS pagina lijst
5.4:Totaal EMS-geheugen
5.5:Vrij EMS-geheugen
5.6:Totale hendels
5.7:Vrije hendels
5.8:\n  Hendel  Pagina's Grootte      Naam\n
#      -------- ------  --------   ----------
# XMS stuff
6.0:XMS stuurprogramma niet geinstalleerd in het systeem.\n
6.1:\nTesten van het XMS geheugen ...\n
6.2:XMS INTERNE FOUT.\n
6.3:INT 2F AX=4309 ondersteund\n
6.4:XMS versie
6.5:XMS stuurprogramma versie
6.6:HMA status
6.7:bestaat
6.8:bestaat niet
6.9:A20 lijn status
6.10:aangeschakeld
6.11:uitgeschakeld
6.12:Vrij XMS-geheugen
6.13:Grootste vrije XMS blok
6.14:Vrije hendels
6.15: Blok    Hendel    Grootte   Sloten\n
#    ------- --------  --------  -------
6.16:Vrij bovengeheugen
6.17:Grootste bovenblok
6.18:Bovengeheugen niet beschikbaar\n
# help message
7.0:FreeDOS MEM versie %d.%d\n
7.1:Laat de hoeveelheid gebruikt en vrij geheugen in uw systeem zien.
7.2:Syntaxis: MEM [/E] [/F] [/C] [/D] [/U] [/X] [/P] [/?]
7.3:/E  Geeft alle informatie over Vergroot Geheugen (EMS)
7.4:/F  Volledige lijst van geheugenblokken
7.5:/C  Classificeer modules die geheugen onder 1 MB gebruiken
7.6:/D  Lijst van stuurprogramma's die nu in het geheugen zijn
7.7:/U  Lijst van programma's in conventioneel- en bovengeheugen.
7.8:/X  Geeft alle informatie over Uitgebreid Geheugen (XMS)
7.9:/P  Pauzeert na ieder scherm vol met informatie
7.10:/?  Laat deze hulpmelding zien
# paging message
8.0:\nDruk op <Enter> om door te gaan of op <Esc> om weg te gaan . . .