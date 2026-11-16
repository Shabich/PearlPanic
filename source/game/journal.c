#include "game/journal.h"

#include "game/progress.h"

// unlock = niveau à terminer ; les articles d'un même niveau s'affichent ensemble
const Article ARTICLES[] = {
    // ------------------------------------------------ Avant le premier niveau
    { 0, NEWS_WORLD, "Le printemps arrive au Quartier des Lanternes",
      "Les lanternes roses ont été rallumées pour la saison. Les habitants espèrent du nouveau dans la "
      "petite boutique abandonnée de la rue des Glycines." },
    { 0, NEWS_BAR, "Une boutique enfin louée !",
      "Une jeune loutre aurait signé le bail de la rue des Glycines. Selon nos sources, il serait "
      "question… de bubble tea ?" },
    // ------------------------------------------------ Le Petit Bar
    { 1, NEWS_BAR, "Pearl Panic ouvre ses portes",
      "Le premier jour fut mouvementé mais les clients sont repartis souriants. « Les perles roulent "
      "comme des billes ! » témoigne un poussin enthousiaste." },
    { 2, NEWS_WORLD, "Mamie Lapine fête ses 80 printemps",
      "La doyenne du quartier prépare une grande fête. Elle aurait déjà trouvé « le meilleur thé au lait "
      "depuis des années »." },
    { 3, NEWS_BAR, "La file d'attente déborde sur le trottoir",
      "Taro, fraise, matcha : les nouvelles recettes font sensation. Les voisins demandent un deuxième "
      "comptoir." },
    { 4, NEWS_BAR, "Perle part à la conquête de la plage",
      "Fort de son succès, Pearl Panic ouvrira une paillote sur la Plage des Coquillages. « Je reviendrai "
      "souvent ! » promet la gérante." },
    // ------------------------------------------------ La Paillote
    { 5, NEWS_WORLD, "Vague de chaleur sur la côte",
      "Les thermomètres s'affolent. Les glaçons de la nouvelle paillote sont pris d'assaut par les "
      "pingouins en vacances." },
    { 6, NEWS_WORLD, "Le Capitaine Phoque rentre au port",
      "Après trois ans en mer, le vieux loup de mer jure qu'il n'a jamais bu un lagon aussi bleu." },
    { 7, NEWS_MYSTERY, "Une critique mystérieuse sillonne le pays",
      "Chapeau immense, carnet de notes : une dégustatrice anonyme visiterait les meilleurs bars du "
      "pays. Personne ne connaît son nom." },
    { 8, NEWS_BAR, "Direction les sommets !",
      "La paillote ne désemplit pas. Perle confie son prochain rêve : un chalet tout chaud au Col des "
      "Flocons." },
    // ------------------------------------------------ Le Chalet
    { 9, NEWS_WORLD, "Premières neiges au Col des Flocons",
      "Les marmottes se réveillent pour une seule raison : le chocolat chaud aux guimauves du nouveau "
      "chalet." },
    { 10, NEWS_WORLD, "Un grand chef en vacances incognito",
      "Le célèbre Chef Panda aurait été aperçu au chalet, carnet de recettes à la main. Il goûte tout et "
      "ne dit presque rien." },
    { 11, NEWS_MYSTERY, "La critique au grand chapeau a encore frappé",
      "Trois bars de montagne ont reçu sa visite. Ses articles, signés « P. », font et défont les "
      "réputations." },
    { 12, NEWS_BAR, "Pearl Panic s'invite en ville",
      "Après la montagne, Perle va ouvrir un kiosque néon sur l'Avenue des Étoiles. Ouverture : à la nuit "
      "tombée !" },
    // ------------------------------------------------ Le Kiosque Néon
    { 13, NEWS_WORLD, "La ville ne dort jamais",
      "Chauves-souris, ratons et pigeons noctambules ont trouvé leur nouveau QG : un kiosque qui brille "
      "de mille couleurs." },
    { 14, NEWS_WORLD, "DJ Raton prépare son plus grand set",
      "Le DJ le plus en vue de l'Avenue des Étoiles cherche « une boisson qui brille autant que ses "
      "platines »." },
    { 15, NEWS_MYSTERY, "Qui est « P. » ?",
      "Un indice : la critique mystère adorerait les roses et les nuages de crème. Les bars de la "
      "forêt seraient sa prochaine étape…" },
    { 16, NEWS_BAR, "Dernière étape : la forêt enchantée",
      "Perle ouvrira bientôt son bar le plus magique, dans un arbre du Bois des Lucioles. Sera-t-il "
      "visité par la critique ?" },
    // ------------------------------------------------ L'Arbre aux Thés
    { 17, NEWS_WORLD, "Les lucioles illuminent le Bois",
      "La saison des lucioles commence. Champignons et escargots se pressent sous l'Arbre aux Thés." },
    { 18, NEWS_WORLD, "Un recueil de poèmes très attendu",
      "La Grenouille poète aurait trouvé l'inspiration dans une tasse de lait à la lavande. Sortie "
      "prévue très bientôt." },
    { 19, NEWS_MYSTERY, "Elle était là !",
      "Une cliente au grand chapeau a été vue à l'Arbre aux Thés. Elle a pris beaucoup de notes. Les "
      "habitués retiennent leur souffle." },
    { 20, NEWS_BAR, "À la une : Pearl Panic, phénomène national",
      "« Cinq bars, une seule passion. » La critique Plume, du Grand Journal, attribue cinq étoiles à "
      "Pearl Panic. Les journalistes accourent de tout le pays !" },
};

const int ARTICLE_COUNT = sizeof(ARTICLES) / sizeof(ARTICLES[0]);

static bool available(int i)
{
    int l = ARTICLES[i].unlock;
    return l == 0 || (l <= LEVEL_COUNT && progress()->stars[l - 1] > 0);
}

int journal_available(void)
{
    int n = 0;
    for (int i = 0; i < ARTICLE_COUNT; i++)
        n += available(i);
    return n;
}

int journal_unread(void)
{
    int n = journal_available() - progress()->journal_read;
    return n > 0 ? n : 0;
}

int journal_nth_recent(int n)
{
    for (int i = ARTICLE_COUNT - 1; i >= 0; i--)
        if (available(i) && n-- == 0)
            return i;
    return -1;
}
