#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "race.h"
#include "cyclist.h"
#include "referee.h"
#include "events.h"

pthread_t referee_thread;
int race_is_on;

struct RaceData{
  int d;
  int n;
  int debug;
};

void *referee_worker(void *args) {
  struct RaceData* race_data = (struct RaceData*) args;;
  int d = race_data->d;
  int n = race_data->n;
  int time = 60;
  struct Referee *referee = malloc(sizeof(struct Referee*));
  referee->step = 1;
  int referee_cyclists_running = get_total_cyclists_running();
  referee->race_is_on = 1;
  while (referee->race_is_on) {
    lock_cyclists();
    // usleep(time*1000);
    check_eliminations();
    // check_rankings();
    referee->race_is_on = check_winner();
    if (get_total_cyclists_running() < referee_cyclists_running) {
      update_step_barrier();
      referee_cyclists_running = get_total_cyclists_running();
      if (referee_cyclists_running == 2) {
        // simulate race on intervals of 20ms
        time = 20;
      }
    }
    // imprime o velodromo inteiro caso debug ativo
    if (race_data->debug) {
      printf("Pista no instante %d ms: \n", referee->step * time);
      print_velodrome();
      usleep(time * 1000);
    }
    // avança a referencia de tempo local
    referee->step++;
    unlock_cyclists();
  }
  print_final_ranking(get_global_ranking());
  printf("Fim da corrida.\n");
}

void initialize_referee(int d, int n, int debug) {
  struct RaceData *race_data = malloc(sizeof(struct RaceData*));
  race_data->d = d;
  race_data->n = n;
  race_data->debug = debug;
  pthread_create(&referee_thread, NULL, referee_worker, race_data);
}
