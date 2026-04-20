# DIX PAS - Décisions figées

## 1. Objet

Ce document liste les décisions déjà prises pour éviter de rouvrir les mêmes discussions pendant le lancement du projet.

Date de consolidation :

- **2026-04-20**

---

## DEC-001 - Architecture musicale

Décision :

- la machine sera un **2 x 10 pas** avec un mode **Chain 20**

Raison :

- identité forte
- format compact
- plus de valeur qu'un simple 10 pas

Impact :

- deux pistes logiques dès la V1

---

## DEC-002 - Interface prioritaire

Décision :

- priorité d'édition = `Probabilite > On/Off > Ratchet`

Raison :

- c'est la logique de jeu la plus pertinente pour l'usage performance défini

Impact :

- la troisième rangée est pensée d'abord pour la probabilité

---

## DEC-003 - Ratchet V1

Décision :

- le ratchet V1 est limité à `x1, x2, x3`

Raison :

- simplifie fortement le timing
- compatible avec le cadrage initial sans sur-complexifier la V1

Impact :

- pas de `x4` en V1

---

## DEC-004 - Interface MIDI live

Décision :

- l'interface musicale principale est le **MIDI DIN 5 broches**

Raison :

- instrument hardware autonome
- pas de dépendance à l'USB dans le flux live

Impact :

- `1 MIDI IN`
- `1 MIDI OUT`

---

## DEC-005 - USB

Décision :

- l'USB n'est pas l'interface musicale principale

Raison :

- séparation claire entre développement et usage live

Impact :

- l'USB ne sert qu'au flash, debug ou service si la carte en possède un

---

## DEC-006 - Gate outputs

Décision :

- la V1 inclut `Gate Out A` et `Gate Out B`

Raison :

- ouvre le projet au hardware analogique sans devoir intégrer du CV pitch

Impact :

- 2 sorties numériques 5 V
- sorties bufferisées
- connectique recommandée en jack mono 3.5 mm

---

## DEC-007 - Comportement Gate

Décision :

- en mode Dual, Gate A suit la piste A et Gate B suit la piste B
- en mode Chain 20, Gate A reste sur les pas 1 à 10 et Gate B sur les pas 11 à 20

Raison :

- comportement prévisible
- conservation de l'identité deux rangées / deux pistes

Impact :

- pas de réinterprétation automatique exotique des gates en mode Chain 20

---

## DEC-008 - MCU V1

Décision :

- la carte retenue pour la V1 est l'**Arduino Nano Every**

Raison :

- logique 5 V
- simple pour MIDI DIN
- simple pour Gate Out
- assez de marge pour la V1
- compatible avec la V2 générative visée aujourd'hui

Impact :

- la documentation technique et la BOM partent de cette base

---

## DEC-009 - Écran

Décision :

- l'écran retenu est un **OLED I2C 128x64 SSD1306**
- référence validée pour la V1 : **Adafruit Product 938**

Raison :

- assez lisible
- simple à intégrer
- suffisant pour le feedback contextuel de la V1
- format `1.3"` plus confortable pour un instrument que les plus petits modules

Impact :

- pas d'écran plus lourd en V1

---

## DEC-010 - Presets

Décision :

- la V1 vise **8 presets minimum**
- la sauvegarde est **explicite**
- le stockage se fait sur **FRAM externe**

Raison :

- fiabilité
- simplicité logicielle
- bonne marge pour la suite

Impact :

- ajout d'une mémoire externe dès la conception

---

## DEC-011 - LEDs V1

Décision :

- pas de LED sur la troisième rangée si cela ralentit le projet

Raison :

- il faut protéger la vélocité d'exécution de la V1

Impact :

- priorité aux LEDs des deux rangées principales et aux LEDs système

---

## DEC-012 - Scope V1

Décision :

- Euclidean et modes génératifs sont reportés après la V1 de base

Raison :

- la V1 doit être réalisable vite
- ces fonctions ne doivent pas retarder le premier instrument jouable

Impact :

- V1 = instrument stable d'abord
- V2 = intelligence musicale ensuite

---

## DEC-013 - Compatibilité V2

Décision :

- le hardware retenu est considéré compatible avec la V2 visée aujourd'hui

Raison :

- les fonctions Euclidean et génératives simples ne demandent pas un MCU plus gros à ce stade

Impact :

- on ne change pas de plateforme maintenant

---

## DEC-014 - Stratégie de licence

Décision :

- code, firmware, scripts et outillage sous `0BSD`
- documentation, hardware et assets sous `CC0 1.0`

Raison :

- correspond à l'intention "grand ouvert"
- pas d'obligation d'attribution
- réutilisation commerciale libre

Impact :

- `LICENSE` racine en `0BSD`
- `LICENSE-CC0.txt` pour la documentation, le hardware et les assets

---

## DEC-015 - Alimentation V1

Décision :

- la V1 sera alimentée par une **alimentation externe dédiée**
- standard retenu : **9 V DC**, **centre positif**, **barrel 2.1 mm**
- conversion système via un **buck 5 V dédié**
- l'USB reste réservé au flash et au debug

Raison :

- architecture plus saine pour un instrument hardware
- évite les achats provisoires et redondants
- conserve une marge confortable pour toute la logique et les périphériques

Impact :

- le schéma et la BOM partent d'un rail principal `5 V` issu du buck
- la référence mécanique exacte du jack pourra dépendre du boîtier final
- `LICENSING.md` sert de table de répartition

---

## 2. Règle de changement

Une decision de cette liste ne doit être rouverte que si :

- un blocage réel apparaît
- une contrainte hardware ou firmware nouvelle le justifie
- ou un bénéfice net et important est démontré
