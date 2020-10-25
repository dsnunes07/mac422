#ifndef velodrome_H
#define velodrome_H

#include <stdio.h>

#ifdef velodrome_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

/* Module constants */

#define LANES 10

/* Module functions */

EXTERN void create_velodrome(int d);

EXTERN int get_velodrome_position(int d, int lane);

EXTERN void set_velodrome_position(int d, int lane, int cyclist_id);

EXTERN void free_velodrome_position(int d, int lane);

EXTERN int velodrome_position_is_free(int d, int lane);

EXTERN void print_velodrome_position(int d);

#undef velodrome_IMPORT
#undef EXTERN
#endif
