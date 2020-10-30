#ifndef race_H
#define race_H

#include <stdlib.h>
#include "cyclist.h"

#ifdef race_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/* Function prototypes */

EXTERN void configure_race(int d, int n);

EXTERN void reach_step_barrier(struct Cyclist *c);

EXTERN void complete_lap(struct Cyclist *c);

EXTERN void check_new_lap(struct Cyclist *c);

EXTERN void check_eliminations();

EXTERN int check_winner();

EXTERN void update_step_barrier();

EXTERN void analyze_cyclists();

EXTERN int get_total_cyclists_running(void);

EXTERN void wait_for_referee();

EXTERN void notify_referee();

EXTERN void notify_cyclists();

EXTERN void wait_cyclists_advance();

EXTERN void wait_for_cyclists_to_finish();

#undef race_IMPORT
#undef EXTERN
#endif
