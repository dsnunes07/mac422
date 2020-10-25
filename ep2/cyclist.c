#define cyclist_IMPORT
#include "cyclist.h"
#include "utils.h"
#include "race.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int d = 0;

/* check if cyclist completed a lap */
int new_lap(struct Cyclist *c) {
  /* step % (d-1) == 0 ==> nova volta */
  if (c->step > 0 && c->step % d == 0)
    return 1;
  return 0;
}

int lap_is_even(struct Cyclist *c) {
  if (c->step % 2*d == 0) {
    return 1;
  }
    
  return 0;
}

/* function that each cyclist thread will execute */
void *pedal(void * args) {
  struct Cyclist *c = (struct Cyclist*) args;
  // 60ms da corrida
  while (c->still_running) {
    if (new_lap(c)) {
      if (lap_is_even(c)) {
        cross_start_line(c);
      } else {
        c->current_lap++;
        advance_step(c);
      }
      
    } else {
      advance_step(c);
    }
    // for now, only moves forward at 60km/h
    update_position(c, (c->position + 1) % d, c->lane);
    usleep(10000);
  }
  pthread_exit(NULL);
}

void initialize_cyclists_threads(struct Cyclist *cyclists, int n) {
  for (int i=0; i < n; i++)
    pthread_create(&(cyclists[i].thread), NULL, pedal, &(cyclists[i]));
}

/* name i-th cyclist as ciclista_i. For now, cyclist name is "ciclista_(id)" */
void name_cyclist(int i, char** name) {
  *name = malloc((11 + i) * sizeof(char));
  snprintf(*name, 11 + i, "ciclista_%d", i);
}
/* picks a random integer between low and high to associate it to a cyclist */
int draw_cyclist_number(int low, int high) {
  return random_integer(low, high);
}

/* initializes n threads that represents the cyclists that are going to race */
struct Cyclist* create_cyclists(int n) {
  static struct Cyclist* cyclists;
  cyclists = malloc(n*sizeof(struct Cyclist));
  for (int i=0; i < n; i++) {
    name_cyclist(i, &(cyclists[i].name));
    cyclists[i].id = i;
    cyclists[i].number = draw_cyclist_number(i, n);
    cyclists[i].country = "Brasil";
    cyclists[i].speed = 0;
    cyclists[i].position = 0;
    cyclists[i].lane = 0;
    cyclists[i].current_lap = 0;
    cyclists[i].still_running = 1;
    cyclists[i].step = 0;
    cyclists[i].checkpoint_ranking = 0;
  }
  return cyclists;
}

/* print all members of cyclist struct in a single line, without labels */
void print_cyclist_data(struct Cyclist *c) {
  printf("%d %s  %d  %s  %d  %d  %d\n", c->id, c->name, c->number, c->country, c->speed, c->position, c->lane);
}

void set_track_length(int length) {
  d = length;
}
