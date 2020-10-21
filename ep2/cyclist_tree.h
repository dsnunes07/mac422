#ifndef tree_H
#define tree_H

#include "cyclist.h"

#ifdef tree_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

struct Node {
  struct Cyclist *data;
  struct Node *left;
  struct Node *right;
};

EXTERN struct Node* create_node(struct Cyclist *data);

EXTERN struct Node* insert_level_order(int *indexes, struct Cyclist* data, struct Node* root, int i, int n);

#undef tree_IMPORT
#undef EXTERN
#endif