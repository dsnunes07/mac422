#define velodrome_IMPORT
#include "velodrome.h"

#include <stdlib.h>

int *velodrome = NULL;

/* Allocates velodrome, that is an integer array contained to this module. If a
position (d, lane) is empty, then its value is -1. Otherwise, the value is the
id of the cyclist in this position */
void create_velodrome(int d) {
  velodrome = malloc(d * LANES * sizeof(int));
  // initializes all positions as empty
  for (int i = 0; i < d * LANES; i++)
    velodrome[i] = -1;
}

/* If the given position has a cyclist, returns its id. Otherwise, returns -1 */
int get_velodrome_position(int d, int lane) {
  return velodrome[d*LANES + lane];
}

/* Place or remove (by setting -1) an cyclist in a velodrome position */
void set_velodrome_position(int d, int lane, int cyclist_id) {
  velodrome[d*LANES + lane] = cyclist_id;
}

/* Check if given position has a cyclist */
int velodrome_position_is_free(int d, int lane) {
  int pos = velodrome[d*LANES + lane];
  if (pos == -1)
    return 1;
  return 0;
}
