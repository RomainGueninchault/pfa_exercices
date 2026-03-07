/**
 * Cet exercice consiste à mettre en œuvre la structure de données "Piece list"
 * utilisée dans plusieurs éditeurs de texte. Il s'agit d'une version étendue
 * de l'examen du cours PG116 2022/2023 session 1. Le sujet est disponible sur
 * la page moodle du cours.
*/

/********* DEBUT DU CODE A NE PAS MODIFIER **********************/

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Représentation d'une portion de texte qui débute au caractère
   `start` du buffer `buffer_id`, de longueur `length` caractères.
   `next` pointe sur la portion suivante dans la piece table
*/
struct piece {
    unsigned int buffer_id;
    size_t start;
    size_t length;
    struct piece * next;
};

/* Représentation d'une piece table comme une liste chaînée de portions,
   terminée par la sentinel PIECE_SENTINEL_PTR (cf. ci-dessous).
   `head` pointe sur la première portion
   `tail` pointe sur la derniere portion
*/
struct piece_list {
    struct piece * head;
    struct piece * tail;
};

/* Sentinelle pour la fin de la piece table (struct piece_list) */
#define PIECE_SENTINEL_PTR ((struct piece *)&piece_sentinel)
extern struct piece const piece_sentinel;

/* Ajoute la portion `p` en tête de la liste `pl`
   PRE: `pl` et `p` ne sont pas NULL
   POST: `p` a été ajoutée en tête de `pl`
   RETOURNE: 0
*/
int add_head(struct piece_list * pl, struct piece * p);

/* Ajoute la portion `p` après la portion `after` dans `pl`
    PRE: `pl`, `after` et `p` ne sont pas NULL
         `p` est une piece de `pl`
    POST: `p` a été ajoutée après `after` dans la liste `pl`
    RETOURNE: -1 si `after` ou `p` est PIECE_SENTINEL_PTR
              0 sinon (ajout effectué)
*/
int add_after(struct piece_list * pl, struct piece * after, struct piece * p);

/* Ajoute la portion `p` en queue de la liste `pl`
    PRE: `pl` et `p` ne sont pas NULL
    POST: `p` a été ajoutée en queue de la liste `pl`
    RETOURNE: -1 si `p` est PIECE_SENTINEL_PTR
              0 sinon (ajout effectué)
*/
int add_tail(struct piece_list * pl, struct piece * p);

/* Retire la portion en tête de la liste `pl`
    PRE: `pl` n'est pas NULL
    POST: la portion en tête de la liste `pl` a ete retirée
    RETOURNE: NULL si `pl` est vide
              la portion retirée sinon
*/
struct piece * remove_head(struct piece_list * pl);

/* Retire la portion qui suit `after` dans la liste `pl`
    PRE: `pl` et `after` ne sont pas NULL
         `after` est une portion de la liste `pl`
    POST: la portion qui suit `after` dans `pl` a été retirée
    RETOURNE: NULL si `after` ou la portion qui suit `after` est 
              PIECE_SENTINEL_PTR
              la portion retirée sinon
*/
struct piece * remove_after(struct piece_list * pl, struct piece * after);

/* Affiche le contenu de la piece table `pl` sur la sortie d'erreur 
*/
void pl__debug(const struct piece_list* pl);

/********** FIN DU CODE A NE PAS MODIFIER ***********************/

/* Alloue et initialise une liste vide
    RETOURNE: une liste vide allouée et initialisée
*/
struct piece_list * pl__create()
{
    struct piece_list * pl = malloc(sizeof(struct piece_list));
    pl->head = PIECE_SENTINEL_PTR;
    pl->tail = PIECE_SENTINEL_PTR;
    return pl;
}

/* Libère la mémoire utilisée par la liste `pl`
    PRE: `pl` n'est pas NULL
    POST: la mémoire allouée pour `pl` et les portions qu'elle contient a été
          libérée
*/
void pl__free(struct piece_list * pl)
{
    assert(pl != NULL);
    for (struct piece * p = pl->head; p != PIECE_SENTINEL_PTR; ) {
        struct piece * pfree = p;
        p = p->next;
        free(pfree);
    }
    free(pl);
}

/* Retourne une chaîne de caractère terminée par 0 qui contient le texte
   représenté par la liste `pl` dont les portions sont extraites de `buffers`
    PRE: `buffers` et `pl` ne sont pas NULL
          les `buffer_id` des portions de `pl` sont des indices valides dans le
          tableau `buffers`
    RETOURNE: la chaîne de caractères terminée par 0 qui contient le texte
              représenté par la liste `pl` dont les portions sont extraites de
              `buffers`
    NOTE: le pointeur retourné doit être libéré par le code qui appelle la
          fonction `pl__to_string`
*/
char * pl__to_string(struct piece_list const * pl, char const * buffers[])
{
    assert(buffers != NULL);
    assert(pl != NULL);

    size_t size = 0;
    size_t capacity = 1;
    char * str = malloc(capacity);

    for (struct piece * p = pl->head; p != PIECE_SENTINEL_PTR; p = p->next) {
        capacity += p->length;
        str = realloc(str, capacity);
        memcpy(str + size, buffers[p->buffer_id] + p->start, p->length);
        size += p->length;
    }

    str[size] = 0;

    return str;
}

/* Découpe la portion de la liste `pl` qui contient le caractère en position
   `pos` en deux portions, et retourne ladite portion
    PRE: `pl` n'est pas NULL
    POST: la portion {buffer_id, start, length} qui contient le caractère en
          position `pos` dans `pl` a été scindée en deux portions dans `pl`:
          {buffer_id, start, l} pour la première et
          {buffer_id, start+l, length-l} pour la seconde
          où `l` est le nombre de caractères avant celui en position `pos`
          dans `pl`
          La seconde portion a été ajoutée à la suite de la première dans la
          liste `pl`
    RETOURNE: un pointeur sur la portion qui a été scindée si `pos` est une
              position valide dans `pl`
              NULL sinon

    Exemple 1:
    `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}] et `pos` = 4
    le caractère en position 4 dans `pl` est le caractère en position 2
    dans la portion {1, 4, 5}. La fonction `pl__split_piece` scinde donc la
    portion {1, 4, 5} en {1, 4, 2} et {1, 6, 3} dans `pl`:
    `pl` = [{0, 0, 2}, {1, 4, 2}, {1, 6, 3}, {0, 7, 9}]
    et retourne un pointeur sur la portion {1, 4, 2} qui vient d'être scindée

    Exemple 2:
    `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}] et `pos` = 0
    le caractère en position 0 dans `pl` est le caractère en position 0
    dans la portion {0, 0, 2}. La fonction `pl__split_piece` scinde donc la
    portion {0, 0, 2} en {0, 0, 0} et {0, 0, 2} dans `pl`:
    `pl` = [{0, 0, 0}, {0, 0, 2}, {1, 4, 5}, {0, 7, 9}]
    et retourne un pointeur sur la portion {0, 0, 0} qui vient d'être scindée

    Exemple 3:
    `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}] et `pos` = 23
    la fonction `pl__split_piece` retourne NULL car `23` n'est pas une position
    valide

    Exemple 4:
    `pl` = [{0, 0, 2}, {1, 0, 0}, {2, 4, 5}] et `pos` = 2
    le caractère en position 2 dans `pl` est le caractère en position 0
    dans la portion {2, 4, 5}. La fonction `pl__split_piece` scinde donc la
    portion {2, 4, 5} en {2, 4, 0} et {2, 4, 5} dans `pl`:
    `pl` = [{0, 0, 2}, {1, 0, 0}, {2, 4, 0}, {2, 4, 5}]
    et retourne un pointeur sur la portion {2, 4, 0} qui vient d'être scindée
*/
struct piece * pl__split_piece(struct piece_list * pl, size_t pos)
{
    assert(pl != NULL);

    struct piece * p = pl->head;
    for (; p != PIECE_SENTINEL_PTR; p = p->next)
    {
        if (pos < p->length)
            break;
        pos -= p->length;
    }

    if (p == PIECE_SENTINEL_PTR)
        return NULL;

    struct piece * newp = malloc(sizeof(struct piece));
    newp->buffer_id = p->buffer_id;
    newp->start = p->start + pos;
    newp->length = p->length - pos;
    newp->next = NULL;

    p->length = pos;

    add_after(pl, p, newp);
    return p;
}

/* Efface le caractère en position `pos` dans la liste `pl`
    PRE: `pl` n'est pas NULL
    POST: le caractère en position `pos` a été retiré de la liste `pl`
    RETOURNE: -1 si la suppression n'a pas pu être effectuée (`pos` invalide,
              ...)
              0 si la suppression a pu être effectuée

    Exemple 1:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}] et `pos` = 4
        la fonction `pl__erase` retourne 0 et la liste `pl` devient:
        `pl` = [{0, 0, 2}, {1, 4, 2}, {1, 7, 2}, {0, 7, 9}]

    Exemple 2:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}] et `pos` = 15
        la fonction `pl__erase` retourne 0 et la liste `pl` devient:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 8}, {0, 16, 0}]

    Exemple 3:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}] et `pos` = 56
        La fonction `pl__erase` retourne -1 car `pos` n'indique pas une
        position valide dans la liste `pl`

    NOTE: La fonction `pl__erase` ne cherche pas à "simplifier" la liste.
    En particulier, elle peut créer des portions de longueur 0 dans la liste
    `pl`
*/
int pl__erase(struct piece_list * pl, size_t pos)
{
    assert(pl != NULL);

    struct piece * p = pl__split_piece(pl, pos);

    if (p == NULL) {
        return -1;
    }

    p->next->start += 1;
    p->next->length -= 1;

    return 0;
}

/* Insère la portion {buffer_id, start, length} au caractère en position `pos`
   dans la liste `pl`
    PRE: `pl` n'est pas NULL
    POST: la portion {buffer_id, start, length} a été ajoutée au caractère en
          position `pos` dans la liste `pl`
    RETOURNE: -1 si l'ajout n'a pas pu être effectué (position invalide, ...)
              0 sinon

    Exemple 1:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}]
        `pos` = 4
        `buffer_id` = 2
        `start` = 3
        `length` = 47
        La position 4 dans `pl` est la position 2 dans {1, 4, 5}. La fonction
        `pl__insert` retourne 0 et:
        `pl` = [{0, 0, 2}, {1, 4, 2}, {2, 3, 47}, {1, 6, 3}, {0, 7, 9}]

    Exemple 2:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}]
        `pos` = 26
        `buffer_id` = 2
        `start` = 3
        `length` = 47
        La fonction `pl__insert` retourne -1 car la position 26 n'est pas
        valide dans `pl`

    Exemple 3:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}]
        `pos` = 16
        `buffer_id` = 2
        `start` = 3
        `length` = 47
        La position 16 est celle qui suit le dernier caractère de `pl`
        (insertion en fin). La fonction `pl__insert` retourne 0 et:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, {2, 3, 47}]

    NOTE: la fonction `pl__insert` se content d'ajouter la nouvelle portion à la
          liste `pl` sans chercher à "simplifier" la liste. En particulier,
          cette opération peut créer des portions de longueur 0 dans la liste `pl`
*/
int pl__insert(struct piece_list * pl, size_t pos, unsigned int buffer_id,
               size_t start, size_t length)
{
    assert(pl != NULL);

    // Check that pos is valid
    size_t chk_length = 0;
    for (struct piece * p = pl->head; p != PIECE_SENTINEL_PTR; p = p->next)
        chk_length += p->length;

    if (pos > chk_length)
        return -1;

    // Create new piece, split and add after split piece
    struct piece * newp = malloc(sizeof(struct piece));
    newp->buffer_id = buffer_id;
    newp->start = start;
    newp->length = length;
    newp->next = NULL;

    struct piece* p = pl__split_piece(pl, pos);

    if (p == NULL)
        add_tail(pl, newp);
    else
        add_after(pl, p, newp);

    return 0;
}

/* Retire les portions de longueur 0, et fusionne les portions consécutives
   adjacentes dans la liste `pl`
    PRE: `pl` n'est pas NULL
    POST: `pl` ne contient pas de portion de longueur 0, ni de portions
           consécutives adjacentes.
           le texte représenté par `pl` n'a pas été modifié

    Exemple 1:
        `pl` = [{0, 0, 2}, {1, 4, 5}, {0, 2, 9}]
        la fonction `pl__compress` laisse la liste `pl` inchangée

    Exemple 2:
        `pl` = [{1, 4, 0}, {2, 3, 0}, {0, 2, 7}, {1, 4, 0}, {6, 2, 0}, 
                {7, 89,0}, {0, 9, 3}, {1, 12, 1}, {1, 13, 2}, {1, 15, 5}
                {3, 4, 0}]
        la fonction `pl__compress` met à jour `pl` en retirant les portions de
        longueur 0 et en fusionnant les portions adjacentes consécutives:
        `pl` = [{0, 2, 10}, {1, 12, 8}]
*/
void pl__compress(struct piece_list * pl)
{
    assert(pl != NULL);

    // Search first piece of length > 0
    while (pl->head != PIECE_SENTINEL_PTR && pl->head->length == 0) {
        struct piece * pfree = remove_head(pl);
        free(pfree);
    }

    // For each piece of length > 0
    for (struct piece * p = pl->head; p != PIECE_SENTINEL_PTR; p = p->next)
    {
        assert(p->length > 0);

        // Remove next pieces which are empty of adjacent
        while (p->next != PIECE_SENTINEL_PTR) {
            // empty piece
            if (p->next->length == 0) {
                struct piece * pfree = remove_after(pl, p);
                free(pfree);
            }
            // adjacent pieces
            else if (p->buffer_id == p->next->buffer_id &&
                     p->start + p->length == p->next->start) {
                struct piece * pfree = remove_after(pl, p);
                p->length += pfree->length;
                free(pfree);
            }
            else
                break;
        }
    }
}

