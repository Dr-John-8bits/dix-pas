# DIX PAS - Audit logiciel / firmware / fonctionnel avant achat matériel

Date :

- **2026-04-20**

## 1. Objet

Ce document fige l'état de l'audit logiciel avant achat matériel.

Objectif :

- vérifier que le projet a été poussé au maximum raisonnable côté logiciel sans hardware
- corriger les écarts logiciels bloquants encore fermables avant achat
- séparer clairement ce qui est déjà validé en simulation de ce qui ne pourra être validé qu'avec les composants réels

---

## 2. Chaîne de validation exécutée

Builds relancés avec succès :

- `pio run -e native`
- `pio run -e native_checks`
- `./.pio/build/native_checks/program`
- `pio run -e nanoevery`

Résultat du runner natif :

- **20 / 20 checks OK**

Empreinte actuelle `Nano Every` :

- **RAM** : `4033 / 6144` (`65.6 %`)
- **Flash** : `33222 / 48640` (`68.3 %`)

---

## 3. Points validés côté logiciel

### 3.1 Moteur musical

Validé :

- `2 x 10 pas`
- mode `Dual`
- mode `Chain 20`
- `Forward`
- `Reverse`
- `PingPong`
- `Random`
- probabilité par pas
- `On / Off`
- ratchet `x1 / x2 / x3`
- gate length
- velocity
- quantification systématique sur gamme active
- gammes V1 :
  - `Major`
  - `Minor`
  - `Harmonic Minor`
  - `Pentatonic Major`
  - `Pentatonic Minor`
  - `Dorian`
  - `Phrygian`
  - `Lydian`
  - `Mixolydian`
  - `Locrian`
  - `Whole Tone`
  - `Chromatic`

### 3.2 Transport et clock

Validé :

- clock interne `96 PPQN`
- clock MIDI externe
- `Start`
- `Stop`
- `Continue`
- conversion `24 PPQN -> 96 PPQN`
- démarrage firmware en état `STOP`

### 3.3 MIDI et Gate

Validé côté firmware :

- encodage `MIDI DIN OUT`
- parser `MIDI DIN IN` temps réel
- `Gate Out A`
- `Gate Out B`
- canal indépendant par piste en `Dual`
- canal unique en `Chain 20`
- séparation correcte des gates en `Chain 20`

### 3.4 UI / UX firmware

Validé :

- sélection de pas piste A
- sélection de pas piste B
- rangée 3 `Probability / OnOff / Ratchet`
- encodeur + bouton encodeur
- `Shift`
- `Mode short`
- `Mode long`
- `Global Edit`
- sélection `Clock Internal / External`
- sélection `Dual / Chain20`
- presets via `encoder button`
- slots génératifs via `encoder button`
- splash boot :
  - `DIX PAS`
  - `by Dr. John`
  - version firmware
- retour automatique à l'écran principal

### 3.5 Stockage et persistance

Validé :

- format preset fixe
- `CRC16`
- `8` slots
- backend mémoire de test
- backend `FRAM I2C`
- chunking I2C
- rappel du slot de démarrage préféré

### 3.6 Bring-up logiciel

Validé :

- mode diagnostic via `SHIFT + RESET`
- mode test `MIDI / Gate` via `SHIFT + PLAY`
- rendu OLED simulé
- driver OLED `SSD1306`
- backend boutons `74HC165`
- backend LEDs `74HC595`
- configuration hardware centralisée

---

## 4. Écarts réels trouvés puis corrigés dans cet audit

### 4.1 Logique de démarrage

Écart corrigé :

- le firmware démarrait sur un projet de démonstration si la FRAM était absente ou si le preset de démarrage était invalide
- ce comportement ne respectait pas la source de vérité

Correction appliquée :

- création d'une logique de démarrage dédiée et testable
- fallback explicite vers le projet de secours
- tentative du `dernier preset chargé`, puis du `dernier preset sauvegardé`
- message de démarrage cohérent :
  - `Loaded Px`
  - `Preset Error`
  - `Default Project`
  - `No FRAM`
  - `No FRAM/OLED`

### 4.2 Cohérence du mode test hardware en `Chain 20`

Écart corrigé :

- le mode test `MIDI / Gate` pouvait encore utiliser le canal MIDI de la piste B alors que `Chain 20` doit exposer un canal unique

Correction appliquée :

- le mode test utilise maintenant lui aussi le canal unique de `Chain 20`

---

## 5. Couverture de test réellement obtenue

Checks natifs explicitement validés :

- registre des gammes et quantification
- roundtrip de preset
- logique de priorité du slot de démarrage
- fallback de démarrage sans stockage
- fallback vers `last_saved` si `last_loaded` est invalide
- backend `FRAM I2C`
- `UiScanner`
- OLED mocké
- écran de boot
- overlay de sortie de boot
- clock MIDI externe
- slots génératifs via UI
- sélection `ClockSource`
- sélection `MachineMode`
- canal visible unique en `Chain 20`
- mode test hardware cohérent en `Chain 20`
- routage `Chain 20` :
  - canal unique
  - split correct des gates
- diagnostic
- affichage du dernier événement `MIDI IN`
- bascule et cycle du mode test `MIDI / Gate`

---

## 6. Ce qui ne peut plus être prouvé sans matériel

Les points suivants ne relèvent plus d'un manque logiciel, mais d'une validation physique :

- niveau réel des sorties `Gate Out`
- conformité électrique réelle de `MIDI IN`
- conformité électrique réelle de `MIDI OUT`
- ordre réel des bits des chaînes `74HC165 / 74HC595`
- adresses I2C réelles et stabilité du bus
- comportement mécanique réel de l'encodeur
- comportement mécanique réel des boutons
- lisibilité réelle de l'OLED en façade
- qualité réelle de l'alimentation finale
- immunité au bruit et à la masse sur la machine câblée

---

## 7. Verdict

Verdict d'audit :

- **aucun blocage logiciel majeur restant n'a été identifié après correction**
- **la chaîne firmware est poussée au maximum utile avant réception du matériel**
- **le prochain vrai front de risque est désormais hardware, pas logiciel**

Formulation honnête :

- on ne peut pas certifier l'électronique sans composants réels
- mais on peut certifier que le projet n'a plus de dette logicielle évidente qui justifierait de retarder l'achat du matériel

En pratique :

- l'achat des composants peut être lancé
- la suite doit être une phase `assemblage -> bring-up -> debug réel`
- on ne doit plus être en phase de conception logicielle de base

