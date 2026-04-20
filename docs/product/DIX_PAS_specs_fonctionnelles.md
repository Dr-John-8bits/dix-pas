# DIX PAS - Specifications fonctionnelles

Note documentaire :

- ce document détaille la spec fonctionnelle
- en cas de conflit avec un autre document, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte

## 1. Positionnement du produit

**DIX PAS** est un **step sequencer MIDI orienté performance live**, pensé comme un vrai instrument et non comme un prototype.

Le produit doit être :

- compact, robuste et lisible sur scène
- immédiat à jouer sans navigation complexe
- expressif malgré une interface réduite
- synchronisable avec un setup MIDI externe
- centré sur une connectique **MIDI DIN 5 broches**
- publiable en open source sur GitHub

La version cible décrite ici repose sur une architecture **2 x 10 pas** avec un mode chaîné **20 pas**.

---

## 2. Concept global

Le séquenceur comporte :

- **2 pistes de 10 pas**
- **1 rangée de contrôle de 10 boutons**
- **1 encodeur principal**
- **1 écran**
- **1 MIDI IN DIN 5 broches**
- **1 MIDI OUT DIN 5 broches**
- **2 sorties Gate numériques**
- **boutons de transport et de mode**

Le comportement musical principal est le suivant :

- en mode **Dual**, chaque piste fonctionne comme un séquenceur de 10 pas indépendant
- en mode **Chain 20**, les deux pistes sont fusionnées en une séquence unique de 20 pas
- en mode **Dual**, chaque piste peut utiliser son propre canal MIDI
- en mode **Chain 20**, la séquence complète utilisé un seul canal MIDI

---

## 3. Modes de fonctionnement

### 3.1 Mode Dual

Le mode Dual est le mode principal de performance.

Caractéristiques :

- piste A = 10 pas
- piste B = 10 pas
- chaque piste a son propre canal MIDI
- chaque piste possède sa propre séquence, ses paramètres de pas et sa longueur active
- les deux pistes partagent le même tempo et la même clock

Usage visé :

- piloter deux synthés différents
- piloter deux timbres sur deux canaux MIDI
- jouer une ligne principale et une contre-ligne

### 3.2 Mode Chain 20

Le mode Chain 20 fusionne les deux pistes en une séquence unique.

Caractéristiques :

- pas 1 a 10 = ancienne piste A
- pas 11 a 20 = ancienne piste B
- un seul canal MIDI pour l'ensemble
- une seule position de lecture
- la rangée de contrôle agit sur la moitié actuellement sélectionnée

Usage visé :

- lignes mélodiques plus longues
- motifs évolutifs
- variations moins cycliques que sur 10 pas

---

## 4. Donnees musicales

### 4.1 Structure d'une piste

Chaque piste contient :

- 10 pas maximum
- une longueur active variable de 1 a 10 pas
- un canal MIDI assignable
- un décalage d'octave global
- une racine harmonique globale
- une gamme globale

### 4.2 Structure d'un pas

Chaque pas doit pouvoir stocker au minimum :

- état actif ou inactif
- note ou degré dans la gamme
- probabilité
- ratchet
- gate length
- accent ou velocity

### 4.3 Gamme globale

Le sequencer doit permettre un fonctionnement musical simple par gamme.

Fonctions attendues :

- choix de la tonique
- choix de la gamme
- édition des notes par degré plutot que par note brute si souhaite
- transposition globale

Le système doit rester assez souple pour permettre plus tard un mode chromatique complet.

---

## 5. Paramètres de pas

### 5.1 Probabilite

La probabilité est le parametre de performance prioritaire.

Exigences :

- accès direct depuis la rangée de contrôle
- édition rapide pas par pas
- valeur de 0 a 100 %
- si le pas ne passe pas, aucune note n'est envoyee

### 5.2 On / Off

L'activation de pas est le second parametre prioritaire.

Exigences :

- édition rapide depuis la rangée de contrôle
- lecture visuelle immédiate de l'état actif ou inactif
- un pas inactif ne joue aucune note, quelle que soit sa probabilité

### 5.3 Ratchet

Le ratchet est le troisième parametre prioritaire.

Exigences :

- accès rapide via combinaison de mode
- valeurs prises en charge en V1 : **x1, x2, x3**
- le comportement doit rester temporellement stable et musical

Le ratchet **x4** n'est pas requis en V1.

### 5.4 Gate Length

Le gate length doit permettre de sortir d'un jeu trop sec.

Exigences :

- reglage par pas
- édition à l'encodeur
- valeurs musicales simples plutot qu'une precision inutile

### 5.5 Accent / Velocity

Chaque pas doit pouvoir être accentue.

Exigences :

- valeur de velocity par pas
- édition fine à l'encodeur
- lecture claire à l'écran

---

## 6. Lecture de séquence

### 6.1 Modes de lecture

Les modes de lecture globaux doivent inclure :

- Forward
- Reverse
- Ping-pong
- Random

### 6.2 Longueur de séquence

Chaque piste doit pouvoir avoir une longueur active inferieure a 10 pas.

Objectifs :

- polymetries simples en mode Dual
- raccourcissement rapide de motif
- experimentation live

### 6.3 Reset et position

Le transport doit permettre :

- play
- stop
- reset position
- redémarrage propre a partir du premier pas

---

## 7. Clock et synchronisation

### 7.1 Modes de clock

Le produit doit proposer :

- clock interne
- clock MIDI externe

### 7.2 Comportement de synchronisation

En clock interne :

- le sequencer genere son tempo
- il peut emettre la clock MIDI

En clock externe :

- le sequencer suit la machine maitre
- il suit au minimum MIDI Clock, Start et Stop

### 7.3 Resolution temporelle

Le système doit être compatible avec la clock MIDI standard a **24 PPQN**, mais la planification interne des evenements doit utiliser une resolution plus fine que le simple pas MIDI brut.

Objectif :

- garantir un ratchet stable
- gérer correctement gate et retriggers
- conserver une sensation propre en live

---

## 8. Interface physique

### 8.1 Rangée 1

La première rangée correspond a la **piste A**.

Fonctions :

- selection de pas 1 a 10
- visualisation de la position de lecture
- indication de l'état du pas

### 8.2 Rangée 2

La deuxième rangée correspond a la **piste B**.

Fonctions :

- selection de pas 1 a 10
- visualisation de la position de lecture
- indication de l'état du pas

### 8.3 Rangée 3

La troisième rangée est la **rangée de contrôle contextuelle**.

Elle agit sur la piste actuellement sélectionnée.

Priorites d'usage :

- probabilité
- on/off
- ratchet

Comportement recommandé :

- mode par défaut de la rangée 3 = **Probabilite**
- bouton de mode court = **On / Off**
- `SHIFT` maintenu = **Ratchet**

La piste actuellement ciblee doit être clairement visible.

### 8.4 Encodeur principal

L'encodeur sert a l'édition fine des paramètres.

Usages cibles :

- note ou degré
- gate
- accent / velocity
- longueur de piste
- tempo
- tonique et gamme
- paramètres globaux

### 8.5 Boutons système

Les commandes système minimales sont :

- Play
- Stop
- Reset
- Shift
- Mode

La V1 ne prévoit pas de bouton `Select` dédié.

La sélection de piste ou de cible d'édition se fait via :

- les boutons de pas
- la rangée de contrôle
- l'encodeur et son poussoir

---

## 9. Affichage écran

L'écran est un element central de lisibilite.

### 9.1 Feedback contextuel instantane

A chaque action utilisateur, l'écran doit afficher immédiatement ce qui est en cours d'édition.

Cela inclut :

- rotation de l'encodeur
- changement de mode
- édition d'un pas
- changement de piste cible
- modification de tempo ou de gamme

Exemples d'affichage :

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

### 9.2 Retour automatique

Après une courte periode d'inactivité, l'écran doit revenir a la vue principale.

### 9.3 Vue principale

La vue principale doit afficher au minimum :

- mode en cours
- état lecture ou arret
- tempo ou statut de synchro externe
- piste ciblee
- pas courant

---

## 10. MIDI et sorties de contrôle

### 10.1 Sorties MIDI

Le produit doit emettre des notes MIDI.

Connectique requise en V1 :

- 1 x **MIDI DIN IN 5 broches**
- 1 x **MIDI DIN OUT 5 broches**

Le **MIDI THRU** n'est pas requis en V1.

L'usage live doit reposer sur le **MIDI DIN standard**.

Si un port USB existe sur la plateforme de développement retenue, il est considéré comme un port de **flash / debug / service**, et non comme une interface musicale principale.

### 10.2 Canaux MIDI

En mode Dual :

- piste A = canal MIDI assignable
- piste B = canal MIDI assignable

En mode Chain 20 :

- un seul canal MIDI assignable pour l'ensemble

### 10.3 Messages pris en charge

Le minimum fonctionnel attendu est :

- Note On
- Note Off
- Clock
- Start
- Stop

### 10.4 Sorties Gate

La V1 doit proposer :

- **Gate Out A**
- **Gate Out B**

Les sorties Gate sont des sorties **numériques**, pas des sorties CV de pitch.

Exigences minimales :

- niveau logique cible : **5 V**
- comportement stable et immédiat
- sorties protégées vis-a-vis du monde exterieur

Comportement recommandé :

- en mode **Dual**, `Gate Out A` suit la piste A et `Gate Out B` suit la piste B
- en mode **Chain 20**, `Gate Out A` reste lié aux pas 1 a 10 et `Gate Out B` reste lié aux pas 11 a 20

La V1 n'inclut pas de sortie CV de note.

---

## 11. Fonctions de performance

### 11.1 Randomize

Le sequencer doit inclure une fonction de randomisation musicale.

Usages minimaux :

- randomize des notes
- randomize de la probabilité
- randomize piste A, piste B ou les deux

La randomisation doit respecter la gamme si le mode gamme est actif.

### 11.2 Undo de securite

Toute fonction de randomisation ou modification globale importante devrait pouvoir être annulee au moins une fois.

### 11.3 Rotation / Nudge

Une fonction de rotation de séquence est fortement recommandée.

Usages vises :

- decaler un motif sans reprogrammer
- creer des variations immédiates en live

---

## 12. Presets et stockage

Le produit doit stocker plusieurs états complets.

Exigences minimales :

- plus de 3 presets
- objectif recommandé : **8 a 16 presets**

Chaque preset doit stocker :

- contenu des deux pistes
- longueurs actives
- canaux MIDI
- gamme et tonique
- tempo
- mode de lecture
- mode Dual ou Chain 20

---

## 13. Exigences de conception

Le produit ne doit pas ressembler a un prototype de laboratoire.

Il doit viser :

- une implantation propre du panneau
- une gestuelle jouable en condition live
- des composants standards et faciles a sourcer
- une architecture claire pour documentation open source

Le firmware doit être structure en modules distincts :

- Clock Engine
- Sequencer Engine
- MIDI Engine
- UI Engine
- Display Engine
- Storage Engine

---

## 14. Priorites pour la V1

Les priorités de la V1 sont :

1. moteur de séquence stable
2. mode Dual 2 x 10 pas
3. mode Chain 20
4. rangée 3 contextuelle avec probabilité en accès direct
5. feedback écran instantane
6. MIDI clock interne et externe
7. Gate Out A / Gate Out B
8. presets

Les fonctions suivantes peuvent être considérées comme secondaires si besoin :

- randomization avancee
- édition très fine de velocity
- modes harmoniques complexes
- ratchet superieur a x3

---

## 15. Recommandation de direction produit

La direction recommandé pour **DIX PAS** est la suivante :

- assumer une machine de performance a **2 pistes de 10 pas**
- garder la **probabilité** comme fonction la plus immédiate
- traiter **on/off** comme seconde couche rapide
- limiter le **ratchet a x3** en V1
- faire de l'écran un vrai outil de feedback, pas un simple accessoire

Cette direction garde une vraie personnalité, apporte plus de valeur qu'un simple 10 pas monophonique, et reste suffisamment compacte pour un projet matériel open source réaliste.

---

## 16. Fonctions futures ciblees

Les évolutions futures les plus pertinentes pour **DIX PAS** sont :

- **Euclidean par piste**
- **generation melodique par gamme**
- **variations génératives contrôlees**

### 16.1 Euclidean

Fonctions cibles :

- densite
- longueur
- rotation
- application sur piste A, piste B ou les deux

### 16.2 Generation melodique

Fonctions cibles :

- generation de notes dans une gamme
- plage d'octave definissable
- comportement type random walk, montee, descente ou alternance
- regeneration rapide d'une variante

### 16.3 Variations génératives

Fonctions cibles :

- seed ou état reproductible
- mutation legere a chaque cycle
- dosage de l'instabilite
- combinaison rythme Euclidean + notes generees

Ces fonctions ne sont pas requises pour la V1 week-end, mais elles constituent la direction naturelle d'une V2 ou V1.1.
