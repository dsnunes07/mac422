#define cyclist_IMPORT
#include "cyclist.h"

struct Cyclist {
    char* name;
    int number;
    char* country;
    int speed;
    int position;
    int lane;
};

/* picks a random integer between low and high to associate it to a cyclist */
int draw_cyclist_number(int low, int high) {
    return (rand() % (high - low + 1)) + low;
}