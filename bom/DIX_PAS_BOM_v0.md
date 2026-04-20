# DIX PAS - BOM v0

Note documentaire :

- cette BOM dérive du scope figé dans `docs/product/DIX_PAS_SOURCE_DE_VERITE.md`
- en cas de conflit de scope, le `SOURCE_DE_VERITE` l'emporte

## 1. Objet

Cette BOM v0 est une base de depart pour lancer l'assemblage du projet.

Elle n'est pas encore une BOM de production finale.

Elle vise a :

- verrouiller les sous-ensembles principaux
- faciliter les premiers achats
- garder de la souplesse avant schéma final et routage PCB

---

## 2. Hypotheses de cette BOM

Hypotheses retenues :

- architecture **2 x 10 pas**
- **3 rangées de 10 boutons**
- **MIDI DIN 5 broches** uniquement
- **1 MIDI IN + 1 MIDI OUT**
- **2 Gate Out**
- base contrôleur : **Arduino Nano Every**
- afficheur **OLED I2C**
- stockage presets sur mémoire externe

---

## 3. Controleur et logique principale

| Sous-système | Qte | Reference recommandée | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Carte contrôleur | 1 | Arduino Nano Every | Arduino Nano Every with headers | Recommandation principale pour la V1 |
| Buffer partagé MIDI OUT / Gate | 1 | TI SN74HC125N | 74HCS125 SMD | `PDIP-14` recommandé pour proto breadboard, 1 canal MIDI OUT, 2 canaux Gate, 1 libre |
| Registre a décalage entrée | 5 | 74HC165 | SN74HC165 | Jusqu'à 40 entrées utilisateur |
| Registre a décalage sortie | 3 | 74HC595 | SN74HC595 | Jusqu'à 24 sorties LEDs |
| Condensateurs de decouplage | 10 | 100 nF ceramique | Equivalent | 1 par CI minimum |

---

## 4. MIDI DIN

| Sous-système | Qte | Reference recommandée | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Connecteur MIDI IN | 1 | REAN NYS325 | Lumberg 0103 | DIN 5 broches femelle châssis |
| Connecteur MIDI OUT | 1 | REAN NYS325 | Lumberg 0103 | DIN 5 broches femelle châssis |
| Optocoupleur MIDI IN | 1 | 6N138 | H11L1 | Reference de travail V1, validation breadboard requise |
| Resistances MIDI OUT | 2 | 220 ohm 1/4 W | Equivalent | Valeur standard du current loop 5 V |
| Resistance MIDI IN serie | 1 | 220 ohm 1/4 W | Equivalent | Serie entree opto |
| Resistance MIDI IN pull-up | 1 | 10 kohm | Equivalent | Pull-up sortie `6N138` |
| Resistance MIDI IN base | 1 | 4.7 kohm | Equivalent | Reglage vitesse / stabilite `6N138` |
| Diode signal | 1 | 1N4148 | Equivalent | Antiparallèle protection entrée opto |
| Condensateur MIDI IN | 1 | 100 nF ceramique | Equivalent | Decouplage `6N138` |

---

## 5. Sorties Gate

| Sous-système | Qte | Reference recommandée | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Connecteur Gate Out A | 1 | Jack mono 3.5 mm panel mount | Jack mono 6.35 mm | 3.5 mm recommandé pour garder le format compact |
| Connecteur Gate Out B | 1 | Jack mono 3.5 mm panel mount | Jack mono 6.35 mm | Meme logique que Gate Out A |
| Passifs Gate Out | 1 set | `2 x 220 ohm` serie + `2 x 100 kohm` pull-down | Equivalent | Base de reference V1 |

---

## 6. Interface utilisateur

| Sous-système | Qte | Reference recommandée | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Boutons de pas / contrôle | 30 | Omron B3F série 12 x 12 mm | E-Switch TL1105 série | Boutons principaux des 3 rangées |
| Boutons système | 5 | Omron B3F ou équivalent | E-Switch TL1105 | Play, Stop, Reset, Shift, Mode |
| Encodeur rotatif avec poussoir | 1 | Bourns PEC11H | Alps / équivalent qualitatif | Detentes franches recommandées |
| LEDs piste A / B | 20 | LED diffuses 3 mm | LED diffuses 5 mm | Couleur à définir avec la facade |
| LEDs système | 4 | LED diffuses 3 mm | Equivalent | Etat mode / sync / transport |
| Resistances LEDs | 24 | 330 ohm | Valeur a ajuster | Une par LED ou reseaux de résistances |

---

## 7. Affichage et stockage

| Sous-système | Qte | Reference recommandée | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Écran | 1 | Adafruit Product 938, OLED 1.3" 128 x 64 I2C SSD1306 | Module SSD1306 0x3C équivalent | Référence V1 validée, format 1.3" recommandé |
| Détails écran | 1 | Adresse `0x3C`, écran `34.5 x 23 mm`, zone active `29.42 x 14.70 mm` | - | Dimensions utiles pour la façade et la lisibilité |
| Memoire presets | 1 | FRAM I2C 32 KB | EEPROM I2C 24LC256 | FRAM recommandée pour ecritures frequentes |
| Exemple FRAM | 1 | Adafruit FRAM 256 Kbit Product 1895 | Module équivalent | Bonne base pour validation rapide |

---

## 8. Alimentation

| Sous-système | Qte | Reference recommandée | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Alimentation externe | 1 | Adaptateur `9 V DC`, centre positif, `1 A` min | Adaptateur régulé équivalent | Standard V1 retenu |
| Connecteur alimentation | 1 | Standard barrel `2.1 mm`, centre positif | Réf mécanique selon boîtier | Le standard électrique est figé, pas la mécanique finale |
| Interrupteur general | 1 | SPST panel mount | Toggle ou rocker équivalent | Marche / arrêt |
| Conversion vers 5 V | 1 | Pololu D24V25F5 | Buck `5 V` 2 A+ équivalent | Buck dédié retenu pour la V1 |
| Protection polarite | 0 | Intégrée au D24V25F5 | À ajouter si autre buck | Pas d'étage séparé si buck retenu |
| Filtrage alim | 1 set | `47-100 µF` entrée, `47-100 µF` sortie, `100 nF` par CI | Equivalent | Base recommandée pour le premier schéma |

---

## 9. Mecanique et assemblage

| Sous-système | Qte | Reference recommandée | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| PCB principale | 1 | Sur mesure | - | A venir |
| Entretoises | 1 set | M3 | Equivalent | Fixation facade / PCB |
| Visserie | 1 set | M3 | Equivalent | A adapter au boîtier |
| Boutons / capuchons | 1 set | Selon facade | Equivalent | A choisir avec l'ergonomie finale |
| Knob encodeur | 1 | Axe compatible encodeur | Equivalent | A choisir selon l'esthetique |

---

## 10. Points encore a verrouiller

Cette BOM ne fige pas encore :

- la référence mécanique exacte du jack d'alimentation
- la couleur finale des LEDs
- la presence ou non de LEDs sur la troisième rangée
- le type exact de boîtier

---

## 11. Priorites d'achat

Si tu veux acheter par vagues, l'ordre recommandé est :

1. carte contrôleur
2. MIDI IN / OUT DIN
3. sorties Gate
4. encodeur
5. écran
6. boutons principaux
7. optocoupleur + logique
8. mémoire externe
9. LEDs et passifs
10. alimentation
11. mécanique

---

## 12. Remarque projet

Cette BOM v0 est volontairement pragmatique.

Elle sert a lancer le projet proprement sans attendre le schéma final, tout en gardant une direction réaliste pour aboutir a un vrai instrument open source.
