# DIX PAS - Plan Proto Breadboard

## 1. Objet

Ce document définit l'ordre recommandé pour valider le hardware de `DIX PAS` sans repartir en conception au moment de la réception des composants.

Le principe retenu :

- valider le projet **par sous-ensembles**
- n'intégrer la façade complète qu'à la fin
- utiliser le firmware existant comme outil de diagnostic et de test

Ce document complète :

- [DIX_PAS_ALIMENTATION_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/engineering/DIX_PAS_ALIMENTATION_REFERENCE.md:1)
- [DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/engineering/DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md:1)
- [DIX_PAS_CABLAGE_REFERENCE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_CABLAGE_REFERENCE.md:1)
- [DIX_PAS_BRING_UP_HARDWARE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_BRING_UP_HARDWARE.md:1)

---

## 2. Règle générale

Ne pas commencer par câbler toute la machine.

L'ordre recommandé est :

1. alimentation
2. MCU
3. bus `I2C`
4. `MIDI OUT`
5. `Gate Out`
6. `MIDI IN`
7. encodeur
8. chaîne boutons `74HC165`
9. chaîne LEDs `74HC595`
10. intégration complète

Critère de passage :

- on ne passe à l'étape suivante que si l'étape courante est **électriquement stable** et **fonctionnellement validée**

---

## 3. Matériel minimal de proto

Prévoir pour la phase breadboard :

- `1` breadboard principale
- `1` breadboard secondaire si besoin
- fils Dupont de bonne qualité
- `1` multimètre
- `1` alimentation `9 V DC` centre positif
- `1` buck `5 V`
- `1` Arduino Nano Every
- `1` écran OLED SSD1306
- `1` FRAM I2C
- `1` connecteur `MIDI OUT`
- `1` connecteur `MIDI IN`
- `2` jacks mono pour `Gate Out`
- `1` optocoupleur `6N138`
- `1` buffer `74HCS125` ou `74HC125`
- quelques résistances et condensateurs selon la référence d'ingénierie
- `1` encodeur avec poussoir
- au moins `1` `74HC165`
- au moins `1` `74HC595`
- quelques boutons et LEDs pour validation partielle

Ne pas attendre d'avoir les `30` boutons montés pour commencer.

---

## 4. Étape 1 - Alimentation seule

Monter uniquement :

- entrée `9 V`
- interrupteur
- buck `5 V`
- rail `5 V`
- rail `GND`

À vérifier :

- tension entrée correcte
- sortie buck stable à `5 V`
- polarité correcte
- aucune chauffe anormale

Critère de validation :

- rail `5 V` stable et propre au multimètre

---

## 5. Étape 2 - Nano Every seul

Ajouter :

- `Arduino Nano Every`
- `5 V`
- `GND`

À vérifier :

- la carte s'alimente correctement
- le flash firmware fonctionne
- la carte boote sans comportement anormal

Critère de validation :

- firmware flashé avec succès
- pas de chauffe ou de reset aléatoire

---

## 6. Étape 3 - Bus I2C

Ajouter :

- OLED SSD1306
- FRAM I2C
- liaisons `A4 / SDA` et `A5 / SCL`

À vérifier :

- splash boot visible
- écran principal visible
- FRAM détectée au démarrage
- sauvegarde / chargement de preset fonctionnels

Outils firmware utiles :

- écran boot
- overlay de statut
- diagnostic hardware

Critère de validation :

- OLED et FRAM fonctionnent ensemble sur le même bus `I2C`

---

## 7. Étape 4 - MIDI OUT

Ajouter :

- connecteur `DIN OUT`
- buffer `74HCS125` ou `74HC125`
- `2 x 220 Ω`

À vérifier :

- émission de notes test
- émission de clock interne
- comportement stable avec un vrai câble MIDI

Outil firmware utile :

- mode test `MIDI / Gate`

Critère de validation :

- une machine externe reçoit clairement les notes de test

---

## 8. Étape 5 - Gate Out A / B

Ajouter :

- les `2` sorties jack
- les voies buffer `Gate Out` sur le même `74HCS125` ou `74HC125`
- `2 x 220 Ω`
- `2 x 100 kΩ`

À vérifier :

- `Gate A` pulse correctement
- `Gate B` pulse correctement
- niveau haut mesuré proche de `5 V`
- retour à l'état bas correct

Outil firmware utile :

- mode test `MIDI / Gate`

Critère de validation :

- les deux gates sont propres et stables

---

## 9. Étape 6 - MIDI IN

Ajouter :

- connecteur `DIN IN`
- `6N138`
- `220 Ω`
- `10 kΩ`
- `4.7 kΩ`
- `1N4148`
- découplage dédié

À vérifier :

- réception `Clock`
- réception `Start`
- réception `Stop`
- réception `Continue`
- affichage du dernier événement `MIDI IN` dans le diagnostic

Outil firmware utile :

- mode diagnostic

Critère de validation :

- les événements temps réel MIDI sont vus sans faux déclenchements

---

## 10. Étape 7 - Encodeur

Ajouter :

- encodeur A -> `D2`
- encodeur B -> `D3`
- poussoir encodeur -> `D4`

À vérifier :

- navigation UI correcte
- rotation sans sauts incohérents
- bouton encodeur reconnu

Critère de validation :

- l'UI répond proprement à l'encodeur

---

## 11. Étape 8 - Première lecture de boutons

Ajouter :

- `1 x 74HC165`
- quelques boutons seulement
- câblage selon l'ordre firmware

À vérifier :

- lecture stable
- pas de ghosting
- mapping logique correct

Critère de validation :

- les boutons testés remontent correctement dans le diagnostic ou l'UI

---

## 12. Étape 9 - Première sortie LEDs

Ajouter :

- `1 x 74HC595`
- quelques LEDs seulement
- résistances série

À vérifier :

- LEDs système
- LEDs de test liées aux actions UI
- ordre de bits conforme

Critère de validation :

- les LEDs attendues s'allument au bon moment

---

## 13. Étape 10 - Chaînes complètes façade

Ajouter ensuite :

- les `5 x 74HC165`
- les `3 x 74HC595`
- toutes les rangées de boutons
- toutes les LEDs
- boutons système

À vérifier :

- mapping complet
- absence de décalage de bits
- comportement stable pendant plusieurs minutes

Critère de validation :

- la façade entière correspond au firmware sans remapping surprise

---

## 14. Étape 11 - Intégration complète

Quand les étapes `1` à `10` sont validées :

- rassembler le montage complet
- refaire le flash firmware
- suivre [DIX_PAS_BRING_UP_HARDWARE.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/setup/DIX_PAS_BRING_UP_HARDWARE.md:1)
- exécuter [DIX_PAS_TESTS_ACCEPTATION_V1.md](/Users/jdebaeck/Documents/Développements/Dix pas/docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md:1)

À ce stade, on ne doit plus être dans la conception.

On doit être dans :

- l'assemblage
- le test
- la correction de câblage
- le debug réel

---

## 15. Règles de sécurité projet

- ne jamais brancher `MIDI IN`, `MIDI OUT`, `Gate`, OLED, FRAM et la façade complète d'un seul coup si les rails ne sont pas validés
- ne jamais ajouter une nouvelle chaîne de registres tant que l'étape précédente n'est pas stable
- toujours mesurer le `5 V` avant le premier flash sur un nouveau montage
- toujours tester `MIDI OUT` et `Gate Out` avec le mode test dédié avant de lancer le séquenceur complet

---

## 16. Définition du vrai stop

Le vrai stop avant achat n'est pas encore atteint tant que :

- la BOM d'achat n'est pas gelée
- les références vraiment achetables ne sont pas listées proprement

Le vrai stop avant intégration firmware sur hardware arrivera quand :

- tout ce document sera prêt
- la BOM sera gelée
- et les checklists de câblage / bring-up seront cohérentes entre elles
