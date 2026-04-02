#include <stdio.h>
#include <stdlib.h>
#include "biquad_design.h"

int main(int argc, char *argv[]) {

    if (argc != 5) {
        return 1;
    }

    int type = atoi(argv[1]);
    float fs = atof(argv[2]);
    float Q = atof(argv[3]);
    float fc = atof(argv[4]);

    if (type < 0 || type > 3) {
        return 1;
    }

    if (fs < 0.0f) {
        return 1;
    }

    if (fc <= 0.0f || fc >= fs / 2) {
        return 1;
    }

    struct biquad filter;

    biquad_design(&filter, type, fs, Q, fc);

    printf("b = [%f, %f, %f];\na = [1, %f, %f];\nfs = %.0f;\n", filter.b0, filter.b1, filter.b2, filter.a1, filter.a2,
           fs);

    return 0;
}
