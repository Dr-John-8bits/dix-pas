# DIX PAS - Audit pré-achat

Date :

- **2026-04-20**

Objet :

- vérifier que le projet est suffisamment mûr avant gel de la BOM et achat matériel
- distinguer ce qui est déjà validé, ce qui doit être corrigé avant achat, et ce qui ne pourra être vérifié qu'avec le hardware

Statut :

- ce document est un **snapshot d'audit**
- certains écarts identifiés ici ont été corrigés juste après cet audit
- les documents de référence courants restent prioritaires sur cette photo d'état

## 1. Méthode

Documents relus :

- `docs/product/DIX_PAS_SOURCE_DE_VERITE.md`
- `docs/product/DIX_PAS_DECISIONS.md`
- `docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md`
- `docs/engineering/DIX_PAS_document_technique.md`
- `docs/engineering/DIX_PAS_UI_STATE_MACHINE.md`
- `docs/engineering/DIX_PAS_PRESET_FORMAT.md`
- `docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md`
- `docs/setup/DIX_PAS_BRING_UP_HARDWARE.md`
- `docs/setup/DIX_PAS_CABLAGE_REFERENCE.md`
- `bom/DIX_PAS_BOM_v0.md`

Firmware relu :

- moteur séquenceur
- UI controller / UI scanner / UI hardware
- stockage FRAM
- OLED
- MIDI DIN IN / OUT
- bring-up Arduino dans `main.cpp`

Vérifications exécutées :

- `pio run -e native_checks`
- `./.pio/build/native_checks/program`
- `pio run -e native`
- `pio run -e nanoevery`

Résultat :

- `15 / 15` checks natifs OK
- build `native` OK
- build `nanoevery` OK
- empreinte actuelle `Nano Every` :
  - `RAM 65.6 %`
  - `Flash 68.1 %`

## 2. État global

Conclusion synthétique :

- le projet est **très avancé côté documentation et firmware**
- la **V1 logique** est déjà largement en place
- le projet n'est **pas encore prêt pour figer une BOM exacte de commande**
- il reste quelques **écarts documentaires et de cadrage matériel** à fermer avant achat

## 3. Points à corriger avant achat

### 3.1 Boutons système : écart réel entre BOM, doc technique, pin map et firmware

Constat :

- la BOM liste **6 boutons système** et cite `Select`
- la doc technique compte **6 boutons système minimum**
- le pin map parle aussi de **6 boutons système**
- le firmware, lui, ne câble et ne lit que **5 boutons système** :
  - `PLAY`
  - `STOP`
  - `RESET`
  - `MODE`
  - `SHIFT`
- il n'existe pas de bouton `Select` implémenté dans l'UI actuelle

Impact :

- risque de commander un bouton de trop
- risque de figer un panneau avant incohérent avec le firmware réel
- le total d'entrées utilisateur documenté n'est pas homogène

Décision à prendre :

- soit **supprimer officiellement `Select`** de la BOM et de la doc
- soit **ajouter un vrai `Select`** au firmware et au panneau

Recommandation :

- **supprimer `Select`** de la BOM et réaligner toute la doc sur `5 boutons système + 1 encodeur poussoir`

Références :

- `bom/DIX_PAS_BOM_v0.md`
- `docs/engineering/DIX_PAS_document_technique.md`
- `docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md`
- `firmware/src/ui_hardware.cpp`

### 3.2 OLED : ambiguïté d'achat entre `SSD1306` et `SH1106`

Constat :

- la BOM et la doc technique présentent encore l'écran comme compatible `SSD1306 / SH1106`
- le firmware OLED actuel initialise explicitement un contrôleur `SSD1306`
- aucun backend `SH1106` n'est aujourd'hui implémenté

Impact :

- risque d'acheter un module `SH1106` qui ne fonctionnera pas sans adaptation logicielle
- risque de faux diagnostic au premier allumage alors que le problème vient simplement du contrôleur acheté

Décision à prendre :

- soit **figer une référence écran `SSD1306`**
- soit **implémenter un vrai support `SH1106`** avant achat

Recommandation :

- **geler l'achat sur un module `SSD1306` confirmé**, avec adresse `0x3C`

Références :

- `bom/DIX_PAS_BOM_v0.md`
- `docs/engineering/DIX_PAS_document_technique.md`
- `docs/setup/DIX_PAS_CABLAGE_REFERENCE.md`
- `firmware/src/oled_display.cpp`

### 3.3 UI en mode `Chain20` : le canal MIDI de la piste B reste éditable mais n'est pas utilisé

Constat :

- l'UI permet encore d'éditer le `MIDI Ch` de la piste focalisée
- en `Chain20`, le moteur utilise un **seul canal**
- ce canal unique est actuellement résolu à partir de `track_a.midi_channel`
- donc le `MIDI Ch` de la piste B devient un réglage visible mais sans effet

Impact :

- comportement potentiellement trompeur pour l'utilisateur
- risque de confusion pendant les tests et le bring-up

Décision à prendre :

- soit **masquer / verrouiller l'édition du canal B en `Chain20`**
- soit **documenter explicitement que le canal unique Chain20 est celui de la piste A**

Recommandation :

- **verrouiller l'UI sur un canal unique clair en `Chain20`**

Références :

- `firmware/src/ui_controller.cpp`
- `firmware/src/display_engine.cpp`
- `firmware/src/sequencer_engine.cpp`

## 4. Points encore ouverts avant gel de la BOM

Ces points ne sont pas des bugs firmware, mais ils empêchent encore de parler d'une BOM complètement figée :

- type exact d'alimentation externe
- schéma exact du `MIDI IN`
- schéma exact du buffer `MIDI OUT`
- topologie finale des `Gate Out`
- référence finale de l'écran
- type exact de boîtier
- décision finale sur les LEDs de la troisième rangée

Sources :

- `bom/DIX_PAS_BOM_v0.md`
- `docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md`

Conséquence :

- on peut continuer à préparer le projet et même acheter certains sous-ensembles sûrs
- mais on ne doit pas encore figer une **commande exhaustive et exacte**

## 5. Ce qui est déjà validé

### 5.1 Produit / UX

Validé logiquement :

- `2 x 10 pas`
- `Chain20`
- priorité d'édition `Probabilité > On/Off > Ratchet`
- `ratchet x1 / x2 / x3`
- `gate`
- `velocity`
- gammes globales quantized, dont `Chromatic`
- transport `Play / Stop / Reset`
- `clock interne`
- `clock MIDI externe`
- `MIDI DIN IN / OUT`
- `Gate Out A / B`
- presets
- splash boot
- diagnostic
- mode test `MIDI / Gate`

### 5.2 Firmware

Validé par build et checks :

- architecture compilable sur Mac
- architecture compilable sur `Nano Every`
- stockage presets avec metadata et CRC
- backend FRAM I2C
- backend OLED I2C
- UI scanner
- UI hardware
- mapping façade centralisé
- guide de bring-up existant

## 6. Ce qui restera à vérifier uniquement avec le matériel

Ces points ne peuvent pas être prouvés complètement avant réception des composants :

- ordre réel des bits `74HC165`
- ordre réel des sorties `74HC595`
- comportement électrique réel du `MIDI DIN IN`
- comportement électrique réel du `MIDI DIN OUT`
- niveau réel des `Gate Out`
- adresses réelles `OLED / FRAM`
- qualité du bus `I2C`
- comportement mécanique réel de l'encodeur
- bruit, rebonds et confort des boutons
- alimentation réelle et qualité des masses

## 7. Recommandation finale

Recommandation de l'audit :

1. corriger les **3 écarts prioritaires** ci-dessus
2. figer ensuite la **BOM exacte**
3. seulement après, lancer l'achat matériel

État du projet à cette date :

- **prêt pour une phase de correction pré-achat**
- **pas encore prêt pour un gel BOM définitif**
- **très bien préparé pour un bring-up matériel rapide une fois ces écarts fermés**
