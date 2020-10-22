#include <stdio.h>
#include <stdlib.h>
#include "race.h"
#include "cyclist.h"
#include "velodrome.h"
#include "utils.h"

struct Cyclist *cyclists;
int *velodrome;
int total_cyclists;

/* Place cylists randomly at start line and return their positions */
void put_cyclists_on_start_line(struct Cyclist *cyclists, int n) {
  int starting_positions[n];
  range(starting_positions, n);
  shuffle(starting_positions, n);
  int groups = n/5;
  for (int i=0; i<=groups; i++) {
    for (int j=0; i*5 + j < n && j < 5; j++) {
      int start = starting_positions[i*5 + j];
      set_velodrome_position(i, 2*j, cyclists[start].id);
      cyclists[start].lane = 2*j;
      cyclists[start].position = i;
    }
  }
}

void configure_race(int d, int n) {
  total_cyclists = n;
  create_velodrome(d);
  set_track_length(d);
  cyclists = create_cyclists(n);
  put_cyclists_on_start_line(cyclists, n);
}

void request_updates(struct Cyclist *cyclist) {
  int d = cyclist->position;
  int lane = cyclist->lane;
  int req_d = cyclist->req_position;
  int req_lane = cyclist->req_lane;
  // check if cyclist can make the requested movement
  if (velodrome_position_is_free(req_d, req_lane)) {
    // empty old position
    set_velodrome_position(d, lane, -1);
    // updates position
    set_velodrome_position(req_d, req_lane, cyclist->id);
    cyclist->position = req_d;
    cyclist->lane = req_lane;
  }
}
