---
title: Handbuch zu Barrierefreiheit für Die Sims
lang: de
toc-title: Inhalt
---

Barrierefreiheit für Die Sims macht The Sims Legacy Collection mit einem Screenreader und nur mit der Tastatur spielbar. Der Mod liest jeden Bildschirm, jeden Dialog und jedes Fenster über Deinen Screenreader vor (NVDA, JAWS, Sprachausgabe und andere), lässt Dich überall mit denselben Tasten durch sie gehen und fügt Werkzeuge hinzu, die ein blinder Spieler braucht: eine Liste der Objekte des Grundstücks, einen Plan des Hauses, einen Erzähler für das, was das Spiel nur zeigt, Plätze, an die ein Möbelstück passt, und einen Cursor zum Bauen von Wänden und Räumen.

Im Spiel öffnet **Strg+Umschalt+F1** dieses Handbuch, in der Sprache des Spiels. **Strg+F1** sagt die Tasten des Bildschirms, auf dem Du gerade bist.

## Erste Schritte

1. Starte das Installationsprogramm oder kopiere `version.dll` und den Ordner `SimsAccess` in den Ordner des Spiels (den mit `Sims.exe`).
2. Starte Deinen Screenreader und dann The Sims Legacy Collection über Steam.
3. Wenn sich das Spielfenster öffnet, hörst Du „Barrierefreiheit für Die Sims geladen“. Das Spiel beginnt auf dem Bildschirm der Nachbarschaft.

Der Mod spricht in der Sprache des Spiels. Um sie zu ändern, öffne Steam, dann die Eigenschaften des Spiels, Allgemein, Sprache. Der Mod spricht Englisch, Französisch, Deutsch, Italienisch, Spanisch, Niederländisch, Dänisch, Schwedisch, Norwegisch, Finnisch, brasilianisches Portugiesisch, Japanisch, Polnisch, vereinfachtes und traditionelles Chinesisch, Thai und Koreanisch.

Der Mod funktioniert mit der aktuellen Steam-Version des Spiels. Wenn das Spiel aktualisiert wird und der Mod „nicht unterstützte Spielversion“ sagt, installiere die nächste Version des Mods.

## Bewegen: überall dieselben Tasten

Jeder Bildschirm, jeder Dialog, jedes Fenster und jeder Modus funktioniert gleich. Du gehst durch einen Ring von **Gruppen** (die Schalter eines Dialogs, die Bedürfnisse, die Familie, der Katalog...). Eine Gruppe enthält **Elemente**, und manche Elemente öffnen eine tiefere **Ebene**.

| Taste | Was sie tut |
|---|---|
| Tab, Umschalt+Tab | Nächste oder vorherige Gruppe. Zuerst werden der Name der Gruppe und die Zahl ihrer Elemente gesagt. |
| Pfeil nach oben, Pfeil nach unten (oder Pfeil nach links, Pfeil nach rechts) | Vorheriges oder nächstes Element der Gruppe. |
| Pos1, Ende | Erstes oder letztes Element der Gruppe. |
| Eingabe | Aktiviert das Element. Auf einer Ebene (einer Art oder einem Raum der Objekte des Grundstücks) öffnet sie diese. |
| Leertaste | Aktiviert, außer in der Grundstücksansicht, wo die Leertaste die Taste „nächster Sim“ des Spiels ist. |
| Rücktaste | Eine Ebene zurück. |
| R | Sagt das Element noch einmal, mit seiner Gruppe. |
| Esc | Schließt das Interaktionsmenü; sonst das Esc des Spiels. |
| Alt+Buchstabe | Die nächste Gruppe, deren Name mit diesem Buchstaben beginnt. |
| Strg+F1 | Die Tasten des Bildschirms, auf dem Du bist. |
| Strg+Umschalt+F1 | Dieses Handbuch. |

Steuerelemente sagen nach ihrem Namen, was sie sind, wie bei einem Screenreader: „Speichern, Schalter“, „Soundoptionen, Registerkarte, ausgewählt“, „Automatisches Zentrieren, Kontrollkästchen, aktiviert“, „Effekte, Schieberegler, 8 von 10“, „Geländedetail, Auswahlfeld, Hoch“.

- **Schieberegler, Auswahlfelder und Charakterbalken**: Pfeil nach links und Pfeil nach rechts (oder Bild auf und Bild ab) ändern den Wert. Eingabe sagt ihn nur noch einmal.
- **Kontrollkästchen**: Eingabe aktiviert und deaktiviert sie.
- **Textfelder** (Familien- und Sim-Namen, Biografien): geh mit den Pfeiltasten auf das Feld und tippe. Jeder Buchstabe wird wiederholt, und die Rücktaste sagt, was sie löscht. Pfeil nach links, Pfeil nach rechts, Pos1, Ende und Entf bewegen sich im Feld. Solange ein Textfeld auf dem Bildschirm ist, tippen Buchstaben, Leertaste und R.
- **Deaktivierte Steuerelemente** sagen „nicht verfügbar“ und werden nicht aktiviert.

Die letzte Gruppe jedes Bildschirms ist **Mod-Hilfe**: ihre Schalter sagen die Tasten dieses Bildschirms (wie Strg+F1) und öffnen dieses Handbuch (wie Strg+Umschalt+F1). Beim Start des Spiels sagt der Mod beide Tasten nach „Barrierefreiheit für Die Sims geladen“.

## Mehr lesen: die Lesepuffer

Nicht alles passt in eine gesprochene Zeile. Der Mod hält zwei Puffer bereit, die Du Zeile für Zeile lesen kannst:

- **Details**: alles über das gerade Gesagte. Die Bewertungen, Räume und Beschreibung eines Produkts; die Aktionen, der Raum, die Entfernung und der Verkaufswert eines Objekts auf dem Grundstück; die Erklärung eines Bedürfnisses oder einer Fähigkeit; der Tooltip eines Steuerelements und sein Platz in seiner Gruppe („3 von 12, Steuerung“).
- **Ereignisse**: was passiert ist, das Neueste zuerst: Käufe, Verkäufe, Geschwindigkeitswechsel, Dialogtexte, eingereihte Aktionen, alles, was der Erzähler gesagt hat (die letzten 200).

| Taste | Was sie tut |
|---|---|
| Strg+Pfeil nach oben | Die nächste Zeile, tiefer im Puffer. |
| Strg+Pfeil nach unten | Zurück Richtung Anfang. |
| Strg+Pfeil nach links, Strg+Pfeil nach rechts | Wechselt zwischen den Puffern. |

## Die Nachbarschaft

Die Hauptgruppe des Nachbarschaftsbildschirms ist **Häuser**: jedes sagt seine Adresse und wer dort wohnt. Eingabe auf einem Haus aktiviert es wie ein Klick, und die Karte oder die Fragen des Spiels folgen. Auf einem Haus liest Strg+Pfeil nach oben seine Details: was die Hausübersicht des Baumodus sagen würde, nach dem letzten Speichern des Hauses (Räume, was fehlt, Räume ohne Zugang). Strg+Umschalt+L sagt alles auf einmal.

Die anderen Gruppen enthalten die Schalter des Spiels: eine Familie erstellen, den Familienvorrat, die Nummer der Nachbarschaft, die Optionen, Beenden.

Ein Haus ohne Familie öffnet sich nur im Baumodus, mit unbegrenztem Geld; der Mod sagt „keine Familie, unbegrenztes Geld“ statt eines Geldbetrags.

## Eine Familie und Sims erstellen

Alles auf diesen Bildschirmen ist eine Gruppe oder ein Feld:

- Tippe den Familiennamen und den Vornamen des Sims in ihre Textfelder.
- **Charakter**: Pfeil nach links und Pfeil nach rechts geben jedem Merkmal einen Punkt oder nehmen ihn weg („4 von 10, noch 6 Punkte“). Das Sternzeichen ergibt sich aus den Punkten.
- **Aussehen**: Alter, Geschlecht und Hautfarbe sowie die Pfeile für Kopf und Körper. Jeder Kopf und jeder Körper wird beschrieben („Kopf 3 von 137: blonde Haare, Bob“).
- Die Biografie ist ebenfalls ein Textfeld.
- Später auf dem Grundstück öffnen Strg+Umschalt und eine Zahl (1 bis 9) die Karte eines Sims, wo die Biografie bearbeitet werden kann.

## Das Grundstück: der Live-Modus

Auf einem Grundstück ist die Oberfläche ein Ring von Gruppen wie jeder Bildschirm. Tab geht in dieser Reihenfolge durch sie:

1. **Status**: Uhrzeit, Vermögen, Spielgeschwindigkeit, Freunde.
2. **Bedürfnisse** des ausgewählten Sims, jedes in Prozent; Eingabe sagt, was das Bedürfnis bedeutet.
3. **Familie**: die Sims der Familie; Eingabe wählt einen aus.
4. **Warteschlange**: was der ausgewählte Sim tun wird; Eingabe bricht eine Aktion ab.
5. **Objekte nach Art** und **Objekte nach Raum**: die Objekte des Grundstücks (siehe unten).
6. **Steuerung**, die **Sim-Leisten** (Stimmung, Beziehungen, Job und Fähigkeiten, Charakter, Interessen, Haus) und jedes Fenster, das Du öffnest.
7. **Mod-Hilfe**.

Auf einem Element einer Live-Leiste (einem Bedürfnis, einer Fähigkeit, einem Job, einer Hausbewertung) sagt Eingabe das Popup des Spiels dazu.

Der Beruf nennt seine Arbeitszeiten und wann die Fahrgemeinschaft kommt. Jede Fähigkeit nennt ihre Stufe und, wenn eine Beförderung mehr braucht, wie viel: „Kochen, 2 von 10, die nächste Beförderung braucht 4“. Die Menüs des Spiels haben keine Option, einen Job zu kündigen: Wer immer wieder fehlt, wird entlassen, und Zeitung und Computer bieten andere Jobs an.

### Sims auswählen und die Kamera

| Taste | Was sie tut |
|---|---|
| Leertaste | Das „nächster Sim“ des Spiels: das nächste Familienmitglied wird ausgewählt und angesagt („Isabel, ausgewählt, Essen“). |
| Strg+1 bis Strg+9 | Wählt den ersten bis neunten Sim der Familie aus. |
| Strg+Umschalt+1 bis Strg+Umschalt+9 | Öffnet die Karte dieses Sims (Biografie). |
| Strg+Umschalt+F | Die Kamera folgt dem ausgewählten Sim oder bleibt frei. |
| Strg+E | Der ausgewählte Sim in einer Zeile: Raum, was er tut, dringende Bedürfnisse, Stimmung. |
| Strg+Umschalt+E | Wie ein Sim aussieht: sein Kopf und sein Körper („Kopf: schwarze Haare, kurz; Körper: Anzug, schwarz“). Der Sim, der unter den Objekten des Grundstücks gewählt ist, sonst der ausgewählte. Die Details eines Sims (Strg+Pfeil nach oben auf ihm) sagen es auch. |
| Strg+T | Sagt jede neue Stunde der Uhr an („es ist 14:00“), oder hört damit auf. Nützlich bei der höchsten Geschwindigkeit. |
| P | Pause (die Taste des Spiels); der Mod sagt Uhrzeit und Vermögen. 1, 2, 3 stellen die Geschwindigkeit ein. |

Die Kamera folgt dem ausgewählten Sim, weil die Klänge des Spiels von dem kommen, was auf dem Bildschirm ist. Während Du die Objekte des Grundstücks durchgehst, fährt die Kamera zu jedem und kehrt danach zum Sim zurück.

Zoom und Blickrichtung der Kamera werden gesagt, wenn sie sich ändern (Plus und Minus zoomen, Komma und Punkt drehen die Ansicht), und stehen auch in der Statuszeile: "Kamera: mittlere Ansicht, Blick nach Nordwesten". Die Blickrichtung ist, was der obere Bildschirmrand zeigt. Der Norden des Mods dreht sich nicht mit der Kamera.

### Die Objekte des Grundstücks

Alles auf dem Grundstück steht in zwei Gruppen: **Objekte nach Art** (Sims, dann die Kategorien des Kaufmodus: Sitzgelegenheiten, Oberflächen, Küchen-Geräte..., dann Sonstige Objekte) und **Objekte nach Raum** (zuerst der Raum des ausgewählten Sims, dann die anderen, jeder mit Stockwerk und Größe). Eingabe auf einer Art oder einem Raum listet deren Objekte auf, die nächsten zuerst, als „Kühlschrank, Küche, 6 Felder Richtung Nordosten“ vom ausgewählten Sim aus. Die Rücktaste geht zurück zu den Arten oder Räumen. Die Raumnamen errät der Mod aus dem, was darin steht.

Du musst nicht mit Tab dorthin: die Bild-Tasten gehen von überall auf dem Grundstück durch die Objekte, im Live-, Kauf- und Baumodus.

| Taste | Was sie tut |
|---|---|
| Bild ab, Bild auf | Nächstes oder vorheriges Objekt der zuletzt gewählten Art oder des zuletzt gewählten Raums. |
| Umschalt+Bild ab, Umschalt+Bild auf | Nächste oder vorherige Art von Objekten. |
| Strg+Bild ab, Strg+Bild auf | Nächster oder vorheriger Raum (beim ersten Drücken: der Raum des ausgewählten Sims). |
| Eingabe | Die Interaktionen des Objekts (im Kaufmodus: es zum Verschieben aufheben). Auf einem Eintrag gleicher Dinge („Klappstuhl, 4 Stück“): listet sie einzeln auf, die nächsten zuerst; Rücktaste führt zurück. |
| Strg+Eingabe | Schickt den ausgewählten Sim zum Objekt. |
| Strg+F | Etwas suchen: Tippe einen Teil des Namens ("tel" findet das Telefon, Akzente sind egal). Pfeil nach oben und unten gehen die Treffer durch, Eingabe wirkt darauf, Strg+Eingabe schickt den Sim hin, Entf entfernt eine Tür oder ein Fenster, Esc führt zurück, wo du warst. |
| Entf | Auf einer Tür oder einem Fenster, im Kauf- oder Baumodus: entfernt es und erstattet den Wert. |

Ein Sim oder ein Objekt, über dem noch eine Sprechblase steht, sagt sie mit („Bella, Essen, denkt: Hunger“). Was kein Sim vom Eingang des Grundstücks aus erreichen kann, sagt „kein Sim kommt dorthin“.

Nach den Sims kommt **Probleme**: was Aufmerksamkeit braucht, etwa Kaputtes, was gereinigt werden muss, Pfützen, Müll, Asche, Feuer und alles mit Fliegen. Zuletzt kommt **Türen, Fenster und Treppen**.

### Interaktionsmenüs

Eingabe auf einem Objekt (oder einem Sim) öffnet das Interaktionsmenü des Spiels. Pfeil nach oben und Pfeil nach unten wählen, Eingabe führt aus, Esc geht eine Ebene zurück oder schließt. Einträge, die mit „...“ enden, öffnen weitere Auswahlen. Die gewählte Aktion wird als eingereiht angesagt („Reden, in der Warteschlange“). Wenn das Menü leer ist, sagt der Mod warum („keine Interaktionen. nur für Kinder: Spielen“).

### Räume und das Haus

| Taste | Was sie tut |
|---|---|
| Strg+L | Der Aufbau des Raums: Größe, Türen und Fenster nach Wand und wohin sie führen; was entlang jeder Wand steht, ist im Details-Puffer. |
| Strg+Umschalt+L | Die Hausübersicht: Stockwerke, Räume, Bewertungen, was fehlt (Toilette, Dusche, Betten, Kühlschrank, Herd, Telefon, Rauchmelder), Räume ohne Zugang, Objekte, die niemand erreichen kann, dunkle oder unbeliebte Räume. |

### Der Hausplan

**Strg+M** schaltet den Plan ein: einen Cursor, der Feld für Feld durch das Haus geht, beginnend beim ausgewählten Sim. Jeder Schritt spielt einen Klang für den Boden und für jede Wand, jeden Zaun, jede Tür und jedes Fenster ringsum.

| Taste | Was sie tut |
|---|---|
| Pfeiltasten | Ein Feld; mit Umschalt fünf. Sagt, was dort steht und wohin es schaut, den Raum und das Stockwerk, wenn sie wechseln, die Wände, Zäune, Geländer, Türen und Fenster auf jeder Seite. Wände, Zäune und Fenster halten den Cursor auf; Türen lassen ihn durch. |
| Alt+Pfeiltasten | Dasselbe, durch Wände hindurch. |
| Komma, Punkt | Was jeder Schritt sagt: alles, nur Objekte, nur Böden (mit Namen, „Heller Granitboden“), nur Wände, nur Räume. |
| Pos1 | Zurück zum ausgewählten Sim. |
| Strg+Umschalt+Bild auf, Bild ab | Das Stockwerk darüber oder darunter. |
| Eingabe | Das Interaktionsmenü dessen, was dort steht (was auf einem Tisch oder Schreibtisch steht, vor dem Tisch: der Computer, nicht sein Schreibtisch). |
| Strg+Eingabe | Schickt den ausgewählten Sim dorthin. |
| Bild ab, Bild auf (und Umschalt, Strg) | Die Objekte des Grundstücks wie immer; der Plan geht zu jedem. |
| R | Alles über das Feld. Strg+L den Aufbau des Raums. |
| Esc oder Strg+M | Verlässt den Plan. |

### Der Erzähler

Vieles zeigt das Spiel nur als Bild. Der Mod sagt es, wenn es passiert, und behält es im Ereignis-Puffer:

- **Sprechblasen** über Sims und Objekten: „Bella denkt: Hunger“, „Mortimer schreit: Harndrang“, „Bella sagt: Aliens“, ein Thema, das der andere Sim nicht mag („..., interessiert sich nicht dafür“), „Beziehung schlechter“ und „Abfalleimer: Fliegen“.
- **Blockierte Wege**: „Bella kommt nicht vorbei: Stuhl ist im Weg“.
- **Besucher**, die kommen und gehen, Geister, der Einbrecher, der Sensenmann, der Pizzabote, die Fahrgemeinschaft und der Schulbus (welches Auto hupt).
- **Mitgenommene Objekte** durch den Gerichtsvollzieher oder einen Einbrecher, mit ihrem Wert; **Rechnungen**, die ankommen.
- **Die Familienwache**: was der ausgewählte Sim zu tun beginnt, auf Deinen Befehl oder von sich aus („Cris, von sich aus: Sitzen“), wenn er fertig ist oder unterbrochen wird („Cris hat Sitzen für Panik unterbrochen; es wartet in der Warteschlange“), Raumwechsel, Bedürfnisse, die niedrig werden, die Stimmung, die rot wird, die Beziehungsänderungen des Tages („Marisa mit Cris: +8, jetzt 28“; das Beziehungsfeld des Spiels zeigt diese Zahlen auch) und Geld, das außerhalb des Kaufmodus bezahlt oder erhalten wird.
- **Ausflüge**: wo Du angekommen bist, und „wieder zu Hause“.

**Strg+N** ändert, wie viel gesagt wird: alles, nur Warnungen oder nichts. Nicht gesprochene Zeilen kommen trotzdem in den Ereignis-Puffer.

**Strg+Umschalt+N** wählt, wessen Gespräche gesagt werden (was Sims sagen, ihre sozialen Interaktionen, die Beziehungsänderungen, die sie bringen): jeder, der mit Deiner Familie redet (am Anfang), alle auf dem Grundstück, nur Deine Familie oder niemand.

Klänge, die Du hörst (das Telefon, die Türklingel, Alarme), werden nicht angesagt.

Aktionen mit einem anderen Sim nennen ihn, egal wer sie begonnen hat: "Homer: Begrüßen, mit Elvira".

### Hilfetipps und das Tutorial

Wenn das Hilfesystem des Spiels einen Tipp bereithält, sagt der Mod „Tipp des Hilfesystems verfügbar: Strg+H liest ihn vor“. **Strg+H** öffnet ihn: der Tipp wird vorgelesen, oder der Mod nennt, worauf der rote Pfeil des Spiels zeigt.

Haus 7 der ersten Nachbarschaft startet das Tutorial des Spiels. Jeder Schritt wird als Tastaturfassung des Spieltexts vorgelesen („Drücke den Schalter Heranzoomen: Tab bis Steuerung, Pfeiltasten bis Heranzoomen, Eingabe“), gefolgt davon, worauf der Pfeil zeigt. Jeder Schritt lässt sich mit der Tastatur erledigen. Das erste Esc während des Tutorials warnt Dich; ein zweites Esc innerhalb von vier Sekunden verlässt es.

### Pizza und andere Dienste

Benutze die Interaktionen des Telefons, um Dienste zu rufen. Der Pizzabote muss begrüßt werden: wenn er ankommt, geh zu Objekte nach Art, Sims, Pfeil nach unten bis zu ihm, Eingabe, Begrüßen. Begrüßt ihn niemand, geht er mit der Pizza wieder, und Du bekommst das Geld zurück.

## Der Kaufmodus

F2 öffnet den Kaufmodus. Die Kategorien des Katalogs sind Registerkarten und seine Produkte Schalter: jedes Produkt sagt Namen, Preis, wichtigste Bewertungen und Beschreibung, und der Details-Puffer enthält alle Bewertungen und die Räume, für die es gedacht ist. Wenn Du über das letzte Produkt einer Seite hinausgehst, wird umgeblättert.

| Taste | Was sie tut |
|---|---|
| Strg+R | Beschränkt den Katalog auf Produkte für eine Raumart (Küche, Bad, Schlafzimmer, Wohnzimmer, Esszimmer, Arbeitszimmer, draußen) oder auf alle. |
| Rücktaste | Das „Zurück“ des Katalogs. |
| Eingabe auf einem Produkt | Beginnt, es zu platzieren. |

### Ein Objekt platzieren

Während Du ein Objekt hältst, hat der Mod schon die Plätze gefunden, an die es passt **und an denen es benutzt werden kann** (die Platzierungsprüfung des Spiels, die Wände und Türen des Raums, der Platz, den Sims zum Benutzen brauchen). Der Platz sagt seinen Raum, was daneben steht und wohin das Objekt schaut.

| Taste | Was sie tut |
|---|---|
| Bild ab, Bild auf (oder Tab) | Die Plätze, die besten zuerst. |
| Strg+Bild ab, Strg+Bild auf | Die Räume, die Plätze haben. |
| Umschalt+Bild ab, Umschalt+Bild auf (oder Komma, Punkt) | Die anderen Richtungen auf demselben Platz. |
| Strg+Umschalt+Bild ab, Bild auf | Neben den Objekten des Grundstücks, die nächsten zuerst. |
| Pfeiltasten | Verschiebt es stattdessen um ein Feld (Umschalt: fünf) und sagt, ob es passt und woran es stößt. Wände, Zäune und Fenster halten es auf; Türen lassen es durch. |
| Alt+Pfeiltasten | Dasselbe, durch Wände hindurch. |
| Eingabe | Kauft es dort (oder stellt es ab, beim Verschieben). |
| Esc | Bricht ab (ein verschobenes Objekt kehrt zurück). |
| Entf | Verkauft das gehaltene Objekt; die Erstattung wird angesagt. |
| R | Sagt den Platz noch einmal. |

Der Details-Puffer enthält das Feld des Platzes und die Plätze, die das Spiel abgelehnt hat, mit dem Grund.

Die Plätze eines Sitzes beginnen dort, wo etwas im Sitzen benutzt wird und noch keinen Sitz hat: „nach Süden, um Computer im Sitzen zu benutzen“. Nachdem ein Objekt platziert ist, sagt der Mod, wie es benutzt wird: „um Computer im Sitzen zu benutzen“, „wird von Bürostuhl aus benutzt“, „Sims kommen dorthin“ oder „kein Sim kommt dorthin“.

### Verschieben und verkaufen

Geh im Kaufmodus mit Bild ab zu einem Objekt des Grundstücks und drücke Eingabe: es wird aufgehoben, und dieselbe Liste von Plätzen erscheint, beginnend mit dem Raum, aus dem es kommt. Der erste Pfeil setzt den Cursor dorthin, wo es stand. Gleiche Objekte werden einzeln aufgeführt, und der letzte Kauf sagt „gerade gekauft“.

## Der Baumodus

F3 öffnet den Baumodus. Seine Werkzeuge sind Registerkarten, ihre Stile Produkte. Die Hausübersicht wird gesprochen, wenn Du den Baumodus betrittst (Strg+Umschalt+L wiederholt sie).

Ein Werkzeug zu drücken führt zu seinen Modellen, in einer eigenen Gruppe mit dem Namen des Werkzeugs; Tab führt zurück zu den Werkzeugen. Du hast nichts in der Hand, bis du auf einem Modell Eingabe drückst.

### Der Bau-Cursor

Eingabe auf einem Wand- oder Zaunstil startet den Bau-Cursor beim ausgewählten Sim. Er bewegt sich auf den Ecken zwischen den Feldern.

| Taste | Was sie tut |
|---|---|
| Pfeiltasten | Eine Ecke; mit Umschalt fünf. Sagt, wo er ist, welche Wände dort zusammentreffen und was dort steht. |
| Leertaste | Markiert den Start. Danach sagt der Cursor die Entfernung vom Start und was Eingabe bauen würde und was es kostet. |
| Eingabe | Baut eine Wand vom Start zum Cursor. |
| Umschalt+Eingabe | Baut die vier Wände eines Raums zwischen Start und Cursor. |
| Strg+Eingabe oder Entf | Entfernt Wände. |
| Komma, Punkt | Wählt eine fertige Raumgröße. |
| Einfügen | Baut den fertigen Raum mit seiner Nordwestecke am Cursor. |
| Tab, Umschalt+Tab (oder Strg+Bild ab, Bild auf) | Springt zum nächsten oder vorherigen Raum. |
| Bild auf, Bild ab | Das Stockwerk darüber oder darunter. |
| Pos1 | Zurück zum ausgewählten Sim. |
| R | Alles: der freie Platz in jede Richtung, der Raum. |
| Esc | Verlässt den Cursor. |

Vor dem Bauen prüft der Mod die neuen Wände. Wenn sie einen Sim einsperren, ein Objekt abschneiden, eine Tür schließen oder einen Teil des Grundstücks abtrennen würden, sagt er es, und ein zweites Eingabe baut trotzdem.

### Böden, Tapeten, Pools und Gelände

Jedes dieser Werkzeuge hat denselben Cursor auf den Feldern (Pfeiltasten, Umschalt für fünf, Pos1, R, Esc; Strg+F1 für die Einzelheiten):

- **Böden**: Eingabe legt Boden auf das Feld oder auf das Rechteck ab einem mit der Leertaste markierten Start; Umschalt+Eingabe auf den ganzen Raum; Strg+Eingabe entfernt ihn.
- **Tapeten**: Eingabe tapeziert alle Wände des Raums unter dem Cursor; Strg+Eingabe entfernt die Tapete.
- **Pools**: die Leertaste markiert eine Ecke, Eingabe gräbt; der Mod warnt, bevor Wasser etwas abtrennt. Strg+Eingabe schüttet Felder wieder zu. Sims brauchen eine Poolleiter (in der Liste des Werkzeugs).
- **Gelände**: von einem markierten Feld bis zum Cursor einebnen, oder den Boden anheben und absenken.

**Gras** benutzt denselben Cursor: Eingabe malt das Feld darunter.

### Türen, Fenster und Treppen

Türen und Fenster werden wie Möbel platziert, aus einer Liste von Plätzen: „in der Wand im Westen von Küche, führt zu draußen, die erste Tür dieses Raums“.

Mit den Pfeilen sagt eine Tür oder ein Fenster, in welche Wand es käme; Drehen wählt die Wand. Nach dem Platzieren einer Tür sagt der Mod, ob Sims hindurchgehen können oder was im Weg steht. Um sie später zu ändern, finde sie unter **Türen, Fenster und Treppen** oder mit Strg+F: Eingabe nimmt sie zum Verschieben auf, Entf entfernt sie (das Strg+Klick des Spiels).

### Das Dach

Das Spiel baut das Dach von selbst über deinen Wänden: Es gibt keinen Cursor und nichts zu platzieren. Das Dach-Tool wählt nur Neigung und Modell, für das ganze Haus. Die Modelle haben im Spiel keine Namen: Der Mod nennt ihre Farbe.

## Klanghinweise

Der Bau-Cursor, das Platzieren und der Hausplan spielen bei jedem Schritt einen kurzen Klang: den Boden, die Wand, den Zaun, die Tür oder das Fenster jeder Seite (jede Seite klingt anders), Objekte, einen Raumwechsel, den Rand des Grundstücks, Hänge und ob ein Objekt passt. **Strg+Umschalt+S** schaltet sie aus und an.

## Weitere Tasten

| Taste | Was sie tut |
|---|---|
| Strg+Umschalt+C | Die Cheat-Konsole des Spiels. Pfeil nach oben und Pfeil nach unten wählen einen Cheat, Tippen ergänzt ihn, Eingabe führt ihn aus (noch einmal: wiederholt ihn), Esc schließt. |
| Strg+Umschalt+V | Hebt das gewählte Steuerelement auf dem Bildschirm hervor, für eine sehende Hilfe. Auf dem Grundstück geht auch der Mauszeiger zum gewählten Ding. |

## Die Tasten des Spiels

Diese gehören zum Spiel und funktionieren weiterhin:

| Taste | Was sie tut |
|---|---|
| F1, F2, F3, F4, F5 | Live-, Kauf-, Bau-, Kamera- und Optionenmodus. |
| P | Pause. |
| 1, 2, 3 | Normale, hohe, höchste Geschwindigkeit. |
| Komma, Punkt | Dreht die Ansicht. |
| Plus, Minus | Heranzoomen und Wegzoomen. |
| J, N | Ja und Nein in Dialogen. |
| Strg+S | Speichern. |

## Einstellungen

Die Einstellungen des Mods stehen in `SimsAccess\SimsAccess.ini` im Ordner des Spiels, Abschnitt `[mod]`. Die Tasten, die sie ändern, speichern sie sofort.

| Einstellung | Werte |
|---|---|
| `narrator` | 2 alles, 1 nur Warnungen, 0 aus (Strg+N). |
| `cues` | 1 Klanghinweise an, 0 aus (Strg+Umschalt+S). |
| `cue_volume` | Lautstärke der Klanghinweise, 0 bis 100 (Standard 80). |
| `show_focus` | 1 hebt das gewählte Steuerelement auf dem Bildschirm hervor (Strg+Umschalt+V). |
| `social` | Wessen Gespräche gesagt werden: 3 alle, 2 jeder, der mit Deiner Familie redet, 1 Deine Familie, 0 niemand (Strg+Umschalt+N). |
| `hours` | 1 sagt jede neue Stunde an (Strg+T). |

## Wenn etwas nicht klappt

- **Gar keine Sprache**: starte Deinen Screenreader vor dem Spiel. Das Protokoll des Mods ist `SimsAccess\SimsAccess.log`; schick es mit Deiner Meldung.
- **„Nicht unterstützte Spielversion“**: das Spiel wurde aktualisiert; installiere die nächste Version des Mods.
- **Eine Taste tut nichts**: Strg+F1 sagt, welche Tasten dort funktionieren, wo Du bist.

## Noch nicht fertig

Die eigenen Ausflüge und Bildschirme von Urlaub total, Megastar und Hokus Pokus sind noch nicht vollständig abgedeckt. Foto- und Kameramodus und das Familienalbum sind rein visuell und bleiben außen vor.
