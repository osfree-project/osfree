#
# Regina felmeddelanden - Svenska
# Written by Mark Hessling <M.Hessling@qut.edu.au>
#
# dutch - Gert van der Kooij <geko@wanadoo.nl>
# french - Mark <cotemark@globetrotter.net>
# german - <florian@grosse-coosmann.de>
# norwegian (bokmal) - Vidar Tysse <vtysse@broadpark.no>
# portuguese - brian <bcarpent@nucleus.com>
# russian - Oleg Kulikov <kulikov@xanadu.ru>
# spanish - Pablo Garcia-Abia <Pablo.Garcia@cern.ch>
# svenska - Jan-Erik Lärka <janerik.larka@gmail.com>
#
# Riktlinjer för översättare
# --------------------------
# - Rader som börjar med # är kommentarer
# - Ord skrivna med VERSALER skall inte översättas
# - På endel meddelanderader med fel finns märken åtföljda av tecknet |.
#   Dessa skall inte heller översättas.
# - Säkerställ att ordningen på ersättningar, d.v.s. %s, %d
#   bibehålls i översättningen. Meddela mig om det är grammatiskt omöjligt,
# - Det finns några kommentarer i slutet av denna fil under rubriken:
#   "Extra generella fraser som behöver översättas:"
#   Översätt även dessa fraser.
# - Skicka de översatta meddelandena i en och samma fil, den här filen,
#   med namnet XX.mts där XX är motsvarigheten till språkkoden enl. ISO.
#   Viktigt: Paketera filen för att säkerställa att filen når mottagaren i dess
#            ursprungliga form
#   Lägg till en kommentar upptill av filen med språk och ditt namn samt e-postadress.
#   Ange om du vill att din e-postadress skall visas i dokumenatationen för Regina.
#   Jag behåller din e-postadress för att kunna kontakta dig i framtiden om behov uppstår
#   av att uppdatera och lägga till enskilda meddelanden, men publicerar endast ditt namn.
# - Till sist men på inget sätt minst; Tack!
#
  0,  1,Fel %s kör %s, rad %d:|<value>,<source>,<linenumber>
  0,  2,Fel %s vid interaktiv sökning:|<value>
  0,  3,Interaktiv sökning. Avsluta avlusning med "Trace Off". ENTER för att fortsätta.
  2,  0,Problem uppstod vid avslutning
  2,  1,Problem uppstod vid avslutning: %s|<description>
  3,  0,Problem uppstod vid initiering
  3,  1,Problem uppstod vid initiering: %s|<description>
  4,  0,Program avbröts
  4,  1,Program avbröts med HALT: %s|<description>
  5,  0,Systemresurser uttömda
  5,  1,Systemresurser uttömda: %s|<description>
  6,  0,Ingen träff på "/*" eller kommentar
  6,  1,Ingen träff på avslutande kommentar till ("/*")
  6,  2,Ingen träff på enkelt kommentarstecken (')
  6,  3,Ingen träff på dubbelt kommentarstecken (")
  7,  0,Förväntade WHEN eller OTHERWISE
  7,  1,SELECT på rad %d skall följas av WHEN; hittade "%s"|<linenumber>,<token>
  7,  2,SELECT på rad %d skall följas av WHEN, OTHERWISE eller END; hittade "%s"|<linenumber>,<token>
  7,  3,Alla WHEN-uttryck till SELECT på rad %d ger resulatatet falskt; Förväntade OTHERWISE på rad|<linenumber>
  8,  0,Oväntat THEN eller ELSE
  8,  1,THEN saknar tillhörande villkor IF eller WHEN
  8,  2,ELSE saknar tillhörande villkor THEN
  9,  0,Oväntat WHEN eller OTHERWISE
  9,  1,WHEN saknar tillhörande SELECT
  9,  2,OTHERWISE saknar tillhörande SELECT
 10,  0,Oväntat END eller ingen kod som hör ihop med END
 10,  1,END saknar tillhörande DO eller SELECT
 10,  2,END som hör till DO på rad %d måste ha en symbol som följs av en motsvarande kontrollvariabel (eller ingen symbol); hittade "%s"|<linenumber>,<token>
 10,  3,END som hör till DO på rad %d får inte följas av en symbol eftersom det inte finns någon kontorllvariabel; Hittade "%s"|<linenumber>,<token>
 10,  4,END som hör till SELECT på rad %d får inte följas av en symbol; hittade "%s"|<linenumber>,<token>
 10,  5,END får inte följa direkt på THEN
 10,  6,END får inte följa direkt på ELSE
 11,  0,[Kontrollstacken är full]
 12,  0,[Frasen är > 1024 tecken]
 13,  0,Felaktigt tecken i programmet
 13,  1,Felaktigt tecken i programmet "('%x'X)"|<hex-kodning>
 14,  0,Ofullständig DO/SELECT/IF
 14,  1,DO-instruktionen måste åtföljas av ett matchande END
 14,  2,SELECT-instruktionen måste åtföljas av ett matchande END
 14,  3,THEN måste följas av en instruktion
 14,  4,ELSE måste följas av en instruktion
 15,  0,Ogiltig hexadecimal eller binär sträng
 15,  1,Ogiltig placering av blanktecken på plats %d i hexadecimal sträng|<position>
 15,  2,Ogilitg placering av blanktecken på plats %d in binär sträng|<position>
 15,  3,Endast 0-9, a-f, A-F och blanktecken får användas i en hexadecimal sträng; hittade "%c"|<char>
 15,  4,Endast 0, 1 och blanktecken får användas i binära strängar; hittade "%c"|<char>
 16,  0,Kunde inte hitta
 16,  1,"%s" kunde inte hittas|<name>
 16,  2,Kan inte SIGNALera till "%s" därför att den finns inuti en grupp av IF, SELECT eller DO|<name>
 16,  3,Kan inte anrpoa "%s" därför att den finns inuti en grupp av IF, SELECT eller DO|<name>
 17,  0,Oväntad PROCEDURE
 17,  1,PROCEDURE får endast användas när den utgör första instruktionen som körs efter ett internt anrop med CALL eller vid funktionsanrop
 18,  0,Förväntade THEN
 18,  1,Nyckelordet IF på rad %d måste följas av ett matchande THEN; hittade "%s"|<linenumber>,<token>
 18,  2,Nyckelordet WHEN på rad %d måste följas av ett matchande THEN; hittade "%s"|<linenumber>,<token>
 19,  0,Förväntade sträng eller symbol
 19,  1,Förväntade sträng eller symbol efter nyckelordet ADDRES; hittade "%s"|<token>
 19,  2,Förväntade sträng eller symbol efter nyckelordet CALL; hittade "%s"|<token>
 19,  3,Förväntade sträng eller symbol efter nyckelordet NAME; hittade "%s"|<token>
 19,  4,Förväntade sträng eller symbol efter nyckelordet SIGNAL; hittade "%s"|<token>
 19,  6,Förväntade sträng eller symbol efter nyckelordet TRACE; hittade "%s"|<token>
 19,  7,Förväntade symbol när mönster tolkades; hittade "%s"|<token>
 20,  0,Förväntade ett namn
 20,  1,Namn krävs; hittade "%s"|<token>
 20,  2,Hittade "%s" där endast ett namn är giltigt|<token>
 21,  0,Felaktigt data i slutet av frasen
 21,  1,Frasen slutade vid ett oväntat märke; hittade "%s"|<token>
 22,  0,Ogiltig sträng av tecken
 22,  1,Ogiltig sträng av tecken '%s'X|<hex-kodning>
 23,  0,Ogiltig sträng av data
 23,  1,Ogiltig sträng av data '%s'X|<hex-kodning>
 24,  0,Ogiltigt anrop till TRACE
 24,  1,Tecknet till TRACE måste vara ett av följande "%s"; hittade "%c"|ACEFILNOR,<value>
 25,  0,Ogiltigt nyckelord hittades
 25,  1,CALL ON måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25,  2,CALL OFF måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25,  3,SIGNAL ON måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25,  4,SIGNAL OFF måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25,  5,ADDRESS WITH måste följas av något av nyckelorden INPUT, OUTPUT eller ERROR; hittade "%s"|<token>
 25,  6,INPUT måste följas av något av nyckelorden STREAM, STEM, LIFO, FIFO eller NORMAL; hittade "%s"|<token>
 25,  7,OUTPUT must be followed by one of the keywords STREAM, STEM, LIFO, FIFO, APPEND, REPLACE or NORMAL; hittade "%s"|<token>
 25,  8,APPEND måste följas av något av nyckelorden STREAM, STEM, LIFO or FIFO; hittade "%s"|<token>
 25,  9,REPLACE måste följas av något av nyckelorden STREAM, STEM, LIFO or FIFO; hittade "%s"|<token>
 25, 11,NUMERIC FORM måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25, 12,PARSE måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25, 13,UPPER måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25, 14,ERROR måste följas av något av nyckelorden STREAM, STEM, LIFO, FIFO, APPEND, REPLACE or NORMAL; hittade "%s"|<token>
 25, 15,NUMERIC måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25, 16,FOREVER måste följas av något av nyckelorden %s; hittade "%s"|<keywords>,<token>
 25, 17,PROCEDURE måste följas av nyckelordet EXPOSE eller ingenting; hittade "%s"|<token>
 26,  0,Felaktigt heltal
 26,  1,Storleken på heltal måste vara inom området för nuvarande inställning DIGITS (%d); hittade "%s"|<value>,<value>
 26,  2,Värdet för upprepning av uttrycket i instruktionen DO måste vara noll eller ett positivt heltal; hittade "%s"|<value>
 26,  3,Värdet av uttrycket FOR i instruktionen DO måste vara noll eller ett positivt heltal; hittade "%s"|<value>
 26,  4,Parametern för tolkning enl. mallen måste vara ett heltal; hittade "%s"|<value>
 26,  5,Värdet på NUMERIC DIGITS måste vara ett positivt heltal; hittade "%s"|<value>
 26,  6,Värdet på NUMERIC FUZZ måste vara noll eller ett positivt heltal; hittade "%s"|<value>
 26,  7,Numret som används i inställningen för TRACE måste vara ett heltal; hittade "%s"|<value>
 26,  8,Operand till höger om upphöjt i ("**") måste vara ett heltal; hittade "%s"|<value>
 26, 11,Resultet av %s %% %s skulle kräva exponentiell notation för nuvarande inställning av NUMERIC DIGITS %d|<value>,<value>,<value>
 26, 12,Resulter av beräkningen %% som används för %s // %s skulle kräva exponentiell notation för nuvarande inställning av NUMERIC DIGITS %d|<value>,<value>,<value>
 27,  0,Ogiltig syntax för DO
 27,  1,Ogiltig användning av nyckelordet "%s" i villkoret DO|<token>
 28,  0,Ogiltig LEAVE eller ITERATE
 28,  1,LEAVE är bara giltigt inom en upprepad DO-sats
 28,  2,ITERATE är bara giltigt inom en upprepad DO-sats
 28,  3,Symbolen som kommer efter LEAVE ("%s") måste antingen matcha kontrollvariabeln för nuvarande upprepad DO-stats eller utelämnas|<token>
 28,  4,Symbolen som kommer efter ITERATE ("%s") måste antingen matcha kontrollvariabeln för nuvarande upprepad DO-stats eller utelämnas|<token>
 29,  0,Miljövariablens namn är för långt
 29,  1,Moljövariablens namn överstiger %d tecken; hittade "%s"|#Limit_EnvironmentName,<name>
 30,  0,Namnet eller strängen är för lång
 30,  1,Namnet överstiger %d tecken|#Limit_Name
 30,  2,Ordagrann sträng överstiger %d tecken|#Limit_Literal
 31,  0,Namnet startar med ett nummer eller "."
 31,  1,Kan inte tilldela ett värde till ett nummer; hittade "%s"|<token>
 31,  2,Variabel får inte starta med ett nummer; hittade "%s"|<token>
 31,  3,Variabel får inte starta med "."; hittade "%s"|<token>
 32,  0,[Felaktig användning av ordstam (stem)]
 33,  0,Ogiltigt resultat av uttryck
 33,  1,Värdet av NUMERIC DIGITS ("%d") måste överstiga värdet av NUMERIC FUZZ ("%d")|<value>,<value>
 33,  2,Värdet av NUMERIC DIGITS ("%d") får inte överstiga %d|<value>,#Limit_Digits
 33,  3,Resultet av uttrycket efter NUMERIC FORM måste börja med "E" eller "S"; hittade "%s"|<value>
 34,  0,Logiskt värde är inte "0" eller "1"
 34,  1,Värdet av uttryck efter nyckelordet IF måste vara exakt "0" eller "1"; hittade "%s"|<value>
 34,  2,Värdet av uttryck efter nyckelordet WHEN måste vara exakt "0" eller "1"; hittade "%s"|<value>
 34,  3,Värdet av uttryck efter nyckelordet WHILE måste vara exakt "0" eller "1"; hittade "%s"|<value>
 34,  4,Värdet av uttryck efter nyckelordet UNTIL måste vara exakt "0" eller "1"; hittade "%s"|<value>
 34,  5,Värdet av uttrycket till vänster om den logiska jämförelsen "%s" måste vara exakt "0" eller "1"; hittade "%s"|<operator>,<value>
 34,  6,Värdet av uttrycket till höger om  den logiska jämförelsen "%s" måste vara exakt "0" eller "1"; hittade "%s"|<operator>,<value>
 35,  0,Ogiltigt uttryck
 35,  1,Ogiltigt uttryck hittades vid "%s"|<token>
 36,  0,Ingen matchning av "(" i uttrycket
 37,  0,Oväntat "," eller ")"
 37,  1,Oväntat ","
 37,  2,Ingen matchning av ")" i uttrycket
 38,  0,Ogiltig mall eller mönster
 38,  1,Ogiltig mall vid tolkning hittades vid "%s"|<token>
 38,  2,Ogiltig position för tolkning vid "%s"|<token>
 38,  3,Instruktionen PARSE VALUE måste följas av nyckelordet WITH
 39,  0,[Stacken är överfull]
 40,  0,Ogiltigt anrop till rutin
 40,  1,Problem uppstod i extern rutin "%s"|<name>
 40,  3,Inte tillräckligt många argument i anrop till "%s"; minsta antal som förväntas är %d|<bif>,<argnumber>
 40,  4,För många argument i anrop till "%s"; maximalt antal som förväntas är %d|<bif>,<argnumber>
 40,  5,Saknar argument i anrop till "%s"; %d förväntas|<bif>,<argnumber>
 40,  9,%s argument %d exponent överstiger %d siffror; hittade "%s"|<bif>,<argnumber>,#Limit_ExponentDigits,<value>
 40, 11,%s argument %d måste vara ett nummer; hittade "%s"|<bif>,<argnumber>,<value>
 40, 12,%s argument %d måste vara ett heltal; hittade "%s"|<bif>,<argnumber>,<value>
 40, 13,%s argument %d måste vara noll eller positivt; hittade "%s"|<bif>,<argnumber>,<value>
 40, 14,%s argument %d måste vara positivt; hittade "%s"|<bif>,<argnumber>,<value>
 40, 17,%s argument 1, måste ha en heltalsdel i området 0:90 och en decimaldel som inte är större än .9; hittade "%s"|<bif>,<value>
 40, 18,%s omräknat måste vara år i området 0001 till 9999|<bif>
 40, 19,%s argument 2, "%s", har inte formatet som beskrivs av argument 3, "%s"|<bif>,<value>,<value>
 40, 21,%s argument %d får inte vara tomt (null)|<bif>,<argnumber>
 40, 23,%s argument %d måste vara ett enkeltecken; hittade "%s"|<bif>,<argnumber>,<value>
 40, 24,%s argument 1 måste vara en binär sträng; hittade "%s"|<bif>,<value>
 40, 25,%s argument 1 måste vara en hexadecimal sträng; hittade "%s"|<bif>,<value>
 40, 26,%s argument 1 måste vara en giltig symbol; hittade "%s"|<bif>,<value>
 40, 27,%s argument 1, måste vara ett giltigt namn på en filström; hittade "%s"|<bif>,<value>
 40, 28,%s argument %d, valet måste starta med något av alternativen "%s"; hittade "%s"|<bif>,<argnumber>,<optionslist>,<value>
 40, 29,%s konverterat till formatet "%s" är inte tillåtet|<bif>,<value>
 40, 31,%s argument 1 ("%d") får inte överstiga 100000|<bif>,<value>
 40, 32,%s skillnaden mellan argument 1 ("%d") och argument 2 ("%d") får inte överstiga 100000|<bif>,<value>,<value>
 40, 33,%s argument 1 ("%d") måste vara mindre eller lika med argument 2 ("%d")|<bif>,<value>,<value>
 40, 34,%s argument 1 ("%d") måste vara mindre eller lika med antalet rader i programmet (%d)|<bif>,<value>,<sourceline()>
 40, 35,%s argument 1 kan inte uttryckas som ett heltal; hittade "%s"|<bif>,<value>
 40, 36,%s argument 1 måste vara ett namn på en använd variabel; hittade "%s"|<bif>,<value>
 40, 37,%s argument 3 måste vara ett benämning på variablerna; hittade "%s"|<bif>,<value>
 40, 38,%s argument %d är inte tillräckligt stor för formatet "%s"|<bif>,<argnumber>,<value>
 40, 39,%s argument 3 är inte noll eller ett; hittade "%s"|<bif>,<value>
 40, 41,%s argument %d måste ligga inom begränsningarna för strömmen; hittade "%s"|<bif>,<argnumber>,<value>
 40, 42,%s argument 1; kan inte placera i denna filström; hittade "%s"|<bif>,<value>
 40, 43,%s argument %d måste vara en icke-alfanumeriskt tecken eller tom sträng; hittade "%s"|<bif>,<argnumber>,<value>
 40, 44,%s argument %d, "%s", är på format som inte fungerar med den angivna separatorn som angivits som argument %d|<bif>,<argnumber>,<value>,<argnumber>
 40,914,[%s argument %d, måste vara en av "%s"; hittade "%s"]|<bif>,<argnumber>,<optionslist>,<value>
 40,920,[%s: Låg-nivå I/O-fel på filström; %s]|<bif>,<description>
 40,921,[%s argument %d, pekar på placering "%s" i filström; inkompatibel med filström som är öppen]|<bif>,<argnumber>,<value>
 40,922,[%s argument %d, för få underkommandon; minimum expected is %d; hittade %d]|<bif>,<argnumber>,<value>,<value>
 40,923,[%s argument %d, för många underkommandon; maximalt förväntat antal är %d; hittade %d]|<bif>,<argnumber>,<value>,<value>
 40,924,[%s argument %d, ogiltig placering angiven; förväntade ett av "%s"; hittade "%s"]|<bif>,<argnumber>,<value>,<value>
 40,930,[RXQUEUE, funktion TIMEOUT, förväntade ett heltal mellan -1 och %d; hittade \"%s\"]|<value>,<value>
 40,980,Oväntad inmatning, antingen av okänd typ eller så innehåller den ogiltigt data%s%s|: ,<location>
 40,981,Talet är utanför tillåtet område%s%s|: ,<location>
 40,982,Strängen är för stor för utrymme som gjorts iordning%s%s|: ,<location>
 40,983,Ogiltig kombination av typ/storlek%s%s|: ,<location>
 40,984,Saknar stöd för nummer av typen NAN, +INF, -INF%s%s|: ,<location>
 40,985,Strukturen är för komplex för statiskt utrymme internt%s%s|: ,<location>
 40,986,Ett element saknas i strukturen%s%s|: ,<location>
 40,987,Ett värde saknas i strukturen%s%s|: ,<location>
 40,988,Namnet eller del av namn är ogiltigt för tolken%s%s|: ,<location>
 40,989,Ett problem uppstod i kopplingen mellan Regina och GCI%s%s|: ,<location>
 40,990,Typen uppfyller inte kraven på grundtyper (argument/returnerar värde)%s%s|: ,<location>
 40,991,Antalet arguments är fel eller saknas%s%s|: ,<location>
 40,992,den interna stacken för GCI är överfull%s%s|: ,<location>
 40,993,GCI räknade för många nästlade LIKE-containers%s%s|: ,<location>
 41,  0,Fel vid aritmetisk översättning
 41,  1,Ickenumeriskt värde ("%s") till vänster om aritmetisk operation "%s"|<value>,<operator>
 41,  2,Ickenumeriskt värde ("%s") till höger om aritmetisk operation "%s"|<value>,<operator>
 41,  3,Ickenumeriskt värde ("%s") används med prefix-operator "%s"|<value>,<operator>
 41,  4,Värdet av TO-uttrycket i en DO-stats måste vara numeriskt; hittade "%s"|<value>
 41,  5,Värdet av BY-uttrycket i en DO-stats måste vara numersikt; hittade "%s"|<value>
 41,  6,Värdet av uttrycket för kontrollvariabeln för DO-statsen måste vara numeriskt; hittade "%s"|<value>
 41,  7,Exponenten överstiger %d siffror; hittade "%s"|#Limit_ExponentDigits,<value>
 42,  0,För omfattande/komplicerad aritmetisk beräkning
 42,  1,För omfattande aritmetisk beräkning upptäcktes vid "%s %s %s"; exponenten för resultatet kräver mer än %d siffror|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  2,Aritmetisk komplicerad beräkning upptäcktes vid "%s %s %s"; exponenten för resultatet kräver mer än %d siffror|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  3,Aritmetisk beräkning inte möjlig; division får inte göras med noll
 43,  0,Rutinen hittades inte
 43,  1,Kunde inte hitta rutinen "%s"|<name>
 44,  0,Funktionen returnerade inget data
 44,  1,Inget data returnerades från funktionen "%s"|<name>
 45,  0,Inget data har angetts i funktionens RETURN-stats
 45,  1,Returvärde förväntades i instruktionen för RETURN eftersom rutinen "%s" anropades som en funktion|<name>
 46,  0,Ogiltig variabelreferens
 46,  1,Extra märke ("%s") hittades i variabelreferens; ")" förväntades|<token>
 47,  0,Oväntad benämning
 47,  1,INTERPRET för data får inte innehålla benämningar; hittade "%s"|<name>
 48,  0,Systemtjänst har slutat fungera
 48,  1,Fel i systemtjänst: %s|<description>
 48,920, Lågnivå I/O-fel på filström: %s %s: %s|<description>,<stream>,<description>
 49,  0,Tolkningsfel
 49,  1,Tolkningsfel: Fel i %s, rad %d: "%s". Var god rapportera felet!|<module>,<linenumber>,<description>
 50,  0,Okänd reserverad symbol
 50,  1,Okänd reserverad symbol "%s"|<token>
 51,  0,Ogiltigt funktionsnamn
 51,  1,Okommenterat funktionsnamn får inte sluta med punkt; hittade "%s"|<token>
 52,  0,Resultet som returnerades "%s" är längre än %d tecken|<name>,#Limit_String
 53,  0,Felaktigt val
 53,  1,Sträng eller symbol förväntades efter nyckelordet STREAM; hittade "%s"|<token>
 53,  2,Variabelreferens föräntades efter nyckelordet STEM; hittade "%s"|<token>
 53,  3,Argument till STEM måste ha en punkt som sista tecken; hittade "%s"|<name>
 53,100,Sträng eller symbol förväntades efter nyckelordet LIFO; hittade "%s"|<token>
 53,101,Sträng eller symbol förväntades efter nyckelordet FIFO; hittade "%s"|<token>
 54,  0,Ogiltigt värde på STEM
 54,  1,För detta STEM APPEND måste värdet av "%s" utgöra antalet rader; hittade "%s"|<name>,<value>
#
# Alla felmeddelanden nedanför är inte definierade av ANSI
#
 60,  0,[Kan inte återgå till början av den tillfälliga filen]
 61,  0,[Felaktig metod för sökning i fil]
 64,  0,[Syntaktiskt fel vid inläsning]
 64,  1,[Syntaktiskt fel på rad %d]
 64,  2,[Generellt syntaktiskt fel på rad %d, kolumn %d]|<linenumber>,<columnnumber>
 90,  0,[Icke-ANSI-karaktäristisk användning med "OPTIONS STRICT_ANSI"]
 90,  1,[%s är en utökning enl. Regina BIF]|<bif>
 90,  2,[%s är en utökad instruktion enl. Regina]|<token>
 90,  3,[%s argument %d, valet måste börja med ett av "%s" när "OPTIONS STRICT_ANSI" används; hittade "%s"; a Regina extension]|<bif>,<argnumber>,<optionslist>,<value>
 90,  4,[%s är en utökad jämförelsen enl. Regina]|<token>
 93,  0,[Ogilitgt anrop till rutin]
 93,  1,[Kommandot STREAM %s måste följas av ett av "%s"; hittade "%s"]|<token>,<value>,<value>
 93,  3,[Kommandot STREAM måste vara ett av "%s"; hittade "%s"]|<value>,<value>
 94,  0,[Fel i extern kö vid överföring]
 94,  1,[External queue timed out]
 94, 99,[Internt fel i extern kö vid överföring: %d "%s"]|<description>,<systemerror>
 94,100,[Generellt systemfel i extern kö vid överföring. %s. %s]|<description>,<systemerror>
 94,101,[Fel vid uppkoppling mot %s på port %d: "%s"]|<machine>,<portnumber>,<systemerror>
 94,102,[Kan inte utläsa IP-adress för %s]|<machine>
 94,103,[Ogiltigt format på servernamn har angivits för kö: "%s"]|<queuename>
 94,104,[Ogiltigt format på namn för kö: "%s"]|<queuename>
 94,105,[Kan inte starta överföring med Windows Socket: %s]|<systemerror>
 94,106,[Maximalt antal externa köer har överskridits: %d]|<maxqueues>
 94,107,[Fel uppstod vid läsning av socket: %s]|<systemerror>
 94,108,[Ogiltig växel angiven. Måste vara en av "%s"]|<switch>
 94,109,[Kunde inte hitta kön \"%s\"]|<queuename>
 94,110,[%s är ogiltig för externa köer]|<bif>
 94,111,[Funktionen %s för RXQUEUE kan inte användas med interna köer]|<functionname>
 94,112,[Unable to %s SESSION queue]|<action>
 95,  0,[Begränsad användning i "säkerhetsläge"]
 95,  1,[%s ogiltig i "säkerhetsläge"]|<token>
 95,  2,[%s argument %d ogiltigt i "säkerhetsläge"]|<bif>,<argnumber>
 95,  3,[%s argument %d: "%s", ogiltigt i "säkerhetsläge"]|<bif>,<argnumber>,<token>
 95,  4,[STREAM argument 3: Öppna fil för skrivåtkomst med WRITE är inte tillåtet i "säkehetsläge"]
 95,  5,[Det är inte tillåtet att köra externa kommandon i "säkerhetsläge"]
100,  0,[Okänt filsystemfel]
#
# Extra generella fraser som behöver översättas:
# Texten inom () motsvarar namnen i error.c
P,  0,Fel %d under körning av "%.*s" på rad %d: %.*s
P,  1,Fel %d.%d: %.*s
P,  2,Fel %d under körning av "%.*s": %.*s
#
# (err1prefix)   "Fel %d under körning av \"%.*s\" på rad %d: %.*s",
# (suberrprefix) "Fel %d.%d: %.*s",
# (err2prefix)   "Fel %d under körning av \"%.*s\": %.*s",
# (erropen)      "Kan inte öppna språkfilen: %s",
# (errcount)     "Felaktigt antal meddelanden i språkfilen: %s",
# (errread)      "Kan inte läsa från språkfilen: %s",
# (errmissing)   "Text saknas i språkfilen: %s.mtb",
# (errcorrupt)   "Språkfilen: %s.mtb är skadad",
