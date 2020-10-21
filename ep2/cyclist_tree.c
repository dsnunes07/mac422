#include <stdlib.h>
#include <stdio.h>
#include "cyclist_tree.h"
#include "cyclist.h"

/* Creates and returns a new node instance containing given data */
struct Node* create_node(struct Cyclist *cyclist) {
  struct Node *node = malloc(sizeof(struct Node*));
  node->data = cyclist;
  node->left = NULL;
  node->right = NULL;
  return node;
}

struct Node* insert_level_order(int *indexes, struct Cyclist* data, struct Node* root, int i, int n) {
  if (i < n) {
    struct Cyclist *next = &(data[indexes[i]]);
    struct Node* temp = create_node(next);
    
    root = temp;
    root->left = insert_level_order(indexes, data, root->left, 2*i + 1, n);
    root->right = insert_level_order(indexes, data, root->right, 2*i + 2, n);
  }
  return root;
}