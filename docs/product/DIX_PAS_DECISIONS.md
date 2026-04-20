# DIX PAS - Decisions figees

## 1. Objet

Ce document liste les decisions deja prises pour eviter de rouvrir les memes discussions pendant le lancement du projet.

Date de consolidation :

- **2026-04-20**

---

## DEC-001 - Architecture musicale

Decision :

- la machine sera un **2 x 10 pas** avec un mode **Chain 20**

Raison :

- identite forte
- format compact
- plus de valeur qu'un simple 10 pas

Impact :

- deux pistes logiques des la V1

---

## DEC-002 - Interface prioritaire

Decision :

- priorite d'edition = `Probabilite > On/Off > Ratchet`

Raison :

- c'est la logique de jeu la plus pertinente pour l'usage performance defini

Impact :

- la troisieme rangee est pensee d'abord pour la probabilite

---

## DEC-003 - Ratchet V1

Decision :

- le ratchet V1 est limite a `x1, x2, x3`

Raison :

- simplifie fortement le timing
- compatible avec le cadrage initial sans sur-complexifier la V1

Impact :

- pas de `x4` en V1

---

## DEC-004 - Interface MIDI live

Decision :

- l'interface musicale principale est le **MIDI DIN 5 broches**

Raison :

- instrument hardware autonome
- pas de dependance a l'USB dans le flux live

Impact :

- `1 MIDI IN`
- `1 MIDI OUT`

---

## DEC-005 - USB

Decision :

- l'USB n'est pas l'interface musicale principale

Raison :

- separation claire entre developpement et usage live

Impact :

- l'USB ne sert qu'au flash, debug ou service si la carte en possede un

---

## DEC-006 - Gate outputs

Decision :

- la V1 inclut `Gate Out A` et `Gate Out B`

Raison :

- ouvre le projet au hardware analogique sans devoir integrer du CV pitch

Impact :

- 2 sorties numeriques 5 V
- sorties bufferisees
- connectique recommandee en jack mono 3.5 mm

---

## DEC-007 - Comportement Gate

Decision :

- en mode Dual, Gate A suit la piste A et Gate B suit la piste B
- en mode Chain 20, Gate A reste sur les pas 1 a 10 et Gate B sur les pas 11 a 20

Raison :

- comportement previsible
- conservation de l'identite deux rangees / deux pistes

Impact :

- pas de reinterpretation automatique exotique des gates en mode Chain 20

---

## DEC-008 - MCU V1

Decision :

- la carte retenue pour la V1 est l'**Arduino Nano Every**

Raison :

- logique 5 V
- simple pour MIDI DIN
- simple pour Gate Out
- assez de marge pour la V1
- compatible avec la V2 generative visee aujourd'hui

Impact :

- la documentation technique et la BOM partent de cette base

---

## DEC-009 - Ecran

Decision :

- l'ecran retenu est un **OLED I2C 128x64**

Raison :

- assez lisible
- simple a integrer
- suffisant pour le feedback contextuel de la V1

Impact :

- pas d'ecran plus lourd en V1

---

## DEC-010 - Presets

Decision :

- la V1 vise **8 presets minimum**
- la sauvegarde est **explicite**
- le stockage se fait sur **FRAM externe**

Raison :

- fiabilite
- simplicite logicielle
- bonne marge pour la suite

Impact :

- ajout d'une memoire externe des la conception

---

## DEC-011 - LEDs V1

Decision :

- pas de LED sur la troisieme rangee si cela ralentit le projet

Raison :

- il faut proteger la velocite d'execution de la V1

Impact :

- priorite aux LEDs des deux rangees principales et aux LEDs systeme

---

## DEC-012 - Scope V1

Decision :

- Euclidean et modes generatifs sont reportes apres la V1 de base

Raison :

- la V1 doit etre realisable vite
- ces fonctions ne doivent pas retarder le premier instrument jouable

Impact :

- V1 = instrument stable d'abord
- V2 = intelligence musicale ensuite

---

## DEC-013 - Compatibilite V2

Decision :

- le hardware retenu est considere compatible avec la V2 visee aujourd'hui

Raison :

- les fonctions Euclidean et generatives simples ne demandent pas un MCU plus gros a ce stade

Impact :

- on ne change pas de plateforme maintenant

---

## DEC-014 - Strategie de licence

Decision :

- code, firmware, scripts et outillage sous `0BSD`
- documentation, hardware et assets sous `CC0 1.0`

Raison :

- correspond a l'intention "grand ouvert"
- pas d'obligation d'attribution
- reutilisation commerciale libre

Impact :

- `LICENSE` racine en `0BSD`
- `LICENSE-CC0.txt` pour la documentation, le hardware et les assets
- `LICENSING.md` sert de table de repartition

---

## 2. Regle de changement

Une decision de cette liste ne doit etre rouverte que si :

- un blocage reel apparait
- une contrainte hardware ou firmware nouvelle le justifie
- ou un benefice net et important est demontre
