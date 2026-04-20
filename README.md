# DIX PAS

**DIX PAS** est un step sequencer MIDI de performance open source, pense comme un vrai instrument hardware.

Direction actuelle :

- 2 pistes de 10 pas
- mode `Chain 20`
- `MIDI DIN 5 broches`
- `2 Gate Out`
- ecran `OLED I2C 128x64`
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
├── notes/
├── LICENSE
├── LICENSE-CC0.txt
├── LICENSING.md
└── README.md
```

## Documentation

Point d'entree recommande :

1. [docs/product/DIX_PAS_SOURCE_DE_VERITE.md](docs/product/DIX_PAS_SOURCE_DE_VERITE.md)
2. [docs/product/DIX_PAS_DECISIONS.md](docs/product/DIX_PAS_DECISIONS.md)
3. [LICENSING.md](LICENSING.md)
4. [docs/setup/DIX_PAS_SETUP_MAC.md](docs/setup/DIX_PAS_SETUP_MAC.md)
5. [docs/engineering/DIX_PAS_document_technique.md](docs/engineering/DIX_PAS_document_technique.md)
6. [docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md](docs/engineering/DIX_PAS_PIN_MAP_DRAFT.md)
7. [docs/engineering/DIX_PAS_UI_STATE_MACHINE.md](docs/engineering/DIX_PAS_UI_STATE_MACHINE.md)
8. [docs/engineering/DIX_PAS_PRESET_FORMAT.md](docs/engineering/DIX_PAS_PRESET_FORMAT.md)
9. [bom/DIX_PAS_BOM_v0.md](bom/DIX_PAS_BOM_v0.md)
10. [docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md](docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md)

Documents annexes :

- [docs/product/DIX_PAS_specs_fonctionnelles.md](docs/product/DIX_PAS_specs_fonctionnelles.md)
- [notes/idee.txt](notes/idee.txt)
- [docs/README.md](docs/README.md)

Regle simple :

- le `SOURCE_DE_VERITE` tranche
- le `DOCUMENT_TECHNIQUE` explique comment realiser
- les `TESTS_ACCEPTATION_V1` disent quand la V1 est validee

## Sans hardware

On peut avancer serieusement sans acheter l'Arduino tout de suite.

On peut d'abord construire :

- la documentation
- le coeur logique du sequenceur
- les presets
- les tests desktop
- les tests MIDI logiques sur Mac

## Licences

Le projet est publie en mode tres ouvert, sans obligation de credit.

- code, firmware, scripts et outillage : `0BSD` dans [LICENSE](LICENSE)
- documentation, BOM, hardware, images et assets : `CC0 1.0` dans [LICENSE-CC0.txt](LICENSE-CC0.txt)

Le detail est documente dans [LICENSING.md](LICENSING.md).

## Demarrage rapide

1. lire [docs/product/DIX_PAS_SOURCE_DE_VERITE.md](docs/product/DIX_PAS_SOURCE_DE_VERITE.md)
2. lire [docs/setup/DIX_PAS_SETUP_MAC.md](docs/setup/DIX_PAS_SETUP_MAC.md)
3. valider le hardware critique a acheter
4. implementer le coeur de sequence dans `firmware/`
5. integrer `MIDI OUT`, `Gate Out` et UI
6. passer la checklist de [docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md](docs/product/DIX_PAS_TESTS_ACCEPTATION_V1.md)
