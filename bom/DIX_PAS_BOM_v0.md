# DIX PAS - BOM v0

Note documentaire :

- cette BOM derive du scope fige dans `docs/product/DIX_PAS_SOURCE_DE_VERITE.md`
- en cas de conflit de scope, le `SOURCE_DE_VERITE` l'emporte

## 1. Objet

Cette BOM v0 est une base de depart pour lancer l'assemblage du projet.

Elle n'est pas encore une BOM de production finale.

Elle vise a :

- verrouiller les sous-ensembles principaux
- faciliter les premiers achats
- garder de la souplesse avant schema final et routage PCB

---

## 2. Hypotheses de cette BOM

Hypotheses retenues :

- architecture **2 x 10 pas**
- **3 rangees de 10 boutons**
- **MIDI DIN 5 broches** uniquement
- **1 MIDI IN + 1 MIDI OUT**
- **2 Gate Out**
- base controleur : **Arduino Nano Every**
- afficheur **OLED I2C**
- stockage presets sur memoire externe

---

## 3. Controleur et logique principale

| Sous-systeme | Qte | Reference recommandee | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Carte controleur | 1 | Arduino Nano Every | Arduino Nano Every with headers | Recommandation principale pour la V1 |
| Buffer / Schmitt trigger | 1 | 74HC14 | 74HCT14 | Protection et mise en forme MIDI OUT / usages annexes |
| Registre a decalage entree | 5 | 74HC165 | SN74HC165 | Jusqu'a 40 entrees utilisateur |
| Registre a decalage sortie | 3 | 74HC595 | SN74HC595 | Jusqu'a 24 sorties LEDs |
| Condensateurs de decouplage | 9 | 100 nF ceramique | Equivalent | 1 par CI minimum |

---

## 4. MIDI DIN

| Sous-systeme | Qte | Reference recommandee | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Connecteur MIDI IN | 1 | REAN NYS325 | Lumberg 0103 | DIN 5 broches femelle châssis |
| Connecteur MIDI OUT | 1 | REAN NYS325 | Lumberg 0103 | DIN 5 broches femelle châssis |
| Optocoupleur MIDI IN | 1 | 6N138 | H11L1 | Le schema exact dependra du composant retenu |
| Resistances MIDI OUT | 2 | 220 ohm 1/4 W | Equivalent | Valeur standard du current loop 5 V |
| Passifs MIDI IN | 1 set | Selon schema de reference | Selon optocoupleur | Valeurs a figer au schema |
| Diode signal | 1 | 1N4148 | Equivalent | Souvent utile sur l'etage MIDI IN selon le montage |

---

## 5. Sorties Gate

| Sous-systeme | Qte | Reference recommandee | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Connecteur Gate Out A | 1 | Jack mono 3.5 mm panel mount | Jack mono 6.35 mm | 3.5 mm recommande pour garder le format compact |
| Connecteur Gate Out B | 1 | Jack mono 3.5 mm panel mount | Jack mono 6.35 mm | Meme logique que Gate Out A |
| Buffer sorties Gate | 1 | 74HC125 | 74HCT125 | Permet d'isoler le MCU du monde exterieur |
| Passifs Gate Out | 1 set | Selon schema de reference | Equivalent | Resistances serie / pull-down selon l'etage retenu |

---

## 6. Interface utilisateur

| Sous-systeme | Qte | Reference recommandee | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Boutons de pas / controle | 30 | Omron B3F serie 12 x 12 mm | E-Switch TL1105 serie | Boutons principaux des 3 rangees |
| Boutons systeme | 6 | Omron B3F ou equivalent | E-Switch TL1105 | Play, Stop, Reset, Shift, Mode, Select |
| Encodeur rotatif avec poussoir | 1 | Bourns PEC11H | Alps / equivalent qualitatif | Detentes franches recommandees |
| LEDs piste A / B | 20 | LED diffuses 3 mm | LED diffuses 5 mm | Couleur a definir avec la facade |
| LEDs systeme | 4 | LED diffuses 3 mm | Equivalent | Etat mode / sync / transport |
| Resistances LEDs | 24 | 330 ohm | Valeur a ajuster | Une par LED ou reseaux de resistances |

---

## 7. Affichage et stockage

| Sous-systeme | Qte | Reference recommandee | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Ecran | 1 | OLED 128 x 64 I2C compatible SSD1306 / SH1106 | Module equivalent | Format 0.96" a 1.3" |
| Exemple ecran | 1 | Adafruit 1.3" OLED Product 938 | Equivalent | Pratique si tu veux une ref simple et documentee |
| Memoire presets | 1 | FRAM I2C 32 KB | EEPROM I2C 24LC256 | FRAM recommandee pour ecritures frequentes |
| Exemple FRAM | 1 | Adafruit FRAM 256 Kbit Product 1895 | Module equivalent | Bonne base pour validation rapide |

---

## 8. Alimentation

| Sous-systeme | Qte | Reference recommandee | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| Connecteur alimentation | 1 | Barrel jack panneau | A definir | A verrouiller avec le boitier |
| Interrupteur general | 1 | Toggle ou rocker panel mount | Equivalent | Marche / arret |
| Conversion vers 5 V | 1 | Buck regulator 5 V | Module DC-DC equivalent | A privilegier si alim externe > 5 V |
| Protection polarite | 1 | Diode Schottky ou MOSFET ideal diode | Equivalent | Recommande |
| Filtrage alim | 1 set | Electrolytiques + ceramiques | Equivalent | A definir au schema |

---

## 9. Mecanique et assemblage

| Sous-systeme | Qte | Reference recommandee | Alternative | Remarques |
| --- | ---: | --- | --- | --- |
| PCB principale | 1 | Sur mesure | - | A venir |
| Entretoises | 1 set | M3 | Equivalent | Fixation facade / PCB |
| Visserie | 1 set | M3 | Equivalent | A adapter au boitier |
| Boutons / capuchons | 1 set | Selon facade | Equivalent | A choisir avec l'ergonomie finale |
| Knob encodeur | 1 | Axe compatible encodeur | Equivalent | A choisir selon l'esthetique |

---

## 10. Points encore a verrouiller

Cette BOM ne fige pas encore :

- le type exact d'alimentation externe
- les valeurs completes de l'etage MIDI IN
- la topologie finale de l'etage Gate Out
- la couleur finale des LEDs
- la presence ou non de LEDs sur la troisieme rangee
- le type exact de boitier
- la reference finale de l'ecran

---

## 11. Priorites d'achat

Si tu veux acheter par vagues, l'ordre recommande est :

1. carte controleur
2. MIDI IN / OUT DIN
3. sorties Gate
4. encodeur
5. ecran
6. boutons principaux
7. optocoupleur + logique
8. memoire externe
9. LEDs et passifs
10. alimentation
11. mecanique

---

## 12. Remarque projet

Cette BOM v0 est volontairement pragmatique.

Elle sert a lancer le projet proprement sans attendre le schema final, tout en gardant une direction realiste pour aboutir a un vrai instrument open source.
