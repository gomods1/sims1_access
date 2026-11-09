---
title: Instrukcja modułu dostępności The Sims
lang: pl
toc-title: Spis treści
---

Moduł dostępności The Sims sprawia, że w The Sims Legacy Collection można grać tylko z czytnikiem
ekranu i klawiaturą. Odczytuje każdy ekran, okno dialogowe i panel przez Twój czytnik ekranu
(NVDA, JAWS, Narrator i inne), pozwala poruszać się po nich wszędzie tymi samymi klawiszami i
dodaje narzędzia, których potrzebuje niewidomy gracz: listę obiektów na parceli, mapę domu,
narratora tego, co gra tylko pokazuje, miejsca, w których mieszczą się meble, oraz kursor do
budowania ścian i pokojów.

Tę instrukcję otwiera w grze **Control+Shift+F1**, w języku gry. **Control+F1** mówi klawisze
ekranu, na którym jesteś.

## Pierwsze kroki

1. Uruchom instalator albo skopiuj `version.dll` i folder `SimsAccess` do folderu gry
   (tego, w którym jest `Sims.exe`).
2. Uruchom czytnik ekranu, a potem The Sims Legacy Collection ze Steama.
3. Gdy otworzy się okno gry, usłyszysz "Moduł dostępności The Sims wczytany". Gra zaczyna się na
   ekranie sąsiedztwa.

Mod mówi w języku gry. Aby go zmienić, otwórz Steam, potem Właściwości gry, Ogólne, Język. Mod
mówi po angielsku, francusku, niemiecku, włosku, hiszpańsku, niderlandzku, duńsku, szwedzku,
norwesku, fińsku, po portugalsku (brazylijski), japońsku, polsku, chińsku (uproszczony i
tradycyjny), tajsku i koreańsku.

Mod działa z aktualną wersją gry ze Steama. Jeśli gra się zaktualizuje, a mod powie "nieobsługiwana
wersja gry", zainstaluj następną wersję moda.

## Poruszanie się: wszędzie te same klawisze

Każdy ekran, okno dialogowe, panel i tryb działa tak samo. Poruszasz się po pierścieniu
**grup** (przyciski okna dialogowego, potrzeby, rodzina, katalog...). Grupa zawiera **elementy**,
a niektóre elementy otwierają głębszy **poziom**.

| Klawisz | Co robi |
|---|---|
| Tab, Shift+Tab | Następna lub poprzednia grupa. Najpierw padają nazwa grupy i liczba elementów. |
| Strzałka w górę, strzałka w dół (lub strzałka w lewo, strzałka w prawo) | Poprzedni lub następny element grupy. W katalogach trybu kupowania i budowania strzałka w lewo i strzałka w prawo przewracają zamiast tego stronę. |
| Home, End | Pierwszy lub ostatni element grupy. |
| Enter | Aktywuje element. Na poziomie (rodzaj lub pokój obiektów na parceli) otwiera go. |
| Spacja | Aktywuje, z wyjątkiem widoku parceli, gdzie Spacja to klawisz gry "następny Sim". |
| Backspace | Wraca o poziom wyżej. |
| R | Powtarza element razem z jego grupą. |
| Escape | Zamyka menu działań; poza nim to Escape samej gry. |
| Alt+litera | Następna grupa, której nazwa zaczyna się na tę literę. |
| Control+F1 | Klawisze ekranu, na którym jesteś. |
| Control+Shift+F1 | Ta instrukcja. |

Elementy sterujące mówią po nazwie, czym są, tak jak czytnik ekranu: "Zapisz, przycisk", "Opcje
dźwięku, karta, wybrana", "Autocentrowanie, pole wyboru, zaznaczone", "F/X, suwak, 8 z 10",
"Szczegóły terenu, selektor, Wysoka".

- **Suwaki, selektory i paski osobowości**: strzałka w lewo i strzałka w prawo (lub Page Up i
  Page Down) zmieniają wartość. Enter tylko ją powtarza.
- **Pola wyboru**: Enter zaznacza je i odznacza.
- **Pola tekstowe** (imiona i nazwiska rodzin i Simów, charakterystyki): przejdź strzałkami na
  pole i pisz. Każda litera jest powtarzana, a Backspace mówi, co usuwa. Strzałka w lewo,
  strzałka w prawo, Home, End i Delete poruszają się w polu. Gdy na ekranie jest pole tekstowe,
  litery, Spacja i R służą do pisania.
- **Wyłączone elementy** mówią "niedostępny" i nie są aktywowane.

Ostatnia grupa każdego ekranu to **pomoc moda**: jej przyciski mówią klawisze danego ekranu (jak
Control+F1) i otwierają tę instrukcję (jak Control+Shift+F1). Po uruchomieniu gry mod mówi oba
klawisze po komunikacie "Moduł dostępności The Sims wczytany".

## Więcej informacji: bufory przeglądania

Nie wszystko mieści się w jednym wypowiedzianym zdaniu. Mod prowadzi dwa bufory, które możesz
czytać wiersz po wierszu:

- **szczegóły**: wszystko o tym, co właśnie padło. Oceny, pokoje i opis produktu; działania,
  pokój, odległość i wartość sprzedaży obiektu na parceli; objaśnienie potrzeby lub umiejętności;
  podpowiedź elementu sterującego i jego miejsce w grupie ("3 z 12, sterowanie").
- **zdarzenia**: co się wydarzyło, od najnowszego: zakupy, sprzedaże, zmiany prędkości, teksty
  okien dialogowych, działania dodane do kolejki, wszystko, co powiedział narrator (ostatnie 200).

| Klawisz | Co robi |
|---|---|
| Control+strzałka w górę | Następny wiersz, głębiej w buforze. |
| Control+strzałka w dół | Z powrotem w stronę początku. |
| Control+strzałka w lewo, Control+strzałka w prawo | Przełącza między buforami. |

## Sąsiedztwo

Główna grupa ekranu sąsiedztwa to **domy**: każdy podaje adres i kto w nim mieszka. Enter na domu
naciska go jak kliknięcie, a potem pojawia się karta lub pytania samej gry. Na domu
Control+strzałka w górę czyta jego szczegóły: to, co powiedziałby przegląd domu z trybu
budowania, według ostatniego zapisu domu (pokoje, czego brakuje, pokoje bez wejścia).
Control+Shift+L mówi to wszystko naraz.

Pozostałe grupy zawierają przyciski gry: tworzenie rodziny, pojemnik na rodziny, numer
sąsiedztwa, opcje, wyjście.

Dom bez rodziny otwiera się tylko w trybie budowania, z nieograniczonymi funduszami; mod mówi
"brak rodziny, nieograniczone fundusze" zamiast kwoty pieniędzy.

## Tworzenie rodziny i Simów

Wszystko na tych ekranach to grupa lub pole:

- Wpisz nazwisko rodziny i imię Sima w ich pola tekstowe.
- **osobowość**: strzałka w lewo i strzałka w prawo dodają lub odejmują punkt każdej cesze ("4 z
  10, pozostało punktów: 6"). Znak zodiaku wynika z punktów.
- **wygląd**: wiek, płeć i skóra oraz strzałki głowy i ciała. Każda głowa i każde ciało są
  opisane ("głowa 3 z 137: blond włosy, bob").
- Charakterystyka też jest polem tekstowym.
- Control+Shift i cyfra (od 1 do 9) na parceli otwierają później kartę Sima, gdzie można
  edytować charakterystykę.

## Parcela: tryb życia

Na parceli HUD jest pierścieniem grup jak każdy ekran. Tab przechodzi przez nie w tej kolejności:

1. **stan**: godzina, fundusze, prędkość gry, przyjaciele.
2. **Potrzeby** wybranego Sima, każda w procentach; Enter mówi, co dana potrzeba oznacza.
3. **rodzina**: Simowie z rodziny; Enter wybiera jednego.
4. **kolejka działań**: co zrobi wybrany Sim; Enter anuluje działanie.
5. **obiekty według rodzaju** i **obiekty według pokoju**: obiekty na parceli (zobacz niżej).
6. **sterowanie**, **panele Sima** (Nastrój, Kontakty, Praca i umiejętności, Osobowość,
   Zainteresowania, Dom) i każdy otwarty przez Ciebie panel.
7. **pomoc moda**.

Na elemencie panelu (potrzeba, umiejętność, praca, ocena domu) Enter odczytuje okienko gry na jego
temat.

Praca podaje godziny i kiedy przyjeżdża samochód. Każda umiejętność podaje swój poziom, a gdy awans wymaga więcej, także ile: "Gotowanie, 2 z 10, następny awans wymaga 4". Menu gry nie mają opcji odejścia z pracy: Sim, który ciągle opuszcza pracę, zostaje zwolniony, a gazeta i komputer oferują inne prace.

### Wybieranie Simów i kamera

| Klawisz | Co robi |
|---|---|
| Spacja | "Następny Sim" gry: następny członek rodziny zostaje wybrany i ogłoszony ("aktywna postać: Isabel, Zjedz"). |
| Control+1 do Control+9 | Wybiera od pierwszego do dziewiątego Sima z rodziny. |
| Control+Shift+1 do Control+Shift+9 | Otwiera kartę tego Sima (charakterystyka). |
| Control+Shift+F | Kamera śledzi wybranego Sima albo jest swobodna. |
| Control+E | Wybrany Sim w jednym zdaniu: pokój, co robi, pilne potrzeby, nastrój. |
| Control+Shift+E | Jak wygląda Sim: jego głowa i ciało ("głowa: czarne włosy, krótkie; ciało: garnitur, czarny"). Sim, na którym jest fokus wśród rzeczy na parceli, a jeśli nie, wybrany Sim. Szczegóły Sima (Control+strzałka w górę na nim) też to podają. |
| Control+T | Podaje każdą nową godzinę zegara ("godzina 14:00") albo przestaje. Przydatne przy najszybszej prędkości. |
| P | Pauza (klawisz gry); mod mówi godzinę i fundusze. 1, 2, 3 ustawiają prędkość. |

Kamera śledzi wybranego Sima, bo dźwięki gry pochodzą z tego, co jest na ekranie. Kiedy
przeglądasz obiekty na parceli, kamera przesuwa się do każdego z nich, a potem wraca do Sima. Przesuwa się powoli, żeby obraz nie pędził (ustawienie `camera_glide`).

Przybliżenie i kierunek kamery są podawane, gdy się zmieniają (plus i minus przybliżają, przecinek i kropka obracają widok), a także w wierszu stanu: "kamera: widok średni, w kierunku północnego zachodu". Kierunek to to, co pokazuje górna krawędź ekranu. Północ moda nie obraca się z kamerą.

### Obiekty na parceli

Wszystko na parceli jest w dwóch grupach: **obiekty według rodzaju** (Simowie, potem kategorie
trybu kupowania: krzesła i kanapy, stoły i szafki, sprzęt AGD..., potem inne obiekty) i **obiekty
według pokoju** (najpierw pokój wybranego Sima, potem pozostałe, każdy z piętrem i wielkością).
Enter na rodzaju lub pokoju wymienia jego obiekty, od najbliższego, jako "Lodówka, kuchnia, w
kierunku północnego wschodu, pól: 6" od wybranego Sima. Backspace wraca do rodzajów lub pokojów.
Nazwy pokojów mod zgaduje na podstawie tego, co w nich jest.

Nie musisz tam przechodzić Tabem: klawisze Page przeglądają obiekty z dowolnego miejsca parceli, w
trybie życia, kupowania i budowania.

| Klawisz | Co robi |
|---|---|
| Page Down, Page Up | Następny lub poprzedni obiekt ostatnio wybranego rodzaju lub pokoju. |
| Shift+Page Down, Shift+Page Up | Następny lub poprzedni rodzaj obiektów. |
| Control+Page Down, Control+Page Up | Następny lub poprzedni pokój (pierwsze naciśnięcie: pokój wybranego Sima). |
| Enter | Działania obiektu (w trybie kupowania: podnosi go, aby go przenieść). Na pozycji z identycznymi rzeczami ("Krzesło składane, 4 takie same"): wymienia je po kolei, od najbliższej; Backspace wraca. |
| Control+Enter | Wysyła wybranego Sima do obiektu. |
| Control+F | Szukaj: wpisz część nazwy ("tel" znajdzie telefon, akcenty nie mają znaczenia). Strzałka w górę i w dół przechodzą przez wyniki, Enter działa na wybranym, Control+Enter posyła tam Sima, Delete usuwa drzwi lub okno, Escape wraca tam, gdzie byłeś. |
| Delete | Na drzwiach lub oknie, w trybie kupowania lub budowania: usuwa je i zwraca pieniądze. |

Sim lub obiekt, nad którym wciąż widać dymek, też to mówi ("Bella, Zjedz, myśli: głód"). Rzecz, do której żaden Sim nie może dojść od wejścia na parcelę, mówi "żaden Sim nie może tam dojść".

Po Simach są **problemy**: to, co wymaga uwagi, czyli zepsute rzeczy, rzeczy do sprzątnięcia, kałuże, śmieci, popiół, ogień i wszystko z muchami. Pusta miska dla zwierzaka też tu jest; na każdej liście miska mówi, czy jest w niej jedzenie. Na końcu są **drzwi, okna i schody**.

### Menu działań

Enter na obiekcie (lub na Simie) otwiera menu działań gry. Strzałka w górę i strzałka w dół
wybierają, Enter wykonuje, Escape cofa o poziom lub zamyka. Pozycje kończące się na "..."
otwierają więcej możliwości. Wybrane działanie zostaje ogłoszone jako dodane do kolejki
("Rozmawiaj, w kolejce"). Gdy menu jest puste, mod mówi dlaczego ("brak działań; tylko dla dzieci:
Baw się").

### Pokoje i dom

| Klawisz | Co robi |
|---|---|
| Control+L | Układ pokoju: wielkość, drzwi i okna według ścian i dokąd prowadzą; co stoi wzdłuż każdej ściany, jest w buforze szczegółów. Mówi też światło pokoju, które liczy się do jego oceny. |
| Control+Shift+L | Przegląd domu: piętra, pokoje, oceny, czego brakuje (toaleta, prysznic, łóżka, lodówka, kuchenka, telefon, czujnik dymu), pokoje bez wejścia, obiekty, do których nikt nie dojdzie, ciemne lub nielubiane pokoje. |

### Mapa domu

**Control+M** włącza mapę: kursor, który przechodzi po domu pole po polu, zaczynając od wybranego
Sima. Każdy krok odtwarza dźwięk podłogi i każdej ściany, płotu, drzwi lub okna wokół.

| Klawisz | Co robi |
|---|---|
| Strzałki | Jedno pole; z Shift pięć. Mówi, co tam stoi i w którą stronę jest zwrócone, pokój i piętro, gdy się zmieniają, ściany, płoty, barierki, drzwi i okna według stron. Ściany, płoty i okna zatrzymują kursor; drzwi go przepuszczają. Z pokojem przychodzi jego światło („światło 64%”: okna i lampy; na zewnątrz światło dzienne), które liczy się do oceny pokoju. |
| Alt+strzałki | To samo, przez ściany. |
| przecinek, kropka | Co mówi każdy krok: wszystko, tylko obiekty, tylko podłogi (po nazwie, "Jasna płytka granitowa"), tylko ściany, tylko pokoje. |
| Home | Wraca do wybranego Sima. |
| Control+Shift+Page Up, Page Down | Piętro wyżej lub niżej. |
| Enter | Menu działań tego, co tam stoi (rzecz na stole lub biurku przed stołem: komputer, a nie jego biurko). |
| Control+Enter | Wysyła tam wybranego Sima. |
| Page Down, Page Up (oraz Shift, Control) | Obiekty na parceli jak zawsze; mapa przechodzi do każdego z nich. |
| R | Wszystko o polu. Control+L układ pokoju. |
| Escape lub Control+M | Zamyka mapę. |

### Narrator

Gra wiele rzeczy pokazuje tylko obrazkami. Mod mówi o nich, gdy się dzieją, i zapisuje je w
buforze zdarzeń:

- **Dymki** nad Simami i obiektami: "Bella myśli: głód", "Mortimer krzyczy: wydalanie", "Bella
  mówi: Obcy", temat, którego drugi Sim nie lubi ("..., nie interesuje się tym"), "relacja się
  pogarsza" i "Kosz na śmieci: muchy".
- **Zablokowane przejścia**: "Bella nie może przejść: na drodze stoi Krzesło".
- **Goście**, którzy przychodzą i odchodzą, duchy, włamywacz, Śmierć, dostawca pizzy, wspólny
  dojazd i szkolny autobus (który samochód trąbi).
- **Zabrane rzeczy**, które wywozi komornik lub kradnie włamywacz, z ich wartością; nadchodzące
  **rachunki**.
- **Obserwacja rodziny**: co wybrany Sim zaczyna robić, na Twoje polecenie lub z własnej woli
  ("Cris, z własnej woli: Usiądź"), kiedy kończy lub przerywa ("Cris przerywa Usiądź na rzecz
  Panikuj; czeka w kolejce"), zmiany pokoju, spadające potrzeby, nastrój zmieniający się na
  czerwony, dzisiejsze zmiany relacji ("Marisa i Cris: +8, teraz 28"; panel relacji gry też
  pokazuje te liczby) oraz pieniądze zapłacone lub otrzymane poza trybem kupowania.
- **Wyjazdy**: dokąd przybyłeś, i "z powrotem w domu".

**Control+N** zmienia, ile jest mówione: wszystko, tylko ostrzeżenia lub nic. Niewypowiedziane
wiersze i tak trafiają do bufora zdarzeń.

**Control+Shift+N** wybiera, czyje rozmowy są mówione (co mówią Simowie, ich interakcje
towarzyskie i zmiany relacji, które przynoszą): każdego, kto rozmawia z twoją rodziną (na
początku), wszystkich na parceli, tylko twojej rodziny albo niczyje.

Dźwięki, które słychać (telefon, dzwonek do drzwi, alarmy), nie są ogłaszane.

Działania z innym Simem podają jego imię, niezależnie od tego, kto je zaczął: "Homer: Przywitaj, z: Elvira".

### Wskazówki pomocy i samouczek

Gdy system pomocy gry ma czekającą wskazówkę, mod mówi "dostępna wskazówka systemu pomocy:
Control+H ją odczyta". **Control+H** ją otwiera: wskazówka zostaje przeczytana albo mod mówi, na co
wskazuje czerwona strzałka gry.

W domu 7 pierwszego sąsiedztwa działa samouczek gry. Każdy krok jest czytany jako wersja tekstu
gry na klawiaturę ("Naciśnij przycisk Zbliżenie: Tab do grupy sterowanie, strzałki do przycisku
Zbliżenie, Enter"), a potem pada, na co wskazuje strzałka. Każdy krok można wykonać klawiaturą.
Pierwszy Escape w czasie samouczka ostrzega; drugi Escape w ciągu czterech sekund go zamyka.

### Pizza i inne usługi

Usługi zamawia się działaniami telefonu. Dostawcę pizzy trzeba przywitać: gdy przyjdzie, przejdź
do obiektów według rodzaju, Simowie, strzałka w dół do niego, Enter, Przywitaj. Jeśli nikt go nie
przywita, odejdzie z pizzą, a pieniądze wrócą.

## Tryb kupowania

F2 otwiera tryb kupowania. Kategorie katalogu to karty, a produkty to przyciski: każdy produkt
podaje nazwę, cenę, główne oceny i opis, a bufor szczegółów zawiera wszystkie jego oceny i pokoje,
do których jest przeznaczony. Przejście za ostatni produkt strony przewraca stronę. Każdy produkt mówi też swój rozmiar w polach, dłuższy bok najpierw („2 na 1 pól”). Pieniądze są podawane w simoleanach, walucie gry.

| Klawisz | Co robi |
|---|---|
| Control+R | Ogranicza katalog do produktów dla jednego rodzaju pomieszczenia (kuchnia, łazienka, sypialnia, salon, jadalnia, gabinet, na zewnątrz) lub wszystkich. |
| Backspace | "Wróć" katalogu. |
| Strzałka w lewo, strzałka w prawo | Poprzednia lub następna strona katalogu; w górę i w dół przechodzą produkt po produkcie. |
| Enter na produkcie | Zaczyna jego ustawianie. |

### Ustawianie obiektu

Gdy trzymasz obiekt, mod ma już znalezione miejsca, w których się mieści **i można go używać**
(sprawdzenie ustawienia przez grę, ściany i drzwi pokoju, miejsce potrzebne Simom do korzystania z
niego). Miejsce podaje swój pokój, obok czego jest i w którą stronę jest zwrócone.

| Klawisz | Co robi |
|---|---|
| Page Down, Page Up (lub Tab) | Miejsca, najlepsze najpierw. |
| Control+Page Down, Control+Page Up | Pokoje, w których są miejsca. |
| Shift+Page Down, Shift+Page Up (lub przecinek, kropka) | Inne kierunki w tym samym miejscu. |
| Control+Shift+Page Down, Page Up | Obok obiektów na parceli, najpierw najbliższych. |
| Strzałki | Zamiast tego przesuwa obiekt o jedno pole (Shift: o pięć), mówiąc, czy się mieści i na co trafia. Ściany, płoty i okna go zatrzymują; drzwi go przepuszczają. |
| Alt+strzałki | To samo, przez ściany. |
| Enter | Kupuje go tam (lub odstawia, przy przenoszeniu). |
| Escape | Anuluje (przenoszony obiekt wraca na miejsce). |
| Delete | Sprzedaje trzymany obiekt; mod podaje zwrot pieniędzy. |
| R | Powtarza miejsce. |

Gdy obiekt nie mieści się tam, gdzie stawiają go strzałki, wiersz kończy się tym, co by
zadziałało: kierunkiem, w którym mieści się na tym samym polu („zmieści się po obróceniu, przodem
w kierunku południa”), a jeśli takiego nie ma, najbliższym polem, na którym się mieści
(„najbliższe miejsce, gdzie się zmieści: w kierunku zachodu, 1 pole, przodem w kierunku północy”).
Przy obiekcie z wielu pól pierwsza strzałka i każdy obrót mówią, na której części jest kursor i
dokąd sięga reszta („kursor na krawędzi od północy, sięga w kierunku południa, 1 pole”); szczegóły
miejsca mówią to samo.

Bufor szczegółów zawiera pole miejsca i miejsca odrzucone przez grę, z powodem.

Miejsca siedziska zaczynają się tam, gdzie coś jest używane na siedząco i nie ma jeszcze siedziska:
"przodem na południe, do używania na siedząco: Komputer". Po ustawieniu obiektu mod mówi, jak
będzie używany: "do używania na siedząco: Komputer", "używany z: Krzesło biurowe", "Simowie mogą
tam dojść" albo "żaden Sim nie może tam dojść".

### Przenoszenie i sprzedawanie

W trybie kupowania przejdź Page Down do obiektu na parceli i naciśnij Enter: zostaje podniesiony i
pojawia się ta sama lista miejsc, zaczynając od pokoju, z którego pochodzi. Pierwsza strzałka
stawia kursor tam, gdzie stał. Identyczne obiekty są wymieniane pojedynczo, a ostatni zakup
mówi "nowy zakup".

## Tryb budowania

F3 otwiera tryb budowania. Jego narzędzia to karty, a ich style to produkty. Po wejściu w tryb
budowania pada przegląd domu (Control+Shift+L go powtarza).

Naciśnięcie narzędzia przenosi do jego modeli, w osobnej grupie o nazwie narzędzia; Tab wraca do narzędzi. Nie masz nic w ręku, dopóki nie naciśniesz Enter na modelu.

### Kursor budowy

Enter na stylu ściany lub płotu uruchamia kursor budowy przy wybranym Simie. Porusza się on po
narożnikach między polami.

| Klawisz | Co robi |
|---|---|
| Strzałki | Jeden narożnik; z Shift pięć. Mówi, gdzie jest, jakie ściany się tam spotykają i co tam stoi. |
| Spacja | Zaznacza początek. Potem kursor mówi odległość od początku i co zbudowałby Enter i ile to kosztuje. |
| Enter | Buduje ścianę od początku do kursora. |
| Shift+Enter | Buduje cztery ściany pokoju między początkiem a kursorem. |
| Control+Enter lub Delete | Usuwa ściany. |
| przecinek, kropka | Wybiera gotowy rozmiar pokoju. |
| Insert | Buduje gotowy pokój z północno-zachodnim narożnikiem przy kursorze. |
| Tab, Shift+Tab (lub Control+Page Down, Page Up) | Skok do następnego lub poprzedniego pokoju. |
| Page Up, Page Down | Piętro wyżej lub niżej. |
| Home | Wraca do wybranego Sima. |
| R | Wszystko: wolne miejsce w każdą stronę, pokój. |
| Escape | Opuszcza kursor. |

Przed budową mod sprawdza nowe ściany. Jeśli zamknęłyby Sima, odcięły obiekt, zamknęły drzwi lub
odcięły część parceli, mówi o tym, a drugi Enter buduje mimo to.

### Podłogi, tapety, baseny i teren

Każde z tych narzędzi ma ten sam kursor na polach (strzałki, Shift o pięć, Home, R, Escape;
Control+F1 podaje szczegóły):

- **Podłogi**: Enter kładzie podłogę na polu albo na prostokącie od początku zaznaczonego Spacją;
  Shift+Enter w całym pokoju; Control+Enter usuwa.
- **Tapety**: Enter tapetuje każdą ścianę pokoju pod kursorem; Control+Enter zdejmuje tapetę.
- **Baseny**: Spacja zaznacza narożnik, Enter kopie; mod ostrzega, zanim woda coś odetnie.
  Control+Enter zasypuje pola. Simowie potrzebują drabinki (jest na liście narzędzia).
- **Teren**: wyrównuje od zaznaczonego pola do kursora albo podnosi i obniża grunt.

**Trawa** używa tego samego kursora: Enter maluje pole pod nim.

### Drzwi, okna i schody

Drzwi i okna ustawia się jak meble, z listy miejsc: "w ścianie od zachodu: kuchnia, prowadzi do:
na zewnątrz, pierwsze drzwi tego pokoju".

Ze strzałkami drzwi lub okno mówią, w której ścianie by się znalazły; obracanie wybiera ścianę. Po ustawieniu drzwi mod mówi, czy Simowie mogą przez nie przejść, albo co blokuje. Aby zmienić je później, znajdź je w **drzwi, okna i schody** lub przez Control+F: Enter podnosi je, by przenieść, Delete usuwa (Control+kliknięcie z gry).

### Dach

Gra sama buduje dach nad ścianami, więc nie ma kursora ani czego ustawiać. Narzędzie Dach wybiera tylko nachylenie i model, dla całego domu. Modele nie mają w grze nazw: mod podaje ich kolor.

## Sygnały dźwiękowe

Kursor budowy, ustawianie obiektów i mapa domu odtwarzają krótki dźwięk przy każdym kroku:
podłoga, ściana, płot, drzwi lub okno z każdej strony (każda strona brzmi inaczej), obiekty,
zmiana pokoju, krawędź parceli, spadki terenu i to, czy obiekt się mieści. **Control+Shift+S**
wyłącza je i włącza.

## Inne klawisze

| Klawisz | Co robi |
|---|---|
| Control+Shift+C | Konsola kodów gry. Strzałka w górę i strzałka w dół wybierają kod, pisanie coś do niego dodaje, Enter go wykonuje (ponownie: powtarza), Escape zamyka. |
| Control+Shift+V | Podświetla wybrany element na ekranie dla widzącej osoby pomagającej. Na parceli wskaźnik myszy przechodzi też do rzeczy, która ma fokus. |

## Klawisze samej gry

Te klawisze należą do gry i nadal działają:

| Klawisz | Co robi |
|---|---|
| F1, F2, F3, F4, F5 | Tryb życia, kupowania, budowania, kamery i opcji. |
| P | Pauza. |
| 1, 2, 3 | Prędkość normalna, szybka, najszybsza. |
| przecinek, kropka | Obracają widok. |
| plus, minus | Przybliżają i oddalają. |
| Y, N | Tak i nie w oknach dialogowych. |
| Control+S | Zapis. |

## Ustawienia

Ustawienia moda są w pliku `SimsAccess\SimsAccess.ini` w folderze gry, w sekcji `[mod]`. Klawisze,
które je zmieniają, od razu je zapisują.

| Ustawienie | Wartości |
|---|---|
| `narrator` | 2 wszystko, 1 tylko ostrzeżenia, 0 wyłączony (Control+N). |
| `cues` | 1 sygnały dźwiękowe włączone, 0 wyłączone (Control+Shift+S). |
| `cue_volume` | Głośność sygnałów dźwiękowych, od 0 do 100 (domyślnie 80). |
| `show_focus` | 1 podświetla wybrany element na ekranie (Control+Shift+V). |
| `social` | Czyje rozmowy są mówione: 3 wszystkich, 2 każdego z twoją rodziną, 1 twojej rodziny, 0 niczyje (Control+Shift+N). |
| `hours` | 1 podaje każdą nową godzinę (Control+T). |
| `camera_glide` | Jak długo kamera płynie do przeglądanego obiektu, w milisekundach (domyślnie 800; 0 to szybkie 250 samej gry). |

## Gdy coś nie działa

- **Brak jakiejkolwiek mowy**: uruchom czytnik ekranu przed grą. Dziennik moda to
  `SimsAccess\SimsAccess.log`; dołącz go do zgłoszenia.
- **"Nieobsługiwana wersja gry"**: gra się zaktualizowała; zainstaluj następną wersję moda.
- **Klawisz nic nie robi**: Control+F1 mówi, które klawisze działają tam, gdzie jesteś.

## Jeszcze nie gotowe

Własne wyjazdy i ekrany dodatków Vacation, Superstar i Makin' Magic nie są jeszcze w pełni
obsługiwane. Tryb zdjęć i kamery oraz album rodzinny są wyłącznie wizualne i zostały pominięte.
