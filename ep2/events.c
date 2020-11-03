#include <stdio.h>
#include "ranking.h"

pthread_mutex_t print_output = PTHREAD_MUTEX_INITIALIZER;

void print_lap_ranking(struct Ranking *r, int lap) {
  pthread_mutex_lock(&print_output);
  printf("--------------------------------------\nRanking final para volta %d\n--------------------------------------\n", lap);
  int classification = 1;
  while (r != NULL) {
    if (!(r->cyclist->broke))
      printf("%-3d. %20s #%-3d %-30s\n", classification++, r->cyclist->name, r->cyclist->number, r->cyclist->country);
    r = r->next;
  }
  pthread_mutex_unlock(&print_output);
}

void notify_cyclist_broke(struct Cyclist *c) {
  pthread_mutex_lock(&print_output);
  printf("[%d] %s quebrou na volta %d!\n", c->step, c->name, c->current_lap);
  pthread_mutex_unlock(&print_output);
}