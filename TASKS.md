# Pearl Panic — Découpage en sous-tâches

Jeu de gestion de bubble tea pour **Nintendo DS**, écrit en **C**.
Pixel art pastel et chaleureux. Le joueur assemble les boissons au stylet,
devine et débloque des recettes, et sert des clients mignons mais très impatients.

## Contraintes plateforme (à garder en tête partout)

| Élément | Valeur |
|---|---|
| Écrans | 2 × 256×192 px — **haut** : comptoir / clients, **bas (tactile)** : poste d'assemblage |
| CPU | ARM9 67 MHz (jeu) + ARM7 (son, entrées, tactile) |
| RAM | 4 Mo — budget assets serré |
| VRAM | 656 Ko répartis en banques A–I |
| Sprites | 128 par écran (OAM), 16 ou 256 couleurs, tailles 8×8 à 64×64 |
| Fonds | 4 couches par écran (tilemaps, bitmap ou affine) |
| Entrées | Stylet (tactile), croix, A/B/X/Y, L/R, Start/Select |
| Son | maxmod (musique MOD/XM + effets) |

**Toolchain recommandée :** BlocksDS (libnds moderne, maintenu) — alternative : devkitPro/devkitARM.
**Émulateurs de test :** melonDS (précis, débogueur GDB), no$gba (débogage VRAM/OAM).
**Matériel :** une vraie DS/DS Lite + linker (R4, DSTWO…) pour valider le tactile et les perfs.

Légende : **[P0]** indispensable au prototype · **[P1]** version jouable complète · **[P2]** polish / bonus
Les dépendances sont notées `→ dépend de`.

---

## 0. Mise en place du projet

- [x] **0.1 [P0]** Installer la toolchain (BlocksDS ou devkitARM) et vérifier qu’un « hello world » tourne sur melonDS. *(BlocksDS via Docker, validé dans melonDS)*
- [x] **0.2 [P0]** Créer l'arborescence : `source/`, `include/`, `gfx/`, `audio/`, `data/`, `tools/`.
- [x] **0.3 [P0]** Makefile : compilation `.nds`, conversion auto des PNG via **grit**, de l'audio via **mmutil**. → 0.1 *(grit remplacé par `tools/png2c.py` : les images sont dessinées en logiciel, cf. 1.6)*
- [x] **0.4 [P0]** Bannière/icône de la ROM (32×32) et titre. → 0.3
- [x] **0.5 [P1]** Script `make run` qui lance melonDS sur la ROM.
- [x] **0.6 [P1]** CI GitHub Actions (image Docker BlocksDS) qui compile la ROM à chaque push.
- [x] **0.7 [P1]** Conventions de code : préfixes de modules (`gfx_`, `snd_`, `cust_`…), `clang-format`, pas d'allocation dynamique dans la boucle de jeu. *(voir README)*

## 1. Moteur de base (engine)

- [x] **1.1 [P0]** Boucle principale à 60 FPS calée sur `swiWaitForVBlank()`, temps en frames. → 0.3
- [x] **1.2 [P0]** Machine à états de scènes (`scene_init / update / draw / exit`) : Titre, Jeu, Fin de journée, Livre de recettes, Options. → 1.1
- [x] **1.3 [P0]** Module d'entrées : touches (pressé / maintenu / relâché) + stylet (`touchRead`, position, tap, drag, release). → 1.1
- [x] **1.4 [P0]** Configuration vidéo : modes et banques VRAM pour les deux écrans, choix de l'écran tactile en bas. → 1.1
- [x] **1.5 [P0]** Gestionnaire de sprites : allocation d'entrées OAM, chargement de tuiles/palettes, priorité/couche. → 1.4 *(remplacé par le rendu logiciel + compositeur de `gfx/screen.c` ; les sprites matériels restent possibles pour du polish)*
- [x] **1.6 [P0]** Gestionnaire de fonds : chargement tilemaps + palettes, scrolling. → 1.4 *(fonds bitmap 16 bits + recomposition des zones modifiées)*
- [ ] **1.7 [P1]** Animation de sprites (frames, durée, boucle / one-shot). → 1.5
- [x] **1.8 [P1]** Rendu de texte : police bitmap pixel art 8×8 (ou 8×16), accents français. → 1.6 *(police 6×11 Latin-1 provisoire, issue de Pillow)*
- [x] **1.9 [P1]** Tweens/easing (déplacement, rebond, scale via affine) pour le « juice ». → 1.5 *(`core/tween.h` : arrivée des clients qui rebondit, départs, pièces)*
- [x] **1.10 [P1]** Transitions de scène (fondu via registres de luminosité `REG_MASTER_BRIGHT`). → 1.2 *(fondu au blanc via `setBrightness`, 8 frames)*
- [x] **1.11 [P1]** Générateur pseudo-aléatoire (xorshift) avec graine.
- [x] **1.12 [P2]** Overlay de debug (FPS, nb sprites, mémoire libre) activable avec Select+L. *(SELECT + L : charge CPU)*

## 2. Direction artistique & assets

- [ ] **2.1 [P0]** Définir la palette pastel maître (ex. 32–48 couleurs) et la découper en palettes de 16 couleurs.
- [x] **2.2 [P0]** Placeholders : rectangles colorés pour gobelet, ingrédients, client — permet de coder sans art final.
- [ ] **2.3 [P1]** Décor écran haut : intérieur de boutique cocooning (comptoir de profil, plantes, guirlandes). *(placeholder en place, art final à faire)*
- [ ] **2.4 [P1]** Décor écran bas : plan de travail (zone gobelet, bacs d'ingrédients, shaker, scelleuse, poubelle). *(placeholder en place, art final à faire)*
- [ ] **2.5 [P1]** Gobelet : vide, niveaux de remplissage, couches de couleur (thé, lait, sirop), couvercle scellé, paille. *(placeholder en place, art final à faire)*
- [ ] **2.6 [P1]** Ingrédients : perles tapioca, popping boba, jelly, thés (noir, vert, jasmin, matcha), laits, sirops (taro, fraise, mangue, brown sugar), glace, toppings mignons (nuage de crème, étoiles…). *(placeholder en place, art final à faire)*
- [ ] **2.7 [P1]** Clients : 6–10 personnages chibi (animaux / créatures), chacun avec animations idle, entrée, attente, stress (3 niveaux), content, furieux, départ. *(placeholder en place, art final à faire)*
- [ ] **2.8 [P1]** UI : bulles de commande, jauge de stress, pièces, icônes, boutons tactiles. *(placeholder en place, art final à faire)*
- [ ] **2.9 [P1]** Écran titre + logo « Pearl Panic ». *(placeholder en place, art final à faire)*
- [x] **2.10 [P2]** Effets : particules (cœurs, étoiles, gouttes, fumée de colère). *(`scenes/fx.c` : cœurs, étoiles, pièces, sueur, fumée, éclaboussures)*
- [x] **2.11 [P0]** Pipeline : fichiers `.grit` par asset, nommage cohérent, vérification du budget VRAM. → 0.3 *(PNG → C via `make assets`)*

## 3. Poste d'assemblage (cœur du gameplay, écran bas)

- [x] **3.1 [P0]** Modèle de données `Drink` : base thé, lait, sirop(s), topping(s), glace, sucre, scellé, ordre des étapes. → 1.1
- [x] **3.2 [P0]** Glisser-déposer au stylet : prendre un ingrédient dans un bac, le lâcher sur le gobelet. → 1.3, 1.5
- [x] **3.3 [P0]** Versement des liquides : maintenir le stylet sur une tireuse = le niveau monte ; relâcher = arrêt (risque de débordement). → 3.1, 3.2
- [x] **3.4 [P0]** Ajout de perles/toppings par cuillerées (nb de cuillerées = quantité). → 3.2
- [x] **3.5 [P1]** Glace et niveau de sucre (boutons ou curseur tactile). → 3.1 *(glaçons = garniture, sucre = pompes à sirop)*
- [x] **3.6 [P1]** Shaker : geste de va-et-vient au stylet (détection de mouvement alterné) pour mélanger. → 1.3
- [x] **3.7 [P1]** Scelleuse : glisser le gobelet sous la machine, tap pour sceller, puis planter la paille. *(bouton « Fermer » ; paille automatique)*
- [x] **3.8 [P0]** Poubelle : jeter la boisson en cours. → 3.2
- [x] **3.9 [P0]** Bouton / geste « Servir » qui envoie la boisson au client actif. → 3.1
- [x] **3.10 [P1]** Rendu visuel du gobelet qui reflète exactement son contenu (couches, perles au fond). → 2.5, 3.1
- [x] **3.11 [P1]** Feedback immédiat : sons, petite secousse, éclaboussure.
- [x] **3.12 [P2]** Gestion de 2 gobelets en parallèle (montée en difficulté). *(amélioration « 2e gobelet », 60 pièces ; toucher un gobelet le rend actif)*

## 4. Clients & jauge de stress (écran haut)

- [x] **4.1 [P0]** Entité `Customer` : type, commande, patience max, patience actuelle, état (arrive, attend, servi, part). → 1.1
- [x] **4.2 [P0]** File d'attente : jusqu'à 3–4 clients au comptoir, positions, entrée/sortie animées. → 4.1, 1.5
- [x] **4.3 [P0]** Jauge de stress par client qui monte avec le temps ; visuel qui change par paliers (sourire → sueur → fumée). → 4.1
- [x] **4.4 [P0]** Client qui part furieux quand la jauge est pleine (pénalité). → 4.3
- [x] **4.5 [P0]** Génération de commandes selon les recettes débloquées et le jour. → 5.1, 1.11
- [x] **4.6 [P1]** Bulle de commande : explicite (icônes) ou énigmatique (« quelque chose de violet et rebondissant ! ») pour la partie devinette. → 4.5, 5.4
- [x] **4.7 [P1]** Personnalités : patience, pourboire, exigence différentes selon le type de client. → 4.1
- [x] **4.8 [P1]** Sélection du client actif (tap sur l'écran du haut impossible → croix gauche/droite ou icônes sur l'écran bas). → 1.3
- [x] **4.9 [P1]** **Ambiance de la boutique** : jauge globale « cocooning » qui baisse avec les clients stressés/furieux ; si elle tombe à zéro, fin de journée anticipée. → 4.3
- [x] **4.10 [P2]** Clients spéciaux récurrents avec mini-histoire. *(Mamie Lapine, Chef Panda, Grenouille poète : 3 visites chacun, accessoires, bonus)*

## 5. Recettes & découverte

- [x] **5.1 [P0]** Table de recettes en données (`data/recipes.c` ou fichier binaire) : nom, ingrédients requis, tolérances. → 3.1
- [x] **5.2 [P0]** Évaluation d'une boisson vs commande → score (0–100 %), critères : bons ingrédients, quantités, ordre, scellé. → 5.1
- [x] **5.3 [P1]** Réaction du client selon le score (ravi / ok / déçu) + paiement + pourboire. → 5.2, 4.1
- [x] **5.4 [P1]** Système d'**expérimentation** : une boisson qui correspond à une recette non encore connue la débloque (« Nouvelle recette ! »). → 5.2
- [x] **5.5 [P1]** Indices : les commandes énigmatiques donnent des pistes ; les essais ratés renvoient un indice (« plus sucré ? »). → 4.6
- [x] **5.6 [P1]** Livre de recettes consultable (pause) : recettes connues, silhouettes des inconnues. → 1.8, 5.4
- [x] **5.7 [P1]** Contenu : ~20–30 recettes réparties sur la progression. *(19 recettes sur 5 jours)*
- [x] **5.8 [P2]** Recettes secrètes / combinaisons farfelues. *(3 recettes secrètes)*

## 6. Économie, progression & méta

- [x] **6.1 [P0]** Structure d'une journée : durée (ex. 3 min), vagues de clients, fin de journée. → 4.2
- [x] **6.2 [P1]** Écran de fin de journée : recettes, pourboires, clients satisfaits/perdus, nouvelles recettes. → 6.1
- [x] **6.3 [P1]** Boutique d'améliorations : nouveaux ingrédients, tireuse plus rapide, décorations (+patience / +ambiance). → 6.2 *(4 améliorations ; les nouveaux ingrédients arrivent par jour)*
- [x] **6.4 [P1]** Courbe de difficulté : patience plus faible, plus de clients, commandes plus complexes au fil des jours.
- [x] **6.5 [P1]** Sauvegarde : jour, argent, recettes débloquées, améliorations. Via libfat/NitroFS sur SD (linker) ; prévoir abstraction `save_read/save_write`. → 1.1 *(fichier `pearlpanic.sav` via libfat ; à valider sur linker)*
- [x] **6.6 [P2]** Objectifs quotidiens / succès. *(un objectif par jour, bonus de pièces)*
- [x] **6.7 [P2]** Mode infini « Rush » avec score. *(débloqué après le jour 1, record sauvegardé)*

## 7. Audio

- [x] **7.1 [P1]** Intégration maxmod (soundbank générée par mmutil). → 0.3
- [x] **7.2 [P1]** Musique lo-fi / cozy pour la boutique, variante plus rapide quand l'ambiance chute. *(boucle MOD générée par `tools/gen_music.py`, à remplacer par une vraie compo)*
- [x] **7.3 [P1]** SFX : versement, perles, shaker, scellage, caisse, client content, client fâché, déblocage de recette. *(sons synthétisés par `tools/gen_audio.py`, placeholders)*
- [x] **7.4 [P2]** Voix « blips » mignonnes par type de client. *(un blip joué à une hauteur différente par espèce)*

## 8. UI & menus

- [x] **8.1 [P1]** Écran titre (Nouvelle partie / Continuer / Options). → 1.2
- [x] **8.2 [P1]** Menu pause (Start) : reprendre, livre de recettes, quitter. → 1.2
- [x] **8.3 [P1]** HUD : argent, horloge de la journée, ambiance. → 1.8
- [x] **8.4 [P1]** Tutoriel guidé du premier jour (première commande pas à pas). → 3.x, 4.x
- [x] **8.5 [P2]** Options : volume musique/SFX, gaucher (miroir de l'écran bas), effacer la sauvegarde. *(volumes, mode gaucher, effacement en deux appuis)*

## 9. Tests, perf & finalisation

- [x] **9.1 [P0]** Tests unitaires hôte (compilés pour PC) de la logique pure : évaluation de recettes, génération de commandes, jauges. → 5.2
- [ ] **9.2 [P1]** Vérifier 60 FPS stables avec 4 clients + effets sur vrai matériel.
- [ ] **9.3 [P1]** Calibrage tactile et zones de tap assez grandes (min ~16×16 px).
- [ ] **9.4 [P1]** Équilibrage (patience, prix, durée de journée) via sessions de playtest.
- [ ] **9.5 [P2]** Test sur DSi / 3DS (mode rétrocompatible) et en émulation.
- [ ] **9.6 [P2]** Build release + notes de version.

---

## Jalons

| Jalon | Contenu | Tâches |
|---|---|---|
| **M1 — Hello DS** | ROM qui compile, deux écrans affichés, stylet lu | 0.1–0.4, 1.1–1.4 |
| **M2 — Prototype gris** | Placeholders : 1 client, 1 recette, glisser-déposer, servir, score | 1.5–1.6, 2.2, 3.1–3.4, 3.8–3.9, 4.1, 5.1–5.2 |
| **M3 — Le rush** | File de clients, jauges de stress, journée complète | 4.2–4.5, 6.1, 9.1 |
| **M4 — Découverte** | Commandes énigmatiques, déblocage de recettes, livre | 4.6, 5.3–5.7 |
| **M5 — Vertical slice** | Art final, son, menus, sauvegarde, 5 jours jouables | 2.x, 7.x, 8.x, 6.2–6.5 |
| **M6 — Contenu & polish** | Toutes recettes/clients, équilibrage, P2 | reste |

## Architecture de code

Voir la section « Architecture » du [README](README.md). Règle clé : `game/`, `gfx/`
et `scenes/` ne dépendent **pas** de libnds → tout le jeu tourne aussi sur PC
(`make test`, `make snap`).

## Reste à faire en priorité

1. Test sur vraie DS (9.2, 9.3) : fluidité du rendu logiciel, précision du stylet, sauvegarde sur linker.
2. Playtest et équilibrage (9.4) : patience, vitesse de l'ambiance, prix, objectifs, cadence du Rush.
3. Art final (2.x) en remplaçant les PNG de `gfx/` ; vraie musique (7.2) et vrais sons (7.3).
4. Restent en P2 : 1.7 (animations image par image, avec l'art final), 9.5 (DSi/3DS), 9.6 (release).
