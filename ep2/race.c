#include <stdio.h>
#include <stdlib.h>
#include "race.h"
#include "cyclist.h"
#include "velodrome.h"
#include "utils.h"

struct Cyclist *cyclists;
int *velodrome;
int total_cyclists;
int line_crosses = 0;

pthread_mutex_t update_position_mutex;
pthread_mutex_t lap_completed;
pthread_barrier_t start_line;
pthread_barrier_t step_barrier;


/* Place cylists randomly at start line and return their positions */
void put_cyclists_on_start_line(struct Cyclist *cyclists, int n) {
  int starting_positions[n];
  range(starting_positions, n);
  shuffle(starting_positions, n);
  int groups = n/5;
  for (int i=0; i<=groups; i++) {
    for (int j=0; i*5 + j < n && j < 5; j++) {
      int start = starting_positions[i*5 + j];
      set_velodrome_position(i, 2*j, cyclists[start].id);
      cyclists[start].lane = 2*j;
      cyclists[start].position = i;
    }
  }
}

void configure_race(int d, int n) {
  total_cyclists = n;
  create_velodrome(d);
  set_track_length(d-1);
  cyclists = create_cyclists(n);
  put_cyclists_on_start_line(cyclists, n);
  pthread_mutex_init(&update_position_mutex, NULL);
  pthread_mutex_init(&lap_completed, NULL);
  pthread_barrier_init(&start_line, NULL, n);
  pthread_barrier_init(&step_barrier, NULL, n);
  initialize_cyclists_threads(cyclists, n);
  for (int i = 0; i < n; i++) {
    pthread_join(cyclists[i].thread, NULL);
  }
}

void update_position(struct Cyclist *c, int d, int lane) {
  pthread_mutex_lock(&update_position_mutex);
  if (velodrome_position_is_free(d, lane)) {
    free_velodrome_position(c->position, c->lane);
    set_velodrome_position(d, lane, c->id);
    c->position = d;
    c->lane = lane;
  } else {
    printf("ciclista bloqueadon\n");
  }
  pthread_mutex_unlock(&update_position_mutex);
}

void leave_race(struct Cyclist *c) {
  free_velodrome_position(c->position, c->lane);
}

void update_number_of_cyclists(int n) {
  total_cyclists = n;
  pthread_barrier_init(&start_line, NULL, n);
}

void check_elimination(struct Cyclist *c) {
  if (c->checkpoint_ranking == total_cyclists) {
    c->still_running = 0;
    update_number_of_cyclists(total_cyclists - 1);
    printf("%s eliminado\n", c->name);
    line_crosses = 0;
  } else {
    c->checkpoint_ranking = 0;
  }
}

void complete_lap(struct Cyclist *c) {
  pthread_mutex_lock(&lap_completed);
  line_crosses++;
  c->checkpoint_ranking = line_crosses;
  pthread_mutex_unlock(&lap_completed);
  c->current_lap++;
}

void winner(struct Cyclist *c) {
  c->still_running = 0;
  printf("%s venceu\n", c->name);
}

void cross_start_line(struct Cyclist *c) {
  complete_lap(c);
  pthread_barrier_wait(&start_line);
  if (total_cyclists > 1) {
    printf("checking if %s is eliminated\n", c->name);
    check_elimination(c);
  } else {
    winner(c);
  }
}

void advance_time(struct Cyclist *c) {
  pthread_barrier_wait(&step_barrier);
  printf("%s está com step = %d\n", c->name, c->step);
}
