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

#undef race_IMPORT
#undef EXTERN
#endif
