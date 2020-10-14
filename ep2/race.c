#include <stdio.h>
#include <stdlib.h>

#include "race.h"
#include "cyclist.h"
#include "velodrome.h"
#include "utils.h"

struct Cyclist *cyclists;
int *velodrome;

/* Place cylists randomly on start line and return their positions */
int* put_cyclists_on_start_line(struct Cyclist *cyclists, int* positions, int n) {
  int groups = n/5;
  for (int i=0; i<=groups; i++) {
    for (int j=0; i*5 + j < n && j < 5; j++) {
      int position = positions[i*5 + j];
      set_velodrome_position(i, 2*j, cyclists[position].id);
    }
  }
  return positions;
}

void configure_race(int d, int n) {
  create_velodrome(d);
  int starting_positions[n];
  range(starting_positions, n);
  shuffle(starting_positions, n);
  cyclists = create_cyclists(n, starting_positions);
  /* for (int i=0; i<n; i++)
    pthread_join(cyclists[i].thread, NULL); */
  put_cyclists_on_start_line(cyclists, starting_positions, n);
}