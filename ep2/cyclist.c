#define cyclist_IMPORT
#include "cyclist.h"
#include "utils.h"
#include "race.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int d = 0;

/* Updates cyclist's current position */
void update_position(struct Cyclist *c) {
  
}

/* function that each cyclist thread will execute */
void *pedal(void * args) {
  struct Cyclist *c = (struct Cyclist*) args;
  while (c->still_running) {
    update_position(c);
  }
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
  }
  initialize_cyclists_threads(cyclists, n);
  return cyclists;
}

/* print all members of cyclist struct in a single line, without labels */
void print_cyclist_data(struct Cyclist *c) {
  printf("%d %s  %d  %s  %d  %d  %d\n", c->id, c->name, c->number, c->country, c->speed, c->position, c->lane);
}

void set_track_length(int length) {
  d = length;
}
