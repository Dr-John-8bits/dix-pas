# DIX PAS - Document technique

Note documentaire :

- ce document decrit la mise en oeuvre technique recommandee
- en cas de conflit produit ou scope, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte

## 1. Objet du document

Ce document decrit une base technique coherente pour lancer le developpement du projet **DIX PAS** en materiel open source.

Il ne remplace pas les schemas electroniques ni le PCB final, mais il fixe :

- les choix techniques deja assumes
- l'architecture hardware recommandee
- l'architecture firmware cible
- les contraintes d'integration entre materiel et logiciel

---

## 2. Choix figes a ce stade

Les choix suivants sont consideres comme valides pour la V1 :

- format musical principal : **2 pistes de 10 pas**
- mode alternatif : **Chain 20**
- connectique live : **MIDI DIN 5 broches uniquement**
- interfaces MIDI requises : **1 MIDI IN + 1 MIDI OUT**
- sorties de controle requises : **Gate Out A + Gate Out B**
- ratchet V1 : **x1, x2, x3**
- controle prioritaire : **Probabilite > On/Off > Ratchet**
- affichage : **feedback contextuel immediat a l'ecran**

Choix exclus de la V1 :

- USB MIDI en usage normal
- TRS MIDI
- MIDI THRU obligatoire
- batterie integree

---

## 3. Plateforme controleur recommandee

### 3.1 Recommandation principale

La plateforme recommande pour lancer le projet est :

- **Arduino Nano Every**

### 3.2 Pourquoi ce choix

Cette carte est la plus coherente avec le projet a ce stade pour les raisons suivantes :

- logique **5 V**, ce qui simplifie a la fois le MIDI DIN et les sorties Gate
- **1 UART** materiel pour le MIDI DIN
- **USB separe** du microcontroleur principal sur la carte, utile pour le flash et le debug sans polluer l'interface MIDI live
- **48 KB Flash** et **6 KB SRAM**, plus confortables qu'un Arduino Micro pour un firmware avec ecran, menu, presets et scan d'interface
- format compact et officiel Arduino
- bon point d'entree pour un projet open source
- les fonctions **Euclidean** et **generatives** envisagees restent compatibles avec cette plateforme

### 3.3 Alternatives possibles

Alternatives raisonnables si besoin plus tard :

- **Arduino Nano ESP32**
- **Arduino Nano 33 IoT**

Ces alternatives apportent plus de marge de calcul, mais elles complexifient davantage la logique de niveau et n'apportent pas d'avantage decisif si le produit reste centre sur le MIDI DIN.

---

## 4. Architecture hardware recommandee

### 4.1 Vue d'ensemble

Architecture recommandee :

- 1 carte controleur Arduino Nano Every
- 1 sous-ensemble MIDI DIN IN
- 1 sous-ensemble MIDI DIN OUT
- 2 sorties Gate bufferisees
- 1 afficheur I2C
- 1 encodeur rotatif a poussoir
- 3 rangees de boutons
- chaines de registres a decalage pour lire les boutons et piloter les LEDs
- stockage non volatil externe pour les presets

### 4.2 Blocs fonctionnels

```text
Alimentation
  -> rail 5 V
  -> carte controleur
  -> afficheur
  -> logique UI
  -> MIDI OUT
  -> Gate OUT A / B

MIDI IN DIN
  -> optocoupleur
  -> RX UART

MCU
  -> TX UART vers MIDI OUT
  -> I2C vers ecran + memoire
  -> SPI / GPIO vers 74HC165 et 74HC595
  -> GPIO bufferises vers Gate OUT A / B
  -> GPIO dedies vers encodeur
```

---

## 5. Interfaces musicales

### 5.1 MIDI IN

Le MIDI IN doit suivre une implementation **opto-isolee** conforme a l'esprit de la specification MIDI DIN.

Recommandations :

- connecteur **DIN 5 broches femelle châssis**
- entree opto-isolee
- respect du courant loop MIDI standard
- pas de contournement de l'isolation

Composant recommande pour l'entree :

- **6N138**

Alternative acceptable :

- **H11L1**

Note :

- le schema exact de l'etage MIDI IN devra etre verrouille avec les bonnes valeurs de resistances et de diode selon l'opto retenu
- il faut absolument conserver une implementation propre et isolee, pas un pseudo-MIDI simplifie

### 5.2 MIDI OUT

Le MIDI OUT doit suivre la logique du **current loop MIDI DIN 5 V**.

Recommandations :

- connecteur **DIN 5 broches femelle châssis**
- etage de sortie protege entre le MCU et la prise
- resistances de limitation conformes au montage retenu

Recommandation de robustesse :

- inserer un **buffer** entre le microcontroleur et la sortie MIDI

Composant recommande :

- **74HC14** ou equivalent Schmitt trigger / buffer

Cela protege mieux le microcontroleur et laisse des portes disponibles pour d'autres usages simples si besoin.

### 5.3 Messages MIDI a gerer

La V1 doit gerer au minimum :

- Note On
- Note Off
- Clock
- Start
- Stop
- Continue

Le parsing MIDI devra etre tolerant aux flux reels et aux messages interleaves.

### 5.4 Sorties Gate

La machine doit fournir deux sorties Gate dediees :

- **Gate Out A**
- **Gate Out B**

Direction recommande pour la V1 :

- sorties **5 V**
- sorties **digitales**
- pas de CV de pitch
- etage de sortie **bufferise**

Comportement fonctionnel recommande :

- en mode **Dual**, Gate A suit la piste A et Gate B suit la piste B
- en mode **Chain 20**, Gate A reste lie aux pas 1 a 10 et Gate B reste lie aux pas 11 a 20

Recommandations hardware :

- ne pas exposer directement une broche MCU sur un jack de sortie
- utiliser un buffer ou un petit etage transistorise
- ajouter une resistance serie de protection selon le schema retenu

Connectique recommandee :

- **2 jacks mono 3.5 mm** pour garder un format compact

Alternative acceptable :

- jacks mono 6.35 mm si tu privilegies un format plus desktop que compact

---

## 6. Interface utilisateur hardware

### 6.1 Boutons

La configuration cible comprend :

- 10 boutons piste A
- 10 boutons piste B
- 10 boutons de controle contextuel
- 6 boutons systeme minimum
- 1 poussoir integre a l'encodeur

Total cible de depart :

- **37 entrees utilisateur**

### 6.2 LEDs

Base recommandee :

- 10 LEDs piste A
- 10 LEDs piste B
- 4 LEDs systeme minimum

Total de depart :

- **24 sorties LEDs**

Si tu veux ajouter un retour lumineux sur la troisieme rangee, il faudra augmenter le nombre de sorties disponibles.

### 6.3 Encodeur

L'encodeur principal doit etre mecanique, a detentes franches, avec poussoir integre.

Reference recommandee :

- **Bourns PEC11H**

Ce type de composant est bien adapte a une gestuelle d'instrument plutot qu'a un simple menu d'appareil.

### 6.4 Ecran

Format recommande :

- **OLED 128 x 64**
- bus **I2C**
- format lisible de **0.96" a 1.3"**

Recommandation concrete :

- module compatible **SSD1306** ou **SH1106**

Le role de l'ecran est d'afficher :

- le pas edite
- la piste cible
- le parametre edite
- la valeur en cours
- l'etat global lecture / tempo / synchro

---

## 7. Strategie d'extension des E/S

### 7.1 Pourquoi des extensions sont necessaires

Le nombre total d'entrees et sorties depasse tres vite ce qu'il est raisonnable de connecter directement au microcontroleur.

Il faut donc prevoir une architecture simple, robuste et peu couteuse.

### 7.2 Strategie recommandee

Pour la V1, la solution la plus simple est :

- **74HC165** pour lire les boutons
- **74HC595** pour piloter les LEDs

Configuration de depart recommandee :

- **5 x 74HC165** pour couvrir jusqu'a 40 entrees
- **3 x 74HC595** pour couvrir jusqu'a 24 sorties

Avantages :

- cout faible
- composants faciles a sourcer
- firmware simple
- pas de matrice complexe
- pas de ghosting

### 7.3 Bus

La chaine d'extension pourra utiliser :

- bus SPI materiel si pratique
- ou protocole serie synchrone simple via GPIO

Recommandation :

- reserver les broches proches du SPI pour cette fonction

---

## 8. Stockage non volatil

### 8.1 Constat

Le stockage interne du Nano Every n'est pas suffisant pour stocker confortablement **8 a 16 presets complets** avec deux pistes, parametres, et etat global.

### 8.2 Recommandation

Ajouter une memoire non volatile externe sur I2C.

Deux options raisonnables :

- **FRAM I2C 32 KB**
- **EEPROM I2C 24LC256**

Recommandation principale :

- **FRAM I2C**

Pourquoi :

- ecritures tres rapides
- usure quasi negligeable
- logique de sauvegarde plus simple

---

## 9. Alimentation

### 9.1 Principe

Le produit doit etre pense comme un appareil hardware autonome.

La recommandation actuelle est :

- alimentation externe dediee
- rail principal **5 V**
- pas de dependance a l'USB pendant le jeu
- port USB reserve au service si la carte controleur en possede un

### 9.2 Direction recommandee

Direction materielle a privilegier :

- entree alim type **barrel jack**
- conversion propre vers **5 V**
- interrupteur general
- protection de polarite

Le choix exact du connecteur et du bloc alim reste a verrouiller pendant la phase schema.

---

## 10. Mapping de broches suggere

Le mapping exact dependra du routage final, mais une base saine est :

- `D0 / RX` : MIDI IN
- `D1 / TX` : MIDI OUT
- `D5` : Gate Out A
- `D6` : Gate Out B
- `A4 / SDA` : I2C ecran
- `A5 / SCL` : I2C memoire / ecran
- `D2` : encodeur A
- `D3` : encodeur B
- `D4` : encodeur poussoir
- `D10` : latch 74HC595
- `D11` : data 74HC595
- `D12` : data 74HC165
- `D13` : clock commun 74HC165 / 74HC595
- `D9` : load 74HC165

Le reste des broches peut servir a :

- LEDs systeme directes
- signaux de debug
- expansions futures

---

## 11. Architecture firmware cible

Le firmware doit etre structure en modules explicites.

Modules recommandes :

- `ClockEngine`
- `MidiDinEngine`
- `SequencerEngine`
- `UiScanner`
- `DisplayEngine`
- `StorageEngine`
- `App`

### 11.1 ClockEngine

Responsabilites :

- tempo interne
- synchronisation externe MIDI Clock
- conversion entre 24 PPQN MIDI et resolution interne
- transport global

### 11.2 Resolution temporelle

Le firmware ne doit pas se limiter brutalement a 24 PPQN pour les evenements internes.

Recommandation :

- utiliser une resolution interne de **96 PPQN**

Cela permet :

- gates plus propres
- affichage d'etats plus stable
- marge de precision pour ratchet et retriggers

Conversion :

- 1 tick MIDI externe = 4 ticks internes

### 11.3 SequencerEngine

Responsabilites :

- gestion des deux pistes
- mode Dual
- mode Chain 20
- avance de lecture
- application de la probabilite
- generation ratchet
- gate et velocity

### 11.4 MidiDinEngine

Responsabilites :

- emission des messages MIDI
- reception des messages de clock et transport
- abstraction simple sur le port serie MIDI

### 11.5 UiScanner

Responsabilites :

- lecture des boutons
- debounce
- lecture encodeur
- generation d'evenements UI

### 11.6 DisplayEngine

Responsabilites :

- ecran principal
- overlays contextuels
- timeout de retour automatique

### 11.7 StorageEngine

Responsabilites :

- lecture / ecriture des presets
- versionnement de structure
- validation des donnees chargees

---

## 12. Modele de donnees recommande

```cpp
struct Step {
  bool active;
  uint8_t degree;
  uint8_t probability;
  uint8_t ratchet;
  uint8_t gate;
  uint8_t velocity;
};

struct Track {
  Step steps[10];
  uint8_t length;
  uint8_t midiChannel;
  int8_t octaveOffset;
};

enum MachineMode {
  MODE_DUAL,
  MODE_CHAIN20
};

struct ProjectState {
  Track trackA;
  Track trackB;
  MachineMode machineMode;
  uint16_t tempoBpm_x10;
  uint8_t rootNote;
  uint8_t scaleId;
  uint8_t playMode;
};
```

---

## 13. Strategie logicielle

### 13.1 Boucle principale

Le firmware peut rester en architecture cooperative si :

- les scans UI sont reguliers
- l'affichage est limite en frequence
- les evenements MIDI sont traites rapidement

### 13.2 Priorites d'execution

Ordre de priorite recommande :

1. timing et evenements MIDI
2. avance du sequencer
3. scan UI
4. rafraichissement LEDs
5. affichage ecran
6. sauvegardes

### 13.3 Sauvegardes

La sauvegarde ne doit jamais bloquer la lecture.

Recommandations :

- sauvegarde explicite utilisateur
- eventuelle sauvegarde differee
- jamais de write long dans le chemin critique de clock

---

## 14. Arborescence de depot recommandee

```text
/docs
/firmware
/hardware
/bom
```

Contenu recommande :

- `docs/` : specs, notes d'architecture, journal technique
- `firmware/` : code Arduino / PlatformIO
- `hardware/` : schemas, PCB, facade, exports gerbers
- `bom/` : BOMs versionnees

---

## 15. Risques techniques principaux

Les principaux points a surveiller sont :

- bruit mecanique et debounce de l'encodeur
- charge CPU si l'affichage est trop bavard
- logique de presets si la memoire externe n'est pas definie tot
- lisibilite reelle de la facade si le panneau est trop compact
- timing si des traitements lents sont laisses dans la boucle critique

---

## 16. Evolutions firmware cibles

Les evolutions futures les plus interessantes pour ce projet sont :

- **Euclidean**
- **generation melodique par gamme**
- **variations generatives progressives**

### 16.1 Euclidean

Fonctions cibles :

- densite
- longueur
- rotation
- activation par piste

### 16.2 Generation melodique

Fonctions cibles :

- notes generees dans la gamme courante
- plage d'octaves
- profils de deplacement simples
- regeneration rapide d'une variante

### 16.3 Variations generatives

Fonctions cibles :

- seed memorisable
- mutation legere a chaque boucle
- dosage du chaos
- combinaison Euclidean pour le rythme et generation pour les notes

Ces fonctions restent largement dans le budget logique d'un **Nano Every** tant que l'interface reste simple et que l'affichage ne devient pas excessivement lourd.

---

## 17. Priorites d'implementation

Ordre recommande pour le developpement :

1. bring-up Nano Every + MIDI DIN OUT
2. reception MIDI Clock via DIN IN
3. bring-up Gate Out A / B
4. moteur de sequence minimal 1 piste
5. extension a 2 pistes / Chain 20
6. scan boutons et encodeur
7. ecran contextuel
8. stockage presets
9. raffinement UX

Cette sequence me permet de prendre en charge le firmware de facon propre pendant que tu avances sur l'assemblage.
