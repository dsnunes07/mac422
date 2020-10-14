#define cyclist_IMPORT
#include "cyclist.h"
#include "utils.h"
#include "cyclist_tree.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* function that each cyclist thread will execute */
void *pedal(void * args) {
  struct Node* tree_node = (struct Node*) args;
  struct Cyclist* cyclist = (struct Cyclist*) tree_node->data;
  
  if (tree_node->left != NULL) {
    struct Cyclist* left = (struct Cyclist*) tree_node->left->data;
    printf("I am cyclist %s at my left is %s \n",cyclist->name, left->name);
  }

  if (tree_node->right != NULL) {
    struct Cyclist* right = (struct Cyclist*) tree_node->right->data;
    printf("I am cyclist %s at my right is %s \n",cyclist->name, right->name);
  }
  sleep(2);

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

/* name i-th cyclist as ciclista_i+1 */
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
    name_cyclist(i+1, &(cyclists[i].name));
    cyclists[i].id = i;
    cyclists[i].number = draw_cyclist_number(i, n);
    cyclists[i].country = "Brasil";
    cyclists[i].speed = 0;
    cyclists[i].position = 0;
    cyclists[i].lane = 0;
  }
  struct Node* root = create_cyclists_tree(cyclists, positions, root, n);
  printf("root cyclist: %s\n", root->data->name);
  initialize_cyclists_threads(root);
  return cyclists;
}

/* print all members of cyclist struct in a single line, without labels */
void print_cyclist_data(struct Cyclist *c) {
  printf("%d %s  %d  %s  %d  %d  %d\n", c->id, c->name, c->number, c->country, c->speed, c->position, c->lane);
}
