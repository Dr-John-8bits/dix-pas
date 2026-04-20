# DIX PAS - Exécution Step-by-Step

## 1. Objet

Ce document décrit **dans quel ordre exécuter le projet**.

But :

- ne pas improviser au moment de l'achat
- ne pas mélanger conception, assemblage et debug
- avancer par étapes validées

Règle simple :

- on ne passe jamais à l'étape suivante tant que l'étape courante n'est pas validée

Ce document est le **guide chef d'orchestre**.

Les autres docs servent de support :

- [DIX_PAS_SETUP_MAC.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_SETUP_MAC.md:1)
- [DIX_PAS_PLAN_PROTO_BREADBOARD.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_PLAN_PROTO_BREADBOARD.md:1)
- [DIX_PAS_CABLAGE_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_CABLAGE_REFERENCE.md:1)
- [DIX_PAS_BRING_UP_HARDWARE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_BRING_UP_HARDWARE.md:1)
- [DIX_PAS_TESTS_ACCEPTATION_V1.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md:1)
- [DIX_PAS_BOM_ACHAT_COEUR_V1.md](/Users/jdebaeck/Documents/Développements/Dix pas/bom/DIX_PAS_BOM_ACHAT_COEUR_V1.md:1)

---

## 2. Vue d'ensemble

Ordre global du projet :

1. valider l'environnement Mac
2. valider le firmware sans hardware
3. figer les achats cœur électronique
4. commander les composants phase 1
5. valider l'alimentation
6. valider `Nano Every`
7. valider `OLED + FRAM`
8. valider `MIDI OUT`
9. valider `Gate Out`
10. valider `MIDI IN`
11. valider encodeur
12. valider chaîne boutons
13. valider chaîne LEDs
14. intégrer le montage complet
15. exécuter les tests d'acceptation V1

---

## 3. Phase 0 - Mac prêt

Objectif :

- vérifier que le poste de travail est réellement prêt avant tout achat

À faire :

1. relire [DIX_PAS_SETUP_MAC.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_SETUP_MAC.md:1)
2. ouvrir un terminal dans le repo
3. exécuter :

```bash
cd "/Users/jdebaeck/Documents/Développements/Dix pas/firmware"
export PLATFORMIO_CORE_DIR="$PWD/.pio-core"
pio run -e native
pio run -e native_checks
./.pio/build/native_checks/program
pio run -e nanoevery
```

Validation :

- les trois builds passent
- le runner `native_checks` termine sans erreur

Si échec :

- corriger l'environnement avant tout achat

---

## 4. Phase 1 - Firmware gelé avant hardware

Objectif :

- s'assurer que la base logicielle est saine avant de toucher au circuit

À faire :

1. relire [firmware/README.md](/Users/jdebaeck/Documents/Développements/Dix pas/firmware/README.md:1)
2. vérifier que les docs de référence sont cohérentes
3. vérifier que les commandes suivantes passent encore :

```bash
cd "/Users/jdebaeck/Documents/Développements/Dix pas/firmware"
export PLATFORMIO_CORE_DIR="$PWD/.pio-core"
pio run -e native
./.pio/build/native/program
pio run -e native_checks
./.pio/build/native_checks/program
pio run -e nanoevery
```

Validation :

- simulation native OK
- checks OK
- build `nanoevery` OK

Si échec :

- ne pas commander tant que le firmware n'est pas revenu au vert

---

## 5. Phase 2 - Achats phase 1

Objectif :

- acheter uniquement les sous-ensembles nécessaires pour valider le cœur électronique

Base d'achat :

- [DIX_PAS_BOM_ACHAT_COEUR_V1.md](/Users/jdebaeck/Documents/Développements/Dix pas/bom/DIX_PAS_BOM_ACHAT_COEUR_V1.md:1)

À acheter d'abord :

- alimentation `9 V`
- buck `5 V`
- `Arduino Nano Every`
- OLED
- FRAM
- `SN74HC125N`
- `6N138`
- `DIN IN / OUT`
- `74HC165`
- `74HC595`
- encodeur
- breadboards
- passifs minimum

Ne pas attendre :

- le boîtier
- la façade finale
- la visserie finale

Validation :

- la commande phase 1 couvre bien toutes les étapes breadboard `1` à `9`

---

## 6. Phase 3 - Préparation avant premier montage

Objectif :

- préparer le banc de test avant la première soudure ou le premier câblage

À faire :

1. imprimer ou garder ouvertes :
   - [DIX_PAS_PLAN_PROTO_BREADBOARD.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_PLAN_PROTO_BREADBOARD.md:1)
   - [DIX_PAS_CABLAGE_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_CABLAGE_REFERENCE.md:1)
   - [DIX_PAS_BRING_UP_HARDWARE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_BRING_UP_HARDWARE.md:1)
2. préparer :
   - multimètre
   - USB de flash
   - câble MIDI DIN
   - cible MIDI ou interface de test

Validation :

- tous les documents et outils sont prêts avant le premier montage

---

## 7. Phase 4 - Alimentation

Objectif :

- valider le `9 V -> buck 5 V`

Doc à suivre :

- [DIX_PAS_ALIMENTATION_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/engineering/DIX_PAS_ALIMENTATION_REFERENCE.md:1)
- [DIX_PAS_PLAN_PROTO_BREADBOARD.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_PLAN_PROTO_BREADBOARD.md:1)

À faire :

1. câbler l'entrée `9 V`
2. câbler l'interrupteur si disponible
3. câbler le buck
4. mesurer le rail `5 V`

Validation :

- `5 V` stable
- pas de chauffe anormale

Si échec :

- stop immédiat
- ne rien brancher d'autre

---

## 8. Phase 5 - Nano Every

Objectif :

- valider la carte seule sur le rail final

À faire :

1. brancher le `Nano Every`
2. connecter l'USB pour le flash
3. compiler et uploader :

```bash
cd "/Users/jdebaeck/Documents/Développements/Dix pas/firmware"
export PLATFORMIO_CORE_DIR="$PWD/.pio-core"
pio run -e nanoevery
pio run -e nanoevery -t upload
```

Validation :

- upload OK
- la carte boote
- pas de reset aléatoire

Si plusieurs ports série existent :

```bash
pio run -e nanoevery -t upload --upload-port /dev/tty.usbmodemXXXX
```

---

## 9. Phase 6 - OLED + FRAM

Objectif :

- valider le bus `I2C`

Doc à suivre :

- [DIX_PAS_CABLAGE_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_CABLAGE_REFERENCE.md:1)
- [DIX_PAS_BRING_UP_HARDWARE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_BRING_UP_HARDWARE.md:1)

À faire :

1. brancher OLED sur `A4 / A5`
2. brancher FRAM sur `A4 / A5`
3. reflasher si nécessaire
4. allumer la machine

Validation :

- splash boot visible
- écran principal visible
- statut `OLED / FRAM` cohérent dans le diagnostic

Commandes utiles ensuite :

- entrer dans le diagnostic avec `SHIFT + RESET`

Si échec :

- vérifier adresses `I2C`
- vérifier masse commune
- vérifier alimentation des modules

---

## 10. Phase 7 - MIDI OUT

Objectif :

- valider l'émission MIDI réelle

Doc à suivre :

- [DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/engineering/DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md:1)
- [DIX_PAS_BRING_UP_HARDWARE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_BRING_UP_HARDWARE.md:1)

À faire :

1. câbler l'étage `MIDI OUT`
2. connecter une machine ou une interface MIDI
3. entrer dans le mode test avec `SHIFT + PLAY`
4. laisser `MODE` sur `Both` ou `MIDI`

Validation :

- notes test reçues
- clock test reçue si observée
- écran cohérent

Si échec :

- vérifier `D1 / TX`
- vérifier buffer `74HC125`
- vérifier résistances `220 Ω`
- vérifier câblage DIN

---

## 11. Phase 8 - Gate Out

Objectif :

- valider les deux gates réelles

À faire :

1. câbler `Gate A`
2. câbler `Gate B`
3. entrer dans le mode test `SHIFT + PLAY`
4. utiliser `MODE` pour rester en `Both` ou `Gate`
5. mesurer les sorties

Validation :

- `Gate A` monte bien
- `Gate B` monte bien
- repos à l'état bas

Si échec :

- vérifier `D5 / D6`
- vérifier `74HC125`
- vérifier `220 Ω`
- vérifier `100 kΩ`
- vérifier masse commune

---

## 12. Phase 9 - MIDI IN

Objectif :

- valider la réception MIDI temps réel

À faire :

1. câbler l'étage `MIDI IN`
2. entrer dans le diagnostic `SHIFT + RESET`
3. envoyer `Clock`, `Start`, `Stop`, `Continue` depuis une source externe

Validation :

- le dernier événement `MIDI IN` change à l'écran

Si échec :

- vérifier orientation du `6N138`
- vérifier `1N4148`
- vérifier `220 Ω`, `10 kΩ`, `4.7 kΩ`
- vérifier câblage DIN

---

## 13. Phase 10 - Encodeur

Objectif :

- valider la navigation UI locale

À faire :

1. câbler `D2`, `D3`, `D4`
2. tourner l'encodeur
3. appuyer sur le poussoir
4. vérifier l'UI et les overlays écran

Validation :

- pas de sauts incohérents
- overlays affichés
- poussoir reconnu

---

## 14. Phase 11 - Première chaîne boutons

Objectif :

- valider la lecture `74HC165` avant façade complète

À faire :

1. câbler `1 x 74HC165`
2. brancher seulement quelques boutons de test
3. entrer dans le diagnostic
4. appuyer sur les boutons test

Validation :

- chaque bouton est vu correctement

Si échec :

- vérifier ordre de bits
- vérifier `actif bas`
- vérifier `PL`, `QH`, `CP`

---

## 15. Phase 12 - Première chaîne LEDs

Objectif :

- valider la sortie `74HC595`

À faire :

1. câbler `1 x 74HC595`
2. brancher quelques LEDs test
3. vérifier les LEDs système et les réactions UI

Validation :

- les bonnes LEDs s'allument au bon moment

Si échec :

- vérifier `SER`, `RCLK`, `SRCLK`
- vérifier résistances LED
- vérifier ordre réel des bits

---

## 16. Phase 13 - Façade complète

Objectif :

- passer du proto sous-ensembles à la vraie façade

À faire :

1. brancher la chaîne complète `74HC165`
2. brancher la chaîne complète `74HC595`
3. brancher toutes les rangées
4. brancher les boutons système
5. reflasher si nécessaire
6. relancer diagnostic et mode test

Validation :

- tout le panneau répond
- pas de décalage de mapping
- pas d'instabilité visible

---

## 17. Phase 14 - Bring-up complet

Objectif :

- valider la machine réelle comme instrument

Doc à suivre :

- [DIX_PAS_BRING_UP_HARDWARE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_BRING_UP_HARDWARE.md:1)

À faire :

1. boot normal
2. diagnostic
3. mode test `MIDI / Gate`
4. test clock interne
5. test clock externe
6. test presets
7. test édition UI

Validation :

- chaque bloc principal a été vu en vrai

---

## 18. Phase 15 - Acceptation V1

Objectif :

- décider si la machine est “V1 validée”

Doc à suivre :

- [DIX_PAS_TESTS_ACCEPTATION_V1.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md:1)

À faire :

1. exécuter les tests d'acceptation
2. noter les échecs réels
3. corriger uniquement les écarts réels

Validation :

- la majorité des tests V1 passent
- les écarts restants sont connus et acceptables

---

## 19. Règle d'arrêt

Il faut s'arrêter et corriger avant de continuer si :

- une tension d'alimentation est fausse
- le `5 V` est instable
- l'OLED ne boote pas
- la FRAM n'est pas détectée
- `MIDI OUT` est muet
- `Gate` reste bloqué haut ou bas
- `MIDI IN` reçoit de faux événements
- les boutons sont décalés
- les LEDs sont décalées

---

## 20. Ce document sert à quoi concrètement

Au moment où tu auras le matériel, tu devras suivre cet ordre :

- acheter
- monter
- flasher
- tester
- corriger
- seulement ensuite intégrer plus loin

Le but est qu'à ce moment-là on ne soit plus dans la conception.

On doit être dans :

- l'exécution
- la validation
- le debug réel
