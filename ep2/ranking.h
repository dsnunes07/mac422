#ifndef ranking_H
#define ranking_H

#include <stdlib.h>
#include "cyclist.h"

#ifdef ranking_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

struct Ranking {
  struct Cyclist *cyclist;
  int broke;
  struct Ranking *next;
};

EXTERN struct Ranking *create_new_ranking_item(struct Cyclist *c, int broke);

EXTERN void append_ranking(struct Ranking *c, struct Ranking **list);

#undef ranking_IMPORT
#undef EXTERN
#endif
