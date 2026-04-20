# DIX PAS - Firmware

Ce dossier accueillera le firmware principal et les outils de simulation desktop.

Structure retenue :

- `src/` : point d'entree firmware
- `include/` : headers projet
- `lib/` : modules internes reutilisables
- `test/` : tests unitaires ou d'integration
- `sim/` : simulateur desktop et outils Mac sans hardware

Objectif de depart :

- isoler le coeur du sequenceur du materiel
- permettre des tests logiques sur Mac avant l'integration Arduino
