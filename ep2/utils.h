#ifndef utils_H
#define utils_H

#ifndef utils_IMPORT
  #define EXTERN
#else
  #define EXTERN extern
#endif

EXTERN void range(int* arr, int n);

EXTERN void shuffle(int *arr, int n);

EXTERN int random_integer(int low, int high);

EXTERN void print_int_array(int *arr, int n);

EXTERN int flip_coin(int probability);

#undef utils_IMPORT
#undef EXTERN
#endif