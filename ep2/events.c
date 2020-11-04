#include <stdio.h>
#include "ranking.h"

pthread_mutex_t print_output = PTHREAD_MUTEX_INITIALIZER;

void print_lap_ranking(struct Ranking *r, int lap) {
  pthread_mutex_lock(&print_output);
  printf("--------------------------------------\nRanking final para volta %d\n--------------------------------------\n", lap);
  printf("pos. nome número país\n");
  int classification = 1;
  while (r != NULL) {
    if (!(r->cyclist->broke))
      printf("%-3d. %-10s #%-5d %-30s\n", classification++, r->cyclist->name, r->cyclist->number, r->cyclist->country);
    r = r->next;
  }
  pthread_mutex_unlock(&print_output);
}

void print_final_ranking(struct Ranking *r) {
  pthread_mutex_lock(&print_output);
  printf("--------------------------------------\nRanking final da corrida\n--------------------------------------\n");
  int classification = 1;
  printf("pos. ultima vez que cruzou a linha de chegada nome número país\n");
  while (r->cyclist != NULL) {
    if (!(r->cyclist->broke))
      printf("%-24d %dms %-10s #%-5d %-30s\n", classification++, r->cyclist->step * 60, r->cyclist->name, r->cyclist->number, r->cyclist->country);
    else
      printf("[quebrou na volta %d] %-10s %dms #%-5d %-30s\n", r->cyclist->current_lap, r->cyclist->name, r->cyclist->step * 60, r->cyclist->number, r->cyclist->country);
    r = r->last;
  }
  pthread_mutex_unlock(&print_output);
}

void notify_cyclist_broke(struct Cyclist *c) {
  pthread_mutex_lock(&print_output);
  printf("[%d] %s quebrou na volta %d!\n", c->step, c->name, c->current_lap);
  pthread_mutex_unlock(&print_output);
}