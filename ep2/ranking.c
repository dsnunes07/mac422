#include <stdio.h>
#include "ranking.h"

void append_ranking(struct Ranking *entry, struct Ranking **list) {
  struct Ranking *first = *list;
  if (*list == NULL) {
    *list = entry;
    return;
  }

  while (first->next != NULL) {
    first = first->next;
  }
  first->next = entry;
  entry->last = first;
}

struct Ranking *create_new_ranking_item(struct Cyclist *c, int broke) {
  struct Ranking *ranking = malloc(sizeof(struct Ranking));
  ranking->cyclist = c;
  ranking->next = NULL;
  ranking->last = NULL;
  ranking->broke = broke;
  return ranking;
}