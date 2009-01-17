#
# Regina error messages - Norwegian (Bokmål)
# Written by Mark Hessling <M.Hessling@qut.edu.au>
#
# norwegian (bokmal) - Vidar Tysse <vtysse@broadpark.no>
#
  0,  1,Feil %s under kjøring av %s, linje %d:|<value>,<source>,<linenumber>
  0,  2,Feil %s i interaktiv sporing:|<value>
  0,  3,Interaktiv sporing.  "Trace Off" avslutter avlusning. ENTER fortsetter.
  2,  0,Svikt under sluttbehandling
  2,  1,Svikt under sluttbehandling: %s|<description>
  3,  0,Svikt under initialisering
  3,  1,Svikt under initialisering: %s|<description>
  4,  0,Program avbrutt
  4,  1,Program avbrutt med HALT-tilstand: %s|<description>
  5,  0,Systemressurser oppbrukt
  5,  1,Systemressurser oppbrukt: %s|<description>
  6,  0,Umatchet "/*" eller anførselstegn
  6,  1,Umatchet kommentar-avgrenser ("/*")
  6,  2,Umatchet enkelt anførselstegn (')
  6,  3,Umatchet dobbelt anførselstegn (")
  7,  0,Ventet WHEN eller OTHERWISE
  7,  1,SELECT på linje %d krever WHEN; fant "%s"|<linenumber>,<token>
  7,  2,SELECT på linje %d krever WHEN, OTHERWISE, eller END; fant "%s"|<linenumber>,<token>
  7,  3,Alle WHEN-uttrykk for SELECT på linje %d er usanne; ventet OTHERWISE|<linenumber>
  8,  0,Uventet THEN eller ELSE
  8,  1,THEN mangler tilhørende IF- eller WHEN-setning
  8,  2,ELSE mangler tilhørende THEN-setning
  9,  0,Uventet WHEN eller OTHERWISE
  9,  1,WHEN mangler tilhørende SELECT
  9,  2,OTHERWISE mangler tilhørende SELECT
 10,  0,Uventet eller umatchet END
 10,  1,END mangler tilhørende DO eller SELECT
 10,  2,END som tilhører DO på linje %d må ha et påfølgende symbol som matcher kontrollvariabelen (eller intet symbol); fant "%s"|<linenumber>,<token>
 10,  3,END som tilhører DO på linje %d skal ikke ha et påfølgende symbol, fordi der ikke er noen kontrollvariabel; fant "%s"|<linenumber>,<token>
 10,  4,END som tilhører SELECT på linje %d skal ikke ha et påfølgende symbol; fant "%s"|<linenumber>,<token>
 10,  5,END skal ikke følge umiddelbart etter THEN
 10,  6,END skal ikke følge umiddelbart etter ELSE
 11,  0,[Kontrollstakken er full]
 12,  0,[Setning > 1024 tegn]
 13,  0,Ugyldig tegn i program
 13,  1,Ugyldig tegn i program "('%x'X)"|<hex-encoding>
 14,  0,Ufullstendig DO/SELECT/IF
 14,  1,DO-instruksjon krever en matchende END
 14,  2,SELECT-instruksjon krever en matchende END
 14,  3,THEN krever en påfølgende instruksjon
 14,  4,ELSE krever en påfølgende instruksjon
 15,  0,Ugyldig heksadesimal eller binær streng
 15,  1,Ugyldig sted for blank i posisjon %d i heksadesimal streng|<position>
 15,  2,Ugyldig sted for blank i posisjon %d i binær streng|<position>
 15,  3,Kun 0-9, a-f, A-F og blank er gyldige i en heksadesimal streng; fant "%c"|<char>
 15,  4,Kun 0, 1 og blank er gyldige i en binær streng; fant "%c"|<char>
 16,  0,Fant ikke plassmerket
 16,  1,Fant ikke plassmerket "%s"|<name>
 16,  2,Kan ikke utføre SIGNAL til plassmerket "%s" fordi det er inne i en IF-, SELECT- eller DO-gruppe|<name>
 16,  3,Kan ikke kalle plassmerket "%s" fordi det er inne i en IF-, SELECT- eller DO-gruppe|<name>
 17,  0,Uventet PROCEDURE
 17,  1,PROCEDURE er gyldig kun når den utføres som første instruksjon etter et internt CALL eller funksjonskall
 18,  0,THEN var ventet
 18,  1,IF-nøkkelord på linje %d krever tilhørende THEN-setning; fant "%s"|<linenumber>,<token>
 18,  2,WHEN-nøkkelord på linje %d krever tilhørende THEN-setning; fant "%s"|<linenumber>,<token>
 19,  0,Ventet streng eller symbol
 19,  1,Ventet streng eller symbol etter ADDRESS-nøkkelord; fant "%s"|<token>
 19,  2,Ventet streng eller symbol etter CALL-nøkkelord; fant "%s"|<token>
 19,  3,Ventet streng eller symbol etter NAME-nøkkelord; fant "%s"|<token>
 19,  4,Ventet streng eller symbol etter SIGNAL-nøkkelord; fant "%s"|<token>
 19,  6,Ventet streng eller symbol etter TRACE-nøkkelord; fant "%s"|<token>
 19,  7,Ventet symbol i mønster for analyse; fant "%s"|<token>
 20,  0,Ventet navn
 20,  1,Krever navn; fant "%s"|<token>
 20,  2,Fant "%s" der kun et navn er gyldig|<token>
 21,  0,Ugyldige data på slutten av setning
 21,  1,Setningen sluttet ved et uventet uttrykk; fant "%s"|<token>
 22,  0,Ugyldig tegnstreng
 22,  1,Ugyldig tegnstreng '%s'X|<hex-encoding>
 23,  0,Ugyldig datastreng
 23,  1,Ugyldig datastreng '%s'X|<hex-encoding>
 24,  0,Ugyldig TRACE-anmodning
 24,  1,TRACE-anmodningsbokstav må være en av "%s"; fant "%c"|ACEFILNOR,<value>
 25,  0,Ugyldig undernøkkelord funnet
 25,  1,CALL ON må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25,  2,CALL OFF må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25,  3,SIGNAL ON må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25,  4,SIGNAL OFF må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25,  5,ADDRESS WITH må etterfølges av ett av nøkkelordene INPUT, OUTPUT eller ERROR; fant "%s"|<token>
 25,  6,INPUT må etterfølges av ett av nøkkelordene STREAM, STEM, LIFO, FIFO eller NORMAL; fant "%s"|<token>
 25,  7,OUTPUT må etterfølges av ett av nøkkelordene STREAM, STEM, LIFO, FIFO, APPEND, REPLACE eller NORMAL; fant "%s"|<token>
 25,  8,APPEND må etterfølges av ett av nøkkelordene STREAM, STEM, LIFO eller FIFO; fant "%s"|<token>
 25,  9,REPLACE må etterfølges av ett av nøkkelordene STREAM, STEM, LIFO eller FIFO; fant "%s"|<token>
 25, 11,NUMERIC FORM må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25, 12,PARSE må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25, 13,UPPER må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25, 14,ERROR må etterfølges av ett av nøkkelordene STREAM, STEM, LIFO, FIFO, APPEND, REPLACE eller NORMAL; fant "%s"|<token>
 25, 15,NUMERIC må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25, 16,FOREVER må etterfølges av ett av nøkkelordene %s; fant "%s"|<keywords>,<token>
 25, 17,PROCEDURE må etterfølges av ett av nøkkelordene EXPOSE; fant "%s"|<token>
 26,  0,Ugyldig heltall
 26,  1,Heltall må passe innenfor gjeldende DIGITS-innstilling(%d); fant "%s"|<value>,<value>
 26,  2,Verdi for repetisjonsantall-uttrykk i DO-instruksjon må være null eller et positivt heltall; fant "%s"|<value>
 26,  3,Verdi for FOR-uttrykk i DO-instruksjon må være null eller et positivt heltall; fant "%s"|<value>
 26,  4,Posisjonsparameter i analysemal må være et heltall; fant "%s"|<value>
 26,  5,Verdi for NUMERIC DIGITS må være null eller et positivt heltall; fant "%s"|<value>
 26,  6,Verdi for NUMERIC FUZZ må være null eller et positivt heltall; fant "%s"|<value>
 26,  7,Tall brukt i TRACE-innstilling må være et heltall; fant "%s"|<value>
 26,  8,Operand til høyre for opphøyings-operator ("**") må være et heltall; fant "%s"|<value>
 26, 11,Resultatet av operasjonen %s %% %s ville trenge eksponentiell notasjon ved gjeldende NUMERIC DIGITS %d|<value>,<value>,<value>
 26, 12,Resultatet av %%-operasjonen brukt for operasjonen %s // %s ville trenge eksponentiell notasjon ved gjeldende NUMERIC DIGITS %d|<value>,<value>,<value>
 27,  0,Ugyldig DO-syntaks
 27,  1,Ugyldig bruk av nøkkelord "%s" i DO-setning|<token>
 28,  0,Ugyldig LEAVE eller ITERATE
 28,  1,LEAVE er gyldig kun innenfor en repeterende DO-løkke
 28,  2,ITERATE er gyldig kun innenfor en repeterende DO-løkke
 28,  3,Symbolet etter LEAVE ("%s") må enten matche kontrollvariabelen i en aktiv DO-løkke eller utelates|<token>
 28,  4,Symbolet etter ITERATE ("%s") må enten matche kontrollvariabelen i en aktiv DO-løkke eller utelates|<token>
 29,  0,Miljønavn er for langt
 29,  1,Miljønavn overskrider %d tegn; fant "%s"|#Limit_EnvironmentName,<name>
 30,  0,Navn for langt eller streng for lang
 30,  1,Navn overskrider %d tegn|#Limit_Name
 30,  2,Strengkonstant overskrider %d tegn|#Limit_Literal
 31,  0,Navn starter med tall eller "."
 31,  1,Et tall kan ikke gis en verdi; fant "%s"|<token>
 31,  2,Variabelsymbol må ikke starte med et tall; fant "%s"|<token>
 31,  3,Variabelsymbol må ikke starte med et "."; fant "%s"|<token>
 32,  0,[Ugyldig bruk av «stem»]
 33,  0,Ugyldig resultat av uttrykk
 33,  1,Verdien for NUMERIC DIGITS "%d" må overstige verdien for NUMERIC FUZZ "(%d)"|<value>,<value>
 33,  2,Verdien for NUMERIC DIGITS "%d" må ikke overstige %d|<value>,#Limit_Digits
 33,  3,Resultatet av uttrykket etter NUMERIC FORM må starte med "E" eller "S"; fant "%s"|<value>
 34,  0,Logisk verdi ikke "0" eller "1"
 34,  1,Verdien av uttrykket etter nøkkelordet IF må være nøyaktig "0" eller "1"; fant "%s"|<value>
 34,  2,Verdien av uttrykket etter nøkkelordet WHEN må være nøyaktig "0" eller "1"; fant "%s"|<value>
 34,  3,Verdien av uttrykket etter nøkkelordet WHILE må være nøyaktig "0" eller "1"; fant "%s"|<value>
 34,  4,Verdien av uttrykket etter nøkkelordet UNTIL må være nøyaktig "0" eller "1"; fant "%s"|<value>
 34,  5,Verdien av uttrykket til venstre for logisk operator "%s" må være nøyaktig "0" eller "1"; fant "%s"|<operator>,<value>
 34,  6,Verdien av uttrykket til høyre for logisk operator "%s" må være nøyaktig "0" eller "1"; fant "%s"|<operator>,<value>
 35,  0,Ugyldig uttrykk
 35,  1,Ugyldig uttrykk oppdaget ved "%s"|<token>
 36,  0,Umatchet "(" i uttrykk
 37,  0,Uventet "," eller ")"
 37,  1,Uventet ","
 37,  2,Umatchet ")" i uttrykk
 38,  0,Ugyldig mal eller mønster
 38,  1,Ugyldig analysemal oppdaget ved "%s"|<token>
 38,  2,Ugyldig analyseposisjon oppdaget ved "%s"|<token>
 38,  3,Instruksjonen PARSE VALUE krever nøkkelordet WITH
 39,  0,[Overflyt i evalueringsstakk]
 40,  0,Uriktig kall til rutine
 40,  1,Ekstern rutine "%s" feilet|<name>
 40,  3,For få argumenter i kall av "%s"; ventet minst %d|<bif>,<argnumber>
 40,  4,For mange argumenter i kall av "%s"; ventet maksimalt %d|<bif>,<argnumber>
 40,  5,Argument mangler i kall av "%s"; argument %d er påkrevet|<bif>,<argnumber>
 40,  9,%s-argument %d eksponent overskrider %d sifre; fant "%s"|<bif>,<argnumber>,#Limit_ExponentDigits,<value>
 40, 11,%s-argument %d må være et tall; fant "%s"|<bif>,<argnumber>,<value>
 40, 12,%s-argument %d må være et heltall; fant "%s"|<bif>,<argnumber>,<value>
 40, 13,%s-argument %d må være null eller positivt; fant "%s"|<bif>,<argnumber>,<value>
 40, 14,%s-argument %d må være positivt; fant "%s"|<bif>,<argnumber>,<value>
 40, 17,%s-argument 1, må ha en heltallsdel i området 0:90 og en desimaldel ikke større enn .9; fant "%s"|<bif>,<value>
 40, 18,%s-konvertering må ha et årstall i området 0001 til 9999|<bif>
 40, 19,%s-argument 2, "%s", er ikke på formatet som er beskrevet av argument 3, "%s"|<bif>,<value>,<value>
 40, 21,%s-argument %d må ikke være tomt|<bif>,<argnumber>
 40, 23,%s-argument %d må være ett enkelt tegn; fant "%s"|<bif>,<argnumber>,<value>
 40, 24,%s-argument 1 må være en binær streng; fant "%s"|<bif>,<value>
 40, 25,%s-argument 1 må være en heksadesimal streng; fant "%s"|<bif>,<value>
 40, 26,%s-argument 1 må være et gyldig symbol; fant "%s"|<bif>,<value>
 40, 27,%s-argument 1, må være et gyldig strømnavn; fant "%s"|<bif>,<value>
 40, 28,%s-argument %d, opsjon må starte med en av "%s"; fant "%s"|<bif>,<argnumber>,<optionslist>,<value>
 40, 29,%s-konvertering til format "%s" er ikke tillatt|<bif>,<value>
 40, 31,%s-argument 1 ("%d") må ikke overstige 100000|<bif>,<value>
 40, 32,%s differansen mellom argument 1 ("%d") og argument 2 ("%d") må ikke overstige 100000|<bif>,<value>,<value>
 40, 33,%s-argument 1 ("%d") må være mindre enn eller likt argument 2 ("%d")|<bif>,<value>,<value>
 40, 34,%s-argument 1 ("%d") må være mindre enn eller likt antall linjer i programmet (%d)|<bif>,<value>,<sourceline()>
 40, 35,%s-argument 1 kan ikke uttrykkes som et heltall; fant "%s"|<bif>,<value>
 40, 36,%s-argument 1 må være et navn på en variabel i poolen; fant "%s"|<bif>,<value>
 40, 37,%s-argument 3 må være navnet på en pool; fant "%s"|<bif>,<value>
 40, 38,%s-argument %d er ikke stort nok til å formatere "%s"|<bif>,<argnumber>,<value>
 40, 39,%s-argument 3 er ikke null eller en; fant "%s"|<bif>,<value>
 40, 41,%s-argument %d må være innenfor grensene for aktuell strøm; fant "%s"|<bif>,<argnumber>,<value>
 40, 42,%s-argument 1; kan ikke posisjonere på aktuell strøm; fant "%s"|<bif>,<value>
 40,914,[%s-argument %d, må være en av "%s"; fant "%s"]|<bif>,<argnumber>,<optionslist>,<value>
 40,920,[%s: lavnivå I/O-feil i strøm; %s]|<bif>,<description>
 40,921,[%s-argument %d, posisjoneringsmodus "%s"; ikke kompatibel med åpnemodus for aktuell strøm]|<bif>,<argnumber>,<value>
 40,922,[%s-argument %d, for få underkommandoer; ventet minst %d; fant %d]|<bif>,<argnumber>,<value>,<value>
 40,923,[%s-argument %d, for mange underkommandoer; ventet maksimalt %d; fant %d]|<bif>,<argnumber>,<value>,<value>
 40,924,[%s-argument %d, ugyldig posisjonsspesifikasjon; ventet en av "%s"; fant "%s"]|<bif>,<argnumber>,<value>,<value>
 40,930,[RXQUEUE, funksjon TIMEOUT, ventet et heltall mellom -1 og %d; fant \"%s\"]|<value>,<value>
 40,980,Uventet input, enten ukjent type eller ugyldige data%s%s|: ,<location>
 40,981,Tall utenfor tillatt område%s%s|: ,<location>
 40,982,Streng for stor for det definerte bufferet%s%s|: ,<location>
 40,983,Ugyldig kombinasjon av type/størrelse%s%s|: ,<location>
 40,984,Ikke-støttet tall som NAN, +INF, -INF%s%s|: ,<location>
 40,985,For kompleks struktur for statisk internbuffer%s%s|: ,<location>
 40,986,Et element mangler i strukturen%s%s|: ,<location>
 40,987,En verdi mangler i strukturen%s%s|: ,<location>
 40,988,Navnet eller en del av navnet er ugyldig for tolkeren%s%s|: ,<location>
 40,989,Det oppstod et problem i grensesnittet mellom Regina og GCI%s%s|: ,<location>
 40,990,Typen fyller ikke kravene for grunntyper (argumenter/returverdi)%s%s|: ,<location>
 40,991,Antall argumenter er feil eller et argument mangler%s%s|: ,<location>
 40,992,GCIs interne stakk for argumenter fikk en overflyt%s%s|: ,<location>
 40,993,GCI telte for mange nøstede LIKE-containere%s%s|: ,<location>
 41,  0,Feil i aritmetisk konvertering
 41,  1,Ikke-numerisk verdi ("%s") til venstre for aritmetisk operasjon "%s"|<value>,<operator>
 41,  2,Ikke-numerisk verdi ("%s") til høyre for aritmetisk operasjon "%s"|<value>,<operator>
 41,  3,Ikke-numerisk verdi ("%s") brukt med prefiks-operator "%s"|<value>,<operator>
 41,  4,Verdien for TO-uttrykk i DO-instruksjon må være numerisk; fant "%s"|<value>
 41,  5,Verdien for BY-uttrykk i DO-instruksjon må være numerisk; fant "%s"|<value>
 41,  6,Verdien for kontrollvariabel-uttrykk i DO-instruksjon må være numerisk; fant "%s"|<value>
 41,  7,Eksponent overskrider %d sifre; fant "%s"|#Limit_ExponentDigits,<value>
 42,  0,Aritmetisk overflyt/underflyt
 42,  1,Aritmetisk overflyt oppdaget ved "%s %s %s"; eksponenten i resultatet krever flere enn %d sifre|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  2,Aritmetisk underflyt oppdaget ved "%s %s %s"; eksponenten i resultatet krever flere enn %d sifre|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  3,Aritmetisk overflyt; divisor må ikke være null
 43,  0,Rutine ikke funnet
 43,  1,Fant ikke rutine "%s"|<name>
 44,  0,Funksjonen returnerte ikke data
 44,  1,Ingen data returnert fra funksjon "%s"|<name>
 45,  0,Ingen data spesifisert på funksjonens RETURN
 45,  1,Ventet data på RETURN-instruksjonen fordi rutine "%s" ble kalt som en funksjon|<name>
 46,  0,Ugyldig variabelreferanse
 46,  1,Ekstra uttrykk ("%s") funnet i variabelreferanse; ventet ")"|<token>
 47,  0,Uventet plassmerke
 47,  1,INTERPRET-data må ikke inneholde plassmerker; fant "%s"|<name>
 48,  0,Svikt i systemtjeneste
 48,  1,Svikt i systemtjeneste: %s|<description>
 48,920,Lavnivå I/O-feil i strøm: %s %s: %s|<description>,<stream>,<description>
 49,  0,Tolkingsfeil
 49,  1,Tolkingsfeil: Feilet i %s, linje %d: "%s". Vennligst rapporter feilen!|<module>,<linenumber>,<description>
 50,  0,Ikke gjenkjent reservert symbol
 50,  1,Ikke gjenkjent reservert symbol "%s"|<token>
 51,  0,Ugyldig funksjonsnavn
 51,  1,Funksjonsnavn som ikke står i anførselstegn må ikke slutte på et punktum; fant "%s"|<token>
 52,  0,Resultat returnert av "%s" er lenger enn %d tegn|<name>,#Limit_String
 53,  0,Ugyldig opsjon
 53,  1,Ventet streng eller symbol etter nøkkelord STREAM; fant "%s"|<token>
 53,  2,Ventet variabelreferanse etter nøkkelord STEM; fant "%s"|<token>
 53,  3,Argument til STEM må ha ett punktum, som siste tegn; fant "%s"|<name>
 53,100,Ventet streng eller symbol etter nøkkelord LIFO; fant "%s"|<token>
 53,101,Ventet streng eller symbol etter nøkkelord FIFO; fant "%s"|<token>
 54,  0,Ugyldig STEM-verdi
 54,  1,For denne STEM APPEND må verdien av "%s" være et linjeantall; fant "%s"|<name>,<value>
#
# All error messages after this point are not defined by ANSI
#
 60,  0,[Kan ikke spole tilbake transient fil]
 61,  0,[Uriktig søkeoperasjon på fil]
 64,  0,[Syntaksfeil under analysering]
 64,  1,[Syntaksfeil på linje %d]
 64,  2,[Generell syntaksfeil på linje %d, kolonne %d]|<linenumber>,<columnnumber>
 90,  0,[Ikke-ANSI-feature brukt med "OPTIONS STRICT_ANSI"]
 90,  1,[BIF %s er en Regina-utvidelse]|<bif>
 90,  2,[Instruksjon %s er en Regina-utvidelse]|<token>
 90,  3,[%s argument %d, opsjon må starte med en av "%s" med "OPTIONS STRICT_ANSI"; fant "%s"; en Regina-utvidelse]|<bif>,<argnumber>,<optionslist>,<value>
 93,  0,[Uriktig kall til rutine]
 93,  1,[STREAM-kommando %s må etterfølges av en av "%s"; fant "%s"]|<token>,<value>,<value>
 93,  3,[STREAM-kommando må være en av "%s"; fant "%s"]|<value>,<value>
 94,  0,[Feil i grensesnitt mot ekstern kø]
 94, 99,[Intern feil med grensesnitt mot ekstern kø: %d "%s"]|<description>,<systemerror>
 94,100,[Generell systemfeil med grensesnitt mot ekstern kø. %s. %s]|<description>,<systemerror>
 94,101,[Feil under tilkobling til %s på port %d: "%s"]|<machine>,<portnumber>,<systemerror>
 94,102,[Ikke i stand til å skaffe IP-adresse for %s]|<machine>
 94,103,[Ugyldig format for tjener i spesifisert kønavn: "%s"]|<queuename>
 94,104,[Ugyldig format for kønavn: "%s"]|<queuename>
 94,105,[Ikke i stand til å starte Windows Socket-grensesnitt: %s]|<systemerror>
 94,106,[Maksimalt antall eksterne køer overskredet: %d]|<maxqueues>
 94,107,[Feil oppstod under lesing av socket: %s]|<systemerror>
 94,108,[Ugyldig bryter oversendt. Må være en av "%s"]|<switch>
 94,109,[Finner ikke køen \"%s\"]|<queuename>
 94,110,[%s ugyldig for eksterne køer]|<bif>
 94,111,[RXQUEUE-funksjon %s ugyldig for interne køer]|<functionname>
 95,  0,[Begrenset feature ble brukt i "sikker" modus]
 95,  1,[%s ugyldig i "sikker" modus]|<token>
 95,  2,[%s argument %d ugyldig i "sikker" modus]|<bif>,<argnumber>
 95,  3,[%s argument %d: "%s", ugyldig i "sikker" modus]|<bif>,<argnumber>,<token>
 95,  4,[STREAM-argument 3: Å åpne filer for WRITE-tilgang er ulovlig i "sikker" modus]
 95,  5,[Det er ulovlig å kjøre eksterne kommandoer i "sikker" modus]
100,  0,[Ukjent filsystemfeil]
#
# Extra general phrases requiring translation:
#
# Feil 3 under kjøring av "<file>" linje 1:
# Ikke i stand til å åpne språkfil: %s
# Uriktig antall meldinger i språkfil: %s
# Ikke i stand til å lese fra språkfil: %s
# Ikke i stand til å vise tekst for feil %d.%d; språkfil: %s ikke tilgjengelig
# Ikke i stand til å vise tekst for feil %d.%d; tekst mangler i språkfil: %s
# Ikke i stand til å vise tekst for feil %d.%d; språkfil: %s er ødelagt
