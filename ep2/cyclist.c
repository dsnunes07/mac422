#define cyclist_IMPORT
#include "cyclist.h"
#include "utils.h"
#include "cyclist_tree.h"
#include "race.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int race_started = 0;
int d = 0;

/* Synchronizes threads with tree barrier */
void synchronize(struct Cyclist* current_cyclist, struct Cyclist* left, struct Cyclist* right) {
  // leaf
  if (left == NULL && right == NULL) {
    current_cyclist->step_finished = 1;
    while (current_cyclist->can_move_on == 0);
    request_updates(current_cyclist);
    current_cyclist->can_move_on = 0;
  } else {
    // inner nodes and root
    if (left != NULL) {
      while (left->step_finished == 0);
      left->step_finished = 0;
    }
    if (right != NULL) {
      while (right->step_finished == 0);
      right->step_finished = 0;
    }
    if (!(current_cyclist->is_root)){
      current_cyclist->step_finished = 1;
      while (current_cyclist->can_move_on == 0);
      request_updates(current_cyclist);
      current_cyclist->can_move_on = 0;
    }
    if (left != NULL)
      left->can_move_on = 1;
    if (right != NULL)
      right->can_move_on = 1;
  }
}

void update_cyclist_speed(struct Cyclist *cyclist) {
  int p = 0;
  if (cyclist->speed == 30) {
    p = 80;
    if (flip_coin(p)) {
      cyclist->speed = 60;
      cyclist->steps_current_speed = 0;
    }
  } else if (cyclist->speed == 60) {
    p = 40;
    if (flip_coin(p)) {
      cyclist->speed = 30;
      cyclist->steps_current_speed = 0;
    }
      
  }
}

void update_cyclist_position(struct Cyclist *cyclist, int step) {
  int speed = cyclist->speed;
  if (speed == 30) {
    if (cyclist->steps_current_speed % 2 == 0)
      cyclist->req_position = cyclist->position + 1;
  } else if (speed == 60) {
    cyclist->req_position = cyclist->position + 1;
  }
}

/* function that each cyclist thread will execute */
void *pedal(void * args) {
  struct Node* tree_node = (struct Node*) args;
  struct Cyclist* cyclist = (struct Cyclist*) tree_node->data;
  struct Cyclist* left = NULL;
  struct Cyclist* right = NULL;
  int current_lap = 1;
  int new_lap = 0;

  if (tree_node->left != NULL) {
    left = tree_node->left->data;
  }

  if (tree_node->right != NULL) {
    right = tree_node->right->data;
  }
  int step = 1;
  while (race_started == 0);
  while (1) {
    if (new_lap) {
      printf("OPA, %s passando na linha de chegada! %d %d %d\n", cyclist->name, current_lap, step, d);
      if (eliminated(cyclist))
        break;
    }
      
    
    update_cyclist_position(cyclist, step);
    if (current_lap > 1 && new_lap) {
      update_cyclist_speed(cyclist);
      new_lap = 0;
    }
    synchronize(cyclist, left, right);
    if (step % d == 0) {
      new_lap = 1;
      current_lap++;
      cyclist->steps_current_speed++;
    }
  }
  
}

struct Node* create_cyclists_tree(struct Cyclist *cyclists, int *positions, struct Node* root, int n) {
  return insert_level_order(positions, cyclists, root, 0, n);
}

void initialize_cyclists_threads(struct Node* root) {
  if (root != NULL) {
    struct Cyclist *cyclist = root->data;
    pthread_create(&(cyclist->thread), NULL, pedal, root);
    initialize_cyclists_threads(root->left);
    initialize_cyclists_threads(root->right);
  }
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
struct Cyclist* create_cyclists(int n, int *positions) {
  static struct Cyclist* cyclists;
  cyclists = malloc(n*sizeof(struct Cyclist));
  for (int i=0; i < n; i++) {
    name_cyclist(i, &(cyclists[i].name));
    cyclists[i].id = i;
    cyclists[i].number = draw_cyclist_number(i, n);
    cyclists[i].country = "Brasil";
    cyclists[i].speed = 0;
    cyclists[i].position = 0;
    cyclists[i].req_position = 0;
    cyclists[i].lane = 0;
    cyclists[i].req_lane = 0;
    cyclists[i].step_finished = 0;
    cyclists[i].can_move_on = 0;
    cyclists[i].is_root = 0;
    cyclists[i].steps_current_speed = 0;
  }
  struct Node* root = create_cyclists_tree(cyclists, positions, root, n);
  root->data->is_root = 1;
  initialize_cyclists_threads(root);
  return cyclists;
}

/* print all members of cyclist struct in a single line, without labels */
void print_cyclist_data(struct Cyclist *c) {
  printf("%d %s  %d  %s  %d  %d  %d\n", c->id, c->name, c->number, c->country, c->speed, c->position, c->lane);
}

void start_race() {
  race_started = 1;
}

void set_track_length(int length) {
  d = length;
}


