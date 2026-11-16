// Le Journal : « L'Écho des Perles », des articles qui arrivent au fil des niveaux
#ifndef PP_JOURNAL_H
#define PP_JOURNAL_H

#include <stdbool.h>

typedef enum {
    NEWS_BAR,       // à propos du bar de Perle
    NEWS_WORLD,     // la vie du monde autour
    NEWS_MYSTERY,   // la critique mystère
} NewsKind;

typedef struct {
    int unlock;          // niveau à terminer pour lire l'article (0 = dès le début)
    NewsKind kind;
    const char *title;
    const char *body;
} Article;

extern const Article ARTICLES[];
extern const int ARTICLE_COUNT;

// Articles disponibles (dans l'ordre), selon la progression
int journal_available(void);
// Articles pas encore lus
int journal_unread(void);
// Index réel du n-ième article disponible, du plus récent au plus ancien
int journal_nth_recent(int n);

#endif
