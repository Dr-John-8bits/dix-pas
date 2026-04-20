# DIX PAS - Firmware

Ce dossier contient maintenant la base du firmware et les outils de simulation desktop.

Structure retenue :

- `src/` : point d'entree firmware
- `include/` : headers projet
- `lib/` : modules internes reutilisables
- `test/` : tests unitaires ou d'integration
- `sim/` : simulateur desktop et outils Mac sans hardware

Objectif de depart :

- isoler le coeur du sequenceur du materiel
- permettre des tests logiques sur Mac avant l'integration Arduino

Modules actuellement poses :

- `ClockEngine`
- `SequencerEngine`
- `App`

Capacites deja implementees :

- clock interne a `96 PPQN`
- pistes `A` et `B`
- mode `Dual`
- mode `Chain 20`
- play modes `Forward`, `Reverse`, `PingPong`, `Random`
- probabilite par pas
- ratchet `x1` a `x3`
- gate length par pas
- velocity par pas
- generation d'evenements `Note On/Off` et `Gate On/Off`

Builds verifies :

- simulation native Mac
- cible `Arduino Nano Every`

Commandes utiles :

```bash
cd firmware
export PLATFORMIO_CORE_DIR="$PWD/.pio-core"

pio run -e native
pio run -e nanoevery
```

Note :

- `PLATFORMIO_CORE_DIR` pointe PlatformIO vers un dossier local au projet
- cela evite de dependre de `~/.platformio` pour ce repo
