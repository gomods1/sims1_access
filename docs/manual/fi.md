---
title: Sims-saavutettavuuden käyttöopas
lang: fi
toc-title: Sisällys
---

Sims-saavutettavuus tekee The Sims Legacy Collectionista pelattavan pelkällä ruudunlukijalla ja
näppäimistöllä. Se lukee jokaisen näkymän, valintaikkunan ja paneelin ruudunlukijasi kautta
(NVDA, JAWS, Lukija ja muut), antaa liikkua niissä samoilla näppäimillä kaikkialla ja lisää
työkalut, joita sokea pelaaja tarvitsee: luettelon tontin esineistä, talon kartan, kertojan sille,
minkä peli vain näyttää, paikat, joihin huonekalut mahtuvat, sekä kohdistimen seinien ja huoneiden
rakentamiseen.

Tämä käyttöopas avautuu pelissä näppäimillä **Control+Vaihto+F1**, pelin kielellä. **Control+F1**
kertoo sen näkymän näppäimet, jossa olet.

## Aloitus

1. Suorita asennusohjelma tai kopioi `version.dll` ja kansio `SimsAccess` pelin kansioon
   (siihen, jossa on `Sims.exe`).
2. Käynnistä ruudunlukija ja sen jälkeen The Sims Legacy Collection Steamista.
3. Kun peli-ikkuna aukeaa, kuulet "Sims-saavutettavuus ladattu". Peli alkaa naapurustonäkymästä.

Modi puhuu pelin kielellä. Vaihda kieli Steamissa: pelin Ominaisuudet, Yleiset, Kieli. Modi puhuu
englantia, ranskaa, saksaa, italiaa, espanjaa, hollantia, tanskaa, ruotsia, norjaa, suomea,
Brasilian portugalia, japania, puolaa, yksinkertaistettua ja perinteistä kiinaa, thaita ja koreaa.

Modi toimii pelin nykyisen Steam-version kanssa. Jos peli päivittyy ja modi sanoo "pelin versiota
ei tueta", asenna modin seuraava versio.

## Liikkuminen: samat näppäimet kaikkialla

Jokainen näkymä, valintaikkuna, paneeli ja tila toimii samalla tavalla. Liikut **ryhmien**
kehässä (valintaikkunan painikkeet, tarpeet, perhe, tavaraluettelo...). Ryhmässä on **kohteita**,
ja jotkin kohteet avaavat syvemmän **tason**.

| Näppäin | Mitä se tekee |
|---|---|
| Sarkain, Vaihto+Sarkain | Seuraava tai edellinen ryhmä. Ensin kerrotaan ryhmän nimi ja kohteiden määrä. |
| Nuoli ylös, nuoli alas (tai nuoli vasemmalle, nuoli oikealle) | Ryhmän edellinen tai seuraava kohde. |
| Home, End | Ryhmän ensimmäinen tai viimeinen kohde. |
| Enter | Aktivoi kohteen. Tasolla (tontin esineiden laji tai huone) avaa sen. |
| Välilyönti | Aktivoi, paitsi tontilla, jossa välilyönti on pelin "seuraava simi" -näppäin. |
| Askelpalautin | Takaisin ylemmälle tasolle. |
| R | Sano kohde uudelleen ryhmänsä kanssa. |
| Escape | Sulkee toimintovalikon; muuten pelin oma Escape. |
| Alt+kirjain | Seuraava ryhmä, jonka nimi alkaa sillä kirjaimella. |
| Control+F1 | Sen näkymän näppäimet, jossa olet. |
| Control+Vaihto+F1 | Tämä käyttöopas. |

Säätimet kertovat nimensä jälkeen, mitä ne ovat, kuten ruudunlukija: "Tallenna, painike",
"Ääniasetukset, välilehti, valittu", "Automaattinen keskitys, valintaruutu, valittu", "Äänet,
liukusäädin, 8, enintään 10", "Maaston tarkkuus, valitsin, Kor.".

- **Liukusäätimet, valitsimet ja luonteen palkit**: nuoli vasemmalle ja nuoli oikealle (tai Page
  Up ja Page Down) muuttavat arvoa. Enter vain sanoo sen uudelleen.
- **Valintaruudut**: Enter valitsee ja poistaa valinnan.
- **Tekstikentät** (perheen ja simien nimet, taustatarinat): siirry kenttään nuolilla ja kirjoita.
  Jokainen kirjain toistetaan, ja Askelpalautin kertoo, minkä se poistaa. Nuoli vasemmalle, nuoli
  oikealle, Home, End ja Delete liikkuvat kentässä. Kun tekstikenttä on näkyvissä, kirjaimet,
  välilyönti ja R kirjoittavat.
- **Pois käytöstä olevat säätimet** sanovat "ei käytettävissä", eikä niitä aktivoida.

Jokaisen näkymän viimeinen ryhmä on **modin ohje**: sen painikkeet kertovat näkymän näppäimet
(kuten Control+F1) ja avaavat tämän käyttöoppaan (kuten Control+Vaihto+F1). Kun peli käynnistyy,
modi sanoo molemmat näppäimet "Sims-saavutettavuus ladattu" -ilmoituksen jälkeen.

## Lisää luettavaa: tarkastelupuskurit

Kaikki ei mahdu yhteen puhuttuun riviin. Modi pitää kahta puskuria, joita voit lukea rivi
riviltä:

- **tiedot**: kaikki juuri sanotusta. Tuotteen arviot, huoneet ja kuvaus; tontin esineen
  toiminnot, huone, etäisyys ja myyntiarvo; tarpeen tai taidon selitys; säätimen työkaluvihje ja
  sen paikka ryhmässään ("3, yhteensä 12, ryhmä ohjaimet").
- **tapahtumat**: mitä tapahtui, uusin ensin: ostot, myynnit, nopeuden muutokset, valintaikkunoiden
  tekstit, jonoon lisätyt toiminnot, kaikki mitä kertoja sanoi (viimeiset 200).

| Näppäin | Mitä se tekee |
|---|---|
| Control+nuoli ylös | Seuraava rivi, syvemmälle puskuriin. |
| Control+nuoli alas | Takaisin kohti alkua. |
| Control+nuoli vasemmalle, Control+nuoli oikealle | Vaihda puskurien välillä. |

## Naapurusto

Naapurustonäkymän pääryhmä on **talot**: jokainen kertoo osoitteensa ja sen, kuka siinä asuu.
Enter talon kohdalla painaa sitä kuten napsautus, ja sen jälkeen tulevat pelin oma kortti tai
kysymykset. Talon kohdalla Control+nuoli ylös lukee sen tiedot: sen, minkä Rakenna-tilan
talon yleiskatsaus kertoisi, talon viimeisimmästä tallennuksesta (huoneet, mitä puuttuu, huoneet,
joihin ei pääse). Control+Vaihto+L sanoo kaiken kerralla.

Muissa ryhmissä ovat pelin painikkeet: perheen luominen, perhesäiliö, naapuruston numero,
asetukset, lopetus.

Talo, jossa ei ole perhettä, avautuu vain Rakenna-tilaan rajattomin rahoin; modi sanoo "ei
perhettä, rajattomasti rahaa" rahasumman sijaan.

## Perheen ja simien luominen

Kaikki näissä näkymissä on ryhmä tai kenttä:

- Kirjoita perheen nimi ja simin etunimi niiden tekstikenttiin.
- **luonne**: nuoli vasemmalle ja nuoli oikealle lisäävät tai poistavat pisteen kustakin
  ominaisuudesta ("4, enintään 10, 6 pistettä jäljellä"). Horoskooppimerkki määräytyy pisteistä.
- **ulkonäkö**: ikä, sukupuoli ja ihonväri sekä pään ja vartalon nuolet. Jokainen pää ja vartalo
  kuvaillaan ("pää 3, yhteensä 137: vaaleat hiukset, polkkatukka").
- Taustatarina on myös tekstikenttä.
- Control+Vaihto ja numero (1–9) tontilla avaavat myöhemmin simin kortin, jossa taustatarinaa voi
  muokata.

## Tontti: Pelaa-tila

Tontilla HUD on ryhmien kehä kuten mikä tahansa näkymä. Sarkain käy ne läpi tässä järjestyksessä:

1. **tila**: aika, rahat, pelin nopeus, ystävät.
2. Valitun simin **Tarpeet**, kukin prosentteina; Enter kertoo, mitä tarve tarkoittaa.
3. **perhe**: perheen simit; Enter valitsee yhden.
4. **toimintojono**: mitä valittu simi aikoo tehdä; Enter peruuttaa toiminnon.
5. **esineet lajeittain** ja **esineet huoneittain**: tontin esineet (katso alempaa).
6. **ohjaimet**, **simin paneelit** (Mieliala, Suhteet, Työ ja taidot, Luonne, Kiinnostukset,
   Talo) ja mikä tahansa avaamasi paneeli.
7. **modin ohje**.

Pelin paneelin kohteessa (tarve, taito, työ, talon arvio) Enter lukee pelin ponnahdusikkunan.

### Simien valinta ja kamera

| Näppäin | Mitä se tekee |
|---|---|
| Välilyönti | Pelin "seuraava simi": seuraava perheenjäsen valitaan ja ilmoitetaan ("Isabel, valittu, Syö"). |
| Control+1–Control+9 | Valitse perheen ensimmäinen–yhdeksäs simi. |
| Control+Vaihto+1–Control+Vaihto+9 | Avaa sen simin kortti (taustatarina). |
| Control+F | Kamera seuraa valittua simiä tai on vapaa. |
| Control+E | Valittu simi yhdellä rivillä: huone, mitä se tekee, kiireelliset tarpeet, mieliala. |
| P | Tauko (pelin näppäin); modi kertoo ajan ja rahat. 1, 2, 3 asettavat nopeuden. |

Kamera seuraa valittua simiä, koska pelin äänet tulevat siitä, mitä näytöllä on. Kun selaat
tontin esineitä, kamera siirtyy kunkin luo ja palaa sen jälkeen simin luo.

### Tontin esineet

Kaikki tontilla on kahdessa ryhmässä: **esineet lajeittain** (simit, sitten Osta-tilan luokat:
istuimet ja sängyt, tasot, kodinkoneet..., sitten muut esineet) ja **esineet huoneittain** (ensin
valitun simin huone, sitten muut, kukin kerroksineen ja kokoineen). Enter lajin tai huoneen
kohdalla luettelee sen esineet, lähin ensin, muodossa "Jääkaappi, keittiö, 6 ruutua
koillissuuntaan" valitusta simistä. Askelpalautin palaa lajeihin tai huoneisiin. Huoneiden nimet
ovat modin arvauksia sen perusteella, mitä niissä on.

Sinun ei tarvitse siirtyä sinne Sarkaimella: Page-näppäimet selaavat esineitä mistä tahansa
tontilla, Pelaa-, Osta- ja Rakenna-tilassa.

| Näppäin | Mitä se tekee |
|---|---|
| Page Down, Page Up | Viimeksi valitun lajin tai huoneen seuraava tai edellinen esine. |
| Vaihto+Page Down, Vaihto+Page Up | Seuraava tai edellinen esineiden laji. |
| Control+Page Down, Control+Page Up | Seuraava tai edellinen huone (ensimmäinen painallus: valitun simin huone). |
| Enter | Esineen toiminnot (Osta-tilassa: nosta se siirrettäväksi). |
| Control+Enter | Lähetä valittu simi esineen luo. |

Simi tai esine, jonka yllä on yhä puhekupla, sanoo senkin ("Bella, Syö, ajattelee: nälkä").

### Toimintovalikot

Enter esineen (tai simin) kohdalla avaa pelin toimintovalikon. Nuoli ylös ja nuoli alas
valitsevat, Enter suorittaa, Escape palaa tason taaksepäin tai sulkee. Kohdat, jotka päättyvät
"...", avaavat lisää vaihtoehtoja. Valittu toiminto ilmoitetaan jonoon lisätyksi ("Juttele,
jonossa"). Kun valikko on tyhjä, modi kertoo syyn ("ei toimintoja; vain lapsille: Leiki").

### Huoneet ja talo

| Näppäin | Mitä se tekee |
|---|---|
| Control+L | Huoneen pohja: koko, ovet ja ikkunat seinittäin ja minne ne johtavat; mitä kunkin seinän vieressä on, löytyy tiedot-puskurista. |
| Control+Vaihto+L | Talon yleiskatsaus: kerrokset, huoneet, arviot, mitä puuttuu (WC, suihku, sängyt, jääkaappi, liesi, puhelin, palovaroitin), huoneet, joihin ei pääse, esineet, joiden luo kukaan ei pääse, pimeät tai epämieluisat huoneet. |

### Talon kartta

**Control+M** ottaa kartan käyttöön: kohdistin kulkee talossa ruutu kerrallaan valitusta simistä
alkaen. Jokainen askel soittaa äänen lattialle ja jokaiselle ympärillä olevalle seinälle,
aidalle, ovelle tai ikkunalle.

| Näppäin | Mitä se tekee |
|---|---|
| Nuolinäppäimet | Yksi ruutu; Vaihdon kanssa viisi. Kertoo, mitä siinä on ja mihin suuntaan se on käännetty, huoneen ja kerroksen kun ne vaihtuvat, seinät, aidat, kaiteet, ovet ja ikkunat sivuittain. |
| Home | Takaisin valittuun simiin. |
| Control+Vaihto+Page Up, Page Down | Ylempi tai alempi kerros. |
| Enter | Siinä olevan asian toimintovalikko. |
| Control+Enter | Lähetä valittu simi sinne. |
| Page Down, Page Up (ja Vaihto, Control) | Tontin esineet kuten aina; kartta siirtyy kuhunkin. |
| R | Kaikki ruudusta. Control+L huoneen pohja. |
| Escape tai Control+M | Poistu kartasta. |

### Kertoja

Peli näyttää monet asiat vain kuvina. Modi sanoo ne niiden tapahtuessa ja tallentaa ne
tapahtumat-puskuriin:

- **Puhekuplat** simien ja esineiden yllä: "Bella ajattelee: nälkä", "Mortimer huutaa: rakko",
  "Bella sanoo: Avaruusoliot", aihe, josta toinen simi ei pidä ("..., ei kiinnosta"), "suhde
  huononee" ja "Roska-astia: kärpäset".
- **Tukitut reitit**: "Bella ei pääse ohi: Tuoli on tiellä".
- **Vieraat**, jotka saapuvat ja lähtevät, aaveet, murtovaras, Viikatemies, pizzalähetti,
  kimppakyyti ja koulubussi (mikä auto tööttää).
- **Viedyt tavarat**, jotka ulosottomies tai murtovaras vie, arvoineen; saapuvat **laskut**.
- **Perheen seuranta**: mitä valittu simi alkaa tehdä, sinun käskystäsi tai omasta aloitteestaan
  ("Cris, omasta aloitteesta: Istu"), milloin se lopettaa, huoneen vaihdokset, laskevat tarpeet,
  punaiseksi muuttuva mieliala, päivän suhdemuutokset ("Marisa ja Cris: +8, nyt 28") sekä
  maksetut tai saadut rahat Osta-tilan ulkopuolella.
- **Matkat**: minne saavuit, ja "taas kotona".

**Control+N** muuttaa sitä, kuinka paljon puhutaan: kaikki, vain varoitukset tai pois. Puhumatta
jääneet rivit menevät silti tapahtumat-puskuriin.

Ääniä, jotka voit kuulla (puhelin, ovikello, hälytykset), ei ilmoiteta.

### Ohjevinkit ja harjoituspeli

Kun pelin ohjejärjestelmällä on vinkki odottamassa, modi sanoo "ohjejärjestelmän vinkki
saatavilla: Control+H lukee sen". **Control+H** avaa sen: vinkki luetaan, tai modi kertoo, mihin
pelin punainen nuoli osoittaa.

Ensimmäisen naapuruston talossa 7 on pelin harjoituspeli. Jokainen vaihe luetaan pelin tekstin
näppäimistöversiona ("Katso sitä lähempää painamalla Siirry lähemmäs -painiketta: Sarkain ryhmään
ohjaimet, nuolinäppäimet kohtaan Siirry lähemmäs, Enter"), ja sen jälkeen kerrotaan, mihin nuoli
osoittaa. Jokaisen vaiheen voi suorittaa näppäimistöllä. Harjoituspelin aikana ensimmäinen Escape
varoittaa; toinen Escape neljän sekunnin sisällä lopettaa sen.

### Pizza ja muut palvelut

Tilaa palveluja puhelimen toiminnoilla. Pizzalähettiä on tervehdittävä: kun hän saapuu, siirry
ryhmään esineet lajeittain, simit, nuoli alas hänen kohdalleen, Enter, Tervehdi. Jos kukaan ei
tervehdi häntä, hän lähtee pizzan kanssa ja rahat palautetaan.

## Osta-tila

F2 avaa Osta-tilan. Tavaraluettelon luokat ovat välilehtiä ja tuotteet painikkeita: jokainen tuote
kertoo nimensä, hintansa, tärkeimmät arvionsa ja kuvauksensa, ja tiedot-puskurissa ovat kaikki sen
arviot ja huoneet, joihin se on tarkoitettu. Kun liikut sivun viimeisen tuotteen ohi, sivu
kääntyy.

| Näppäin | Mitä se tekee |
|---|---|
| Control+R | Rajaa luettelo yhdenlaisen huoneen tuotteisiin (keittiö, kylpyhuone, makuuhuone, olohuone, ruokailuhuone, työhuone, ulkona) tai kaikkiin. |
| Askelpalautin | Luettelon "Takaisin". |
| Enter tuotteen kohdalla | Aloita sen sijoittaminen. |

### Esineen sijoittaminen

Kun pidät esinettä, modi on jo löytänyt paikat, joihin se mahtuu **ja joissa sitä voi käyttää**
(pelin sijoitustarkistus, huoneen seinät ja ovet, tila, jonka simit tarvitsevat käyttääkseen
sitä). Paikka kertoo huoneensa, minkä vieressä se on ja mihin suuntaan se on käännetty.

| Näppäin | Mitä se tekee |
|---|---|
| Page Down, Page Up (tai Sarkain) | Paikat, parhaat ensin. |
| Control+Page Down, Control+Page Up | Huoneet, joissa on paikkoja. |
| Vaihto+Page Down, Vaihto+Page Up (tai pilkku, piste) | Muut suunnat samassa paikassa. |
| Control+Vaihto+Page Down, Page Up | Tontin esineiden vieressä, lähin ensin. |
| Nuolinäppäimet | Siirrä sitä sen sijaan ruutu kerrallaan (Vaihto: viisi); kertoo, mahtuuko se ja mihin se törmää. |
| Enter | Osta se siihen (tai laske se alas siirrettäessä). |
| Escape | Peruuta (siirretty esine palaa paikalleen). |
| Delete | Myy pidetty esine; hyvitys kerrotaan. |
| R | Sano paikka uudelleen. |

Tiedot-puskurissa ovat paikan ruutu ja paikat, jotka peli hylkäsi, syineen.

### Siirtäminen ja myyminen

Osta-tilassa siirry Page Downilla tontin esineeseen ja paina Enter: se nostetaan ja sama paikkojen
luettelo tulee esiin. Samanlaiset esineet luetellaan yksi kerrallaan, ja viimeisin ostos sanoo
"juuri ostettu".

## Rakenna-tila

F3 avaa Rakenna-tilan. Sen työkalut ovat välilehtiä ja niiden tyylit tuotteita. Talon
yleiskatsaus luetaan, kun siirryt Rakenna-tilaan (Control+Vaihto+L toistaa sen).

### Rakennuskohdistin

Enter seinä- tai aitatyylin kohdalla käynnistää rakennuskohdistimen valitun simin kohdalta. Se
liikkuu ruutujen välisissä kulmissa.

| Näppäin | Mitä se tekee |
|---|---|
| Nuolinäppäimet | Yksi kulma; Vaihdon kanssa viisi. Kertoo, missä se on, mitkä seinät kohtaavat siinä ja mitä siinä on. |
| Välilyönti | Merkitse alku. Sen jälkeen kohdistin kertoo etäisyyden alusta ja mitä Enter rakentaisi ja mitä se maksaa. |
| Enter | Rakenna seinä alusta kohdistimeen. |
| Vaihto+Enter | Rakenna huoneen neljä seinää alun ja kohdistimen väliin. |
| Control+Enter tai Delete | Poista seiniä. |
| pilkku, piste | Valitse valmis huonekoko. |
| Insert | Rakenna valmis huone niin, että sen luoteiskulma on kohdistimessa. |
| Sarkain, Vaihto+Sarkain (tai Control+Page Down, Page Up) | Hyppää seuraavaan tai edelliseen huoneeseen. |
| Page Up, Page Down | Ylempi tai alempi kerros. |
| Home | Takaisin valittuun simiin. |
| R | Kaikki: vapaa tila joka suuntaan, huone. |
| Escape | Poistu kohdistimesta. |

Ennen rakentamista modi tarkistaa uudet seinät. Jos ne sulkisivat simin sisään, estäisivät pääsyn
esineen luo, sulkisivat oven tai erottaisivat osan tontista, modi kertoo sen, ja toinen Enter
rakentaa silti.

### Lattiat, tapetit, uima-altaat ja maasto

Jokaisella näistä työkaluista on sama kohdistin ruuduilla (nuolinäppäimet, Vaihto viidelle, Home,
R, Escape; Control+F1 kertoo yksityiskohdat):

- **Lattiat**: Enter päällystää ruudun tai suorakulmion Välilyönnillä merkitystä alusta;
  Vaihto+Enter koko huoneen; Control+Enter poistaa.
- **Tapetit**: Enter tapetoi kohdistimen alla olevan huoneen jokaisen seinän; Control+Enter
  poistaa tapetin.
- **Uima-altaat**: Välilyönti merkitsee kulman, Enter kaivaa; modi varoittaa ennen kuin vesi
  erottaa jotain. Control+Enter täyttää ruudut takaisin. Simit tarvitsevat altaan tikkaat (ne ovat
  työkalun listassa).
- **Maasto**: tasoita merkitystä ruudusta kohdistimeen tai nosta ja laske maata.

### Ovet, ikkunat ja portaat

Ovet ja ikkunat sijoitetaan kuten huonekalut, paikkojen luettelosta: "keittiö: länsiseinässä,
johtaa tilaan ulkona, huoneen ensimmäinen ovi".

## Äänivihjeet

Rakennuskohdistin, sijoittaminen ja talon kartta soittavat lyhyen äänen jokaisella askeleella:
lattia, kunkin sivun seinä, aita, ovi tai ikkuna (jokainen sivu kuulostaa erilaiselta), esineet,
huoneen vaihtuminen, tontin reuna, rinteet ja se, mahtuuko esine. **Control+Vaihto+S** kytkee ne
pois ja päälle.

## Muut näppäimet

| Näppäin | Mitä se tekee |
|---|---|
| Control+Vaihto+C | Pelin huijauskonsoli. Nuoli ylös ja nuoli alas valitsevat huijauksen, kirjoittaminen lisää siihen, Enter suorittaa sen (uudelleen: toistaa), Escape sulkee. |
| Control+Vaihto+V | Korosta valittu säädin näytöllä näkevää auttajaa varten. |

## Pelin omat näppäimet

Nämä ovat pelin omia ja toimivat yhä:

| Näppäin | Mitä se tekee |
|---|---|
| F1, F2, F3, F4, F5 | Pelaa, Osta, Rakenna, Kamera ja Peliasetukset. |
| P | Tauko. |
| 1, 2, 3 | Normaali, nopea, nopein nopeus. |
| pilkku, piste | Käännä näkymää. |
| plus, miinus | Lähennä ja loitonna. |
| K, E | Kyllä ja ei valintaikkunoissa. |
| Control+S | Tallenna. |

## Asetukset

Modin asetukset ovat tiedostossa `SimsAccess\SimsAccess.ini` pelin kansiossa, osiossa `[mod]`.
Asetuksia muuttavat näppäimet tallentavat ne heti.

| Asetus | Arvot |
|---|---|
| `narrator` | 2 kaikki, 1 vain varoitukset, 0 pois (Control+N). |
| `cues` | 1 äänivihjeet päällä, 0 pois (Control+Vaihto+S). |
| `cue_volume` | Äänivihjeiden voimakkuus, 0–100 (oletus 80). |
| `show_focus` | 1 korostaa valitun säätimen näytöllä (Control+Vaihto+V). |

## Kun jokin menee vikaan

- **Ei puhetta lainkaan**: käynnistä ruudunlukija ennen peliä. Modin loki on
  `SimsAccess\SimsAccess.log`; lähetä se ilmoituksesi mukana.
- **"Pelin versiota ei tueta"**: peli on päivittynyt; asenna modin seuraava versio.
- **Näppäin ei tee mitään**: Control+F1 kertoo, mitkä näppäimet toimivat siellä, missä olet.

## Vielä tekemättä

Vacationin, Superstarin ja Makin' Magicin omat matkat ja näkymät eivät ole vielä kokonaan
tuettuja. Valokuva- ja kameratila sekä perhealbumi ovat pelkästään visuaalisia, ja ne on jätetty
pois.
