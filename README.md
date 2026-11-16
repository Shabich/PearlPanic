# Pearl Panic

Jeu de gestion de bubble tea pour **Nintendo DS**, écrit en C. Pixel art pastel,
assemblage des boissons au stylet, clients mignons mais très impatients, recettes
à découvrir.

Le découpage du projet en sous-tâches est dans [`TASKS.md`](TASKS.md).

## Construire et jouer

| Commande | Effet |
|---|---|
| `make` | compile `pearlpanic.nds` (toolchain BlocksDS dans Docker si elle n'est pas installée) |
| `make run` | lance la ROM dans melonDS (`~/Applications` ou `/Applications`) |
| `make test` | tests unitaires de la logique, compilés pour le PC |
| `make snap` | joue un scénario scripté sur PC et écrit des captures dans `build/snap/` |
| `make assets` | régénère les graphismes placeholder et leur conversion en C |
| `make clean` | supprime les artefacts |

Prérequis : Docker (ou BlocksDS installé avec `BLOCKSDS` défini), un compilateur C
pour le PC, Python 3 + Pillow pour les outils.

## Comment on joue

- **Écran du haut** : les clients au comptoir, leur commande et leur jauge de patience,
  l'ambiance de la boutique (cœur) et le ticket de la commande sélectionnée.
- **Écran du bas (stylet)** :
  - maintenir une **tireuse** pour verser (thé, lait) : les traits du gobelet = 10 % ;
  - toucher une **pompe** pour une dose de sirop ;
  - glisser (ou toucher) un **bac** pour ajouter perles, gelée, glaçons, crème… ;
  - **Fermer** scelle le gobelet ; frotter le gobelet de gauche à droite le **secoue** ;
  - **Servir** (ou A) donne la boisson au client choisi avec `<` `>` (ou L/R).
- Les commandes « ??? » sont des **énigmes** : réussir la boisson débloque la recette.
  Certaines recettes secrètes ne se trouvent qu'en expérimentant.
- Chaque journée a un **objectif** (servir N clients, boissons parfaites…) qui rapporte des pièces.
- **Mode Rush** (après le jour 1) : pas d'horloge, les clients arrivent de plus en plus vite,
  la partie s'arrête quand l'ambiance tombe à zéro. Le record est sauvegardé.
- START : pause, livre de recettes. **Options** depuis le titre : volumes, mode gaucher,
  effacement de la sauvegarde. SELECT + L : charge du processeur (debug).

## Architecture

```
source/
  core/       boucle (app.c), scènes, entrées, aléatoire
  gfx/        dessin logiciel RGB15 (canvas), police 6x11 Latin-1, compositeur des écrans
  game/       logique pure : ingrédients, boisson, recettes + score, clients, journée, sauvegarde
  scenes/     titre, boutique (logique + rendu), fin de journée, livre de recettes
  plat/       interface plateforme ; plat/nds/ = implémentation DS (libnds, maxmod, libfat)
  gen/        fichiers générés par tools/png2c.py (images, police) — ne pas éditer
host/         implémentation PC de la plateforme + scénario de captures
tests/        tests unitaires (make test)
tools/        générateurs : graphismes, conversion PNG -> C, sons, musique MOD
gfx/          PNG sources (remplaçables par l'art final)
audio/        effets WAV et musique MOD (convertis par mmutil)
```

Choix techniques :

- Chaque écran est un **fond bitmap 16 bits** (MODE_5, BG3). Le jeu dessine en
  logiciel ; un fond statique est gardé en RAM et seules les zones qui changent sont
  recomposées chaque frame (`gfx/screen.c`).
- `game/`, `gfx/` et `scenes/` ne dépendent **pas** de libnds : tout le jeu tourne aussi
  sur PC, ce qui permet les tests et les captures automatiques.
- La VRAM n'accepte pas les écritures 8 bits : le dessin n'écrit que des u16/u32 et le
  build DS utilise `-fno-tree-loop-distribute-patterns`.

## Conventions

- C11 (gnu11), 4 espaces, accolade de fonction sur sa propre ligne.
- Préfixes par module : `drink_`, `recipe_`, `day_`, `progress_`, `draw_`, `text_`,
  `screen_`, `plat_`, `ui_`…
- Pas d'allocation dynamique ; états en variables statiques.
- Les textes du jeu sont en UTF-8 dans le code, limités au Latin-1 (accents français OK).
- Les images de `gfx/` et les sons de `audio/` sont des placeholders générés par `tools/`.
