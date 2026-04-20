# DIX PAS - BOM d'Achat Cœur V1

## 1. Objet

Cette BOM sert à acheter le **cœur électronique** du projet sans attendre le boîtier final.

Elle est pensée pour :

- un premier montage **breadboard / sous-ensembles**
- une validation progressive du hardware
- éviter les achats provisoires inutiles

Elle ne couvre pas encore :

- le boîtier final
- la façade finale
- les capuchons et choix esthétiques
- la mécanique de montage définitive

Références documentaires associées :

- [DIX_PAS_SOURCE_DE_VERITE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/product/DIX_PAS_SOURCE_DE_VERITE.md:1)
- [DIX_PAS_ALIMENTATION_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/engineering/DIX_PAS_ALIMENTATION_REFERENCE.md:1)
- [DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/engineering/DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md:1)
- [DIX_PAS_PLAN_PROTO_BREADBOARD.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_PLAN_PROTO_BREADBOARD.md:1)

---

## 2. Sous-ensembles à acheter maintenant

| Sous-système | Qte | Référence recommandée | Remarque |
| --- | ---: | --- | --- |
| MCU | 1 | Arduino Nano Every | Base officielle V1 |
| Buck 5 V | 1 | Pololu D24V25F5 | Alimentation finale retenue |
| OLED | 1 | Adafruit Product 938 | Référence écran V1 validée |
| FRAM | 1 | Adafruit Product 1895 | Breakout I2C pratique pour proto |
| DIN châssis MIDI IN | 1 | REAN NYS325 | Même référence pour IN et OUT |
| DIN châssis MIDI OUT | 1 | REAN NYS325 | Même référence pour IN et OUT |
| Registre entrée | 5 | TI SN74HC165N | `PDIP-16`, pratique sur breadboard |
| Registre sortie | 3 | TI SN74HC595N | `PDIP-16`, pratique sur breadboard |
| Buffer partagé MIDI/Gate | 1 | TI SN74HC125N | `PDIP-14`, achat recommandé pour proto |
| Optocoupleur MIDI IN | 1 | 6N138 `DIP-8` | Famille retenue pour V1 |
| Encodeur | 1 | Bourns PEC11H | Avec poussoir intégré |

---

## 3. Consommables et passifs minimum

À acheter soit au détail, soit en petit assortiment :

| Élément | Qte minimum | Remarque |
| --- | ---: | --- |
| Résistances `220 Ω` | 5 | `MIDI OUT`, `MIDI IN`, `Gate Out` |
| Résistances `10 kΩ` | 1 | Pull-up `6N138` |
| Résistances `4.7 kΩ` | 1 | Réglage `6N138` |
| Résistances `100 kΩ` | 2 | Pull-down `Gate Out` |
| Résistances `330 Ω` | 24 | LEDs façade |
| Diodes `1N4148` | 2 | `1` utile + marge |
| Condensateurs `100 nF` | 12 | Découplage CI + marge |
| Condensateurs `47 à 100 µF` | 2 | Entrée / sortie buck |
| Breadboards | 2 | Une principale + une annexe |
| Fils Dupont | 1 set | Longueurs variées |
| Supports DIP-8 | 2 | `6N138` + marge |
| Supports DIP-14 | 2 | `74HC125` + marge |
| Supports DIP-16 | 8 | `74HC165` et `74HC595` |

---

## 4. Éléments à acheter après validation breadboard

Ces éléments dépendent davantage de la mécanique finale ou du ressenti :

| Sous-système | Qte | Référence recommandée | Statut |
| --- | ---: | --- | --- |
| Boutons des `3` rangées | 30 | Omron B3F-1000 | Achat recommandé dès que la façade démarre |
| Boutons système | 5 | Omron B3F-1000 | Même référence pour garder une sensation homogène |
| LEDs façade | 24 | LED diffuses `3 mm` | Couleur finale encore ouverte |
| Jacks `Gate Out` panneau | 2 | Switchcraft 35PM1 | Référence désormais recommandée |
| Jack alimentation `2.1 mm` | 1 | Switchcraft 721AU | Référence désormais recommandée |
| Interrupteur panneau | 1 | Famille C&K PT Series, `SPST` maintenu `ON/OFF` | Suffixe exact à figer avec le panneau final |
| Entretoises, visserie, panneau, boîtier | - | - | Hors scope de cette BOM cœur |

---

## 5. Choix d'achat importants

### 5.1 Pourquoi `SN74HC125N` et pas directement `74HCS125`

Le firmware et l'architecture restent compatibles avec la famille `74HC125`.

Le choix `SN74HC125N` est retenu ici parce que :

- il existe en `PDIP-14`
- il est bien plus simple à utiliser sur breadboard
- il évite d'acheter tout de suite un composant `SMD` + adaptateur juste pour le proto

Le `74HCS125` reste acceptable plus tard si on veut compacter le montage final.

### 5.2 Pourquoi la FRAM breakout Adafruit

Le but n'est pas d'optimiser le coût final du PCB dès maintenant.

Le but est de :

- valider rapidement le bus `I2C`
- valider la persistance presets
- éviter de perdre du temps sur une petite intégration mémoire au tout début

### 5.3 Pourquoi les `74HC165N` et `74HC595N` en `PDIP`

Même logique :

- facile à breadboarder
- facile à remplacer
- cohérent avec la phase de validation par sous-ensembles

### 5.4 Pourquoi `B3F-1000` pour les boutons

Le choix `Omron B3F-1000` est recommandé parce que :

- la famille `B3F` est toujours en production
- le format `12 x 12 mm` through-hole est adapté à un premier circuit propre
- la page officielle OMRON annonce une durabilité étendue pour les `12 x 12 mm`

Alternative si tu veux une variante plus protégée plus tard :

- `Omron B3W-1000`

### 5.5 Pourquoi `35PM1` et `721AU`

Ces deux références sont retenues parce qu'elles sont déjà de vraies pièces panneau :

- `Switchcraft 35PM1` pour les `Gate Out`
- `Switchcraft 721AU` pour l'alimentation `2.1 mm`

Elles évitent de partir sur des jacks trop “proto only” qu'il faudrait remplacer juste après.

---

## 6. Ordre d'achat recommandé

1. alimentation `9 V` + buck `5 V`
2. `Arduino Nano Every`
3. OLED + FRAM
4. `MIDI OUT` + `Gate Out`
5. `MIDI IN`
6. encodeur
7. logique `74HC165 / 74HC595`
8. passifs / supports / breadboards
9. façade et mécanique plus tard

---

## 7. Statut

Cette BOM n'est pas encore la BOM mécanique finale du projet.

En revanche, elle est suffisante pour :

- commencer les achats utiles
- valider le circuit réel
- avancer jusqu'au bring-up complet sans re-réfléchir à l'architecture de base
