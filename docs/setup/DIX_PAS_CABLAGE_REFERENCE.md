# DIX PAS - Câblage de Référence

Ce document sert de checklist de câblage au moment de l'assemblage.

Le but n'est pas de remplacer le schéma.

Le but est de vérifier rapidement que :

- chaque sous-système est bien relié
- le câblage correspond au firmware
- le bring-up se fera sans ambiguïté

Source de vérité firmware côté câblage :

- [hardware_config.hpp](/Users/jdebaeck/Documents/Développements/Dix pas/firmware/include/dixpas/hardware_config.hpp:1)

Référence documentaire complémentaire :

- [DIX_PAS_PIN_MAP_DRAFT.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md:1)

## 1. Nano Every

Réservations de broches à respecter :

- `D0` : `MIDI IN RX`
- `D1` : `MIDI OUT TX`
- `D2` : encodeur A
- `D3` : encodeur B
- `D4` : poussoir encodeur
- `D5` : `Gate Out A`
- `D6` : `Gate Out B`
- `D9` : `74HC165 PL`
- `D10` : `74HC595 RCLK`
- `D11` : `74HC595 SER`
- `D12` : `74HC165 QH`
- `D13` : clock partagé `74HC165 / 74HC595`
- `A4` : `I2C SDA`
- `A5` : `I2C SCL`

Checklist :

- [ ] le `Nano Every` est orienté correctement
- [ ] les broches `D0 / D1` ne sont utilisées que pour le MIDI DIN
- [ ] `A4 / A5` sont réservées au bus `I2C`
- [ ] `D11 / D12 / D13` sont réservées à la chaîne série façade

## 2. Encodeur

Connexions à vérifier :

- [ ] encodeur phase A -> `D2`
- [ ] encodeur phase B -> `D3`
- [ ] poussoir encodeur -> `D4`
- [ ] masse encodeur reliée à la masse commune

Hypothèse firmware :

- encodeur et poussoir en actif bas

## 3. Chaîne boutons `74HC165`

Hypothèse firmware :

- `5 x 74HC165`
- boutons actifs bas
- ordre logique :
  - bits `0..9` : rangée piste A
  - bits `10..19` : rangée piste B
  - bits `20..29` : rangée 3
  - bit `30` : `PLAY`
  - bit `31` : `STOP`
  - bit `32` : `RESET`
  - bit `33` : `MODE`
  - bit `34` : `SHIFT`

Connexions à vérifier :

- [ ] `PL` -> `D9`
- [ ] `QH` final de chaîne -> `D12`
- [ ] `CP` / clock partagé -> `D13`
- [ ] alimentation correcte de chaque `74HC165`
- [ ] masse commune sur toute la chaîne
- [ ] ordre réel des registres compatible avec le mapping firmware

## 4. Chaîne LEDs `74HC595`

Hypothèse firmware :

- `3 x 74HC595`
- ordre logique :
  - bits `0..9` : LEDs piste A
  - bits `10..19` : LEDs piste B
  - bits `20..23` : LEDs système

Connexions à vérifier :

- [ ] `RCLK` / latch -> `D10`
- [ ] `SER` / data -> `D11`
- [ ] `SRCLK` / clock partagé -> `D13`
- [ ] alimentation correcte de chaque `74HC595`
- [ ] résistances de LEDs présentes
- [ ] ordre réel des registres compatible avec le mapping firmware

## 5. OLED I2C

Hypothèse firmware :

- contrôleur `SSD1306`
- adresse `0x3C`
- bus `I2C` à `400 kHz`

Connexions à vérifier :

- [ ] `SDA` écran -> `A4`
- [ ] `SCL` écran -> `A5`
- [ ] alimentation correcte de l'écran
- [ ] masse commune

## 6. FRAM I2C

Hypothèse firmware :

- adresse `0x50`
- bus `I2C` partagé avec l'OLED

Connexions à vérifier :

- [ ] `SDA` FRAM -> `A4`
- [ ] `SCL` FRAM -> `A5`
- [ ] alimentation correcte de la FRAM
- [ ] masse commune
- [ ] pas de conflit d'adresse sur le bus

## 7. MIDI DIN OUT

Connexions à vérifier :

- [ ] `TX` UART depuis `D1`
- [ ] étage de sortie MIDI conforme au schéma retenu
- [ ] connecteur DIN orienté correctement
- [ ] masse et blindage gérés comme prévu au schéma

## 8. MIDI DIN IN

Connexions à vérifier :

- [ ] entrée DIN conforme
- [ ] optocoupleur orienté correctement
- [ ] sortie logique de l'entrée MIDI reliée à `D0`
- [ ] masse et alimentation de l'étage d'entrée correctes

## 9. Gate Out

Connexions à vérifier :

- [ ] `D5` -> étage `Gate Out A`
- [ ] `D6` -> étage `Gate Out B`
- [ ] sorties prévues pour un niveau haut `5 V`
- [ ] jacks câblés correctement
- [ ] masse commune propre

## 10. Contrôle final avant flash

Avant le premier firmware sur la vraie machine :

- [ ] aucun court-circuit visible
- [ ] continuité masse vérifiée
- [ ] alimentation vérifiée à vide
- [ ] connecteurs orientés correctement
- [ ] câblage conforme à [hardware_config.hpp](/Users/jdebaeck/Documents/Développements/Dix pas/firmware/include/dixpas/hardware_config.hpp:1)
- [ ] câblage conforme au schéma définitif

Ensuite seulement :

- suivre [DIX_PAS_BRING_UP_HARDWARE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_BRING_UP_HARDWARE.md:1)
