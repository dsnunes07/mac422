#include <stdlib.h>
#include <stdio.h>
#include "cyclist.h"

int main(int argc, char **argv) {
    for (int i=0; i < 10; i++) {
        int number = draw_cyclist_number(1, 10);
        printf("Cyclist number is %d\n", number);
    }
}