# DIX PAS - Pin Map Draft

Note documentaire :

- ce document propose un mapping initial de broches pour lancer le dev et le schema
- ce mapping est un brouillon de travail
- en cas de conflit de scope, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte

## 1. Objet

Ce document fige un premier mapping des broches pour l'architecture cible :

- Arduino Nano Every
- MIDI DIN IN / OUT
- Gate Out A / B
- OLED I2C
- encodeur
- chaines 74HC165 / 74HC595

Le but est d'eviter de perdre du temps plus tard a re-decider les memes affectations.

---

## 2. Principes

Principes retenus :

- reserver `D0 / D1` au MIDI DIN
- reserver `A4 / A5` au bus I2C
- reserver `D11 / D12 / D13` a la chaine serie UI
- garder les Gates sur des GPIO simples
- garder quelques broches libres pour debug ou ajustements

---

## 3. Mapping propose

| Fonction | Broche Nano Every | Statut | Remarques |
| --- | --- | --- | --- |
| MIDI IN RX | `D0` | reserve | UART MIDI DIN IN |
| MIDI OUT TX | `D1` | reserve | UART MIDI DIN OUT |
| Encodeur A | `D2` | reserve | interruption ou polling rapide |
| Encodeur B | `D3` | reserve | interruption ou polling rapide |
| Encodeur poussoir | `D4` | reserve | entree directe |
| Gate Out A | `D5` | reserve | sortie bufferisee |
| Gate Out B | `D6` | reserve | sortie bufferisee |
| LED systeme 1 | `D7` | provisoire | libre si non utilisee |
| LED systeme 2 | `D8` | provisoire | libre si non utilisee |
| 74HC165 load | `D9` | reserve | `PL` / capture entrees |
| 74HC595 latch | `D10` | reserve | `RCLK` / latch sorties |
| 74HC595 data | `D11` | reserve | `SER` / MOSI |
| 74HC165 data | `D12` | reserve | `QH` / MISO |
| Clock commun 165/595 | `D13` | reserve | `SCK` commun |
| Libre / futur 1 | `A0` | libre | debug ou LED systeme |
| Libre / futur 2 | `A1` | libre | debug ou service |
| Libre / futur 3 | `A2` | libre | reserve |
| Libre / futur 4 | `A3` | libre | reserve |
| I2C SDA | `A4` | reserve | ecran + FRAM |
| I2C SCL | `A5` | reserve | ecran + FRAM |

---

## 4. Bus d'entrees

Les entrees utilisateur ne sont pas lues directement par le MCU.

Strategie retenue :

- `5 x 74HC165`

Capacite :

- jusqu'a `40` entrees

Entrees cibles :

- 30 boutons des trois rangees
- 6 boutons systeme
- 1 poussoir encodeur si besoin de le remonter dans la chaine plus tard

Le premier jet de firmware peut laisser l'encodeur sur GPIO directs et les autres boutons sur la chaine `74HC165`.

---

## 5. Bus de sorties

Les LEDs ne sont pas pilotees directement par le MCU.

Strategie retenue :

- `3 x 74HC595`

Capacite :

- jusqu'a `24` sorties

Sorties cibles :

- 10 LEDs piste A
- 10 LEDs piste B
- 4 LEDs systeme

Les LEDs de la troisieme rangee restent hors scope V1 si elles ralentissent le projet.

---

## 6. Gates

Les Gates ne doivent pas sortir directement du MCU vers un jack.

Direction retenue :

- `D5` -> buffer -> `Gate Out A`
- `D6` -> buffer -> `Gate Out B`

Contrainte :

- niveau cible `5 V`

Recommendation :

- buffer type `74HC125` ou etage equivalent

---

## 7. I2C

Le bus I2C relie :

- l'ecran OLED
- la FRAM externe

Broches retenues :

- `A4` = SDA
- `A5` = SCL

Remarque :

- si plusieurs modules I2C sont utilises, il faudra verifier les adresses et la qualite du bus

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

- LEDs systeme supplementaires
- debug
- instrumentation
- extension future non critique

---

## 9. Points a verrouiller plus tard

Ce document ne fige pas encore :

- le schema exact du buffer MIDI OUT
- le schema exact du MIDI IN
- la topologie exacte des Gate Out
- la presence finale des LEDs systeme directes

---

## 10. Recommandation de mise en oeuvre

Au moment du schema :

1. garder ce mapping autant que possible
2. ne changer une broche que s'il y a une contrainte physique ou electrique reelle
3. si une broche change, mettre a jour ce document tout de suite
