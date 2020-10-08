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

#endif
