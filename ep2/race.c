#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "race.h"
#include "cyclist.h"
#include "referee.h"
#include "velodrome.h"
#include "lap_list.h"
#include "utils.h"
#include "ranking.h"
#include "events.h"
#include <math.h>
#define ELIMINATED -2
#define CONTINUE -3

struct Cyclist *cyclists;
struct Node* global_current_lap;
struct Node* laps;
int total_cyclists_running;
int total_positions;
int cyclists_finished = 0;
int cyclists_released = 0;
int total_cyclists_participating = 0;

// mutex for update cyclists current lap
pthread_mutex_t lap_completed = PTHREAD_MUTEX_INITIALIZER;
// barriers that controls race flow
pthread_barrier_t step_barrier;
int referee_working = 0;

int get_total_cyclists_running() {
  return total_cyclists_running;
}

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

// initialize cyclists laps with first lap of the linked list
void assign_starting_laps() {
  int lap_num = 1;
  laps = create_new_lap(lap_num);
  global_current_lap = laps;
  for (int i=0; i < total_cyclists_running; i++)
    cyclists[i].current_lap = 1;
}

int eliminatory_lap(struct Cyclist *c) {
  if (c->current_lap > 1 && c->current_lap % 2 == 0) {
    return 1;
  }
  return 0;
}

// get a lap from the linked list
struct Node *get_lap_data(int lap_num) {
  // check if laps current lap next lap is null
  struct Node *lap_list = laps;
  while(lap_list != NULL) {
    if (lap_list->lap_num == lap_num) {
      // printf("volta %d total ciclistas: %d\n", lap_num, lap_list->total_cyclists);
      return lap_list;
    }
    lap_list = lap_list->next;
  }
}

void check_elimination(struct Cyclist *c) {
  struct Node *current_lap = get_lap_data(c->current_lap);
  if (current_lap->line_crosses == total_cyclists_running) {
    printf("%s ficou em último na volta %d e será eliminado!\n", c->name, c->current_lap);
    c->must_stop = 1;
  }
}

void lock_cyclists() {
  for (int i = 0; i < total_cyclists_participating; i++) {
    if (cyclists[i].still_running) {
      while (cyclists[i].run)
        usleep(100);
      pthread_mutex_lock(&(cyclists[i].mutex));
    }
  }
}

void unlock_cyclists() {
  for (int i = 0; i < total_cyclists_participating; i++) {
    pthread_mutex_unlock(&(cyclists[i].mutex));
    cyclists[i].run = 1;
  }
}


int get_total_line_crosses(int lap_num) {
  struct Node* lap_list = laps;
  while (lap_list->next != NULL) {
    if (lap_list->lap_num == lap_num)
      return lap_list->line_crosses;
    lap_list = lap_list->next;
  }
  return 0;
}

void update_current_lap_crosses(struct Cyclist *c) {
  struct Node* lap_list = laps;
  while (lap_list != NULL) {
    if (lap_list->lap_num == c->current_lap)
      lap_list->line_crosses++;
    lap_list = lap_list->next;
  }
}

void add_cyclist_lap_data_into_ranking(struct Cyclist *c) {
  struct Node* current_lap = get_lap_data(c->current_lap);
  struct Ranking *r = create_new_ranking_item(c, c->broke);
  append_ranking(r, &(current_lap->lap_ranking));
}

/* check if cyclist completed a lap */
void check_new_lap(struct Cyclist *c) {
  if (c->step > 0 && c->crossing_line == 0 && c->last_velodrome_position > c->velodrome_position) {
    c->crossing_line = 1;
    pthread_mutex_lock(&lap_completed);
    // printf("[%d] %s completou a volta %d\n", c->step, c->name, c->current_lap);
    update_current_lap_crosses(c);
    struct Node* current_lap = get_lap_data(c->current_lap);
    printf("[volta %d] %s %d %d\n", c->current_lap, c->name, current_lap->line_crosses, total_cyclists_running);
    add_cyclist_lap_data_into_ranking(c);
    if (current_lap->line_crosses == total_cyclists_running) {
      print_lap_ranking(get_lap_data(c->current_lap)->lap_ranking, c->current_lap);
    }
    if (eliminatory_lap(c))
      check_elimination(c);
    if (current_lap->next == NULL) {
      int next_lap_num = current_lap->lap_num + 1;
      struct Node *new_lap = create_new_lap(next_lap_num);
      global_current_lap->next = new_lap;
      global_current_lap = new_lap;
    }
    check_if_broken(c);
    if (c->must_stop) {
      pthread_mutex_unlock(&lap_completed);
      return;
    }
    c->current_lap++;
    update_speed(c);
    pthread_mutex_unlock(&lap_completed);
  }
}

int cyclist_state(struct Cyclist *c) {
  if (c->still_running && c->must_stop) {
    return ELIMINATED;
  }
  return CONTINUE;
}

// barrier where all active cyclists will keep waiting
// for each other before wake up the referee to update
// the race
void wait_cyclists_advance() {
  pthread_barrier_wait(&step_barrier);
}

void initialize_race_barriers() {
  pthread_barrier_init(&step_barrier, NULL, total_cyclists_running);
}

int cyclists_size;
void configure_race(int d, int n) {
  total_positions = d;
  total_cyclists_running = n;
  total_cyclists_participating = n;
  cyclists_size = n;
  initialize_race_barriers();
  create_velodrome(d);
  set_track_length(d-1);
  cyclists = create_cyclists(n);
  initialize_referee(d, n);
  put_cyclists_on_start_line(cyclists, n);
  assign_starting_laps();
  initialize_cyclists_threads(cyclists, n);
}

void check_eliminations() {
  for (int i = 0; i < cyclists_size; i++) {
    int state = cyclist_state(&(cyclists[i]));
    if (state == ELIMINATED) {
      total_cyclists_running--;
      cyclists[i].still_running = 0;
    }
  }
}

void check_if_broken(struct Cyclist *c) {
  if ((c->current_lap % 6 == 0) && (get_total_cyclists_running() > 5)) {
    int crash = flip_coin(5);
    if (crash) {
      notify_cyclist_broke(c);
      c->must_stop = 1;
    }
  }
}

int check_winner() {
  if (total_cyclists_running > 1)
    return 1;
  else {
    for (int i = 0; i < cyclists_size; i++) {
      struct Cyclist *c = &(cyclists[i]);
      if (c->still_running) {
        printf("PARABÉNS! %s é o vencedor!\n", c->name);
        c->still_running = 0;
        return 0;
      }
    }
  }
}

void update_step_barrier() {
  pthread_barrier_destroy(&step_barrier);
  pthread_barrier_init(&step_barrier, NULL, total_cyclists_running);
}
