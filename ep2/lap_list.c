#include <stdio.h>
#include <stdlib.h>
#include "lap_list.h"

void list_append(struct Node *node, struct Node *root) {
  struct Node* temp = root;
  while (temp->next != NULL) {
    temp = temp->next;
  }
  temp->next = node;
}

struct Node* create_new_lap(int lap_num) {
  struct Node* lap = malloc(sizeof(struct Node));
  lap->lap_num = lap_num;
  lap->cyclists_on_lap = 1;
  lap->line_crosses = 0;
  lap->next = NULL;
  lap->lap_ranking = NULL;
  return lap;
}
