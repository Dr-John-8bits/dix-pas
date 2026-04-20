# DIX PAS - Setup Mac

Note documentaire :

- ce document decrit l'environnement recommande pour travailler sur macOS
- en cas de conflit de scope produit, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte

## 1. Objet

Ce document explique comment preparer un Mac pour :

- travailler sur la documentation
- developper le firmware
- tester une grande partie de la logique sans hardware

Le but est d'avancer fort sur le projet avant achat du materiel.

---

## 2. Reponse courte

Non, il n'est pas necessaire d'acheter l'Arduino tout de suite pour commencer serieusement.

On peut avancer sans hardware sur :

- la logique du sequencer
- les presets
- les modes machine
- le moteur MIDI logique
- plus tard, Euclidean et generatif

Le hardware sera necessaire plus tard pour :

- le vrai MIDI DIN electrique
- les Gate Out reels
- l'ecran OLED reel
- les boutons, LEDs et encodeur
- l'upload et la validation sur la carte cible

---

## 3. Logiciels a installer

### 3.1 Requis

Installer :

- **Git**
- **Visual Studio Code**
- **PlatformIO IDE** dans VS Code

Si macOS le demande pendant l'installation des outils de build :

- installer les **Command Line Tools** d'Apple

### 3.2 Optionnel

Optionnel mais utile :

- **Arduino CLI**

Il n'est pas obligatoire si on travaille avec PlatformIO.

---

## 4. Outil principal recommande

L'outil principal recommande est :

- **PlatformIO dans VS Code**

Pourquoi :

- simple a prendre en main
- bon support Arduino
- build cible + build desktop possibles
- pratique pour separer logique pure et couche hardware

Important :

- si on utilise PlatformIO IDE dans VS Code, il n'est pas necessaire d'installer PlatformIO Core separement

---

## 5. Strategie de developpement recommandee

Le projet devrait etre structure pour supporter deux environnements :

- un environnement **desktop / native**
- un environnement **Nano Every**

### 5.1 Environnement native

L'environnement `native` sert a :

- compiler le coeur logique sur le Mac
- lancer des tests rapides
- valider les presets
- valider le moteur de sequence
- valider les generateurs futurs

### 5.2 Environnement Nano Every

L'environnement `nanoevery` sert a :

- compiler le firmware cible reel
- uploader plus tard sur la carte
- valider l'integration hardware

---

## 6. Tests MIDI sans hardware

Sur Mac, on peut tester une partie de la logique MIDI sans Arduino physique.

La voie recommandee est d'utiliser :

- **Audio MIDI Setup**
- le **Driver IAC**

Le Driver IAC permet d'echanger du MIDI entre apps sur le Mac.

Usage type :

- l'app de test ou le simulateur envoie des evenements MIDI vers un bus IAC
- une autre app MIDI peut lire ce flux

Cela permet de valider :

- ordre des notes
- canaux MIDI
- comportement de clock logique
- transport

---

## 7. Mise en place IAC

### 7.1 Activation

Sur macOS :

1. ouvrir `Audio MIDI Setup`
2. ouvrir `Window > Show MIDI Studio`
3. double-cliquer `IAC Driver`
4. cocher `Device is online`
5. creer au moins un bus, par exemple `DIX_PAS_TEST`

### 7.2 Usage

Ensuite :

- le simulateur ou outil de test envoie vers `DIX_PAS_TEST`
- une autre app MIDI peut le lire

---

## 8. Ce qu'on peut tester sans hardware

Avant achat du materiel, on peut valider :

- structure des pistes
- lecture `Dual`
- lecture `Chain 20`
- modes de lecture
- probabilite
- ratchet logique
- gate length logique
- velocity
- transport
- presets
- structure future Euclidean
- structure future generative

On peut egalement produire :

- des logs d'evenements MIDI
- des tests unitaires
- un simulateur simple en console

---

## 9. Ce qu'on ne peut pas tester sans hardware

Sans carte ni montage, on ne pourra pas valider :

- niveau reel du `MIDI DIN OUT`
- reception electrique du `MIDI DIN IN`
- niveau reel de `Gate Out A / B`
- comportement electrique des buffers
- bruit mecanique de l'encodeur
- debounce reel
- visibilite reelle de l'ecran
- lisibilite du panneau final

---

## 10. Workflow recommande

Ordre recommande :

1. consolider la doc
2. creer le repo
3. mettre en place PlatformIO
4. coder le coeur logique testable sur Mac
5. ajouter un simulateur ou test harness
6. seulement ensuite acheter et integrer le hardware

---

## 11. Structure de code recommande

Pour travailler proprement sans hardware au debut :

- `core/` : logique pure du sequencer
- `platform/desktop/` : tests et simulation Mac
- `platform/nanoevery/` : adaptation Arduino

La logique pure ne doit pas dependre directement :

- des pins Arduino
- de `digitalWrite`
- de `Serial`
- de l'ecran reel

Elle doit plutot consommer des interfaces abstraites.

---

## 12. Installation minimale recommandee

Checklist simple :

1. installer `VS Code`
2. installer l'extension `PlatformIO IDE`
3. verifier que `git --version` fonctionne
4. activer un bus IAC dans `Audio MIDI Setup`
5. plus tard seulement, brancher le Nano Every

---

## 13. References officielles utiles

- [PlatformIO IDE for VSCode](https://docs.platformio.org/en/latest/integration/ide/vscode.html)
- [PlatformIO installation](https://docs.platformio.org/en/latest/core/installation/index.html)
- [Arduino CLI](https://docs.arduino.cc/arduino-cli/)
- [Arduino CLI getting started](https://docs.arduino.cc/arduino-cli/getting-started)
- [Audio MIDI Setup User Guide](https://support.apple.com/guide/audio-midi-setup/welcome/mac)
- [Transfer MIDI information between apps with IAC Driver](https://support.apple.com/es-us/guide/audio-midi-setup/ams1013/mac)
- [Test your MIDI setup](https://support.apple.com/en-us/102047)
