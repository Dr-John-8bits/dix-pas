# DIX PAS - Référence interfaces MIDI et Gate

## 1. Objet

Ce document fixe les topologies de référence pour :

- `MIDI DIN IN`
- `MIDI DIN OUT`
- `Gate Out A`
- `Gate Out B`

Objectif :

- verrouiller une base électrique cohérente avant la BOM finale
- limiter les achats redondants
- permettre un test propre sur breadboard puis une intégration sur circuit définitif

## 2. Principes retenus

Principes :

- logique système en **5 V**
- `MIDI DIN` conforme au standard courant loop `5 mA`
- `MIDI IN` opto-isolé
- `MIDI OUT` bufferisé
- `Gate Out` bufferisées, `5 V`, `active high`
- mutualisation maximale quand elle reste saine

Décision d'architecture :

- un **seul buffer partagé de la famille 74HC125** est retenu
- usages prévus :
  - `1` canal pour `MIDI OUT`
  - `2` canaux pour `Gate Out A/B`
  - `1` canal restant libre

Choix d'achat recommandé pour le proto breadboard :

- **SN74HC125N** en `PDIP-14`

Alternative compacte acceptable :

- `74HCS125` en `SMD` si un montage plus dense est visé plus tard

Cette topologie est une **recommandation d'ingénierie** à partir :

- du standard MIDI officiel
- des caractéristiques du `Nano Every`
- des familles logiques `74HC/HCS`
- des optocoupleurs `6N138`

## 3. MIDI DIN OUT

### 3.1 Topologie retenue

Chaîne logique :

`D1 / TX`
-> `1 canal 74HC125`
-> résistance série `220 Ω`
-> `DIN pin 5`

et en parallèle :

`+5 V`
-> résistance série `220 Ω`
-> `DIN pin 4`

Références de connecteur :

- `DIN pin 4` : branche alimentation courant loop
- `DIN pin 5` : branche signal courant loop
- `DIN pin 2` : blindage / référence châssis selon intégration mécanique

### 3.2 Valeurs retenues

Valeurs de référence :

- `2 x 220 Ω` pour le current loop MIDI OUT
- `1 x 100 nF` au plus près du buffer partagé

### 3.3 Pourquoi cette solution

Raisons :

- respecte la logique du current loop MIDI
- ajoute une couche de protection entre MCU et prise DIN
- évite d'ajouter un buffer séparé juste pour le MIDI OUT
- garde une sortie simple à tester

## 4. MIDI DIN IN

### 4.1 Topologie retenue

Optocoupleur retenu :

- **6N138**

Chaîne logique de référence :

`DIN pin 4`
-> résistance `220 Ω`
-> anode LED opto

`DIN pin 5`
-> cathode LED opto

Protection LED opto :

- `1 x 1N4148` en antiparallèle sur la LED d'entrée de l'opto

Sortie logique :

- `6N138 pin 8` -> `+5 V`
- `6N138 pin 5` -> `GND`
- `6N138 pin 6` -> `D0 / RX`
- pull-up `10 kΩ` entre `pin 6` et `+5 V`
- résistance `4.7 kΩ` entre `pin 7` et `GND`
- `100 nF` entre `pin 8` et `pin 5`

### 4.2 Valeurs retenues

Valeurs de référence :

- `1 x 220 Ω` en série côté entrée opto
- `1 x 10 kΩ` pull-up sortie
- `1 x 4.7 kΩ` sur la base / bande passante du `6N138`
- `1 x 1N4148`
- `1 x 100 nF`

### 4.3 Statut et prudence

Cette topologie est la **référence V1 recommandée**.

Important :

- c'est une recommandation d'ingénierie cohérente avec le standard MIDI et l'usage courant du `6N138`
- si le breadboard montre un front trop mou ou une réception imparfaite, le premier point d'ajustement sera la combinaison `pull-up / résistance pin 7`
- si l'opto retenu change, les valeurs peuvent devoir évoluer

## 5. Gate Out A / B

### 5.1 Topologie retenue

Chaque sortie suit la même structure :

`D5 ou D6`
-> `1 canal 74HC125`
-> résistance série `220 Ω`
-> tip jack `3.5 mm`

et en parallèle sur la sortie :

- pull-down `100 kΩ` vers `GND`

Raccordement :

- `tip` = signal gate
- `sleeve` = masse

### 5.2 Valeurs retenues

Par sortie :

- `1 x 220 Ω` série
- `1 x 100 kΩ` pull-down

Pour les deux sorties :

- `2 x 220 Ω`
- `2 x 100 kΩ`

### 5.3 Comportement visé

Comportement électrique :

- niveau haut cible : `5 V`
- `active high`
- repos à l'état bas

Comportement fonctionnel :

- `Dual` : `Gate A` suit piste A, `Gate B` suit piste B
- `Chain20` : `Gate A` suit pas `1..10`, `Gate B` suit pas `11..20`

## 6. Schéma fonctionnel résumé

Résumé minimal :

- `MIDI IN`
  - `DIN` -> `220 Ω` -> `6N138`
  - `1N4148`
  - `10 kΩ` pull-up
  - `4.7 kΩ` base
  - `100 nF`

- `MIDI OUT`
  - `+5 V` -> `220 Ω` -> `DIN 4`
  - `TX` -> `74HC125` -> `220 Ω` -> `DIN 5`

- `Gate A/B`
  - `GPIO` -> `74HC125` -> `220 Ω` -> jack tip
  - `100 kΩ` pull-down vers masse

## 7. Impact sur la BOM

Cette référence implique au minimum :

- `1 x 6N138`
- `1 x 74HC125` de référence d'achat proto
- `3 x 220 Ω` pour MIDI
- `2 x 220 Ω` pour Gate
- `2 x 100 kΩ` pour Gate
- `1 x 10 kΩ`
- `1 x 4.7 kΩ`
- `1 x 1N4148`
- `2 x 100 nF` dédiés à ces étages minimum

## 8. Ce qui restera à vérifier sur breadboard

À vérifier avec le vrai montage :

- réception MIDI propre sur `6N138`
- absence de faux déclenchements
- niveau haut réel des `Gate Out`
- comportement sur câble MIDI réel
- stabilité avec l'alimentation finale `9 V -> buck 5 V`

## 9. Statut

Ce document fixe la référence de travail avant :

- la BOM finale
- le schéma détaillé
- le breadboard de validation
