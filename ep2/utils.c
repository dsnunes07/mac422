#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* Fills array arr with n integers in ascending order */
void range(int *arr, int n) {
  for (int i=0; i<n; i++)
    arr[i] = i;
}

/* Evenly shuffles arr */
void shuffle(int *arr, int n) {
  // set current time as random seed
  //srand(time(NULL));
  for (int i=0; i<n; i++) {
    int random_index = rand() % (i+1);
    int temp = arr[i];
    arr[i] = arr[random_index];
    arr[random_index] = temp;
  }
}

int random_integer(int low, int high) {
  return (rand() % (high - low + 1)) + low;
}

void print_int_array(int *arr, int n){
  for (int i=0; i < n; i++)
    printf("%d ", arr[i]);
  printf("\n");
}
