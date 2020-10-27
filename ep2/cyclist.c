#define cyclist_IMPORT
#include "cyclist.h"
#include "utils.h"
#include "race.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

int d = 0;

/* check if cyclist completed a lap */
int new_lap(struct Cyclist *c) {
  if (c->step > 0 && c->crossing_line == 0 && c->last_velodrome_position > c->velodrome_position) {
    c->crossing_line = 1;
    return 1;
  }
  return 0;
}

void update_speed(struct Cyclist *c) {
  /*  0.5: 30km/h 
      1: 60 km/h
      1.5: 90km/h
  */
 if (c->speed == 0.5) {
   if (flip_coin(80))
    c->speed = 1;
 } else if (c->speed == 1) {
   if (flip_coin(40))
    c->speed = 0.5;
 }
 printf("%s velocidade: %f\n==================\n", c->name, c->speed);
}

/* function that each cyclist thread will execute */
void *pedal(void * args) {
  struct Cyclist *c = (struct Cyclist*) args;
  // 60ms da corrida
  while (c->still_running) {
    if (new_lap(c)) {
      complete_lap(c);
      update_speed(c);
    }
    update_position(c);
    advance_step(c);
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
    cyclists[i].speed = 1.0;
    cyclists[i].real_position = 0.0;
    cyclists[i].velodrome_position = 0;
    cyclists[i].last_velodrome_position = 0;
    cyclists[i].lane = 0;
    cyclists[i].still_running = 1;
    cyclists[i].step = 0;
    cyclists[i].checkpoint_ranking = 0;
    cyclists[i].crossing_line = 0;
  }
  return cyclists;
}

/* print all members of cyclist struct in a single line, without labels */
void print_cyclist_data(struct Cyclist *c) {
  printf("%d  %s  %d  %s  %f  %f  %d\n", c->id, c->name, c->number, c->country, c->speed, c->real_position, c->lane);
}

void set_track_length(int length) {
  d = length;
}
