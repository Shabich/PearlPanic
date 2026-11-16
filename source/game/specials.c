#include "game/specials.h"

#include <string.h>

#include "game/recipes.h"
#include "game/world.h"

const SpecialCustomer SPECIALS[] = {
    [SP_MAMIE] = { "Mamie Lapine", CT_LAPIN, ACC_BOW_GLASSES, 3, 3, 15, {
        { 2, "Thé au lait perles",
          "Bonjour ma petite ! Comme au bon vieux temps : un thé au lait perles.",
          "Exactement comme celui de mon mari… Merci !" },
        { 3, "Brown sugar boba",
          "Mon petit-fils adore le sucre brun. Tu m'en prépares un pour lui ?",
          "Il va être ravi ! Tiens, un petit quelque chose." },
        { 4, "Le Câlin Total",
          "C'est mon anniversaire ! Je veux un lait tout doux, sucré, avec un nuage ET des perles.",
          "Le plus beau des anniversaires ! Tu iras loin, ma petite loutre." },
    } },
    [SP_CAPITAINE] = { "Capitaine Phoque", CT_PHOQUE, ACC_SAILOR, 3, 3, 18, {
        { 6, "Lagon bleu",
          "Ohé ! Un lagon bleu pour le vieux loup de mer !",
          "Comme les eaux des tropiques, moussaillon !" },
        { 7, "Écume salée",
          "La mer est agitée ce matin. Mets-moi de l'écume dans un thé vert !",
          "Ça a le goût du large !" },
        { 8, "Phare au lait",
          "Je rentre au port. Un thé noir au lait, crème salée et perles, comme mon phare !",
          "Je vais raconter ta paillote dans tous les ports du monde !" },
    } },
    [SP_CHEF] = { "Chef Panda", CT_PANDA, ACC_CHEF_HAT, 3, 4, 20, {
        { 10, "Chaï crémeux",
          "Chef pâtissier en vacances. Un chaï crémeux, apprentie. Je goûte.",
          "Équilibré. Pas mal du tout." },
        { 11, "Matcha des cimes",
          "Un matcha bien secoué. Je regarde.",
          "Mousse parfaite. Je suis impressionné." },
        { 12, "Avalanche",
          "Pour mon nouveau dessert, il me faut une avalanche de guimauves sur un chocolat crémeux.",
          "Magnifique ! Ton chalet sera sur ma carte !" },
    } },
    [SP_DJ] = { "DJ Raton", CT_RATON, ACC_HEADPHONES, 3, 4, 20, {
        { 14, "Néon bleu",
          "Yo ! Il me faut un truc qui brille pour mon set de ce soir.",
          "Ça brille autant que mes platines !" },
        { 15, "Fraise pétillante",
          "Mon public a soif ! Un soda rose avec de la gelée fluo.",
          "Le dancefloor va exploser !" },
        { 16, "Nuit étoilée",
          "Dernier morceau : je veux une nuit bleue et violette, pleine d'étoiles.",
          "Je t'ai dédié mon dernier morceau !" },
    } },
    [SP_POETE] = { "Grenouille poète", CT_GRENOUILLE, ACC_BERET, 3, 3, 20, {
        { 17, "Thé de rose",
          "Je cherche l'inspiration… Une rose, peut-être ?",
          "Ah ! Un poème me vient… Merci !" },
        { 18, "Lavande au lait",
          "Pour mon haïku, il me faut un rêve de lavande.",
          "« Lait de lavande, un rêve au creux de la tasse, le bois s'endort. »" },
        { 20, "Potion des fées",
          "Mon recueil s'appelle « Potions ». Il me faut une potion des fées !",
          "Mon recueil est fini. Il t'est dédié !" },
    } },
    [SP_PLUME] = { "Mlle Plume", CT_CHAT_NOIR, ACC_PRESS_HAT, 2, 5, 30, {
        { 19, "Rose crème",
          "Hmm… Une rose au lait, avec de la crème. Je prends des notes, ne faites pas attention.",
          "Intéressant. Très intéressant… À bientôt." },
        { 20, "Grand Final",
          "Je suis Plume, critique au Grand Journal. Pour mon article : votre Grand Final !",
          "Mon article sort demain matin. Préparez-vous…" },
        { 0, NULL, NULL, NULL },
    } },
};

const int SPECIAL_COUNT = sizeof(SPECIALS) / sizeof(SPECIALS[0]);

int special_world(int s)
{
    return level_world(SPECIALS[s].visits[0].level);
}

int specials_due(int level, const uint8_t steps[], int out[SPECIALS_PER_LEVEL])
{
    int n = 0;
    for (int s = 0; s < SPECIAL_COUNT && n < SPECIALS_PER_LEVEL; s++) {
        int step = steps[s];
        if (step >= SPECIALS[s].visit_count)
            continue;
        int vl = SPECIALS[s].visits[step].level;
        // Visite à ce niveau, ou rattrapage plus loin dans le même monde
        if (level >= vl && level_world(level) == level_world(vl))
            out[n++] = s;
    }
    return n;
}

int special_recipe(int s, int step)
{
    if (s < 0 || s >= SPECIAL_COUNT || step < 0 || step >= SPECIALS[s].visit_count)
        return -1;
    const char *name = SPECIALS[s].visits[step].recipe;
    for (int i = 0; i < RECIPE_COUNT; i++)
        if (strcmp(RECIPES[i].name, name) == 0)
            return i;
    return -1;
}
