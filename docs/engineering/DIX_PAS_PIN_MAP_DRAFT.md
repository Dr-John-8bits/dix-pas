# DIX PAS - Pin Map Draft

Note documentaire :

- ce document propose un mapping initial de broches pour lancer le dev et le schéma
- ce mapping est un brouillon de travail
- en cas de conflit de scope, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte
- le point de vérité firmware côté brochage est `firmware/include/dixpas/hardware_config.hpp`

## 1. Objet

Ce document figé un premier mapping des broches pour l'architecture cible :

- Arduino Nano Every
- MIDI DIN IN / OUT
- Gate Out A / B
- OLED I2C
- encodeur
- chaines 74HC165 / 74HC595

Le but est d'éviter de perdre du temps plus tard a re-decider les mêmes affectations.

---

## 2. Principes

Principes retenus :

- reserver `D0 / D1` au MIDI DIN
- reserver `A4 / A5` au bus I2C
- reserver `D11 / D12 / D13` a la chaine série UI
- garder les Gates sur des GPIO simples
- garder quelques broches libres pour debug ou ajustements

---

## 3. Mapping propose

| Fonction | Broche Nano Every | Statut | Remarques |
| --- | --- | --- | --- |
| MIDI IN RX | `D0` | réservé | UART MIDI DIN IN |
| MIDI OUT TX | `D1` | réservé | UART MIDI DIN OUT |
| Encodeur A | `D2` | réservé | interruption ou polling rapide |
| Encodeur B | `D3` | réservé | interruption ou polling rapide |
| Encodeur poussoir | `D4` | réservé | entrée directe |
| Gate Out A | `D5` | réservé | sortie bufferisée |
| Gate Out B | `D6` | réservé | sortie bufferisée |
| LED système 1 | `D7` | provisoire | libre si non utilisée |
| LED système 2 | `D8` | provisoire | libre si non utilisée |
| 74HC165 load | `D9` | réservé | `PL` / capture entrées |
| 74HC595 latch | `D10` | réservé | `RCLK` / latch sorties |
| 74HC595 data | `D11` | réservé | `SER` / MOSI |
| 74HC165 data | `D12` | réservé | `QH` / MISO |
| Clock commun 165/595 | `D13` | réservé | `SCK` commun |
| Libre / futur 1 | `A0` | libre | debug ou LED système |
| Libre / futur 2 | `A1` | libre | debug ou service |
| Libre / futur 3 | `A2` | libre | réservé |
| Libre / futur 4 | `A3` | libre | réservé |
| I2C SDA | `A4` | réservé | écran + FRAM |
| I2C SCL | `A5` | réservé | écran + FRAM |

---

## 4. Bus d'entrées

Les entrées utilisateur ne sont pas lues directement par le MCU.

Stratégie retenue :

- `5 x 74HC165`

Capacite :

- jusqu'a `40` entrées

Entrees cibles :

- 30 boutons des trois rangées
- 6 boutons système
- 1 poussoir encodeur si besoin de le remonter dans la chaine plus tard

Le premier jet de firmware peut laisser l'encodeur sur GPIO directs et les autres boutons sur la chaine `74HC165`.

### 4.1 Mapping logique firmware de la chaîne `74HC165`

Le firmware actuel suppose l'ordre logique suivant sur les `40` entrées de la chaîne :

- bits `0..9` : rangée piste A, pas `1..10`
- bits `10..19` : rangée piste B, pas `1..10`
- bits `20..29` : rangée 3, pas `1..10`
- bit `30` : `PLAY`
- bit `31` : `STOP`
- bit `32` : `RESET`
- bit `33` : `MODE`
- bit `34` : `SHIFT`
- bits `35..39` : réservés

Hypothèse retenue dans le code :

- boutons de la chaîne `74HC165` câblés en **actif bas**
- encodeur sur `D2 / D3`
- poussoir encodeur sur `D4`, lui aussi en **actif bas**

Important :

- si l'ordre réel des registres ou l'ordre des bits change au schéma, il faudra mettre à jour le mapping dans `firmware/src/ui_hardware.cpp`

---

## 5. Bus de sorties

Les LEDs ne sont pas pilotees directement par le MCU.

Stratégie retenue :

- `3 x 74HC595`

Capacite :

- jusqu'a `24` sorties

Sorties cibles :

- 10 LEDs piste A
- 10 LEDs piste B
- 4 LEDs système

Les LEDs de la troisième rangée restent hors scope V1 si elles ralentissent le projet.

### 5.1 Mapping logique firmware de la chaîne `74HC595`

Le firmware actuel suppose l'ordre logique suivant sur les `24` sorties :

- bits `0..9` : LEDs rangée piste A, pas `1..10`
- bits `10..19` : LEDs rangée piste B, pas `1..10`
- bit `20` : LED système `TRANSPORT`
- bit `21` : LED système `SYNC EXT`
- bit `22` : LED système `SHIFT`
- bit `23` : LED système `GLOBAL`

Comportement actuel côté firmware :

- LEDs piste A / B : état des steps actifs, avec sélection toujours visible
- LED `TRANSPORT` : lecture active
- LED `SYNC EXT` : clock MIDI externe sélectionnée
- LED `SHIFT` : `SHIFT` maintenu
- LED `GLOBAL` : page `GLOBAL_EDIT` active

Important :

- si l'ordre réel des `74HC595` change au schéma, il faudra ajuster le mapping dans `firmware/src/panel_led_driver.cpp`

---

## 6. Gates

Les Gates ne doivent pas sortir directement du MCU vers un jack.

Direction retenue :

- `D5` -> buffer -> `Gate Out A`
- `D6` -> buffer -> `Gate Out B`

Contrainte :

- niveau cible `5 V`

Recommendation :

- buffer type `74HC125` ou étage équivalent

---

## 7. I2C

Le bus I2C relié :

- l'écran OLED
- la FRAM externe

Broches retenues :

- `A4` = SDA
- `A5` = SCL

Remarque :

- si plusieurs modules I2C sont utilisés, il faudra verifier les adresses et la qualite du bus
- hypothèse actuelle : OLED en `0x3C`
- hypothèse actuelle : FRAM en `0x50`
- le firmware OLED vise d'abord un module `SSD1306`
- le firmware découpe les transferts FRAM pour rester sous les limites de buffer I2C Arduino

---

## 8. Broches libres

Les broches suivantes sont volontairement laissees libres ou peu engagees :

- `D7`
- `D8`
- `A0`
- `A1`
- `A2`
- `A3`

Usages possibles plus tard :

- LEDs système supplementaires
- debug
- instrumentation
- extension future non critique

---

## 9. Points a verrouiller plus tard

Ce document ne figé pas encore :

- le schéma exact du buffer MIDI OUT
- le schéma exact du MIDI IN
- la topologie exacte des Gate Out
- la presence finale des LEDs système directes

---

## 10. Recommandation de mise en oeuvre

Au moment du schéma :

1. garder ce mapping autant que possible
2. ne changer une broche que s'il y a une contrainte physique ou electrique réelle
3. si une broche change, mettre à jour ce document tout de suite
