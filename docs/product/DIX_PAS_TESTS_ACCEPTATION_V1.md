# DIX PAS - Tests d'acceptation V1

## 1. Objet

Ce document definit les tests minimaux a passer pour considerer la V1 comme exploitable.

Le but n'est pas de couvrir tous les cas imaginables.

Le but est de verifier que l'instrument est :

- stable
- jouable
- cohérent avec la vision produit

---

## 2. Conditions de test

Base de test recommandée :

- alimentation externe `9 V DC` via la chaîne finale de conversion `5 V`
- écran branche
- boutons et encodeur branches
- MIDI DIN IN et OUT branches sur un setup de test
- Gate Out A et Gate Out B branches sur deux cibles de test visibles

---

## 3. Tests de démarrage

### T-001 - Boot propre

Critere :

- la machine demarre sans freeze
- le splash `DIX PAS / by Dr. John / version` apparait
- l'écran affiche un état principal cohérent

### T-002 - Etat initial sain

Critere :

- aucune note ne reste accrochee au boot
- Gate A et Gate B sont bas au repos

### T-003 - Navigation de base

Critere :

- l'encodeur reagit
- les boutons reagissent
- aucun comportement erratique immédiat

### T-004 - Mode diagnostic

Critere :

- `SHIFT + RESET` ouvre bien le diagnostic
- l'état `FRAM / OLED` est visible
- le dernier événement `MIDI IN` est visible

### T-005 - Mode test MIDI / Gate

Critere :

- `SHIFT + PLAY` ouvre bien le mode test
- `MODE` fait tourner `Both`, `MIDI`, `Gate`
- `STOP` force les sorties test à l'état bas
- `RESET` relance le cycle depuis `A On`

---

## 4. Tests de séquence

### T-010 - Lecture simple piste A

Critere :

- une séquence simple sur piste A joue correctement
- la position de lecture est visible

### T-011 - Lecture simple piste B

Critere :

- une séquence simple sur piste B joue correctement

### T-012 - Mode Dual

Critere :

- piste A et piste B peuvent jouer en parallele
- chaque piste suit ses propres pas

### T-013 - Mode Chain 20

Critere :

- les pas 1 a 20 sont lus dans le bon ordre
- la transition entre pas 10 et pas 11 est propre

### T-013b - Sélection du mode machine

Critere :

- le mode `Dual / Chain20` est réglable depuis `Global Edit`
- l'écran reflète correctement le mode actif

### T-014 - Longueur de piste

Critere :

- une piste peut être raccourcie
- la lecture respecte la longueur definie

### T-015 - Modes de lecture

Critere :

- Forward, Reverse, Ping-pong et Random fonctionnent

---

## 5. Tests d'édition

### T-020 - Selection de pas

Critere :

- un pas peut être selectionne clairement sur chaque piste

### T-021 - Probabilite

Critere :

- la troisième rangée permet d'editer la probabilité
- le comportement musical change effectivement

### T-022 - On / Off

Critere :

- un pas peut être active ou desactive
- un pas inactif ne joue pas

### T-023 - Ratchet

Critere :

- `x1`, `x2`, `x3` sont disponibles
- le timing reste propre

### T-024 - Gate length

Critere :

- la valeur de gate d'un pas peut être modifiee
- la duree entendue ou observee change réellement

### T-025 - Velocity

Critere :

- la velocity par pas change bien la valeur envoyee en MIDI

---

## 6. Tests écran

### T-030 - Feedback encodeur

Critere :

- a chaque rotation de l'encodeur, l'écran montre le parametre et la valeur éditées

### T-031 - Feedback changement de mode

Critere :

- l'écran reflète le mode d'édition actif

### T-032 - Retour automatique

Critere :

- après inactivité, l'écran revient a la vue principale

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

- la séquence chainee utilisé bien un seul canal

### T-043 - Clock interne

Critere :

- la machine peut jouer sur sa propre clock
- le tempo est stable

### T-043b - Sélection de source d'horloge

Critere :

- la source `Internal / External` est réglable depuis `Global Edit`
- l'écran reflète correctement la source active

### T-044 - Clock externe

Critere :

- la machine suit MIDI Clock externe
- Start et Stop externes sont bien pris en compte
- le diagnostic montre bien `In:Clock`, `In:Start`, `In:Stop`

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

- Gate A reste lié aux pas 1 a 10
- Gate B reste lié aux pas 11 a 20

### T-053 - Niveau logique Gate

Critere :

- les sorties Gate atteignent le niveau logique attendu
- les sorties retombent bien a l'état bas

### T-054 - Stop / Reset Gate

Critere :

- les deux sorties Gate retombent a l'état bas sur stop et reset

---

## 9. Tests presets

### T-060 - Sauvegarde preset

Critere :

- un preset peut être sauvegarde sans erreur

### T-061 - Rappel preset

Critere :

- un preset rappelle correctement séquence, tempo, canaux et état machine

### T-062 - Persistance

Critere :

- après extinction puis rallumage, les presets sauvegardes sont toujours presents

---

## 10. Definition de validation

La V1 est validée si :

- tous les tests critiques de `T-001` a `T-054` passent
- les presets fonctionnent au minimum sur sauvegarde / rappel / persistance
- la machine est jouable sans bug bloquant evident

Les fonctions Euclidean et génératives ne sont pas necessaires pour declarer la V1 validée.
