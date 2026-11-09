---
title: Manual til Sims-tilgængelighed
lang: da
toc-title: Indhold
---

Sims-tilgængelighed gør The Sims Legacy Collection spilbart med en skærmlæser og kun tastaturet. Modden læser hver skærm, hver dialogboks og hvert panel op gennem din skærmlæser (NVDA, JAWS, Oplæser og andre), lader dig bevæge dig rundt i dem med de samme taster overalt og tilføjer de værktøjer, en blind spiller har brug for: en liste over grundens objekter, et kort over huset, en fortæller for det, spillet kun viser, steder hvor møbler kan stå, og en markør til at bygge mure og rum.

Denne manual åbnes med **Kontrol+Skift+F1** i spillet, på spillets sprog. **Kontrol+F1** siger tasterne på den skærm, du er på.

## Kom i gang

1. Kør installationsprogrammet, eller kopiér `version.dll` og mappen `SimsAccess` ind i spillets mappe (den med `Sims.exe`).
2. Start din skærmlæser, og start derefter The Sims Legacy Collection fra Steam.
3. Når spilvinduet åbner, hører du "Sims-tilgængelighed indlæst". Spillet starter på nabolagsskærmen.

Modden taler spillets sprog. For at skifte det skal du åbne Steam, så spillets Egenskaber, Generelt, Sprog. Modden taler engelsk, fransk, tysk, italiensk, spansk, hollandsk, dansk, svensk, norsk, finsk, brasiliansk portugisisk, japansk, polsk, forenklet og traditionelt kinesisk, thai og koreansk.

Modden virker med den nuværende Steam-version af spillet. Hvis spillet bliver opdateret, og modden siger "spilversionen understøttes ikke", skal du installere moddens næste version.

## Bevæg dig rundt: de samme taster overalt

Hver skærm, dialogboks, hvert panel og hver funktion virker på samme måde. Det, du bevæger dig igennem, er en ring af **grupper** (en dialogboks' knapper, behovene, familien, kataloget...). En gruppe indeholder **elementer**, og nogle elementer åbner et dybere **niveau**.

| Tast | Hvad den gør |
|---|---|
| Tab, Skift+Tab | Næste eller forrige gruppe. Gruppens navn og antal elementer siges først. |
| Pil op, Pil ned (eller Venstre pil, Højre pil) | Forrige eller næste element i gruppen. I katalogerne i køb- og byggetilstand bladrer Venstre pil og Højre pil i stedet siden. |
| Home, End | Første eller sidste element i gruppen. |
| Enter | Aktiverer elementet. På et niveau (en type eller et rum blandt grundens objekter): åbner det. |
| Mellemrum | Aktiverer, undtagen på grunden, hvor Mellemrum er spillets tast for "næste Sim". |
| Backspace | Et niveau tilbage. |
| R | Siger elementet igen, med dets gruppe. |
| Escape | Lukker handlingsmenuen; ellers spillets egen Escape. |
| Alt+bogstav | Den næste gruppe, hvis navn begynder med det bogstav. |
| Kontrol+F1 | Tasterne på den skærm, du er på. |
| Kontrol+Skift+F1 | Denne manual. |

Kontrolelementer siger, hvad de er, efter deres navn, som en skærmlæser gør: "Gem, knap", "Lydindstillinger, fane, valgt", "Auto-centrering, afkrydsningsfelt, markeret", "Lydeffekter, skyder, 8 af 10", "Terrændetaljer, værdivælger, Høj".

- **Skydere, værdivælgere og personlighedsbjælker**: Venstre pil og Højre pil (eller Side op og Side ned) ændrer værdien. Enter siger den bare igen.
- **Afkrydsningsfelter**: Enter markerer og fjerner markeringen.
- **Tekstfelter** (navne på familier og Simmere, biografier): gå til feltet med piletasterne, og skriv. Hvert bogstav gentages, og Backspace siger, hvad den sletter. Venstre pil, Højre pil, Home, End og Delete flytter i feltet. Så længe der er et tekstfelt på skærmen, skriver bogstaver, Mellemrum og R.
- **Deaktiverede kontrolelementer** siger "ikke tilgængelig" og aktiveres ikke.

Den sidste gruppe på hver skærm er **hjælp til modden**: dens knapper siger tasterne på den skærm (som Kontrol+F1 gør) og åbner denne manual (som Kontrol+Skift+F1 gør). Når spillet starter, siger modden begge taster efter "Sims-tilgængelighed indlæst".

## Læs mere: gennemgangsbufferne

Ikke alt kan være i én talt linje. Modden har to buffere, du kan læse linje for linje:

- **detaljer**: alt om det, der lige blev sagt. Et produkts vurderinger, rum og beskrivelse; et objekts handlinger, rum, afstand og salgsværdi på grunden; forklaringen på et behov eller en færdighed; et kontrolelements værktøjstip og dets plads i gruppen ("3 af 12, kontroller").
- **hændelser**: hvad der er sket, nyeste først: køb, salg, skift af hastighed, tekster fra dialogbokse, handlinger i kø, alt hvad fortælleren har sagt (de sidste 200).

| Tast | Hvad den gør |
|---|---|
| Kontrol+Pil op | Næste linje, dybere ind i bufferen. |
| Kontrol+Pil ned | Tilbage mod toppen. |
| Kontrol+Venstre pil, Kontrol+Højre pil | Skifter mellem bufferne. |

## Nabolaget

Nabolagsskærmens hovedgruppe er **huse**: hvert hus siger sin adresse, og hvem der bor der. Enter på et hus aktiverer det, som et klik gør, og derefter følger spillets eget kort eller spørgsmål. På et hus læser Kontrol+Pil op dets detaljer: det, Byggefunktionens husoversigt ville sige, ud fra husets sidste gemte version (rum, hvad der mangler, rum uden adgang). Kontrol+Skift+L siger det hele på én gang.

De andre grupper indeholder spillets knapper: skab en familie, familiebeholderen, nabolagets nummer, indstillinger, afslut.

Et hus uden familie åbner kun i Byggefunktion, med ubegrænsede penge; modden siger "ingen familie, ubegrænsede penge" i stedet for et beløb.

## Skab en familie og Simmere

Alt på disse skærme er en gruppe eller et felt:

- Skriv familienavnet og Simmens fornavn i deres tekstfelter.
- **personlighed**: Venstre pil og Højre pil giver hvert træk et point mere eller mindre ("4 af 10, 6 point tilbage"). Stjernetegnet følger af pointene.
- **udseende**: alder, køn og hudfarve samt pilene for hoved og krop. Hvert hoved og hver krop beskrives ("hoved 3 af 137: lyst hår, page").
- Biografien er også et tekstfelt.
- Kontrol+Skift og et tal (1 til 9) på grunden åbner senere en Sims kort, hvor biografien kan redigeres.

## Grunden: Spillefunktion

På en grund er kontrolpanelet en ring af grupper som enhver skærm. Tab går gennem dem i denne rækkefølge:

1. **status**: klokken, penge, spillets hastighed, venner.
2. **Behov** for den valgte Sim, hvert som en procent; Enter siger, hvad behovet betyder.
3. **familie**: familiens Simmere; Enter vælger en af dem.
4. **handlingskø**: hvad den valgte Sim vil gøre; Enter annullerer en handling.
5. **objekter efter type** og **objekter efter rum**: grundens objekter (se nedenfor).
6. **kontroller**, **Sim-paneler** (Humør, Forhold, Job og Færdigheder, Personlighed, Interesser, Hus) og ethvert panel, du åbner.
7. **hjælp til modden**.

På et element i et panel i Spillefunktion (et behov, en færdighed, et job, en vurdering af huset) siger Enter spillets pop op-tekst for det.

Jobbet siger arbejdstiden, og hvornår samkørslen kommer. Hver færdighed siger sit niveau og, når en forfremmelse kræver mere, hvor meget: "Madlavning, 2 af 10, den næste forfremmelse kræver 4". Spillets menuer har ingen mulighed for at sige op: En sim, der bliver ved med at udeblive, bliver fyret, og avisen og computeren tilbyder andre job.

### Vælg Simmere og kameraet

| Tast | Hvad den gør |
|---|---|
| Mellemrum | Spillets "næste Sim": det næste familiemedlem vælges og annonceres ("Isabel, valgt, Spise"). |
| Kontrol+1 til Kontrol+9 | Vælger familiens første til niende Sim. |
| Kontrol+Skift+1 til Kontrol+Skift+9 | Åbner den Sims kort (biografi). |
| Kontrol+Skift+F | Kameraet følger den valgte Sim eller er frit. |
| Kontrol+E | Den valgte Sim på én linje: rum, hvad den laver, akutte behov, humør. |
| Kontrol+Skift+E | Hvordan en Sim ser ud: hoved og krop ("hoved: sort hår, kort; krop: jakkesæt, sort"). Den Sim, der har fokus blandt grundens objekter, ellers den valgte. En Sims detaljer (Kontrol+Pil op på den) siger det også. |
| Kontrol+T | Siger hver ny time på uret ("klokken er 14:00"), eller holder op med det. Nyttigt ved den højeste hastighed. |
| P | Pause (spillets tast); modden siger klokken og pengene. 1, 2, 3 indstiller hastigheden. |

Kameraet følger den valgte Sim, fordi spillets lyde kommer fra det, der er på skærmen. Mens du gennemgår grundens objekter, går kameraet hen til hvert objekt og vender bagefter tilbage til Simmen. Det glider langsomt derhen, så billedet ikke farer af sted (indstillingen `camera_glide`).

Zoom og kameraets retning siges, når de ændres (plus og minus zoomer, komma og punktum drejer visningen), og de står også i statuslinjen: "kamera: mellem visning, vendt mod nordvest". Retningen er det, toppen af skærmen viser. Modets nord drejer ikke med kameraet.

### Grundens objekter

Alt på grunden findes i to grupper: **objekter efter type** (Simmere, så Købefunktionens kategorier: siddepladser, overflader, maskiner..., derefter andre objekter) og **objekter efter rum** (først den valgte Sims rum, så de andre, hvert med etage og størrelse). Enter på en type eller et rum viser dets objekter, nærmeste først, som "Køleskab, køkken, 6 fliser mod nordøst" fra den valgte Sim. Backspace går tilbage til typerne eller rummene. Rummenes navne er moddens gæt ud fra det, der står i dem.

Du behøver ikke at tabbe derhen: Side-tasterne gennemgår objekterne fra hvor som helst på grunden, i Spillefunktion, Købefunktion og Byggefunktion.

| Tast | Hvad den gør |
|---|---|
| Side ned, Side op | Næste eller forrige objekt af den type eller i det rum, du sidst valgte. |
| Skift+Side ned, Skift+Side op | Næste eller forrige type objekter. |
| Kontrol+Side ned, Kontrol+Side op | Næste eller forrige rum (første tryk: den valgte Sims rum). |
| Enter | Objektets handlinger (i Købefunktion: samler det op for at flytte det). På en linje med ens ting ("Klapstol, 4 ens"): viser dem én ad gangen, nærmeste først; Backspace går tilbage. |
| Kontrol+Enter | Sender den valgte Sim hen til objektet. |
| Kontrol+F | Find noget: skriv en del af navnet ("tel" finder telefonen, accenter er ligegyldige). Pil op og ned går gennem resultaterne, Enter handler på det, Kontrol+Enter sender simmen derhen, Delete fjerner en dør eller et vindue, Escape går tilbage, hvor du var. |
| Delete | På en dør eller et vindue, i købe- eller byggefunktion: fjerner det og refunderer det. |

En Sim eller et objekt, der stadig viser en boble, siger den også ("Bella, Spise, tænker: sult"). Noget, som ingen Sim kan nå fra grundens indgang, siger "ingen Sim kan nå det".

Efter simmerne kommer **problemer**: det, der kræver opmærksomhed, som ting i stykker, det der skal rengøres, vandpytter, affald, aske, ild og alt med fluer over. En tom madskål til kæledyr er her også; i alle lister siger en madskål, om der er mad i den. Til sidst kommer **døre, vinduer og trapper**.

### Handlingsmenuer

Enter på et objekt (eller på en Sim) åbner spillets handlingsmenu. Pil op og Pil ned vælger, Enter udfører, Escape går et niveau tilbage eller lukker. Punkter, der ender på "...", åbner flere valg. Den valgte handling siges som sat i kø ("Snak, i kø"). Når menuen er tom, siger modden hvorfor ("ingen handlinger; kun for børn: Lege").

### Rum og huset

| Tast | Hvad den gør |
|---|---|
| Kontrol+L | Rummets indretning: størrelse, døre og vinduer for hver mur, og hvor de fører hen; hvad der står langs hver mur, er i detaljebufferen. Den siger også rummets lys, som tæller med i rummets score. |
| Kontrol+Skift+L | Husoversigten: etager, rum, vurderinger, hvad der mangler (toilet, bruser, senge, køleskab, komfur, telefon, røgalarm), rum uden adgang, objekter som ingen kan nå, mørke eller upopulære rum. |

### Husets kort

**Kontrol+M** slår kortet til: en markør, der går gennem huset felt for felt, med start ved den valgte Sim. Hvert skridt spiller en lyd for gulvet og for hver mur, hvert hegn, hver dør eller hvert vindue omkring det.

| Tast | Hvad den gør |
|---|---|
| Piletasterne | Ét felt; med Skift fem. Siger, hvad der står der, og hvilken vej det vender, rummet og etagen når de skifter, murene, hegnene, gelænderne, dørene og vinduerne på hver side. Mure, hegn og vinduer standser markøren; døre lukker den igennem. Med rummet kommer dets lys ("lys 64 %": vinduer og lamper; dagslys udenfor), som tæller med i rummets score. |
| Alt+Piletasterne | Det samme, gennem mure. |
| Komma, punktum | Hvad hvert skridt siger: alt, kun objekter, kun gulve (ved navn, "Lyse granitfliser"), kun mure, kun rum. |
| Home | Tilbage til den valgte Sim. |
| Kontrol+Skift+Side op, Side ned | Etagen ovenover eller nedenunder. |
| Enter | Handlingsmenuen for det, der står der (noget på et bord eller skrivebord før bordet: computeren, ikke dens skrivebord). |
| Kontrol+Enter | Sender den valgte Sim derhen. |
| Side ned, Side op (og Skift, Kontrol) | Grundens objekter som altid; kortet går hen til hvert af dem. |
| R | Alt om feltet. Kontrol+L rummets indretning. |
| Escape eller Kontrol+M | Forlader kortet. |

### Fortælleren

Spillet viser mange ting kun som billeder. Modden siger dem, mens de sker, og gemmer dem i hændelsesbufferen:

- **Bobler** over Simmere og objekter: "Bella tænker: sult", "Mortimer råber: blære", "Bella siger: Rumvæsener", et emne den anden Sim ikke kan lide ("..., ikke interesseret"), "forholdet bliver dårligere" og "Affaldsspand: fluer".
- **Blokerede veje**: "Bella kan ikke komme forbi: Stol står i vejen".
- **Gæster**, der kommer og går, spøgelser, indbrudstyven, Manden med leen, pizzabuddet, samkørslen og skolebussen (hvilken bil der dytter).
- **Ting taget** af fogeden eller en indbrudstyv, med deres værdi; **regninger** der kommer.
- **Familievagten**: hvad den valgte Sim begynder at lave, på din ordre eller af sig selv ("Cris, af sig selv: Sid ned"), når den er færdig eller bliver afbrudt ("Cris stoppede Sid ned til fordel for Panik; den venter i køen"), skift af rum, behov der bliver lave, humøret der bliver rødt, dagens ændringer i forhold ("Marisa med Cris: +8, nu 28"; spillets forholdspanel viser også disse tal), og penge betalt eller modtaget uden for Købefunktion.
- **Ture**: hvor du er ankommet, og "hjemme igen".

**Kontrol+N** ændrer, hvor meget der siges: alt, kun advarsler eller fra. Linjer, der ikke siges, kommer stadig i hændelsesbufferen.

**Kontrol+Skift+N** vælger, hvis samtaler der siges (hvad Simmerne siger, deres sociale handlinger, de ændringer i forhold, de medfører): alle, der taler med din familie (fra start), alle på grunden, kun din familie eller ingen.

Lyde, du kan høre (telefonen, dørklokken, alarmer), annonceres ikke.

Handlinger med en anden sim nævner denne, uanset hvem der startede dem: "Homer: Hils, med Elvira".

### Hjælpetips og træningsspillet

Når spillets hjælpesystem har et tip klar, siger modden "der er et tip fra hjælpesystemet: Kontrol+H læser det". **Kontrol+H** åbner det: tippet læses op, eller modden siger, hvad spillets røde pil peger på.

Hus 7 i det første nabolag kører spillets træningsspil. Hvert trin læses som en tastaturversion af spillets tekst ("Tryk på knappen Zoom ind: Tab til kontroller, piletasterne til Zoom ind, Enter"), efterfulgt af hvad pilen peger på. Hvert trin kan klares med tastaturet. Det første tryk på Escape under træningsspillet advarer dig; et andet tryk på Escape inden for fire sekunder forlader det.

### Pizza og andre tjenester

Brug telefonens handlinger til at ringe efter tjenester. Pizzabuddet skal hilses på: når han kommer, gå til objekter efter type, Simmere, Pil ned til ham, Enter, Hils. Hvis ingen hilser på ham, går han igen med pizzaen, og pengene kommer tilbage.

## Købefunktion

F2 åbner Købefunktion. Katalogets kategorier er faner, og dets produkter er knapper: hvert produkt siger sit navn, pris, vigtigste vurderinger og beskrivelse, og detaljebufferen indeholder alle dets vurderinger og de rum, det er beregnet til. Går du forbi en sides sidste produkt, vendes siden. Hvert produkt siger også sin størrelse i fliser, den længste side først ("2 gange 1 fliser"). Penge siges i simoleoner, spillets valuta.

| Tast | Hvad den gør |
|---|---|
| Kontrol+R | Begrænser kataloget til produkter til én slags rum (køkken, badeværelse, soveværelse, stue, spisestue, arbejdsværelse, udendørs) eller alle. |
| Backspace | Katalogets "Tilbage". |
| Venstre pil, Højre pil | Katalogets forrige eller næste side; op og ned går produkt for produkt. |
| Enter på et produkt | Begynder at placere det. |

### Placér et objekt

Mens du holder et objekt, har modden allerede fundet de steder, hvor det kan stå **og kan bruges** (spillets placeringstjek, rummets mure og døre, den plads Simmerne skal bruge for at bruge det). Stedet siger sit rum, hvad det står ved siden af, og hvilken vej det vender.

| Tast | Hvad den gør |
|---|---|
| Side ned, Side op (eller Tab) | Stederne, de bedste først. |
| Kontrol+Side ned, Kontrol+Side op | De rum, der har steder. |
| Skift+Side ned, Skift+Side op (eller komma, punktum) | De andre retninger på samme sted. |
| Kontrol+Skift+Side ned, Side op | Ved siden af grundens objekter, nærmeste først. |
| Piletasterne | Flytter det i stedet ét felt (Skift: fem) og siger, om det kan stå der, og hvad det støder ind i. Mure, hegn og vinduer standser det; døre lukker det igennem. |
| Alt+Piletasterne | Det samme, gennem mure. |
| Enter | Køber det der (eller sætter det ned, når du flytter). |
| Escape | Annullerer (et flyttet objekt går tilbage). |
| Delete | Sælger det objekt, du holder; tilbagebetalingen siges. |
| R | Siger stedet igen. |

Når objektet ikke passer, hvor pilene sætter det, slutter linjen med det, der ville gå: den
retning, det passer i på samme flise ("passer drejet, vendt mod syd"), ellers den nærmeste flise,
hvor det passer ("nærmeste sted det passer 1 flise mod vest, vendt mod nord"). For et objekt på
flere fliser siger den første pil og hver drejning, hvilken del af det markøren er på, og hvor
resten går hen ("markøren på kanten mod nord, den strækker sig 1 flise mod syd"); stedets
detaljer siger det samme.

Detaljebufferen indeholder stedets felt og de steder, spillet afviste, med grunden.

En siddeplads' steder kommer først, hvor noget bruges siddende og endnu ikke har en siddeplads: "vendt mod syd, til at bruge Computer siddende". Når et objekt er placeret, siger modden, hvordan det skal bruges: "til at bruge Computer siddende", "bruges fra Kontorstol", "Simmerne kan nå det" eller "ingen Sim kan nå det".

### Flyt og sælg

I Købefunktion: gå med Side ned til et objekt på grunden, og tryk Enter: det samles op, og den samme liste over steder vises, begyndende i det rum, det kom fra. Den første piletast sætter markøren, hvor det stod. Ens objekter vises ét ad gangen, og det seneste køb siger "lige købt".

## Byggefunktion

F3 åbner Byggefunktion. Dens værktøjer er faner, deres stilarter er produkter. Husoversigten siges, når du går ind i Byggefunktion (Kontrol+Skift+L gentager den).

Når du trykker på et værktøj, kommer du til dets modeller i en gruppe for sig med værktøjets navn; Tab går tilbage til værktøjerne. Du har intet i hånden, før du trykker Enter på en model.

### Byggemarkøren

Enter på en stil af mur eller hegn starter byggemarkøren ved den valgte Sim. Den bevæger sig på hjørnerne mellem felterne.

| Tast | Hvad den gør |
|---|---|
| Piletasterne | Ét hjørne; med Skift fem. Siger, hvor den er, hvilke mure der mødes der, og hvad der står der. |
| Mellemrum | Markerer starten. Derefter siger markøren afstanden fra starten, og hvad Enter ville bygge og koste. |
| Enter | Bygger en mur fra starten til markøren. |
| Skift+Enter | Bygger de fire mure i et rum mellem starten og markøren. |
| Kontrol+Enter eller Delete | Fjerner mure. |
| komma, punktum | Vælger en færdig rumstørrelse. |
| Insert | Bygger det færdige rum med nordvesthjørnet ved markøren. |
| Tab, Skift+Tab (eller Kontrol+Side ned, Side op) | Springer til næste eller forrige rum. |
| Side op, Side ned | Etagen ovenover eller nedenunder. |
| Home | Tilbage til den valgte Sim. |
| R | Alt: den frie plads i hver retning, rummet. |
| Escape | Forlader markøren. |

Før der bygges, tjekker modden de nye mure. Hvis de ville spærre en Sim inde, afskære et objekt, lukke en dør eller afskære en del af grunden, siger den det, og et andet tryk på Enter bygger alligevel.

### Gulve, tapet, pøle og terræn

Hvert af disse værktøjer har den samme markør på felter (piletasterne, Skift for fem, Home, R, Escape; Kontrol+F1 for detaljerne):

- **Gulve**: Enter lægger gulv på feltet eller på rektanglet fra en start markeret med Mellemrum; Skift+Enter hele rummet; Kontrol+Enter fjerner.
- **Tapet**: Enter tapetserer hver mur i rummet under markøren; Kontrol+Enter tager det af.
- **Pøle**: Mellemrum markerer et hjørne, Enter graver; modden advarer, før vand afskærer noget. Kontrol+Enter fylder felter op igen. Simmerne skal have en poolstige (i værktøjets liste).
- **Terræn**: jævner fra et markeret felt til markøren, eller hæver og sænker jorden.

**Græs** bruger samme markør: Enter maler feltet under den.

### Døre, vinduer og trapper

Døre og vinduer placeres som møbler, fra en liste over steder: "i væggen mod vest i køkken, fører ud til udenfor, rummets første dør".

Med pilene siger en dør eller et vindue, hvilken væg det ville komme i; at dreje det vælger væggen. Når en dør er placeret, siger modet, om simmerne kan gå igennem, eller hvad der er i vejen. For at ændre den senere: find den under **døre, vinduer og trapper** eller med Kontrol+F. Enter tager den op for at flytte den, Delete fjerner den (spillets Kontrol+klik).

### Taget

Spillet bygger selv taget over dine vægge, så der er ingen markør og intet at placere. Tagværktøjet vælger kun hældning og model, for hele huset. Modellerne har ingen navne i spillet: modet siger deres farve.

## Lydsignaler

Byggemarkøren, placeringen og husets kort spiller en kort lyd ved hvert skridt: gulvet, muren, hegnet, døren eller vinduet på hver side (hver side lyder forskelligt), objekter, et nyt rum, grundens kant, skråninger, og om et objekt kan stå der. **Kontrol+Skift+S** slår dem fra og til.

## Andre taster

| Tast | Hvad den gør |
|---|---|
| Kontrol+Skift+C | Spillets snydekonsol. Pil op og Pil ned vælger en snydekode, det du skriver føjes til den, Enter kører den (igen: gentager), Escape lukker. |
| Kontrol+Skift+V | Fremhæver det valgte kontrolelement på skærmen for en seende hjælper. På grunden går musemarkøren også hen til den ting, der har fokus. |

## Spillets egne taster

Disse er spillets og virker stadig:

| Tast | Hvad den gør |
|---|---|
| F1, F2, F3, F4, F5 | Spillefunktion, Købefunktion, Byggefunktion, Kamerafunktion og Opsætning. |
| P | Pause. |
| 1, 2, 3 | Normal, hurtig, hurtigst hastighed. |
| komma, punktum | Roterer visningen. |
| plus, minus | Zoom ind og ud. |
| J, N | Ja og nej i dialogbokse. |
| Kontrol+S | Gem. |

## Indstillinger

Moddens indstillinger ligger i `SimsAccess\SimsAccess.ini` i spillets mappe, sektionen `[mod]`. De taster, der ændrer dem, gemmer dem med det samme.

| Indstilling | Værdier |
|---|---|
| `narrator` | 2 alt, 1 kun advarsler, 0 fra (Kontrol+N). |
| `cues` | 1 lydsignaler til, 0 fra (Kontrol+Skift+S). |
| `cue_volume` | Lydsignalernes lydstyrke, 0 til 100 (standard 80). |
| `show_focus` | 1 fremhæver det valgte kontrolelement på skærmen (Kontrol+Skift+V). |
| `social` | Hvis samtaler der siges: 3 alle, 2 alle, der taler med din familie, 1 din familie, 0 ingen (Kontrol+Skift+N). |
| `hours` | 1 siger hver ny time (Kontrol+T). |
| `camera_glide` | Hvor længe kameraet er om at glide hen til et objekt, du gennemgår, i millisekunder (standard 800; 0 spillets egen hurtige 250). |

## Når noget går galt

- **Ingen tale overhovedet**: start din skærmlæser før spillet. Moddens log er `SimsAccess\SimsAccess.log`; send den med din rapport.
- **"Spilversionen understøttes ikke"**: spillet er blevet opdateret; installér moddens næste version.
- **En tast gør ingenting**: Kontrol+F1 siger, hvilke taster der virker, hvor du er.

## Ikke færdigt endnu

Vacations, Superstars og Makin' Magics egne ture og skærme er endnu ikke helt dækket. Fotofunktionen og kamerafunktionen samt familiealbummet er kun visuelle og er udeladt.
