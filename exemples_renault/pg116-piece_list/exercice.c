#include "grader2.h"
#include "provided.h"

// Buffers
char const * buffers_A[] = {"Une magnifique structure de donnees", " geniale"}; // fig 1 from subject
char const * buffers_B[] = {"Les betteraves sont violettes", "carottes", "cuites", "et", " les ", " La"};

// Same as struct piece, without the next pointer
struct portion {
  unsigned int buffer_id;
  size_t start;
  size_t length;
};

#define PORTION_END {0, 0, 0}
#define PORTION_IS_END(p) ((p).buffer_id == 0 && (p).start == 0 && (p).length == 0)

/* Creates a piece from a portion
*/
struct piece * __portion_to_piece(struct portion const * portion)
{
  struct piece * piece = malloc(sizeof(*piece));
  piece->buffer_id = portion->buffer_id;
  piece->start = portion->start;
  piece->length = portion->length;
  piece->next = NULL;
  return piece;
}

extern void set_buffers(char const** buff);

/* Display the piece table, portion by portion, separated by commas */
char * pl__to_comma_string(struct piece_list const * pl, char const * buffers[])
{
    assert(buffers != NULL);
    assert(pl != NULL);

    size_t size = 0;
    size_t capacity = 1;
    char * str = malloc(capacity);

    for (struct piece * p = pl->head; p != PIECE_SENTINEL_PTR; p = p->next) {
      /* fprintf(stderr, "<%d,%d,%d>\n", p->buffer_id, p->start, p->length); */
        capacity += p->length+1;
        str = realloc(str, capacity);
        memcpy(str + size, buffers[p->buffer_id] + p->start, p->length);
        str[size+p->length] = '|';
        size += p->length+1;
    }

    if (size > 0)
      size--;
    str[size] = 0;

    return str;
}


/* Pushes all portions in `portions` at the back of piece list `pl`
   The array `portions` shall be terminated by {0, 0, 0} (PORTION_END)
*/
void __push_back(struct piece_list * pl, struct portion portions[])
{
  for (struct portion * portion = portions; !PORTION_IS_END(*portion); ++portion)
    add_tail(pl, __portion_to_piece(portion));
}

/* Memory assertion
*/
#define ASSERT_MEMORY_NOBLOCK(__begin, __end, __expected, __on, ...)    \
    do {                                                            \
        long leaks = ALLOCATED(__end) - ALLOCATED(__begin); \
        long faults = __end.fault - __begin.fault;          \
        int has_failed;                                             \
        if ((has_failed = (__expected != __on)))                    \
            MSG(__VA_ARGS__);                                       \
        if (leaks)                                                  \
            MSG(COLOR(FAILED, "Memory leak"));                   \
        if (faults)                                                 \
            MSG(COLOR(FAILED, "Invalid free")" on %ld pointer%s",   \
                    faults, faults == 1 ? "" : "s");                \
        if (has_failed || leaks || faults) {                        \
            FAILED("Called by:\n\t%s", last_call_buffer);           \
            return;                                                 \
        } else                                                      \
            PASSED();                                               \
    } while (0)

/********* TEST CASES **********/


NO_TEST_CASE(pl__create)
TEST_FUNCTION(pl__create)
{
  UNUSED_TEST_CASES;
  set_buffers(buffers_A); // Should not be very useful

  struct piece_list * pl = DESCRIBE(pl__create(), "pl__create()");

  ASSERT_TRUE((pl != NULL), "le pointeur peut être déréférencé");
  ASSERT_PTR(PIECE_SENTINEL_PTR, pl->head, "pointeur `head`");
  ASSERT_PTR(PIECE_SENTINEL_PTR, pl->tail, "pointeur `tail`");

  __pl__free(pl);
}



NO_TEST_CASE(pl__create_memory)
TEST_FUNCTION_WITH(pl__create_memory, pl__create)
{
  UNUSED_TEST_CASES;
  set_buffers(buffers_A); // Should not be very useful

  setbuf(stdout, NULL); //printf stdout no extra alloc or extra memory leak.
  struct a_stats begin = BEGIN_MEMORY_CHECKING();

  struct piece_list * pl = DESCRIBE(pl__create(), "pl__create()");

  ASSERT_EXACT_MSG(sizeof(struct piece_list), pointer_info(pl),
                     "Allocation taille incorrecte. " CALLED_BY);
  
  __pl__free(pl);

  struct a_stats finally = END_MEMORY_CHECKING();

  ASSERT_MEMORY_NOBLOCK(begin, finally,
                        1, finally.allocated - begin.allocated,
                        "Gestion mémoire incorrecte");

  ASSERT_MEMORY_NOBLOCK(begin, finally,
                        1, finally.freed - begin.freed,
                        "Gestion mémoire incorrecte");

  ASSERT_MEMORY_NOBLOCK(begin, finally,
                        0, finally.reallocated - begin.reallocated,
                        "Gestion mémoire incorrecte");
}



TEST_CASES(pl__free_memory, struct portion portions[10], char const * msg, char const** buffs)
{
  {{PORTION_END}, "piece table vide", buffers_A},
  {{{0, 0, 34}, PORTION_END}, "piece table de taille 1", buffers_A},
  {{{0, 23, 5}, {1, 24, 12}, PORTION_END}, "piece table de taille 2", buffers_A},
  {{{0, 4, 3}, {0, 17, 6}, {1, 4, 2}, {0, 10, 0}, {2, 2, 2}, {0, 10, 3}, PORTION_END}, "piece table de taille > 2", buffers_B},
}

TEST_FUNCTION_WITH(pl__free_memory, pl__free)
{
  FOR_EACH_TEST {
    setbuf(stdout, NULL); //printf stdout no extra alloc or extra memory leak.
    set_buffers(_.buffs);
    struct a_stats begin = BEGIN_MEMORY_CHECKING();

    // Create, then free the piece list
    struct piece_list * pl = __pl__create();
    __push_back(pl, _.portions);
    DESCRIBE(pl__free(pl), _.msg);
    struct a_stats finally = END_MEMORY_CHECKING();


    unsigned long const allocated = finally.allocated - begin.allocated;
    unsigned long const reallocated = finally.reallocated - begin.reallocated;
    unsigned long const freed = finally.freed - begin.freed;

    ASSERT_MEMORY_NOBLOCK(begin, finally, 0, reallocated, "Gestion mémoire incorrecte");
    ASSERT_MEMORY_NOBLOCK(begin, finally, 0, allocated - freed, "Gestion mémoire incorrecte");
  }
}



TEST_CASES(pl__to_string, struct portion portions[10], char const * expected, char const * msg)
{
  {{PORTION_END}, "", "piece table vide"},
  {{{0, 0, 29}, PORTION_END}, "Les betteraves sont violettes", "piece table de taille 1"},
  {{{0, 0, 4}, {1, 0, 8}, PORTION_END}, "Les carottes", "piece table de taille 2"},
  {{{0, 0, 4}, {1, 0, 8}, {0, 14, 6}, {2, 0, 6}, PORTION_END}, "Les carottes sont cuites", "piece table de taille > 2"},
  {{{5, 1, 1}, {4, 2, 3}, {1, 0, 6}, {0, 12, 3}, {3, 0, 2}, {4, 0, 3}, {0, 2, 18}, {2, 0, 4}, {0, 27, 2}, PORTION_END}, "Les carottes et les betteraves sont cuites", "piece table de taille > 2"},
  {{{0, 7, 0}, {0, 0, 4}, {1, 0, 0}, {1, 0, 8}, {0, 5, 0}, PORTION_END}, "Les carottes", "piece table de taille > 2, portions de longueur 0"},
}

TEST_FUNCTION(pl__to_string)
{
  FOR_EACH_TEST {
    set_buffers(buffers_B);
    // Create the piece list
    struct piece_list * pl = __pl__create();
    __push_back(pl, _.portions);

    // Check string
    char * str = DESCRIBE(pl__to_string(pl, buffers_B), _.msg);
    ASSERT_STRING(_.expected, str, "texte attendu dans la piece table ");
    free(str);
    
    __pl__free(pl);
  }
}



TEST_CASES(pl__to_string_memory, struct portion portions[10], char const * expected, char const * msg)
{
  {{PORTION_END}, "", "piece table vide"},
  {{{0, 0, 29}, PORTION_END}, "Les betteraves sont violettes", "piece table de taille 1"},
  {{{0, 0, 4}, {1, 0, 8}, PORTION_END}, "Les carottes", "piece table de taille 2"},
  {{{0, 0, 4}, {1, 0, 8}, {0, 14, 6}, {2, 0, 6}, PORTION_END}, "Les carottes sont cuites", "piece table de taille > 2"},
  {{{5, 1, 1}, {4, 2, 3}, {1, 0, 6}, {0, 12, 3}, {3, 0, 2}, {4, 0, 3}, {0, 2, 18}, {2, 0, 4}, {0, 27, 2}, PORTION_END}, "Les carottes et les betteraves sont cuites", "piece table de taille > 2"},
  {{{0, 7, 0}, {0, 0, 4}, {1, 0, 0}, {1, 0, 8}, {0, 5, 0}, PORTION_END}, "Les carottes", "piece table de taille > 2, portions de longueur 0"},
}

TEST_FUNCTION_WITH(pl__to_string_memory, pl__to_string)
{
  FOR_EACH_TEST {
    set_buffers(buffers_B);
    // Create the piece list
    struct piece_list * pl = __pl__create();
    __push_back(pl, _.portions);

    struct a_stats begin = BEGIN_MEMORY_CHECKING();

    // Check string
    char * str = DESCRIBE(pl__to_string(pl, buffers_B), _.msg);

    ASSERT_EXACT_MSG(strlen(_.expected) + 1, pointer_info(str),
                     "Allocation taille incorrecte. " CALLED_BY);
    free(str);

    struct a_stats finally = END_MEMORY_CHECKING();

    ASSERT_MEMORY_NOBLOCK(begin, finally,
			  0, finally.allocated_size - finally.freed_size,
                          "Mauvaise utilisation de la mémoire dynamique");
    __pl__free(pl);
  }
}


TEST_CASES(pl__split_piece, struct portion portions[20], size_t pos, char const * msg)
{
  {{{1, 3, 5}, PORTION_END}, 2, "Piece table de taille 1, découpage au milieu"},
  {{{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, PORTION_END}, 4, "Piece table de taille 3, découpage de la 2ème portion"},
  {{{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, PORTION_END}, 8, "Piece table de taille 3, découpage de la 3ème portion"},
  {{{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, PORTION_END}, 0, "Piece table de taille 3, découpage en position 0"},
  {{{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, PORTION_END}, 23, "Piece table de taille 3, position invalide"},
  {{PORTION_END}, 0, "Piece table vide, position invalide"},
  {{{1, 5, 0}, PORTION_END}, 0, "Découpage d'une portion de longueur 0"},
}

TEST_FUNCTION(pl__split_piece)
{
  set_buffers(buffers_B);
  FOR_EACH_TEST {
    // Create expected piece list
    struct piece_list * expected_pl = pl__create();
    __push_back(expected_pl, _.portions);
    struct piece * expected_p = __pl__split_piece(expected_pl, _.pos);

    // Create the piece list and split
    struct piece_list * pl = pl__create();
    __push_back(pl, _.portions);
    struct piece * p = DESCRIBE(pl__split_piece(pl, _.pos), _.msg);

    // Check that returned piece tables are equal
    if (expected_p == NULL) {
      ASSERT_PTR(NULL, p, AS_RETURN);
    }
    else {
      // Check returned pieces
      ASSERT_FALSE((NULL == p), AS_RETURN);

      ASSERT_UINT(expected_p->buffer_id, p->buffer_id, AS_RETURN);
      ASSERT_UINT(expected_p->start, p->start, AS_RETURN);
      ASSERT_UINT(expected_p->length, p->length, AS_RETURN);

      // Check that the two piece lists coincide
      expected_p = expected_pl->head;
      p = pl->head;
      for (; expected_p != PIECE_SENTINEL_PTR && p != PIECE_SENTINEL_PTR;
           expected_p = expected_p->next, p = p->next) {
        ASSERT_UINT(expected_p->buffer_id, p->buffer_id, AS_RESULT);
        ASSERT_UINT(expected_p->start, p->start, AS_RESULT);
        ASSERT_UINT(expected_p->length, p->length, AS_RESULT);
      }

      ASSERT_PTR(PIECE_SENTINEL_PTR, expected_p, " piece table incorrecte");
      ASSERT_PTR(PIECE_SENTINEL_PTR, p, " piece table incorrecte");
    }

    // Finalize
    pl__free(expected_pl);
    pl__free(pl);
  }
}


TEST_CASES(pl__split_piece_memory, struct portion portions[20], size_t pos, size_t expected_malloc_calls, char const * msg)
{
  {{{1, 3, 5}, PORTION_END}, 2, 1, "Piece table de taille 1, découpage au milieu"},
    {{{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, PORTION_END}, 4, 1, "Piece table de taille 3, découpage de la 2ème portion"},
      {{{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, PORTION_END}, 8, 1, "Piece table de taille 3, découpage de la 3ème portion"},
	{{{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, PORTION_END}, 0, 1, "Piece table de taille 3, découpage en position 0"},
	  {{{0, 0, 2}, {1, 4, 5}, {0, 7, 9}, PORTION_END}, 23, 0, "Piece table de taille 3, position invalide"},
	    {{PORTION_END}, 0, 0, "Piece table vide, position invalide"},
	      {{{1, 5, 0}, PORTION_END}, 0, 0, "Découpage d'une portion de longueur 0"},
}

TEST_FUNCTION_WITH(pl__split_piece_memory, pl__split_piece)
{
  set_buffers(buffers_B);
  FOR_EACH_TEST {
    setbuf(stdout, NULL); //printf stdout no extra alloc or extra memory leak.

    struct a_stats begin = BEGIN_MEMORY_CHECKING();

    struct piece_list * pl = pl__create();
    __push_back(pl, _.portions);

    unsigned long const allocated_by_us = get_a_stats().allocated;

    // Create the piece list, split and free
    DESCRIBE(pl__split_piece(pl, _.pos), _.msg);
    
    pl__free(pl);

    struct a_stats finally = END_MEMORY_CHECKING();

    unsigned long const reallocated = finally.reallocated - begin.reallocated;

    ASSERT_MEMORY_NOBLOCK(begin, finally, 0, reallocated, "Gestion mémoire incorrecte");
    ASSERT_MEMORY_NOBLOCK(begin, finally, _.expected_malloc_calls, finally.allocated - allocated_by_us, "Gestion mémoire incorrecte");
  }
}


TEST_CASES(pl__erase, struct portion portions[10], size_t pos, int expected_return, char const * expected_str, char const * msg)
{
  {{{0, 0, 35}, PORTION_END}, 3, 0, "Unemagnifique structure de donnees", "Piece table de taille 1, suppression au milieu"},
  {{{1, 0, 8}, PORTION_END}, 0, 0, "geniale", "piece table de taille 1, suppression au début"},
  {{{1, 0, 8}, PORTION_END}, 7, 0, " genial", "piece table de taille 1, suppression en fin"},
  {{{0, 0, 4}, {0, 15, 20}, {1, 0, 8}, PORTION_END}, 8, 0, "Une struture de donnees geniale", "piece table de taille > 1, suppression au milieu"},
  {{{1, 1, 7}, {0, 14, 12}, {1, 2, 1}, {0, 27, 8}, PORTION_END}, 19, 0, "geniale structure d donnees", "piece table de taille > 1, suppression portion de taille 1"},
  {{{1, 1, 7}, {0, 14, 12}, {1, 2, 1}, {0, 27, 8}, PORTION_END}, 27, 0, "geniale structure de donnee", "piece table de taille > 1, suppression en fin"},
  {{PORTION_END}, 2, -1, "", "suppression piece table vide"},
  {{{0, 0, 4}, {0, 15, 20}, {1, 0, 8}, PORTION_END}, 32, -1, "Une structure de donnees geniale", "piece table de taille > 1, hors borne"},
}

TEST_FUNCTION(pl__erase)
{
  set_buffers(buffers_A);
  FOR_EACH_TEST {
    // Create the piece list
    struct piece_list * pl = __pl__create();
    __push_back(pl, _.portions);

    // Erase
    int res = DESCRIBE(pl__erase(pl, _.pos), _.msg);
    ASSERT_INT(_.expected_return, res, AS_RETURN);

    // Check string
    char * str = __pl__to_string(pl, buffers_A);
    ASSERT_STRING(_.expected_str, str, "texte attendu dans la piece table ");

    free(str);
    __pl__free(pl);
  }
}



NO_TEST_CASE(pl__erase_multiple)
TEST_FUNCTION_WITH(pl__erase_multiple, pl__erase)
{
  UNUSED_TEST_CASES;
  set_buffers(buffers_B);

  // Create a place table
  struct piece_list * pl = __pl__create();
  struct portion portions[] = {{5, 1, 1}, {0, 1, 3}, {1, 0, 4}, {0, 25, 4}, {0, 14, 5}, {4, 0, 1}, {2, 0, 6}, PORTION_END};
  __push_back(pl, portions); // "Les carottes sont cuites" (buffers_B)

  int res = 0;
  char * str = NULL;

  // Erase 'a'
  res = DESCRIBE(pl__erase(pl, 5), "Suppression de 'a' en position 5");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_B);
  ASSERT_STRING("Les crottes sont cuites", str, AS_RESULT);
  free(str);

  // Erase 'u'
  res = DESCRIBE(pl__erase(pl, 18), "Suppression de 'u' en position 18");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_B);
  ASSERT_STRING("Les crottes sont cites", str, AS_RESULT);
  free(str);

  // Erase last char 's'
  res = DESCRIBE(pl__erase(pl, 21), "Suppression du dernier caractère 's' en position 21");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_B);
  ASSERT_STRING("Les crottes sont cite", str, AS_RESULT);
  free(str);

  // Erase first char 'L'
  res = DESCRIBE(pl__erase(pl, 0), "Suppression du premier caractère 'L' en position 0");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_B);
  ASSERT_STRING("es crottes sont cite", str, AS_RESULT);
  free(str);

  // Erase first char 'e'
  res = DESCRIBE(pl__erase(pl, 0), "Suppression du premier caractère 'e' en position 0");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_B);
  ASSERT_STRING("s crottes sont cite", str, AS_RESULT);
  free(str);

  // Erase last char 'e'
  res = DESCRIBE(pl__erase(pl, 18), "Suppression du dernier caractère 'e' en position 18");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_B);
  ASSERT_STRING("s crottes sont cit", str, AS_RESULT);
  free(str);

  // Erase char 'n'
  res = DESCRIBE(pl__erase(pl, 12), "Suppression de 'n' en position 12");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_B);
  ASSERT_STRING("s crottes sot cit", str, AS_RESULT);
  free(str);

  // Try erase out-of-bound
  res = DESCRIBE(pl__erase(pl, 17), "Suppression en position 17 (hors bornes)");
  ASSERT_INT(-1, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_B);
  ASSERT_STRING("s crottes sot cit", str, AS_RESULT);
  free(str);

  __pl__free(pl);
}


TEST_CASES(pl__insert, struct portion portions[10], struct portion portion, size_t pos, int expected_return, char const * expected_str, char const * msg)
{
  {{{0, 0, 29}, PORTION_END}, {2, 0, 6}, 14, 0, "Les betteravescuites sont violettes", "piece table de taille 1, insertion au milieu"},
  {{PORTION_END}, {0, 4, 10}, 0, 0, "betteraves", "piece table vide, insertion en tête"},
  {{PORTION_END}, {1, 0, 1}, 1, -1, "", "piece table vide, insertion position invalide"},
  {{{1, 0, 8}, PORTION_END}, {0, 0, 4}, 0, 0, "Les carottes", "piece table de taille 1, insertion en tête"},
  {{{0, 4, 11}, PORTION_END}, {0, 20, 9}, 11, 0, "betteraves violettes", "piece table de taille 1, insertion en queue"},
  {{{0, 0, 3}, {0, 19, 10}, PORTION_END}, {1, 0, 8}, 4, 0, "Les carottesviolettes", "piece table de taille > 1, insertion au début d'une portion"},
  {{{0, 0, 15}, {0, 15, 14}, PORTION_END}, {2, 0, 6}, 14, 0, "Les betteravescuites sont violettes", "piece table de taille > 1, insertion en fin d'une portion"},
  {{{5, 1, 1}, {0, 1, 3}, {1, 0, 8}, {2, 0, 6}, {3, 0, 2}, {0, 19, 10}, PORTION_END}, {0, 14, 6}, 12, 0, "Les carottes sont cuiteset violettes", "piece table de taille > 1, insertion au début d'une portion"},
  {{{4, 1, 4}, {0, 4, 25}, PORTION_END}, {2, 0, 5}, 14, 0, "les betteravescuite sont violettes", "piece table de taille > 1, insertion au milieu d'une portion"},
  {{{5, 1, 1}, {0, 1, 3}, {1, 0, 8}, PORTION_END}, {1, 2, 3}, 13, -1, "Les carottes", "piece table de taille > 1, insertion position invalide"},
}

TEST_FUNCTION(pl__insert)
{
  set_buffers(buffers_B);
  FOR_EACH_TEST {
    // Create the piece list
    struct piece_list * pl = __pl__create();
    __push_back(pl, _.portions);

    // Insert
    int res = DESCRIBE(pl__insert(pl, _.pos, _.portion.buffer_id, _.portion.start, _.portion.length), _.msg);
    ASSERT_INT(_.expected_return, res, AS_RETURN);

    // Check string
    char * str = __pl__to_string(pl, buffers_B);
    ASSERT_STRING(_.expected_str, str, "texte attendu dans la piece table ");

    free(str);
    __pl__free(pl);
  }
}



NO_TEST_CASE(pl__insert_multiple)
TEST_FUNCTION_WITH(pl__insert_multiple, pl__insert)
{
  UNUSED_TEST_CASES;
  set_buffers(buffers_A);

  int res = 0;
  char * str = NULL;

  // Create an empty place table
  struct piece_list * pl = __pl__create();

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("", str, AS_RESULT);
  free(str);

  // Insert "structure " at head
  res = DESCRIBE(pl__insert(pl, 0, 0, 15, 10), "Insertion de 'structure ' en tête");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("structure ", str, AS_RESULT);
  free(str);

  // Insert "donnees" at tail
  res = DESCRIBE(pl__insert(pl, 10, 0, 28, 7), "Insertion de 'donnees' en queue");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("structure donnees", str, AS_RESULT);
  free(str);

  // Insert " de" in the middle
  res = DESCRIBE(pl__insert(pl, 9, 0, 24, 3), "Insertion de ' de' au milieu");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("structure de donnees", str, AS_RESULT);
  free(str);

  // Insert " geniale" at tail
  res = DESCRIBE(pl__insert(pl, 20, 1, 0, 8), "Insertion de ' geniale' en queue");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("structure de donnees geniale", str, AS_RESULT);
  free(str);

  // Insert 'Une " at head
  res = DESCRIBE(pl__insert(pl, 0, 0, 0, 4), "Insertion de 'Une ' en tête");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("Une structure de donnees geniale", str, AS_RESULT);
  free(str);

  // Out-of-bounds insertion
  res = DESCRIBE(pl__insert(pl, 33, 0, 0, 4), "Insertion en position invalide");
  ASSERT_INT(-1, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("Une structure de donnees geniale", str, AS_RESULT);
  free(str);

  // Insertion in the middle of a piece
  res = DESCRIBE(pl__insert(pl, 6, 0, 4, 3), "Insertion de 'mag' en milieu d'une portion");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("Une stmagructure de donnees geniale", str, AS_RESULT);
  free(str);

  // Insertion at the head of a piece
  res = DESCRIBE(pl__insert(pl, 16, 1, 2, 2), "Insertion de 'en' en tête d'une portion");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("Une stmagructureen de donnees geniale", str, AS_RESULT);
  free(str);

  // Insertion at the tail of a piece
  res = DESCRIBE(pl__insert(pl, 37, 0, 34, 1), "Insertion de 's' en fin d'une portion");
  ASSERT_INT(0, res, AS_RETURN);

  str = __pl__to_string(pl, buffers_A);
  ASSERT_STRING("Une stmagructureen de donnees geniales", str, AS_RESULT);
  free(str);

  __pl__free(pl);
}


NO_TEST_CASE(pl__insert_multiple_memory)
TEST_FUNCTION_WITH(pl__insert_multiple_memory, pl__insert)
{
  UNUSED_TEST_CASES;
  set_buffers(buffers_A);

  setbuf(stdout, NULL); //printf stdout no extra alloc or extra memory leak.
  struct a_stats begin = BEGIN_MEMORY_CHECKING();

  // Create an empty place table, then insert pieces
  struct piece_list * pl = __pl__create();

  pl__insert(pl, 0, 0, 15, 10);  // Insert "structure " at head
  pl__insert(pl, 10, 0, 28, 7);  // Insert "donnees" at tail
  pl__insert(pl, 9, 0, 24, 3);   // Insert " de" in the middle
  pl__insert(pl, 20, 1, 0, 8);   // Insert " geniale" at tail
  pl__insert(pl, 0, 0, 0, 4);    // Insert 'Une " at head
  pl__insert(pl, 33, 0, 0, 4);   // Out-of-bounds insertion
  pl__insert(pl, 6, 0, 4, 3);    // Insertion in the middle of a piece
  pl__insert(pl, 16, 1, 2, 2);   // Insertion at the head of a piece
  pl__insert(pl, 37, 0, 34, 1);  // Insertion at the tail of a piece

  __pl__free(pl);

  struct a_stats finally = END_MEMORY_CHECKING();

  unsigned long const allocated = (finally.allocated - begin.allocated) + (finally.reallocated - begin.reallocated);
  unsigned long const freed = finally.freed - begin.freed;

  ASSERT_MEMORY_NOBLOCK(begin, finally, 0, allocated - freed, "Gestion mémoire incorrecte");
}



TEST_CASES(pl__compress, struct portion init_portions[20], struct portion expected_portions[20], char const * msg)
{
  {{PORTION_END}, {PORTION_END}, "piece table vide"},
  {{{1, 4, 2}, {2, 3, 2}, {4, 7, 2}, PORTION_END}, {{1, 4, 2}, {2, 3, 2}, {4, 7, 2}, PORTION_END}, "piece table sans portions à compresser"},
  {{{1, 4, 0}, {2, 3, 0}, {0, 2, 7}, {1, 4, 0}, {0, 12, 3}, {1, 12, 1}, {3, 4, 0}, {7, 9, 0}, PORTION_END}, {{0, 2, 7}, {0, 12, 3}, {1, 12, 1}, PORTION_END}, "piece table avec des portions de longueur 0, sans portions adjacentes"},
  {{{0, 1, 2}, {0, 3, 4}, {0, 7, 8}, PORTION_END}, {{0, 1, 14}, PORTION_END}, "piece table contenant uniquement des portions adjacentes"},
  {{{2, 2, 2}, {2, 4, 2}, {0, 6, 7}, {0, 13, 1}, {1, 2, 3}, {0, 5, 2}, {0, 7, 6}, {0, 13, 1}, {0, 14, 9}, PORTION_END}, {{2, 2, 4}, {0, 6, 8}, {1, 2, 3}, {0, 5, 18}, PORTION_END}, "piece table avec des portions adjacentes et des portions non adjacentes"},
  {{{1, 2, 3}, {1, 5, 3}, {0, 6, 7}, {0, 13, 1}, {6, 7, 0}, {1, 2, 3}, {5, 4, 0}, {3, 4, 0}, {8, 9, 0}, {0, 5, 2}, {0, 7, 6}, {0, 13, 1}, {0, 14, 0}, {0, 14, 9}, PORTION_END}, {{1, 2, 6}, {0, 6, 8}, {1, 2, 3}, {0, 5, 18}, PORTION_END}, "piece table avec des portions adjacentes et des portions de longueur 0"},
  {{{1, 4, 0}, {2, 3, 0}, {0, 2, 7}, {1, 4, 0}, {0, 9, 3}, {1, 12, 1}, {2, 1, 5}, {3, 4, 0}, PORTION_END}, {{0, 2, 10}, {1, 12, 1}, {2, 1, 5}, PORTION_END}, "piece table avec des portions adjacentes, des portions non adjacentes et des portions de longueur 0"},
  {{{1, 4, 0}, {2, 3, 0}, {0, 2, 7}, {1, 4, 0}, {0, 2, 0}, {1, 89, 0}, {0, 9, 3}, {1, 12, 1}, {1, 13, 2}, {1, 15, 5}, {0, 4, 0}, PORTION_END}, {{0, 2, 10}, {1, 12, 8}, PORTION_END}, "piece table avec des portions adjacentes, des portions non adjacentes et des portions de longueur 0"},
}

TEST_FUNCTION(pl__compress)
{
  set_buffers(buffers_B);
  FOR_EACH_TEST {
    // compute compressed piece table
    struct piece_list * pl = pl__create();
    __push_back(pl, _.init_portions);
    DESCRIBE(pl__compress(pl), _.msg);

    // compute expected compressed piece list
    struct piece_list * expected_pl = pl__create();
    __push_back(expected_pl, _.expected_portions);

    // check that the two piece tables coincide
    struct piece * p = pl->head, * expected_p = expected_pl->head;
    for (; p != PIECE_SENTINEL_PTR && expected_p != PIECE_SENTINEL_PTR; p = p->next, expected_p = expected_p->next) {
      ASSERT_UINT(expected_p->buffer_id, p->buffer_id, "portion incorrecte ");
      ASSERT_UINT(expected_p->start, p->start, "portion incorrecte ");
      ASSERT_UINT(expected_p->length, p->length, "portion incorrecte ");
    }
    ASSERT_PTR(PIECE_SENTINEL_PTR, p, "piece table compressée incorrecte");
    ASSERT_PTR(PIECE_SENTINEL_PTR, expected_p, "piece table compressée incorrecte");

    pl__free(pl);
    pl__free(expected_pl);
  }
}

TEST_CASES(pl__compress_memory, struct portion init_portions[20], struct portion expected_portions[20], char const * msg)
{
  {{PORTION_END}, {PORTION_END}, "piece table vide"},
  {{{1, 4, 2}, {2, 1, 4}, {0, 7, 9}, PORTION_END}, {{1, 4, 2}, {2, 1, 4}, {0, 7, 9}, PORTION_END}, "piece table sans portion de longueur 0"},
  {{{1, 4, 0}, {2, 3, 0}, {0, 2, 7}, {1, 4, 0}, {0, 12, 3}, {1, 12, 1}, {3, 4, 0}, {7, 9, 0}, PORTION_END}, {{0, 2, 7}, {0, 12, 3}, {1, 12, 1}, PORTION_END}, "piece table avec des portions de longueur 0, sans portions adjacentes"},
  {{{0, 1, 2}, {0, 3, 4}, {0, 7, 8}, PORTION_END}, {{0, 1, 14}, PORTION_END}, "piece table contenant uniquement des portions adjacentes"},
  {{{1, 2, 3}, {1, 5, 3}, {0, 6, 7}, {0, 13, 1}, {1, 2, 3}, {0, 5, 2}, {0, 7, 6}, {0, 13, 1}, {0, 14, 9}, PORTION_END}, {{1, 2, 6}, {0, 6, 8}, {1, 2, 3}, {0, 5, 18}, PORTION_END}, "piece table avec des portions adjacentes et des portions non adjacentes"},
  {{{1, 2, 3}, {1, 5, 3}, {0, 6, 7}, {0, 13, 1}, {6, 7, 0}, {1, 2, 3}, {5, 4, 0}, {3, 4, 0}, {8, 9, 0}, {0, 5, 2}, {0, 7, 6}, {0, 13, 1}, {0, 14, 0}, {0, 14, 9}, PORTION_END}, {{1, 2, 6}, {0, 6, 8}, {1, 2, 3}, {0, 5, 18}, PORTION_END}, "piece table avec des portions adjacentes et des portions de longueur 0"},
  {{{1, 4, 0}, {2, 3, 0}, {0, 2, 7}, {1, 4, 0}, {0, 9, 3}, {1, 12, 1}, {0, 13, 6}, {3, 4, 0}, PORTION_END}, {{0, 2, 10}, {1, 12, 1}, {0, 13, 6}, PORTION_END}, "piece table avec des portions adjacentes, des portions non adjacentes et des portions de longueur 0"},
}

TEST_FUNCTION_WITH(pl__compress_memory, pl__compress)
{
  set_buffers(buffers_B);
  FOR_EACH_TEST {

    setbuf(stdout, NULL); //printf stdout no extra alloc or extra memory leak.
    struct a_stats begin = BEGIN_MEMORY_CHECKING();

    // compute compressed piece table
    struct piece_list * pl = pl__create();
    __push_back(pl, _.init_portions);
    DESCRIBE(pl__compress(pl), _.msg);
    pl__free(pl);

    struct a_stats finally = END_MEMORY_CHECKING();

    unsigned long const allocated = finally.allocated - begin.allocated;
    unsigned long const reallocated = finally.reallocated - begin.reallocated;
    unsigned long const freed = finally.freed - begin.freed;

    ASSERT_MEMORY_NOBLOCK(begin, finally, 0, reallocated, "Gestion mémoire incorrecte");
    ASSERT_MEMORY_NOBLOCK(begin, finally, 0, allocated - freed, "Gestion mémoire incorrecte");
  }
}


NO_TEST_CASE(pl__full)
TEST_FUNCTION_WITH(pl__full, pl__compress)
{
  UNUSED_TEST_CASES;

  ASSERT_IMPLEMENTED_SILENT(pl__create);
  ASSERT_IMPLEMENTED_SILENT(pl__compress);
  ASSERT_IMPLEMENTED_SILENT(pl__erase);
  ASSERT_IMPLEMENTED_SILENT(pl__free);
  ASSERT_IMPLEMENTED_SILENT(pl__insert);
  ASSERT_IMPLEMENTED_SILENT(pl__to_string);

  int res = 0;
  char * str = NULL;

  // Create an empty place table
  set_buffers(buffers_A);
  struct piece_list * pl = pl__create();

  str = pl__to_string(pl, buffers_A);
  ASSERT_STRING("", str, AS_RESULT);
  free(str);

  // Insert some piece
  set_buffers(buffers_B);
  res = DESCRIBE(pl__insert(pl, 0, 0, 3, 11), "Insertion de ' betteraves' en tête");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING(" betteraves", str, AS_RESULT);
  free(str);

  // Erase in the middle of a piece
  res = DESCRIBE(pl__erase(pl, 3), "Suppression du 1er 't'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING(" beteraves", str, AS_RESULT);
  free(str);

  // Insert at the beginning of a block
  res = DESCRIBE(pl__insert(pl, 0, 4, 1, 3), "Insertion de 'les' en tête");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("les beteraves", str, AS_RESULT);
  free(str);

  // Compress
  DESCRIBE(pl__compress(pl), "Compression");

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("les beteraves", str, AS_RESULT);
  free(str);

  // Erase from the beginning of a block
  res = DESCRIBE(pl__erase(pl, 0), "Suppression du 1er caractère 'l'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("es beteraves", str, AS_RESULT);
  free(str);

  // Erase at the end of a block
  res = DESCRIBE(pl__erase(pl, 11), "Suppression du dernier caractère 's'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("es beterave", str, AS_RESULT);
  free(str);

  // Insert at the middle of a piece
  res = DESCRIBE(pl__insert(pl, 1, 2, 1, 4), "Insertion de 'uite' au milieu d'une portion");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites beterave", str, AS_RESULT);
  free(str);

  // Erase last 7 characters over two pieces
  res = DESCRIBE(pl__erase(pl, 14), "Suppression du dernier caractère 'e'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites beterav", str, AS_RESULT);
  free(str);

  res = DESCRIBE(pl__erase(pl, 13), "Suppression du dernier caractère 'v'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites betera", str, AS_RESULT);
  free(str);

  res = DESCRIBE(pl__erase(pl, 12), "Suppression du dernier caractère 'a'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites beter", str, AS_RESULT);
  free(str);

  res = DESCRIBE(pl__erase(pl, 11), "Suppression du dernier caractère 'r'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites bete", str, AS_RESULT);
  free(str);

  res = DESCRIBE(pl__erase(pl, 10), "Suppression du dernier caractère 'e'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites bet", str, AS_RESULT);
  free(str);

  res = DESCRIBE(pl__erase(pl, 9), "Suppression du dernier caractère 't'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites be", str, AS_RESULT);
  free(str);

  res = DESCRIBE(pl__erase(pl, 8), "Suppression du dernier caractère 'e'");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites b", str, AS_RESULT);
  free(str);

  // Compress
  DESCRIBE(pl__compress(pl), "Compression");

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites b", str, AS_RESULT);
  free(str);

  // Invalid erase
  res = DESCRIBE(pl__erase(pl, 8), "Suppression invalide");
  ASSERT_INT(-1, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites b", str, AS_RESULT);
  free(str);

  // Invalid insert
  res = DESCRIBE(pl__insert(pl, 9, 2, 1, 4), "Insertion invalide");
  ASSERT_INT(-1, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites b", str, AS_RESULT);
  free(str);

  // Insert at end
  res = DESCRIBE(pl__insert(pl, 8, 0, 19, 10), "Insertion de ' violettes' en queue");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("euites b violettes", str, AS_RESULT);
  free(str);

  // Insert at head
  res = DESCRIBE(pl__insert(pl, 0, 0, 0, 4), "Insertion de 'Les ' en tête");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("Les euites b violettes", str, AS_RESULT);
  free(str);

  // Insert in the middle
  res = DESCRIBE(pl__insert(pl, 6, 3, 1, 1), "Insertion de 't' au milieu");
  ASSERT_INT(0, res, AS_RETURN);

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("Les eutites b violettes", str, AS_RESULT);
  free(str);

  // Compress
  DESCRIBE(pl__compress(pl), "Compression");

  str = pl__to_string(pl, buffers_B);
  ASSERT_STRING("Les eutites b violettes", str, AS_RESULT);
  free(str);

  pl__free(pl);
}



NO_TEST_CASE(pl__full_memory)
TEST_FUNCTION_WITH(pl__full_memory, pl__compress)
{
  UNUSED_TEST_CASES;

  ASSERT_IMPLEMENTED_SILENT(pl__create);
  ASSERT_IMPLEMENTED_SILENT(pl__compress);
  ASSERT_IMPLEMENTED_SILENT(pl__erase);
  ASSERT_IMPLEMENTED_SILENT(pl__free);
  ASSERT_IMPLEMENTED_SILENT(pl__insert);
  ASSERT_IMPLEMENTED_SILENT(pl__to_string);

  setbuf(stdout, NULL); //printf stdout no extra alloc or extra memory leak.
  set_buffers(buffers_B);
  struct a_stats begin = BEGIN_MEMORY_CHECKING();

  // Create an empty place table
  struct piece_list * pl = DESCRIBE(pl__create(), "Create empty piece table");

  // Insert in an empty list
  DESCRIBE(pl__insert(pl, 0, 0, 3, 11), "Insertion de ' betteraves' en tête");
  // Erase in the middle of a piece
  DESCRIBE(pl__erase(pl, 3), "Suppression du 1er 't'");
  // Insert at the beginning of a block
  DESCRIBE(pl__insert(pl, 0, 4, 1, 3), "Insertion de 'les' en tête");
  // Compress
  DESCRIBE(pl__compress(pl), "Compression");
  // Erase from the beginning of a block
  DESCRIBE(pl__erase(pl, 0), "Suppression du 1er caractère 'l'");
  // Erase at the end of a block
  DESCRIBE(pl__erase(pl, 11), "Suppression du dernier caractère 's'");
  // Insert at the middle of a piece
  DESCRIBE(pl__insert(pl, 1, 2, 1, 4), "Insertion de 'uite' au milieu d'une portion");
  // Erase last 7 characters over two pieces
  DESCRIBE(pl__erase(pl, 14), "Suppression du dernier caractère 'e'");
  DESCRIBE(pl__erase(pl, 13), "Suppression du dernier caractère 'v'");
  DESCRIBE(pl__erase(pl, 12), "Suppression du dernier caractère 'a'");
  DESCRIBE(pl__erase(pl, 11), "Suppression du dernier caractère 'r'");
  DESCRIBE(pl__erase(pl, 10), "Suppression du dernier caractère 'e'");
  DESCRIBE(pl__erase(pl, 9), "Suppression du dernier caractère 't'");
  DESCRIBE(pl__erase(pl, 8), "Suppression du dernier caractère 'e'");
  // Compress
  DESCRIBE(pl__compress(pl), "Compression");
  // Invalid erase
  DESCRIBE(pl__erase(pl, 8), "Suppression invalide");
  // Invalid insert
  DESCRIBE(pl__insert(pl, 9, 2, 1, 4), "Insertion invalide");
  // Insert at end
  DESCRIBE(pl__insert(pl, 8, 0, 19, 10), "Insertion de ' violettes' en queue");
  // Insert at head
  DESCRIBE(pl__insert(pl, 0, 0, 0, 4), "Insertion de 'Les ' en tête");
  // Insert in the middle
  DESCRIBE(pl__insert(pl, 6, 3, 1, 1), "Insertion de 't' au milieu");
  // Compress
  DESCRIBE(pl__compress(pl), "Compression");

  pl__free(pl);

  struct a_stats finally = END_MEMORY_CHECKING();

  unsigned long const allocated = finally.allocated - begin.allocated;
  unsigned long const reallocated = finally.reallocated - begin.reallocated;
  unsigned long const freed = finally.freed - begin.freed;

  ASSERT_MEMORY_NOBLOCK(begin, finally, 0, reallocated, "Gestion mémoire incorrecte");
  ASSERT_MEMORY_NOBLOCK(begin, finally, 0, allocated - freed, "Gestion mémoire incorrecte");
}


EXERCICE(
  "piece_list",
  (pl__create),
  (pl__create_memory),
  (pl__free_memory),
  (pl__to_string),
  (pl__to_string_memory),
  (pl__split_piece),
  (pl__split_piece_memory),
  (pl__erase),
  (pl__erase_multiple),
  (pl__insert),
  (pl__insert_multiple),
  (pl__insert_multiple_memory),
  (pl__compress),
  (pl__compress_memory),
  (pl__full),
  (pl__full_memory)
);
