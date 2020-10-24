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

EXTERN void start_race(void);

EXTERN void request_updates(struct Cyclist *cyclist);

EXTERN int eliminated(struct Cyclist *cyclist);

EXTERN void update_position(struct Cyclist *c, int d, int lane);

EXTERN void leave_race(struct Cyclist *c);

EXTERN void cross_start_line(struct Cyclist *c);

EXTERN void update_number_of_cyclists_active(int n);

EXTERN void advance_time(struct Cyclist *c);

#undef race_IMPORT
#undef EXTERN
#endif
