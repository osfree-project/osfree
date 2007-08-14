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
# 1. Do tego tàumaczenia zostaàa wybrana strona kodowa IBM-CP852 (Latin 2).
# 2. Niekt¢re komunikaty wymagaày ekwilibrystyki sàownej i gramatycznej daj•c w efekcie dziwne, choÜ dopuszczalne, rezultaty.
# 3. Gdzieniegdzie wyst©puj• niekonsekwencje (celowe lub nie) w tàumaczeniu niekt¢rych zwrot¢w.
# 4. To tàumaczenie nie zostaào gruntownie przetestowane, wobec czego czasem mog• wychodziÜ krzaki z kontekstu.
# 5. Prosz© o zgàaszanie do mnie <bk@bn.pl> "dziwnych" komunikat¢w oraz wszelkich sugestii.
# 6. Aktualna wersja tego tàumaczenia jest zawsze dost©pna ze strony Projektu Tàumaczenia Regina REXX pod adresem http://www.bn.pl/~bk/serv/rrtp/
#
#
#
#
#
  0,  1,Bà•d %s podczas dziaàania %s, linia %d:|<value>,<source>,<linenumber>
  0,  2,Bà•d %s w interaktywnym òladzie:|<value>
  0,  3,Interaktywny òlad.  "Trace Off", aby zako‰czyÜ "debugowanie". ENTER, by kontynuowaÜ.
  2,  0,Bà•d podczas finalizacji
  2,  1,Bà•d podczas finalizacji: %s|<description>
  3,  0,Bà•d podczas inicjalizacji
  3,  1,Bà•d podczas inicjalizacji: %s|<description>
  4,  0,Program zostaà przerwany
  4,  1,Program przerwany z warunkiem stop'u (HALT): %s|<description>
  5,  0,Zasoby systemowe zostaày wyczerpane
  5,  1,Zasoby systemowe zostaày wyczerpane: %s|<description>
  6,  0,Niedomkni©ty "/*", apostrof lub cudzysà¢w
  6,  1,Niedomkni©ty separator komentarza ("/*")
  6,  2,Niedomkni©ty apostrof (')
  6,  3,Niedomkni©ty cudzysà¢w (")
  7,  0,Spodziewane WHEN lub OTHERWISE
  7,  1,SELECT w linii %d wymaga WHEN; znaleziono "%s"|<linenumber>,<token>
  7,  2,SELECT w linii %d wymaga WHEN, OTHERWISE, lub END; znaleziono "%s"|<linenumber>,<token>
  7,  3,W SELECT z linii %d wszystkie wyraæenia WHEN s• faàszywe; spodziewane OTHERWISE|<linenumber>
  8,  0,Niespodziewane THEN lub ELSE
  8,  1,THEN nie ma odpowiadaj•cej klauzuli IF lub WHEN
  8,  2,ELSE nie ma odpowiadaj•cej klauzuli THEN
  9,  0,Niespodziewane WHEN lub OTHERWISE
  9,  1,WHEN nie ma odpowiadaj•cego SELECT
  9,  2,OTHERWISE nie ma odpowiadaj•cego SELECT
 10,  0,Niespodziewane lub niesparowane END
 10,  1,END nie ma odpowiadaj•cego DO lub SELECT
 10,  2,Po END odpowiadaj•cemu DO z linii %d musi wyst•piÜ symbol, kt¢ry odpowiada zmiennej steruj•cej (lub brak symbolu); znaleziono "%s"|<linenumber>,<token>
 10,  3,Po END odpowiadaj•cemu DO z linii %d nie moæe wyst•piÜ symbol, poniewaæ nie ma zmiennej steruj•cej; znaleziono "%s"|<linenumber>,<token>
 10,  4,Po END odpowiadaj•cemu SELECT z linii %d nie moæe wyst•piÜ symbol; znaleziono "%s"|<linenumber>,<token>
 10,  5,END nie moæe wyst•piÜ bezpoòrednio po THEN
 10,  6,END nie moæe wyst•piÜ bezpoòrednio po ELSE
 11,  0,[Stos steruj•cy peàny]
 12,  0,[Klauzula > 1024 znak¢w]
 13,  0,Niedozwolony znak w programie
 13,  1,Niedozwolony znak w programie "('%x'X)"|<hex-encoding>
 14,  0,Niekompletne DO/SELECT/IF
 14,  1,Instrukcja DO wymaga zamykaj•cego END
 14,  2,Instrukcja SELECT wymaga zamykaj•cego END
 14,  3,Po THEN wymagana jest instrukcja
 14,  4,Po ELSE wymagana jest instrukcja
 15,  0,Niepoprawny àa‰cuch szesnastkowy lub dw¢jkowy
 15,  1,Niepoprawne miejsce biaàego znaku na pozycji %d w àa‰cuchu szesnastkowym|<position>
 15,  2,Niepoprawne miejsce biaàego znaku na pozycji %d w àa‰cuchu dw¢jkowym|<position>
 15,  3,Jedynie 0-9, a-f, A-F, oraz biaàe znaki s• dopuszczalne w àa‰cuchu szesnastkowym; znaleziono "%c"|<char>
 15,  4,Jedynie 0, 1, oraz biaàe znaki s• dopuszczalne w àancuchu dw¢jkowym; znaleziono "%c"|<char>
 16,  0,Nie znaleziono etykiety
 16,  1,Etykieta "%s" nie zostaàa znaleziona|<name>
 16,  2,Nie moæna uæyÜ SIGNAL do etykiety "%s", poniewaæ znajduje si© wewn•trz grupy IF, SELECT lub DO |<name>
 16,  3,Nie moæna wywoàaÜ etykiety "%s", poniewaæ znajduje si© wewn•trz grupy IF, SELECT lub DO|<name>
 17,  0,Niespodziewane PROCEDURE
 17,  1,PROCEDURE jest dopuszczalne tylko jeòli jest pierwsz• instrukcj• wykonywan• po wewn©trznym CALL lub wywoàaniu funkcji
 18,  0,Spodziewane THEN
 18,  1,Sàowo kluczowe IF w linii %d wymaga odpowiadaj•cej klauzuli THEN; znaleziono "%s"|<linenumber>,<token>
 18,  2,Sàowo kluczowe WHEN w linii %d wymaga odpowiadaj•cej klauzuli THEN; znaleziono "%s"|<linenumber>,<token>
 19,  0,Spodziewany àa‰cuch albo symbol
 19,  1,Spodziewany àa‰cuch albo symbol po sàowie kluczowym ADDRESS; znaleziono "%s"|<token>
 19,  2,Spodziewany àa‰cuch albo symbol po sàowie kluczowym CALL; znaleziono "%s"|<token>
 19,  3,Spodziewany àa‰cuch albo symbol po sàowie kluczowym NAME; znaleziono "%s"|<token>
 19,  4,Spodziewany àa‰cuch albo symbol po sàowie kluczowym SIGNAL; znaleziono "%s"|<token>
 19,  6,Spodziewany àa‰cuch albo symbol po sàowie kluczowym TRACE; znaleziono "%s"|<token>
 19,  7,Oczekiwano symbolu we wzorcu rozbioru gramatycznego; znaleziono "%s"|<token>
 20,  0,Spodziewana nazwa
 20,  1,Wymagana nazwa; znaleziono "%s"|<token>
 20,  2,Znaleziono "%s" tam, gdzie tylko nazwa jest moæe byÜ uæyta|<token>
 21,  0,Niepoprawne dane na ko‰cu klauzuli
 21,  1,Klauzula zako‰czona niespodziewanym znacznikiem; znaleziono "%s"|<token>
 22,  0,Niepoprawny àa‰cuch znakowy
 22,  1,Niepoprawny àa‰cuch znakowy '%s'X|<hex-encoding>
 23,  0,Niepoprawny àa‰cuch danych
 23,  1,Niepoprawny àa‰cuch danych '%s'X|<hex-encoding>
 24,  0,Niepoprawne æ•danie TRACE
 24,  1,Litera æ•dania TRACE musi byÜ jedn• z "%s"; znaleziono "%c"|ACEFILNOR,<value>
 25,  0,Niepoprawne pod-sàowo kluczowe
 25,  1,Po CALL ON musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25,  2,Po CALL OFF musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25,  3,Po SIGNAL ON musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25,  4,Po SIGNAL OFF musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25,  5,Po ADDRESS WITH musi wyst©powaÜ jedno ze sà¢w kluczowych INPUT, OUTPUT lub ERROR; znaleziono "%s"|<token>
 25,  6,Po INPUT musi wyst©powaÜ jedno ze sà¢w kluczowych STREAM, STEM, LIFO, FIFO lub NORMAL; znaleziono "%s"|<token>
 25,  7,Po OUTPUT musi wyst©powaÜ jedno ze sà¢w kluczowych STREAM, STEM, LIFO, FIFO, APPEND, REPLACE lub NORMAL; znaleziono "%s"|<token>
 25,  8,Po APPEND musi wyst©powaÜ jedno ze sà¢w kluczowych STREAM, STEM, LIFO lub FIFO; znaleziono "%s"|<token>
 25,  9,Po REPLACE musi wyst©powaÜ jedno ze sà¢w kluczowych STREAM, STEM, LIFO lub FIFO; znaleziono "%s"|<token>
 25, 11,Po NUMERIC FORM musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 12,Po PARSE musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 13,Po UPPER musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 14,Po ERROR musi wyst©powaÜ jedno ze sà¢w kluczowych STREAM, STEM, LIFO, FIFO, APPEND, REPLACE lub NORMAL; znaleziono "%s"|<token>
 25, 15,Po NUMERIC musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 16,Po FOREVER musi wyst©powaÜ jedno ze sà¢w kluczowych %s; znaleziono "%s"|<keywords>,<token>
 25, 17,Po PROCEDURE musi wyst©powaÜ sàowo kluczowe EXPOSE lub nie powinno wyst©powaÜ nic; znaleziono "%s"|<token>
 26,  0,Niepoprawna liczba caàkowita
 26,  1,Liczby caàkowite musz• byÜ dostosowane do obecnego ustawienia DIGITS(%d); znaleziono "%s"|<value>,<value>
 26,  2,WartoòÜ wyraæeni liczby powt¢rze‰ w instrukcji DO musi byÜ liczb• caàkowit• nieujemn•; znaleziono "%s"|<value>
#
 26,  3,WartoòÜ wyraæenia FOR w instrukcji DO musi byÜ liczb• caàkowit• nieujemn•; znaleziono "%s"|<value>
 26,  4,Parametr pozycyjny szablonu rozbioru gramatycznego musi byÜ liczb• caàkowit•; znaleziono "%s"|<value>
 26,  5,WartoòÜ NUMERIC DIGITS  musi byÜ liczb• caàkowit• nieujemn•; znaleziono "%s"|<value>
 26,  6,WatoòÜ NUMERIC FUZZ musi byÜ liczb• caàkowit• nieujemn•; znaleziono "%s"|<value>
 26,  7,Liczba uæyta w ustawieniu TRACE musi byÜ caàkowita; znaleziono "%s"|<value>
 26,  8,Wykàadnik operatora pot©gowania ("**") musi byÜ liczb• caàkowit•; znaleziono "%s"|<value>
 26, 11,Rezultat operacji %s %% %s wymagaàby wykàadniczej notacji przy obecnym NUMERIC DIGITS %d|<value>,<value>,<value>
 26, 12,Rezultat operacji %% uæyty dla operacji %s // %s wymagaàby wykà•dniczej notacji przy obecnym NUMERIC DIGITS %d|<value>,<value>,<value>
 27,  0,Niepoprawna skàadnia DO
 27,  1,Niepoprawne uæycie sàowa kluczowego "%s" w klauzuli DO|<token>
 28,  0,Niepoprawne LEAVE lub ITERATE
 28,  1,LEAVE jest dopuszczalne tylko w obszarze powtarzaj•cej p©tli DO
 28,  2,ITERATE jest dopuszczalne tylko w obszarze powtarzaj•cej p©tli DO
 28,  3,Symbol nast©puj•cy po LEAVE ("%s") musi odpowiadaÜ b•d´ zmiennej steruj•cej, b•d´ musi zostaÜ pomini©ty|<token>
 28,  4,Symbol nast©puj•cy po ITERATE ("%s") musi odpowiadaÜ b•d´ zmiennej steruj•cej, b•d´ musi zostaÜ pomini©ty|<token>
 29,  0,Nazwa òrodowiska jest za dàuga
 29,  1,DàugoòÜ nazwy òrodowiska przekracza %d znak(i/¢w); znaleziono "%s"|#Limit_EnvironmentName,<name>
 30,  0,Za dàuga nazwa lub àa‰cuch
 30,  1,DàugoòÜ nazwy przekracza %d znak(i/¢w)|#Limit_Name
 30,  2,Literaà àa‰cuchowy przekracza dàugoòÜ %d znak(i/ow)|#Limit_Literal
 31,  0,Nazwa zaczyna si© cyfr• lub "."
 31,  1,Nie moæna przypisaÜ wartoòci liczbie; znaleziono "%s"|<token>
 31,  2,Nazwa zmiennej nie moæe zaczynaÜ si© cyfr•; znaleziono "%s"|<token>
 31,  3,Nazwa zmiennej nie moæe zaczynaÜ si© "."; znaleziono "%s"|<token>
 32,  0,[Niepoprawne uæycie stem]
 33,  0,Niepoprawny wynik wyraæenia
 33,  1,WartoòÜ NUMERIC DIGITS "%d" musi przekraczaÜ wartoòÜ NUMERIC FUZZ "(%d)"|<value>,<value>
 33,  2,WartoòÜ NUMERIC DIGITS "%d" nie moæe przekraczaÜ %d|<value>,#Limit_Digits
 33,  3,Wynik wyraæenia nast©puj•cego po NUMERIC FORM musi zaczynaÜ si© od "E" lub "S"; znaleziono "%s"|<value>
 34,  0,WartoòÜ logiczna nie jest "0" lub "1"
 34,  1,WartoòÜ wyraæenia nast©puj•cego po sàowie kluczowym IF musi byÜ dokàadnie "0" lub "1"; znaleziono "%s"|<value>
 34,  2,WartoòÜ wyraæenia nast©puj•cego po sàowie kluczowym WHEN musi byÜ dokàadnie "0" lub "1"; znaleziono "%s"|<value>
 34,  3,WartoòÜ wyraæenia nast©puj•cego po sàowie kluczowym WHILE musi byÜ dokàadnie "0" lub "1"; znaleziono "%s"|<value>
 34,  4,WartoòÜ wyraæenia nast©puj•cego po sàowie kluczowym UNTIL musi byÜ dokàadnie "0" lub "1"; znaleziono "%s"|<value>
 34,  5,WartoòÜ wyraæenia po lewej operatora logicznego "%s" musi byÜ dokàadnie "0" lub "1"; znaleziono "%s"|<operator>,<value>
 34,  6,WartoòÜ wyraæenia po prawej operatora logicznego "%s" musi byÜ dokàadnie "0" lub "1"; znaleziono "%s"|<operator>,<value>
 35,  0,Niepoprawne wyraæenie
 35,  1,Wykryto niepoprawne wyraæenie w "%s"|<token>
 36,  0,Niedomkni©ty "(" w wyraæeniu
 37,  0,Niespodziewany "," or ")"
 37,  1,Niespodziewany ","
 37,  2,Nieotwarty ")" w wyraæeniu
 38,  0,Niepoprawny szablon lub wz¢r
 38,  1,Wykryto niepoprawny szablon rozbioru gramatycznego w "%s"|<token>
 38,  2,Wykryto niepoprawn• pozycj© rozbioru gramatycznego w "%s"|<token>
 38,  3,Instrukcja PARSE VALUE wymaga sàowa kluczowego WITH
 39,  0,[Przepeànienie stosu wartoòciowania]
 40,  0,Niepoprawne wywoàanie podprogramu
 40,  1,Zawi¢dà zewn©trzny podprogram "%s"|<name>
 40,  3,Niewystarczaj•ca liczba argument¢w przy wywoàaniu "%s"; oczekiwane minimum to %d|<bif>,<argnumber>
 40,  4,Zbyt duæa liczba argument¢w przy wywoàaniu "%s"; oczekiwane maksimum to %d|<bif>,<argnumber>
 40,  5,Brakuj•cy argument przy wyoàaniu "%s"; argument %d jest wymagany|<bif>,<argnumber>
 40,  9,W %s argument %d wykàadnik przekracza %d cyfr(y); znaleziono "%s"|<bif>,<argnumber>,#Limit_ExponentDigits,<value>
 40, 11,W %s argument %d musi byÜ liczb•; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 12,W %s argument %d musi byÜ liczb• caàkowit•; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 13,W %s argument %d musi byÜ liczb• nieujemn•; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 14,W %s argument %d musi byÜ dodatni; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 17,Argument %s nr 1, musi mieÜ cz©òÜ caàkowit• z zakresu 0:90 i cz©òÜ dziesi©tn• nie wi©ksz• niæ .9; znaleziono "%s"|<bif>,<value>
 40, 18,Rok w konwersji %s musi byÜ z zakresu od 0001 to 9999|<bif>
 40, 19,Argument %s nr 2, "%s", nie jest w formacie opisanym przez 3 argument, "%s"|<bif>,<value>,<value>
 40, 21,W %s argument %d nie moæe byÜ pusty|<bif>,<argnumber>
 40, 23,W %s argument %d musi byÜ pojedynczym znakiem; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 24,W %s argument 1 musi byÜ àa‰cuchem dw¢jkowym; znaleziono "%s"|<bif>,<value>
 40, 25,W %s argument 1 musi byÜ àa‰cuchem szesnastkowym; znaleziono "%s"|<bif>,<value>
 40, 26,Argument %s nr 1 musi byÜ a valid symbol; found "%s"|<bif>,<value>
 40, 27,Argument %s nr 1, musi byÜ poprawn• nazw• strumienia; znaleziono "%s"|<bif>,<value>
 40, 28,Argument %s nr %d, opcja musi zaczynaÜ si© jednym z "%s"; znaleziono "%s"|<bif>,<argnumber>,<optionslist>,<value>
 40, 29,Konwersja %s do formatu "%s" nie jest dozwolona|<bif>,<value>
 40, 31,Argument %s nr 1 ("%d") nie moæe przekraczaÜ 100000|<bif>,<value>
 40, 32,W %s r¢ænica mi©dzy argumentem 1 ("%d") a argumentem 2 ("%d") nie moæe przekraczaÜ 100000|<bif>,<value>,<value>
 40, 33,Argument %s nr 1 ("%d") musi byÜ mniejszy lub r¢wny od argumentu 2 ("%d")|<bif>,<value>,<value>
 40, 34,Argument %s nr 1 ("%d") musi byÜ mniejszy lub r¢wny od liczby linii w programie (%d)|<bif>,<value>,<sourceline()>
 40, 35,Argument %s nr 1 nie moæe byÜ wyraæony jako liczba caàkowita; znaleziono "%s"|<bif>,<value>
 40, 36,W %s argument 1 musi byÜ nazw• zmiennej zasobu; znaleziono "%s"|<bif>,<value>
 40, 37,W %s argument 3 musi byÜ nazw• zasobu; znaleziono "%s"|<bif>,<value>
 40, 38,Argument %s nr %d nie jest wystarczaj•co duæy aby sformatowaÜ "%s"|<bif>,<argnumber>,<value>
 40, 39,Argument %s nr 3 nie jest zerem lub jedynk•; znaleziono "%s"|<bif>,<value>
 40, 41,W %s argument %d musi zawieraÜ si© w ograniczeniach strumienia; znaleziono "%s"|<bif>,<argnumber>,<value>
 40, 42,Argument %s nr 1; nie powinien wyst•piÜ w strumieniu; znaleziono "%s"|<bif>,<value>
 40,914,[Argument %s nr %d, musi byÜ jednym z "%s"; znaleziono "%s"]|<bif>,<argnumber>,<optionslist>,<value>
 40,920,[%s: niskopoziomowy bà•d I/O strumienia; %s]|<bif>,<description>
 40,921,[Argument %s nr %d, tryb pozycjonowania strumienia "%s"; niekompatybilny z trybem otwarcia strumienia]|<bif>,<argnumber>,<value>
 40,922,[Argument %s nr %d, za maào pod-polece‰; oczekiwane minimum to %d; znaleziono %d]|<bif>,<argnumber>,<value>,<value>
 40,923,[Argument %s nr %d, za wiele pod-polece‰; oczekiwane maksimum to %d; znaleziono %d]|<bif>,<argnumber>,<value>,<value>
 40,924,[Argument %s nr %d, niepoprawny parametr pozycjonowania; spodziewane jedno z "%s"; znaleziono "%s"]|<bif>,<argnumber>,<value>,<value>
 40,930,[RXQUEUE, funkcja TIMEOUT, spodziewano liczby caàkowitej z zakresu od -1 do %d; znaleziono \"%s\"]|<value>,<value>
 40,980,Niespodziewana wartoòÜ na wejòciu, albo nieznany typ albo niedozwolone dane%s%s|:,<location>
 40,981,Liczba poza dozwolonym zakresem%s%s|:,<location>
 40,982,ùa‰cuch za duæy dla zdefiniowanego bufora%s%s|:,<location>
 40,983,Niepoprawna kombinacja typ/rozmiar%s%s|:,<location>
 40,984,Niewspierana liczba jak NAN, +INF, -INF%s%s|:,<location>
 40,985,Struktura zbyt zàoæona dla statycznego, wewn©trznego bufora%s%s|:,<location>
 40,986,Brak elementu struktury%s%s|:,<location>
 40,987,Brak wartoòci struktury%s%s|:,<location>
 40,988,Nazwa b•d´ jej cz©òÜ jest niedozwolona dla interpretera%s%s|:,<location>
 40,989,Wyst•pià problem na interfejsie mi©dzy Regin• a GCI%s%s|:,<location>
 40,990,Typ nie speània wymaga‰ dla typ¢w podstawowych (argumenty/zwracana wartoòÜ)%s%s|:,<location>
 40,991,Niepoprawna liczba argument¢w%s%s|:,<location>
 41,  0,Niepoprawna konwersja arytmetyczna
 41,  1,Nieliczbowa wartoòÜ ("%s") lewej strony operacji arytmetycznej "%s"|<value>,<operator>
 41,  2,Nieliczbowa wartoòÜ ("%s") prawej strony operacji arytmetycznej "%s"|<value>,<operator>
 41,  3,Nieliczbowa wartoòÜ ("%s") uæyta z przedrostkiem opertora "%s"|<value>,<operator>
 41,  4,WartoòÜ wyraæenia TO w instrukcji DO musi byÜ liczbow•; znaleziono "%s"|<value>
 41,  5,WartoòÜ wyraæenia BY w instrukcji DO musi byÜ liczbow•; znaleziono "%s"|<value>
 41,  6,WartoòÜ wyraæenia zmiennej steruj•cej w instrucji DO musi byÜ liczbow•; znaleziono "%s"|<value>
 41,  7,Wykàadnik przekracza %d cyfr; znaleziono "%s"|#Limit_ExponentDigits,<value>
 42,  0,Nadmiar/niedomiar arytmetyczny
 42,  1,Wykryto nadmiar arytmetyczny w "%s %s %s"; wyraziciel rezultatu wymaga wi©cej niæ %d cyfr|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  2,Wykryto niedomiar arytmetyczny w "%s %s %s"; wyraziciel rezultatu wymaga wi©cej niæ %d cyfr|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  3,Nadmiar arytmetyczny; dzielnik nie moæe byÜ zerem
 43,  0,Nie znaleziono podprogramu
 43,  1,Nie moæna odnale´Ü podprogramu "%s"|<name>
 44,  0,Funkcja nie zwr¢ciàa danych
 44,  1,Brak danych zwr¢conych przez funkcj© "%s"|<name>
 45,  0,Brak danych wyspecyfikowanych przy RETURN funkcji
 45,  1,Spodziewano danych z instrukcj• RETURN, poniewaæ podprogram "%s" zostaà wywoàany jako funkcja|<name>
 46,  0,Niepoprawne odniesienie do zmiennej
 46,  1,Znaleziono dodatkowy skàadnik ("%s") w odniesieniu zmiennej; ")" spodziewano|<token>
 47,  0,Niespodziewana etykieta
 47,  1,Dane INTERPRET nie mog• zawieraÜ etytkiet; znaleziono "%s"|<name>
 48,  0,Bà•d w usàudze systemowej
 48,  1,Bà•d w usàudze systemowej: %s|<description>
 48,920,Niskopoziomowy bà•d strumienia I/O: %s %s: %s|<description>,<stream>,<description>
 49,  0,Bà•d interpretacji
 49,  1,Bà•d interpretacji: Bà•d w %s, linia %d: "%s". Prosz© zgàosiÜ ten bà•d!|<module>,<linenumber>,<description>
 50,  0,Nierozpoznany zastrzeæony symbol
 50,  1,Nierozpoznany zastrzeæony symbol "%s"|<token>
 51,  0,Niepoprawna nazwa funkcji
 51,  1,Nazwy funkcji nie uj©te w cudzysà¢w nie mog• ko‰czyÜ si© kropk•; znaleziono "%s"|<token>
 52,  0,Wynik zwr¢cony przez "%s" jest dàuæszy niæ %d znak(i/¢w)|<name>,#Limit_String
 53,  0,Niepoprawna opcja
 53,  1,ùa‰cuch lub symbol oczekiwany po sàowie kluczowym STREAM; znaleziono "%s"|<token>
 53,  2,Po sàowie kluczowym STEM oczekiwane odniesienie do zmiennej; znaleziono "%s"|<token>
 53,  3,Argument do STEM musi mieÜ jedn• kropk© jako ostatni znak; znaleziono "%s"|<name>
 53,100, ùa‰cuch lub symbol spodziewane po sàowie kluczowym LIFO; znaleziono "%s"|<token>
 53,101,ùa‰cuch lub symbol spodziewane po sàowie kluczowym FIFO; znaleziono "%s"|<token>
 54,  0,Niepoprawna wartoòÜ STEM
 54,  1,Dla tego STEM APPEND, wartoòÜ "%s" musi byÜ liczb• linii; znaleziono "%s"|<name>,<value>
#
# Wszystkie poniæsze komunikaty nie s• zdefiniowane przez ANSI
#
 60,  0,[Nie moæna cofn•Ü kursora pliku tymczasowego]
 61,  0,[Niepoprawna operacja pozycjonowania na pliku]
 64,  0,[Bà•d skàadni podczas rozbioru gramatycznego]
 64,  1,[Bà•d skàadni w linii %d]
 64,  2,[Podstawowy bà•d skà•dni w linii %d, kolumna %d]|<linenumber>,<columnnumber>
 90,  0,[Opcja niezgodna z ANSI uæyta przy "OPTIONS STRICT_ANSI"]
 90,  1,[%s jest funkcj• BIF rozszerzenia Regina'y]|<bif>
 90,  2,[%s jest instrukcj• rozszerzenia Regina'y]|<token>
 90,  3,[Argument %s nr %d, Przy "OPTIONS STRICT_ANSI" opcja musi zaczynaÜ si© jednym z "%s; znaleziono "%s"; rozszerzenie Regina'y]|<bif>,<argnumber>,<optionslist>,<value>
 93,  0,[Niepoprawne wywoàanie procedury]
 93,  1,[Po komendzie STREAM %s musi wyst©powaÜ jedna z "%s"; znaleziono "%s"]|<token>,<value>,<value>
 93,  3,[Komenda STREAM musi byÜ jedn• z "%s"; znaleziono "%s"]|<value>,<value>
 94,  0,[Bà•d interfejsu zewn©trznej kolejki]
 94, 99,[Wewn©trzny bà•d z interfejsem zewn©trznej kolejki: %d "%s"]|<description>,<systemerror
 94,100,[Podstawowy bà•d systemu z interfejsem zewn©trznej kolejki. %s. %s]|<description>,<systemerror>
 94,101,[Bà•d podczas à•czenia si© z %s na porcie %d: "%s"]|<machine>,<portnumber>,<systemerror>
 94,102,[Nie moæna uzyskaÜ adresu IP dla %s]|<machine>
 94,103,[Niepoprawny format dla serwera w podanej nazwie kolejki: "%s"]|<queuename>
 94,104,[Niepoprawny format nazwy kolejki: "%s"]|<queuename>
 94,105,[Nie moæna uruchomiÜ interfejsu gniazd systemu Windows: %s]|<systemerror>
 94,106,[Przekroczono maksymaln• liczb© zewn©trznych kolejek: %d]|<maxqueues>
 94,107,[Wyst•pià bà•d podczas czytania z gniazda: %s]|<systemerror>
 94,108,[Przekazano niepoprawny przeà•cznik. Musi byÜ jednym z "%s"]|<switch>
 94,109,[Kolejka \"%s\" nie znaleziona]|<queuename>
 94,110,[%s niepoprawna dla zewn©trznych kolejek]|<bif>
 94,111,[Funkcja RXQUEUE %s niepoprawna dla wewn©trznych kolejek]|<functionname>
 95,  0,[Zastrzeæona opcja uæyta w trybie "bezpiecznym"]
 95,  1,[%s niedozwolony w trybie "bezpiecznym"]|<token>
 95,  2,[%s argument %d niedozwolony w trybie "bezpiecznym"]|<bif>,<argnumber>
 95,  3,[Argument %s nr %d: "%s", niedozwolony w trybie "bezpiecznym"]|<bif>,<argnumber>,<token>
 95,  4,[3 argument STREAM: Otwieranie plik¢w w trybie WRITE jest niepoprawne w trybie "bezpiecznym"]
 95,  5,[Uruchamianie polece‰ zewn©trznych jest niedozwolone w trybie "bezpiecznym"]
100,  0,[Nieznany bà•d systemu plik¢w]
#
# Extra general phrases requiring translation:
# The text in () is the corresponding array name in error.c
#
# (err1prefix)   "Bà•d %d podczas dziaàania \"%.*s\", linia %d: %.*s",
# (suberrprefix) "Bà•d %d.%d: %.*s",
# (err2prefix)   "Bà•d %d podczas dziaàania \"%.*s\": %.*s",
# (erropen)      "Nie moæna otworzyÜ pliku j©zyka: %s",
# (errcount)     "Niepoprawna liczba komunikat¢w w pliku j©zyka: %s",
# (errread)      "Nie moæna czytaÜ z pliku j©zyka: %s",
# (errmissing)   "Brakuje tekstu w pliku j©zyka: %s.mtb",
# (errcorrupt)   "Plik j©zyka: %s.mtb jest znieksztaàcony",
