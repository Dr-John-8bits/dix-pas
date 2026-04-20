# DIX PAS - Document technique

Note documentaire :

- ce document décrit la mise en oeuvre technique recommandée
- en cas de conflit produit ou scope, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte

## 1. Objet du document

Ce document décrit une base technique cohérente pour lancer le développement du projet **DIX PAS** en matériel open source.

Il ne remplace pas les schémas électroniques ni le PCB final, mais il fixe :

- les choix techniques déjà assumés
- l'architecture hardware recommandée
- l'architecture firmware cible
- les contraintes d'intégration entre matériel et logiciel

---

## 2. Choix figés à ce stade

Les choix suivants sont consideres comme valides pour la V1 :

- format musical principal : **2 pistes de 10 pas**
- mode alternatif : **Chain 20**
- connectique live : **MIDI DIN 5 broches uniquement**
- interfaces MIDI requises : **1 MIDI IN + 1 MIDI OUT**
- sorties de contrôle requises : **Gate Out A + Gate Out B**
- ratchet V1 : **x1, x2, x3**
- contrôle prioritaire : **Probabilite > On/Off > Ratchet**
- affichage : **feedback contextuel immédiat à l'écran**

Choix exclus de la V1 :

- USB MIDI en usage normal
- TRS MIDI
- MIDI THRU obligatoire
- batterie intégrée

---

## 3. Plateforme contrôleur recommandée

### 3.1 Recommandation principale

La plateforme recommandée pour lancer le projet est :

- **Arduino Nano Every**

### 3.2 Pourquoi ce choix

Cette carte est la plus cohérente avec le projet à ce stade pour les raisons suivantes :

- logique **5 V**, ce qui simplifie à la fois le MIDI DIN et les sorties Gate
- **1 UART** matériel pour le MIDI DIN
- **USB séparé** du microcontrôleur principal sur la carte, utile pour le flash et le debug sans polluer l'interface MIDI live
- **48 KB Flash** et **6 KB SRAM**, plus confortables qu'un Arduino Micro pour un firmware avec écran, menu, presets et scan d'interface
- format compact et officiel Arduino
- bon point d'entrée pour un projet open source
- les fonctions **Euclidean** et **génératives** envisagées restent compatibles avec cette plateforme

### 3.3 Alternatives possibles

Alternatives raisonnables si besoin plus tard :

- **Arduino Nano ESP32**
- **Arduino Nano 33 IoT**

Ces alternatives apportent plus de marge de calcul, mais elles complexifient davantage la logique de niveau et n'apportent pas d'avantage decisif si le produit reste centré sur le MIDI DIN.

---

## 4. Architecture hardware recommandée

### 4.1 Vue d'ensemble

Architecture recommandée :

- 1 carte contrôleur Arduino Nano Every
- 1 sous-ensemble MIDI DIN IN
- 1 sous-ensemble MIDI DIN OUT
- 2 sorties Gate bufferisées
- 1 afficheur I2C
- 1 encodeur rotatif à poussoir
- 3 rangées de boutons
- chaînes de registres à décalage pour lire les boutons et piloter les LEDs
- stockage non volatil externe pour les presets

### 4.2 Blocs fonctionnels

```text
Alimentation
  -> rail 5 V
  -> carte contrôleur
  -> afficheur
  -> logique UI
  -> MIDI OUT
  -> Gate OUT A / B

MIDI IN DIN
  -> optocoupleur
  -> RX UART

MCU
  -> TX UART vers MIDI OUT
  -> I2C vers écran + mémoire
  -> SPI / GPIO vers 74HC165 et 74HC595
  -> GPIO bufferises vers Gate OUT A / B
  -> GPIO dédiés vers encodeur
```

---

## 5. Interfaces musicales

### 5.1 MIDI IN

Le MIDI IN doit suivre une implémentation **opto-isolée** conforme à l'esprit de la spécification MIDI DIN.

Recommandations :

- connecteur **DIN 5 broches femelle châssis**
- entrée opto-isolée
- respect du courant loop MIDI standard
- pas de contournement de l'isolation

Composant recommandé pour l'entrée :

- **6N138**

Alternative acceptable :

- **H11L1**

Note :

- le schéma exact de l'étage MIDI IN devra être verrouille avec les bonnes valeurs de résistances et de diode selon l'opto retenu
- il faut absolument conserver une implémentation propre et isolée, pas un pseudo-MIDI simplifié

### 5.2 MIDI OUT

Le MIDI OUT doit suivre la logique du **current loop MIDI DIN 5 V**.

Recommandations :

- connecteur **DIN 5 broches femelle châssis**
- étage de sortie protégé entre le MCU et la prise
- résistances de limitation conformes au montage retenu

Recommandation de robustesse :

- insérer un **buffer** entre le microcontrôleur et la sortie MIDI

Composant recommandé :

- **74HC14** ou équivalent Schmitt trigger / buffer

Cela protégé mieux le microcontrôleur et laisse des portes disponibles pour d'autres usages simples si besoin.

### 5.3 Messages MIDI à gérer

La V1 doit gérer au minimum :

- Note On
- Note Off
- Clock
- Start
- Stop
- Continue

Le parsing MIDI devra être tolérant aux flux réels et aux messages interleaves.

### 5.4 Sorties Gate

La machine doit fournir deux sorties Gate dédiées :

- **Gate Out A**
- **Gate Out B**

Direction recommandée pour la V1 :

- sorties **5 V**
- sorties **digitales**
- pas de CV de pitch
- étage de sortie **bufferisé**

Comportement fonctionnel recommandé :

- en mode **Dual**, Gate A suit la piste A et Gate B suit la piste B
- en mode **Chain 20**, Gate A reste lié aux pas 1 à 10 et Gate B reste lié aux pas 11 à 20

Recommandations hardware :

- ne pas exposer directement une broche MCU sur un jack de sortie
- utiliser un buffer ou un petit étage transistorise
- ajouter une resistance série de protection selon le schéma retenu

Connectique recommandée :

- **2 jacks mono 3.5 mm** pour garder un format compact

Alternative acceptable :

- jacks mono 6.35 mm si tu privilégies un format plus desktop que compact

---

## 6. Interface utilisateur hardware

### 6.1 Boutons

La configuration cible comprend :

- 10 boutons piste A
- 10 boutons piste B
- 10 boutons de contrôle contextuel
- 6 boutons système minimum
- 1 poussoir intégré à l'encodeur

Total cible de départ :

- **37 entrées utilisateur**

### 6.2 LEDs

Base recommandée :

- 10 LEDs piste A
- 10 LEDs piste B
- 4 LEDs système minimum

Total de départ :

- **24 sorties LEDs**

Si tu veux ajouter un retour lumineux sur la troisième rangée, il faudra augmenter le nombre de sorties disponibles.

### 6.3 Encodeur

L'encodeur principal doit être mécanique, à détentes franches, avec poussoir intégré.

Référence recommandée :

- **Bourns PEC11H**

Ce type de composant est bien adapté à une gestuelle d'instrument plutôt qu'à un simple menu d'appareil.

### 6.4 Écran

Format recommandé :

- **OLED 128 x 64**
- bus **I2C**
- format lisible de **0.96" à 1.3"**

Recommandation concrète :

- module compatible **SSD1306** ou **SH1106**

Hypothèse firmware actuelle :

- cible principale : **SSD1306**
- adresse I2C OLED retenue par défaut : `0x3C`
- rendu texte sur `4 lignes`
- police bitmap simple intégrée au firmware, sans dépendance externe
- rendu paginé sans framebuffer complet pour préserver la RAM du `Nano Every`

Le rôle de l'écran est d'afficher :

- le pas édité
- la piste cible
- le paramètre édité
- la valeur en cours
- l'état global lecture / tempo / synchro

---

## 7. Stratégie d'extension des E/S

### 7.1 Pourquoi des extensions sont nécessaires

Le nombre total d'entrées et sorties dépasse très vite ce qu'il est raisonnable de connecter directement au microcontrôleur.

Il faut donc prévoir une architecture simple, robuste et peu coûteuse.

### 7.2 Stratégie recommandée

Pour la V1, la solution la plus simple est :

- **74HC165** pour lire les boutons
- **74HC595** pour piloter les LEDs

Configuration de départ recommandée :

- **5 x 74HC165** pour couvrir jusqu'à 40 entrées
- **3 x 74HC595** pour couvrir jusqu'à 24 sorties

Avantages :

- coût faible
- composants faciles à sourcer
- firmware simple
- pas de matrice complexe
- pas de ghosting

### 7.3 Bus

La chaîne d'extension pourra utiliser :

- bus SPI matériel si pratique
- ou protocole série synchrone simple via GPIO

Recommandation :

- réserver les broches proches du SPI pour cette fonction

---

## 8. Stockage non volatil

### 8.1 Constat

Le stockage interne du Nano Every n'est pas suffisant pour stocker confortablement **8 à 16 presets complets** avec deux pistes, paramètres et état global.

### 8.2 Recommandation

Ajouter une mémoire non volatile externe sur I2C.

Deux options raisonnables :

- **FRAM I2C 32 KB**
- **EEPROM I2C 24LC256**

Recommandation principale :

- **FRAM I2C**

Pourquoi :

- ecritures très rapides
- usure quasi negligeable
- logique de sauvegarde plus simple

Hypothèse firmware actuelle :

- adresse I2C FRAM retenue : `0x50`
- adressage mémoire sur `16 bits`
- transferts découpés en petits blocs pour rester compatibles avec les limites de buffer Arduino

---

## 9. Alimentation

### 9.1 Principe

Le produit doit être pensé comme un appareil hardware autonome.

La recommandation actuelle est :

- alimentation externe dédiée
- rail principal **5 V**
- pas de dépendance à l'USB pendant le jeu
- port USB réservé au service si la carte contrôleur en possède un

### 9.2 Direction recommandée

Direction matérielle à privilégier :

- entrée alim type **barrel jack**
- conversion propre vers **5 V**
- interrupteur general
- protection de polarite

Le choix exact du connecteur et du bloc alim reste à verrouiller pendant la phase schéma.

---

## 10. Mapping de broches suggere

Le mapping exact dependra du routage final, mais une base saine est :

- `D0 / RX` : MIDI IN
- `D1 / TX` : MIDI OUT
- `D5` : Gate Out A
- `D6` : Gate Out B
- `A4 / SDA` : I2C écran
- `A5 / SCL` : I2C mémoire / écran
- `D2` : encodeur A
- `D3` : encodeur B
- `D4` : encodeur poussoir
- `D10` : latch 74HC595
- `D11` : data 74HC595
- `D12` : data 74HC165
- `D13` : clock commun 74HC165 / 74HC595
- `D9` : load 74HC165

Le reste des broches peut servir à :

- LEDs système directes
- signaux de debug
- expansions futures

---

## 11. Architecture firmware cible

Le firmware doit être structure en modules explicites.

Modules recommandés :

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

Le firmware ne doit pas se limiter brutalement à 24 PPQN pour les événements internes.

Recommandation :

- utiliser une resolution interne de **96 PPQN**

Cela permet :

- gates plus propres
- affichage d'états plus stable
- marge de precision pour ratchet et retriggers

Conversion :

- 1 tick MIDI externe = 4 ticks internes

### 11.3 SequencerEngine

Responsabilites :

- gestion des deux pistes
- mode Dual
- mode Chain 20
- avance de lecture
- application de la probabilité
- generation ratchet
- gate et velocity

### 11.4 MidiDinEngine

Responsabilites :

- emission des messages MIDI
- reception des messages de clock et transport
- abstraction simple sur le port série MIDI

### 11.5 UiScanner

Responsabilites :

- lecture des boutons
- debounce
- lecture encodeur
- generation d'evenements UI

### 11.6 DisplayEngine

Responsabilites :

- écran principal
- overlays contextuels
- timeout de retour automatique

### 11.7 StorageEngine

Responsabilites :

- lecture / ecriture des presets
- versionnement de structure
- validation des donnees chargees

---

## 12. Modele de donnees recommandé

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

## 13. Stratégie logicielle

### 13.1 Boucle principale

Le firmware peut rester en architecture cooperative si :

- les scans UI sont reguliérs
- l'affichage est limite en frequence
- les evenements MIDI sont traites rapidement

### 13.2 Priorités d'exécution

Ordre de priorité recommandé :

1. timing et evenements MIDI
2. avance du sequencer
3. scan UI
4. rafraichissement LEDs
5. affichage écran
6. sauvegardes

### 13.3 Sauvegardes

La sauvegarde ne doit jamais bloquer la lecture.

Recommandations :

- sauvegarde explicite utilisateur
- eventuelle sauvegarde differee
- jamais de write long dans le chemin critique de clock

---

## 14. Arborescence de depot recommandée

```text
/docs
/firmware
/hardware
/bom
```

Contenu recommandé :

- `docs/` : specs, notes d'architecture, journal technique
- `firmware/` : code Arduino / PlatformIO
- `hardware/` : schémas, PCB, facade, exports gerbers
- `bom/` : BOMs versionnees

---

## 15. Risques techniques principaux

Les principaux points à surveiller sont :

- bruit mécanique et debounce de l'encodeur
- charge CPU si l'affichage est trop bavard
- logique de presets si la mémoire externe n'est pas definie tot
- lisibilite réelle de la facade si le panneau est trop compact
- timing si des traitements lents sont laisses dans la boucle critique

---

## 16. Evolutions firmware cibles

Les évolutions futures les plus interessantes pour ce projet sont :

- **Euclidean**
- **generation melodique par gamme**
- **variations génératives progressives**

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

### 16.3 Variations génératives

Fonctions cibles :

- seed memorisable
- mutation légère à chaque boucle
- dosage du chaos
- combinaison Euclidean pour le rythme et generation pour les notes

Ces fonctions restent largement dans le budget logique d'un **Nano Every** tant que l'interface reste simple et que l'affichage ne devient pas excessivement lourd.

---

## 17. Priorites d'implémentation

Ordre recommandé pour le développement :

1. bring-up Nano Every + MIDI DIN OUT
2. reception MIDI Clock via DIN IN
3. bring-up Gate Out A / B
4. moteur de séquence minimal 1 piste
5. extension à 2 pistes / Chain 20
6. scan boutons et encodeur
7. écran contextuel
8. stockage presets
9. raffinement UX

Cette séquence me permet de prendre en charge le firmware de facon propre pendant que tu avances sur l'assemblage.
