# DIX PAS - UI State Machine

Note documentaire :

- ce document fige le comportement de l'interface utilisateur pour accelerer le firmware
- en cas de conflit de scope produit, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte

## 1. Objet

Ce document decrit les etats et transitions principaux de l'interface.

Le but est d'eviter une UI floue au moment de coder.

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
- faire de l'ecran un feedback contextuel, pas un menu permanent
- reduire le nombre de vrais etats UI
- preferer des overlays temporaires a des sous-menus

---

## 3. Variables d'etat UI

Variables minimales a maintenir :

- `machine_mode` : `DUAL` ou `CHAIN20`
- `transport_state` : `STOPPED` ou `PLAYING`
- `track_focus` : `A` ou `B`
- `selected_step_a` : `0..9`
- `selected_step_b` : `0..9`
- `row3_base_mode` : `PROB` ou `ONOFF`
- `shift_held` : `true / false`
- `encoder_target` : `NOTE`, `GATE`, `VELOCITY`
- `global_target` : `TEMPO`, `ROOT`, `SCALE`, `PLAYMODE`, `TRACK_LENGTH`, `MIDI_CHANNEL`, `PRESET_SLOT`
- `ui_page` : `HOME`, `STEP_EDIT`, `GLOBAL_EDIT`
- `overlay_active` : `true / false`

---

## 4. Etats principaux

### 4.1 BOOT

Description :

- etat de demarrage

Actions :

- initialiser l'etat machine
- charger les reglages par defaut ou le dernier contexte utile
- afficher un ecran de boot court

Transition de sortie :

- `BOOT -> HOME`

### 4.2 HOME

Description :

- etat normal de la machine
- visualisation du contexte courant

Affichage minimal :

- mode machine
- tempo ou sync externe
- piste ciblee
- pas courant
- etat transport

Actions possibles :

- selection de pas
- edition rapide via rangee 3
- edition via encodeur
- transport
- entree en edition globale

### 4.3 STEP_EDIT

Description :

- ce n'est pas une page lourde
- c'est un etat logique de travail sur un pas

On y est quand :

- un pas est selectionne
- l'utilisateur tourne l'encodeur
- l'utilisateur agit sur la rangee 3

Affichage :

- overlay temporaire avec piste, pas, parametre et valeur

Sortie :

- retour automatique vers `HOME` apres timeout d'inactivite

### 4.4 GLOBAL_EDIT

Description :

- edition des parametres machine ou piste non lies a un bouton de pas direct

Contenu cible :

- tempo
- root
- scale
- play mode
- longueur de piste
- canal MIDI
- preset slot

Affichage :

- overlay ou page legere

Sortie :

- retour vers `HOME` par action utilisateur ou timeout selon le parametre

---

## 5. Comportement des controles

## 5.1 Rangee 1

Role :

- selection des pas de la piste A

Effets :

- `track_focus = A`
- `selected_step_a = bouton_presse`
- activation de l'overlay de step

En mode `CHAIN20` :

- la rangee 1 represente les pas `1..10`

## 5.2 Rangee 2

Role :

- selection des pas de la piste B

Effets :

- `track_focus = B`
- `selected_step_b = bouton_presse`
- activation de l'overlay de step

En mode `CHAIN20` :

- la rangee 2 represente les pas `11..20`

## 5.3 Rangee 3

Role :

- edition rapide contextuelle

Comportement :

- si `shift_held = false`, la rangee agit selon `row3_base_mode`
- si `shift_held = true`, la rangee agit en mode `RATCHET`

Modes retenus :

- `PROB`
- `ONOFF`
- `RATCHET` temporaire via `SHIFT`

## 5.4 Encodeur

Role :

- edition fine du parametre courant

En `STEP_EDIT` :

- edite `encoder_target`

En `GLOBAL_EDIT` :

- edite `global_target`

## 5.5 Poussoir encodeur

Role retenu :

- cycle le `encoder_target` dans `STEP_EDIT`

Cycle recommande :

- `NOTE -> GATE -> VELOCITY -> NOTE`

## 5.6 Bouton MODE

Appui court :

- alterne `row3_base_mode`

Cycle recommande :

- `PROB <-> ONOFF`

Appui long :

- entre ou sort de `GLOBAL_EDIT`

## 5.7 Bouton SHIFT

Role :

- modificateur momentane

Effets :

- si maintenu, la rangee 3 passe temporairement en `RATCHET`

## 5.8 Bouton PLAY

Effet :

- passe `transport_state` a `PLAYING`

## 5.9 Bouton STOP

Effet :

- passe `transport_state` a `STOPPED`
- force `Note Off`
- force `Gate Out A / B` a l'etat bas

## 5.10 Bouton RESET

Effet :

- remet la position de lecture au debut
- force les Gates a l'etat bas

---

## 6. Global edit

## 6.1 Entree

`HOME -> GLOBAL_EDIT`

Condition :

- appui long sur `MODE`

## 6.2 Parametres globaux

Ordre recommande de cycle :

1. `TEMPO`
2. `ROOT`
3. `SCALE`
4. `PLAYMODE`
5. `TRACK_LENGTH`
6. `MIDI_CHANNEL`
7. `PRESET_SLOT`

## 6.3 Navigation

Comportement recommande :

- rotation encodeur = change valeur
- poussoir encodeur = parametre global suivant
- appui long sur `MODE` = retour `HOME`

---

## 7. Overlays ecran

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

Apres une courte inactivite :

- `overlay_active = false`
- retour visuel a `HOME`

---

## 8. Transitions principales

```text
BOOT -> HOME

HOME -> STEP_EDIT
  quand un pas est choisi ou edite

STEP_EDIT -> HOME
  apres timeout

HOME -> GLOBAL_EDIT
  MODE long

GLOBAL_EDIT -> HOME
  MODE long
```

---

## 9. Decisions implicites pour coder vite

Pour accelerer le developpement firmware :

- pas de menu imbrique en V1
- pas de curseur multi-niveaux
- pas de page de configuration lourde
- un seul target encodeur de step a la fois
- un seul target global a la fois

---

## 10. Points a garder stables

Ces regles ne devraient pas changer legerement au fil du dev.

Sinon :

- le firmware se complique
- les tests deviennent flous
- l'instrument perd sa coherence
