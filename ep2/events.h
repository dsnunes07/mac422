#ifndef events_H
#define events_H

#include <stdlib.h>
#include "ranking.h"

#ifdef events_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN void print_lap_ranking(struct Ranking *r, int lap);

EXTERN void notify_cyclist_broke(struct Cyclist *c);

#undef events_IMPORT
#undef EXTERN
#endif
