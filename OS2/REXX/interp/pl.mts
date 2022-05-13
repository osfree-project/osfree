#
# pl.mts Version 1, 22-08-2001
#
# Regina error messages - Polish
# Original in English Written by Mark Hessling <M.Hessling@qut.edu.au>
# Translated by Bartosz Kozlowski <bk@bn.pl>
#
#
# NOTES (Bartosz Kozlowski):
# -----
# 1. Codepage IBM-CP852 (Latin 2) was selected for this translation.
# 2. Some messages required verbal and grammatical tricks that caused weird, though acceptable, results.
# 3. Here and there, there are some inconsistencies (deliberate or not) in translations of particular expressions.
# 4. This translation was not thoroughly tested so sometimes there may come out some buggy messages out of context.
# 5. Please report "weird" messages and all suggestions to me <bk@bn.pl>.
# 6. Current version of this translation is always available from Regina REXX Translation Project at http://www.bn.pl/~bk/serv/rrtp/
#
#
# UWAGI (Bartosz Kozlowski):
# -----
# 1. Do tego tÍumaczenia zostaÍa wybrana strona kodowa IBM-CP852 (Latin 2).
# 2. NiektÛre komunikaty wymagaÍy ekwilibrystyki sÍownej i gramatycznej daj•c w efekcie dziwne, choÂ dopuszczalne, rezultaty.
# 3. Gdzieniegdzie wyst©puj• niekonsekwencje (celowe lub nie) w tÍumaczeniu niektÛrych zwrotÛw.
# 4. To tÍumaczenie nie zostaÍo gruntownie przetestowane, wobec czego czasem mog• wychodziÂ krzaki z kontekstu.
# 5. Prosz© o zgÍaszanie do mnie <bk@bn.pl> "dziwnych" komunikatÛw oraz wszelkich sugestii.
# 6. Aktualna wersja tego tÍumaczenia jest zawsze dost©pna ze strony Projektu TÍumaczenia Regina REXX pod adresem http://www.bn.pl/~bk/serv/rrtp/
#
#
#
#
#
# TODO
# 40.44
# 40,992
# 40,993
#
  0,  1,BÍ•d %s podczas dziaÍania %s, linia %d:|<value>,<source>,<linenumber>
  0,  2,BÍ•d %s w interaktywnym òladzie:|<value>
  0,  3,Interaktywny òlad.  "Trace Off", aby zako‰czyÂ "debugowanie". ENTER, by kontynuowaÂ.
  2,  0,BÍ•d podczas finalizacji
  2,  1,BÍ•d podczas finalizacji: %s|<description>
  3,  0,BÍ•d podczas inicjalizacji
  3,  1,BÍ•d podczas inicjalizacji: %s|<description>
  4,  0,Program zostaÍ przerwany
  4,  1,Program przerwany z warunkiem stop'u (HALT): %s|<description>
  5,  0,Zasoby systemowe zostaÍy wyczerpane
  5,  1,Zasoby systemowe zostaÍy wyczerpane: %s|<description>
  6,  0,Niedomkni©ty "/*", apostrof lub cudzysÍÛw
  6,  1,Niedomkni©ty separator komentarza ("/*")
  6,  2,Niedomkni©ty apostrof (')
  6,  3,Niedomkni©ty cudzysÍÛw (")
  7,  0,Spodziewane WHEN lub OTHERWISE
  7,  1,SELECT w linii %d wymaga WHEN; znaleziono "%s"|<linenumber>,<token>
  7,  2,SELECT w linii %d wymaga WHEN, OTHERWISE, lub END; znaleziono "%s"|<linenumber>,<token>
  7,  3,W SELECT z linii %d wszystkie wyraæenia WHEN s• faÍszywe; spodziewane OTHERWISE|<linenumber>
  8,  0,Niespodziewane THEN lub ELSE
  8,  1,THEN nie ma odpowiadaj•cej klauzuli IF lub WHEN
  8,  2,ELSE nie ma odpowiadaj•cej klauzuli THEN
  9,  0,Niespodziewane WHEN lub OTHERWISE
  9,  1,WHEN nie ma odpowiadaj•cego SELECT
  9,  2,OTHERWISE nie ma odpowiadaj•cego SELECT
 10,  0,Niespodziewane lub niesparowane END
 10,  1,END nie ma odpowiadaj•cego DO lub SELECT
 10,  2,Po END odpowiadaj•cemu DO z linii %d musi wyst•piÂ symbol, ktÛry odpowiada zmiennej steruj•cej (lub brak symbolu); znaleziono "%s"|<linenumber>,<token>
 10,  3,Po END odpowiadaj•cemu DO z linii %d nie moæe wyst•piÂ symbol, poniewaæ nie ma zmiennej steruj•cej; znaleziono "%s"|<linenumber>,<token>
 10,  4,Po END odpowiadaj•cemu SELECT z linii %d nie moæe wyst•piÂ symbol; znaleziono "%s"|<linenumber>,<token>
 10,  5,END nie moæe wyst•piÂ bezpoòrednio po THEN
 10,  6,END nie moæe wyst•piÂ bezpoòrednio po ELSE
 11,  0,[Stos steruj•cy peÍny]
 12,  0,[Klauzula > 1024 znakÛw]
 13,  0,Niedozwolony znak w programie
 13,  1,Niedozwolony znak w programie "('%x'X)"|<hex-encoding>
 14,  0,Niekompletne DO/SELECT/IF
 14,  1,Instrukcja DO wymaga zamykaj•cego END
 14,  2,Instrukcja SELECT wymaga zamykaj•cego END
 14,  3,Po THEN wymagana jest instrukcja
 14,  4,Po ELSE wymagana jest instrukcja
 15,  0,Niepoprawny Ía‰cuch szesnastkowy lub dwÛjkowy
 15,  1,Niepoprawne miejsce biaÍego znaku na pozycji %d w Ía‰cuchu szesnastkowym|<position>
 15,  2,Niepoprawne miejsce biaÍego znaku na pozycji %d w Ía‰cuchu dwÛjkowym|<position>
 15,  3,Jedynie 0-9, a-f, A-F, oraz biaÍe znaki s• dopuszczalne w Ía‰cuchu szesnastkowym; znaleziono "%c"|<char>
 15,  4,Jedynie 0, 1, oraz biaÍe znaki s• dopuszczalne w Íancuchu dwÛjkowym; znaleziono "%c"|<char>
 16,  0,Nie znaleziono etykiety
 16,  1,Etykieta "%s" nie zostaÍa znaleziona|<name>
 16,  2,Nie moæna uæyÂ SIGNAL do etykiety "%s", poniewaæ znajduje si© wewn•trz grupy IF, SELECT lub DO |<name>
 16,  3,Nie moæna wywoÍaÂ etykiety "%s", poniewaæ znajduje si© wewn•trz grupy IF, SELECT lub DO|<name>
 17,  0,Niespodziewane PROCEDURE
 17,  1,PROCEDURE jest dopuszczalne tylko jeòli jest pierwsz• instrukcj• wykonywan• po wewn©trznym CALL lub wywoÍaniu funkcji
 18,  0,Spodziewane THEN
 18,  1,SÍowo kluczowe IF w linii %d wymaga odpowiadaj•cej klauzuli THEN; znaleziono "%s"|<linenumber>,<token>
 18,  2,SÍowo kluczowe WHEN w linii %d wymaga odpowiadaj•cej klauzuli THEN; znaleziono "%s"|<linenumber>,<token>
 19,  0,Spodziewany Ía‰cuch albo symbol
 19,  1,Spodziewany Ía‰cuch albo symbol po sÍowie kluczowym ADDRESS; znaleziono "%s"|<token>
 19,  2,Spodziewany Ía‰cuch albo symbol po sÍowie kluczowym CALL; znaleziono "%s"|<token>
 19,  3,Spodziewany Ía‰cuch albo symbol po sÍowie kluczowym NAME; znaleziono "%s"|<token>
 19,  4,Spodziewany Ía‰cuch albo symbol po sÍowie kluczowym SIGNAL; znaleziono "%s"|<token>
 19,  6,Spodziewany Ía‰cuch albo symbol po sÍowie kluczowym TRACE; znaleziono "%s"|<token>
 19,  7,Oczekiwano symbolu we wzorcu rozbioru gramatycznego; znaleziono "%s"|<token>
 20,  0,Spodziewana nazwa
 20,  1,Wymagana nazwa; znaleziono "%s"|<token>
 20,  2,Znaleziono "%s" tam, gdzie tylko nazwa jest moæe byÂ uæyta|<token>
 21,  0,Niepoprawne dane na ko‰cu klauzuli
 21,  1,Klauzula zako‰czona niespodziewanym znacznikiem; znaleziono "%s"|<token>
 22,  0,Niepoprawny Ía‰cuch znakowy
 22,  1,Niepoprawny Ía‰cuch znakowy '%s'X|<hex-encoding>
 23,  0,Niepoprawny Ía‰cuch danych
 23,  1,Niepoprawny Ía‰cuch danych '%s'X|<hex-encoding>
 24,  0,Niepoprawne æ•danie TRACE
 24,  1,Litera æ•dania TRACE musi byÂ jedn• z "%s"; znaleziono "%c"|ACEFILNOR,<value>
 25,  0,Niepoprawne pod-sÍowo kluczowe
 25,  1,Po CALL ON musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25,  2,Po CALL OFF musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25,  3,Po SIGNAL ON musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25,  4,Po SIGNAL OFF musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25,  5,Po ADDRESS WITH musi wyst©powaÂ jedno ze sÍÛw kluczowych INPUT, OUTPUT lub ERROR; znaleziono "%s"|<token>
 25,  6,Po INPUT musi wyst©powaÂ jedno ze sÍÛw kluczowych STREAM, STEM, LIFO, FIFO lub NORMAL; znaleziono "%s"|<token>
 25,  7,Po OUTPUT musi wyst©powaÂ jedno ze sÍÛw kluczowych STREAM, STEM, LIFO, FIFO, APPEND, REPLACE lub NORMAL; znaleziono "%s"|<token>
 25,  8,Po APPEND musi wyst©powaÂ jedno ze sÍÛw kluczowych STREAM, STEM, LIFO lub FIFO; znaleziono "%s"|<token>
 25,  9,Po REPLACE musi wyst©powaÂ jedno ze sÍÛw kluczowych STREAM, STEM, LIFO lub FIFO; znaleziono "%s"|<token>
 25, 11,Po NUMERIC FORM musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 12,Po PARSE musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 13,Po UPPER musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 14,Po ERROR musi wyst©powaÂ jedno ze sÍÛw kluczowych STREAM, STEM, LIFO, FIFO, APPEND, REPLACE lub NORMAL; znaleziono "%s"|<token>
 25, 15,Po NUMERIC musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 16,Po FOREVER musi wyst©powaÂ jedno ze sÍÛw kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 17,Po PROCEDURE musi wyst©powaÂ sÍowo kluczowe EXPOSE lub nie powinno wyst©powaÂ nic; znaleziono "%s"|<token>
 26,  0,Niepoprawna liczba caÍkowita
 26,  1,Liczby caÍkowite musz• byÂ dostosowane do obecnego ustawienia DIGITS(%d); znaleziono "%s"|<value>,<value>
 26,  2,WartoòÂ wyraæeni liczby powtÛrze‰ w instrukcji DO musi byÂ liczb• caÍkowit• nieujemn•; znaleziono "%s"|<value>
#
 26,  3,WartoòÂ wyraæenia FOR w instrukcji DO musi byÂ liczb• caÍkowit• nieujemn•; znaleziono "%s"|<value>
 26,  4,Parametr pozycyjny szablonu rozbioru gramatycznego musi byÂ liczb• caÍkowit•; znaleziono "%s"|<value>
 26,  5,WartoòÂ NUMERIC DIGITS  musi byÂ liczb• caÍkowit• nieujemn•; znaleziono "%s"|<value>
 26,  6,WatoòÂ NUMERIC FUZZ musi byÂ liczb• caÍkowit• nieujemn•; znaleziono "%s"|<value>
 26,  7,Liczba uæyta w ustawieniu TRACE musi byÂ caÍkowita; znaleziono "%s"|<value>
 26,  8,WykÍadnik operatora pot©gowania ("**") musi byÂ liczb• caÍkowit•; znaleziono "%s"|<value>
 26, 11,Rezultat operacji %s %% %s wymagaÍby wykÍadniczej notacji przy obecnym NUMERIC DIGITS %d|<value>,<value>,<value>
 26, 12,Rezultat operacji %% uæyty dla operacji %s // %s wymagaÍby wykÍ•dniczej notacji przy obecnym NUMERIC DIGITS %d|<value>,<value>,<value>
 27,  0,Niepoprawna skÍadnia DO
 27,  1,Niepoprawne uæycie sÍowa kluczowego "%s" w klauzuli DO|<token>
 28,  0,Niepoprawne LEAVE lub ITERATE
 28,  1,LEAVE jest dopuszczalne tylko w obszarze powtarzaj•cej p©tli DO
 28,  2,ITERATE jest dopuszczalne tylko w obszarze powtarzaj•cej p©tli DO
 28,  3,Symbol nast©puj•cy po LEAVE ("%s") musi odpowiadaÂ b•d´ zmiennej steruj•cej, b•d´ musi zostaÂ pomini©ty|<token>
 28,  4,Symbol nast©puj•cy po ITERATE ("%s") musi odpowiadaÂ b•d´ zmiennej steruj•cej, b•d´ musi zostaÂ pomini©ty|<token>
 29,  0,Nazwa òrodowiska jest za dÍuga
 29,  1,DÍugoòÂ nazwy òrodowiska przekracza %d znak(i/Ûw); znaleziono "%s"|#Limit_EnvironmentName,<name>
 30,  0,Za dÍuga nazwa lub Ía‰cuch
 30,  1,DÍugoòÂ nazwy przekracza %d znak(i/Ûw)|#Limit_Name
 30,  2,LiteraÍ Ía‰cuchowy przekracza dÍugoòÂ %d znak(i/ow)|#Limit_Literal
 31,  0,Nazwa zaczyna si© cyfr• lub "."
 31,  1,Nie moæna przypisaÂ wartoòci liczbie; znaleziono "%s"|<token>
 31,  2,Nazwa zmiennej nie moæe zaczynaÂ si© cyfr•; znaleziono "%s"|<token>
 31,  3,Nazwa zmiennej nie moæe zaczynaÂ si© "."; znaleziono "%s"|<token>
 32,  0,[Niepoprawne uæycie stem]
 33,  0,Niepoprawny wynik wyraæenia
 33,  1,WartoòÂ NUMERIC DIGITS ("%d") musi przekraczaÂ wartoòÂ NUMERIC FUZZ ("%d")|<value>,<value>
 33,  2,WartoòÂ NUMERIC DIGITS ("%d") nie moæe przekraczaÂ %d|<value>,#Limit_Digits
 33,  3,Wynik wyraæenia nast©puj•cego po NUMERIC FORM musi zaczynaÂ si© od "E" lub "S"; znaleziono "%s"|<value>
 34,  0,WartoòÂ logiczna nie jest "0" lub "1"
 34,  1,WartoòÂ wyraæenia nast©puj•cego po sÍowie kluczowym IF musi byÂ dokÍadnie "0" lub "1"; znaleziono "%s"|<value>
 34,  2,WartoòÂ wyraæenia nast©puj•cego po sÍowie kluczowym WHEN musi byÂ dokÍadnie "0" lub "1"; znaleziono "%s"|<value>
 34,  3,WartoòÂ wyraæenia nast©puj•cego po sÍowie kluczowym WHILE musi byÂ dokÍadnie "0" lub "1"; znaleziono "%s"|<value>
 34,  4,WartoòÂ wyraæenia nast©puj•cego po sÍowie kluczowym UNTIL musi byÂ dokÍadnie "0" lub "1"; znaleziono "%s"|<value>
 34,  5,WartoòÂ wyraæenia po lewej operatora logicznego "%s" musi byÂ dokÍadnie "0" lub "1"; znaleziono "%s"|<operator>,<value>
 34,  6,WartoòÂ wyraæenia po prawej operatora logicznego "%s" musi byÂ dokÍadnie "0" lub "1"; znaleziono "%s"|<operator>,<value>
 35,  0,Niepoprawne wyraæenie
 35,  1,Wykryto niepoprawne wyraæenie w "%s"|<token>
 36,  0,Niedomkni©ty "(" w wyraæeniu
 37,  0,Niespodziewany "," or ")"
 37,  1,Niespodziewany ","
 37,  2,Nieotwarty ")" w wyraæeniu
 38,  0,Niepoprawny szablon lub wzÛr
 38,  1,Wykryto niepoprawny szablon rozbioru gramatycznego w "%s"|<token>
 38,  2,Wykryto niepoprawn• pozycj© rozbioru gramatycznego w "%s"|<token>
 38,  3,Instrukcja PARSE VALUE wymaga sÍowa kluczowego WITH
 39,  0,[PrzepeÍnienie stosu wartoòciowania]
 40,  0,Niepoprawne wywoÍanie podprogramu
 40,  1,ZawiÛdÍ zewn©trzny podprogram "%s"|<name>
 40,  3,Niewystarczaj•ca liczba argumentÛw przy wywoÍaniu "%s"; oczekiwane minimum to %d|<bif>,<argnumber>
 40,  4,Zbyt duæa liczba argumentÛw przy wywoÍaniu "%s"; oczekiwane maksimum to %d|<bif>,<argnumber>
 40,  5,Brakuj•cy argument przy wyoÍaniu "%s"; argument %d jest wymagany|<bif>,<argnumber>
 40,  9,W %s argument %d wykÍadnik przekracza %d cyfr(y); znaleziono "%s"|<bif>,<argnumber>,#Limit_ExponentDigits,<value>
 40, 11,W %s argument %d musi byÂ liczb•; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 12,W %s argument %d musi byÂ liczb• caÍkowit•; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 13,W %s argument %d musi byÂ liczb• nieujemn•; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 14,W %s argument %d musi byÂ dodatni; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 17,Argument %s nr 1, musi mieÂ cz©òÂ caÍkowit• z zakresu 0:90 i cz©òÂ dziesi©tn• nie wi©ksz• niæ .9; znaleziono "%s"|<bif>,<value>
 40, 18,Rok w konwersji %s musi byÂ z zakresu od 0001 to 9999|<bif>
 40, 19,Argument %s nr 2, "%s", nie jest w formacie opisanym przez 3 argument, "%s"|<bif>,<value>,<value>
 40, 21,W %s argument %d nie moæe byÂ pusty|<bif>,<argnumber>
 40, 23,W %s argument %d musi byÂ pojedynczym znakiem; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 24,W %s argument 1 musi byÂ Ía‰cuchem dwÛjkowym; znaleziono "%s"|<bif>,<value>
 40, 25,W %s argument 1 musi byÂ Ía‰cuchem szesnastkowym; znaleziono "%s"|<bif>,<value>
 40, 26,Argument %s nr 1 musi byÂ a valid symbol; found "%s"|<bif>,<value>
 40, 27,Argument %s nr 1, musi byÂ poprawn• nazw• strumienia; znaleziono "%s"|<bif>,<value>
 40, 28,Argument %s nr %d, opcja musi zaczynaÂ si© jednym z "%s"; znaleziono "%s"|<bif>,<argnumber>,<optionslist>,<value>
 40, 29,Konwersja %s do formatu "%s" nie jest dozwolona|<bif>,<value>
 40, 31,Argument %s nr 1 ("%d") nie moæe przekraczaÂ 100000|<bif>,<value>
 40, 32,W %s rÛænica mi©dzy argumentem 1 ("%d") a argumentem 2 ("%d") nie moæe przekraczaÂ 100000|<bif>,<value>,<value>
 40, 33,Argument %s nr 1 ("%d") musi byÂ mniejszy lub rÛwny od argumentu 2 ("%d")|<bif>,<value>,<value>
 40, 34,Argument %s nr 1 ("%d") musi byÂ mniejszy lub rÛwny od liczby linii w programie (%d)|<bif>,<value>,<sourceline()>
 40, 35,Argument %s nr 1 nie moæe byÂ wyraæony jako liczba caÍkowita; znaleziono "%s"|<bif>,<value>
 40, 36,W %s argument 1 musi byÂ nazw• zmiennej zasobu; znaleziono "%s"|<bif>,<value>
 40, 37,W %s argument 3 musi byÂ nazw• zasobu; znaleziono "%s"|<bif>,<value>
 40, 38,Argument %s nr %d nie jest wystarczaj•co duæy aby sformatowaÂ "%s"|<bif>,<argnumber>,<value>
 40, 39,Argument %s nr 3 nie jest zerem lub jedynk•; znaleziono "%s"|<bif>,<value>
 40, 41,W %s argument %d musi zawieraÂ si© w ograniczeniach strumienia; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 42,Argument %s nr 1; nie powinien wyst•piÂ w strumieniu; znaleziono "%s"|<bif>,<value>
 40, 43,%s argument %d musi byÂ pojedynczy znak nie alfanumeryczny lub Ía‰cuch pusty; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 44,%s argument %d, "%s", is a format incompatible with the separator specified in argument %d|<bif>,<argnumber>,<value>,<argnumber>
 40,914,[Argument %s nr %d, musi byÂ jednym z "%s"; znaleziono "%s"]|<bif>,<argnumber>,<optionslist>,<value>
 40,920,[%s: niskopoziomowy bÍ•d I/O strumienia; %s]|<bif>,<description>
 40,921,[Argument %s nr %d, tryb pozycjonowania strumienia "%s"; niekompatybilny z trybem otwarcia strumienia]|<bif>,<argnumber>,<value>
 40,922,[Argument %s nr %d, za maÍo pod-polece‰; oczekiwane minimum to %d; znaleziono %d]|<bif>,<argnumber>,<value>,<value>
 40,923,[Argument %s nr %d, za wiele pod-polece‰; oczekiwane maksimum to %d; znaleziono %d]|<bif>,<argnumber>,<value>,<value>
 40,924,[Argument %s nr %d, niepoprawny parametr pozycjonowania; spodziewane jedno z "%s"; znaleziono "%s"]|<bif>,<argnumber>,<value>,<value>
 40,930,[RXQUEUE, funkcja TIMEOUT, spodziewano liczby caÍkowitej z zakresu od 0 do %d; znaleziono \"%s\"]|<value>,<value>
 40,980,Niespodziewana wartoòÂ na wejòciu, albo nieznany typ albo niedozwolone dane%s%s|:,<location>
 40,981,Liczba poza dozwolonym zakresem%s%s|:,<location>
 40,982,ùa‰cuch za duæy dla zdefiniowanego bufora%s%s|:,<location>
 40,983,Niepoprawna kombinacja typ/rozmiar%s%s|:,<location>
 40,984,Niewspierana liczba jak NAN, +INF, -INF%s%s|:,<location>
 40,985,Struktura zbyt zÍoæona dla statycznego, wewn©trznego bufora%s%s|:,<location>
 40,986,Brak elementu struktury%s%s|:,<location>
 40,987,Brak wartoòci struktury%s%s|:,<location>
 40,988,Nazwa b•d´ jej cz©òÂ jest niedozwolona dla interpretera%s%s|:,<location>
 40,989,Wyst•piÍ problem na interfejsie mi©dzy Regin• a GCI%s%s|:,<location>
 40,990,Typ nie speÍnia wymaga‰ dla typÛw podstawowych (argumenty/zwracana wartoòÂ)%s%s|:,<location>
 40,991,Niepoprawna liczba argumentÛw%s%s|:,<location>
 40,992,GCI's internal stack for arguments got an overflow%s%s|: ,<location>
 40,993,GCI counted too many nested LIKE containers%s%s|: ,<location>
 41,  0,Niepoprawna konwersja arytmetyczna
 41,  1,Nieliczbowa wartoòÂ ("%s") lewej strony operacji arytmetycznej "%s"|<value>,<operator>
 41,  2,Nieliczbowa wartoòÂ ("%s") prawej strony operacji arytmetycznej "%s"|<value>,<operator>
 41,  3,Nieliczbowa wartoòÂ ("%s") uæyta z przedrostkiem opertora "%s"|<value>,<operator>
 41,  4,WartoòÂ wyraæenia TO w instrukcji DO musi byÂ liczbow•; znaleziono "%s"|<value>
 41,  5,WartoòÂ wyraæenia BY w instrukcji DO musi byÂ liczbow•; znaleziono "%s"|<value>
 41,  6,WartoòÂ wyraæenia zmiennej steruj•cej w instrucji DO musi byÂ liczbow•; znaleziono "%s"|<value>
 41,  7,WykÍadnik przekracza %d cyfr; znaleziono "%s"|#Limit_ExponentDigits,<value>
 42,  0,Nadmiar/niedomiar arytmetyczny
 42,  1,Wykryto nadmiar arytmetyczny w "%s %s %s"; wyraziciel rezultatu wymaga wi©cej niæ %d cyfr|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  2,Wykryto niedomiar arytmetyczny w "%s %s %s"; wyraziciel rezultatu wymaga wi©cej niæ %d cyfr|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  3,Nadmiar arytmetyczny; dzielnik nie moæe byÂ zerem
 43,  0,Nie znaleziono podprogramu
 43,  1,Nie moæna odnale´Â podprogramu "%s"|<name>
 44,  0,Funkcja nie zwrÛciÍa danych
 44,  1,Brak danych zwrÛconych przez funkcj© "%s"|<name>
 45,  0,Brak danych wyspecyfikowanych przy RETURN funkcji
 45,  1,Spodziewano danych z instrukcj• RETURN, poniewaæ podprogram "%s" zostaÍ wywoÍany jako funkcja|<name>
 46,  0,Niepoprawne odniesienie do zmiennej
 46,  1,Znaleziono dodatkowy skÍadnik ("%s") w odniesieniu zmiennej; ")" spodziewano|<token>
 47,  0,Niespodziewana etykieta
 47,  1,Dane INTERPRET nie mog• zawieraÂ etytkiet; znaleziono "%s"|<name>
 48,  0,BÍ•d w usÍudze systemowej
 48,  1,BÍ•d w usÍudze systemowej: %s|<description>
 48,920,Niskopoziomowy bÍ•d strumienia I/O: %s %s: %s|<description>,<stream>,<description>
 49,  0,BÍ•d interpretacji
 49,  1,BÍ•d interpretacji: BÍ•d w %s, linia %d: "%s". Prosz© zgÍosiÂ ten bÍ•d!|<module>,<linenumber>,<description>
 50,  0,Nierozpoznany zastrzeæony symbol
 50,  1,Nierozpoznany zastrzeæony symbol "%s"|<token>
 51,  0,Niepoprawna nazwa funkcji
 51,  1,Nazwy funkcji nie uj©te w cudzysÍÛw nie mog• ko‰czyÂ si© kropk•; znaleziono "%s"|<token>
 52,  0,Wynik zwrÛcony przez "%s" jest dÍuæszy niæ %d znak(i/Ûw)|<name>,#Limit_String
 53,  0,Niepoprawna opcja
 53,  1,ùa‰cuch lub symbol oczekiwany po sÍowie kluczowym STREAM; znaleziono "%s"|<token>
 53,  2,Po sÍowie kluczowym STEM oczekiwane odniesienie do zmiennej; znaleziono "%s"|<token>
 53,  3,Argument do STEM musi mieÂ jedn• kropk© jako ostatni znak; znaleziono "%s"|<name>
 53,100, ùa‰cuch lub symbol spodziewane po sÍowie kluczowym LIFO; znaleziono "%s"|<token>
 53,101,ùa‰cuch lub symbol spodziewane po sÍowie kluczowym FIFO; znaleziono "%s"|<token>
 54,  0,Niepoprawna wartoòÂ STEM
 54,  1,Dla tego STEM APPEND, wartoòÂ "%s" musi byÂ liczb• linii; znaleziono "%s"|<name>,<value>
#
# Wszystkie poniæsze komunikaty nie s• zdefiniowane przez ANSI
#
 60,  0,[Nie moæna cofn•Â kursora pliku tymczasowego]
 61,  0,[Niepoprawna operacja pozycjonowania na pliku]
 64,  0,[BÍ•d skÍadni podczas rozbioru gramatycznego]
 64,  1,[BÍ•d skÍadni w linii %d]
 64,  2,[Podstawowy bÍ•d skÍ•dni w linii %d, kolumna %d]|<linenumber>,<columnnumber>
 90,  0,[Opcja niezgodna z ANSI uæyta przy "OPTIONS STRICT_ANSI"]
 90,  1,[%s jest funkcj• BIF rozszerzenia Regina'y]|<bif>
 90,  2,[%s jest instrukcj• rozszerzenia Regina'y]|<token>
 90,  3,[Argument %s nr %d, Przy "OPTIONS STRICT_ANSI" opcja musi zaczynaÂ si© jednym z "%s; znaleziono "%s"; rozszerzenie Regina'y]|<bif>,<argnumber>,<optionslist>,<value>
 90,  4,[%s jest operatora rozszerzenia Regina'y]|<token>
 93,  0,[Niepoprawne wywoÍanie procedury]
 93,  1,[Po komendzie STREAM %s musi wyst©powaÂ jedna z "%s"; znaleziono "%s"]|<token>,<value>,<value>
 93,  3,[Komenda STREAM musi byÂ jedn• z "%s"; znaleziono "%s"]|<value>,<value>
 94,  0,[BÍ•d interfejsu zewn©trznej kolejki]
 94,  1,[External queue timed out]
 94, 99,[Wewn©trzny bÍ•d z interfejsem zewn©trznej kolejki: %d "%s"]|<description>,<systemerror
 94,100,[Podstawowy bÍ•d systemu z interfejsem zewn©trznej kolejki. %s. %s]|<description>,<systemerror>
 94,101,[BÍ•d podczas Í•czenia si© z %s na porcie %d: "%s"]|<machine>,<portnumber>,<systemerror>
 94,102,[Nie moæna uzyskaÂ adresu IP dla %s]|<machine>
 94,103,[Niepoprawny format dla serwera w podanej nazwie kolejki: "%s"]|<queuename>
 94,104,[Niepoprawny format nazwy kolejki: "%s"]|<queuename>
 94,105,[Nie moæna uruchomiÂ interfejsu gniazd systemu Windows: %s]|<systemerror>
 94,106,[Przekroczono maksymaln• liczb© zewn©trznych kolejek: %d]|<maxqueues>
 94,107,[Wyst•piÍ bÍ•d podczas czytania z gniazda: %s]|<systemerror>
 94,108,[Przekazano niepoprawny przeÍ•cznik. Musi byÂ jednym z "%s"]|<switch>
 94,109,[Kolejka \"%s\" nie znaleziona]|<queuename>
 94,110,[%s niepoprawna dla zewn©trznych kolejek]|<bif>
 94,111,[Funkcja RXQUEUE %s niepoprawna dla wewn©trznych kolejek]|<functionname>
 94,112,[Unable to %s SESSION queue]|<action>
 95,  0,[Zastrzeæona opcja uæyta w trybie "bezpiecznym"]
 95,  1,[%s niedozwolony w trybie "bezpiecznym"]|<token>
 95,  2,[%s argument %d niedozwolony w trybie "bezpiecznym"]|<bif>,<argnumber>
 95,  3,[Argument %s nr %d: "%s", niedozwolony w trybie "bezpiecznym"]|<bif>,<argnumber>,<token>
 95,  4,[3 argument STREAM: Otwieranie plikÛw w trybie WRITE jest niepoprawne w trybie "bezpiecznym"]
 95,  5,[Uruchamianie polece‰ zewn©trznych jest niedozwolone w trybie "bezpiecznym"]
100,  0,[Nieznany bÍ•d systemu plikÛw]
#
# Extra general phrases requiring translation:
# The text in () is the corresponding array name in error.c
#
P,  0,Bà•d %d podczas dziaàania "%.*s", linia %d: %.*s
P,  1,Bà•d %d.%d: %.*s
P,  2,Bà•d %d podczas dziaàania "%.*s": %.*s
