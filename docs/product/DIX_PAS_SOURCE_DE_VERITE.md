# DIX PAS - Source de verite

## 1. Statut du document

Ce document est la **reference canonique** du projet.

En cas de conflit entre plusieurs documents :

- ce document fait foi

Les autres documents servent de details, d'implementation ou de support.

Date de consolidation :

- **2026-04-20**

---

## 2. Vision produit

**DIX PAS** est un **step sequencer MIDI oriente performance live**, compact, direct, et pense comme un vrai instrument hardware.

Objectif :

- programmer vite
- jouer sans menu complexe
- varier en live sans perdre le controle

Le projet doit rester open source et documente proprement pour permettre un demarrage rapide du developpement.

## 2.1 Regime d'ouverture

Le projet adopte une politique de diffusion tres ouverte.

Objectif :

- reutilisation libre
- usage commercial libre
- pas d'obligation d'attribution

Strategie retenue :

- code, firmware, scripts et outillage sous `0BSD`
- documentation, hardware et assets sous `CC0 1.0`

Le detail pratique est documente dans `LICENSING.md`.

---

## 3. Scope V1 fige

La V1 du projet comprend :

- **2 pistes de 10 pas**
- **mode Chain 20**
- **MIDI DIN 5 broches uniquement**
- **1 MIDI IN**
- **1 MIDI OUT**
- **2 Gate Out**
- **1 ecran OLED I2C 128x64**
- **1 encodeur principal avec poussoir**
- **3 rangees de 10 boutons**
- **presets**
- **clock interne et externe**

La V1 doit etre realisable rapidement et proprement, idealement dans un premier sprint tres court.

---

## 4. Comportement musical fige

### 4.1 Modes machine

Deux modes machine sont retenus :

- **Dual**
- **Chain 20**

En mode **Dual** :

- piste A = 10 pas
- piste B = 10 pas
- chaque piste peut utiliser son propre canal MIDI
- Gate A suit la piste A
- Gate B suit la piste B

En mode **Chain 20** :

- les deux pistes forment une sequence unique de 20 pas
- un seul canal MIDI est utilise pour la sequence complete
- Gate A reste lie aux pas 1 a 10
- Gate B reste lie aux pas 11 a 20

### 4.2 Parametres par pas

Chaque pas stocke au minimum :

- active / inactive
- note ou degre
- probabilite
- ratchet
- gate length
- velocity

### 4.3 Priorite d'edition

La priorite d'usage de la machine est :

1. probabilite
2. on / off
3. ratchet
4. le reste a l'encodeur

### 4.4 Ratchet

La V1 supporte :

- `x1`
- `x2`
- `x3`

Le `x4` n'est pas requis en V1.

### 4.5 Modes de lecture

La V1 doit inclure :

- Forward
- Reverse
- Ping-pong
- Random

---

## 5. Interface physique figee

### 5.1 Rangees

Le panneau comprend :

- rangee 1 = piste A
- rangee 2 = piste B
- rangee 3 = controle contextuel

### 5.2 Role de la rangee 3

Par defaut :

- probabilite

Acces secondaire :

- on / off

Acces via shift :

- ratchet

### 5.3 Ecran

L'ecran doit afficher immediatement l'action en cours quand l'utilisateur :

- tourne l'encodeur
- change de mode
- edite un pas
- change la piste cible
- change le tempo ou la gamme

Apres inactivite, l'ecran revient automatiquement a une vue principale simple.

---

## 6. Interfaces externes figees

### 6.1 MIDI

Connectique live officielle :

- `MIDI DIN IN 5 broches`
- `MIDI DIN OUT 5 broches`

Le projet n'utilise pas l'USB MIDI comme interface musicale principale.

Si un port USB existe sur la carte de developpement :

- il est reserve au flash, au debug et au service

### 6.2 Gate

La V1 fournit :

- `Gate Out A`
- `Gate Out B`

Contraintes retenues :

- sorties numeriques
- `5 V`
- `active high`
- sorties bufferisees
- pas de CV pitch en V1

Connectique recommande :

- `2 x jack mono 3.5 mm`

---

## 7. Materiel fige

Les choix hardware valides pour la V1 sont :

- **MCU** : Arduino Nano Every
- **affichage** : OLED I2C 128x64
- **memoire presets** : FRAM I2C externe
- **MIDI** : DIN 5 broches
- **Gate** : 2 sorties 5 V bufferisees

Pourquoi le **Nano Every** est retenu :

- logique 5 V
- simple pour MIDI DIN
- simple pour Gate Out
- assez de marge pour V1
- assez de marge pour la V2 generative envisagee

---

## 8. Stockage et presets

La V1 doit supporter :

- **8 presets minimum**

La sauvegarde retenue est :

- **sauvegarde explicite**

Le stockage utilise :

- **FRAM externe**

---

## 9. Ce qui est explicitement hors scope V1

La V1 n'inclut pas :

- CV pitch
- USB MIDI live
- TRS MIDI
- MIDI THRU obligatoire
- ecran plus complexe qu'un petit OLED texte / graphique simple
- LEDs sur la troisieme rangee si cela ralentit le projet
- moteur Euclidean dans le build week-end
- moteur generatif dans le build week-end

---

## 10. Compatibilite V2

Le choix materiel retenu est considere comme **compatible avec la V2 visee aujourd'hui**.

La V2 ciblee comprend potentiellement :

- Euclidean par piste
- generation melodique selon gamme
- variantes generatives simples
- seed reproductible
- mutation legere par cycle

Ces fonctions restent compatibles avec :

- 2 pistes de 10 pas
- Nano Every
- ecran OLED simple
- FRAM externe

Une re-evaluation materielle ne serait necessaire que si le projet grossit nettement au-dela de cette direction.

---

## 11. Roadmap produit

### 11.1 V1

La V1 doit prioriser :

1. moteur de sequence stable
2. MIDI DIN
3. Gate Out A / B
4. interface live directe
5. presets
6. ecran contextuel

### 11.2 V1.1 / V2

Les evolutions les plus pertinentes sont :

- Euclidean
- generation melodique par gamme
- variations generatives controlees

Ces evolutions sont desirees, mais ne doivent pas ralentir la mise au point de la V1.

---

## 12. Regles de developpement

Pour avancer vite :

- ne pas reouvrir les decisions deja validees sans raison forte
- garder la V1 compacte
- privilegier la lisibilite du jeu live
- ne pas ajouter de complexite d'interface si elle n'apporte pas de benefice scene direct

---

## 13. Definition de reussite V1

La V1 est consideree reussie si :

- la machine sequence correctement
- le MIDI DIN est stable
- les deux Gate Out fonctionnent correctement
- l'interface est jouable
- les presets fonctionnent
- l'ecran donne un feedback immediat

Pour la validation detaillee, voir :

- `docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md`
