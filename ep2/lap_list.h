#ifndef linked_list_H
#define linked_list_H

#ifndef linked_list_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

struct Node {
  int lap_num;
  int line_crosses;
  struct Node* next;
};

EXTERN void list_append(struct Node* node, struct Node* root);

EXTERN struct Node* create_new_lap(int lap_num);

#undef linked_list_IMPORT
#undef EXTERN
#endif