#include "provided.h"

#include <stdio.h>
#include <unistd.h>

struct piece const piece_sentinel = {.buffer_id = 0, .start = 0, .length = 0,
                                     .next = PIECE_SENTINEL_PTR};

int add_head(struct piece_list * pl, struct piece * p)
{
    assert(pl != NULL);
    assert(p != NULL);
    p->next = pl->head;
    pl->head = p;
    if (pl->tail == PIECE_SENTINEL_PTR)
        pl->tail = p;
    return 0;
}

int add_after(struct piece_list * pl, struct piece * after, struct piece * p)
{
    assert(pl != NULL);
    assert(after != NULL);
    assert(p != NULL);

    if (after == PIECE_SENTINEL_PTR)
        return -1;
    if (p == PIECE_SENTINEL_PTR)
        return -1;

    p->next = after->next;
    after->next = p;
    if (pl->tail == after)
        pl->tail = p;
    return 0;
}

int add_tail(struct piece_list * pl, struct piece * p)
{
    assert(pl != NULL);
    assert(p != NULL);
    if (pl->head == PIECE_SENTINEL_PTR)
        return add_head(pl, p);
    return add_after(pl, pl->tail, p);
}

struct piece * remove_head(struct piece_list * pl)
{
    assert(pl != NULL);

    if (pl->head == PIECE_SENTINEL_PTR)
        return NULL;

    struct piece * p = pl->head;
    pl->head = pl->head->next;
    p->next = NULL;

    if (pl->tail == p)
        pl->tail = pl->head;

    return p;
}

struct piece * remove_after(struct piece_list * pl, struct piece * after)
{
    assert(pl != NULL);
    assert(after != NULL);

    if (after == PIECE_SENTINEL_PTR)
        return NULL;
    if (after->next == PIECE_SENTINEL_PTR)
        return NULL;

    struct piece * p = after->next;
    after->next = after->next->next;
    p->next = NULL;

    if (pl->tail == p)
        pl->tail = after;

    return p;
}

/* The buffers used inside the debug function */
static char const** global_buffers = NULL;

static char const** get_buffers() {
  return global_buffers;
}
void set_buffers(char const** buff) {
  /* fprintf(stderr, "Setting buffers to %p\n", buff); */
  global_buffers = buff;
}

void pl__debug(const struct piece_list* pl) {
  struct piece* p = pl->head;
  fprintf(stderr, "Piece table : [");
  while (p != PIECE_SENTINEL_PTR) {
    fprintf(stderr, "<%d,%ld,%ld>", p->buffer_id, p->start, p->length);
    p = p->next;
  }
  fprintf(stderr, "]\n              |");

  char const** buff = get_buffers();
  if (buff == NULL) {
    fprintf(stderr, "\npl__debug : BUFFERS NOT INITIALIZED\n");
    return;
  }

  fflush(stderr);

  for (p = pl->head; p != PIECE_SENTINEL_PTR; p = p->next) {
    write(2, buff[p->buffer_id] + p->start, p->length);
    write(2, "|", 1);
  }
  fprintf(stderr, "\n");
}
