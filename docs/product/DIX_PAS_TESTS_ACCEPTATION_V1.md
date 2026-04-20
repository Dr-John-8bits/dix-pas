# DIX PAS - Tests d'acceptation V1

## 1. Objet

Ce document definit les tests minimaux a passer pour considerer la V1 comme exploitable.

Le but n'est pas de couvrir tous les cas imaginables.

Le but est de verifier que l'instrument est :

- stable
- jouable
- coherent avec la vision produit

---

## 2. Conditions de test

Base de test recommandee :

- alimentation nominale stable
- ecran branche
- boutons et encodeur branches
- MIDI DIN IN et OUT branches sur un setup de test
- Gate Out A et Gate Out B branches sur deux cibles de test visibles

---

## 3. Tests de demarrage

### T-001 - Boot propre

Critere :

- la machine demarre sans freeze
- l'ecran affiche un etat principal coherent

### T-002 - Etat initial sain

Critere :

- aucune note ne reste accrochee au boot
- Gate A et Gate B sont bas au repos

### T-003 - Navigation de base

Critere :

- l'encodeur reagit
- les boutons reagissent
- aucun comportement erratique immediat

---

## 4. Tests de sequence

### T-010 - Lecture simple piste A

Critere :

- une sequence simple sur piste A joue correctement
- la position de lecture est visible

### T-011 - Lecture simple piste B

Critere :

- une sequence simple sur piste B joue correctement

### T-012 - Mode Dual

Critere :

- piste A et piste B peuvent jouer en parallele
- chaque piste suit ses propres pas

### T-013 - Mode Chain 20

Critere :

- les pas 1 a 20 sont lus dans le bon ordre
- la transition entre pas 10 et pas 11 est propre

### T-014 - Longueur de piste

Critere :

- une piste peut etre raccourcie
- la lecture respecte la longueur definie

### T-015 - Modes de lecture

Critere :

- Forward, Reverse, Ping-pong et Random fonctionnent

---

## 5. Tests d'edition

### T-020 - Selection de pas

Critere :

- un pas peut etre selectionne clairement sur chaque piste

### T-021 - Probabilite

Critere :

- la troisieme rangee permet d'editer la probabilite
- le comportement musical change effectivement

### T-022 - On / Off

Critere :

- un pas peut etre active ou desactive
- un pas inactif ne joue pas

### T-023 - Ratchet

Critere :

- `x1`, `x2`, `x3` sont disponibles
- le timing reste propre

### T-024 - Gate length

Critere :

- la valeur de gate d'un pas peut etre modifiee
- la duree entendue ou observee change reellement

### T-025 - Velocity

Critere :

- la velocity par pas change bien la valeur envoyee en MIDI

---

## 6. Tests ecran

### T-030 - Feedback encodeur

Critere :

- a chaque rotation de l'encodeur, l'ecran montre le parametre et la valeur edites

### T-031 - Feedback changement de mode

Critere :

- l'ecran reflete le mode d'edition actif

### T-032 - Retour automatique

Critere :

- apres inactivite, l'ecran revient a la vue principale

### T-033 - Vue principale

Critere :

- la vue principale affiche au minimum tempo ou sync, mode, piste ciblee et pas courant

---

## 7. Tests MIDI

### T-040 - MIDI OUT notes

Critere :

- Note On et Note Off sont emis correctement

### T-041 - Canaux MIDI en mode Dual

Critere :

- piste A et piste B peuvent utiliser deux canaux differents

### T-042 - Canal MIDI en mode Chain 20

Critere :

- la sequence chainee utilise bien un seul canal

### T-043 - Clock interne

Critere :

- la machine peut jouer sur sa propre clock
- le tempo est stable

### T-044 - Clock externe

Critere :

- la machine suit MIDI Clock externe
- Start et Stop externes sont bien pris en compte

### T-045 - Arret propre

Critere :

- sur stop, aucune note ne reste bloquee

---

## 8. Tests Gate

### T-050 - Gate Out A en mode Dual

Critere :

- Gate A suit la piste A

### T-051 - Gate Out B en mode Dual

Critere :

- Gate B suit la piste B

### T-052 - Gate en mode Chain 20

Critere :

- Gate A reste lie aux pas 1 a 10
- Gate B reste lie aux pas 11 a 20

### T-053 - Niveau logique Gate

Critere :

- les sorties Gate atteignent le niveau logique attendu
- les sorties retombent bien a l'etat bas

### T-054 - Stop / Reset Gate

Critere :

- les deux sorties Gate retombent a l'etat bas sur stop et reset

---

## 9. Tests presets

### T-060 - Sauvegarde preset

Critere :

- un preset peut etre sauvegarde sans erreur

### T-061 - Rappel preset

Critere :

- un preset rappelle correctement sequence, tempo, canaux et etat machine

### T-062 - Persistance

Critere :

- apres extinction puis rallumage, les presets sauvegardes sont toujours presents

---

## 10. Definition de validation

La V1 est validee si :

- tous les tests critiques de `T-001` a `T-054` passent
- les presets fonctionnent au minimum sur sauvegarde / rappel / persistance
- la machine est jouable sans bug bloquant evident

Les fonctions Euclidean et generatives ne sont pas necessaires pour declarer la V1 validee.
