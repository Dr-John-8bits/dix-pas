# DIX PAS - UI State Machine

Note documentaire :

- ce document figé le comportement de l'interface utilisateur pour accelerer le firmware
- en cas de conflit de scope produit, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte

## 1. Objet

Ce document décrit les états et transitions principaux de l'interface.

Le but est d'éviter une UI floue au moment de coder.

L'objectif est une machine :

- directe
- lisible
- rapide a jouer
- sans arborescence profonde

---

## 2. Principes generaux

Principes retenus :

- toujours garder une piste ciblee
- toujours garder un pas selectionne par piste
- faire de l'écran un feedback contextuel, pas un menu permanent
- reduire le nombre de vrais états UI
- preferer des overlays temporaires a des sous-menus

---

## 3. Variables d'état UI

Variables minimales a maintenir :

- `machine_mode` : `DUAL` ou `CHAIN20`
- `clock_source` : `INTERNAL` ou `EXTERNAL_MIDI`
- `transport_state` : `STOPPED` ou `PLAYING`
- `track_focus` : `A` ou `B`
- `selected_step_a` : `0..9`
- `selected_step_b` : `0..9`
- `row3_base_mode` : `PROB` ou `ONOFF`
- `shift_held` : `true / false`
- `encoder_target` : `NOTE`, `GATE`, `VELOCITY`
- `global_target` : `TEMPO`, `CLOCK_SOURCE`, `MACHINE_MODE`, `ROOT`, `SCALE`, `PLAYMODE`, `TRACK_LENGTH`, `MIDI_CHANNEL`, `PRESET_SLOT`, `GENERATIVE_SLOT`
- `ui_page` : `BOOT`, `HOME`, `STEP_EDIT`, `GLOBAL_EDIT`, `DIAGNOSTIC`, `HARDWARE_TEST`
- `overlay_active` : `true / false`

---

## 4. Etats principaux

### 4.1 BOOT

Description :

- état de démarrage

Actions :

- initialiser l'état machine
- charger les reglages par défaut ou le dernier contexte utile
- afficher un écran de boot court

Transition de sortie :

- `BOOT -> HOME`

### 4.2 HOME

Description :

- état normal de la machine
- visualisation du contexte courant

Affichage minimal :

- mode machine
- tempo ou sync externe
- piste ciblee
- pas courant
- état transport

Actions possibles :

- selection de pas
- édition rapide via rangée 3
- édition via encodeur
- transport
- entrée en édition globale

### 4.3 STEP_EDIT

Description :

- ce n'est pas une page lourde
- c'est un état logique de travail sur un pas

On y est quand :

- un pas est selectionne
- l'utilisateur tourne l'encodeur
- l'utilisateur agit sur la rangée 3

Affichage :

- overlay temporaire avec piste, pas, parametre et valeur

Sortie :

- retour automatique vers `HOME` après timeout d'inactivité

### 4.4 GLOBAL_EDIT

Description :

- édition des paramètres machine ou piste non liés a un bouton de pas direct

Contenu cible :

- tempo
- source d'horloge
- mode machine
- root
- scale
- play mode
- longueur de piste
- canal MIDI
- preset slot
- generative slot

Affichage :

- overlay ou page legere

Sortie :

- retour vers `HOME` par action utilisateur ou timeout selon le parametre

### 4.5 DIAGNOSTIC

Description :

- page de validation hardware
- n'édite pas la séquence

Accès :

- `SHIFT + RESET`

Affichage :

- état `FRAM / OLED`
- dernier événement de façade
- dernier événement `MIDI IN`
- état transport / clock

### 4.6 HARDWARE_TEST

Description :

- page de test dédiée au bring-up réel
- force des sorties connues sans dépendre du séquenceur normal

Accès :

- `SHIFT + PLAY`

Modes :

- `Both`
- `MIDI`
- `Gate`

Contrôles :

- `MODE` : change le sous-mode de test
- `STOP` : force toutes les sorties test à l'état bas
- `RESET` : relance le cycle depuis `A On`
- `SHIFT + PLAY` : quitte le mode test

---

## 5. Comportement des contrôles

## 5.1 Rangee 1

Rôle :

- selection des pas de la piste A

Effets :

- `track_focus = A`
- `selected_step_a = bouton_presse`
- activation de l'overlay de step

En mode `CHAIN20` :

- la rangée 1 represente les pas `1..10`

## 5.2 Rangee 2

Rôle :

- selection des pas de la piste B

Effets :

- `track_focus = B`
- `selected_step_b = bouton_presse`
- activation de l'overlay de step

En mode `CHAIN20` :

- la rangée 2 represente les pas `11..20`

## 5.3 Rangee 3

Rôle :

- édition rapide contextuelle

Comportement :

- si `shift_held = false`, la rangée agit selon `row3_base_mode`
- si `shift_held = true`, la rangée agit en mode `RATCHET`

Modes retenus :

- `PROB`
- `ONOFF`
- `RATCHET` temporaire via `SHIFT`

## 5.4 Encodeur

Rôle :

- édition fine du parametre courant

En `STEP_EDIT` :

- edite `encoder_target`

En `GLOBAL_EDIT` :

- edite `global_target`

## 5.5 Poussoir encodeur

Rôle retenu :

- cycle le `encoder_target` dans `STEP_EDIT`

Cycle recommandé :

- `NOTE -> GATE -> VELOCITY -> NOTE`

## 5.6 Bouton MODE

Appui court :

- en `HOME` ou `STEP_EDIT`, alterne `row3_base_mode`
- en `GLOBAL_EDIT`, avance `global_target`
- en `HARDWARE_TEST`, fait tourner `Both -> MIDI -> Gate`

Cycle recommandé :

- `PROB <-> ONOFF`

Appui long :

- entre ou sort de `GLOBAL_EDIT`

## 5.7 Bouton SHIFT

Rôle :

- modificateur momentane

Effets :

- si maintenu, la rangée 3 passe temporairement en `RATCHET`
- combiné a `RESET`, ouvre ou ferme `DIAGNOSTIC`
- combiné a `PLAY`, ouvre ou ferme `HARDWARE_TEST`

## 5.8 Bouton PLAY

Effet :

- passe `transport_state` a `PLAYING`
- si `SHIFT` est maintenu, ouvre ou ferme `HARDWARE_TEST`

## 5.9 Bouton STOP

Effet :

- passe `transport_state` a `STOPPED`
- force `Note Off`
- force `Gate Out A / B` a l'état bas
- en `HARDWARE_TEST`, force les sorties test a l'état bas sans quitter la page

## 5.10 Bouton RESET

Effet :

- remet la position de lecture au debut
- force les Gates a l'état bas
- si `SHIFT` est maintenu, ouvre ou ferme `DIAGNOSTIC`
- en `HARDWARE_TEST`, relance le cycle de test depuis `A On`

---

## 6. Global edit

## 6.1 Entree

`HOME -> GLOBAL_EDIT`

Condition :

- appui long sur `MODE`

## 6.2 Paramètres globaux

Ordre recommandé de cycle :

1. `TEMPO`
2. `CLOCK_SOURCE`
3. `MACHINE_MODE`
4. `ROOT`
5. `SCALE`
6. `PLAYMODE`
7. `TRACK_LENGTH`
8. `MIDI_CHANNEL`
9. `PRESET_SLOT`
10. `GENERATIVE_SLOT`

## 6.3 Navigation

Comportement recommandé :

- rotation encodeur = change valeur
- poussoir encodeur = parametre global suivant
- appui long sur `MODE` = retour `HOME`

---

## 7. Overlays écran

## 7.1 Regle generale

Toute action importante doit declencher un overlay temporaire.

## 7.2 Contenu minimal

Un overlay contient :

- piste ciblee
- pas cible
- nom du parametre
- valeur

Exemples :

```text
Track A - Step 4
Probability: 70%
```

```text
Track B - Step 8
Ratchet: x3
```

```text
Track A - Step 2
Gate: 60%
```

## 7.3 Timeout

Après une courte inactivité :

- `overlay_active = false`
- retour visuel a `HOME`

---

## 8. Transitions principales

```text
BOOT -> HOME

HOME -> STEP_EDIT
  quand un pas est choisi ou edite

STEP_EDIT -> HOME
  après timeout

HOME -> GLOBAL_EDIT
  MODE long

GLOBAL_EDIT -> HOME
  MODE long

HOME -> DIAGNOSTIC
  SHIFT + RESET

DIAGNOSTIC -> HOME
  SHIFT + RESET

HOME -> HARDWARE_TEST
  SHIFT + PLAY

HARDWARE_TEST -> HOME
  SHIFT + PLAY
```

---

## 9. Décisions implicites pour coder vite

Pour accelerer le développement firmware :

- pas de menu imbrique en V1
- pas de curseur multi-niveaux
- pas de page de configuration lourde
- un seul target encodeur de step à la fois
- un seul target global à la fois

---

## 10. Points a garder stables

Ces règles ne devraient pas changer legerement au fil du dev.

Sinon :

- le firmware se complique
- les tests deviennent flous
- l'instrument perd sa coherence
