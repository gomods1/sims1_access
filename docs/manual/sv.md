---
title: Handbok för Tillgänglighet för The Sims
lang: sv
toc-title: Innehåll
---

Tillgänglighet för The Sims gör The Sims Legacy Collection spelbart med en skärmläsare och bara tangentbordet. Modden läser upp varje skärm, dialogruta och panel genom din skärmläsare (NVDA, JAWS, Skärmläsaren och andra), låter dig röra dig genom dem med samma tangenter överallt och lägger till verktyg som en blind spelare behöver: en lista över tomtens föremål, en karta över huset, en berättare för det spelet bara visar, platser där möbler får plats och en markör för att bygga väggar och rum.

Den här handboken öppnas med **Kontroll+Skift+F1** i spelet, på spelets språk. **Kontroll+F1** säger tangenterna på skärmen du är på.

## Kom igång

1. Kör installationsprogrammet, eller kopiera `version.dll` och mappen `SimsAccess` till spelets mapp (den med `Sims.exe`).
2. Starta din skärmläsare och starta sedan The Sims Legacy Collection från Steam.
3. När spelfönstret öppnas hör du "Tillgänglighet för The Sims laddad". Spelet startar på grannskapsskärmen.

Modden talar spelets språk. För att byta det öppnar du Steam, sedan spelets Egenskaper, Allmänt, Språk. Modden talar engelska, franska, tyska, italienska, spanska, nederländska, danska, svenska, norska, finska, brasiliansk portugisiska, japanska, polska, förenklad och traditionell kinesiska, thai och koreanska.

Modden fungerar med den nuvarande Steam-versionen av spelet. Om spelet uppdateras och modden säger "spelversionen stöds inte", installera moddens nästa version.

## Röra sig: samma tangenter överallt

Varje skärm, dialogruta, panel och läge fungerar på samma sätt. Det du rör dig genom är en ring av **grupper** (en dialogrutas knappar, behoven, familjen, katalogen...). En grupp innehåller **objekt**, och vissa objekt öppnar en djupare **nivå**.

| Tangent | Vad den gör |
|---|---|
| Tabb, Skift+Tabb | Nästa eller föregående grupp. Gruppens namn och antal objekt sägs först. |
| Pil upp, Pil ned (eller Pil vänster, Pil höger) | Föregående eller nästa objekt i gruppen. |
| Home, End | Första eller sista objektet i gruppen. |
| Enter | Aktiverar objektet. På en nivå (en sort eller ett rum bland tomtens föremål): öppnar den. |
| Mellanslag | Aktiverar, utom på tomten, där Mellanslag är spelets tangent för "nästa sim". |
| Backspace | En nivå tillbaka. |
| R | Säger objektet igen, med dess grupp. |
| Escape | Stänger interaktionsmenyn; annars spelets egen Escape. |
| Alt+bokstav | Nästa grupp vars namn börjar på den bokstaven. |
| Kontroll+F1 | Tangenterna på skärmen du är på. |
| Kontroll+Skift+F1 | Den här handboken. |

Kontroller säger vad de är efter sitt namn, som en skärmläsare gör: "Spara, knapp", "Ljudalternativ, flik, vald", "Auto-centrering, kryssruta, markerad", "Ljudeffekter, skjutreglage, 8 av 10", "Detaljerad terräng, väljare, Hög".

- **Skjutreglage, väljare och personlighetsstaplar**: Pil vänster och Pil höger (eller Page Up och Page Down) ändrar värdet. Enter säger det bara igen.
- **Kryssrutor**: Enter markerar och avmarkerar dem.
- **Textfält** (namn på familjer och simmar, bakgrunder): gå till fältet med piltangenterna och skriv. Varje bokstav upprepas, och Backspace säger vad den raderar. Pil vänster, Pil höger, Home, End och Delete flyttar i fältet. Så länge ett textfält finns på skärmen skriver bokstäver, Mellanslag och R.
- **Inaktiverade kontroller** säger "inte tillgänglig" och aktiveras inte.

Den sista gruppen på varje skärm är **modhjälp**: dess knappar säger tangenterna på den skärmen (som Kontroll+F1 gör) och öppnar den här handboken (som Kontroll+Skift+F1 gör). När spelet startar säger modden båda tangenterna efter "Tillgänglighet för The Sims laddad".

## Läsa mer: granskningsbuffertarna

Allt får inte plats i en uppläst rad. Modden har två buffertar som du kan läsa rad för rad:

- **detaljer**: allt om det som just sades. En produkts betyg, rum och beskrivning; ett föremåls åtgärder, rum, avstånd och försäljningsvärde på tomten; förklaringen av ett behov eller en färdighet; en kontrolls verktygstips och dess plats i gruppen ("3 av 12, kontroller").
- **händelser**: vad som har hänt, nyast först: köp, försäljningar, byten av hastighet, texter i dialogrutor, åtgärder i kö, allt som berättaren sagt (de senaste 200).

| Tangent | Vad den gör |
|---|---|
| Kontroll+Pil upp | Nästa rad, djupare in i bufferten. |
| Kontroll+Pil ned | Tillbaka mot början. |
| Kontroll+Pil vänster, Kontroll+Pil höger | Växlar mellan buffertarna. |

## Grannskapet

Grannskapsskärmens huvudgrupp är **hus**: varje hus säger sin adress och vem som bor där. Enter på ett hus aktiverar det, som ett klick gör, och sedan följer spelets eget kort eller frågor. På ett hus läser Kontroll+Pil upp dess detaljer: det som Bygglägets husöversikt skulle säga, enligt husets senaste sparning (rum, vad som saknas, rum utan ingång). Kontroll+Skift+L säger allt på en gång.

De andra grupperna innehåller spelets knappar: skapa en familj, familjelådan, grannskapets nummer, alternativ, avsluta.

Ett hus utan familj öppnas bara i Byggläge, med obegränsade pengar; modden säger "ingen familj, obegränsade pengar" i stället för en summa.

## Skapa en familj och simmar

Allt på de här skärmarna är en grupp eller ett fält:

- Skriv familjenamnet och simmens förnamn i deras textfält.
- **personlighet**: Pil vänster och Pil höger ger eller tar en poäng från varje egenskap ("4 av 10, 6 poäng kvar"). Stjärntecknet följer av poängen.
- **utseende**: ålder, kön och hudfärg, och pilarna för huvud och kropp. Varje huvud och kropp beskrivs ("huvud 3 av 137: blont hår, page").
- Bakgrunden är också ett textfält.
- Kontroll+Skift och en siffra (1 till 9) på tomten öppnar senare en simmes kort, där bakgrunden kan redigeras.

## Tomten: Boendeläge

På en tomt är kontrollpanelen en ring av grupper som vilken skärm som helst. Tabb går igenom dem i den här ordningen:

1. **status**: tid, pengar, spelets hastighet, vänner.
2. **Behov** för den valda simmen, vart och ett i procent; Enter säger vad behovet betyder.
3. **familj**: familjens simmar; Enter väljer en.
4. **åtgärdskö**: vad den valda simmen ska göra; Enter avbryter en åtgärd.
5. **föremål efter sort** och **föremål efter rum**: tomtens föremål (se nedan).
6. **kontroller**, **simpaneler** (Humör, Förhållanden, Jobb och Färdigheter, Personlighet, Intressen, Hus) och varje panel du öppnar.
7. **modhjälp**.

På ett objekt i en panel i Boendeläge (ett behov, en färdighet, ett jobb, ett betyg för huset) läser Enter upp spelets popupruta för det.

Jobbet säger arbetstiderna och när samåkningen kommer. Varje färdighet säger sin nivå och, när en befordran kräver mer, hur mycket: "Matlagning, 2 av 10, nästa befordran kräver 4". Spelets menyer har inget alternativ för att säga upp sig: en sim som fortsätter att missa jobbet får sparken, och tidningen och datorn erbjuder andra jobb.

### Välja simmar och kameran

| Tangent | Vad den gör |
|---|---|
| Mellanslag | Spelets "nästa sim": nästa familjemedlem väljs och sägs ("Isabel, vald, Ät"). |
| Kontroll+1 till Kontroll+9 | Väljer familjens första till nionde sim. |
| Kontroll+Skift+1 till Kontroll+Skift+9 | Öppnar den simmens kort (bakgrund). |
| Kontroll+Skift+F | Kameran följer den valda simmen, eller är fri. |
| Kontroll+E | Den valda simmen på en rad: rum, vad den gör, akuta behov, humör. |
| Kontroll+Skift+E | Hur en sim ser ut: huvud och kropp ("huvud: svart hår, kort; kropp: kostym, svart"). Simmen som har fokus bland tomtens föremål, annars den valda. En simmes detaljer (Kontroll+Pil upp på den) säger det också. |
| Kontroll+T | Säger varje ny timme på klockan ("klockan är 14:00"), eller slutar med det. Användbart på högsta hastigheten. |
| P | Paus (spelets tangent); modden säger tid och pengar. 1, 2, 3 ställer in hastigheten. |

Kameran följer den valda simmen, eftersom spelets ljud kommer från det som syns på skärmen. Medan du bläddrar bland tomtens föremål går kameran till varje föremål och kommer sedan tillbaka till simmen. Den glider dit långsamt, så att bilden inte rusar (inställningen `camera_glide`).

Zoomen och kamerans riktning sägs när de ändras (plus och minus zoomar, komma och punkt vrider vyn), och de finns också i statusraden: "kamera: mellanvy, vänd mot nordväst". Riktningen är det som skärmens överkant visar. Modets norr vrids inte med kameran.

### Tomtens föremål

Allt på tomten finns i två grupper: **föremål efter sort** (simmar, sedan Köplägets kategorier: sittplatser, ytor, hushållsapparater..., sedan andra föremål) och **föremål efter rum** (först den valda simmens rum, sedan de andra, vart och ett med våning och storlek). Enter på en sort eller ett rum listar dess föremål, närmast först, som "Kylskåp, kök, 6 rutor åt nordost" från den valda simmen. Backspace går tillbaka till sorterna eller rummen. Rummens namn är moddens gissning utifrån vad som finns i dem.

Du behöver inte tabba dit: Page-tangenterna bläddrar bland föremålen var du än är på tomten, i Boendeläge, Köpläge och Byggläge.

| Tangent | Vad den gör |
|---|---|
| Page Down, Page Up | Nästa eller föregående föremål av den sort eller i det rum du valde senast. |
| Skift+Page Down, Skift+Page Up | Nästa eller föregående sort av föremål. |
| Kontroll+Page Down, Kontroll+Page Up | Nästa eller föregående rum (första trycket: den valda simmens rum). |
| Enter | Föremålets interaktioner (i Köpläge: plockar upp det för att flytta det). På en rad med likadana saker ("Fällstol, 4 likadana"): listar dem en i taget, närmast först; Backspace går tillbaka. |
| Kontroll+Enter | Låter den valda simmen gå till föremålet. |
| Kontroll+F | Sök något: skriv en del av namnet ("tel" hittar telefonen, accenter spelar ingen roll). Pil upp och ned går igenom träffarna, Enter agerar på den, Kontroll+Enter skickar dit simmen, Delete tar bort en dörr eller ett fönster, Escape går tillbaka dit du var. |
| Delete | På en dörr eller ett fönster, i köp- eller byggläge: tar bort det och ger pengarna tillbaka. |

En sim eller ett föremål som fortfarande visar en bubbla säger den också ("Bella, Ät, tänker: hunger"). Något som ingen sim kan nå från tomtens ingång säger "ingen sim kan nå den".

Efter simmarna kommer **problem**: det som behöver uppmärksamhet, som trasiga saker, det som behöver städas, vattenpölar, skräp, aska, eld och allt med flugor över. En tom matskål för husdjur finns också här; i alla listor säger en matskål om det finns mat i den. Sist kommer **dörrar, fönster och trappor**.

### Interaktionsmenyer

Enter på ett föremål (eller på en sim) öppnar spelets interaktionsmeny. Pil upp och Pil ned väljer, Enter utför, Escape går en nivå tillbaka eller stänger. Alternativ som slutar på "..." öppnar fler val. Den valda åtgärden sägs som köad ("Prata, i kö"). När menyn är tom säger modden varför ("inga interaktioner; bara för barn: Lek").

### Rum och huset

| Tangent | Vad den gör |
|---|---|
| Kontroll+L | Rummets planlösning: storlek, dörrar och fönster för varje vägg och vart de leder; vad som står längs varje vägg finns i detaljbufferten. |
| Kontroll+Skift+L | Husöversikten: våningar, rum, betyg, vad som saknas (toalett, dusch, sängar, kylskåp, spis, telefon, brandvarnare), rum utan ingång, föremål som ingen kan nå, mörka eller ogillade rum. |

### Husets karta

**Kontroll+M** slår på kartan: en markör som går genom huset ruta för ruta, med början vid den valda simmen. Varje steg spelar ett ljud för golvet och för varje vägg, staket, dörr eller fönster runt om.

| Tangent | Vad den gör |
|---|---|
| Piltangenterna | En ruta; med Skift fem. Säger vad som står där och åt vilket håll det är vänt, rummet och våningen när de byts, väggarna, staketen, räckena, dörrarna och fönstren på varje sida. Väggar, staket och fönster stoppar markören; dörrar släpper igenom den. |
| Alt+Piltangenterna | Samma sak, genom väggar. |
| Komma, punkt | Vad varje steg säger: allt, bara föremål, bara golv (med namn, "Ljus granitplatta"), bara väggar, bara rum. |
| Home | Tillbaka till den valda simmen. |
| Kontroll+Skift+Page Up, Page Down | Våningen ovanför eller under. |
| Enter | Interaktionsmenyn för det som står där (något på ett bord eller skrivbord före bordet: datorn, inte dess skrivbord). |
| Kontroll+Enter | Låter den valda simmen gå dit. |
| Page Down, Page Up (och Skift, Kontroll) | Tomtens föremål som vanligt; kartan går till vart och ett. |
| R | Allt om rutan. Kontroll+L rummets planlösning. |
| Escape eller Kontroll+M | Lämnar kartan. |

### Berättaren

Spelet visar många saker bara som bilder. Modden säger dem när de händer och sparar dem i händelsebufferten:

- **Bubblor** över simmar och föremål: "Bella tänker: hunger", "Mortimer skriker: blåsa", "Bella säger: Utomjordingar", ett ämne som den andra simmen ogillar ("..., inte intresserad"), "relationen blir sämre" och "Papperskorg: flugor".
- **Blockerade vägar**: "Bella kommer inte förbi: Stol står i vägen".
- **Besökare** som kommer och går, spöken, inbrottstjuven, Liemannen, pizzabudet, samåkningen och skolbussen (vilken bil som tutar).
- **Saker som tas** av kronofogden eller en inbrottstjuv, med deras värde; **räkningar** som kommer.
- **Familjevakten**: vad den valda simmen börjar göra, på din order eller på eget initiativ ("Cris, på eget initiativ: Sitt"), när den är klar eller blir avbruten ("Cris avbröt Sitt för Panik; det väntar i kön"), byten av rum, behov som blir låga, humöret som blir rött, dagens ändringar i förhållanden ("Marisa med Cris: +8, nu 28"; spelets relationspanel visar också de här siffrorna), och pengar som betalas eller tas emot utanför Köpläge.
- **Resor**: vart du har kommit, och "hemma igen".

**Kontroll+N** ändrar hur mycket som sägs: allt, bara varningar eller av. Rader som inte sägs hamnar ändå i händelsebufferten.

**Kontroll+Skift+N** väljer vems samtal som sägs (vad simmarna säger, deras sociala interaktioner, relationsändringarna de för med sig): alla som pratar med din familj (från början), alla på tomten, bara din familj eller ingen.

Ljud som du kan höra (telefonen, dörrklockan, larm) meddelas inte.

Handlingar med en annan sim nämner den simmen, vem som än började: "Homer: Hälsa, med Elvira".

### Hjälptips och övningen

När spelets hjälpsystem har ett tips som väntar säger modden "det finns ett tips från hjälpsystemet: Kontroll+H läser det". **Kontroll+H** öppnar det: tipset läses upp, eller modden säger vad spelets röda pil pekar på.

Hus 7 i det första grannskapet kör spelets övning. Varje steg läses som en tangentbordsversion av spelets text ("tryck på knappen Zooma in: Tabb till kontroller, piltangenterna till Zooma in, Enter"), följt av vad pilen pekar på. Varje steg kan klaras med tangentbordet. Första Escape under övningen varnar dig; en andra Escape inom fyra sekunder lämnar den.

### Pizza och andra tjänster

Använd telefonens interaktioner för att ringa efter tjänster. Pizzabudet måste hälsas på: när han kommer, gå till föremål efter sort, simmar, Pil ned till honom, Enter, Hälsa. Om ingen hälsar på honom går han med pizzan och pengarna kommer tillbaka.

## Köpläge

F2 öppnar Köpläge. Katalogens kategorier är flikar och dess produkter är knappar: varje produkt säger sitt namn, pris, viktigaste betyg och beskrivning, och detaljbufferten innehåller alla dess betyg och de rum den är tänkt för. Går du förbi en sidas sista produkt vänds sidan.

| Tangent | Vad den gör |
|---|---|
| Kontroll+R | Begränsar katalogen till produkter för en sorts rum (kök, badrum, sovrum, vardagsrum, matsal, arbetsrum, utomhus) eller alla. |
| Backspace | Katalogens "Backa". |
| Enter på en produkt | Börjar placera den. |

### Placera ett föremål

Medan du håller ett föremål har modden redan hittat platserna där det får plats **och kan användas** (spelets placeringskontroll, rummets väggar och dörrar, utrymmet simmarna behöver för att använda det). Platsen säger sitt rum, vad den står bredvid och åt vilket håll den är vänd.

| Tangent | Vad den gör |
|---|---|
| Page Down, Page Up (eller Tabb) | Platserna, de bästa först. |
| Kontroll+Page Down, Kontroll+Page Up | Rummen som har platser. |
| Skift+Page Down, Skift+Page Up (eller komma, punkt) | De andra riktningarna på samma plats. |
| Kontroll+Skift+Page Down, Page Up | Bredvid tomtens föremål, närmast först. |
| Piltangenterna | Flyttar det i stället en ruta (Skift: fem) och säger om det får plats och vad det stöter emot. Väggar, staket och fönster stoppar det; dörrar släpper igenom det. |
| Alt+Piltangenterna | Samma sak, genom väggar. |
| Enter | Köper det där (eller ställer ner det, när du flyttar). |
| Escape | Avbryter (ett flyttat föremål går tillbaka). |
| Delete | Säljer föremålet du håller; återbetalningen sägs. |
| R | Säger platsen igen. |

Detaljbufferten innehåller platsens ruta och de platser spelet avvisade, med skälet.

En sittplats platser kommer först där något används sittande och ännu inte har någon sittplats: "vänd mot söder, för att använda Dator sittande". När ett föremål har placerats säger modden hur det kommer att användas: "för att använda Dator sittande", "används från Kontorsstol", "simmarna kan nå den" eller "ingen sim kan nå den".

### Flytta och sälja

I Köpläge, gå med Page Down till ett föremål på tomten och tryck Enter: det plockas upp och samma lista över platser visas, med början i rummet det kom från. Den första pilen sätter markören där det stod. Likadana föremål listas ett i taget, och det senaste köpet säger "nyss köpt".

## Byggläge

F3 öppnar Byggläge. Dess verktyg är flikar, deras stilar är produkter. Husöversikten läses upp när du går in i Byggläge (Kontroll+Skift+L upprepar den).

När du trycker på ett verktyg kommer du till dess modeller, i en egen grupp med verktygets namn; Tab går tillbaka till verktygen. Du har inget i handen förrän du trycker Enter på en modell.

### Byggmarkören

Enter på en stil av vägg eller staket startar byggmarkören vid den valda simmen. Den rör sig på hörnen mellan rutorna.

| Tangent | Vad den gör |
|---|---|
| Piltangenterna | Ett hörn; med Skift fem. Säger var den är, vilka väggar som möts där och vad som står där. |
| Mellanslag | Markerar starten. Sedan säger markören avståndet från starten och vad Enter skulle bygga och kosta. |
| Enter | Bygger en vägg från starten till markören. |
| Skift+Enter | Bygger de fyra väggarna i ett rum mellan starten och markören. |
| Kontroll+Enter eller Delete | Tar bort väggar. |
| komma, punkt | Väljer en färdig rumsstorlek. |
| Insert | Bygger det färdiga rummet med nordvästra hörnet vid markören. |
| Tabb, Skift+Tabb (eller Kontroll+Page Down, Page Up) | Hoppar till nästa eller föregående rum. |
| Page Up, Page Down | Våningen ovanför eller under. |
| Home | Tillbaka till den valda simmen. |
| R | Allt: det fria utrymmet åt varje håll, rummet. |
| Escape | Lämnar markören. |

Innan något byggs kontrollerar modden de nya väggarna. Om de skulle stänga in en sim, skära av ett föremål, stänga en dörr eller stänga av en del av tomten säger den det, och en andra Enter bygger ändå.

### Golv, tapeter, pooler och terräng

Vart och ett av de här verktygen har samma markör på rutor (piltangenterna, Skift för fem, Home, R, Escape; Kontroll+F1 för detaljerna):

- **Golv**: Enter lägger golv på rutan, eller på rektangeln från en start markerad med Mellanslag; Skift+Enter hela rummet; Kontroll+Enter tar bort.
- **Tapeter**: Enter tapetserar varje vägg i rummet under markören; Kontroll+Enter tar bort tapeten.
- **Pooler**: Mellanslag markerar ett hörn, Enter gräver; modden varnar innan vatten skär av något. Kontroll+Enter fyller igen rutor. Simmar behöver en poolstege (i verktygets lista).
- **Terräng**: jämnar ut från en markerad ruta till markören, eller höjer och sänker marken.

**Gräs** använder samma markör: Enter målar rutan under den.

### Dörrar, fönster och trappor

Dörrar och fönster placeras som möbler, från en lista över platser: "i väggen i väster i kök, leder till utomhus, rummets första dörr".

Med pilarna säger en dörr eller ett fönster vilken vägg det skulle hamna i; att vrida det väljer väggen. När en dörr är placerad säger modet om simmarna kan gå igenom, eller vad som står i vägen. För att ändra den senare: hitta den under **dörrar, fönster och trappor** eller med Kontroll+F. Enter tar upp den för att flytta den, Delete tar bort den (spelets Kontroll+klick).

### Taket

Spelet bygger taket själv över dina väggar, så det finns ingen markör och inget att placera. Takverktyget väljer bara lutning och modell, för hela huset. Modellerna har inga namn i spelet: modet säger deras färg.

## Ljudsignaler

Byggmarkören, placeringen och husets karta spelar ett kort ljud vid varje steg: golvet, väggen, staketet, dörren eller fönstret på varje sida (varje sida låter olika), föremål, ett nytt rum, tomtens kant, sluttningar, och om ett föremål får plats. **Kontroll+Skift+S** stänger av och sätter på dem.

## Andra tangenter

| Tangent | Vad den gör |
|---|---|
| Kontroll+Skift+C | Spelets fuskkonsol. Pil upp och Pil ned väljer ett fusk, det du skriver läggs till, Enter kör det (igen: upprepar), Escape stänger. |
| Kontroll+Skift+V | Lyser upp den valda kontrollen på skärmen för en seende hjälpare. På tomten går muspekaren också till saken som har fokus. |

## Spelets egna tangenter

De här hör till spelet och fungerar fortfarande:

| Tangent | Vad den gör |
|---|---|
| F1, F2, F3, F4, F5 | Boendeläge, Köpläge, Byggläge, Kameraläge och Alternativ. |
| P | Paus. |
| 1, 2, 3 | Normal, snabb, snabbaste hastighet. |
| komma, punkt | Vrider vyn. |
| plus, minus | Zoomar in och ut. |
| J, N | Ja och nej i dialogrutor. |
| Kontroll+S | Spara. |

## Inställningar

Moddens inställningar finns i `SimsAccess\SimsAccess.ini` i spelets mapp, avsnittet `[mod]`. Tangenterna som ändrar dem sparar dem direkt.

| Inställning | Värden |
|---|---|
| `narrator` | 2 allt, 1 bara varningar, 0 av (Kontroll+N). |
| `cues` | 1 ljudsignaler på, 0 av (Kontroll+Skift+S). |
| `cue_volume` | Ljudsignalernas volym, 0 till 100 (standard 80). |
| `show_focus` | 1 lyser upp den valda kontrollen på skärmen (Kontroll+Skift+V). |
| `social` | Vems samtal som sägs: 3 alla, 2 alla som pratar med din familj, 1 din familj, 0 ingen (Kontroll+Skift+N). |
| `hours` | 1 säger varje ny timme (Kontroll+T). |
| `camera_glide` | Hur länge kameran glider till ett föremål du bläddrar till, i millisekunder (standard 800; 0 spelets egna snabba 250). |

## När något går fel

- **Inget tal alls**: starta din skärmläsare före spelet. Moddens logg är `SimsAccess\SimsAccess.log`; skicka med den i din rapport.
- **"Spelversionen stöds inte"**: spelet har uppdaterats; installera moddens nästa version.
- **En tangent gör ingenting**: Kontroll+F1 säger vilka tangenter som fungerar där du är.

## Inte klart än

Vacations, Superstars och Makin' Magics egna resor och skärmar täcks inte helt än. Fotoläget och kameraläget och familjealbumet är bara visuella och har utelämnats.
