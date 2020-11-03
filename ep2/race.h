#ifndef race_H
#define race_H

#include <stdlib.h>
#include "cyclist.h"
#include <pthread.h>

#ifdef race_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/* Function prototypes */

EXTERN void configure_race(int d, int n, int debug);

EXTERN void complete_lap(struct Cyclist *c);

EXTERN void check_new_lap(struct Cyclist *c);

EXTERN void check_if_broken(struct Cyclist *c);

EXTERN void check_eliminations();

EXTERN int check_winner();

EXTERN void update_step_barrier();

EXTERN int get_total_cyclists_running(void);

EXTERN struct Ranking *get_global_ranking(void);

EXTERN void add_to_global_ranking(struct Cyclist *c);

EXTERN void wait_cyclists_advance();

EXTERN void lock_cyclists();

EXTERN void unlock_cyclists();

EXTERN void check_rankings();

EXTERN struct Cyclist *get_cyclist(int id);

EXTERN void print_velodrome();

#undef race_IMPORT
#undef EXTERN
#endif
