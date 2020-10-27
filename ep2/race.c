#include <stdio.h>
#include <stdlib.h>
#include "race.h"
#include "cyclist.h"
#include "velodrome.h"
#include "lap_list.h"
#include "utils.h"
#include <math.h>

#define N_BARRIERS 2

struct Cyclist *cyclists;
int total_cyclists_running;
int next_step_cyclists_running;
int total_positions;
struct Node* global_current_lap;
int current_barrier = 0;
struct Node* laps;

pthread_mutex_t update_position_mutex;
pthread_mutex_t lap_completed;
pthread_barrier_t barriers[N_BARRIERS];


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
      cyclists[start].velodrome_position = i;
    }
  }
}

void initialize_laps() {
  int lap_num = 1;
  laps = create_new_lap(lap_num);
  // assign laps to cyclists
  printf("total cyclists running = %d\n", total_cyclists_running);
  for (int i=0; i < total_cyclists_running; i++)
    cyclists[i].current_lap = laps;
}

void configure_race(int d, int n) {
  total_positions = d;
  total_cyclists_running = n;
  next_step_cyclists_running = n+1;
  create_velodrome(d);
  set_track_length(d-1);
  cyclists = create_cyclists(n);
  put_cyclists_on_start_line(cyclists, n);
  pthread_mutex_init(&update_position_mutex, NULL);
  pthread_mutex_init(&lap_completed, NULL);
  pthread_barrier_init(&(barriers[0]), NULL, n);
  pthread_barrier_init(&(barriers[1]), NULL, n);
  initialize_laps();
  initialize_cyclists_threads(cyclists, n);
  for (int i = 0; i < n; i++) {
    pthread_join(cyclists[i].thread, NULL);
  }
}

void move_forward(struct Cyclist *c) {
  c->real_position += c->speed;
  if (ceil(c->real_position) == c->real_position) {
    c->last_velodrome_position = c->velodrome_position;
    c->velodrome_position = (int) c->real_position;
    c->velodrome_position = c->velodrome_position % total_positions;
    free_velodrome_position(c->last_velodrome_position, c->lane);
    set_velodrome_position(c->velodrome_position, c->lane, c->id);
    c->crossing_line = 0;
  }
}

void update_position(struct Cyclist *c) {
  pthread_mutex_lock(&update_position_mutex);
  move_forward(c);
  pthread_mutex_unlock(&update_position_mutex);
}

void update_total_cyclists_running(int n) {
  next_step_cyclists_running = n;
  pthread_barrier_destroy(&(barriers[!current_barrier]));
  pthread_barrier_init(&(barriers[!current_barrier]), NULL, n);
}

void check_elimination(struct Cyclist *c) { 
  if (fmod(c->current_lap->lap_num, 2) != 0) {
    printf("%s volta atual nao eliminatoria %d\n", c->name, c->current_lap->lap_num);
    return;
  }
  printf("%s volta atual %d ciclistas %d total crosses %d c_ranking %d\n", c->name, c->current_lap->lap_num, total_cyclists_running, c->current_lap->line_crosses, c->checkpoint_ranking);
  if (c->checkpoint_ranking == total_cyclists_running) {
    /* update_total_cyclists_running(total_cyclists_running - 1);
    advance_step(c);
    c->still_running = 0; */
    printf("%s foi eliminado\n", c->name);
  } else {
    c->checkpoint_ranking = 0;
  }
}

void complete_lap(struct Cyclist *c) {
  pthread_mutex_lock(&lap_completed);
  c->current_lap->line_crosses++;
  c->checkpoint_ranking = c->current_lap->line_crosses;
  check_elimination(c);
  if (c->current_lap->next == NULL) {
      struct Node* lap = create_new_lap(c->current_lap->lap_num + 1);
      list_append(lap, c->current_lap);
      printf("%s was the first on lap %d\n", c->name, lap->lap_num);
    }
  c->current_lap = c->current_lap->next;
  printf("%s is now on lap %d\n", c->name, c->current_lap->lap_num);
  pthread_mutex_unlock(&lap_completed);
}

void advance_step(struct Cyclist *c) {
  pthread_barrier_wait(&(barriers[current_barrier]));
  c->step++;
  printf("%s step %d\n", c->name, c->step);
  if (total_cyclists_running > next_step_cyclists_running) {
    printf("number of cyclists is changing from %d to %d on next step......\n", total_cyclists_running, next_step_cyclists_running);
    current_barrier = !current_barrier;
    total_cyclists_running = next_step_cyclists_running;
  }
}
