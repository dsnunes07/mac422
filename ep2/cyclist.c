#define cyclist_IMPORT
#include "cyclist.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* function that each cyclist thread will execute */
void *pedal(void * args) {
  struct Cyclist *c = (struct Cyclist*) args;

}

/* name i-th cyclist as ciclista_i+1 */
void name_cyclist(int i, char** name) {
  *name = malloc((11 + i) * sizeof(char));
  snprintf(*name, 11 + i, "ciclista_%d", i);
}
/* picks a random integer between low and high to associate it to a cyclist */
int draw_cyclist_number(int low, int high) {
  return (rand() % (high - low + 1)) + low;
}

/* initializes n threads that represents the cyclists that are going to race */
struct Cyclist* create_cyclists(int n) {
  static struct Cyclist* cyclists;
  cyclists = malloc(n*sizeof(struct Cyclist));
  for (int i=0; i < n; i++) {
    name_cyclist(i+1, &(cyclists[i].name));
    cyclists[i].id = i;
    cyclists[i].number = draw_cyclist_number(i, n);
    cyclists[i].country = "Brasil";
    cyclists[i].speed = 0;
    cyclists[i].position = 0;
    cyclists[i].lane = 0;
    pthread_create(&(cyclists[i].thread), NULL, pedal, &(cyclists[i]));
  }
  return cyclists;
}

/* print all members of cyclist struct in a single line, without labels */
void print_cyclist_data(struct Cyclist *c) {
  printf("%d %s  %d  %s  %d  %d  %d\n", c->id, c->name, c->number, c->country, c->speed, c->position, c->lane);
}

/* Shuffle cyclists before place them on start line, so they start in random positions */
void shuffle_cyclists(struct Cyclist *cyclists, int n) {
  if (n > 1) {
    // uses current time as random seed
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
      // draw a random number between 0 and i
      int random_index = rand() % (i+1);
      struct Cyclist temp = cyclists[i];
      cyclists[i] = cyclists[random_index];
      cyclists[random_index] = temp;
    }
  }
}
