#
# Regina error messages - Turkish
# Written by Haluk POLAT <bilgi@halukpolat.com>
#
# Dutch - Gert van der Kooij <geko@wanadoo.nl>
# French - Mark <cotemark@globetrotter.net>
# German - Florian Grosse-Coosmann <florian@grosse-coosmann.de>
# Norwegian (bokmal) - Vidar Tysse <vtsystem@bgnett.no>
# Polish - Bartosz Kozlowski <bk@bn.pl>
# Portuguese - Brian Carpenter <bcarpent@nucleus.com>
# Russian - Oleg Kulikov <kulikov@xanadu.ru>
# Spanish - Pablo Garcia-Abia <Pablo.Garcia@cern.ch>
# Swedish - Jan-Erik L„rka <jan-erik@os2ug.se>
# Turkish - Haluk POLAT <bilgi@halukpolat.com>
#
# Guidelines for Translators
# --------------------------
# - Lines beginning with # are comments
# - Any word in UPPER CASE should not be translated
# - On some error message lines, there are tokens following the
#   | character. Again these should not be translated.
# - Please ensure that the order of the substitutions; ie the %s, %d
#   placemarkers are maintained. If this is gramatically impossible,
#   please let me know.
# - There are some comments at the end of this file under the heading:
#   "Extra general phrases requiring translation:"
#   Please translate these phrases also.
# - Return the translated error messages in a complete file, the same as this
#   file, with the name XX.mts where XX is the ISO language abbreviation.
#   Important: Please zip the file up to ensure that the file arrives in
#              its original form
#   Add a comment at the top of the file indicating the language, and your name and email address.
#   If you do not want your email address published in the Regina documentation
#   please indicate this. I will retain your address as a contact for future
#   individual message translations, but will only publish your name.
# - And last but by no means least; thanks!
#
# TODO
# 40.44
#
  0,  1,%s kaynaginda %s hatasi, satir no %d:|<value>,<source>,<linenumber>
  0,  2,Etkilesimli izlemede %s hatasi:|<value>
  0,  3,Etkilesimli izleme.  Izlemeyi sonlandirmak icin "Trace Off" kullaniniz. Devam etmek icin ENTER tusuna basiniz.
  2,  0,Sonuclandirma esnasindaki hata
  2,  1,Sonuclandirma esnasindaki hata: %s|<description>
  3,  0,Baslatma esnasindaki hata
  3,  1,Baslatma esnasindaki hata: %s|<description>
  4,  0,Izlence yarida kesildi
  4,  1,Izlence HALT durumu ile yarida kesildi: %s|<description>
  5,  0,Dizge kaynaklari tukendi
  5,  1,Dizge kaynaklari tukendi: %s|<description>
  6,  0,Karsiligi olmayan "/*" dizgisi veya cift-tirnak isareti
  6,  1,Karsiligi olmayan aciklama satiri sonlandiricisi ("/*")
  6,  2,Karsiligi olmayan tek-tirnak isareti (')
  6,  3,Karsiligi olmayan cift-tirnak isareti (")
  7,  0,WHEN veya OTHERWISE anahtar kelimesi belirtilmemis
  7,  1,%d numarali satirdaki SELECT ifadesi WHEN ifadesini gerektirir; bulunan "%s"|<linenumber>,<token>
  7,  2,%d numarali satirdaki SELECT ifadesi WHEN, OTHERWISE veya END ifadesine gerektirir; bulunan "%s"|<linenumber>,<token>
  7,  3,%d numarali satirdaki SELECT ifadesine ait tum WHEN ifadeleri yanlistir; OTHERWISE ifadesi belirtilmis|<linenumber>
  8,  0,THEN veya ELSE belirtilmemis
  8,  1,THEN ifadesine karsilik gelen IF veya WHEN cumlecigi yok
  8,  2,ELSE ifadesine karsilik gelen THEN cumlecigi yok
  9,  0,Belirtilmemis WHEN veya OTHERWISE
  9,  1,WHEN ifadesine karsilik gelen SELECT yok
  9,  2,OTHERWISE ifadesinin karsiligi olan SELECT yok
 10,  0,Belirtilmemis veya karsiligi olmayan END
 10,  1,END ifadesinin karsiligi DO veya SELECT yok
 10,  2,%d numarali satirdaki DO ifadesinin karsiligi olan END ifadesi denetim degiskenine (veya sembol olmayan bir degiskene) karsilik gelen bir sembole sahip olmalidir; bulunan "%s"|<linenumber>,<token>
 10,  3,%d numarali satirdaki DO ifadesinin karsiligi olan END ifadesi denetim degiskeni olmayan bir sembole sahip olmamalidir; bulunan "%s"|<linenumber>,<token>
 10,  4,%d numarali satirdaki SELECT ifadesinin karsiligi olan END ifadesi devamindaki bir sembole sahip olmamalidir; bulunan "%s"|<linenumber>,<token>
 10,  5,END ifadesi dogrudan dogruya THEN ifadesinin devaminda olamaz
 10,  6,END ifadesi dogrudan dogruya ELSE ifadesinin devaminda olamaz
 11,  0,[Denetim yigiti doludur]
 12,  0,[1024 karakterden fazla karakter iceren cumlecik olamaz]
 13,  0,Izlenceniz (betiginiz) icerisinde gecersiz bir karakter var
 13,  1,Izlenceniz (betiginiz) icerisinde gecersiz bir karakter var "('%x'X)"|<hex-encoding>
 14,  0,Tamamlanmis DO/SELECT/IF obegi
 14,  1,DO ifadesi karsilik gelen bir END ifadesine gereksinim duyar
 14,  2,SELECT ifadesi karsilik gelen bir END ifadesine gereksinim duyar
 14,  3,THEN ifadesi devaminda bir buyruga gereksinim duyar
 14,  4,ELSE ifadesi devaminda bir buyruga gereksinim duyar
 15,  0,Gecersiz onaltili sayi ya da ikili sayi turundeki dizgi
 15,  1,Onaltili sayi turundeki dizginin %d numarali konumunda gecersiz bosluk yeri|<position>
 15,  2,Ikili sayi turundeki dizginin %d numarali konumunda gecersiz bosluk yeri|<position>
 15,  3,Bir onaltili sayi turundeki dizgide yalnizca 0-9, a-f, A-F ve bosluk karakteri bulunabilir; bulunan "%c"|<char>
 15,  4,Bir ikili sayi turundeki dizgide yalnizca 0, 1 ve bosluk karakteri bulunabilir; bulunan "%c"|<char>
 16,  0,Etiket bulunamadi
 16,  1,"%s" isimli etiket bulunamadi|<name>
 16,  2,Bir IF, SELECT veya DO obeginin icerisinde bulunan "%s" isimli etikete SIGNAL ile gidilemez|<name>
 16,  3,Bir IF, SELECT veya DO obeginin icerisinde bulunan "%s" isimli etikete basvuru yapilamaz|<name>
 17,  0,PROCEDURE anahtar kelimesi belirtilmemis
 17,  1,PROCEDURE anahtar kelimesi bir dahili alt-yordam veya islev cagirisindan sonraki ilk buyruk oldugunda gecerlidir
 18,  0,THEN anahtar kelimesi belirtilir
 18,  1,%d numarali satirdaki IF anahtar kelimesi karsilik gelen THEN cumlecigine gereksinim duyar; bulunan "%s"|<linenumber>,<token>
 18,  2,%d numarali satirdaki WHEN anahtar kelimesi karsilik gelen THEN cumlecigine gereksinim duyar; bulunan "%s"|<linenumber>,<token>
 19,  0,Dizgi veya sembol belirtilmesi beklenir
 19,  1,ADDRESS anahtar kelimesinden sonra dizgi veya sembol belirtilmesi beklenir; bulunan "%s"|<token>
 19,  2,CALL anahtar kelimesinden sonra dizgi veya sembol belirtilmesi beklenir; bulunan "%s"|<token>
 19,  3,NAME anahtar kelimesinden sonra dizgi veya sembol belirtilmesi beklenir; bulunan "%s"|<token>
 19,  4,SIGNAL anahtar kelimesinden sonra dizgi veya sembol belirtilmesi beklenir; bulunan "%s"|<token>
 19,  6,TRACE anahtar kelimesinden sonra dizgi veya sembol belirtilmesi beklenir; bulunan "%s"|<token>
 19,  7,Oruntu parcalama isleminde sembol belirtilmesi beklenir; bulunan "%s"|<token>
 20,  0,Isim belirtilmesi beklenir
 20,  1,Isime gereksinim duyulmaktadir; bulunan "%s"|<token>
 20,  2,Yalniz bir isimin gecerli oldugu yerde "%s" bulundu|<token>
 21,  0,Cumlecigin sonunda gecersiz veri var
 21,  1,Beklenmeyen bir belirtimde sonlandirilmis cumlecik; bulunan "%s"|<token>
 22,  0,Gecersiz karakter katari (dizgi)
 22,  1,Gecersiz karakter katari (dizgi) '%s'X|<hex-encoding>
 23,  0,Gecersiz veri katari
 23,  1,Gecersiz veri katari '%s'X|<hex-encoding>
 24,  0,Gecersiz TRACE istegi
 24,  1,TRACE istek harfi "%s" ifadesi gibi bir sey olmalidir; bulunan "%c"|ACEFILNOR,<value>
 25,  0,Gecersiz alt-anahtar-kelime bulundu
 25,  1,CALL ON ifadesi %s anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25,  2,CALL OFF ifadesi %s anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25,  3,SIGNAL ON ifadesi %s anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25,  4,SIGNAL OFF ifadesi %s anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25,  5,ADDRESS WITH ifadesi INPUT, OUTPUT veya ERROR anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<token>
 25,  6,INPUT ifadesi STREAM, STEM, LIFO, FIFO veya NORMAL anahtar kelimelerinden bir tanesi ile devam etmelidir; bulunan "%s"|<token>
 25,  7,OUTPUT ifadesi STREAM, STEM, LIFO, FIFO, APPEND, REPLACE veya NORMAL anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<token>
 25,  8,APPEND ifadesi STREAM, STEM, LIFO veya FIFO anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<token>
 25,  9,REPLACE anahtar kelimesi STREAM, STEM, LIFO veya FIFO anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<token>
 25, 11,NUMERIC FORM ifadesi %s anahtar kelimelerinden birisi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25, 12,PARSE ifadesi %s anahtar kelimelerinden bir tanesi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25, 13,UPPER ifadesi %s anahtar kelimelerinden bir tanesi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25, 14,ERROR ifadesi STREAM, STEM, LIFO, FIFO, APPEND, REPLACE veya NORMAL anahtar kelimelerinden bir tanesi ile devam etmelidir; bulunan "%s"|<token>
 25, 15,NUMERIC ifadesi %s anahtar kelimelerinden bir tanesi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25, 16,FOREVER ifadesi %s anahtar kelimelerinden bir tanesi ile devam etmelidir; bulunan "%s"|<keywords>,<token>
 25, 17,PROCEDURE ifadesi EXPOSE anahtar kelimesi devam etmelidir veya devaminda herhangi bir ifade olmamalidir; bulunan "%s"|<token>
 26,  0,Gecersiz tumsel sayi degeri
 26,  1,Tumsel sayilar DIGITS anahtar kelimesiyle yapilan mevcut ayara uygun olmalidir(%d); bulunan "%s"|<value>,<value>
 26,  2,DO buyrugundaki tekrar sayisinin degerinden olusan ifade sifir ya da bir pozitif tumsel sayi olmalidir; bulunan "%s"|<value>
 26,  3,DO buyrugundaki FOR ifadesinin degeri sifir ya da bir pozitif tumsel sayi olmalidir; bulunan "%s"|<value>
 26,  4,Parcalama sablonunun konumsal degistirgesi bir pozitif tumsel sayi olmalidir; bulunan "%s"|<value>
 26,  5,NUMERIC DIGITS belirtiminin degeri sifir ya da bir pozitif tumsel sayi olmalidir; bulunan "%s"|<value>
 26,  6,NUMERIC FUZZ belirtiminin degeri sifir ya da bir pozitif tumsel sayi olmalidir; bulunan "%s"|<value>
 26,  7,TRACE ayarinda kullanilan sayi bir tumsel sayi olmalidir; bulunan "%s"|<value>
 26,  8,Ust alma islecinin ("**") saginda yer alacak islec bir tumsel sayi olmalidir; bulunan "%s"|<value>
 26, 11,%s %% %s isleminin sonucu NUMERIC DIGITS %d ifadesine ait gecerli ustel gosterime gereksinim duyabilir|<value>,<value>,<value>
 26, 12,%s // %s islemi icin kullanilan %% isleminin sonucu NUMERIC DIGITS %d ifadesine ait gecerli ustel gosterime gereksinim duyabilir|<value>,<value>,<value>
 27,  0,DO anahtar kelimesine ait gecersiz soz dizim
 27,  1,DO cumlecigindeki "%s" anahtar kelimesinin kullanimi gecersizdir|<token>
 28,  0,LEAVE veya ITERATE anahtar kelimesine ait gecersiz kullanim
 28,  1,LEAVE anahtar kelimesi yalnizca tekrarli bir DO dongusu icerisinde gecerlidir
 28,  2,ITERATE anahtar kelimesi yalnizca tekrarli bir DO dongusu icerisinde gecerlidir
 28,  3,LEAVE ("%s") ifadesini takip eden sembol ya mevcut DO dongusune ait bir denetim degiskenine karsilik gelmelidir ya da ihmal edilmelidir|<token>
 28,  4,ITERATE ("%s") ifadesini takip eden sembol ya mevcut DO dongusune ait bir denetim degiskenine karsilik gelmelidir ya da ihmal edilmelidir|<token>
 29,  0,Ortam adi cok uzun
 29,  1,Ortam ismi en fazla %d tane karaktere sahip olabilir; bulunan "%s"|#Limit_EnvironmentName,<name>
 30,  0,Isim ya da dizgi cok uzun
 30,  1,Isim en fazla %d tane karaktere sahip olabilir|#Limit_Name
 30,  2,Hazir dizgi en fazla %d tane karaktere sahip olabilir|#Limit_Literal
 31,  0,Isim sayi ile ya da nokta (".") ile baslar
 31,  1,Bir deger bir sayisal degere atanamaz; bulunan "%s"|<token>
 31,  2,Degisken sembolu bir sayi ile baslamamalidir; bulunan "%s"|<token>
 31,  3,Degisken sembolu bir nokta (".") ile baslamamalidir; bulunan "%s"|<token>
 32,  0,[Gecersiz dizinli degisken kullanimi]
 33,  0,Gecersiz ifade sonucu
 33,  1,("%d") seklindeki NUMERIC DIGITS degeri en fazla ("%d") seklindeki NUMERIC FUZZ degerine sahip olmalidir|<value>,<value>
 33,  2,("%d") sekindeki NUMERIC DIGITS degeri %d degerine ulasmamalidir|<value>,#Limit_Digits
 33,  3,Izleyen NUMERIC FORM ifadesinin sonucu "E" veya "S" ile baslamalidir; bulunan "%s"|<value>
 34,  0,Mantiksal deger "0" veya "1" degildir
 34,  1,Izleyen IF anahtar kelimesi ifadesinin degeri kesinlikle "0" veya "1" olmalidir; bulunan "%s"|<value>
 34,  2,Izleyen WHEN anahtar kelimesi ifadesinin degeri kesinlikle "0" veya "1" olmalidir; bulunan "%s"|<value>
 34,  3,Izleyen WHILE anahtar kelimesi ifadesinin degeri kesinlikle "0" veya "1" olmalidir; bulunan "%s"|<value>
 34,  4,Izleyen UNTIL anahtar kelimesi ifadesinin degeri kesinlikle "0" veya "1" olmalidir; bulunan "%s"|<value>
 34,  5,"%s" mantiksal islecinin solundaki ifadenin degeri kesinlikle "0" veya "1" olmalidir; bulunan "%s"|<operator>,<value>
 34,  6,"%s" mantiksal islecinin sagindaki ifadenin degeri kesinlikle "0" veya "1" olmalidir; bulunan "%s"|<operator>,<value>
 35,  0,Gecersiz ifade
 35,  1,"%s" ogesinde gecersiz ifadeyle karsilasildi|<token>
 36,  0,Ifade icerisinde "(" karakterinin karsiligi olan ")" karakteri yok
 37,  0,Belirtilmedigi dusunulen "," veya ")" karakteri
 37,  1,Belirtilmedigi dusunulen "," karakteri
 37,  2,Ifade icerisinde ")" karakterinin karsiligi olan "(" karakteri yok
 38,  0,Gecersiz sablon veya oruntu
 38,  1,"%s" ogesinde gecersiz sablon parcalamasi ile karsilasildi|<token>
 38,  2,"%s" ogesinde gecersiz konum parcalamasi ile karsilasildi|<token>
 38,  3,PARSE VALUE buyrugu WITH anahtar kelimesine gereksinim duyar
 39,  0,[Degerlendirim yigiti dolu]
 40,  0,Dogru olmayan yordam cagrimi
 40,  1,"%s" dissal yordami hatayla karsilasti|<name>
 40,  3,"%s" ogesinin cagrilmasinda yetersiz degistirge var; asgari beklenilen %d tanedir|<bif>,<argnumber>
 40,  4,"%s" ogesinin cagrilmasinda cok fazla sayida degistirge var; asgari beklenilen %d tanedir|<bif>,<argnumber>
 40,  5,"%s" ogesinin cagirilmasinda kayip degistirge var; %d degistirgesine gerek var|<bif>,<argnumber>
 40,  9,%s degistirgesini iceren %d. siradaki ogenin usteli en fazla %d tane haneye ulasmalidir; bulunan "%s"|<bif>,<argnumber>,#Limit_ExponentDigits,<value>
 40, 11,%s degistirgesini iceren %d. siradaki oge bir sayi olmalidir; bulunan "%s"|<bif>,<argnumber>,<value>
 40, 12,%s degistirgesini iceren %d. siradaki oge bir tumsel sayi olmalidir; bulunan "%s"|<bif>,<argnumber>,<value>
 40, 13,%s degistirgesini iceren %d. siradaki oge sifir veya pozitif olmalidir; bulunan "%s"|<bif>,<argnumber>,<value>
 40, 14,%s degistirgesini iceren %d. siradaki oge pozitif olmalidir; bulunan "%s"|<bif>,<argnumber>,<value>
 40, 17,1. %s degistirgesinin tam kismi 0-90 araliginda olmalidir ve ondalik kismi da .9 degerinden buyuk olmamalidir; bulunan "%s"|<bif>,<value>
 40, 18,%s donusumu 0001 ile 9999 arasinda bir yil olmalidir|<bif>
 40, 19,2. %s degistirgesi olan "%s" ogesi 3. degistirge olan "%s" ogesi tarafindan belirlenen bicimde degildir|<bif>,<value>,<value>
 40, 21,%s degistirgesini iceren %d. siradaki oge bos olmamalidir|<bif>,<argnumber>
 40, 23,%s degistirgesini iceren %d. siradaki oge tek bir karakterden olusmalidir; bulunan "%s"|<bif>,<argnumber>,<value>
 40, 24,1. sirada bulunan %s degistirgesi ikili sayidan olusan bir dizgi olmalidir; bulunan "%s"|<bif>,<value>
 40, 25,1. sirada bulunan %s degistirgesi onaltili sayidan olusan bir dizgi olmalidir; bulunan "%s"|<bif>,<value>
 40, 26,1. sirada bulunan %s degistirgesi gecerli bir simge olmalidir; bulunan "%s"|<bif>,<value>
 40, 27,1. sirada bulunan %s degistirgesi gecerli bir akim (dosya) ismi olmalidir; bulunan "%s"|<bif>,<value>
 40, 28,%s degistirgesini iceren %d. siradaki ogenin isaret ettigi secenek "%s" ogelerinden birisi ile baslamalidir; bulunan "%s"|<bif>,<argnumber>,<optionslist>,<value>
 40, 29,%s donusumunun ait oldugu "%s" bicimine izin verilmez|<bif>,<value>
 40, 31,%s degistirgesini iceren 1. siradaki ("%d") ogesi 100000 degerine ulasmamalidir|<bif>,<value>
 40, 32,%s degistirgesini iceren 1. siradaki ("%d") ogesi ile 2. sirada bulunan ("%d") degistirgesi arasindaki fark 100000 degerine ulasmamalidir|<bif>,<value>,<value>
 40, 33,%s degistirgesini iceren 1. siradaki ("%d") ogesi 2. sirada bulunan degistirgenin isaret ettigi ("%d") ogesine esit veya ondan kucuk olmalidir|<bif>,<value>,<value>
 40, 34,%s degistirgesini iceren 1. siradaki ("%d") ogesi izlencedeki toplam satir sayisi olan (%d) degerinden kucuk veya ona esit olmalidir|<bif>,<value>,<sourceline()>
 40, 35,1. siradaki %s degistirgesi bir tumsel sayi olarak ifade edilemez; bulunan "%s"|<bif>,<value>
 40, 36,1. siradaki %s degistirgesi degisken havuzundaki degisken isimlerinden birisi olmalidir; bulunan "%s"|<bif>,<value>
 40, 37,3. siradaki %s degistirgesi bir degisken havuzunun ismi olmalidir; bulunan "%s"|<bif>,<value>
 40, 38,%s degistirgesini iceren %d. siradaki oge "%s" bicimindeki ifade gibi yeteri kadar buyuk degildir|<bif>,<argnumber>,<value>
 40, 39,3. siradaki %s degistirgesi sifir (0) ya da bir (1) degildir; bulunan "%s"|<bif>,<value>
 40, 41,%s degistirgesini iceren %d. siradaki oge, akim (dosya) sinirsal degerleri icerisinde yer almalidir; bulunan "%s"|<bif>,<argnumber>,<value>
 40, 42,1. siradaki %s degistirgesi bu akim (dosya) uzerinde konumlandirilamaz; bulunan "%s"|<bif>,<value>
 40, 43,%s siradaki %d olmayan tek bir alfanümerik karakter veya bos dize olmalidir; bulunan "%s"|<bif>,<argnumber>,<value>
 40, 44,%s argument %d, "%s", is a format incompatible with the separator specified in argument %d|<bif>,<argnumber>,<value>,<argnumber>
 40,914,[%s degistirgesini iceren %d. siradaki oge, "%s" ogelerinden birisi olmalidir; bulunan "%s"]|<bif>,<argnumber>,<optionslist>,<value>
 40,920,[%s: dusuk-seviye akim (dosya) G/C hatasi; %s]|<bif>,<description>
 40,921,[%s degistirgesini iceren %d. siradaki oge, "%s" akimina (dosyasina) ait konumlandirma kipi; akim (dosya) acma kipi ile uyumsuzdur]|<bif>,<argnumber>,<value>
 40,922,[%s degistirgesini iceren %d. siradaki oge birkac taneden fazla alt-buyruk isaret etmektedir; en az %d tane beklenir; bulunan %d]|<bif>,<argnumber>,<value>,<value>
 40,923,[%s degistirgesini iceren %d. siradaki oge bir hayli fazla alt-buyruk isaret etmektedir; en fazla %d tane beklenir; bulunan %d]|<bif>,<argnumber>,<value>,<value>
 40,924,[%s degistirgesini iceren %d ogesi gecersiz konumsal belirtimini isaret etmektedir; "%s" ogelerinden bir tanesinin belirtilmesi beklenir; bulunan "%s"]|<bif>,<argnumber>,<value>,<value>
 40,930,[RXQUEUE uzerinden kullanilan TIMEOUT islevinin 0 ile %d arasindaki bir tumsel sayiyi isaret etmesi beklenir; bulunan \"%s\"]|<value>,<value>
 40,980,Beklenilmeyen giris, ya bilinmeyen tipten ya da kural disi %s%s verisinden kaynaklanmaktadir|: ,<location>
 40,981,Izin verilen %s%s araligin disindaki kadar sayi vardir|: ,<location>
 40,982,%s%s seklinde tanimli ara bolge icin cok buyuk dizgi soz konusu|: ,<location>
 40,983,Kural disi %s%s seklindeli tip ve/veya boyut birlikteligi soz konusu|: ,<location>
 40,984,%s%s ogesi kapsaminda gecerli olan NAN, +INF veya -INF benzeri desteklenmeyen sayi var|: ,<location>
 40,985,Icsel %s%s ara bolgesi icin cok karmasik yapi soz konusu|: ,<location>
 40,986,Yapinin bir elementi %s%s ogesinin isaret ettigi kayip durumdadir|: ,<location>
 40,987,Yapinin bir degeri %s%s ogesinin isaret ettigi kayip durumdadir|: ,<location>
 40,988,Ismin bir parcasi veya tamami %s%s yorumlayicisi icin kural disidir|: ,<location>
 40,989,Regina ve %s%s ismindeki CGI betigi arasindaki arayuzde bir sorun ile karsilasildi|: ,<location>
 40,990,Degistirgeler ve/veya donus degerleri gibi %s%s ogesinin isaret ettigi basit tipler icin gereksinimleri duzenleyecek (uygun olacak) bir tip soz konusu degildir|: ,<location>
 40,991,Degistirgelerin sayisi yanlis veya bir degistirgesi %s%s gibi kayipdir|: ,<location>
 40,992,Degistirgeler icin gecerli olan GCI ogesinin icsel yigitinda tasmaoldu %s%s|: , <location>
 40,993,GCI ogesi cok sayida ic ice gecmis LIKE tasiyicisi saydi %s%s|: ,<location>
 41,  0,Kotu aritmetiksel donusum soz konusu
 41,  1,"%s" aritmetik islecinin sol tarafinda ("%s") seklindeki sayisal olmayan degerle karsilasildi|<value>,<operator>
 41,  2,"%s" aritmetik islecinin sag tarafinda ("%s") seklindeki sayisal olmayan degerle karsilasildi|<value>,<operator>
 41,  3,"%s" on ek isleci ile ("%s") seklindeki sayisal olmayan deger kullanilir|<value>,<operator>
 41,  4,DO buyrugundaki TO ifadesinin degeri sayisal olmak zorundadir; bulunan "%s"|<value>
 41,  5,DO buyrugundaki BY ifadesinin degeri sayisal olmak zorundadir; bulunan "%s"|<value>
 41,  6,DO buyruguna ait denetim degiskeni ifadesinin degeri sayisal olmak zorundadir; bulunan "%s"|<value>
 41,  7,Ustsel deger %d tane haneye ulasir; bulunan "%s"|#Limit_ExponentDigits,<value>
 42,  0,Aritmetiksel tasma/azalma
 42,  1,"%s %s %s" ogesinde aritmetiksel tasma ile karsilasildi; sonucun ustel degeri %d tane basamaktan daha fazlasina gereksinim duyar|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  2,"%s %s %s" ogesinde aritmetik azalma ile karsilasildi; sonucun ustel degeri %d tane basamaktan daha fazlasina gereksinim duyar|<value>,<operator>,<value>,#Limit_ExponentDigits
 42,  3,Aritmetiksel tasma; bolucu sifir olmamalidir
 43,  0,Yordam mevcut degil
 43,  1,"%s" ismindeki yordam bulunamadi|<name>
 44,  0,Islev bir deger geri gondermedi
 44,  1,"%s" isimli islevden donen bir deger mevcut degil|<name>
 45,  0,Isleve ait RETURN anahtar kelimesinden sonra bir veri belirtilmemis
 45,  1,"%s" yordami bir islev olarak cagrildigindan dolayi RETURN anahtar kelimesiyle donecek bir veri degerinin belirtilmesi beklenir|<name>
 46,  0,Gecersiz degisken basvurusu
 46,  1,Degisken basvurusunda "%s" adindaki ekstra oge ile karsilasildi; ")" karakterinin belirtilmesi beklenir|<token>
 47,  0,Beklenilmeyen etiket soz konusu
 47,  1,INTERPRET anahtar kelimesine ait veri etiketleri icermemelidir; bulunan "%s"|<name>
 48,  0,Dizge hizmetinde hata ile karsilasildi
 48,  1,Dizge hizmetinde karsilasilan hata: %s|<description>
 48,920,Dusuk seviye akim (dosya) G/C hatasi: %s %s: %s|<description>,<stream>,<description>
 49,  0,Yorumlama Hatasi
 49,  1,Yorumlama Hatasi: %s ogesinin %d numarali satirinda karsilasilan hata: "%s". Lutfen hatayi bildiriniz !|<module>,<linenumber>,<description>
 50,  0,Taninmayan ayrilmis sembol
 50,  1,"%s" adinda taninmayan ayrilmis sembol|<token>
 51,  0,Gecersiz islev ismi
 51,  1,Tirnak isaretleri arasinda belirtilmemis islev isimleri bir tane nokta ile sonlanmamalidir; bulunan "%s"|<token>
 52,  0,"%s" ogesinden donen sonuc %d tane karakterden buyuk olamaz|<name>,#Limit_String
 53,  0,Gecersiz secenek
 53,  1,STREAM anahtar kelimesinden sonra dizgi ya da simge belirtilmesi beklenir; bulunan "%s"|<token>
 53,  2,STEM anahtar kelimesinden sonra degisken basvurusunun belirtilmesi beklenir; bulunan "%s"|<token>
 53,  3,STEM ogesine ait degistirgenin son karakteri bir tane nokta isareti olmalidir; bulunan "%s"|<name>
 53,100,LIFO anahtar kelimesinden sonra dizgi veya sembol belirtilmesi beklenir; bulunan "%s"|<token>
 53,101,FIFO anahtar kelimesinden sonra dizgi veya sembol belirtilmesi beklenir; bulunan "%s"|<token>
 54,  0,Gecersiz STEM degeri soz konusu
 54,  1,Bu STEM APPEND ogesi icin "%s" degeri satirlarin toplam sayisi olmalidir; bulunan "%s"|<name>,<value>
#
# All error messages after this point are not defined by ANSI
#
 60,  0,[Gecisli dosyanin basina gidilemez]
 61,  0,[Dosya uzerinde uygunsuz arastirma islemi]
 64,  0,[Parcalama yaparken soz dizim hatasi olustu]
 64,  1,[%d numarali satirda soz dizim hatasi]
 64,  2,[%d numarali satirin %d numarali dikecinde genel soz dizim hatasi]|<linenumber>,<columnnumber>
 90,  0,["OPTIONS STRICT_ANSI" ile kullanilan ANSI olmayan ozellik soz konusu]
 90,  1,[%s ogesi bir Regina uzanti BIF elemanidir]|<bif>
 90,  2,[%s ogesi bir Regina uzanti buyrugudur]|<token>
 90,  3,[%s degistirgesini iceren %d. siradaki ogenin isaret ettigi senecek "OPTIONS_STRICT_ANSI" ile belirtilmis "%s" ogelerinden birisi ile baslamalidir; bulunan "%s"; bir Regina uzantisidir]|<bif>,<argnumber>,<optionslist>,<value>
 90,  4,[%s ogesi bir Regina uzanti islecinin]|<token>
 93,  0,[Dogru olmayan yordam cagrimi]
 93,  1,[%s seklindeki STREAM buyrugu "%s" ogelerinin bir tanesi tarafindan takip edilmelidir; bulunan "%s"]|<token>,<value>,<value>
 93,  3,[STREAM buyrugu "%s" ogelerinden bir tanesi olmalidir; bulunan "%s"]|<value>,<value>
 94,  0,[Dissal kuyruk arayuzu hatasi soz konusu]
 94,  1,[External queue timed out]
 94, 99,[Dissal kuyruk arayuzu ile icsel hata soz konusu: %d "%s"]|<description>,<systemerror>
 94,100,[Dissal kuyruk arayuzu ile genel dizge hatasi soz konusu. %s. %s]|<description>,<systemerror>
 94,101,[%s ogesini iceren %d kapiya yapilan baglantida hata ile karsilasildi: "%s"]|<machine>,<portnumber>,<systemerror>
 94,102,[%s ogesi icin IP bulunaginin elde edilmesinde sorun ile karsilasildi]|<machine>
 94,103,[Belirtilmis kuyruk ismindeki sunucu icin gecersiz bicim soz konusu: "%s"]|<queuename>
 94,104,[Kuyruk ismi icin gecersiz bicim soz konusu: "%s"]|<queuename>
 94,105,[Windows yuva arayuzunun baslatilmasinda hata(lar) ile karsilasildi: %s]|<systemerror>
 94,106,[Dissal kuyruklarin azami sayisinin ulastigi deger: %d]|<maxqueues>
 94,107,[Yuva okuma isleminde hata(lar) ile karsilasildi: %s]|<systemerror>
 94,108,[Gecersiz anahtar gecirildi. "%s" ogelerinden bir tanesi olmalidir]|<switch>
 94,109,[\"%s\" kuyrugu bulunamadi]|<queuename>
 94,110,[%s ogesi dissal kuyruklar icin gecersizdir]|<bif>
 94,111,[%s ismindeki RXQUEUE islevi icsel kuyruklar icin gecersizdir]|<functionname>
 95,  0,["guvenli" kipte sinirli ozellik kullanilir]
 95,  1,[%s ogesi "guvenli" kipte gecersizdir]|<token>
 95,  2,[%s degistirgesini iceren %d. siradaki oge "guvenli" kipte gecersizdir]|<bif>,<argnumber>
 95,  3,[%s degistirgesini iceren %d. siradaki "%s" ogesi "guvenli" kipte gecersizdir]|<bif>,<argnumber>,<token>
 95,  4,[3. siradaki STREAM degistirgesi: WRITE erisimi icin dosyalarin acilmasi "guvenli" kipte gecersizdir]
 95,  5,["guvenli" kipte dissal buyruklarin calistirilmasi gecersizdir]
100,  0,[Bilinmeyen dosya dizgesi hatasi soz konusu]
#
# Extra general phrases requiring translation:
# The text in () is the corresponding array name in error.c
P,  0,%d hatasi "%.*s" calisiyorken %d numarali satirda olustu: %.*s
P,  1,Hata %d.%d: %.*s
P,  2,%d hatasi "%.*s" calisiyorken olustu: %.*s
#
# (err1prefix)   "%d hatasi \"%.*s\" calisiyorken %d numarali satirda olustu: %.*s",
# (suberrprefix) "Hata %d.%d: %.*s",
# (err2prefix)   "%d hatasi \"%.*s\" calisiyorken olustu: %.*s",
# (erropen)      "Dil dosyasinin acilmasinda hata ile karsilasildi: %s",
# (errcount)     "Dil dosyasindaki iletilerin sayisinda yanlislik var: %s",
# (errread)      "Dil dosyasinin okunmasinda hata ile karsilasildi: %s",
# (errmissing)   "Dil dosyasinda eksik metin var: %s.mtb",
# (errcorrupt)   "%s.mtb ismindeki dil dosyasinda kirilma hatasi var",
