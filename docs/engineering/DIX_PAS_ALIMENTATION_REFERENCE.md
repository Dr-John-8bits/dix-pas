# DIX PAS - Alimentation de référence

## 1. Objet

Ce document fixe l'architecture d'alimentation recommandée pour la V1.

Objectif :

- éviter une alimentation provisoire qui deviendrait inutile plus tard
- tester le circuit avec la même logique d'alimentation que l'instrument final
- fournir une base claire pour la BOM, le schéma et le bring-up

## 2. Architecture retenue

Architecture validée :

- **alimentation externe dédiée**
- **adaptateur 9 V DC régulé**
- **polarité centre positif**
- **connecteur barrel 2.1 mm**
- **interrupteur général**
- **buck régulateur 5 V dédié**
- **rail système 5 V unique**
- **USB réservé au flash et au debug**

Référence de buck recommandée :

- **Pololu D24V25F5**

Pourquoi cette base est retenue :

- marge confortable pour la V1
- plus saine qu'une alimentation reposant sur l'USB
- plus saine que de faire porter tout le système par la voie `VIN` du `Nano Every`
- évite des achats provisoires qui seraient abandonnés ensuite

## 3. Chaîne d'alimentation recommandée

Chaîne logique :

`adaptateur 9 V DC`
-> `jack barrel 2.1 mm`
-> `interrupteur`
-> `buck 5 V`
-> `rail 5 V`
-> `Nano Every + OLED + FRAM + logique + LEDs + étages MIDI/Gate`

Règles :

- toutes les masses doivent être communes
- le `5 V` système doit venir du buck dédié
- l'USB ne doit pas être la source principale de l'instrument final

## 4. Références retenues

### 4.1 Adaptateur externe

Spécification retenue :

- `9 V DC`
- `centre positif`
- `2.1 mm`
- `1 A minimum` recommandé

Note :

- la valeur `1 A minimum` est une recommandation de marge de conception
- ce n'est pas une contrainte issue d'une mesure finale du projet

### 4.2 Buck 5 V

Référence recommandée :

- **Pololu D24V25F5**

Caractéristiques utiles :

- sortie `5 V`
- entrée `6 V à 38 V`
- courant continu typique jusqu'à `2.5 A`
- protection contre inversion de polarité intégrée

Cette marge est largement suffisante pour la V1 et laisse de la place pour :

- OLED
- FRAM
- LEDs
- logique CMOS
- étages MIDI
- `Gate Out`

### 4.3 Connecteur d'alimentation

Standard électrique retenu :

- `barrel 2.1 mm`
- `centre positif`

Important :

- la **géométrie mécanique exacte** du jack dépendra du futur panneau ou PCB
- ce point peut rester ouvert tant que le boîtier n'est pas figé
- le **standard électrique**, lui, est désormais figé

## 5. Règles de distribution

Règles de base retenues :

- un seul rail principal `5 V`
- un plan ou bus de masse propre et commun
- condensateurs de découplage `100 nF` au plus près de chaque CI
- capacité de réservoir côté entrée et côté sortie du buck
- attention particulière aux retours de courant des LEDs et des sorties externes

Recommandation simple pour le premier schéma :

- `47 µF` à `100 µF` côté entrée du buck
- `47 µF` à `100 µF` côté sortie `5 V`
- `100 nF` par circuit logique

## 6. Ce qu'on évite volontairement

Pour la V1, on évite :

- une alimentation finale uniquement par USB
- une architecture où le `VIN` du `Nano Every` alimente à lui seul tout le système
- une alimentation hybride confuse `USB + alim externe` sans séparation claire

## 7. Breadboard et validation

Le breadboard reste pertinent pour :

- tester le buck
- valider le rail `5 V`
- tester `OLED`, `FRAM`, `MIDI IN`, `MIDI OUT`, `Gate`

La bonne méthode n'est donc pas :

- `USB -> tout le projet`

La bonne méthode est :

- `alimentation finale retenue`
- `buck 5 V retenu`
- puis validation des sous-blocs sur breadboard

## 8. Impact sur la BOM

La BOM finale devra au minimum contenir :

- un adaptateur `9 V DC` centre positif
- un standard de jack `2.1 mm`
- un interrupteur général
- un buck `5 V` dédié
- les condensateurs de filtrage associés

Le seul point qui peut encore dépendre du boîtier plus tard est :

- la **référence mécanique exacte** du jack et de l'interrupteur

## 9. Statut

Ce document fixe la direction d'alimentation de la V1.

Il devient la base de référence avant :

- les schémas `MIDI IN`
- `MIDI OUT`
- `Gate Out`
- la BOM finale
