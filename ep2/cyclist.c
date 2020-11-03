#define cyclist_IMPORT
#include "cyclist.h"
#include "utils.h"
#include "race.h"
#include "velodrome.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

int d = 0;
pthread_mutex_t update_position_mutex;

void update_speed(struct Cyclist *c) {
  /*  0.5: 30km/h 
      1: 60 km/h
      1.5: 90km/h
  */
 if (c->speed == 0.5) {
   if (flip_coin(80))
    c->speed = 1;
 } else if (c->speed == 1) {
   if (flip_coin(40))
    c->speed = 0.5;
 }
}

void move_forward(struct Cyclist *c) {
  c->real_position += c->speed;
  if (ceil(c->real_position) == c->real_position) {
    c->last_velodrome_position = c->velodrome_position;
    c->velodrome_position = (int) c->real_position;
    c->velodrome_position = c->velodrome_position % d;
    free_velodrome_position(c->last_velodrome_position, c->lane);
    set_velodrome_position(c->velodrome_position, c->lane, c->id);
    c->crossing_line = 0;
  }
}

int completing_new_lap(struct Cyclist *c) {
  if (c->step > 0 && c->crossing_line == 0 && c->last_velodrome_position > c->velodrome_position)
    return 1;
  return 0;
}

int break_if_necessary(struct Cyclist *c) {
  double speed = c->speed;
  int position = c->velodrome_position;
  int lane = c->lane;
  int next_id = get_velodrome_position(position+1, lane);
  if (next_id != -1) {
    struct Cyclist *ahead = get_cyclist(next_id);
    if (ahead->speed < c->speed) {
      c->speed = ahead->speed;
    }
  }
}

void overtake(struct Cyclist *c) {
  int position = c->velodrome_position;
  int lane = c->lane;
  set_velodrome_position(position, lane, -1);
  set_velodrome_position(position+1, lane+1, c->id);
}

void update_position(struct Cyclist *c) {
  pthread_mutex_lock(&update_position_mutex);
  break_if_necessary(c);
  move_forward(c);
  pthread_mutex_unlock(&update_position_mutex);
}

/* function that each cyclist thread will execute */
void *pedal(void * args) {
  struct Cyclist *c = (struct Cyclist*) args;
  while (c->still_running) {
    while (!(c->run))
      usleep(100);
    if (c->must_stop) {
      break;
    }
    pthread_mutex_lock(&(c->mutex));
    update_position(c);
    check_new_lap(c);
    // espere os outros ciclistas chegarem nesse passo
    wait_cyclists_advance();
    // avança um passo no tempo local
    c->step++;
    //referee_wake_up();
    pthread_mutex_unlock(&(c->mutex));
    c->run = 0;
  }
  c->run = 0;
  pthread_mutex_unlock(&(c->mutex));
  pthread_exit(NULL);
}

void initialize_cyclists_threads(struct Cyclist *cyclists, int n) {
  pthread_mutex_init(&update_position_mutex, NULL);
  for (int i = 0; i < n; i++)
    pthread_create(&(cyclists[i].thread), NULL, pedal, &(cyclists[i]));
  for (int i = 0; i < n; i++)
    pthread_join(cyclists[i].thread, NULL);
}

/* name i-th cyclist as ciclista_i. For now, cyclist name is "ciclista_(id)" */
void name_cyclist(int i, char** name) {
  *name = malloc((11 + i) * sizeof(char));
  snprintf(*name, 11 + i, "ciclista_%d", i);
}
/* picks a random integer between low and high to associate it to a cyclist */
int draw_cyclist_number(int low, int high) {
  return random_integer(low, high);
}

/* initializes n threads that represents the cyclists that are going to race */
struct Cyclist* create_cyclists(int n) {
  static struct Cyclist* cyclists;
  cyclists = malloc(n*sizeof(struct Cyclist));
  for (int i=0; i < n; i++) {
    name_cyclist(i, &(cyclists[i].name));
    cyclists[i].id = i;
    cyclists[i].number = draw_cyclist_number(i, n);
    cyclists[i].country = "Brasil";
    cyclists[i].speed = 1.0;
    cyclists[i].real_position = 0.0;
    cyclists[i].velodrome_position = 0;
    cyclists[i].last_velodrome_position = 0;
    cyclists[i].lane = 0;
    cyclists[i].still_running = 1;
    cyclists[i].must_stop = 0;
    cyclists[i].step = 1;
    cyclists[i].checkpoint_ranking = 0;
    cyclists[i].crossing_line = 0;
    pthread_mutex_t cyclist_lock = PTHREAD_MUTEX_INITIALIZER;
    cyclists[i].mutex = cyclist_lock;
    cyclists[i].run = 1;
    cyclists[i].broke = 0;
  }
  return cyclists;
}

/* print all members of cyclist struct in a single line, without labels */
void print_cyclist_data(struct Cyclist *c) {
  printf("%d  %s  %d  %s  %f  %f  %d\n", c->id, c->name, c->number, c->country, c->speed, c->real_position, c->lane);
}

void set_track_length(int length) {
  d = length;
}
