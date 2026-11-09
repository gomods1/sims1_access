---
title: Håndbok for Sims-tilgjengelighet
lang: nb
toc-title: Innhold
---

Sims-tilgjengelighet gjør The Sims Legacy Collection spillbart med en skjermleser og bare tastaturet. Modden leser opp hver skjerm, hver dialogboks og hvert panel gjennom skjermleseren din (NVDA, JAWS, Skjermleser og andre), lar deg bevege deg gjennom dem med de samme tastene overalt, og legger til verktøy en blind spiller trenger: en liste over tomtens objekter, et kart over huset, en forteller for det spillet bare viser, steder der møbler får plass, og en markør for å bygge vegger og rom.

Denne håndboken åpnes med **Kontroll+Skift+F1** i spillet, på spillets språk. **Kontroll+F1** sier tastene på skjermen du er på.

## Kom i gang

1. Kjør installasjonsprogrammet, eller kopier `version.dll` og mappen `SimsAccess` inn i spillets mappe (den med `Sims.exe`).
2. Start skjermleseren din, og start deretter The Sims Legacy Collection fra Steam.
3. Når spillvinduet åpnes, hører du "Sims-tilgjengelighet lastet inn". Spillet starter på nabolagsskjermen.

Modden snakker spillets språk. For å bytte det åpner du Steam, deretter spillets Egenskaper, Generelt, Språk. Modden snakker engelsk, fransk, tysk, italiensk, spansk, nederlandsk, dansk, svensk, norsk, finsk, brasiliansk portugisisk, japansk, polsk, forenklet og tradisjonell kinesisk, thai og koreansk.

Modden virker med den nåværende Steam-versjonen av spillet. Hvis spillet blir oppdatert og modden sier "spillversjonen støttes ikke", installerer du moddens neste versjon.

## Bevege seg rundt: de samme tastene overalt

Hver skjerm, dialogboks, hvert panel og hver modus virker på samme måte. Det du beveger deg gjennom, er en ring av **grupper** (knappene i en dialogboks, behovene, familien, katalogen...). En gruppe inneholder **elementer**, og noen elementer åpner et dypere **nivå**.

| Tast | Hva den gjør |
|---|---|
| Tab, Skift+Tab | Neste eller forrige gruppe. Gruppens navn og antall elementer sies først. |
| Pil opp, Pil ned (eller Venstre pil, Høyre pil) | Forrige eller neste element i gruppen. I katalogene i kjøpe- og byggemodus blar Venstre pil og Høyre pil i stedet siden. |
| Home, End | Første eller siste element i gruppen. |
| Enter | Aktiverer elementet. På et nivå (en type eller et rom blant tomtens objekter): åpner det. |
| Mellomrom | Aktiverer, unntatt på tomten, der Mellomrom er spillets tast for "neste sim". |
| Backspace | Et nivå tilbake. |
| R | Sier elementet igjen, med gruppen det er i. |
| Escape | Lukker handlingsmenyen; ellers spillets egen Escape. |
| Alt+bokstav | Neste gruppe der navnet begynner med den bokstaven. |
| Kontroll+F1 | Tastene på skjermen du er på. |
| Kontroll+Skift+F1 | Denne håndboken. |

Kontroller sier hva de er etter navnet sitt, slik en skjermleser gjør: "Lagre, knapp", "Lydvalg, fane, valgt", "Autosentrering, avmerkingsboks, avmerket", "Lydeffekter, glidebryter, 8 av 10", "Terrengdetalj, verdivelger, Høy".

- **Glidebrytere, verdivelgere og personlighetsstolper**: Venstre pil og Høyre pil (eller Side opp og Side ned) endrer verdien. Enter sier den bare igjen.
- **Avmerkingsbokser**: Enter merker av og fjerner merket.
- **Tekstfelt** (navn på familier og simmer, biografier): gå til feltet med piltastene og skriv. Hver bokstav gjentas, og Backspace sier hva den sletter. Venstre pil, Høyre pil, Home, End og Delete flytter i feltet. Så lenge et tekstfelt er på skjermen, skriver bokstaver, Mellomrom og R.
- **Deaktiverte kontroller** sier "utilgjengelig" og aktiveres ikke.

Den siste gruppen på hver skjerm er **modhjelp**: knappene der sier tastene på den skjermen (slik Kontroll+F1 gjør) og åpner denne håndboken (slik Kontroll+Skift+F1 gjør). Når spillet starter, sier modden begge tastene etter "Sims-tilgjengelighet lastet inn".

## Lese mer: gjennomgangsbufferne

Ikke alt får plass i én opplest linje. Modden har to buffere du kan lese linje for linje:

- **detaljer**: alt om det som nettopp ble sagt. Et produkts vurderinger, rom og beskrivelse; handlingene, rommet, avstanden og salgsverdien til et objekt på tomten; forklaringen på et behov eller en ferdighet; verktøytipset til en kontroll og plassen den har i gruppen ("3 av 12, kontroller").
- **hendelser**: det som har skjedd, nyeste først: kjøp, salg, endringer i hastighet, tekster fra dialogbokser, handlinger i kø, alt fortelleren har sagt (de siste 200).

| Tast | Hva den gjør |
|---|---|
| Kontroll+Pil opp | Neste linje, dypere inn i bufferen. |
| Kontroll+Pil ned | Tilbake mot toppen. |
| Kontroll+Venstre pil, Kontroll+Høyre pil | Bytter mellom bufferne. |

## Nabolaget

Hovedgruppen på nabolagsskjermen er **hus**: hvert hus sier adressen sin og hvem som bor der. Enter på et hus aktiverer det, slik et klikk gjør, og deretter følger spillets eget kort eller spørsmål. På et hus leser Kontroll+Pil opp detaljene: det husoversikten i Byggmodus ville si, etter husets siste lagring (rom, hva som mangler, rom uten inngang). Kontroll+Skift+L sier alt på én gang.

De andre gruppene inneholder spillets knapper: lag en familie, familiebingen, nabolagets nummer, valg, avslutt.

Et hus uten familie åpnes bare i Byggmodus, med ubegrensede penger; modden sier "ingen familie, ubegrensede penger" i stedet for et beløp.

## Lage en familie og simmer

Alt på disse skjermene er en gruppe eller et felt:

- Skriv familienavnet og simmens fornavn i tekstfeltene deres.
- **personlighet**: Venstre pil og Høyre pil legger til eller tar bort et poeng fra hvert trekk ("4 av 10, 6 poeng igjen"). Stjernetegnet følger av poengene.
- **utseende**: alder, kjønn og hudfarge, og pilene for hode og kropp. Hvert hode og hver kropp beskrives ("hode 3 av 137: blondt hår, pasjehår").
- Biografien er også et tekstfelt.
- Kontroll+Skift og et tall (1 til 9) på tomten åpner senere kortet til en sim, der biografien kan redigeres.

## Tomten: Levende modus

På en tomt er kontrollpanelet en ring av grupper som enhver skjerm. Tab går gjennom dem i denne rekkefølgen:

1. **status**: klokkeslett, midler, spillhastighet, venner.
2. **Behov** for den valgte simmen, hvert i prosent; Enter sier hva behovet betyr.
3. **familie**: familiens simmer; Enter velger en.
4. **handlingskø**: det den valgte simmen skal gjøre; Enter avbryter en handling.
5. **objekter etter type** og **objekter etter rom**: tomtens objekter (se nedenfor).
6. **kontroller**, **sim-paneler** (Humør, Forhold, Jobb og Ferdigheter, Personlighet, Interesser, Hus), og alle paneler du åpner.
7. **modhjelp**.

På et element i et panel i Levende modus (et behov, en ferdighet, en jobb, en vurdering av huset) leser Enter opp spillets hurtigvindu for det.

Jobben sier arbeidstiden og når samkjøringen kommer. Hver ferdighet sier nivået sitt og, når en forfremmelse krever mer, hvor mye: "Matlaging, 2 av 10, neste forfremmelse krever 4". Spillets menyer har ikke noe valg for å si opp: en sim som stadig uteblir fra jobben får sparken, og avisen og datamaskinen tilbyr andre jobber.

### Velge simmer og kameraet

| Tast | Hva den gjør |
|---|---|
| Mellomrom | Spillets "neste sim": neste familiemedlem velges og sies ("Isabel, valgt, Spis"). |
| Kontroll+1 til Kontroll+9 | Velger familiens første til niende sim. |
| Kontroll+Skift+1 til Kontroll+Skift+9 | Åpner kortet til den simmen (biografi). |
| Kontroll+Skift+F | Kameraet følger den valgte simmen, eller er fritt. |
| Kontroll+E | Den valgte simmen på én linje: rom, hva den gjør, akutte behov, humør. |
| Kontroll+Skift+E | Hvordan en sim ser ut: hodet og kroppen ("hode: svart hår, kort; kropp: dress, svart"). Simmen som har fokus blant tomtens objekter, ellers den valgte. En sims detaljer (Kontroll+Pil opp på den) sier det også. |
| Kontroll+T | Sier hver ny time på klokken ("klokken er 14:00"), eller slutter med det. Nyttig på raskeste hastighet. |
| P | Pause (spillets tast); modden sier klokkeslett og midler. 1, 2, 3 stiller inn hastigheten. |

Kameraet følger den valgte simmen, fordi spillets lyder kommer fra det som er på skjermen. Mens du blar gjennom tomtens objekter, går kameraet til hvert objekt og kommer tilbake til simmen etterpå. Det glir sakte dit, så bildet ikke farer av gårde (innstillingen `camera_glide`).

Zoom og kameraets retning sies når de endres (pluss og minus zoomer, komma og punktum snur visningen), og de står også i statuslinjen: "kamera: middels visning, vendt mot nordvest". Retningen er det toppen av skjermen viser. Modens nord snur seg ikke med kameraet.

### Tomtens objekter

Alt på tomten finnes i to grupper: **objekter etter type** (simmer, deretter kategoriene i Kjøpemodus: sitteplasser, overflater, apparater..., så andre objekter) og **objekter etter rom** (først rommet til den valgte simmen, så de andre, hvert med etasje og størrelse). Enter på en type eller et rom viser objektene der, nærmeste først, som "Kjøleskap, kjøkken, 6 fliser mot nordøst" fra den valgte simmen. Backspace går tilbake til typene eller rommene. Rommenes navn er moddens gjetning ut fra det som står i dem.

Du trenger ikke å tabbe dit: Side-tastene blar gjennom objektene fra hvor som helst på tomten, i Levende modus, Kjøpemodus og Byggmodus.

| Tast | Hva den gjør |
|---|---|
| Side ned, Side opp | Neste eller forrige objekt av typen eller i rommet du valgte sist. |
| Skift+Side ned, Skift+Side opp | Neste eller forrige type objekter. |
| Kontroll+Side ned, Kontroll+Side opp | Neste eller forrige rom (første trykk: rommet til den valgte simmen). |
| Enter | Objektets handlinger (i Kjøpemodus: plukker det opp for å flytte det). På en oppføring med like ting ("Klappstol, 4 like"): viser dem én og én, nærmeste først; Backspace går tilbake. |
| Kontroll+Enter | Sender den valgte simmen til objektet. |
| Kontroll+F | Finn noe: skriv en del av navnet ("tel" finner telefonen, aksenter spiller ingen rolle). Pil opp og ned går gjennom treffene, Enter handler på det, Kontroll+Enter sender simmen dit, Delete fjerner en dør eller et vindu, Escape går tilbake dit du var. |
| Delete | På en dør eller et vindu, i kjøps- eller byggmodus: fjerner det og gir pengene tilbake. |

En sim eller et objekt som fortsatt viser en boble, sier den også ("Bella, Spis, tenker: sult"). En ting ingen sim kan komme til fra tomtens inngang, sier "ingen sim kan nå det".

Etter simmene kommer **problemer**: det som trenger oppmerksomhet, som ødelagte ting, det som må rengjøres, vannpytter, søppel, aske, ild og alt med fluer over. En tom matskål for kjæledyr er her også; i alle lister sier en matskål om det er mat i den. Til slutt kommer **dører, vinduer og trapper**.

### Handlingsmenyer

Enter på et objekt (eller på en sim) åpner spillets handlingsmeny. Pil opp og Pil ned velger, Enter utfører, Escape går et nivå tilbake eller lukker. Valg som slutter på "..." åpner flere valg. Den valgte handlingen sies som satt i kø ("Snakk, i kø"). Når menyen er tom, sier modden hvorfor ("ingen handlinger; bare for barn: Lek").

### Rom og huset

| Tast | Hva den gjør |
|---|---|
| Kontroll+L | Rommets planløsning: størrelse, dører og vinduer for hver vegg og hvor de fører; det som står langs hver vegg, er i detaljbufferen. Den sier også rommets lys, som teller med i rommets poengsum. |
| Kontroll+Skift+L | Husoversikten: etasjer, rom, vurderinger, hva som mangler (toalett, dusj, senger, kjøleskap, komfyr, telefon, røykvarsler), rom uten inngang, objekter ingen kan nå, mørke eller mislikte rom. |

### Husets kart

**Kontroll+M** slår på kartet: en markør som går gjennom huset rute for rute, med start ved den valgte simmen. Hvert steg spiller en lyd for gulvet og for hver vegg, hvert gjerde, hver dør eller hvert vindu rundt.

| Tast | Hva den gjør |
|---|---|
| Piltastene | Én rute; med Skift fem. Sier hva som står der og hvilken vei det vender, rommet og etasjen når de skifter, veggene, gjerdene, rekkverkene, dørene og vinduene på hver side. Vegger, gjerder og vinduer stopper markøren; dører slipper den gjennom. Med rommet kommer lyset ("lys 64 %": vinduer og lamper; dagslys ute), som teller med i rommets poengsum. |
| Alt+Piltastene | Det samme, gjennom vegger. |
| Komma, punktum | Hva hvert steg sier: alt, bare objekter, bare gulv (med navn, "Lys granittflis"), bare vegger, bare rom. |
| Home | Tilbake til den valgte simmen. |
| Kontroll+Skift+Side opp, Side ned | Etasjen over eller under. |
| Enter | Handlingsmenyen for det som står der (en ting på et bord eller en pult før bordet: datamaskinen, ikke pulten). |
| Kontroll+Enter | Sender den valgte simmen dit. |
| Side ned, Side opp (og Skift, Kontroll) | Tomtens objekter som alltid; kartet går til hvert av dem. |
| R | Alt om ruten. Kontroll+L rommets planløsning. |
| Escape eller Kontroll+M | Forlater kartet. |

### Fortelleren

Spillet viser mange ting bare som bilder. Modden sier dem mens de skjer og lagrer dem i hendelsesbufferen:

- **Bobler** over simmer og objekter: "Bella tenker: sult", "Mortimer roper: blære", "Bella sier: Romvesener", et emne den andre simmen misliker ("..., ikke interessert"), "forholdet blir dårligere", og "Søppeldunk: fluer".
- **Blokkerte veier**: "Bella kommer ikke forbi: Stol står i veien".
- **Besøkende** som kommer og går, spøkelser, innbruddstyven, Mannen med ljåen, pizzabudet, samkjøringen og skolebussen (hvilken bil som tuter).
- **Ting som blir tatt** av namsmannen eller en innbruddstyv, med verdien; **regninger** som kommer.
- **Familievakten**: hva den valgte simmen begynner å gjøre, på din ordre eller på eget initiativ ("Cris, på eget initiativ: Sitt"), når den er ferdig eller blir avbrutt ("Cris avbrøt Sitt for Panikk; den venter i køen"), bytte av rom, behov som blir lave, humøret som blir rødt, dagens endringer i forhold ("Marisa med Cris: +8, nå 28"; spillets forholdspanel viser også disse tallene), og penger betalt eller mottatt utenfor Kjøpemodus.
- **Turer**: hvor du har kommet, og "hjemme igjen".

**Kontroll+N** endrer hvor mye som sies: alt, bare advarsler eller av. Linjer som ikke sies, havner likevel i hendelsesbufferen.

**Kontroll+Skift+N** velger hvem sine samtaler som sies (det simmene sier, sosiale handlinger og forholdsendringene de gir): alle som snakker med familien din (i starten), alle på tomten, bare familien din eller ingen.

Lyder du kan høre (telefonen, dørklokken, alarmer), blir ikke meldt.

Handlinger med en annen sim nevner den simmen, uansett hvem som startet dem: "Homer: Hils, med Elvira".

### Hjelpetips og opplæringen

Når spillets hjelpesystem har et tips som venter, sier modden "det finnes et tips fra hjelpesystemet: Kontroll+H leser det". **Kontroll+H** åpner det: tipset leses opp, eller modden sier hva spillets røde pil peker på.

Hus 7 i det første nabolaget kjører spillets opplæring. Hvert trinn leses som en tastaturversjon av spillets tekst ("Trykk på knappen Zoom inn: Tab til kontroller, piltastene til Zoom inn, Enter"), etterfulgt av hva pilen peker på. Hvert trinn kan fullføres med tastaturet. Første Escape under opplæringen advarer deg; en andre Escape innen fire sekunder avslutter den.

### Pizza og andre tjenester

Bruk telefonens handlinger for å ringe etter tjenester. Pizzabudet må hilses på: når han kommer, gå til objekter etter type, simmer, Pil ned til ham, Enter, Hils. Hvis ingen hilser på ham, går han med pizzaen, og pengene kommer tilbake.

## Kjøpemodus

F2 åpner Kjøpemodus. Katalogens kategorier er faner og produktene er knapper: hvert produkt sier navn, pris, viktigste vurderinger og beskrivelse, og detaljbufferen inneholder alle vurderingene og rommene det er ment for. Går du forbi det siste produktet på en side, blas det til neste side. Hvert produkt sier også størrelsen i fliser, den lengste siden først ("2 ganger 1 fliser"). Penger sies i simoleoner, spillets valuta.

| Tast | Hva den gjør |
|---|---|
| Kontroll+R | Begrenser katalogen til produkter for én type rom (kjøkken, bad, soverom, stue, spisestue, arbeidsrom, utendørs) eller alle. |
| Backspace | Katalogens "Tilbake". |
| Venstre pil, Høyre pil | Katalogens forrige eller neste side; opp og ned går produkt for produkt. |
| Enter på et produkt | Begynner å plassere det. |

### Plassere et objekt

Mens du holder et objekt, har modden allerede funnet stedene der det får plass **og kan brukes** (spillets plasseringssjekk, rommets vegger og dører, plassen simmene trenger for å bruke det). Stedet sier rommet sitt, hva det står ved siden av og hvilken vei det vender.

| Tast | Hva den gjør |
|---|---|
| Side ned, Side opp (eller Tab) | Stedene, de beste først. |
| Kontroll+Side ned, Kontroll+Side opp | Rommene som har steder. |
| Skift+Side ned, Skift+Side opp (eller komma, punktum) | De andre retningene på samme sted. |
| Kontroll+Skift+Side ned, Side opp | Ved siden av tomtens objekter, nærmeste først. |
| Piltastene | Flytter det i stedet én rute (Skift: fem), og sier om det får plass og hva det støter mot. Vegger, gjerder og vinduer stopper det; dører slipper det gjennom. |
| Alt+Piltastene | Det samme, gjennom vegger. |
| Enter | Kjøper det der (eller setter det ned, når du flytter). |
| Escape | Avbryter (et flyttet objekt går tilbake). |
| Delete | Selger objektet du holder; refusjonen sies. |
| R | Sier stedet igjen. |

Når objektet ikke passer der pilene setter det, slutter linjen med det som ville gå: retningen
det passer i på samme flis ("passer snudd, vendt mot sør"), ellers nærmeste flis der det passer
("nærmeste sted den passer 1 flis mot vest, vendt mot nord"). For et objekt på flere fliser sier
den første pilen og hver vending hvilken del av det markøren er på, og hvor resten går
("markøren på kanten mot nord, den strekker seg 1 flis mot sør"); stedets detaljer sier det
samme.

Detaljbufferen inneholder stedets rute og stedene spillet avviste, med grunnen.

For en sitteplass kommer først stedene der noe brukes sittende og ennå ikke har sete: "vendt mot sør, for å bruke Datamaskin sittende". Etter at et objekt er plassert, sier modden hvordan det skal brukes: "for å bruke Datamaskin sittende", "brukes fra Kontorstol", "simmene kan nå det" eller "ingen sim kan nå det".

### Flytte og selge

I Kjøpemodus går du med Side ned til et objekt på tomten og trykker Enter: det plukkes opp, og den samme listen over steder vises, med start i rommet det kom fra. Den første pilen setter markøren der det stod. Like objekter vises ett og ett, og det siste kjøpet sier "nettopp kjøpt".

## Byggmodus

F3 åpner Byggmodus. Verktøyene er faner, stilene deres er produkter. Husoversikten leses opp når du går inn i Byggmodus (Kontroll+Skift+L gjentar den).

Når du trykker på et verktøy, kommer du til modellene, i en egen gruppe med verktøyets navn; Tab går tilbake til verktøyene. Du har ingenting i hånden før du trykker Enter på en modell.

### Byggmarkøren

Enter på en stil av vegg eller gjerde starter byggmarkøren ved den valgte simmen. Den beveger seg på hjørnene mellom rutene.

| Tast | Hva den gjør |
|---|---|
| Piltastene | Ett hjørne; med Skift fem. Sier hvor den er, hvilke vegger som møtes der og hva som står der. |
| Mellomrom | Merker starten. Deretter sier markøren avstanden fra starten og hva Enter ville bygge og koste. |
| Enter | Bygger en vegg fra starten til markøren. |
| Skift+Enter | Bygger de fire veggene i et rom mellom starten og markøren. |
| Kontroll+Enter eller Delete | Fjerner vegger. |
| komma, punktum | Velger en ferdig romstørrelse. |
| Insert | Bygger det ferdige rommet med nordvesthjørnet ved markøren. |
| Tab, Skift+Tab (eller Kontroll+Side ned, Side opp) | Hopper til neste eller forrige rom. |
| Side opp, Side ned | Etasjen over eller under. |
| Home | Tilbake til den valgte simmen. |
| R | Alt: den frie plassen i hver retning, rommet. |
| Escape | Forlater markøren. |

Før noe bygges, sjekker modden de nye veggene. Hvis de ville stenge inne en sim, skjære av et objekt, stenge en dør eller stenge av en del av tomten, sier den det, og en andre Enter bygger likevel.

### Gulv, tapet, svømmebassenger og terreng

Hvert av disse verktøyene har den samme markøren på ruter (piltastene, Skift for fem, Home, R, Escape; Kontroll+F1 for detaljene):

- **Gulv**: Enter legger gulv på ruten, eller på rektangelet fra en start merket med Mellomrom; Skift+Enter hele rommet; Kontroll+Enter fjerner.
- **Tapet**: Enter tapetserer hver vegg i rommet under markøren; Kontroll+Enter tar det av.
- **Svømmebassenger**: Mellomrom merker et hjørne, Enter graver; modden advarer før vann skjærer av noe. Kontroll+Enter fyller igjen ruter. Simmene trenger en bassengstige (i verktøyets liste).
- **Terreng**: jevner ut fra en merket rute til markøren, eller hever og senker bakken.

**Gress** bruker samme markør: Enter maler ruten under den.

### Dører, vinduer og trapper

Dører og vinduer plasseres som møbler, fra en liste over steder: "i veggen mot vest i kjøkken, fører ut til utenfor, rommets første dør".

Med pilene sier en dør eller et vindu hvilken vegg det ville komme i; å snu det velger veggen. Når en dør er plassert, sier moden om simmene kan gå gjennom, eller hva som er i veien. For å endre den senere: finn den under **dører, vinduer og trapper** eller med Kontroll+F. Enter tar den opp for å flytte den, Delete fjerner den (spillets Kontroll+klikk).

### Taket

Spillet bygger taket selv over veggene dine, så det er ingen markør og ingenting å plassere. Takverktøyet velger bare helning og modell, for hele huset. Modellene har ingen navn i spillet: moden sier fargen deres.

## Lydsignaler

Byggmarkøren, plasseringen og husets kart spiller en kort lyd for hvert steg: gulvet, veggen, gjerdet, døren eller vinduet på hver side (hver side låter forskjellig), objekter, bytte av rom, tomtens kant, skråninger, og om et objekt får plass. **Kontroll+Skift+S** slår dem av og på.

## Andre taster

| Tast | Hva den gjør |
|---|---|
| Kontroll+Skift+C | Spillets juksekonsoll. Pil opp og Pil ned velger en juksekode, det du skriver legges til, Enter kjører den (igjen: gjentar), Escape lukker. |
| Kontroll+Skift+V | Fremhever den valgte kontrollen på skjermen for en seende hjelper. På tomten går musepekeren også til tingen som har fokus. |

## Spillets egne taster

Disse hører til spillet og virker fortsatt:

| Tast | Hva den gjør |
|---|---|
| F1, F2, F3, F4, F5 | Levende modus, Kjøpemodus, Byggmodus, Kameramodus og Valgmodus. |
| P | Pause. |
| 1, 2, 3 | Normal, rask, raskeste hastighet. |
| komma, punktum | Roterer visningen. |
| plus, minus | Zoom inn og ut. |
| J, N | Ja og nei i dialogbokser. |
| Kontroll+S | Lagre. |

## Innstillinger

Moddens innstillinger ligger i `SimsAccess\SimsAccess.ini` i spillets mappe, delen `[mod]`. Tastene som endrer dem, lagrer dem med en gang.

| Innstilling | Verdier |
|---|---|
| `narrator` | 2 alt, 1 bare advarsler, 0 av (Kontroll+N). |
| `cues` | 1 lydsignaler på, 0 av (Kontroll+Skift+S). |
| `cue_volume` | Lydsignalenes volum, 0 til 100 (standard 80). |
| `show_focus` | 1 fremhever den valgte kontrollen på skjermen (Kontroll+Skift+V). |
| `social` | Hvem sine samtaler som sies: 3 alle, 2 alle som snakker med familien din, 1 familien din, 0 ingen (Kontroll+Skift+N). |
| `hours` | 1 sier hver ny time (Kontroll+T). |
| `camera_glide` | Hvor lenge kameraet bruker på å gli til et objekt du blar til, i millisekunder (standard 800; 0 spillets egne raske 250). |

## Når noe går galt

- **Ingen tale i det hele tatt**: start skjermleseren før spillet. Moddens logg er `SimsAccess\SimsAccess.log`; send den med rapporten din.
- **"Spillversjonen støttes ikke"**: spillet er oppdatert; installer moddens neste versjon.
- **En tast gjør ingenting**: Kontroll+F1 sier hvilke taster som virker der du er.

## Ikke ferdig ennå

Vacations, Superstars og Makin' Magics egne turer og skjermer er ikke helt dekket ennå. Fotomodus og kameramodus og familiealbumet er bare visuelle og er utelatt.
