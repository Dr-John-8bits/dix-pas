# DIX PAS

**DIX PAS** est un step sequencer MIDI de performance open source, pensé comme un vrai instrument hardware.

Direction actuelle :

- 2 pistes de 10 pas
- mode `Chain 20`
- `MIDI DIN 5 broches`
- `2 Gate Out`
- écran `OLED I2C 128x64`
- `Arduino Nano Every`

## Arborescence

```text
.
├── assets/
│   └── branding/
├── bom/
├── docs/
│   ├── engineering/
│   ├── product/
│   └── setup/
├── firmware/
│   ├── include/
│   ├── lib/
│   ├── sim/
│   ├── src/
│   └── test/
├── hardware/
│   ├── enclosure/
│   ├── panel/
│   ├── pcb/
│   └── schematics/
├── LICENSE
├── LICENSE-CC0.txt
├── LICENSING.md
└── README.md
```

## Documentation

Point d'entrée recommandé :

1. [docs/product/DIX_PAS_SOURCE_DE_VERITE.md](docs/product/DIX_PAS_SOURCE_DE_VERITE.md)
2. [docs/product/DIX_PAS_DECISIONS.md](docs/product/DIX_PAS_DECISIONS.md)
3. [LICENSING.md](LICENSING.md)
4. [docs/setup/DIX_PAS_SETUP_MAC.md](docs/setup/DIX_PAS_SETUP_MAC.md)
5. [docs/setup/DIX_PAS_CABLAGE_REFERENCE.md](docs/setup/DIX_PAS_CABLAGE_REFERENCE.md)
6. [docs/setup/DIX_PAS_BRING_UP_HARDWARE.md](docs/setup/DIX_PAS_BRING_UP_HARDWARE.md)
7. [docs/setup/DIX_PAS_PLAN_PROTO_BREADBOARD.md](docs/setup/DIX_PAS_PLAN_PROTO_BREADBOARD.md)
8. [docs/setup/DIX_PAS_EXECUTION_STEP_BY_STEP.md](docs/setup/DIX_PAS_EXECUTION_STEP_BY_STEP.md)
9. [docs/setup/DIX_PAS_ACHATS_PAR_PHASE.md](docs/setup/DIX_PAS_ACHATS_PAR_PHASE.md)
10. [docs/engineering/DIX_PAS_document_technique.md](docs/engineering/DIX_PAS_document_technique.md)
11. [docs/engineering/DIX_PAS_ALIMENTATION_REFERENCE.md](docs/engineering/DIX_PAS_ALIMENTATION_REFERENCE.md)
12. [docs/engineering/DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md](docs/engineering/DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md)
13. [docs/engineering/DIX_PAS_AUDIT_PRE_ACHAT_2026-04-20.md](docs/engineering/DIX_PAS_AUDIT_PRE_ACHAT_2026-04-20.md)
14. [docs/engineering/DIX_PAS_AUDIT_LOGICIEL_PRE_MATERIEL_2026-04-20.md](docs/engineering/DIX_PAS_AUDIT_LOGICIEL_PRE_MATERIEL_2026-04-20.md)
15. [docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md](docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md)
16. [docs/engineering/DIX_PAS_UI_STATE_MACHINE.md](docs/engineering/DIX_PAS_UI_STATE_MACHINE.md)
17. [docs/engineering/DIX_PAS_PRESET_FORMAT.md](docs/engineering/DIX_PAS_PRESET_FORMAT.md)
18. [bom/DIX_PAS_BOM_v0.md](bom/DIX_PAS_BOM_v0.md)
19. [bom/DIX_PAS_BOM_ACHAT_COEUR_V1.md](bom/DIX_PAS_BOM_ACHAT_COEUR_V1.md)
20. [docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md](docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md)

Documents annexes :

- [docs/product/DIX_PAS_specs_fonctionnelles.md](docs/product/DIX_PAS_specs_fonctionnelles.md)
- [docs/README.md](docs/README.md)

Règle simple :

- le `SOURCE_DE_VERITE` tranche
- le `DOCUMENT_TECHNIQUE` explique comment réaliser
- les `TESTS_ACCEPTATION_V1` disent quand la V1 est validée

## Lecture avant achat

Si ton objectif est de savoir **quoi acheter avant de lancer les commandes**, l'ordre de lecture recommandé est :

1. [docs/product/DIX_PAS_SOURCE_DE_VERITE.md](docs/product/DIX_PAS_SOURCE_DE_VERITE.md)
2. [docs/product/DIX_PAS_DECISIONS.md](docs/product/DIX_PAS_DECISIONS.md)
3. [docs/engineering/DIX_PAS_AUDIT_LOGICIEL_PRE_MATERIEL_2026-04-20.md](docs/engineering/DIX_PAS_AUDIT_LOGICIEL_PRE_MATERIEL_2026-04-20.md)
4. [docs/engineering/DIX_PAS_document_technique.md](docs/engineering/DIX_PAS_document_technique.md)
5. [docs/engineering/DIX_PAS_ALIMENTATION_REFERENCE.md](docs/engineering/DIX_PAS_ALIMENTATION_REFERENCE.md)
6. [docs/engineering/DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md](docs/engineering/DIX_PAS_INTERFACES_MIDI_GATE_REFERENCE.md)
7. [docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md](docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md)
8. [bom/DIX_PAS_BOM_v0.md](bom/DIX_PAS_BOM_v0.md)
9. [bom/DIX_PAS_BOM_ACHAT_COEUR_V1.md](bom/DIX_PAS_BOM_ACHAT_COEUR_V1.md)
10. [docs/setup/DIX_PAS_ACHATS_PAR_PHASE.md](docs/setup/DIX_PAS_ACHATS_PAR_PHASE.md)

Après ça, tu auras la base la plus utile pour préparer l'achat sans rentrer trop tôt dans les détails d'assemblage.

## Sans hardware

On peut avancer sérieusement sans acheter l'Arduino tout de suite.

On peut d'abord construire :

- la documentation
- le cœur logique du séquenceur
- les presets
- les tests desktop
- les tests MIDI logiques sur Mac

## Licences

Le projet est publié en mode très ouvert, sans obligation de crédit.

- code, firmware, scripts et outillage : `0BSD` dans [LICENSE](LICENSE)
- documentation, BOM, hardware, images et assets : `CC0 1.0` dans [LICENSE-CC0.txt](LICENSE-CC0.txt)

Le détail est documenté dans [LICENSING.md](LICENSING.md).

## Démarrage rapide

1. lire [docs/product/DIX_PAS_SOURCE_DE_VERITE.md](docs/product/DIX_PAS_SOURCE_DE_VERITE.md)
2. lire [docs/engineering/DIX_PAS_AUDIT_LOGICIEL_PRE_MATERIEL_2026-04-20.md](docs/engineering/DIX_PAS_AUDIT_LOGICIEL_PRE_MATERIEL_2026-04-20.md)
3. lire [docs/setup/DIX_PAS_ACHATS_PAR_PHASE.md](docs/setup/DIX_PAS_ACHATS_PAR_PHASE.md)
4. lire [docs/setup/DIX_PAS_EXECUTION_STEP_BY_STEP.md](docs/setup/DIX_PAS_EXECUTION_STEP_BY_STEP.md)
5. commander le cœur électronique validé
6. suivre le bring-up dans [docs/setup/DIX_PAS_BRING_UP_HARDWARE.md](docs/setup/DIX_PAS_BRING_UP_HARDWARE.md)
7. passer la checklist de [docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md](docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md)
