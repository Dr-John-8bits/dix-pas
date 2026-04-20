# DIX PAS - Bring-Up Hardware

Ce document décrit l'ordre recommandé pour allumer, flasher et valider la première machine réelle.

Objectif :

- aller du plus simple au plus risqué
- isoler rapidement les pannes
- éviter de déboguer plusieurs sous-systèmes en même temps

## 1. Pré-requis

Avant le premier allumage, préparer :

- le repo à jour
- `PlatformIO` fonctionnel sur le Mac
- un câble USB pour flasher la carte de développement
- un multimètre
- au moins un câble `MIDI DIN 5 broches`
- idéalement un synthé ou une interface MIDI DIN pour écouter `MIDI OUT`
- idéalement une source de clock ou de transport MIDI DIN pour tester `MIDI IN`

Outils utiles mais non obligatoires :

- une alimentation de labo
- une LED de test avec résistance
- un oscilloscope ou analyseur logique

## 2. Avant mise sous tension

Vérifier visuellement :

- orientation du `Nano Every`
- orientation de l'`OLED`
- orientation de la `FRAM`
- orientation des `74HC165`
- orientation des `74HC595`
- polarité de l'alimentation
- masse commune cohérente
- absence de court-circuit visible

Ne brancher aucun instrument externe tant que :

- l'alimentation n'est pas vérifiée
- les sorties `Gate` ne sont pas mesurées

## 3. Flash du firmware

Depuis le dossier `firmware/` :

```bash
export PLATFORMIO_CORE_DIR="$PWD/.pio-core"
pio run -e nanoevery
pio run -e nanoevery -t upload
```

Si l'upload échoue :

- vérifier le port série
- vérifier le câble USB
- vérifier que le `Nano Every` est bien alimenté

## 4. Premier boot attendu

Au démarrage, l'écran doit afficher brièvement :

- `DIX PAS`
- `by Dr. John`
- la version firmware au format `aa.mm.jj.itération`

Puis la machine revient sur l'écran principal ou un message de démarrage.

Comportements attendus :

- pas d'auto-play
- `Gate A` bas
- `Gate B` bas
- LEDs cohérentes avec l'état `STOP`

Si l'écran reste noir :

- vérifier `OLED`
- vérifier le bus `I2C`
- vérifier l'adresse réelle de l'écran
- vérifier l'alimentation `3.3 V / 5 V` selon le module retenu

## 5. Mode diagnostic

Entrée :

- `SHIFT + RESET`

Objectif :

- vérifier rapidement que la façade répond
- confirmer la présence `FRAM / OLED`
- voir le dernier événement `MIDI IN`

Lecture attendue :

- ligne 1 : `Diagnostic`
- ligne 2 : état `FRAM / OLED`
- ligne 3 : dernier événement d'entrée façade
- ligne 4 : dernier événement `MIDI IN`, transport et source d'horloge

Tests à faire :

- appuyer sur plusieurs boutons de steps
- tourner l'encodeur
- appuyer sur `MODE`
- appuyer sur `PLAY`, `STOP`, `RESET`

Résultat attendu :

- chaque action change la ligne d'événement

Si une touche semble décalée :

- vérifier l'ordre des bits `74HC165`
- vérifier le mapping firmware / câblage
- vérifier les niveaux `actif bas / actif haut`

## 6. Mode test MIDI / Gate

Entrée :

- `SHIFT + PLAY`

Commandes :

- `MODE` : fait tourner `Both`, `MIDI`, `Gate`
- `STOP` : force toutes les sorties test à l'état bas
- `RESET` : relance le cycle depuis `A On`
- `SHIFT + PLAY` : quitte le mode test

Cycle de test :

- `A On`
- `A Off`
- `B On`
- `B Off`

Le cycle avance automatiquement toutes les `500 ms`.

### 6.1 MIDI OUT

En mode `Both` ou `MIDI`, le firmware envoie :

- piste A : note de test `C3`
- piste B : note de test `C4`
- canal MIDI courant de chaque piste

Sur une machine par défaut :

- piste A sur canal `1`
- piste B sur canal `2`

Validation attendue :

- le récepteur MIDI voit les notes arriver
- l'écran affiche une ligne du type `A On Ch01 C3`
- puis `A Off`, puis la même chose pour `B`

Si rien ne sort :

- vérifier le câblage `DIN OUT`
- vérifier la masse et les résistances de sortie
- vérifier que le récepteur écoute le bon canal

### 6.2 Gate OUT

En mode `Both` ou `Gate`, les sorties :

- `Gate A`
- `Gate B`

doivent commuter en `5 V` actif haut.

Validation attendue :

- au multimètre, la sortie monte puis redescend
- l'écran affiche `GA:Hi` ou `GB:Hi` selon la phase

Si la gate ne monte pas :

- vérifier le niveau de sortie réel
- vérifier le buffer ou transistor éventuel
- vérifier la référence de masse

## 7. Test MIDI IN

Le test `MIDI IN` se fait depuis le mode diagnostic.

Entrée :

- `SHIFT + RESET`

Pour valider ensuite la vraie synchro externe du séquenceur :

1. passer en `Global Edit`
2. aller sur `Clock`
3. choisir `External`

Envoyer ensuite depuis une machine externe :

- `Clock`
- `Start`
- `Continue`
- `Stop`

Validation attendue :

- la dernière ligne du diagnostic change vers `In:Clock`, `In:Start`, `In:Continue`, `In:Stop`

Important :

- cette validation ne dépend pas du séquenceur normal
- même si l'horloge externe n'est pas encore utilisée en jeu, la réception doit déjà être visible

Si rien n'est reçu :

- vérifier l'entrée `DIN IN`
- vérifier l'optocoupleur
- vérifier le sens de câblage
- vérifier la vitesse MIDI standard `31250 bauds`

## 8. Test FRAM

Procédure :

1. démarrer la machine
2. charger ou créer un pattern
3. sauver un preset
4. couper l'alimentation
5. rallumer

Validation attendue :

- la machine recharge le dernier preset connu
- aucun message d'erreur `Preset Error`

Si la FRAM est absente ou mal câblée :

- la machine doit continuer à booter
- le diagnostic doit indiquer l'absence de stockage

## 9. Ordre de validation conseillé

Ordre recommandé sur la vraie machine :

1. alimentation
2. boot OLED
3. boutons / encodeur via diagnostic
4. LEDs
5. `MIDI OUT`
6. `Gate Out`
7. `MIDI IN`
8. `FRAM`
9. séquenceur complet

## 10. Symptômes fréquents

Pas d'écran mais carte flashable :

- problème `OLED`
- problème `I2C`

Boutons incohérents :

- ordre des bits `74HC165`
- inversion actif bas / actif haut

LEDs incohérentes :

- ordre des bits `74HC595`
- inversion logique

MIDI OUT muet :

- problème de câblage `DIN OUT`
- canal MIDI mal compris côté récepteur

MIDI IN muet :

- problème d'optocoupleur
- problème de câblage de l'entrée DIN

Gate présente à l'écran mais pas en mesure :

- problème d'étage de sortie
- problème de jack ou de masse

## 11. Critère de passage

On peut considérer le bring-up de base réussi quand :

- la machine boote
- l'écran affiche correctement
- les boutons et l'encodeur sont vus par le diagnostic
- `MIDI OUT` est validé
- `Gate A/B` sont validés
- `MIDI IN` est visible dans le diagnostic
- la `FRAM` lit et écrit correctement

À ce moment-là, on peut passer sereinement au débogage d'ergonomie et au séquenceur complet en conditions réelles.
