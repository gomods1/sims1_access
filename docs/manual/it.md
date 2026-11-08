---
title: Manuale di Accessibilità per The Sims
lang: it
toc-title: Indice
---

Accessibilità per The Sims rende The Sims Legacy Collection giocabile con uno screen reader e la sola tastiera. Legge ogni schermata, finestra di dialogo e pannello con il tuo screen reader (NVDA, JAWS, Assistente vocale e altri), ti fa muovere ovunque con gli stessi tasti e aggiunge gli strumenti di cui ha bisogno un giocatore cieco: un elenco degli oggetti del lotto, una mappa della casa, un narratore per ciò che il gioco si limita a mostrare, le posizioni in cui un mobile ci sta e un cursore per costruire muri e stanze.

Nel gioco, **Control+Maiusc+F1** apre questo manuale, nella lingua del gioco. **Control+F1** dice i tasti della schermata in cui ti trovi.

## Per cominciare

1. Avvia il programma di installazione, oppure copia `version.dll` e la cartella `SimsAccess` nella cartella del gioco (quella con `Sims.exe`).
2. Avvia lo screen reader, poi avvia The Sims Legacy Collection da Steam.
3. Quando si apre la finestra del gioco senti "Accessibilità per The Sims caricata". Il gioco parte dalla schermata del Vicinato.

La mod parla nella lingua del gioco. Per cambiarla, apri Steam, poi le Proprietà del gioco, Generali, Lingua. La mod parla inglese, francese, tedesco, italiano, spagnolo, olandese, danese, svedese, norvegese, finlandese, portoghese brasiliano, giapponese, polacco, cinese semplificato e tradizionale, thailandese e coreano.

La mod funziona con la versione Steam attuale del gioco. Se il gioco si aggiorna e la mod dice "versione del gioco non supportata", installa la versione successiva della mod.

## Muoversi: gli stessi tasti ovunque

Ogni schermata, finestra di dialogo, pannello e modalità funziona allo stesso modo. Ti muovi in un anello di **gruppi** (i pulsanti di una finestra di dialogo, i bisogni, la famiglia, il catalogo...). Un gruppo contiene degli **elementi**, e alcuni elementi aprono un **livello** più profondo.

| Tasto | Cosa fa |
|---|---|
| Tab, Maiusc+Tab | Gruppo successivo o precedente. Prima vengono detti il nome del gruppo e il numero dei suoi elementi. |
| Freccia su, freccia giù (o freccia sinistra, freccia destra) | Elemento precedente o successivo del gruppo. |
| Inizio, Fine | Primo o ultimo elemento del gruppo. |
| Invio | Attiva l'elemento. Su un livello (un tipo o una stanza degli oggetti del lotto) lo apre. |
| Spazio | Attiva, tranne nella vista del lotto, dove Spazio è il tasto "Sim successivo" del gioco. |
| Backspace | Torna indietro di un livello. |
| R | Ripete l'elemento, con il suo gruppo. |
| Esc | Chiude il menu delle interazioni; altrimenti l'Esc del gioco. |
| Alt+lettera | Il gruppo successivo il cui nome inizia con quella lettera. |
| Control+F1 | I tasti della schermata in cui ti trovi. |
| Control+Maiusc+F1 | Questo manuale. |

I controlli dicono cosa sono dopo il loro nome, come fa uno screen reader: "Salva, pulsante", "Opzioni Sonoro, scheda, selezionata", "Auto-Centering, casella di controllo, selezionata", "Effetti, cursore, 8 su 10", "Terreno, selettore, Alta".

- **Cursori, selettori e barre della personalità**: freccia sinistra e freccia destra (o Pagina su e Pagina giù) cambiano il valore. Invio lo ripete soltanto.
- **Caselle di controllo**: Invio le seleziona e le deseleziona.
- **Campi di testo** (nomi di famiglie e di Sim, biografie): vai sul campo con le frecce e scrivi. Ogni lettera viene ripetuta, e Backspace dice cosa cancella. Freccia sinistra, freccia destra, Inizio, Fine e Canc si muovono nel campo. Finché un campo di testo è sullo schermo, le lettere, Spazio e R scrivono.
- **I controlli disattivati** dicono "non disponibile" e non vengono attivati.

L'ultimo gruppo di ogni schermata è **aiuto della mod**: i suoi pulsanti dicono i tasti di quella schermata (come Control+F1) e aprono questo manuale (come Control+Maiusc+F1). All'avvio del gioco, la mod dice entrambi i tasti dopo "Accessibilità per The Sims caricata".

## Leggere di più: i buffer di lettura

Non tutto sta in una frase. La mod tiene due buffer che puoi leggere riga per riga:

- **dettagli**: tutto su ciò che è appena stato detto. Le valutazioni, le stanze e la descrizione di un prodotto; le azioni, la stanza, la distanza e il valore di vendita di un oggetto del lotto; la spiegazione di un bisogno o di un'abilità; il suggerimento di un controllo e il suo posto nel gruppo ("3 di 12, controlli").
- **eventi**: cosa è successo, dal più recente: acquisti, vendite, cambi di velocità, testi delle finestre di dialogo, azioni messe in coda, tutto ciò che ha detto il narratore (gli ultimi 200).

| Tasto | Cosa fa |
|---|---|
| Control+freccia su | La riga successiva, più avanti nel buffer. |
| Control+freccia giù | Torna verso l'inizio. |
| Control+freccia sinistra, Control+freccia destra | Passa da un buffer all'altro. |

## Il Vicinato

Il gruppo principale della schermata del Vicinato è **case**: ognuna dice il suo indirizzo e chi ci abita. Invio su una casa la attiva, come un clic, e seguono la scheda o le domande del gioco. Su una casa, Control+freccia su legge i suoi dettagli: quello che direbbe la panoramica della casa in Costruisci, in base all'ultimo salvataggio della casa (stanze, cosa manca, stanze senza accesso). Control+Maiusc+L dice tutto in una volta.

Gli altri gruppi contengono i pulsanti del gioco: crea una famiglia, il deposito delle famiglie, il numero del Vicinato, le opzioni, esci.

Una casa senza famiglia si apre solo in Costruisci, con fondi illimitati; la mod dice "nessuna famiglia, fondi illimitati" invece di una somma di denaro.

## Creare una famiglia e i Sim

Tutto in queste schermate è un gruppo o un campo:

- Scrivi il cognome della famiglia e il nome del Sim nei loro campi di testo.
- **personalità**: freccia sinistra e freccia destra aggiungono o tolgono un punto a ogni tratto ("4 su 10, restano 6 punti"). Il segno zodiacale deriva dai punti.
- **aspetto**: età, sesso e carnagione, e le frecce di viso e corpo. Ogni viso e ogni corpo viene descritto ("viso 3 di 137: capelli biondi, caschetto").
- Anche la biografia è un campo di testo.
- Più tardi, nel lotto, Control+Maiusc e un numero (da 1 a 9) aprono la scheda di un Sim, dove si può modificare la biografia.

## Il lotto: la modalità Vivi

In un lotto l'interfaccia è un anello di gruppi come qualsiasi schermata. Tab li scorre in quest'ordine:

1. **stato**: ora, denaro, velocità del gioco, amici.
2. **bisogni** del Sim selezionato, ognuno in percentuale; Invio dice cosa significa il bisogno.
3. **famiglia**: i Sim della famiglia; Invio ne seleziona uno.
4. **coda delle azioni**: cosa farà il Sim selezionato; Invio annulla un'azione.
5. **oggetti per tipo** e **oggetti per stanza**: gli oggetti del lotto (vedi sotto).
6. **controlli**, i **pannelli del Sim** (Caratteristiche, cioè l'umore, Relazioni, Lavoro e Abilità, Personalità, Interesse, Casa) e qualsiasi pannello tu apra.
7. **aiuto della mod**.

Su un elemento di un pannello di Vivi (un bisogno, un'abilità, un lavoro, una valutazione della casa), Invio legge il riquadro informativo del gioco.

Il lavoro dice gli orari e quando passa l'auto. Ogni abilità dice il suo livello e, quando una promozione ne richiede di più, quanto: "Cucina, 2 su 10, la prossima promozione richiede 4". I menu del gioco non hanno un'opzione per lasciare il lavoro: un Sim che salta il lavoro di continuo viene licenziato, e il giornale e il computer offrono altri lavori.

### Selezionare i Sim e la telecamera

| Tasto | Cosa fa |
|---|---|
| Spazio | Il "Sim successivo" del gioco: viene selezionato e annunciato il membro successivo della famiglia ("Isabel, Sim selezionato, Mangia"). |
| Da Control+1 a Control+9 | Seleziona dal primo al nono Sim della famiglia. |
| Da Control+Maiusc+1 a Control+Maiusc+9 | Apre la scheda di quel Sim (biografia). |
| Control+Maiusc+F | La telecamera segue il Sim selezionato, o resta libera. |
| Control+E | Il Sim selezionato in una frase: stanza, cosa sta facendo, bisogni urgenti, umore. |
| Control+Maiusc+E | Che aspetto ha un Sim: la testa e il corpo ("testa: capelli neri, corti; corpo: completo, nero"). Il Sim su cui si trova il focus tra gli oggetti del lotto, altrimenti quello selezionato. Anche i dettagli di un Sim (Control+Freccia su su di lui) lo dicono. |
| Control+T | Dice ogni nuova ora dell'orologio ("sono le 14:00"), o smette di dirla. Utile alla velocità massima. |
| P | Pausa (il tasto del gioco); la mod dice l'ora e il denaro. 1, 2, 3 impostano la velocità. |

La telecamera segue il Sim selezionato, perché i suoni del gioco provengono da ciò che è sullo schermo. Mentre scorri gli oggetti del lotto, la telecamera va su ognuno e poi torna al Sim. Ci scivola lentamente, così la vista non corre (l'impostazione `camera_glide`).

Lo zoom e la direzione della telecamera vengono detti quando cambiano (più e meno per lo zoom, virgola e punto ruotano la vista), e sono anche nella riga di stato: "telecamera: vista media, rivolta a nord-ovest". La direzione è ciò che mostra la parte alta dello schermo. Il nord del mod non ruota con la telecamera.

### Gli oggetti del lotto

Tutto ciò che c'è nel lotto si trova in due gruppi: **oggetti per tipo** (i Sim, poi le categorie di Compra: sedie, divani e letti, tavoli, elettrodomestici..., poi altri oggetti) e **oggetti per stanza** (prima la stanza del Sim selezionato, poi le altre, ognuna con il suo piano e la sua misura). Invio su un tipo o una stanza elenca i suoi oggetti, dal più vicino, come "Frigorifero, cucina, a 6 caselle a nord-est" dal Sim selezionato. Backspace torna ai tipi o alle stanze. I nomi delle stanze sono un'ipotesi della mod in base a ciò che contengono.

Non serve andarci con Tab: i tasti Pagina scorrono gli oggetti da qualsiasi punto del lotto, in Vivi, Compra e Costruisci.

| Tasto | Cosa fa |
|---|---|
| Pagina giù, Pagina su | Oggetto successivo o precedente del tipo o della stanza scelti per ultimi. |
| Maiusc+Pagina giù, Maiusc+Pagina su | Tipo di oggetti successivo o precedente. |
| Control+Pagina giù, Control+Pagina su | Stanza successiva o precedente (alla prima pressione: la stanza del Sim selezionato). |
| Invio | Le interazioni dell'oggetto (in Compra: lo prende per spostarlo). Su una voce di cose uguali ("Sedia pieghevole, 4 uguali"): le elenca una per una, prima la più vicina; Backspace torna indietro. |
| Control+Invio | Manda il Sim selezionato fino all'oggetto. |
| Control+F | Cerca qualcosa: scrivi parte del nome ("tel" trova il telefono, gli accenti non contano). Freccia su e giù scorrono i risultati, Invio agisce, Control+Invio manda lì il Sim, Canc rimuove una porta o finestra, Esc torna dov'eri. |
| Canc | Su una porta o finestra, in modalità acquisto o costruzione: la rimuove e la rimborsa. |

Un Sim o un oggetto che mostra ancora un fumetto lo dice anche ("Bella, Mangia, pensa: mangiare"). Ciò che nessun Sim può raggiungere dall'ingresso del lotto dice "nessun Sim può raggiungerlo".

Dopo i Sim viene **problemi**: ciò che richiede attenzione, come oggetti rotti, cose da pulire, pozzanghere, rifiuti, cenere, fuoco e tutto ciò che ha mosche. Anche una ciotola per animali vuota è qui; in ogni elenco una ciotola dice se contiene cibo. Per ultimo viene **porte, finestre e scale**.

### I menu delle interazioni

Invio su un oggetto (o su un Sim) apre il menu delle interazioni del gioco. Freccia su e freccia giù scelgono, Invio esegue, Esc torna indietro di un livello o chiude. Le voci che finiscono con "..." aprono altre scelte. L'azione scelta viene annunciata in coda ("Parla, in coda"). Quando il menu è vuoto la mod dice perché ("nessuna interazione. solo per bambini: Gioca").

### Le stanze e la casa

| Tasto | Cosa fa |
|---|---|
| Control+L | La disposizione della stanza: misura, porte e finestre per muro e dove portano; cosa c'è lungo ogni muro è nel buffer dei dettagli. |
| Control+Maiusc+L | La panoramica della casa: piani, stanze, valutazioni, cosa manca (water, doccia, letti, frigorifero, fornello, telefono, rilevatore di fumo), stanze senza accesso, oggetti che nessuno può raggiungere, stanze buie o sgradite. |

### La mappa della casa

**Control+M** attiva la mappa: un cursore che percorre la casa casella per casella, partendo dal Sim selezionato. Ogni passo riproduce un suono per il pavimento e per ogni muro, recinto, porta o finestra intorno.

| Tasto | Cosa fa |
|---|---|
| Frecce | Una casella; con Maiusc, cinque. Dice cosa c'è e verso dove è rivolto, la stanza e il piano quando cambiano, i muri, recinti, ringhiere, porte e finestre su ogni lato. Muri, recinti e finestre fermano il cursore; le porte lo lasciano passare. |
| Alt+Frecce | Lo stesso, attraverso i muri. |
| Virgola, punto | Cosa dice ogni passo: tutto, solo oggetti, solo pavimenti (per nome, "Piastrella di granito chiaro"), solo muri, solo stanze. |
| Inizio | Torna al Sim selezionato. |
| Control+Maiusc+Pagina su, Pagina giù | Il piano di sopra o di sotto. |
| Invio | Il menu delle interazioni di ciò che c'è (ciò che sta su un tavolo o una scrivania prima del tavolo: il computer, non la sua scrivania). |
| Control+Invio | Manda lì il Sim selezionato. |
| Pagina giù, Pagina su (e Maiusc, Control) | Gli oggetti del lotto come sempre; la mappa va su ognuno. |
| R | Tutto sulla casella. Control+L la disposizione della stanza. |
| Esc o Control+M | Chiude la mappa. |

### Il narratore

Il gioco mostra molte cose solo come immagini. La mod le dice quando succedono e le tiene nel buffer degli eventi:

- **I fumetti** sopra Sim e oggetti: "Bella pensa: mangiare", "Mortimer grida: WC", "Bella dice: Alieni", un argomento che all'altro Sim non piace ("..., nessun interesse"), "il rapporto peggiora" e "Cestino per i Rifiuti: mosche".
- **I percorsi bloccati**: "Bella non riesce a passare: Sedia blocca la strada".
- **I visitatori** che arrivano e se ne vanno, i fantasmi, il ladro, la Morte, il fattorino della pizza, il car pooling e lo scuolabus (quale auto suona il clacson).
- **Gli oggetti portati via** dall'ufficiale giudiziario o da un ladro, con il loro valore; **le bollette** che arrivano.
- **La sorveglianza della famiglia**: cosa comincia a fare il Sim selezionato, per tuo ordine o di sua iniziativa ("Cris, di sua iniziativa: Siediti"), quando finisce o viene interrotto ("Cris ha lasciato Siediti per Panico; resta in coda"), i cambi di stanza, i bisogni che scendono, l'umore che diventa rosso, i cambi di relazione della giornata ("Marisa con Cris: +8, ora 28"; anche il pannello delle relazioni del gioco mostra questi numeri) e il denaro pagato o ricevuto fuori da Compra.
- **I viaggi**: dove sei arrivato, e "di nuovo a casa".

**Control+N** cambia quanto viene detto: tutto, solo avvisi, o niente. Le righe non dette finiscono comunque nel buffer degli eventi.

**Control+Maiusc+N** sceglie di chi si dicono le conversazioni (cosa dicono i Sim, le loro interazioni sociali, i cambi di relazione che portano): chiunque parli con la tua famiglia (all'inizio), tutti quelli del lotto, solo la tua famiglia o nessuno.

I suoni che senti (il telefono, il campanello, gli allarmi) non vengono annunciati.

Le azioni con un altro Sim lo nominano, chiunque le abbia iniziate: "Homer: Saluta, con Elvira".

### Consigli e tutorial

Quando il sistema di aiuto del gioco ha un consiglio in attesa, la mod dice "c'è un consiglio del sistema di aiuto: Control+H lo legge". **Control+H** lo apre: il consiglio viene letto, oppure la mod nomina ciò che indica la freccia rossa del gioco.

La casa 7 del primo Vicinato avvia il tutorial del gioco. Ogni passo viene letto in una versione da tastiera del testo del gioco ("Premi il pulsante Zoom +: Tab fino ai controlli, frecce fino a Zoom +, Invio"), seguita da ciò che indica la freccia. Ogni passo si può completare da tastiera. Il primo Esc durante il tutorial ti avvisa; un secondo Esc entro quattro secondi lo chiude.

### La pizza e gli altri servizi

Usa le interazioni del telefono per chiamare i servizi. Il fattorino della pizza va salutato: quando arriva, vai in oggetti per tipo, Sims, freccia giù fino a lui, Invio, Incontra. Se nessuno lo saluta, se ne va con la pizza e il denaro ti viene restituito.

## La modalità Compra

F2 apre Compra. Le categorie del catalogo sono schede e i suoi prodotti pulsanti: ogni prodotto dice nome, prezzo, valutazioni principali e descrizione, e il buffer dei dettagli contiene tutte le sue valutazioni e le stanze a cui è destinato. Andando oltre l'ultimo prodotto di una pagina si gira pagina.

| Tasto | Cosa fa |
|---|---|
| Control+R | Limita il catalogo ai prodotti per un tipo di stanza (cucina, bagno, camera da letto, soggiorno, sala da pranzo, studio, esterno) o a tutti. |
| Backspace | L'"Indietro" del catalogo. |
| Invio su un prodotto | Inizia a posizionarlo. |

### Posizionare un oggetto

Mentre tieni un oggetto, la mod ha già trovato le posizioni in cui ci sta **e si può usare** (il controllo di posizionamento del gioco, i muri e le porte della stanza, lo spazio che serve ai Sim per usarlo). La posizione dice la sua stanza, cosa c'è accanto e verso dove è rivolto l'oggetto.

| Tasto | Cosa fa |
|---|---|
| Pagina giù, Pagina su (o Tab) | Le posizioni, prima le migliori. |
| Control+Pagina giù, Control+Pagina su | Le stanze che hanno posizioni. |
| Maiusc+Pagina giù, Maiusc+Pagina su (o virgola, punto) | Gli altri orientamenti nella stessa posizione. |
| Control+Maiusc+Pagina giù, Pagina su | Accanto agli oggetti del lotto, prima i più vicini. |
| Frecce | Lo sposta invece di una casella (Maiusc: cinque), dicendo se ci sta e contro cosa urta. Muri, recinti e finestre lo fermano; le porte lo lasciano passare. |
| Alt+Frecce | Lo stesso, attraverso i muri. |
| Invio | Lo compra lì (o lo posa, quando lo sposti). |
| Esc | Annulla (un oggetto spostato torna al suo posto). |
| Canc | Vende l'oggetto che tieni; viene detto il rimborso. |
| R | Ripete la posizione. |

Il buffer dei dettagli contiene la casella della posizione e le posizioni rifiutate dal gioco, con il motivo.

Le posizioni di una seduta cominciano da dove qualcosa si usa da seduti e non ha ancora una seduta: "rivolto a sud, per usare Computer da seduti". Dopo aver posato un oggetto la mod dice come verrà usato: "per usare Computer da seduti", "si usa da Sedia da ufficio", "i Sim possono raggiungerlo" o "nessun Sim può raggiungerlo".

### Spostare e vendere

In Compra, vai con Pagina giù su un oggetto del lotto e premi Invio: viene preso e compare lo stesso elenco di posizioni, a partire dalla stanza da cui viene. La prima freccia mette il cursore dove si trovava. Gli oggetti identici sono elencati uno per uno, e l'ultimo acquistato dice "ultimo acquisto".

## La modalità Costruisci

F3 apre Costruisci. I suoi strumenti sono schede, i loro stili prodotti. La panoramica della casa viene letta quando entri in Costruisci (Control+Maiusc+L la ripete).

Premere uno strumento porta ai suoi modelli, in un gruppo a parte con il nome dello strumento; Tab torna agli strumenti. Non hai nulla in mano finché non premi Invio su un modello.

### Il cursore di costruzione

Invio su uno stile di muro o di recinto avvia il cursore di costruzione sul Sim selezionato. Si muove sugli angoli tra le caselle.

| Tasto | Cosa fa |
|---|---|
| Frecce | Un angolo; con Maiusc, cinque. Dice dove si trova, i muri che vi si incontrano e cosa c'è. |
| Spazio | Segna l'inizio. Il cursore dice poi la distanza dall'inizio e cosa costruirebbe Invio e quanto costa. |
| Invio | Costruisce un muro dall'inizio al cursore. |
| Maiusc+Invio | Costruisce i quattro muri di una stanza tra l'inizio e il cursore. |
| Control+Invio o Canc | Toglie muri. |
| virgola, punto | Sceglie una misura di stanza già pronta. |
| Ins | Costruisce la stanza già pronta con l'angolo nord-ovest sul cursore. |
| Tab, Maiusc+Tab (o Control+Pagina giù, Pagina su) | Salta alla stanza successiva o precedente. |
| Pagina su, Pagina giù | Il piano di sopra o di sotto. |
| Inizio | Torna al Sim selezionato. |
| R | Tutto: lo spazio libero in ogni direzione, la stanza. |
| Esc | Esce dal cursore. |

Prima di costruire, la mod controlla i nuovi muri. Se chiudessero dentro un Sim, impedissero di raggiungere un oggetto, chiudessero una porta o isolassero parte del lotto, lo dice, e un secondo Invio costruisce comunque.

### Pavimenti, carta da parati, piscine e terreno

Ognuno di questi strumenti ha lo stesso cursore sulle caselle (frecce, Maiusc per cinque, Inizio, R, Esc; Control+F1 per i dettagli):

- **Pavimenti**: Invio mette il pavimento sulla casella, o sul rettangolo da un inizio segnato con Spazio; Maiusc+Invio su tutta la stanza; Control+Invio lo toglie.
- **Carta da parati**: Invio tappezza tutti i muri della stanza sotto il cursore; Control+Invio la toglie.
- **Piscine**: Spazio segna un angolo, Invio scava; la mod avvisa prima che l'acqua isoli qualcosa. Control+Invio riempie di nuovo le caselle. I Sim hanno bisogno di una scaletta per piscina (nell'elenco dello strumento).
- **Terreno**: livella da una casella segnata fino al cursore, oppure alza e abbassa il terreno.

**Erba** usa lo stesso cursore: Invio dipinge la casella sotto.

### Porte, finestre e scale

Porte e finestre si posizionano come i mobili, da un elenco di posizioni: "nel muro ovest di cucina, dà su esterno, la prima porta di quella stanza".

Con le frecce, una porta o finestra dice in quale muro andrebbe; ruotarla sceglie il muro. Dopo aver collocato una porta il mod dice se i Sim possono passarci, o cosa la blocca. Per cambiarla in seguito, trovala in **porte, finestre e scale** o con Control+F: Invio la prende per spostarla, Canc la rimuove (il Control+clic del gioco).

### Il tetto

Il gioco costruisce il tetto da solo sopra i muri: non c'è cursore né nulla da collocare. Lo strumento Tetto sceglie solo pendenza e modello, per tutta la casa. I modelli non hanno nome nel gioco: il mod dice il colore.

## Segnali sonori

Il cursore di costruzione, il posizionamento e la mappa della casa riproducono un breve suono a ogni passo: il pavimento, il muro, il recinto, la porta o la finestra di ogni lato (ogni lato suona diverso), gli oggetti, un cambio di stanza, il bordo del lotto, i pendii e se un oggetto ci sta. **Control+Maiusc+S** li disattiva e li riattiva.

## Altri tasti

| Tasto | Cosa fa |
|---|---|
| Control+Maiusc+C | La console dei trucchi del gioco. Freccia su e freccia giù scelgono un trucco, ciò che scrivi si aggiunge, Invio lo esegue (di nuovo: lo ripete), Esc chiude. |
| Control+Maiusc+V | Evidenzia sullo schermo il controllo selezionato, per chi ti aiuta e vede. Sul lotto, anche il puntatore del mouse va sulla cosa selezionata. |

## I tasti del gioco

Sono del gioco e funzionano sempre:

| Tasto | Cosa fa |
|---|---|
| F1, F2, F3, F4, F5 | Vivi, Compra, Costruisci, Macchina Fotografica e Opzioni. |
| P | Pausa. |
| 1, 2, 3 | Velocità normale, alta, massima. |
| virgola, punto | Ruota la visuale. |
| più, meno | Zoom avanti e indietro. |
| S, N | Sì e no nelle finestre di dialogo. |
| Control+S | Salva. |

## Impostazioni

Le impostazioni della mod sono in `SimsAccess\SimsAccess.ini` nella cartella del gioco, sezione `[mod]`. I tasti che le cambiano le salvano subito.

| Impostazione | Valori |
|---|---|
| `narrator` | 2 tutto, 1 solo avvisi, 0 niente (Control+N). |
| `cues` | 1 segnali sonori attivi, 0 disattivati (Control+Maiusc+S). |
| `cue_volume` | Volume dei segnali sonori, da 0 a 100 (predefinito 80). |
| `show_focus` | 1 evidenzia sullo schermo il controllo selezionato (Control+Maiusc+V). |
| `social` | Di chi si dicono le conversazioni: 3 tutti, 2 chiunque parli con la tua famiglia, 1 la tua famiglia, 0 nessuno (Control+Maiusc+N). |
| `hours` | 1 dice ogni nuova ora (Control+T). |
| `camera_glide` | Quanto impiega la telecamera a scivolare sull'oggetto che scorri, in millisecondi (predefinito 800; 0 i rapidi 250 del gioco). |

## Quando qualcosa non va

- **Nessuna voce**: avvia lo screen reader prima del gioco. Il registro della mod è `SimsAccess\SimsAccess.log`; invialo con la tua segnalazione.
- **"Versione del gioco non supportata"**: il gioco si è aggiornato; installa la versione successiva della mod.
- **Un tasto non fa niente**: Control+F1 dice quali tasti funzionano dove ti trovi.

## Non ancora fatto

I viaggi e le schermate propri di The Sims Vacanze, The Sims Superstar e The Sims Magie non sono ancora del tutto coperti. La modalità foto e Macchina Fotografica e l'album di famiglia sono solo visivi e sono esclusi.
