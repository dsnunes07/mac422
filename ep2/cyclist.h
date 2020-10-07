#ifndef cyclist_H
#define cyclist_H

#include <stdlib.h>

/* The purpose of the module1_IMPORT and the EXTERN macros is to allow the
definition file to be included by client modules AND the implementation of the
module, so that global public variables can be declared only once, and the
compiler can check if the function prototypes do really match their
implementation. */

#ifdef cyclist_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

/* Constants declarations here */

/* Types declarations here */
typedef struct Cyclist cyclist;

/* Global variables declarations here */

/* Function prototypes here */
EXTERN int draw_cyclist_number(int low, int high);

#undef cyclist_IMPORT
#undef EXTERN
#endif