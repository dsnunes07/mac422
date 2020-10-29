#ifndef referee_H
#define referee_H

/* The purpose of the module1_IMPORT and the EXTERN macros is to allow the
definition file to be included by client modules AND the implementation of the
module, so that global public variables can be declared only once, and the
compiler can check if the function prototypes do really match their
implementation. */

#ifdef referee_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif
/* Global variables declarations here */

struct Referee {
  int step;
  int race_is_on;
};

EXTERN void initialize_referee(int n, int d);

/* Function prototypes here */

#undef referee_IMPORT
#undef EXTERN
#endif