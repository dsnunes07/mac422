#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "race.h"
#include "cyclist.h"
#include "referee.h"
#include "velodrome.h"
#include "lap_list.h"
#include "utils.h"
#include <math.h>
#define ELIMINATED -2
#define CONTINUE -3

struct Cyclist *cyclists;
struct Node* global_current_lap;
struct Node* laps;
int total_cyclists_running;
int total_positions;
int referee_waiting = 1;

// mutex for update cyclists current lap
pthread_mutex_t lap_completed = PTHREAD_MUTEX_INITIALIZER;
// mutex for only one cyclists send a signal referee
pthread_mutex_t wake_referee = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t referee_mutex = PTHREAD_MUTEX_INITIALIZER;
// barriers that controls race flow
pthread_barrier_t step_barrier;

pthread_cond_t referee_finished = PTHREAD_COND_INITIALIZER;
pthread_cond_t cyclists_finished = PTHREAD_COND_INITIALIZER;

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
  // assign laps to cyclists
  printf("numéro de participantes: %d\n", total_cyclists_running);
  for (int i=0; i < total_cyclists_running; i++)
    cyclists[i].current_lap = 1;
}

int eliminatory_lap(struct Cyclist *c) {
  if (c->current_lap > 1 && c->current_lap % 2 == 0) {
    printf("%s completou volta eliminatória\n", c->name);
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
      return lap_list;
    }
    lap_list = lap_list->next;
  }
}

void check_elimination(struct Cyclist *c) {
  struct Node *current_lap = get_lap_data(c->current_lap);
  printf("[%d] verificando eliminação do ciclista %s volta %d %d\n", c->step, c->name, c->current_lap, current_lap->line_crosses);
  if (current_lap->line_crosses == total_cyclists_running) {
    printf("%s deve ser eliminado\n", c->name);
    c->must_stop = 1;
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
/* check if cyclist completed a lap */
void check_new_lap(struct Cyclist *c) {
  if (c->step > 0 && c->crossing_line == 0 && c->last_velodrome_position > c->velodrome_position) {
    update_speed(c);
    c->crossing_line = 1;
    pthread_mutex_lock(&lap_completed);
    printf("[%d] %s completou a volta %d\n", c->step, c->name, c->current_lap);
    update_current_lap_crosses(c);
    if (eliminatory_lap(c))
      check_elimination(c);
    struct Node* current_lap = get_lap_data(c->current_lap);
    if (current_lap->next == NULL) {
      int next_lap_num = current_lap->lap_num + 1;
      struct Node *new_lap = create_new_lap(next_lap_num);
      global_current_lap->next = new_lap;
      global_current_lap = new_lap;
    }
    c->current_lap++;
    pthread_mutex_unlock(&lap_completed);
  }
}

int cyclist_state(struct Cyclist *c) {
  if (c->still_running && c->must_stop) {
    printf("removendo ciclista %s\n", c->name);
    total_cyclists_running--;
    c->still_running = 0;
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

// each cyclist will wait inside its own mutex
// for a signal that he or she can continue the race to
// the next step
void wait_for_referee(struct Cyclist *c) {
  pthread_mutex_lock(&(c->mutex));
  printf("[%d] %s esperando juiza\n", c->step, c->name);
  pthread_cond_wait(&referee_finished, &(c->mutex));
  pthread_mutex_unlock(&(c->mutex));
  printf("[%d] juiza chegou, %s pode continuar\n", c->step, c->name);
}

// a single cyclist, the first that locks the mutex,
// will notify the referee that he can continue to the next
// iteration
void notify_referee() {
  pthread_mutex_lock(&wake_referee);
  if (referee_waiting) {
    pthread_cond_signal(&cyclists_finished);
    referee_waiting = 0;
  }
  pthread_mutex_unlock(&wake_referee);
}

// referee will keep waiting to be unlocked by notify_referee() function
void wait_for_cyclists_to_finish() {
  pthread_mutex_lock(&referee_mutex);
  pthread_cond_wait(&cyclists_finished, &referee_mutex);
  pthread_mutex_unlock(&referee_mutex);
}

// referee will send a signal to every cyclist waiting at
// referee_finished condition
void notify_cyclists() {
  pthread_mutex_lock(&referee_mutex);
  pthread_cond_broadcast(&referee_finished);
  pthread_mutex_unlock(&referee_mutex);
}

int cyclists_size;
void configure_race(int d, int n) {
  total_positions = d;
  total_cyclists_running = n;
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

void check_winner() {
  if (total_cyclists_running > 1)
    return;
  else {
    for (int i = 0; i < cyclists_size; i++) {
      struct Cyclist *c = &(cyclists[i]);
      if (c->still_running) {
        printf("PARABÉNS! %s é o vencedor!\n", c->name);
      }
    }
  }
}

void update_step_barrier() {
  pthread_barrier_destroy(&step_barrier);
  pthread_barrier_init(&step_barrier, NULL, total_cyclists_running);
}
