# DIX PAS - Preset Format

Note documentaire :

- ce document figé la structure logique des presets et leur stockage recommandé
- en cas de conflit de scope produit, `docs/product/DIX_PAS_SOURCE_DE_VERITE.md` l'emporte

## 1. Objet

Ce document definit :

- ce qu'un preset contient
- ce qu'il ne contient pas
- comment il est stocke
- comment il evolue dans le temps

Le but est d'éviter une logique de sauvegarde improvis ee au moment du firmware.

---

## 2. Regles produit

Les règles déjà retenues sont :

- la V1 supporte `8 presets minimum`
- la sauvegarde est `explicite`
- le stockage se fait sur `FRAM externe`

Un preset doit rappeler le contenu musical et les paramètres de jeu.

Il ne doit pas sauver des états transitoires non utiles.

---

## 3. Ce qu'un preset contient

Chaque preset contient :

- contenu piste A
- contenu piste B
- longueurs de piste
- canaux MIDI
- mode machine `DUAL` ou `CHAIN20`
- mode de lecture
- tempo
- root
- scale
- offsets d'octave

Chaque pas contient :

- active / inactive
- degree
- probability
- ratchet
- gate
- velocity

---

## 4. Ce qu'un preset ne contient pas

Un preset ne contient pas :

- état transport `PLAYING / STOPPED`
- position de lecture courante
- état courant des Gates
- état courant des notes tenues
- état temporaire de l'UI
- overlays écran
- état momentane de `SHIFT`

Ces informations sont runtime, pas preset.

---

## 5. Modele logique recommandé

```cpp
struct StepV1 {
  uint8_t flags;
  uint8_t degree;
  uint8_t probability;
  uint8_t ratchet;
  uint8_t gate;
  uint8_t velocity;
};

struct TrackV1 {
  StepV1 steps[10];
  uint8_t length;
  uint8_t midiChannel;
  int8_t octaveOffset;
  uint8_t reserved;
};

struct PresetPayloadV1 {
  TrackV1 trackA;
  TrackV1 trackB;
  uint8_t machineMode;
  uint8_t playMode;
  uint16_t tempoBpm_x10;
  uint8_t rootNote;
  uint8_t scaleId;
  uint8_t reserved[8];
};
```

Convention sur `flags` :

- bit 0 = step active
- bits 1..7 = reserves

---

## 6. Enveloppe de stockage recommandée

Pour rendre le stockage robuste, chaque preset doit être stocke dans un slot fixe avec un header.

Format recommandé :

```cpp
struct PresetSlotHeaderV1 {
  char magic[4];          // "DXP1"
  uint8_t formatVersion;  // 1
  uint8_t slotIndex;      // 0..7
  uint16_t payloadSize;
  uint16_t crc16;
  uint8_t flags;
  uint8_t reserved[5];
};
```

Puis :

- `PresetSlotHeaderV1`
- `PresetPayloadV1`

Avantages :

- validation simple
- detection de donnees corrompues
- évolution de format plus facile

---

## 7. Stratégie FRAM recommandée

La FRAM retenue donne largement assez de marge.

Stratégie simple recommandée :

- reserver un bloc pour metadonnees globales
- reserver ensuite `8 slots` de taille fixe

Plan d'adressage recommandé :

- `0x0000 - 0x00FF` : metadata globale
- `0x0100 - 0x01FF` : preset 0
- `0x0200 - 0x02FF` : preset 1
- `0x0300 - 0x03FF` : preset 2
- `0x0400 - 0x04FF` : preset 3
- `0x0500 - 0x05FF` : preset 4
- `0x0600 - 0x06FF` : preset 5
- `0x0700 - 0x07FF` : preset 6
- `0x0800 - 0x08FF` : preset 7

Taille par slot recommandée :

- `256 bytes`

Pourquoi :

- adressage simple
- évolution facile
- lecture / ecriture directe

---

## 8. Metadata globale recommandée

La metadata globale peut contenir :

- version de stockage
- dernier slot charge
- dernier slot sauvegarde
- flags de validite

Exemple :

```cpp
struct StorageMetadataV1 {
  char magic[4];           // "DXPM"
  uint8_t storageVersion;  // 1
  uint8_t lastLoadedSlot;
  uint8_t lastSavedSlot;
  uint8_t flags;
  uint8_t reserved[8];
};
```

Cette zone n'est pas obligatoire pour la V1 minimale, mais elle est recommandée.

---

## 9. Comportement de sauvegarde recommandé

### 9.1 Sauvegarde

La sauvegarde est :

- explicite

Flux recommandé :

1. l'utilisateur choisit un slot
2. la machine construit le payload
3. la machine calcule le `crc16`
4. la machine ecrit `header + payload`
5. la machine confirme visuellement

### 9.2 Chargement

Flux recommandé :

1. l'utilisateur choisit un slot
2. la machine lit le header
3. elle verifie `magic`, `version`, `payloadSize`, `crc16`
4. si valide, elle charge
5. sinon, elle affiche une erreur simple

---

## 10. Versionnement

Le format doit pouvoir evoluer.

Regles recommandées :

- commencer a `formatVersion = 1`
- ne jamais reinterpre ter silencieusement un vieux slot sans verification
- si le format change, incrementer la version
- ajouter des champs en fin de payload quand possible

Si un slot d'une ancienne version est charge :

- soit migrer explicitement
- soit refuser proprement

---

## 11. Valeurs par défaut recommandées

Si aucun preset valide n'est trouve :

- machine mode = `DUAL`
- play mode = `FORWARD`
- tempo = `120.0 BPM`
- root = `C`
- scale = `minor` ou preset projet par défaut
- piste A et B = steps vides ou pattern simple de test

---

## 12. Consequences firmware

Pour coder vite, il faut separer :

- `ProjectState` runtime
- `PresetPayloadV1` serialisable

Le firmware doit fournir :

- `serializePreset()`
- `deserializePreset()`
- `validatePreset()`
- `loadPreset(slot)`
- `savePreset(slot)`

---

## 13. Décision pratique

Pour la V1, la bonne decision est :

- format binaire fixe
- slots fixes
- header + payload + crc

Il ne faut pas partir sur :

- JSON en firmware
- structures variables
- sauvegarde implicite permanente

Le binaire fixe sera plus rapide, plus propre et plus fiable.
