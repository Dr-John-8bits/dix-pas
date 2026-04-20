# DIX PAS - Firmware

Ce dossier contient la base du firmware et les outils de simulation desktop.

Structure retenue :

- `src/` : point d'entrée firmware
- `include/` : headers projet
- `lib/` : modules internes réutilisables
- `test/` : tests unitaires ou d'intégration
- `sim/` : simulateur desktop et outils Mac sans hardware

Objectif de départ :

- isoler le cœur du séquenceur du matériel
- permettre des tests logiques sur Mac avant l'intégration Arduino

Modules actuellement posés :

- `ClockEngine`
- `SequencerEngine`
- `App`
- `MidiDinEngine`
- `MidiDinInputEngine`
- `GateOutputEngine`
- `StorageEngine`
- `FramI2cBackend`
- `UiController`
- `UiScanner`
- `UiHardware`
- `PanelLedDriver`
- `GenerativeEngine`
- `DisplayEngine`
- `OledDisplay`

Capacités déjà implémentées :

- clock interne à `96 PPQN`
- pistes `A` et `B`
- mode `Dual`
- mode `Chain 20`
- play modes `Forward`, `Reverse`, `PingPong`, `Random`
- gammes globales `Major`, `Minor`, `Harmonic Minor`, `Pentatonic Major`, `Pentatonic Minor`, `Dorian`, `Phrygian`, `Lydian`, `Mixolydian`, `Locrian`, `Whole Tone`, `Chromatic`
- probabilité par pas
- quantification systématique sur la gamme active, sans micro-tonalité en V1
- ratchet `x1` à `x3`
- gate length par pas
- velocity par pas
- génération d'événements `Note On/Off` et `Gate On/Off`
- encodage `MIDI DIN` en octets
- réception `MIDI DIN` temps réel (`Clock`, `Start`, `Continue`, `Stop`)
- support clock interne et clock externe MIDI
- état runtime des deux `Gate Out`
- sauvegarde / chargement de presets via slots fixes + `CRC16`
- backend `FRAM I2C` prêt pour l'Arduino avec probing, adressage 16 bits et transferts découpés
- logique UI pour sélection de pas, rangée 3, encodeur, global edit
- scanner d'entrées hardware-agnostique avec debounce, appui court / long et encodeur quadrature
- backend Arduino pour lecture encodeur direct + chaîne `74HC165`
- driver Arduino pour la façade LEDs via chaîne `74HC595`
- fondation V2 pour `Euclidean`, génération mélodique par gamme et mutation légère
- exposition UI légère des slots génératifs via `Global Edit`
- chargement d'un preset via `encoder button` sur `Preset`
- sauvegarde explicite via `SHIFT + encoder button` sur `Preset`
- application d'un slot génératif via `encoder button` sur `Generative`
- mutation d'un slot génératif via `SHIFT + encoder button` sur `Generative`
- rendu d'un écran texte simulant l'OLED
- adaptateur OLED I2C matériel `SSD1306` prêt pour l'Arduino
- rendu OLED paginé sans framebuffer complet pour économiser la RAM du `Nano Every`
- splash de boot avec branding et version firmware
- tentative de chargement automatique du dernier preset connu au démarrage
- boot Arduino en état `STOP`, sans auto-play

Fondation générative déjà posée :

- génération `Euclidean` par piste
- profils mélodiques `Ascending`, `Descending`, `PingPong`, `Random`, `RandomWalk`, `Alternating`
- mutation légère reproductible à partir d'un `seed`
- combinaison rythme + gamme déjà testable en simulation native
- banque interne de slots génératifs déjà branchée à l'UI globale

Builds vérifiés :

- simulation native Mac
- runner local `native_checks`
- cible `Arduino Nano Every`

Le binaire natif actuel simule notamment :

- overlays d'édition
- save/load de presets
- apply/mutate de slots génératifs
- transport interne
- clock externe MIDI
- rendu texte de l'écran

Le runner `native_checks` valide actuellement :

- registre des gammes et quantification
- roundtrip de stockage preset
- roundtrip `FRAM I2C` mocké avec validation du chunking
- initialisation et rendu `OLED` mockés
- écran de boot et version firmware
- `UiScanner` pour `MODE short` / `MODE long`
- clock MIDI externe
- application / mutation des slots génératifs depuis l'UI

Commandes utiles :

```bash
cd firmware
export PLATFORMIO_CORE_DIR="$PWD/.pio-core"

pio run -e native
pio run -e native_checks
pio run -e nanoevery
./.pio/build/native/program
./.pio/build/native_checks/program
```

Note :

- `PLATFORMIO_CORE_DIR` pointe PlatformIO vers un dossier local au projet
- cela évite de dépendre de `~/.platformio` pour ce repo
