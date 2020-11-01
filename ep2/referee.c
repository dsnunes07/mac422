#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "race.h"
#include "cyclist.h"
#include "referee.h"

pthread_t referee_thread;
int race_is_on;

struct RaceData{
  int d;
  int n;
};

void *referee_worker(void *args) {
  struct RaceData* race_data = (struct RaceData*) args;;
  int d = race_data->d;
  int n = race_data->n;
  struct Referee *referee = malloc(sizeof(struct Referee*));
  referee->step = 1;
  int referee_cyclists_running = get_total_cyclists_running();
  referee->race_is_on = 1;
  while (referee->race_is_on) {
    lock_cyclists();
    check_eliminations();
    referee->race_is_on = check_winner();
    if (get_total_cyclists_running() < referee_cyclists_running) {
      update_step_barrier();
      printf("barreira atualizada para %d\n", get_total_cyclists_running());
      referee_cyclists_running = get_total_cyclists_running();
    }
    // avança a referencia de tempo local
    referee->step++;
    unlock_cyclists();
  }
  printf("Fim da corrida.\n");
}

void initialize_referee(int d, int n) {
  struct RaceData *race_data = malloc(sizeof(struct RaceData*));
  race_data->d = d;
  race_data->n = n;
  pthread_create(&referee_thread, NULL, referee_worker, race_data);
}