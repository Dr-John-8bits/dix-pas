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
- `UiController`
- `DisplayEngine`

Capacités déjà implémentées :

- clock interne à `96 PPQN`
- pistes `A` et `B`
- mode `Dual`
- mode `Chain 20`
- play modes `Forward`, `Reverse`, `PingPong`, `Random`
- probabilité par pas
- ratchet `x1` à `x3`
- gate length par pas
- velocity par pas
- génération d'événements `Note On/Off` et `Gate On/Off`
- encodage `MIDI DIN` en octets
- réception `MIDI DIN` temps réel (`Clock`, `Start`, `Continue`, `Stop`)
- support clock interne et clock externe MIDI
- état runtime des deux `Gate Out`
- sauvegarde / chargement de presets via slots fixes + `CRC16`
- logique UI pour sélection de pas, rangée 3, encodeur, global edit
- chargement d'un preset via `encoder button` sur `Preset`
- sauvegarde explicite via `SHIFT + encoder button` sur `Preset`
- rendu d'un écran texte simulant l'OLED

Builds vérifiés :

- simulation native Mac
- cible `Arduino Nano Every`

Le binaire natif actuel simule notamment :

- overlays d'édition
- save/load de presets
- transport interne
- clock externe MIDI
- rendu texte de l'écran

Commandes utiles :

```bash
cd firmware
export PLATFORMIO_CORE_DIR="$PWD/.pio-core"

pio run -e native
pio run -e nanoevery
./.pio/build/native/program
```

Note :

- `PLATFORMIO_CORE_DIR` pointe PlatformIO vers un dossier local au projet
- cela évite de dépendre de `~/.platformio` pour ce repo
