# DIX PAS - Source de vérité

## 1. Statut du document

Ce document est la **référence canonique** du projet.

En cas de conflit entre plusieurs documents :

- ce document fait foi

Les autres documents servent de détails, d'implémentation ou de support.

Date de consolidation :

- **2026-04-20**

---

## 2. Vision produit

**DIX PAS** est un **step sequencer MIDI orienté performance live**, compact, direct, et pensé comme un vrai instrument hardware.

Objectif :

- programmer vite
- jouer sans menu complexe
- varier en live sans perdre le contrôle

Le projet doit rester open source et documenté proprement pour permettre un démarrage rapide du développement.

## 2.1 Régime d'ouverture

Le projet adopte une politique de diffusion très ouverte.

Objectif :

- réutilisation libre
- usage commercial libre
- pas d'obligation d'attribution

Stratégie retenue :

- code, firmware, scripts et outillage sous `0BSD`
- documentation, hardware et assets sous `CC0 1.0`

Le détail pratique est documenté dans `LICENSING.md`.

---

## 3. Scope V1 figé

La V1 du projet comprend :

- **2 pistes de 10 pas**
- **mode Chain 20**
- **MIDI DIN 5 broches uniquement**
- **1 MIDI IN**
- **1 MIDI OUT**
- **2 Gate Out**
- **1 écran OLED I2C 128x64 SSD1306**
- **1 encodeur principal avec poussoir**
- **3 rangées de 10 boutons**
- **presets**
- **clock interne et externe**
- **source d'horloge réglable par l'utilisateur**

La V1 doit être réalisable rapidement et proprement, idéalement dans un premier sprint très court.

---

## 4. Comportement musical figé

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

- les deux pistes forment une séquence unique de 20 pas
- un seul canal MIDI est utilisé pour la séquence complète
- Gate A reste lié aux pas 1 à 10
- Gate B reste lié aux pas 11 à 20

### 4.2 Paramètres par pas

Chaque pas stocke au minimum :

- active / inactive
- note ou degré
- probabilité
- ratchet
- gate length
- velocity

Règle de quantification :

- chaque step est toujours quantized sur la gamme globale active
- la V1 ne gère pas de micro-tonalité
- la gamme `Chromatic` permet d'accéder aux `12` demi-tons standards

### 4.3 Priorité d'édition

La priorité d'usage de la machine est :

1. probabilité
2. on / off
3. ratchet
4. le reste à l'encodeur

Mode diagnostic retenu :

- accès via `SHIFT + RESET`
- destiné au bring-up hardware
- n'édite pas la séquence pendant le diagnostic
- affiche l'état `FRAM / OLED`
- affiche le dernier événement `MIDI IN`

Mode test `MIDI / Gate` retenu :

- accès via `SHIFT + PLAY`
- destiné à valider rapidement `MIDI OUT` et `Gate Out A/B`
- `MODE` fait tourner les cibles `Both`, `MIDI`, `Gate`
- `STOP` force les sorties test à l'état bas
- `RESET` relance le cycle de test depuis `A On`
- `SHIFT + PLAY` quitte le mode test

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

### 4.6 Gammes globales V1

La V1 supporte les gammes suivantes :

- Major
- Minor
- Harmonic Minor
- Pentatonic Major
- Pentatonic Minor
- Dorian
- Phrygian
- Lydian
- Mixolydian
- Locrian
- Whole Tone
- Chromatic

Règle retenue :

- pas de doublon de nom pour une même structure de gamme
- `Aeolian` n'est pas exposée comme nom séparé dans la V1
- le nom canonique retenu est `Minor`

---

## 5. Interface physique figée

### 5.1 Rangées

Le panneau comprend :

- rangée 1 = piste A
- rangée 2 = piste B
- rangée 3 = contrôle contextuel

### 5.2 Rôle de la rangée 3

Par défaut :

- probabilité

Accès secondaire :

- on / off

Accès via shift :

- ratchet

### 5.3 Écran

L'écran doit afficher immédiatement l'action en cours quand l'utilisateur :

- tourne l'encodeur
- change de mode
- édite un pas
- change la piste cible
- change le tempo ou la gamme

Après inactivité, l'écran revient automatiquement à une vue principale simple.

Au démarrage, l'écran affiche un court splash :

- `DIX PAS`
- `by Dr. John`
- la version firmware au format `aa.mm.jj.itération`

---

## 6. Interfaces externes figées

### 6.1 MIDI

Connectique live officielle :

- `MIDI DIN IN 5 broches`
- `MIDI DIN OUT 5 broches`

Le projet n'utilise pas l'USB MIDI comme interface musicale principale.

Si un port USB existe sur la carte de développement :

- il est réservé au flash, au debug et au service

### 6.2 Gate

La V1 fournit :

- `Gate Out A`
- `Gate Out B`

Contraintes retenues :

- sorties numériques
- `5 V`
- `active high`
- sorties bufferisées
- pas de CV pitch en V1

Connectique recommandée :

- `2 x jack mono 3.5 mm`

---

## 7. Matériel figé

Les choix hardware valides pour la V1 sont :

- **MCU** : Arduino Nano Every
- **affichage** : OLED I2C 128x64 SSD1306, format `1.3"` validé
- **mémoire presets** : FRAM I2C externe
- **MIDI** : DIN 5 broches
- **Gate** : 2 sorties 5 V bufferisées
- **alimentation** : entrée externe `9 V DC`, `centre positif`, conversion dédiée vers `5 V`

Pourquoi le **Nano Every** est retenu :

- logique 5 V
- simple pour MIDI DIN
- simple pour Gate Out
- assez de marge pour V1
- assez de marge pour la V2 générative envisagée

---

## 8. Stockage et presets

La V1 doit supporter :

- **8 presets minimum**

La sauvegarde retenue est :

- **sauvegarde explicite**

Le stockage utilisé :

- **FRAM externe**

Comportement au démarrage :

- la machine tente de recharger le dernier preset connu
- priorité au dernier preset chargé
- sinon dernier preset sauvegardé
- sinon projet par défaut de secours
- si un preset est invalide, la machine démarre quand même sur le projet de secours

---

## 9. Ce qui est explicitement hors scope V1

La V1 n'inclut pas :

- CV pitch
- USB MIDI live
- TRS MIDI
- MIDI THRU obligatoire
- écran plus complexe qu'un petit OLED texte / graphique simple
- LEDs sur la troisième rangée si cela ralentit le projet
- moteur Euclidean dans le build week-end
- moteur génératif dans le build week-end

Nuance de développement validée :

- le firmware peut préparer et exposer ces fonctions en mode expérimental
- elles ne sont pas requises pour considérer la V1 hardware comme réussie
- elles ne doivent pas ralentir le bring-up matériel ni compliquer l'interface de base

---

## 10. Compatibilité V2

Le choix matériel retenu est considéré comme **compatible avec la V2 visée aujourd'hui**.

La V2 ciblée comprend potentiellement :

- Euclidean par piste
- génération mélodique selon gamme
- variantes génératives simples
- seed reproductible
- mutation légère par cycle

Ces fonctions restent compatibles avec :

- 2 pistes de 10 pas
- Nano Every
- écran OLED simple
- FRAM externe

Une re-evaluation matérielle ne serait nécessaire que si le projet grossit nettement au-delà de cette direction.

---

## 11. Roadmap produit

### 11.1 V1

La V1 doit prioriser :

1. moteur de séquence stable
2. MIDI DIN
3. Gate Out A / B
4. interface live directe
5. presets
6. écran contextuel

### 11.2 V1.1 / V2

Les évolutions les plus pertinentes sont :

- Euclidean
- génération mélodique par gamme
- variations génératives contrôlées

État logiciel actuel accepté :

- le firmware peut déjà proposer des slots génératifs internes
- ces slots peuvent être appliqués ou mutés depuis l'édition globale
- cette avance logicielle est autorisée tant qu'elle reste modulaire

Ces évolutions sont désirées, mais ne doivent pas ralentir la mise au point de la V1.

---

## 12. Règles de développement

Pour avancer vite :

- ne pas reouvrir les décisions déjà validées sans raison forte
- garder la V1 compacte
- privilégier la lisibilité du jeu live
- ne pas ajouter de complexité d'interface si elle n'apporte pas de bénéfice scène direct

---

## 13. Définition de réussite V1

La V1 est considérée réussie si :

- la machine séquence correctement
- le MIDI DIN est stable
- les deux Gate Out fonctionnent correctement
- l'interface est jouable
- les presets fonctionnent
- l'écran donne un feedback immédiat

Pour la validation détaillée, voir :

- `docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md`
