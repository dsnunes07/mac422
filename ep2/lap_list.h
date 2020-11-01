#ifndef linked_list_H
#define linked_list_H

#include "ranking.h"

#ifndef linked_list_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

struct Node {
  int lap_num;
  int cyclists_on_lap;
  int line_crosses;
  struct Ranking *lap_ranking;
  struct Node* next;
};

EXTERN void list_append(struct Node* node, struct Node* root);

EXTERN struct Node* create_new_lap(int lap_num);

#undef linked_list_IMPORT
#undef EXTERN
#endif