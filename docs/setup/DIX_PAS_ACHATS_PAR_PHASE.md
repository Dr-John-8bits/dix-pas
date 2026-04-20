# DIX PAS - Achats Par Phase

## 1. Objet

Ce document traduit la BOM en **plan d'achat concret**.

Le but est d'éviter :

- les achats redondants
- les achats trop tôt
- les achats mécaniques figés alors que le panneau ne l'est pas encore

Principe :

- acheter **maintenant** ce qui est utile et stable
- acheter **ensuite** ce qui dépend de la validation breadboard
- acheter **plus tard** ce qui dépend du panneau / boîtier

Documents associés :

- [DIX_PAS_BOM_v0.md](/Users/jdebaeck/Documents/Développements/Dix pas/bom/DIX_PAS_BOM_v0.md:1)
- [DIX_PAS_BOM_ACHAT_COEUR_V1.md](/Users/jdebaeck/Documents/Développements/Dix pas/bom/DIX_PAS_BOM_ACHAT_COEUR_V1.md:1)
- [DIX_PAS_PLAN_PROTO_BREADBOARD.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_PLAN_PROTO_BREADBOARD.md:1)
- [DIX_PAS_EXECUTION_STEP_BY_STEP.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_EXECUTION_STEP_BY_STEP.md:1)

---

## 2. Règle simple

Avant la première commande, se poser trois questions :

1. est-ce que cette pièce est déjà figée dans la doc ?
2. est-ce qu'elle sera utile dès la phase breadboard ?
3. est-ce qu'elle dépend du panneau final ?

Si la réponse est :

- `figée + utile tout de suite + ne dépend pas du panneau` : acheter maintenant
- `figée + utile plus tard` : acheter en phase 2
- `dépend du panneau` : attendre

---

## 3. Phase 1 - Achat immédiat

Cette phase sert à faire tourner le **cœur électronique** et le firmware réel.

### 3.1 À commander maintenant

| Sous-système | Qte | Référence recommandée | Pourquoi maintenant |
| --- | ---: | --- | --- |
| Carte MCU | 1 | Arduino Nano Every | Base du projet |
| Buck 5 V | 1 | Pololu D24V25F5 | Alimentation finale retenue |
| Alimentation externe | 1 | Adaptateur `9 V DC`, centre positif, `1 A` min | Nécessaire dès la phase alim |
| OLED | 1 | Adafruit Product 938 | Référence écran validée |
| FRAM | 1 | Adafruit Product 1895 | Validation presets et `I2C` |
| DIN MIDI IN | 1 | REAN NYS325 | Bloc MIDI figé |
| DIN MIDI OUT | 1 | REAN NYS325 | Bloc MIDI figé |
| Buffer MIDI/Gate | 1 | TI SN74HC125N | Proto breadboard facile |
| Optocoupleur MIDI IN | 1 | `6N138` `DIP-8` | Bloc MIDI IN retenu |
| Registres entrée | 5 | TI SN74HC165N | Façade lecture boutons |
| Registres sortie | 3 | TI SN74HC595N | Façade LEDs |
| Encodeur | 1 | Bourns PEC11H | UI locale |
| Breadboards | 2 | Format standard | Banc de proto |
| Fils Dupont | 1 set | Assortiment mâle-mâle / mâle-femelle | Banc de proto |
| Câble USB | 1 | compatible `Nano Every` | Flash firmware |
| Câble MIDI DIN | 1 | standard 5 broches | Test `MIDI IN/OUT` |
| Multimètre | 1 | si absent | Validation alim / gates |

### 3.2 Passifs minimum à acheter en même temps

| Élément | Qte minimum |
| --- | ---: |
| Résistances `220 Ω` | 5 |
| Résistances `10 kΩ` | 1 |
| Résistances `4.7 kΩ` | 1 |
| Résistances `100 kΩ` | 2 |
| Résistances `330 Ω` | 24 |
| Diodes `1N4148` | 2 |
| Condensateurs `100 nF` | 12 |
| Condensateurs `47 à 100 µF` | 2 |
| Supports DIP-8 | 2 |
| Supports DIP-14 | 2 |
| Supports DIP-16 | 8 |

### 3.3 Résultat attendu de la phase 1

Avec cette seule commande, tu dois pouvoir valider :

- alimentation
- `Nano Every`
- `OLED`
- `FRAM`
- `MIDI OUT`
- `Gate Out`
- `MIDI IN`
- encodeur
- premières chaînes `74HC165 / 74HC595`

---

## 4. Phase 2 - Achat après validation breadboard

Cette phase commence quand :

- le cœur électronique est validé
- `MIDI`, `Gate`, `OLED`, `FRAM` et l'encodeur fonctionnent réellement

### 4.1 À commander à ce moment-là

| Sous-système | Qte | Référence recommandée | Pourquoi attendre cette phase |
| --- | ---: | --- | --- |
| Boutons des rangées | 30 | Omron B3F-1000 | Une fois la façade confirmée |
| Boutons système | 5 | Omron B3F-1000 | Même sensation, même empreinte |
| LEDs piste A / B | 20 | LED diffuses `3 mm` | Couleur encore ouverte |
| LEDs système | 4 | LED diffuses `3 mm` | Couleur encore ouverte |
| Jacks `Gate Out` | 2 | Switchcraft 35PM1 | Dès que l'implantation panneau est confirmée |
| Jack alimentation | 1 | Switchcraft 721AU | Pièce finale cohérente, à câbler quand l'alim est validée |

### 4.2 Pourquoi ces pièces ne sont pas en phase 1

- elles sont moins critiques que le cœur électronique
- elles peuvent attendre la validation breadboard sans bloquer l'architecture
- elles immobilisent un peu plus la façade

Nuance :

- si tu veux regrouper les commandes et que tu acceptes ce léger risque, `35PM1`, `721AU` et `B3F-1000` peuvent aussi être achetés dès la phase 1

---

## 5. Phase 3 - Achat après choix du panneau

Cette phase commence quand :

- le routage logique est validé
- le placement façade est clair
- la hauteur disponible et la profondeur utile sont connues

### 5.1 À commander seulement à ce moment-là

| Sous-système | Statut |
| --- | --- |
| Interrupteur général exact | dépend du panneau |
| Capuchons / keycaps | dépend du style final |
| Knob encodeur final | dépend de l'esthétique et de l'espace |
| Entretoises / visserie définitive | dépend de l'épaisseur panneau |
| Boîtier | dépend de tout le reste |
| Panneau avant | dépend de l'implantation réelle |
| PCB final | dépend du schéma validé et des dimensions retenues |

### 5.2 Ce qu'il ne faut pas acheter trop tôt

- un interrupteur final si le diamètre de perçage n'est pas décidé
- la visserie finale si le boîtier n'est pas choisi
- un boîtier générique au hasard

---

## 6. Version la plus prudente

Si tu veux minimiser au maximum le risque d'achat inutile :

### Commande 1

- tout le **cœur électronique**
- tout le **matériel breadboard**
- aucun élément de façade finale sauf encodeur

### Commande 2

- boutons
- LEDs
- jacks `Gate`
- jack alimentation

### Commande 3

- interrupteur exact
- mécanique finale
- boîtier
- panneau

---

## 7. Version la plus efficace

Si tu veux limiter le nombre de commandes sans prendre trop de risque :

### Commande 1

- tout le **cœur électronique**
- `B3F-1000`
- `35PM1`
- `721AU`

### Commande 2

- LEDs de couleur finale
- interrupteur exact
- mécanique finale

Mon avis :

- c'est probablement le meilleur compromis pour `DIX PAS`

---

## 8. Ce qu'on sait maintenant avec assez de confiance

Pièces vraiment bien verrouillées :

- `Arduino Nano Every`
- `Pololu D24V25F5`
- `Adafruit 938`
- `Adafruit 1895`
- `REAN NYS325`
- `SN74HC125N`
- `SN74HC165N`
- `SN74HC595N`
- `6N138`
- `Bourns PEC11H`
- `Omron B3F-1000`
- `Switchcraft 35PM1`
- `Switchcraft 721AU`

Pièce encore semi-ouverte :

- interrupteur général exact

---

## 9. Règle d'arrêt achat

Tu peux lancer les achats phase 1 sans attendre davantage.

Tu peux lancer les achats phase 2 après validation breadboard.

Tu ne dois pas lancer les achats phase 3 tant que le panneau n'est pas suffisamment défini.
