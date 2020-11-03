#ifndef cyclist_H
#define cyclist_H

#include <stdlib.h>
#include <pthread.h>

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
struct Cyclist {
  int id;
  char* name;
  int number;
  char* country;
  double speed;
  double real_position;
  int last_velodrome_position;
  int velodrome_position;
  int lane;
  pthread_t thread;
  pthread_mutex_t mutex;
  int still_running;
  int must_stop;
  int step;
  int checkpoint_ranking;
  int crossing_line;
  int current_lap;
  int new_lap;
  int run;
  int broke;
};

/* Function prototypes here */
EXTERN struct Cyclist* create_cyclists(int n);

EXTERN void initialize_cyclists_threads(struct Cyclist *cyclists, int n);

EXTERN int draw_cyclist_number(int low, int high);

EXTERN void print_cyclist_data(struct Cyclist *c);

EXTERN void set_track_length(int length);

EXTERN void update_speed(struct Cyclist *c);

#undef cyclist_IMPORT
#undef EXTERN
#endif