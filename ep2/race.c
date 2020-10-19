#include <stdio.h>
#include <stdlib.h>
#include "race.h"
#include "cyclist.h"
#include "velodrome.h"
#include "utils.h"

struct Cyclist *cyclists;
int *velodrome;

/* Place cylists randomly at start line and return their positions */
void put_cyclists_on_start_line(struct Cyclist *cyclists, int* starting_position, int n) {
  int groups = n/5;
  for (int i=0; i<=groups; i++) {
    for (int j=0; i*5 + j < n && j < 5; j++) {
      int start = starting_position[i*5 + j];
      set_velodrome_position(i, 2*j, cyclists[start].id);
      cyclists[start].lane = 2*j;
      cyclists[start].position = i;
    }
  }
}

void configure_race(int d, int n) {
  create_velodrome(d);
  int starting_positions[n];
  range(starting_positions, n);
  shuffle(starting_positions, n);
  cyclists = create_cyclists(n, starting_positions);
  put_cyclists_on_start_line(cyclists, starting_positions, n);
  start_race();
  for (int i=0; i < n; i++) {
    pthread_join(cyclists[i].thread, NULL);
  }
  /* for (int i=0; i<=n/5; i++)
    print_velodrome_position(i);
  print_int_array(starting_positions, n); */
}