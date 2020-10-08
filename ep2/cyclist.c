#define cyclist_IMPORT
#include "cyclist.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* function that each cyclist thread will execute */
void *pedal(void * args) {
  struct Cyclist *c = (struct Cyclist*) args;
  /* while (1) {
    printf("%s aquecendo...\n", c->name);
    sleep(1);
  } */
  printf("I am %s and I will race tonight\n", c->name);

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

  // let cyclists warming up...
  for (int i=0; i < n; i++) {
    pthread_join(cyclists[i].thread, NULL);
  }

  return cyclists;
}

/* print all members of cyclist struct in a single line, without labels */
void print_cyclist_data(struct Cyclist *c) {
  printf("%s  %d  %s  %d  %d  %d\n", c->name, c->number, c->country, c->speed, c->position, c->lane);
}
